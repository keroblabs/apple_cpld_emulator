#ifndef slot_roms_h
#define slot_roms_h

#include "rom_backing.h"

// Add the slot's SLOTn_ROM definition and 4096-byte ROM array here, or
// include a header that provides both, before its default block below.
#ifndef SLOT1_ROM
static const uint8_t slot1_rom_bin[ ROM_SLOT_SIZE ] = { 0 };
#endif

#ifndef SLOT2_ROM
static const uint8_t slot2_rom_bin[ ROM_SLOT_SIZE ] = { 0 };
#endif

#ifndef SLOT4_ROM
static const uint8_t slot4_rom_bin[ ROM_SLOT_SIZE ] = { 0 };
#endif

#ifndef SLOT5_ROM
static const uint8_t slot5_rom_bin[ ROM_SLOT_SIZE ] = { 0 };
#endif

#ifndef SLOT6_ROM
static const uint8_t slot6_rom_bin[ ROM_SLOT_SIZE ] = { 0 };
#endif

#ifndef SLOT7_ROM
static const uint8_t slot7_rom_bin[ ROM_SLOT_SIZE ] = { 0 };
#endif

#endif /* slot_roms_h */
