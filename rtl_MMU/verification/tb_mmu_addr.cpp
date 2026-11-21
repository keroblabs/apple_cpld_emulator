// CPLD-ROM-0001. Drive one PHI_0 rising edge per CPU cycle and sample the
// settled high phase, as the emulator's memory_bus_latch does.
#include <cstdio>
#include <cstdint>
#include "VMMU_addr.h"
#include "verilated.h"

struct Switches {
    int mode_sel = 1;
    int slotxrom = 0;
    int slot3rom = 1;
    int ramrd = 0;
    int exp16_bank = 0;
    int exp16_ram = 0;
};

struct Bus {
    uint32_t address;
    int rom_en_n = 1;
    int ram_en_n = 1;
    int iosel_n = 1;
    int iostb_n = 1;
    int c0xx_n = 1;
    int c6xx_n = 1;
};

static int failures;
static int checks;

static void cpu_cycle(VMMU_addr &dut, uint16_t address, bool read, const Switches &sw)
{
    dut.MODE_SEL = sw.mode_sel;
    dut.SW_SLOTXROM = sw.slotxrom;
    dut.SW_SLOT3ROM = sw.slot3rom;
    dut.SW_RAMRD = sw.ramrd;
    dut.EXP16_BANK = sw.exp16_bank;
    dut.EXP16_RAM = sw.exp16_ram;

    dut.PHI_0 = 0;
    dut.ADDR = address;
    dut.CPU_RW = read;
    dut.eval();
    dut.PHI_0 = 1;
    dut.eval();
}

static void reset(VMMU_addr &dut)
{
    dut.RESET = 0;
    dut.SW_RAMWR = 0;
    dut.SW_80STORE = 0;
    dut.SW_HIRES = 0;
    dut.SW_PAGE2 = 0;
    dut.SW_ALTZP = 0;
    dut.EXP16_WRITE = 0;
    cpu_cycle(dut, 0, true, {});
    dut.RESET = 1;
}

static void expect(const VMMU_addr &dut, const char *test, const char *signal,
                   uint32_t actual, uint32_t expected)
{
    ++checks;
    if (actual != expected) {
        ++failures;
        printf("FAIL %s at $%04X: %s = $%05X, expected $%05X\n",
               test, dut.ADDR, signal, actual, expected);
    }
}

static void expect_bus(const VMMU_addr &dut, const char *test, const Bus &bus)
{
#ifdef FULL_MMU_ADDR
    const uint32_t expected_address = bus.address;
#else
    const uint32_t expected_address = bus.address >> 12;
#endif
    expect(dut, test, "MEM_ADDR_OUT", dut.MEM_ADDR_OUT, expected_address);
    expect(dut, test, "ROM_EN_N", dut.ROM_EN_N, bus.rom_en_n);
    expect(dut, test, "RAM_EN_N", dut.RAM_EN_N, bus.ram_en_n);
    expect(dut, test, "IOSEL_N", dut.IOSEL_N, bus.iosel_n);
    expect(dut, test, "IOSTB_N", dut.IOSTB_N, bus.iostb_n);
    expect(dut, test, "C0XX_N", dut.C0XX_N, bus.c0xx_n);
    expect(dut, test, "C6XX_N", dut.C6XX_N, bus.c6xx_n);
}

int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    VMMU_addr dut;

    const Switches external;
    for (int slot = 1; slot <= 7; ++slot) {
        reset(dut);
        const uint16_t cpu_entry = 0xC000 + slot * 0x100;
        const uint32_t base = slot * 0x1000u;
        Bus entry{base};
        entry.iosel_n = slot == 6;
        entry.c6xx_n = slot != 6;

        cpu_cycle(dut, cpu_entry, true, external);
        expect_bus(dut, "first external IOSEL read", entry);
        cpu_cycle(dut, cpu_entry + 0xFF, true, external);
        entry.address = base + 0xFF;
        expect_bus(dut, "last external IOSEL byte", entry);

        Bus expansion{base + 0x800};
        expansion.iostb_n = 0;
        cpu_cycle(dut, 0xC800, true, external);
        expect_bus(dut, "first external IOSTB byte", expansion);
        cpu_cycle(dut, 0xCFFE, true, external);
        expansion.address = base + 0xFFE;
        expect_bus(dut, "penultimate external IOSTB byte", expansion);
        cpu_cycle(dut, 0xCFFF, true, external);
        expansion.address = base + 0xFFF;
        expect_bus(dut, "CFFF retains selected slot for the read", expansion);
        cpu_cycle(dut, 0xC800, true, external);
        expansion.address = 0x0800;
        expect_bus(dut, "C800 after CFFF release", expansion);
    }

    Switches onboard = external;
    onboard.mode_sel = 0;
    reset(dut);
    cpu_cycle(dut, 0xC5AB, true, onboard);
    expect_bus(dut, "onboard backing for external IOSEL", {0x50AB, 0, 1, 0});
    cpu_cycle(dut, 0xCFFF, true, onboard);
    expect_bus(dut, "onboard backing for external CFFF", {0x5FFF, 0, 1, 1, 0});
    cpu_cycle(dut, 0xCFFF, true, onboard);
    expect_bus(dut, "consecutive CFFF uses released slot", {0x0FFF, 0, 1, 1, 0});
    cpu_cycle(dut, 0xC800, true, onboard);
    expect_bus(dut, "onboard IOSTB after CFFF release", {0x0800, 0, 1, 1, 0});
    cpu_cycle(dut, 0xC6FF, true, onboard);
    expect_bus(dut, "external slot 6 remains on C6XX_N", {0x60FF, 1, 1, 1, 1, 1, 0});
    cpu_cycle(dut, 0xC800, true, onboard);
    expect_bus(dut, "onboard IOSTB after selecting slot 6", {0x6800, 0, 1, 1, 0});
    cpu_cycle(dut, 0xCFFF, false, onboard);
    expect_bus(dut, "CFFF write leaves ROM disabled", {0, 1, 1, 1, 0});
    cpu_cycle(dut, 0xC700, true, onboard);
    expect_bus(dut, "IOSEL immediately after CFFF write", {0x7000, 0, 1, 0});
    cpu_cycle(dut, 0xC700, false, onboard);
    expect_bus(dut, "IOSEL write leaves ROM disabled", {0, 1, 1, 0});
    cpu_cycle(dut, 0xC800, true, onboard);
    expect_bus(dut, "cross-slot expansion after IOSEL", {0x7800, 0, 1, 1, 0});

    Switches internal = external;
    internal.slotxrom = 1;
    reset(dut);
    cpu_cycle(dut, 0xC600, true, internal);
    expect_bus(dut, "internal Apple C6 ROM and C6XX_N decode", {0xC600, 0, 1, 1, 1, 1, 0});
    cpu_cycle(dut, 0xCFFF, true, internal);
    expect_bus(dut, "internal Apple CFFF", {0xCFFF, 0});
    cpu_cycle(dut, 0xC800, true, internal);
    expect_bus(dut, "internal Apple C800 after CFFF", {0xC800, 0});
    cpu_cycle(dut, 0xC7FF, true, internal);
    expect_bus(dut, "internal Apple C7 ROM", {0xC7FF, 0});

    Switches internal_slot3 = external;
    internal_slot3.slot3rom = 0;
    reset(dut);
    cpu_cycle(dut, 0xC300, true, internal_slot3);
    expect_bus(dut, "internal slot 3 IOSEL", {0xC300, 0});
    cpu_cycle(dut, 0xC800, true, internal_slot3);
    expect_bus(dut, "internal slot 3 expansion", {0xC800, 0});
    cpu_cycle(dut, 0xCFFF, true, internal_slot3);
    expect_bus(dut, "internal slot 3 CFFF", {0xCFFF, 0});
    cpu_cycle(dut, 0xC800, true, internal_slot3);
    expect_bus(dut, "internal slot 3 expansion released", {0x0800, 1, 1, 1, 0});

    cpu_cycle(dut, 0xC080, true, external);
    expect_bus(dut, "C0xx I/O only", {0, 1, 1, 1, 1, 0});
    cpu_cycle(dut, 0xD000, true, external);
    expect_bus(dut, "system ROM start", {0xD000, 0});
    cpu_cycle(dut, 0xFFFF, true, external);
    expect_bus(dut, "system ROM end", {0xFFFF, 0});
    cpu_cycle(dut, 0x2000, true, external);
    expect_bus(dut, "main RAM", {0x2000, 1, 0});

    Switches aux = external;
    aux.ramrd = 1;
    cpu_cycle(dut, 0x2000, true, aux);
    expect_bus(dut, "auxiliary RAM", {0x12000, 1, 0});

    Switches language_card = external;
    language_card.exp16_ram = 1;
    cpu_cycle(dut, 0xD123, true, language_card);
    expect_bus(dut, "language card replaces ROM", {0xC123, 1, 0});

    printf("%d checks, %d failures\n", checks, failures);
    return failures != 0;
}
