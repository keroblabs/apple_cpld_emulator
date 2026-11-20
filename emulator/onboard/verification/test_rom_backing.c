// CPLD-ROM-0003 — Unit tests for the emulator ROM image backing.
//
// Exercises the rom_backing module in isolation (no full emulator build):
//   * system ROM load + system-ROM read
//   * synthetic slot-7 entry ROM overlay visible at C700-C7FF
//   * synthetic slot-7 expansion overlay visible at C800-CFFE
//   * no aliasing between slot-7 overlay and system ROM
//   * explicit bounds: reads past the logical space are reported, not wrapped
//   * install bounds rejection (straddling / out-of-range regions)

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
        printf("  FAIL: %-24s got=0x%02X exp=0x%02X\n", name, got, exp);
    }
}

// Trace capture for the last traced read.
static uint32_t t_cpu, t_phys; static int t_rom_en_n; static uint8_t t_data;
static int t_count = 0;
static void trace_cb(uint32_t cpu, uint32_t phys, int rom_en_n, uint8_t data)
{
    t_cpu = cpu; t_phys = phys; t_rom_en_n = rom_en_n; t_data = data; t_count++;
}

int main(void)
{
    printf("=== rom_backing unit tests ===\n");

    // Build a recognisable synthetic system ROM: 32 KiB where byte value is the
    // low 8 bits of the CPU address (so system[$C000-$8000] area is checkable).
    static uint8_t sys_rom[32 * 1024];
    for (uint32_t i = 0; i < sizeof(sys_rom); i++) {
        // CPU address = 0x8000 + i ; store a system marker 0x5n pattern.
        sys_rom[i] = (uint8_t)(0x50 | ((0x8000 + i) & 0x0F));
    }

    rom_backing_init(sys_rom, sizeof(sys_rom), 0x8000, 0xA0);

    // [1] System ROM visible at its CPU addresses.
    printf("[1] system ROM load\n");
    check_eq("sys F800", rom_backing_read(0xF800, NULL),
             (uint8_t)(0x50 | (0xF800 & 0x0F)));
    check_eq("sys D000", rom_backing_read(0xD000, NULL),
             (uint8_t)(0x50 | (0xD000 & 0x0F)));
    // Idle fill below the system base.
    check_eq("idle 0x2000", rom_backing_read(0x2000, NULL), 0xA0);

    // [2] Slot-7 entry ROM overlay at C700-C7FF (distinguishable pattern 0x7n).
    printf("[2] slot-7 entry overlay C700-C7FF\n");
    uint8_t slot7[256];
    for (int i = 0; i < 256; i++) slot7[i] = (uint8_t)(0x70 | (i & 0x0F));
    check(rom_backing_install(0xC700, slot7, sizeof(slot7)) == 0, "install C700 ok");
    check_eq("C700", rom_backing_read(0xC700, NULL), 0x70);
    check_eq("C7FF", rom_backing_read(0xC7FF, NULL), (uint8_t)(0x70 | 0x0F));
    // ProDOS signature bytes region C7FC-C7FF writable within the overlay.
    check_eq("C7FC", rom_backing_read(0xC7FC, NULL), (uint8_t)(0x70 | 0x0C));

    // [3] Slot-7 expansion overlay at C800-CFFF (2 KiB, pattern 0x8n).
    printf("[3] slot-7 expansion overlay C800-CFFF\n");
    uint8_t exp[0x800];
    for (int i = 0; i < 0x800; i++) exp[i] = (uint8_t)(0x80 | (i & 0x0F));
    check(rom_backing_install(0xC800, exp, sizeof(exp)) == 0, "install C800 ok");
    check_eq("C800", rom_backing_read(0xC800, NULL), 0x80);
    check_eq("CFFE", rom_backing_read(0xCFFE, NULL), (uint8_t)(0x80 | (0x7FE & 0x0F)));

    // [4] No aliasing: overlays replaced system bytes only in their windows;
    // neighbouring system ROM bytes are intact.
    printf("[4] no aliasing with system ROM\n");
    check_eq("C6FF sys intact", rom_backing_read(0xC6FF, NULL),
             (uint8_t)(0x50 | (0xC6FF & 0x0F)));
    // CFFF is inside the installed expansion overlay window (C800..CFFF).
    check_eq("CFFF overlay", rom_backing_read(0xCFFF, NULL),
             (uint8_t)(0x80 | (0x7FF & 0x0F)));
    check_eq("D000 sys intact", rom_backing_read(0xD000, NULL),
             (uint8_t)(0x50 | (0xD000 & 0x0F)));

    // [5] Explicit bounds: out-of-range read reports and returns idle fill.
    printf("[5] bounds checking\n");
    int oob = 0;
    uint8_t b = rom_backing_read(0x10000, &oob);
    check(oob == 1, "read 0x10000 flagged out of bounds");
    check_eq("oob returns idle", b, 0xA0);

    // Install rejection: region straddling the top of the space.
    check(rom_backing_install(0xFF80, slot7, 256) == -1, "reject straddling install");
    check(rom_backing_install(0x10000, slot7, 1) == -1, "reject base out of range");
    check(rom_backing_install(0xC700, 0, 16) == -1, "reject null data");

    // [6] Trace hook fires with correct fields.
    printf("[6] trace hook\n");
    rom_backing_set_trace(trace_cb);
    t_count = 0;
    uint8_t d = rom_backing_read_traced(0xC700, 0xC700, 0 /*asserted*/);
    check(t_count == 1, "trace called once");
    check_eq("trace data", t_data, 0x70);
    check_eq("trace cpu", t_cpu, 0xC700);
    check_eq("trace phys", t_phys, 0xC700);
    check(t_rom_en_n == 0, "trace rom_en_n asserted");
    check_eq("traced read value", d, 0x70);
    rom_backing_set_trace(0);

    printf("\n=== rom_backing: %d checks, %d failures ===\n", g_checks, g_fail);
    return g_fail ? 1 : 0;
}
