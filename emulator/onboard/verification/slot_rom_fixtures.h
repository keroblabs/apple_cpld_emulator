#ifndef slot_rom_fixtures_h
#define slot_rom_fixtures_h

// Distinct slot bytes in IOSEL, IOSTB and the padding between them.
#define SLOT_BYTES(n) { [0x000] = 0x10 + (n), [0x0FF] = 0x20 + (n), \
                        [0x100] = 0x30 + (n), [0x7FF] = 0x40 + (n), \
                        [0x800] = 0x50 + (n), [0xFFE] = 0x60 + (n), \
                        [0xFFF] = 0x70 + (n) }

#define SLOT1_ROM
static const unsigned char slot1_rom_bin[4096] = SLOT_BYTES(1);
#define SLOT2_ROM
static const unsigned char slot2_rom_bin[4096] = SLOT_BYTES(2);
#define SLOT4_ROM
static const unsigned char slot4_rom_bin[4096] = SLOT_BYTES(4);
#define SLOT5_ROM
static const unsigned char slot5_rom_bin[4096] = SLOT_BYTES(5);
#define SLOT6_ROM
static const unsigned char slot6_rom_bin[4096] = SLOT_BYTES(6);
#define SLOT7_ROM
static const unsigned char slot7_rom_bin[4096] = SLOT_BYTES(7);

#undef SLOT_BYTES
#endif /* slot_rom_fixtures_h */
