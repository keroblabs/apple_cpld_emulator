// CPLD-ROM-0003 — Unit tests for the emulator ROM image backing.
//
// Exercises rom_backing in isolation (no full emulator build):
//   * system ROM load + system-ROM read
//   * per-slot 4 KiB window geometry (IOSEL page / IOSTB expansion)
//   * validated region install + overlap/geometry rejection
//   * packed *.romimg loader (valid + malformed)
//   * explicit bounds: reads past the logical space are reported, not wrapped
//   * OOB visibility on the ACTUAL traced path (rom_backing_read_traced)
//   * integration-style check against the RTL-verified MEM_ADDR_OUT mapping
//     ({ slot_index, ADDR[11:0] }), i.e. the physical addresses tb_mmu_addr.cpp
//     proved the MMU emits.

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "rom_backing.h"

static int g_fail = 0;
static int g_checks = 0;

static void check(int cond, const char *msg)
{
    g_checks++;
    if (!cond) { g_fail++; printf("  FAIL: %s\n", msg); }
}

static void check_eq(const char *name, uint32_t got, uint32_t exp)
{
    g_checks++;
    if (got != exp) {
        g_fail++;
        printf("  FAIL: %-26s got=0x%05X exp=0x%05X\n", name, got, exp);
    }
}

// Trace capture (new 5-arg signature incl. out_of_bounds).
static uint32_t t_cpu, t_phys; static int t_rom_en_n, t_oob; static uint8_t t_data;
static int t_count = 0;
static void trace_cb(uint32_t cpu, uint32_t phys, int rom_en_n, uint8_t data, int oob)
{
    t_cpu = cpu; t_phys = phys; t_rom_en_n = rom_en_n; t_data = data; t_oob = oob;
    t_count++;
}

// Mirror of the RTL IO-firmware mapping verified by tb_mmu_addr.cpp.
static uint32_t rtl_iofw_phys(int slot, uint16_t apple_addr)
{
    return ((uint32_t)slot << 12) | (apple_addr & 0x0FFF);
}

int main(void)
{
    printf("=== rom_backing unit tests ===\n");

    // Synthetic system ROM: 32 KiB, byte = 0x5n pattern keyed on CPU address.
    static uint8_t sys_rom[32 * 1024];
    for (uint32_t i = 0; i < sizeof(sys_rom); i++)
        sys_rom[i] = (uint8_t)(0x50 | ((0x8000 + i) & 0x0F));

    rom_backing_init(sys_rom, sizeof(sys_rom), 0x8000, 0xA0);

    // [1] System ROM visible at its CPU addresses; idle fill below base.
    printf("[1] system ROM load\n");
    check_eq("sys F800", rom_backing_read(0xF800, NULL), (uint8_t)(0x50 | (0xF800 & 0x0F)));
    check_eq("sys D000", rom_backing_read(0xD000, NULL), (uint8_t)(0x50 | (0xD000 & 0x0F)));
    check_eq("idle 0x2000", rom_backing_read(0x2000, NULL), 0xA0);

    // [2] Window geometry helpers.
    printf("[2] per-slot 4 KiB window geometry\n");
    check_eq("slot7 entry phys", rom_backing_slot_entry_phys(7), 0x7700);
    check_eq("slot1 entry phys", rom_backing_slot_entry_phys(1), 0x1100);
    check_eq("slot7 exp phys",   rom_backing_slot_expansion_phys(7), 0x7800);
    check_eq("slot3 exp phys",   rom_backing_slot_expansion_phys(3), 0x3800);

    // [3] Validated region install: slot-7 entry (IOSEL) + expansion (IOSTB).
    printf("[3] validated region install (slot 7)\n");
    uint8_t entry7[256];
    for (int i = 0; i < 256; i++) entry7[i] = (uint8_t)(0x70 | (i & 0x0F));
    check(rom_backing_install_region(ROM_REGION_SLOT_ENTRY, 7, entry7, 256) == 0,
          "install slot7 entry ok");
    check_eq("entry @7700", rom_backing_read(0x7700, NULL), 0x70);
    check_eq("entry @77FF", rom_backing_read(0x77FF, NULL), (uint8_t)(0x70 | 0x0F));

    uint8_t exp7[0x800];
    for (int i = 0; i < 0x800; i++) exp7[i] = (uint8_t)(0x80 | (i & 0x0F));
    check(rom_backing_install_region(ROM_REGION_SLOT_EXP, 7, exp7, 0x800) == 0,
          "install slot7 expansion ok");
    check_eq("exp @7800", rom_backing_read(0x7800, NULL), 0x80);
    check_eq("exp @7FFF", rom_backing_read(0x7FFF, NULL), (uint8_t)(0x80 | (0x7FF & 0x0F)));

    // [4] Rejections: oversize, bad slot, overlap, system overlap.
    printf("[4] install validation / rejection\n");
    check(rom_backing_install_region(ROM_REGION_SLOT_ENTRY, 7, entry7, 0x200) == -1,
          "reject oversize entry (>256)");
    check(rom_backing_install_region(ROM_REGION_SLOT_EXP, 7, exp7, 0x1000) == -1,
          "reject oversize expansion (>2 KiB)");
    check(rom_backing_install_region(ROM_REGION_SLOT_ENTRY, 0, entry7, 16) == -1,
          "reject slot 0");
    check(rom_backing_install_region(ROM_REGION_SLOT_ENTRY, 8, entry7, 16) == -1,
          "reject slot 8");
    check(rom_backing_install_region(ROM_REGION_SLOT_ENTRY, 7, entry7, 16) == -1,
          "reject overlap with existing slot7 entry");
    check(rom_backing_install(0x8000, sys_rom, 16) == -1,
          "reject overlap with system ROM region");
    check(rom_backing_install(0xFF80, entry7, 256) == -1, "reject straddling top");
    check(rom_backing_install(0x10000, entry7, 1) == -1, "reject base out of range");
    check(rom_backing_install(0x1000, 0, 16) == -1, "reject null data");

    // [5] No aliasing between IO firmware region and system ROM.
    printf("[5] no aliasing with system ROM\n");
    check_eq("C6FF-region untouched", rom_backing_read(0x6FFF, NULL), 0xA0); // slot6 window empty
    check_eq("D000 sys intact", rom_backing_read(0xD000, NULL), (uint8_t)(0x50 | (0xD000 & 0x0F)));

    // [6] Bounds + OOB visibility on the ACTUAL traced path.
    printf("[6] bounds and OOB visibility (traced path)\n");
    rom_backing_set_trace(trace_cb);
    rom_backing_oob_reset();
    t_count = 0;
    uint8_t d = rom_backing_read_traced(0xC700, 0x7700, 0 /*asserted*/);
    check(t_count == 1, "trace called once");
    check_eq("trace data", t_data, 0x70);
    check_eq("trace cpu", t_cpu, 0xC700);
    check_eq("trace phys", t_phys, 0x7700);
    check(t_rom_en_n == 0, "trace rom_en_n asserted");
    check(t_oob == 0, "in-range read not flagged OOB");
    check_eq("traced read value", d, 0x70);
    check_eq("oob count still 0", rom_backing_oob_count(), 0);
    // Out-of-range through the real traced path (phys > 0xFFFF).
    uint8_t b = rom_backing_read_traced(0xFFFF, 0x10000, 0);
    check(t_oob == 1, "OOB read flagged on traced path");
    check_eq("OOB returns idle", b, 0xA0);
    check_eq("oob count incremented", rom_backing_oob_count(), 1);
    rom_backing_set_trace(0);

    // [7] Packed *.romimg loader: build a valid 2-entry image and load it.
    printf("[7] romimg loader (valid)\n");
    rom_backing_init(sys_rom, sizeof(sys_rom), 0x8000, 0xA0); // fresh space
    {
        uint8_t img[8 + 2 * 16 + 256 + 0x800];
        memset(img, 0, sizeof(img));
        img[0]='C'; img[1]='P'; img[2]='R'; img[3]='M';
        img[4]=1; img[5]=0;          // version 1
        img[6]=2; img[7]=0;          // 2 entries
        uint32_t tbl = 8;
        uint32_t pay_entry = tbl + 2 * 16;
        uint32_t pay_exp   = pay_entry + 256;
        // entry 0: slot7 ENTRY
        img[tbl+0]=ROM_REGION_SLOT_ENTRY; img[tbl+1]=7;
        img[tbl+2]=0x00; img[tbl+3]=0xC7;              // apple base C700 (LE)
        img[tbl+4]=0x00; img[tbl+5]=0x01;              // length 256 (LE)
        img[tbl+8]=(uint8_t)pay_entry; img[tbl+9]=(uint8_t)(pay_entry>>8);
        // entry 1: slot7 EXPANSION
        img[tbl+16+0]=ROM_REGION_SLOT_EXP; img[tbl+16+1]=7;
        img[tbl+16+2]=0x00; img[tbl+16+3]=0xC8;        // apple base C800
        img[tbl+16+4]=0x00; img[tbl+16+5]=0x08;        // length 0x800
        img[tbl+16+8]=(uint8_t)pay_exp; img[tbl+16+9]=(uint8_t)(pay_exp>>8);
        for (int i=0;i<256;i++)   img[pay_entry+i]=(uint8_t)(0xC0 | (i & 0x0F));
        for (int i=0;i<0x800;i++) img[pay_exp+i]  =(uint8_t)(0xD0 | (i & 0x0F));

        check(rom_backing_load_image(img, sizeof(img)) == 0, "load valid romimg ok");
        check_eq("img entry @7700", rom_backing_read(0x7700, NULL), 0xC0);
        check_eq("img exp   @7800", rom_backing_read(0x7800, NULL), 0xD0);

        // Malformed images are rejected.
        uint8_t bad = img[0]; img[0]='X';
        check(rom_backing_load_image(img, sizeof(img)) == -2, "reject bad magic");
        img[0]=bad;
        check(rom_backing_load_image(img, 4) == -1, "reject truncated header");
    }

    // [8] Integration-style: read the EXACT physical addresses the MMU emits
    // (verified by tb_mmu_addr.cpp) and confirm the CPU-visible bytes.
    printf("[8] integration vs RTL-verified MEM_ADDR_OUT mapping\n");
    rom_backing_init(sys_rom, sizeof(sys_rom), 0x8000, 0xA0);
    rom_backing_install_region(ROM_REGION_SLOT_ENTRY, 7, entry7, 256);
    rom_backing_install_region(ROM_REGION_SLOT_EXP, 7, exp7, 0x800);
    // CPU reads $C700 -> MMU emits {7, 0x700} = 0x7700.
    check_eq("CPU C700 -> entry", rom_backing_read(rtl_iofw_phys(7, 0xC700), NULL), 0x70);
    // CPU reads $C7FF -> {7, 0x7FF} = 0x77FF (last entry byte, still first 2 KiB).
    check_eq("CPU C7FF -> entry end",
             rom_backing_read(rtl_iofw_phys(7, 0xC7FF), NULL), (uint8_t)(0x70 | 0x0F));
    // CPU reads $C800 -> {7, 0x800} = 0x7800 (expansion).
    check_eq("CPU C800 -> expansion", rom_backing_read(rtl_iofw_phys(7, 0xC800), NULL), 0x80);
    // After CFFF release the selector is slot 0; CPU $C800 -> {0, 0x800} = 0x0800 (empty).
    check_eq("CPU C800 slot0 after CFFF",
             rom_backing_read(rtl_iofw_phys(0, 0xC800), NULL), 0xA0);
    // System ROM path unchanged: CPU $F800 -> raw 0xF800.
    check_eq("CPU F800 -> system", rom_backing_read(0xF800, NULL),
             (uint8_t)(0x50 | (0xF800 & 0x0F)));

    printf("\n=== rom_backing: %d checks, %d failures ===\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
