// CPLD-ROM-0003 — Emulator ROM image backing.
//
// Provides the CPU-readable ROM path addressed by the MMU's translated
// physical address (MEM_ADDR_OUT) when ROM_EN_N is asserted. See
// docs/rom-memory-map.md for the logical layout this implements.
//
// Grounding (CPLD-ROM-0001): under MODE_SEL=1 MEM_ADDR_OUT equals the raw CPU
// address (bit 16 = 0), so this module maintains a 64 KiB logical ROM space
// indexed by physical address. Slot firmware is installed as an overlay at its
// Apple $Cn00 window; expansion ROM at $C800. Bounds are explicit: a read past
// the logical space is reported rather than silently wrapped.

#ifndef rom_backing_h
#define rom_backing_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ROM_BACKING_SIZE        0x10000u   // 64 KiB logical ROM space

// Optional trace hook: cpu/phys address, ROM_EN_N (0 = asserted) and byte.
typedef void (*rom_backing_trace_fn)( uint32_t cpu_addr,
                                      uint32_t phys_addr,
                                      int rom_en_n,
                                      uint8_t data );

// Initialise the logical ROM space: fill with idle_fill, then copy the system
// ROM image (length clamped to the logical space) at system_base.
void rom_backing_init( const uint8_t *system_rom,
                       uint32_t system_len,
                       uint32_t system_base,
                       uint8_t idle_fill );

// Install an overlay (slot entry ROM, expansion ROM, etc.) at an Apple base
// address. Returns 0 on success, -1 if the region falls outside the logical
// space (nothing is written on failure).
int rom_backing_install( uint32_t apple_base, const uint8_t *data, uint32_t len );

// Read a byte for the given physical address. out_of_bounds (optional) is set
// to 1 if phys_addr is outside the logical space; in that case the idle-fill
// value is returned instead of wrapping.
uint8_t rom_backing_read( uint32_t phys_addr, int *out_of_bounds );

// Register an optional trace callback (NULL disables tracing).
void rom_backing_set_trace( rom_backing_trace_fn fn );

// Trace-aware read helper used by the memory controller; forwards to the
// registered trace callback (if any) with the originating CPU address.
uint8_t rom_backing_read_traced( uint32_t cpu_addr, uint32_t phys_addr, int rom_en_n );

#ifdef __cplusplus
}
#endif

#endif /* rom_backing_h */
