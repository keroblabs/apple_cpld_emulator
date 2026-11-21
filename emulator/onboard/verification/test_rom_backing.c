// CPLD-ROM-0003. Verify raw ROM copies, slot isolation and backing bounds.
#include "rom_backing.h"
#include <stdio.h>
#include <string.h>

static int checks;
static int failures;
static int traced_oob;

static void check(int condition, const char *message)
{
    ++checks;
    if (!condition) {
        ++failures;
        printf("FAIL: %s\n", message);
    }
}

static void check_region(uint32_t base, const uint8_t *expected, size_t len)
{
    ++checks;
    for (size_t i = 0; i < len; ++i) {
        const uint8_t byte = rom_backing_read(base + i, NULL);
        if (byte != expected[i]) {
            ++failures;
            printf("FAIL at $%05zX: got $%02X, expected $%02X\n", base + i, byte, expected[i]);
            return;
        }
    }
}

static void capture_oob(uint32_t cpu_addr, uint32_t phys_addr,
                        int rom_en_n, uint8_t data, int oob)
{
    check(cpu_addr == 0xFFFF && phys_addr == 0x40000 && rom_en_n == 0 && data == 0,
          "OOB trace reports the original request and returned byte");
    traced_oob = oob;
}

int main(void)
{
    static uint8_t system_rom[ROM_SYSTEM_SIZE];
    static uint8_t slots[7][4096];

    for (size_t i = 0; i < sizeof(system_rom); ++i) {
        system_rom[i] = (uint8_t)(i ^ (i >> 8));
    }
    for (int slot = 1; slot <= 7; ++slot) {
        for (size_t i = 0; i < sizeof(slots[0]); ++i) {
            slots[slot - 1][i] = (uint8_t)(slot * 17 + (i ^ (i >> 8)));
        }
    }
    // These bytes are payload too.
    memcpy(slots[6], "CPRM", 4);

    rom_backing_init(system_rom);
    check(ROM_BACKING_SIZE == 256 * 1024, "256 KiB backing size");
    check_region(0x08000, system_rom, sizeof(system_rom));
    check(rom_backing_read(0x07000, NULL) == 0, "slot entry starts empty");
    check(rom_backing_read(0x07800, NULL) == 0, "slot expansion starts empty");

    for (int slot = 1; slot <= 7; ++slot) {
        check(rom_backing_load_slot(slot, slots[slot - 1], sizeof(slots[0])) == 0,
              "accept a complete raw slot binary");
    }
    for (int slot = 1; slot <= 7; ++slot) {
        check_region(slot * 0x1000u, slots[slot - 1], sizeof(slots[0]));
    }
    check_region(0x08000, system_rom, sizeof(system_rom));
    check(rom_backing_read(0x00FFF, NULL) == 0, "released slot remains empty");

    check(rom_backing_load_slot(-1, slots[0], 4096) == -1, "reject negative slot");
    check(rom_backing_load_slot(0, slots[0], 4096) == -1, "reject reserved slot 0");
    check(rom_backing_load_slot(8, slots[0], 4096) == -1, "reject slot 8");
    check(rom_backing_load_slot(7, NULL, 4096) == -1, "reject null data");
    check(rom_backing_load_slot(7, slots[0], 4095) == -1, "reject short binary");
    check(rom_backing_load_slot(7, slots[0], 4097) == -1, "reject long binary");
    check(rom_backing_load_slot(7, slots[0], SIZE_MAX) == -1, "reject overflowing length");
    check_region(0x07000, slots[6], sizeof(slots[0]));

    check(rom_backing_load_slot(7, slots[0], sizeof(slots[0])) == 0,
          "replace an installed slot");
    check_region(0x07000, slots[0], sizeof(slots[0]));
    check_region(0x06000, slots[5], sizeof(slots[0]));
    check_region(0x08000, system_rom, sizeof(system_rom));

    int oob = 1;
    check(rom_backing_read(0x10000, &oob) == 0 && !oob, "space above 64 KiB is valid");
    check(rom_backing_read(0x3FFFF, &oob) == 0 && !oob, "last byte is in range");
    rom_backing_set_trace(capture_oob);
    check(rom_backing_read_traced(0xFFFF, 0x40000, 0) == 0, "return zero above backing");
    check(traced_oob && rom_backing_oob_count() == 1, "trace and count the OOB read");
    rom_backing_set_trace(NULL);

    rom_backing_init(system_rom);
    check(rom_backing_read(0x07000, NULL) == 0, "initialisation clears previous slot loads");
    check(rom_backing_oob_count() == 0, "initialisation clears OOB count");

    printf("rom_backing: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
