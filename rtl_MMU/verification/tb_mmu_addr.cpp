// CPLD-ROM-0001 — MMU slot ROM address generation and control-signal regression.
//
// Drives the verilated MMU_addr module (which pulls in MMU_mem_map,
// MMU_addr16, MMU_mem_select and MMU_slot_ctrl) over realistic CPU read/write
// cycles and locks in the observed address/enable/strobe behaviour.
//
// Timing model mirrors the emulator (cpld/rtl_memory_bus.cpp): each CPU cycle
// drives exactly one PHI_0 rising edge and the outputs are sampled while PHI_0
// is high, after that edge. The stateful slot selector therefore reflects the
// slot latched for the current address.
//
// KEY FINDING (locked in by these tests):
//   * With MODE_SEL=1 (the emulator's standing configuration, set in
//     cpld/rtl_cpu_bus.cpp) onboard_fw_n is always high, so the slot-index
//     translation branch of MMU_addr is NEVER taken. Firmware ROM reads use
//     the rom_enabled branch and MEM_ADDR_OUT equals the raw CPU address
//     (bit16 = 0). The slot selector state does not alter MEM_ADDR_OUT.
//   * The slot-index translated address { 2'b00, slot_index, ADDR[11:0] } is
//     only produced when MODE_SEL=0 AND slot firmware is disabled, i.e. the
//     external-card decode path.
//
// Build both the 5-bit default and the FULL_MMU_ADDR variants; see run_tests.sh.

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include "VMMU_addr.h"
#include "verilated.h"

static VMMU_addr *dut = nullptr;
static int g_failures = 0;
static int g_checks = 0;

// Soft-switch / expansion input vector applied on each access.
struct Switches {
    int mode_sel     = 1; // 1 = emulator config; firmware ROM uses raw address
    int slotxrom     = 1; // enable onboard slot firmware ROM path
    int slot3rom     = 0;
    int ramrd        = 0;
    int ramwr        = 0;
    int store80      = 0;
    int altzp        = 0;
    int hires        = 0;
    int page2        = 0;
    int exp16_bank   = 0;
    int exp16_ram    = 0;
    int exp16_write  = 0;
};

static void apply_switches(const Switches &s)
{
    dut->MODE_SEL    = s.mode_sel;
    dut->SW_SLOTXROM = s.slotxrom;
    dut->SW_SLOT3ROM = s.slot3rom;
    dut->SW_RAMRD    = s.ramrd;
    dut->SW_RAMWR    = s.ramwr;
    dut->SW_80STORE  = s.store80;
    dut->SW_ALTZP    = s.altzp;
    dut->SW_HIRES    = s.hires;
    dut->SW_PAGE2    = s.page2;
    dut->EXP16_BANK  = s.exp16_bank;
    dut->EXP16_RAM   = s.exp16_ram;
    dut->EXP16_WRITE = s.exp16_write;
}

// One CPU cycle: low phase, rising edge, sample while high.
static void cpu_cycle(uint16_t addr, int cpu_rw, const Switches &s)
{
    apply_switches(s);
    dut->ADDR   = addr;
    dut->CPU_RW = cpu_rw;

    dut->PHI_0 = 0;
    dut->eval();
    dut->PHI_0 = 1;
    dut->eval();
}

static void do_reset(void)
{
    Switches s;
    apply_switches(s);
    dut->ADDR = 0;
    dut->CPU_RW = 1;
    dut->RESET = 0; // active-low reset asserted
    for (int i = 0; i < 4; i++) {
        dut->PHI_0 = 0; dut->eval();
        dut->PHI_0 = 1; dut->eval();
    }
    dut->RESET = 1;
    dut->PHI_0 = 0; dut->eval();
}

static uint32_t mem_addr(void)
{
    return (uint32_t)dut->MEM_ADDR_OUT;
}

#define CHECK(cond, msg) do {                                            \
    g_checks++;                                                          \
    if (!(cond)) {                                                       \
        g_failures++;                                                    \
        printf("  FAIL: %s\n", msg);                                     \
    }                                                                    \
} while (0)

static void expect_eq(const char *name, uint32_t got, uint32_t exp)
{
    g_checks++;
    if (got != exp) {
        g_failures++;
        printf("  FAIL: %-18s got=0x%05X exp=0x%05X\n", name, got, exp);
    }
}

// Report the signal snapshot for the truth table.
static void dump(const char *label, uint16_t addr)
{
    printf("  %-22s ADDR=%04X ROM_EN_N=%d RAM_EN_N=%d IOSEL_N=%d IOSTB_N=%d "
           "C0XX_N=%d C6XX_N=%d MEM_ADDR_OUT=0x%05X\n",
           label, addr, dut->ROM_EN_N, dut->RAM_EN_N, dut->IOSEL_N,
           dut->IOSTB_N, dut->C0XX_N, dut->C6XX_N, mem_addr());
}

// For firmware ROM (rom_enabled branch) MEM_ADDR_OUT is the raw CPU address in
// the FULL build, or {1'b0, ADDR[15:12]} in the 5-bit build.
static void expect_rom_addr(const char *name, uint16_t addr)
{
#ifdef FULL_MMU_ADDR
    expect_eq(name, mem_addr(), (uint32_t)addr & 0x0FFFF);
#else
    expect_eq(name, mem_addr(), (uint32_t)((addr >> 12) & 0x1F));
#endif
}

int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    dut = new VMMU_addr;

#ifdef FULL_MMU_ADDR
    const char *build = "FULL_MMU_ADDR (17-bit)";
#else
    const char *build = "default (5-bit upper)";
#endif
    printf("=== MMU_addr regression (%s) ===\n", build);

    Switches sw; // defaults: MODE_SEL=1, SLOTXROM=1

    //----------------------------------------------------------------------
    // Test 1: slot entry ROM C100..C700 (IOSEL window), slot firmware on.
    // rom_enabled branch -> MEM_ADDR_OUT == raw CPU address.
    //----------------------------------------------------------------------
    printf("\n[1] Slot entry ROM windows (IOSEL, SW_SLOTXROM=1, MODE_SEL=1)\n");
    for (int slot = 1; slot <= 7; slot++) {
        do_reset();
        uint16_t addr = 0xC000 | (slot << 8); // Cn00
        cpu_cycle(addr, 1, sw);
        dump("CnXX IOSEL read", addr);
        char m[64];
        snprintf(m, sizeof(m), "slot%d ROM_EN_N asserted", slot);
        CHECK(dut->ROM_EN_N == 0, m);
        CHECK(dut->IOSEL_N == 1, "IOSEL_N deasserted when firmware ROM wins");
        expect_rom_addr("slot phys addr", addr);
    }

    //----------------------------------------------------------------------
    // Test 2: C700/C7FF boundaries for slot 7.
    //----------------------------------------------------------------------
    printf("\n[2] Slot 7 entry boundaries C700/C7FF\n");
    do_reset();
    cpu_cycle(0xC700, 1, sw);
    dump("C700 read", 0xC700);
    CHECK(dut->ROM_EN_N == 0, "C700 ROM_EN_N asserted");
    expect_rom_addr("C700 phys", 0xC700);
    cpu_cycle(0xC7FF, 1, sw);
    dump("C7FF read", 0xC7FF);
    CHECK(dut->ROM_EN_N == 0, "C7FF ROM_EN_N asserted");
    expect_rom_addr("C7FF phys", 0xC7FF);

    //----------------------------------------------------------------------
    // Test 3: C800..CFFE expansion window. In MODE_SEL=1 the address is the
    // raw CPU address (slot state does not translate it).
    //----------------------------------------------------------------------
    printf("\n[3] C800 expansion window (IOSTB, firmware ROM)\n");
    do_reset();
    cpu_cycle(0xC700, 1, sw);           // select slot 7
    cpu_cycle(0xC800, 1, sw);           // enter expansion window
    dump("C800 after C700", 0xC800);
    CHECK(dut->ROM_EN_N == 0, "C800 ROM_EN_N asserted");
    CHECK(dut->IOSTB_N == 1, "IOSTB_N deasserted when firmware ROM wins");
    expect_rom_addr("C800 phys", 0xC800);
    cpu_cycle(0xCFFE, 1, sw);
    dump("CFFE after C700", 0xCFFE);
    CHECK(dut->ROM_EN_N == 0, "CFFE ROM_EN_N asserted");
    expect_rom_addr("CFFE phys", 0xCFFE);

    //----------------------------------------------------------------------
    // Test 4: CFFF read. Still a firmware ROM access (rom_enabled) and it
    // resets the slot selector to 0 for subsequent external-decode cycles.
    //----------------------------------------------------------------------
    printf("\n[4] CFFF release behaviour\n");
    do_reset();
    cpu_cycle(0xC700, 1, sw);           // select slot 7
    cpu_cycle(0xCFFF, 1, sw);           // release
    dump("CFFF read", 0xCFFF);
    CHECK(dut->ROM_EN_N == 0, "CFFF ROM_EN_N asserted (firmware ROM)");
    expect_rom_addr("CFFF phys", 0xCFFF);
    cpu_cycle(0xC800, 1, sw);
    dump("C800 after CFFF", 0xC800);
    expect_rom_addr("C800 phys", 0xC800);

    //----------------------------------------------------------------------
    // Test 5: external-card slot translation path (MODE_SEL=0, firmware off).
    // Here the slot-index branch IS taken and MEM_ADDR_OUT is the translated
    // address { 2'b00, slot_index, ADDR[11:0] }.
    //----------------------------------------------------------------------
    printf("\n[5] External-card slot translation (MODE_SEL=0, SW_SLOTXROM=0)\n");
    {
        Switches ext5 = sw; ext5.mode_sel = 0; ext5.slotxrom = 0;
        do_reset();
        cpu_cycle(0xC500, 1, ext5);     // slot 5 IOSEL
        dump("C500 ext slot5", 0xC500);
        CHECK(dut->IOSEL_N == 0, "C500 IOSEL_N asserted (external card)");
#ifdef FULL_MMU_ADDR
        expect_eq("C500 xlat phys", mem_addr(), ((uint32_t)5 << 12) | 0x500);
#else
        expect_eq("C500 xlat upper5", mem_addr(), (uint32_t)5); // {2'b00, slot}
#endif
        cpu_cycle(0xC800, 1, ext5);     // expansion follows selected slot 5
        dump("C800 ext after C500", 0xC800);
#ifdef FULL_MMU_ADDR
        expect_eq("C800 xlat phys", mem_addr(), ((uint32_t)5 << 12) | 0x800);
#else
        expect_eq("C800 xlat upper5", mem_addr(), (uint32_t)5);
#endif
    }

    //----------------------------------------------------------------------
    // Test 6: C6XX_N dedicated decode for an external slot-6 controller.
    //----------------------------------------------------------------------
    printf("\n[6] C6XX_N decode (external slot-6 controller compatibility)\n");
    do_reset();
    Switches ext6 = sw; ext6.slotxrom = 0;
    cpu_cycle(0xC600, 1, ext6);
    dump("C600 ext slot6", 0xC600);
    CHECK(dut->C6XX_N == 0, "C6XX_N asserted at C600 with slot firmware off");
    cpu_cycle(0xC700, 1, ext6);
    dump("C700 ext slot7", 0xC700);
    CHECK(dut->C6XX_N == 1, "C6XX_N deasserted outside C6xx");

    //----------------------------------------------------------------------
    // Test 7: C0XX I/O decode (soft-switch page) is not a ROM window.
    //----------------------------------------------------------------------
    printf("\n[7] C0XX decode\n");
    do_reset();
    cpu_cycle(0xC080, 1, sw);
    dump("C080 read", 0xC080);
    CHECK(dut->C0XX_N == 0, "C0XX_N asserted at C080");
    CHECK(dut->ROM_EN_N == 1, "C0XX is not a ROM window");

    //----------------------------------------------------------------------
    // Test 8: Dxxx-Fxxx high ROM (D_FXXX path, not expansion RAM).
    //----------------------------------------------------------------------
    printf("\n[8] High ROM D000/F000 read\n");
    do_reset();
    cpu_cycle(0xF800, 1, sw);
    dump("F800 read", 0xF800);
    CHECK(dut->ROM_EN_N == 0, "F800 ROM_EN_N asserted (system ROM)");
    expect_rom_addr("F800 phys", 0xF800);
    cpu_cycle(0xD000, 1, sw);
    dump("D000 read", 0xD000);
    CHECK(dut->ROM_EN_N == 0, "D000 ROM_EN_N asserted (system ROM)");
    expect_rom_addr("D000 phys", 0xD000);

    //----------------------------------------------------------------------
    // Test 9: slot firmware disabled (SW_SLOTXROM=0) -> IOSEL exposed, no ROM.
    //----------------------------------------------------------------------
    printf("\n[9] Slot firmware disabled exposes IOSEL, no onboard ROM\n");
    do_reset();
    Switches off = sw; off.slotxrom = 0;
    cpu_cycle(0xC500, 1, off); // slot 5 IOSEL, external card territory
    dump("C500 fw-off", 0xC500);
    CHECK(dut->ROM_EN_N == 1, "C500 no onboard ROM when firmware disabled");
    CHECK(dut->IOSEL_N == 0, "C500 IOSEL_N asserted when firmware disabled");

    //----------------------------------------------------------------------
    // Test 10: write cycle to slot ROM window does not assert ROM_EN_N.
    //----------------------------------------------------------------------
    printf("\n[10] Write to ROM window (CPU_RW=0) must not enable ROM read\n");
    do_reset();
    cpu_cycle(0xC700, 0, sw); // write
    dump("C700 write", 0xC700);
    CHECK(dut->ROM_EN_N == 1, "ROM not enabled on write cycle");

    printf("\n=== %s: %d checks, %d failures ===\n", build, g_checks, g_failures);
    delete dut;
    return g_failures ? 1 : 0;
}
