// CPLD-ROM-0001/0003. Drive the combined IOU/MMU and the same scheduler,
// soft switches and memory controller used by the board.
#include "apple2rom.h"
#include "memory_controller.h"
#include "rom_backing.h"
#include "rtl_model.h"
#include "ram_bus_hw.h"

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

static void expect_rom(cpld_data_t &cpld, uint16_t cpu, uint32_t phys,
                       uint8_t byte, int rom_en_n)
{
    const unsigned previous_traces = traces;
    const uint8_t data = access(cpld, cpu);
    ++checks;
    if (data != byte || cpld.MMU.mem_address != phys || traces != previous_traces + 1 ||
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

int main(void)
{
    cpld_data_t cpld{};
    rtl_model_init();
    rtl_model_reset();
    rtl_model_state_reset(&cpld);
    memory_controller_init();
    rom_backing_set_trace(trace_rom);

    // Empty slots and the reserved expansion window read as an empty bus.
    expect_rom(cpld, 0xC700, 0x07000, 0, 1);
    expect_rom(cpld, 0xC800, 0x07800, 0, 1);
    expect_rom(cpld, 0xCFFF, 0x07FFF, 0, 1);
    expect_rom(cpld, 0xC800, 0x00800, 0, 1);
    expect_rom(cpld, 0xC6FF, 0x060FF, 0, 1);
    check(!cpld.IOU.iosel_6_n && cpld.MMU.IOSEL_N, "slot 6 uses the dedicated decode");

    uint8_t slot6[4096], slot7[4096];
    for (size_t i = 0; i < sizeof(slot7); ++i) {
        slot6[i] = (uint8_t)(0x61 + (i ^ (i >> 8)));
        slot7[i] = (uint8_t)(0x71 + (i ^ (i >> 8)));
    }
    check(memory_controller_load_slot_rom(6, slot6, sizeof(slot6)) == 0, "load raw slot 6");
    check(memory_controller_load_slot_rom(7, slot7, sizeof(slot7)) == 0, "load raw slot 7");
    check(memory_controller_load_slot_rom(7, slot6, sizeof(slot6) - 1) < 0,
          "reject a short binary through the board API");

    // Dump the two complete Apple-facing slot-7 windows through the sampled bus.
    for (uint16_t i = 0; i < 0x100; ++i) {
        expect_rom(cpld, 0xC700 + i, 0x07000 + i, slot7[i], 1);
    }
    for (uint16_t i = 0; i < 0x800; ++i) {
        expect_rom(cpld, 0xC800 + i, 0x07800 + i, slot7[0x800 + i], 1);
    }
    expect_rom(cpld, 0xC800, 0x00800, 0, 1);

    expect_rom(cpld, 0xC600, 0x06000, slot6[0x000], 1);
    expect_rom(cpld, 0xC6FF, 0x060FF, slot6[0x0FF], 1);
    expect_rom(cpld, 0xC800, 0x06800, slot6[0x800], 1);
    expect_rom(cpld, 0xCFFE, 0x06FFE, slot6[0xFFE], 1);
    expect_rom(cpld, 0xC700, 0x07000, slot7[0x000], 1);
    expect_rom(cpld, 0xC800, 0x07800, slot7[0x800], 1);
    const unsigned before_write = traces;
    access(cpld, 0xCFFF, false, 0xFF);
    check(traces == before_write, "CFFF write releases without a ROM read");
    expect_rom(cpld, 0xC800, 0x00800, 0, 1);
    check(access(cpld, 0xC0EC) == 0x6E && floppy_reads == 1 && !cpld.IOU.devsel_6_n,
          "slot-6 I/O still reaches the floppy controller");

    // Internal slot-3 ROM is enabled by default. C00B enables external slot 3.
    expect_apple_rom(cpld, 0xC300);
    expect_apple_rom(cpld, 0xC800);
    expect_apple_rom(cpld, 0xCFFF);
    expect_rom(cpld, 0xC800, 0x00800, 0, 1);
    access(cpld, 0xC00B, false);
    expect_rom(cpld, 0xC300, 0x03000, 0, 1);

    // INTCXROM selects the original Apple Cx bytes even with card ROMs installed.
    access(cpld, 0xC007, false);
    for (unsigned cpu = 0xC100; cpu <= 0xFFFF; ++cpu) {
        expect_apple_rom(cpld, cpu);
    }
    expect_apple_rom(cpld, 0xC800);
    access(cpld, 0xC006, false);
    expect_rom(cpld, 0xC7FF, 0x070FF, slot7[0x0FF], 1);
    expect_apple_rom(cpld, 0xFFFC);
    expect_apple_rom(cpld, 0xFFFD);

    // Exercise the language card via the actual C08x soft switches.
    access(cpld, 0xC083);
    access(cpld, 0xC083);
    access(cpld, 0xD123, false, 0x5A);
    check(access(cpld, 0xD123) == 0x5A && !cpld.MMU.ram_en_n, "language card selects RAM");
    access(cpld, 0xC082);
    expect_apple_rom(cpld, 0xD123);

    check(rom_backing_oob_count() == 0, "all RTL-generated ROM reads in bounds");
    // An invalid address is injected here since MEM_ADDR_OUT is only 17 bits.
    cpld.MMU.mem_address = 0x40000;
    check(memory_controller_access(&cpld, 0xD123, true, 0) == 0 &&
          last_trace.out_of_bounds && rom_backing_oob_count() == 1,
          "memory controller exposes an out-of-bounds ROM read");

    printf("ROM bus: %u checks, %u failures\n", checks, failures);
    return failures != 0;
}
