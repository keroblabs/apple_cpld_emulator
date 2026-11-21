#ifndef rom_backing_h
#define rom_backing_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// SST39VF010 backing. The MMU selects the lower 64 KiB for ROM reads.
#define ROM_BACKING_SIZE 0x20000u
#define ROM_SYSTEM_BASE  0x08000u
#define ROM_SYSTEM_SIZE  0x08000u
#define ROM_SLOT_SIZE    0x01000u

// Populate the shared backing from the compiled Apple and slot ROM arrays.
void rom_backing_init( void );

// Export the populated backing in physical address order. Returns -1 on error.
int rom_backing_write_image( const char *path );

// Check the physical address against the 128 KiB backing. Out-of-range reads
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
