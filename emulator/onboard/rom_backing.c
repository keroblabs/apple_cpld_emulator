#define _POSIX_C_SOURCE 200809L

#include "rom_backing.h"
#include "apple2rom.h"
#include "slot_roms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

_Static_assert( sizeof( apple_rom_bin ) == ROM_SYSTEM_SIZE, "Unexpected Apple ROM size" );
_Static_assert( sizeof( slot1_rom_bin ) == ROM_SLOT_SIZE, "slot1_rom_bin must contain 4096 bytes" );
_Static_assert( sizeof( slot2_rom_bin ) == ROM_SLOT_SIZE, "slot2_rom_bin must contain 4096 bytes" );
_Static_assert( sizeof( slot4_rom_bin ) == ROM_SLOT_SIZE, "slot4_rom_bin must contain 4096 bytes" );
_Static_assert( sizeof( slot5_rom_bin ) == ROM_SLOT_SIZE, "slot5_rom_bin must contain 4096 bytes" );
_Static_assert( sizeof( slot6_rom_bin ) == ROM_SLOT_SIZE, "slot6_rom_bin must contain 4096 bytes" );
_Static_assert( sizeof( slot7_rom_bin ) == ROM_SLOT_SIZE, "slot7_rom_bin must contain 4096 bytes" );

// Slot 0 is the released selector. Slot 3 uses the internal Apple ROM.
static const uint8_t *const slot_roms[8] = {
    [1] = slot1_rom_bin,
    [2] = slot2_rom_bin,
    [4] = slot4_rom_bin,
    [5] = slot5_rom_bin,
    [6] = slot6_rom_bin,
    [7] = slot7_rom_bin,
};

static uint8_t rom_space[ ROM_BACKING_SIZE ];
static rom_backing_trace_fn rom_trace;
static uint32_t rom_oob_reads;

void rom_backing_init( void )
{
    memset( rom_space, 0, sizeof( rom_space ) );
    memcpy( &rom_space[ ROM_SYSTEM_BASE ], apple_rom_bin, sizeof( apple_rom_bin ) );
    for( size_t slot = 0; slot < sizeof( slot_roms ) / sizeof( slot_roms[0] ); ++slot )
    {
        if( slot_roms[slot] )
        {
            memcpy( &rom_space[ slot * ROM_SLOT_SIZE ], slot_roms[slot], ROM_SLOT_SIZE );
        }
    }
    rom_oob_reads = 0;
}

int rom_backing_write_image( const char *path )
{
    if( path == NULL || path[0] == '\0' )
    {
        return -1;
    }

    // Publish the new image only after all bytes have been written and closed.
    const size_t temp_size = strlen( path ) + sizeof( ".tmp.XXXXXX" );
    char *temp_path = malloc( temp_size );
    if( temp_path == NULL )
    {
        return -1;
    }
    snprintf( temp_path, temp_size, "%s.tmp.XXXXXX", path );

    const int fd = mkstemp( temp_path );
    if( fd == -1 )
    {
        free( temp_path );
        return -1;
    }

    int result = -1;
    FILE *file = fdopen( fd, "wb" );
    if( file )
    {
        const size_t written = fwrite( rom_space, 1, sizeof( rom_space ), file );
        if( fclose( file ) == 0 && written == sizeof( rom_space ) )
        {
            result = rename( temp_path, path );
        }
    }
    else
    {
        close( fd );
    }
    if( result != 0 )
    {
        unlink( temp_path );
    }
    free( temp_path );
    return result;
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
