// Reject incomplete exports and check the compiled image and physical bounds.
#define _POSIX_C_SOURCE 200809L

#include "rom_backing.h"
#include "apple2rom.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>

static int checks;
static int failures;
static int traced_oob;
static uint32_t traced_address;

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
          "export contains exactly 131072 bytes");
    fclose(file);

    check(memcmp(actual, expected, sizeof(actual)) == 0,
          "export matches the Apple ROM and zero-filled slot and reserved areas");
}

static void capture_oob(uint32_t cpu_addr, uint32_t phys_addr,
                        int rom_en_n, uint8_t data, int oob)
{
    check(cpu_addr == 0xFFFF && rom_en_n == 0 && data == 0,
          "OOB trace reports the CPU request and returned byte");
    traced_address = phys_addr;
    traced_oob = oob;
}

static void check_partial_export(const char *path, const uint8_t *expected)
{
    struct rlimit original;
    const int have_limit = getrlimit(RLIMIT_FSIZE, &original) == 0;
    check(have_limit, "read the file size limit");
    if (!have_limit) {
        return;
    }

    struct rlimit limited = original;
    limited.rlim_cur = 1024;
    void (*original_handler)(int) = signal(SIGXFSZ, SIG_IGN);
    check(original_handler != SIG_ERR, "handle the imposed file size limit");
    if (original_handler == SIG_ERR) {
        return;
    }

    const int limit_applied = setrlimit(RLIMIT_FSIZE, &limited) == 0;
    check(limit_applied, "impose a file size limit below the image size");
    if (limit_applied) {
        check(rom_backing_write_image(path) == -1, "report a write cut short by the file size limit");
        check(setrlimit(RLIMIT_FSIZE, &original) == 0, "restore the file size limit");
    }
    check(signal(SIGXFSZ, original_handler) != SIG_ERR, "restore the file size signal handler");
    check_export(path, expected);
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Expected a temporary image path and a directory to test export failure\n");
        return 1;
    }

    static uint8_t expected[0x20000];
    memcpy(expected + 0x08000, apple_rom_bin, sizeof(apple_rom_bin));
    rom_backing_init();
    check(ROM_BACKING_SIZE == sizeof(expected), "128 KiB backing size");
    check_backing(expected);

    check(rom_backing_write_image(NULL) == -1, "reject a null output path");
    check(rom_backing_write_image("") == -1, "reject an empty output path");
    check(rom_backing_write_image(argv[2]) == -1, "reject a directory as an output file");
    check(rom_backing_write_image(argv[1]) == 0, "export the compiled ROM image");
    check_export(argv[1], expected);
    check(rom_backing_write_image(argv[1]) == 0, "replace an image with a complete export");
    check_export(argv[1], expected);
    check_partial_export(argv[1], expected);

    int oob = 1;
    check(rom_backing_read(0x0FFFF, &oob) == expected[0x0FFFF] && !oob,
          "last byte of the lower bank is from Apple ROM");
    check(rom_backing_read(0x10000, &oob) == 0 && !oob, "first reserved byte is in range and zero");
    check(rom_backing_read(0x1FFFF, &oob) == 0 && !oob, "last reserved byte is in range and zero");
    rom_backing_set_trace(capture_oob);
    check(rom_backing_read_traced(0xFFFF, 0x20000, 0) == 0, "return zero at the first OOB address");
    check(traced_oob && traced_address == 0x20000 && rom_backing_oob_count() == 1,
          "trace and count the first OOB address");
    check(rom_backing_read_traced(0xFFFF, 0x2FFFF, 0) == 0, "return zero without wrapping into Apple ROM");
    check(traced_oob && traced_address == 0x2FFFF && rom_backing_oob_count() == 2,
          "trace and count the higher OOB address");
    rom_backing_set_trace(NULL);

    rom_backing_init();
    check_backing(expected);
    check(rom_backing_oob_count() == 0, "initialisation clears OOB count");

    printf("rom_backing: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
