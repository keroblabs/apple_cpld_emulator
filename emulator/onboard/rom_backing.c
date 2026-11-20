// CPLD-ROM-0003 — Emulator ROM image backing implementation.
// See rom_backing.h and docs/rom-memory-map.md.

#include "rom_backing.h"
#include <string.h>

static uint8_t             rom_space[ ROM_BACKING_SIZE ];
static uint8_t             rom_idle_fill = 0xA0;
static rom_backing_trace_fn rom_trace    = 0;

void rom_backing_init( const uint8_t *system_rom,
                       uint32_t system_len,
                       uint32_t system_base,
                       uint8_t idle_fill )
{
    rom_idle_fill = idle_fill;
    memset( rom_space, idle_fill, ROM_BACKING_SIZE );

    if( system_rom && system_base < ROM_BACKING_SIZE )
    {
        uint32_t space = ROM_BACKING_SIZE - system_base;
        uint32_t len   = ( system_len < space ) ? system_len : space;
        memcpy( &rom_space[ system_base ], system_rom, len );
    }
}

int rom_backing_install( uint32_t apple_base, const uint8_t *data, uint32_t len )
{
    if( data == 0 )
    {
        return -1;
    }
    // Reject regions that fall outside or straddle the end of the logical space.
    if( apple_base >= ROM_BACKING_SIZE ||
        len > ROM_BACKING_SIZE ||
        apple_base + len > ROM_BACKING_SIZE )
    {
        return -1;
    }

    memcpy( &rom_space[ apple_base ], data, len );
    return 0;
}

uint8_t rom_backing_read( uint32_t phys_addr, int *out_of_bounds )
{
    if( phys_addr >= ROM_BACKING_SIZE )
    {
        if( out_of_bounds )
        {
            *out_of_bounds = 1;
        }
        return rom_idle_fill;
    }

    if( out_of_bounds )
    {
        *out_of_bounds = 0;
    }
    return rom_space[ phys_addr ];
}

void rom_backing_set_trace( rom_backing_trace_fn fn )
{
    rom_trace = fn;
}

uint8_t rom_backing_read_traced( uint32_t cpu_addr, uint32_t phys_addr, int rom_en_n )
{
    int oob = 0;
    uint8_t data = rom_backing_read( phys_addr, &oob );

    if( rom_trace )
    {
        rom_trace( cpu_addr, phys_addr, rom_en_n, data );
    }

    (void)oob;
    return data;
}
