#ifndef memory_controller_h
#define memory_controller_h

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////////

#include "board.h"

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

void memory_controller_init( void );

// Initialise from a complete raw ROM file. Returns -1 when loading fails.
int memory_controller_init_raw( const char *path );

// After memory_controller_init and before CPU execution, copy a 4 KiB binary
// into slot 1..7. IOSEL starts at offset 0, IOSTB at 0x800. Returns 0 on
// success (replacing the slot) or -1 on invalid input or in raw-file mode.
int memory_controller_load_slot_rom( int slot, const uint8_t *image, size_t image_len );

uint8_t memory_controller_access( void * p_cpld, uint32_t cpu_address, bool_t is_read, uint8_t data_in );

void memory_controller_tick( void * p_cpld, uint64_t cycles );

//////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* memory_controller_h */
