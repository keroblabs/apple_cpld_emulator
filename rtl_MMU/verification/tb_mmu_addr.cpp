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
// DESIGN RULE UNDER TEST (CPLD-ROM-0001 rework, owner directive):
//   IO firmware ROM (IOSEL/IOSTB with onboard firmware enabled) is mapped into
//   a per-slot 4 KiB window: MEM_ADDR_OUT = { slot_index, ADDR[11:0] }.
//     * First 2 KiB of the window ($x000-$x7FF) holds the IOSEL page
//       ($Cn00-$CnFF lands at window + (n<<8) .. +0xFF).
//     * Second 2 KiB ($x800-$xFFF) holds the IOSTB expansion ($C800-$CFFF).
//   System ROM ($D000-$FFFF, D_FXXX) stays mapped to its raw CPU address.
//   This mapping is active in BOTH MODE_SEL=1 (emulator) and the external-card
//   path (MODE_SEL=0), so CFFF slot-selector release is now observable.
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

struct Switches {
    int mode_sel     = 1; // 1 = emulator config
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

static uint32_t mem_addr(void) { return (uint32_t)dut->MEM_ADDR_OUT; }

#define CHECK(cond, msg) do {                                            \
    g_checks++;                                                          \
    if (!(cond)) { g_failures++; printf("  FAIL: %s\n", msg); }          \
} while (0)

static void expect_eq(const char *name, uint32_t got, uint32_t exp)
{
    g_checks++;
    if (got != exp) {
        g_failures++;
        printf("  FAIL: %-20s got=0x%05X exp=0x%05X\n", name, got, exp);
    }
}

static void dump(const char *label, uint16_t addr)
{
    printf("  %-24s ADDR=%04X ROM_EN_N=%d RAM_EN_N=%d IOSEL_N=%d IOSTB_N=%d "
           "C0XX_N=%d C6XX_N=%d MEM_ADDR_OUT=0x%05X\n",
           label, addr, dut->ROM_EN_N, dut->RAM_EN_N, dut->IOSEL_N,
           dut->IOSTB_N, dut->C0XX_N, dut->C6XX_N, mem_addr());
}

// Expected MEM_ADDR_OUT for an IO firmware ROM access to the given CPU address
// in the given selected slot: { slot, ADDR[11:0] } (FULL) or { slot } (5-bit).
static void expect_iofw(const char *name, int slot, uint16_t addr)
{
#ifdef FULL_MMU_ADDR
    expect_eq(name, mem_addr(), ((uint32_t)slot << 12) | (addr & 0x0FFF));
#else
    expect_eq(name, mem_addr(), (uint32_t)slot & 0x1F);
#endif
}

// Expected MEM_ADDR_OUT for a system ROM access (raw CPU address).
static void expect_sys(const char *name, uint16_t addr)
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

    Switches sw; // MODE_SEL=1, SLOTXROM=1

    //----------------------------------------------------------------------
    // Test 1: slot entry ROM C100..C700 (IOSEL) -> per-slot 4 KiB window.
    //----------------------------------------------------------------------
    printf("\n[1] Slot entry IOSEL -> 4 KiB/slot window { slot, ADDR[11:0] }\n");
    for (int slot = 1; slot <= 7; slot++) {
        do_reset();
        uint16_t addr = 0xC000 | (slot << 8); // Cn00
        cpu_cycle(addr, 1, sw);
        dump("CnXX IOSEL read", addr);
        char m[64];
        snprintf(m, sizeof(m), "slot%d ROM_EN_N asserted", slot);
        CHECK(dut->ROM_EN_N == 0, m);
        CHECK(dut->IOSEL_N == 1, "IOSEL_N deasserted when firmware ROM wins");
        // slot==0 would be C000 (I/O page); here slot 1..7, slot_index==slot.
        expect_iofw("slot IOSEL phys", slot, addr);
    }

    //----------------------------------------------------------------------
    // Test 2: slot-7 window boundaries C700/C7FF (IOSEL page in first 2 KiB).
    //----------------------------------------------------------------------
    printf("\n[2] Slot 7 IOSEL boundaries C700/C7FF\n");
    do_reset();
    cpu_cycle(0xC700, 1, sw);
    dump("C700 read", 0xC700);
    CHECK(dut->ROM_EN_N == 0, "C700 ROM_EN_N asserted");
    expect_iofw("C700 phys", 7, 0xC700);   // 0x7000 | 0x700 = 0x7700
    cpu_cycle(0xC7FF, 1, sw);
    dump("C7FF read", 0xC7FF);
    CHECK(dut->ROM_EN_N == 0, "C7FF ROM_EN_N asserted");
    expect_iofw("C7FF phys", 7, 0xC7FF);   // 0x7000 | 0x7FF = 0x77FF
#ifdef FULL_MMU_ADDR
    CHECK((mem_addr() & 0x800) == 0, "C7FF lands in first 2 KiB of window");
#endif

    //----------------------------------------------------------------------
    // Test 3: C800..CFFE expansion (IOSTB) -> second 2 KiB of selected slot.
    //----------------------------------------------------------------------
    printf("\n[3] C800 expansion (IOSTB) -> second 2 KiB of selected slot 7\n");
    do_reset();
    cpu_cycle(0xC700, 1, sw);           // select slot 7
    cpu_cycle(0xC800, 1, sw);           // expansion window
    dump("C800 after C700", 0xC800);
    CHECK(dut->ROM_EN_N == 0, "C800 ROM_EN_N asserted");
    CHECK(dut->IOSTB_N == 1, "IOSTB_N deasserted when firmware ROM wins");
    expect_iofw("C800 phys", 7, 0xC800); // 0x7800
#ifdef FULL_MMU_ADDR
    CHECK((mem_addr() & 0x800) != 0, "C800 lands in second 2 KiB of window");
#endif
    cpu_cycle(0xCFFE, 1, sw);
    dump("CFFE after C700", 0xCFFE);
    CHECK(dut->ROM_EN_N == 0, "CFFE ROM_EN_N asserted");
    expect_iofw("CFFE phys", 7, 0xCFFE); // 0x7FFE

    //----------------------------------------------------------------------
    // Test 4: CFFF slot-selector release is now OBSERVABLE (ROM1-R1).
    // Select slot 7, read CFFF (slot_index -> 0 on that latch), then the next
    // expansion access maps into slot 0's window.
    //----------------------------------------------------------------------
    printf("\n[4] CFFF release observable via slot translation (MODE_SEL=1)\n");
    do_reset();
    cpu_cycle(0xC700, 1, sw);           // select slot 7
    cpu_cycle(0xC800, 1, sw);           // confirm slot 7 expansion
    expect_iofw("C800 slot7", 7, 0xC800);
    cpu_cycle(0xCFFF, 1, sw);           // release strobe (still firmware ROM)
    dump("CFFF read", 0xCFFF);
    CHECK(dut->ROM_EN_N == 0, "CFFF ROM_EN_N asserted (firmware ROM)");
    cpu_cycle(0xC800, 1, sw);           // expansion now owned by slot 0
    dump("C800 after CFFF", 0xC800);
    expect_iofw("C800 slot0 after CFFF", 0, 0xC800); // 0x0800

    //----------------------------------------------------------------------
    // Test 5: cross-slot change C700 -> C600 reselects expansion owner.
    //----------------------------------------------------------------------
    printf("\n[5] Cross-slot C700 -> C600 reselects expansion owner\n");
    do_reset();
    cpu_cycle(0xC700, 1, sw);
    cpu_cycle(0xC800, 1, sw);
    expect_iofw("C800 slot7", 7, 0xC800);
    cpu_cycle(0xC600, 1, sw);           // select slot 6 (onboard firmware)
    cpu_cycle(0xC800, 1, sw);
    dump("C800 after C600", 0xC800);
    expect_iofw("C800 slot6", 6, 0xC800); // 0x6800

    //----------------------------------------------------------------------
    // Test 6: external-card path (MODE_SEL=0, firmware off) uses same window.
    //----------------------------------------------------------------------
    printf("\n[6] External-card path (MODE_SEL=0, SW_SLOTXROM=0)\n");
    {
        Switches ext = sw; ext.mode_sel = 0; ext.slotxrom = 0;
        do_reset();
        cpu_cycle(0xC500, 1, ext);
        dump("C500 ext slot5", 0xC500);
        CHECK(dut->IOSEL_N == 0, "C500 IOSEL_N asserted (external card)");
        expect_iofw("C500 xlat", 5, 0xC500); // 0x5500 / upper5 = 5
        cpu_cycle(0xC800, 1, ext);
        dump("C800 ext after C500", 0xC800);
        expect_iofw("C800 xlat slot5", 5, 0xC800); // 0x5800
    }

    //----------------------------------------------------------------------
    // Test 7: C6XX_N decode and bus-ownership (ROM1-R3).
    // C6XX_N is a PURE ADDRESS DECODE of $C6xx IOSEL, independent of
    // rom_enabled. With onboard slot-6 firmware ENABLED, both ROM_EN_N and
    // C6XX_N assert at C600: the external slot-6 controller and onboard ROM
    // are mutually exclusive by configuration (disable SW_SLOTXROM for slot 6,
    // or do not fit an external card). Documented in docs/rom-memory-map.md
    // and coordinated with CPLD-HW-0001.
    //----------------------------------------------------------------------
    printf("\n[7] C6XX_N decode / bus-ownership coexistence\n");
    do_reset();
    cpu_cycle(0xC600, 1, sw); // onboard firmware ON
    dump("C600 fw-on", 0xC600);
    CHECK(dut->C6XX_N == 0, "C6XX_N asserts at C600 (pure decode)");
    CHECK(dut->ROM_EN_N == 0, "onboard ROM_EN_N also asserts at C600 (fw on)");
    // With firmware OFF the onboard ROM backs off; external card owns the bus.
    do_reset();
    Switches ext6 = sw; ext6.slotxrom = 0;
    cpu_cycle(0xC600, 1, ext6);
    dump("C600 fw-off", 0xC600);
    CHECK(dut->C6XX_N == 0, "C6XX_N asserts at C600 (fw off)");
    CHECK(dut->ROM_EN_N == 1, "onboard ROM backs off at C600 when fw off");
    cpu_cycle(0xC6FF, 1, ext6);
    dump("C6FF fw-off", 0xC6FF);
    CHECK(dut->C6XX_N == 0, "C6XX_N asserts across C6xx (C6FF)");
    cpu_cycle(0xC700, 1, ext6);
    dump("C700 fw-off", 0xC700);
    CHECK(dut->C6XX_N == 1, "C6XX_N deasserts outside C6xx");

    //----------------------------------------------------------------------
    // Test 8: C0XX I/O page is not a ROM window.
    //----------------------------------------------------------------------
    printf("\n[8] C0XX decode\n");
    do_reset();
    cpu_cycle(0xC080, 1, sw);
    dump("C080 read", 0xC080);
    CHECK(dut->C0XX_N == 0, "C0XX_N asserted at C080");
    CHECK(dut->ROM_EN_N == 1, "C0XX is not a ROM window");

    //----------------------------------------------------------------------
    // Test 9: system ROM D000/F800 stays raw-addressed (unchanged).
    //----------------------------------------------------------------------
    printf("\n[9] System ROM D000/F800 raw mapping\n");
    do_reset();
    cpu_cycle(0xF800, 1, sw);
    dump("F800 read", 0xF800);
    CHECK(dut->ROM_EN_N == 0, "F800 ROM_EN_N asserted (system ROM)");
    expect_sys("F800 phys", 0xF800);
    cpu_cycle(0xD000, 1, sw);
    dump("D000 read", 0xD000);
    CHECK(dut->ROM_EN_N == 0, "D000 ROM_EN_N asserted (system ROM)");
    expect_sys("D000 phys", 0xD000);

    //----------------------------------------------------------------------
    // Test 10: RAM path + soft switches (main vs aux read via SW_RAMRD).
    //----------------------------------------------------------------------
    printf("\n[10] RAM path with SW_RAMRD main/aux (addr_16 bank)\n");
    do_reset();
    cpu_cycle(0x2000, 1, sw);           // main read
    dump("2000 main read", 0x2000);
    CHECK(dut->RAM_EN_N == 0, "2000 RAM_EN_N asserted");
    CHECK(dut->ROM_EN_N == 1, "2000 not a ROM window");
#ifdef FULL_MMU_ADDR
    expect_eq("2000 main phys", mem_addr(), 0x02000);
#endif
    {
        Switches aux = sw; aux.ramrd = 1; // aux-bank read
        do_reset();
        cpu_cycle(0x2000, 1, aux);
        dump("2000 aux read", 0x2000);
        CHECK(dut->RAM_EN_N == 0, "2000 aux RAM_EN_N asserted");
#ifdef FULL_MMU_ADDR
        expect_eq("2000 aux phys (bank16)", mem_addr(), 0x12000);
#endif
    }

    //----------------------------------------------------------------------
    // Test 11: write cycle to slot ROM window must not assert ROM_EN_N.
    //----------------------------------------------------------------------
    printf("\n[11] Write to ROM window (CPU_RW=0)\n");
    do_reset();
    cpu_cycle(0xC700, 0, sw);
    dump("C700 write", 0xC700);
    CHECK(dut->ROM_EN_N == 1, "ROM not enabled on write cycle");

    printf("\n=== %s: %d checks, %d failures ===\n", build, g_checks, g_failures);
    delete dut;
    return g_failures ? 1 : 0;
}
