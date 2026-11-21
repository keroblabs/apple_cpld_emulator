// CPLD-ROM-0003/0004. Check the default compiled image and its exact export.
#include "rom_backing.h"
#include "apple2rom.h"
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

static void check_backing(const uint8_t *expected)
{
    ++checks;
    for (uint32_t address = 0; address < ROM_BACKING_SIZE; ++address) {
        const uint8_t byte = rom_backing_read(address, NULL);
        if (byte != expected[address]) {
            ++failures;
            printf("FAIL at $%05X: got $%02X, expected $%02X\n", address, byte, expected[address]);
            return;
        }
    }
}

static void check_export(const char *path, const uint8_t *expected)
{
    static uint8_t actual[ROM_BACKING_SIZE];
    FILE *file = fopen(path, "rb");
    check(file != NULL, "open the exported image");
    if (!file) {
        return;
    }

    const size_t count = fread(actual, 1, sizeof(actual), file);
    check(count == sizeof(actual) && fgetc(file) == EOF && !ferror(file),
          "export contains exactly 262144 bytes");
    fclose(file);

    check(memcmp(actual, expected, sizeof(actual)) == 0,
          "export matches the Apple ROM and zero-filled slot and reserved areas");
}

static void capture_oob(uint32_t cpu_addr, uint32_t phys_addr,
                        int rom_en_n, uint8_t data, int oob)
{
    check(cpu_addr == 0xFFFF && phys_addr == 0x40000 && rom_en_n == 0 && data == 0,
          "OOB trace reports the original request and returned byte");
    traced_oob = oob;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Expected a temporary image path and a directory to test export failure\n");
        return 1;
    }

    static uint8_t expected[0x40000];
    memcpy(expected + 0x08000, apple_rom_bin, sizeof(apple_rom_bin));
    rom_backing_init();
    check(ROM_BACKING_SIZE == sizeof(expected), "256 KiB backing size");
    check_backing(expected);

    check(rom_backing_write_image(NULL) == -1, "reject a null output path");
    check(rom_backing_write_image("") == -1, "reject an empty output path");
    check(rom_backing_write_image(argv[2]) == -1, "reject a directory as an output file");
    check(rom_backing_write_image(argv[1]) == 0, "export the compiled ROM image");
    check_export(argv[1], expected);
    check(rom_backing_write_image(argv[1]) == 0, "replace an image with a complete export");
    check_export(argv[1], expected);

    int oob = 1;
    check(rom_backing_read(0x3FFFF, &oob) == 0 && !oob, "last byte is in range");
    rom_backing_set_trace(capture_oob);
    check(rom_backing_read_traced(0xFFFF, 0x40000, 0) == 0, "return zero above backing");
    check(traced_oob && rom_backing_oob_count() == 1, "trace and count the OOB read");
    rom_backing_set_trace(NULL);

    rom_backing_init();
    check_backing(expected);
    check(rom_backing_oob_count() == 0, "initialisation clears OOB count");

    printf("rom_backing: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
