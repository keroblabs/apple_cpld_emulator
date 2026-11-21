#include "rom_backing.h"
#include <string.h>

static uint8_t rom_space[ ROM_BACKING_SIZE ];
static rom_backing_trace_fn rom_trace;
static uint32_t rom_oob_reads;

void rom_backing_init( const uint8_t system_rom[ ROM_SYSTEM_SIZE ] )
{
    memset( rom_space, 0, sizeof( rom_space ) );
    memcpy( &rom_space[ ROM_SYSTEM_BASE ], system_rom, ROM_SYSTEM_SIZE );
    rom_oob_reads = 0;
}

int rom_backing_load_slot( int slot, const uint8_t *image, size_t image_len )
{
    if( slot < 1 || slot > 7 || image == NULL || image_len != ROM_SLOT_SIZE )
    {
        return -1;
    }

    memcpy( &rom_space[ (size_t)slot * ROM_SLOT_SIZE ], image, ROM_SLOT_SIZE );
    return 0;
}

uint8_t rom_backing_read( uint32_t phys_addr, int *out_of_bounds )
{
    const int invalid_address = phys_addr >= ROM_BACKING_SIZE;
    if( out_of_bounds )
    {
        *out_of_bounds = invalid_address;
    }
    return invalid_address ? 0 : rom_space[ phys_addr ];
}

void rom_backing_set_trace( rom_backing_trace_fn fn )
{
    rom_trace = fn;
}

uint32_t rom_backing_oob_count( void )
{
    return rom_oob_reads;
}

void rom_backing_oob_reset( void )
{
    rom_oob_reads = 0;
}

uint8_t rom_backing_read_traced( uint32_t cpu_addr, uint32_t phys_addr, int rom_en_n )
{
    int oob;
    uint8_t data = rom_backing_read( phys_addr, &oob );

    if( oob )
    {
        rom_oob_reads++;
    }
    if( rom_trace )
    {
        rom_trace( cpu_addr, phys_addr, rom_en_n, data, oob );
    }

    return data;
}
