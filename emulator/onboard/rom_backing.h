#ifndef rom_backing_h
#define rom_backing_h

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ROM_BACKING_SIZE 0x40000u
#define ROM_SYSTEM_BASE  0x08000u
#define ROM_SYSTEM_SIZE  0x08000u
#define ROM_SLOT_SIZE    0x01000u

// Clear the backing and copy the complete Apple ROM to ROM_SYSTEM_BASE.
void rom_backing_init( const uint8_t system_rom[ ROM_SYSTEM_SIZE ] );

// Copy one raw binary into slot * ROM_SLOT_SIZE. IOSEL is at +0x000 and
// IOSTB at +0x800. A valid load replaces the whole slot. An invalid slot
// (outside 1..7), pointer or length returns -1 before modifying the backing.
int rom_backing_load_slot( int slot, const uint8_t *image, size_t image_len );

// Check the physical address against the 256 KiB backing. Out-of-range reads
// return zero and set out_of_bounds when the caller supplies it.
uint8_t rom_backing_read( uint32_t phys_addr, int *out_of_bounds );

// Trace hook for internal and slot ROM reads. ROM_EN_N is the sampled pin
// level (0 when asserted). Slot strobes can select a read with ROM_EN_N high.
typedef void (*rom_backing_trace_fn)( uint32_t cpu_addr,
                                      uint32_t phys_addr,
                                      int rom_en_n,
                                      uint8_t data,
                                      int out_of_bounds );

uint8_t rom_backing_read_traced( uint32_t cpu_addr, uint32_t phys_addr, int rom_en_n );
void rom_backing_set_trace( rom_backing_trace_fn fn );
uint32_t rom_backing_oob_count( void );
void rom_backing_oob_reset( void );

#ifdef __cplusplus
}
#endif

#endif /* rom_backing_h */
