// CPLD-ROM-0001/0003. Drive the combined IOU/MMU and the same scheduler,
// soft switches and memory controller used by the board.
#include "apple2rom.h"
#include "memory_controller.h"
#include "rom_backing.h"
#include "rtl_model.h"
#include "rtl_interface.h"
#include "ram_bus_hw.h"
#include <fstream>

extern "C" {
#include "disc_drive.h"
}

// The video mirror and disk media are outside this test. Observe the
// controller's slot-6 I/O handoff without touching a disk image.
static unsigned floppy_reads;
extern "C" void disc_drive_init(void) {}
extern "C" void disc_process(uint64_t) {}
extern "C" uint8_t disc_io_access(uint8_t, uint8_t, bool_t write)
{
    floppy_reads += !write;
    return 0x6E;
}

uint8_t ram_bus_hw_access(uint32_t, uint8_t, bool_t) { return 0; }

static unsigned checks;
static unsigned failures;
static unsigned traces;

struct Trace {
    uint32_t cpu_addr;
    uint32_t phys_addr;
    int rom_en_n;
    uint8_t data;
    int out_of_bounds;
};

static Trace last_trace;

static void trace_rom(uint32_t cpu_addr, uint32_t phys_addr, int rom_en_n,
                      uint8_t data, int out_of_bounds)
{
    last_trace = {cpu_addr, phys_addr, rom_en_n, data, out_of_bounds};
    ++traces;
}

static void check(bool ok, const char *message)
{
    ++checks;
    if (!ok) {
        ++failures;
        printf("FAIL: %s\n", message);
    }
}

static uint8_t access(cpld_data_t &cpld, uint16_t addr, bool read = true, uint8_t byte = 0)
{
    rtl_model_iterate(addr, read, byte, &cpld.MMU, &cpld.IOU);
    return memory_controller_access(&cpld, addr, read, byte);
}

static uint16_t spi_exchange(cpld_data_t &cpld, uint8_t selector, uint8_t payload,
                             bool interleave_mailbox_reads = false)
{
    cpld_model_t context = {&cpld.MMU, &cpld.IOU, p_model};
    spi_slave_send_data(&context, selector, payload);

    for (unsigned cycle = 0; cycle < 64 && !spi_slave_is_done(&context); ++cycle) {
        // Each dummy or mailbox access advances both the CPU PHI_0 and SPI
        // master state machines, exercising their normal shared scheduler.
        const uint16_t address = interleave_mailbox_reads && cycle % 3 == 1
                               ? 0xC0F3 : 0x0200;
        access(cpld, address);
    }

    check(spi_slave_is_done(&context), "13-bit SPI transfer completes under concurrent PHI_0");
    return spi_slave_get_received_data(&context);
}

static void test_spi_mailbox_and_keyboard(cpld_data_t &cpld)
{
    access(cpld, 0xC0F1, false, 0xB6);
    check(access(cpld, 0xC0F1) == 0x80,
          "Apple $C0F1 write sets the mailbox pending bit");
    const uint16_t apple_byte = spi_exchange(cpld, 0, 0);
    check((apple_byte & 0xFF) == 0xB6 && ((apple_byte >> 8) & 0x07) == 1,
          "SPI selector 0 consumes the Apple byte and returns its pending flag");
    check(access(cpld, 0xC0F1) == 0,
          "selector 0 clears the Apple-write pending bit through PHI_0");

    spi_exchange(cpld, 1, 0x69);
    check(access(cpld, 0xC0F3) == 0x80,
          "SPI selector 1 publishes a byte and sets the Apple-read pending bit");
    check(access(cpld, 0xC0F2) == 0x69,
          "Apple $C0F2 reads the byte posted by SPI selector 1");
    check(access(cpld, 0xC0F3) == 0,
          "Apple $C0F2 read clears the read-pending bit");

    spi_exchange(cpld, 2, 0x55, true);
    check(access(cpld, 0xC000) == 0xD5,
          "SPI selector 2 publishes the seven-bit keyboard value and pending flag");
    const uint16_t status = spi_exchange(cpld, 5, 0);
    check((((status >> 8) & 0x1F) & (1 << 2)) != 0,
          "selector 5 reports the keyboard pending status bit");
    access(cpld, 0xC010, false, 0);
    check(access(cpld, 0xC000) == 0x55,
          "keyboard strobe clears pending without changing the keycode");
    const uint16_t cleared = spi_exchange(cpld, 5, 0);
    check((((cleared >> 8) & 0x1F) & (1 << 2)) == 0,
          "selector 5 reports keyboard pending cleared");
}

static void expect_rom(cpld_data_t &cpld, uint16_t cpu, uint32_t phys,
                       uint8_t byte, int rom_en_n)
{
    const unsigned previous_traces = traces;
    const uint8_t data = access(cpld, cpu);
    ++checks;
    if (data != byte || cpld.MMU.mem_address >= 0x10000 || cpld.MMU.mem_address != phys ||
        cpld.MMU.bus_rd_n || !cpld.MMU.bus_wr_n ||
        traces != previous_traces + 1 ||
        last_trace.cpu_addr != cpu || last_trace.phys_addr != phys ||
        last_trace.rom_en_n != rom_en_n || last_trace.data != byte || last_trace.out_of_bounds) {
        ++failures;
        printf("FAIL $%04X: read $%02X from $%05X (ROM_EN_N=%d, trace delta=%u); "
               "expected $%02X from $%05X (ROM_EN_N=%d)\n",
               cpu, data, cpld.MMU.mem_address, cpld.MMU.rom_en_n,
               traces - previous_traces, byte, phys, rom_en_n);
    }
}

static void expect_apple_rom(cpld_data_t &cpld, uint16_t cpu)
{
    expect_rom(cpld, cpu, cpu, apple_rom_bin[cpu - 0x8000], 0);
}

static constexpr unsigned compiled_slots[] = {1, 2, 4, 5, 6, 7};

static bool read_export(const char *path, uint8_t *bytes)
{
    const bool written = rom_backing_write_image(path) == 0;
    check(written, "export the ROM from the initialized board controller");
    if (!written) {
        return false;
    }

    std::ifstream input(path, std::ios::binary);
    input.read(reinterpret_cast<char *>(bytes), ROM_BACKING_SIZE);
    const bool complete = input && input.peek() == EOF && !input.bad();
    check(complete, "export contains exactly 131072 bytes");
    if (!complete) {
        return false;
    }

    // Check the whole file, including the IOSEL padding and unused regions.
    static uint8_t expected[0x20000];
    static_assert(sizeof(expected) == ROM_BACKING_SIZE, "128 KiB ROM backing");
    memcpy(expected + 0x8000, apple_rom_bin, sizeof(apple_rom_bin));
    const unsigned offsets[] = {0x000, 0x0FF, 0x100, 0x7FF, 0x800, 0xFFE, 0xFFF};
    for (unsigned slot : compiled_slots) {
        for (unsigned i = 0; i < sizeof(offsets) / sizeof(offsets[0]); ++i) {
            expected[slot * 0x1000 + offsets[i]] = 0x10 * (i + 1) + slot;
        }
    }
    check(memcmp(bytes, expected, sizeof(expected)) == 0,
          "file matches all compiled sources and zero-filled regions at their physical offsets");
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Expected a temporary ROM export path\n");
        return 1;
    }

    cpld_data_t cpld{};
    rtl_model_init();
    rtl_model_reset();
    rtl_model_state_reset(&cpld);
    memory_controller_init();
    rom_backing_set_trace(trace_rom);

    static uint8_t bytes[ROM_BACKING_SIZE];
    if (!read_export(argv[1], bytes)) {
        return 1;
    }
    test_spi_mailbox_and_keyboard(cpld);
    expect_rom(cpld, 0xC800, 0x00800, 0, 1); // No expansion owner at reset.

    // Internal slot-3 ROM remains available from the Apple image.
    expect_apple_rom(cpld, 0xC300);
    expect_apple_rom(cpld, 0xC800);
    expect_apple_rom(cpld, 0xCFFF);
    expect_rom(cpld, 0xC800, 0x00800, 0, 1);
    access(cpld, 0xC00B, false);
    expect_rom(cpld, 0xC300, 0x03000, 0, 1);
    expect_rom(cpld, 0xC3FF, 0x030FF, 0, 1);
    expect_rom(cpld, 0xC800, 0x03800, 0, 1);
    expect_rom(cpld, 0xCFFF, 0x03FFF, 0, 1);
    expect_rom(cpld, 0xC800, 0x00800, 0, 1);

    // Probe every reachable byte of the six compiled slot fixtures.
    for (unsigned slot : compiled_slots) {
        const uint16_t cpu_entry = 0xC000 + slot * 0x100;
        const uint32_t base = slot * 0x1000;
        for (uint16_t offset = 0; offset < 0x100; ++offset) {
            expect_rom(cpld, cpu_entry + offset, base + offset, bytes[base + offset], 1);
        }
        for (uint16_t offset = 0; offset < 0x800; ++offset) {
            const uint32_t phys = base + 0x800 + offset;
            expect_rom(cpld, 0xC800 + offset, phys, bytes[phys], 1);
        }
        expect_rom(cpld, 0xC800, 0x00800, 0, 1);
    }

    expect_rom(cpld, 0xC6FF, 0x060FF, bytes[0x060FF], 1);
    check(!cpld.IOU.iosel_6_n && cpld.MMU.IOSEL_N, "slot 6 uses the dedicated decode");
    expect_rom(cpld, 0xC800, 0x06800, bytes[0x06800], 1);
    expect_rom(cpld, 0xC700, 0x07000, bytes[0x07000], 1);
    expect_rom(cpld, 0xC800, 0x07800, bytes[0x07800], 1);
    const unsigned before_write = traces;
    access(cpld, 0xCFFF, false, 0xFF);
    check(traces == before_write && cpld.MMU.bus_rd_n && cpld.MMU.bus_wr_n,
          "CFFF write releases without a ROM read or memory write strobe");
    const unsigned before_rom_write = traces;
    access(cpld, 0xD123, false, 0x55);
    check(traces == before_rom_write && cpld.MMU.bus_rd_n && cpld.MMU.bus_wr_n,
          "ordinary system-ROM write does not reach flash");
    expect_rom(cpld, 0xC800, 0x00800, 0, 1);
    check(access(cpld, 0xC0EC) == 0x6E && floppy_reads == 1 && !cpld.IOU.devsel_6_n,
          "slot-6 I/O still reaches the floppy controller");

    // Check the Apple runtime ROM bytes against the same exported file.
    access(cpld, 0xC007, false);
    for (unsigned cpu = 0xC100; cpu <= 0xFFFF; ++cpu) {
        expect_rom(cpld, cpu, cpu, bytes[cpu], 0);
    }
    expect_apple_rom(cpld, 0xC800);
    access(cpld, 0xC006, false);
    expect_rom(cpld, 0xC7FF, 0x070FF, bytes[0x070FF], 1);
    expect_apple_rom(cpld, 0xFFFC);
    expect_apple_rom(cpld, 0xFFFD);

    const unsigned before_ram = traces;
    access(cpld, 0x0000, false, 0x5C);
    check(cpld.MMU.bus_rd_n && !cpld.MMU.bus_wr_n,
          "RAM write strobe is sampled and mutually exclusive");
    access(cpld, 0xC009, false); // ALTZP selects auxiliary zero page.
    access(cpld, 0x0000, false, 0xA6);
    check(!cpld.MMU.ram_en_n && cpld.MMU.rom_en_n && cpld.MMU.mem_address == 0x10000,
          "auxiliary zero-page write uses RAM address bit 16");
    check(access(cpld, 0x0000) == 0xA6 && cpld.MMU.bus_rd_n == 0 && cpld.MMU.bus_wr_n &&
          !cpld.MMU.ram_en_n && cpld.MMU.rom_en_n &&
          cpld.MMU.mem_address == 0x10000, "auxiliary zero-page read uses RAM address bit 16");
    check(memory_controller_access(&cpld, 0x0000, FALSE, 0xFF) == 0xA6,
          "sampled BUS_RD_N selects RAM even if the caller direction disagrees");
    cpld.MMU.bus_rd_n = TRUE;
    cpld.MMU.bus_wr_n = FALSE;
    memory_controller_access(&cpld, 0x0000, TRUE, 0x3C);
    cpld.MMU.bus_wr_n = TRUE;
    check(access(cpld, 0x0000) == 0x3C,
          "sampled BUS_WR_N selects RAM even if the caller direction disagrees");
    access(cpld, 0xC008, false);
    check(access(cpld, 0x0000) == 0x5C && !cpld.MMU.ram_en_n && cpld.MMU.mem_address == 0,
          "main zero page kept its own byte");
    check(traces == before_ram && rom_backing_read(0x10000, nullptr) == 0,
          "auxiliary RAM activity leaves the reserved ROM bank untouched");

    // Exercise the language card via the actual C08x soft switches.
    access(cpld, 0xC083);
    access(cpld, 0xC083);
    access(cpld, 0xD123, false, 0x5A);
    check(access(cpld, 0xD123) == 0x5A && !cpld.MMU.ram_en_n, "language card selects RAM");
    access(cpld, 0xC082);
    expect_apple_rom(cpld, 0xD123);

    check(rom_backing_oob_count() == 0, "all RTL-generated ROM reads in bounds");
    // An invalid address is injected here since MEM_ADDR_OUT is only 17 bits.
    cpld.MMU.mem_address = 0x20000;
    check(memory_controller_access(&cpld, 0xD123, true, 0) == 0 &&
          last_trace.phys_addr == 0x20000 && last_trace.out_of_bounds && rom_backing_oob_count() == 1,
          "memory controller exposes an out-of-bounds ROM read");

    printf("ROM bus: %u checks, %u failures\n", checks, failures);
    return failures != 0;
}
