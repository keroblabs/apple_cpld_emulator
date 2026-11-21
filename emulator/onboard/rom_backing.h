// CPLD-ROM-0003 — Emulator ROM image backing.
//
// Provides the CPU-readable ROM path addressed by the MMU's translated
// physical address (MEM_ADDR_OUT) when ROM_EN_N is asserted. See
// docs/rom-memory-map.md for the logical layout this implements.
//
// Physical (logical ROM) layout — enforced per CPLD-ROM-0001/0002 rework:
//
//   0x0000 .. 0x7FFF  IO firmware ROM: 8 per-slot 4 KiB windows.
//                     Slot N window base = N << 12.
//                       first  2 KiB (window+0x000..0x7FF): IOSEL page
//                         Apple $Cn00-$CnFF -> window + (N<<8) .. +0xFF
//                       second 2 KiB (window+0x800..0xFFF): IOSTB expansion
//                         Apple $C800-$CFFF -> window + 0x800 .. +0xFFF
//   0x8000 .. 0xFFFF  System ROM image (Apple $8000-$FFFF). The CPU-reachable
//                     system ROM window is $D000-$FFFF (raw-addressed by the
//                     MMU); $8000-$CFFF of the image is reserved space.
//
// The MMU emits MEM_ADDR_OUT = { slot_index, ADDR[11:0] } for IO firmware ROM
// and the raw CPU address for system ROM, so this module is indexed directly
// by the physical address with explicit bounds (no silent & 0xFFFF wrap).

#ifndef rom_backing_h
#define rom_backing_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ROM_BACKING_SIZE        0x10000u   // 64 KiB logical ROM space

// IO firmware ROM region and per-slot window geometry.
#define ROM_IO_FW_BASE          0x0000u
#define ROM_IO_FW_END           0x8000u    // exclusive; 8 slots * 4 KiB
#define ROM_SLOT_WINDOW         0x1000u    // 4 KiB per slot
#define ROM_SLOT_ENTRY_SIZE     0x0100u    // IOSEL page (256 bytes)
#define ROM_SLOT_EXP_OFFSET     0x0800u    // second 2 KiB
#define ROM_SLOT_EXP_SIZE       0x0800u    // IOSTB expansion (2 KiB)
#define ROM_SYSTEM_BASE         0x8000u    // system ROM image base

// Region kinds for validated installation and for the packed image format.
enum {
    ROM_REGION_SYSTEM     = 0,
    ROM_REGION_SLOT_ENTRY = 1,  // IOSEL page
    ROM_REGION_SLOT_EXP   = 2   // IOSTB expansion
};

// Trace hook: cpu/phys address, ROM_EN_N (0 = asserted), byte and whether the
// physical address was out of bounds (1) or valid (0).
typedef void (*rom_backing_trace_fn)( uint32_t cpu_addr,
                                      uint32_t phys_addr,
                                      int rom_en_n,
                                      uint8_t data,
                                      int out_of_bounds );

// Physical window helpers (see layout above).
uint32_t rom_backing_slot_entry_phys( int slot );      // IOSEL page base
uint32_t rom_backing_slot_expansion_phys( int slot );  // IOSTB expansion base

// Initialise: fill with idle_fill, copy the system ROM image (length clamped)
// at system_base, and reset region-occupancy tracking.
void rom_backing_init( const uint8_t *system_rom,
                       uint32_t system_len,
                       uint32_t system_base,
                       uint8_t idle_fill );

// Low-level install at an absolute physical address. Returns 0 on success,
// -1 on a bounds error or overlap with an already-installed region.
int rom_backing_install( uint32_t phys_base, const uint8_t *data, uint32_t len );

// Baseline fill at an absolute physical address: bounds-checked, but does NOT
// mark occupancy and is NOT overlap-checked. Used to seed default per-slot
// firmware that a later packaged image may legitimately override. Returns 0
// on success, -1 on a bounds error.
int rom_backing_fill_baseline( uint32_t phys_base, const uint8_t *data, uint32_t len );

// Validated, window-aware install. kind is ROM_REGION_*; slot is 1..7 for the
// slot regions (ignored for SYSTEM). Enforces the per-slot 4 KiB geometry and
// rejects overlaps. Returns 0 on success, negative on error.
int rom_backing_install_region( int kind, int slot, const uint8_t *data, uint32_t len );

// Parse and install a packed *.romimg image (see docs/rom-memory-map.md §4).
// Returns 0 on success, negative on a malformed/overflowing/overlapping image.
int rom_backing_load_image( const uint8_t *img, uint32_t img_len );

// Read a byte for the given physical address. out_of_bounds (optional) is set
// to 1 if phys_addr is outside the logical space; the idle-fill value is then
// returned instead of wrapping.
uint8_t rom_backing_read( uint32_t phys_addr, int *out_of_bounds );

// Trace-aware read used by the memory controller. Forwards to the registered
// trace callback (if any) with the originating CPU address and the OOB flag,
// and counts out-of-bounds reads (see rom_backing_oob_count).
uint8_t rom_backing_read_traced( uint32_t cpu_addr, uint32_t phys_addr, int rom_en_n );

// Register an optional trace callback (NULL disables tracing).
void rom_backing_set_trace( rom_backing_trace_fn fn );

// Number of out-of-bounds ROM reads observed on the traced path, and a reset.
uint32_t rom_backing_oob_count( void );
void     rom_backing_oob_reset( void );

#ifdef __cplusplus
}
#endif

#endif /* rom_backing_h */
