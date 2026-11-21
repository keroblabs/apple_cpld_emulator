// CPLD-ROM-0003 — Emulator ROM image backing implementation.
// See rom_backing.h and docs/rom-memory-map.md.

#include "rom_backing.h"
#include <string.h>

#define ROM_PAGE_SHIFT      8                                   // 256-byte pages
#define ROM_PAGE_COUNT      ( ROM_BACKING_SIZE >> ROM_PAGE_SHIFT )

static uint8_t              rom_space[ ROM_BACKING_SIZE ];
static uint8_t              rom_occupied[ ROM_PAGE_COUNT ];      // 256-byte granularity
static uint8_t              rom_idle_fill = 0xA0;
static rom_backing_trace_fn rom_trace     = 0;
static uint32_t             rom_oob_reads = 0;

//////////////////////////////////////////////////////////////////////////////

uint32_t rom_backing_slot_entry_phys( int slot )
{
    // Apple $Cn00 -> window (N<<12) + (N<<8); 256-byte IOSEL page in first 2 KiB.
    return ( (uint32_t)slot << 12 ) | ( (uint32_t)slot << 8 );
}

uint32_t rom_backing_slot_expansion_phys( int slot )
{
    // Apple $C800 -> window (N<<12) + 0x800; 2 KiB IOSTB region in second 2 KiB.
    return ( (uint32_t)slot << 12 ) | ROM_SLOT_EXP_OFFSET;
}

//////////////////////////////////////////////////////////////////////////////

static int range_is_free( uint32_t base, uint32_t len )
{
    uint32_t first = base >> ROM_PAGE_SHIFT;
    uint32_t last  = ( base + len - 1 ) >> ROM_PAGE_SHIFT;
    for( uint32_t p = first; p <= last; p++ )
    {
        if( rom_occupied[ p ] )
        {
            return 0;
        }
    }
    return 1;
}

static void range_mark( uint32_t base, uint32_t len )
{
    uint32_t first = base >> ROM_PAGE_SHIFT;
    uint32_t last  = ( base + len - 1 ) >> ROM_PAGE_SHIFT;
    for( uint32_t p = first; p <= last; p++ )
    {
        rom_occupied[ p ] = 1;
    }
}

//////////////////////////////////////////////////////////////////////////////

void rom_backing_init( const uint8_t *system_rom,
                       uint32_t system_len,
                       uint32_t system_base,
                       uint8_t idle_fill )
{
    rom_idle_fill = idle_fill;
    rom_oob_reads = 0;
    memset( rom_space, idle_fill, ROM_BACKING_SIZE );
    memset( rom_occupied, 0, sizeof( rom_occupied ) );

    if( system_rom && system_base < ROM_BACKING_SIZE )
    {
        uint32_t space = ROM_BACKING_SIZE - system_base;
        uint32_t len   = ( system_len < space ) ? system_len : space;
        memcpy( &rom_space[ system_base ], system_rom, len );
        if( len > 0 )
        {
            range_mark( system_base, len );
        }
    }
}

int rom_backing_install( uint32_t phys_base, const uint8_t *data, uint32_t len )
{
    if( data == 0 || len == 0 )
    {
        return -1;
    }
    // Reject regions that fall outside or straddle the end of the logical space.
    if( phys_base >= ROM_BACKING_SIZE ||
        len > ROM_BACKING_SIZE ||
        phys_base + len > ROM_BACKING_SIZE )
    {
        return -1;
    }
    // Reject overlap with an already-installed region.
    if( !range_is_free( phys_base, len ) )
    {
        return -1;
    }

    memcpy( &rom_space[ phys_base ], data, len );
    range_mark( phys_base, len );
    return 0;
}

int rom_backing_fill_baseline( uint32_t phys_base, const uint8_t *data, uint32_t len )
{
    if( data == 0 || len == 0 )
    {
        return -1;
    }
    if( phys_base >= ROM_BACKING_SIZE ||
        len > ROM_BACKING_SIZE ||
        phys_base + len > ROM_BACKING_SIZE )
    {
        return -1;
    }
    // Baseline content: no occupancy marking so a packaged image may override.
    memcpy( &rom_space[ phys_base ], data, len );
    return 0;
}

int rom_backing_install_region( int kind, int slot, const uint8_t *data, uint32_t len )
{
    uint32_t base;
    uint32_t max_len;

    if( data == 0 || len == 0 )
    {
        return -1;
    }

    switch( kind )
    {
        case ROM_REGION_SYSTEM:
            base    = ROM_SYSTEM_BASE;
            max_len = ROM_BACKING_SIZE - ROM_SYSTEM_BASE;
            break;

        case ROM_REGION_SLOT_ENTRY:
            if( slot < 1 || slot > 7 )
            {
                return -1;
            }
            base    = rom_backing_slot_entry_phys( slot );
            max_len = ROM_SLOT_ENTRY_SIZE;
            // Entry page must stay within the first 2 KiB of the slot window.
            if( ( base & 0x800u ) != 0 ||
                ( ( base + ROM_SLOT_ENTRY_SIZE - 1 ) & ~0xFFFu ) != ( (uint32_t)slot << 12 ) )
            {
                return -1;
            }
            break;

        case ROM_REGION_SLOT_EXP:
            if( slot < 1 || slot > 7 )
            {
                return -1;
            }
            base    = rom_backing_slot_expansion_phys( slot );
            max_len = ROM_SLOT_EXP_SIZE;
            break;

        default:
            return -1;
    }

    if( len > max_len )
    {
        return -1;
    }

    return rom_backing_install( base, data, len );
}

//////////////////////////////////////////////////////////////////////////////
// Packed *.romimg loader (see docs/rom-memory-map.md §4).
//   0x0000  4   magic 'C','P','R','M'
//   0x0004  2   version (LE)
//   0x0006  2   entry count N (LE)
//   0x0008  N*16  entry table
//   ...     payload regions
// Entry (16 bytes):
//   0x00  1   kind (ROM_REGION_*)
//   0x01  1   slot (0 for SYSTEM; 1..7 for slot regions)
//   0x02  2   Apple CPU base address (LE)  [informational]
//   0x04  4   length (LE)
//   0x08  4   payload file offset (LE)
//   0x0C  4   CRC32 (LE, 0 = unchecked)   [not verified here]

static uint16_t rd16( const uint8_t *p ) { return (uint16_t)( p[0] | ( p[1] << 8 ) ); }
static uint32_t rd32( const uint8_t *p )
{
    return (uint32_t)p[0] | ( (uint32_t)p[1] << 8 ) |
           ( (uint32_t)p[2] << 16 ) | ( (uint32_t)p[3] << 24 );
}

int rom_backing_load_image( const uint8_t *img, uint32_t img_len )
{
    if( img == 0 || img_len < 8 )
    {
        return -1;
    }
    if( !( img[0] == 'C' && img[1] == 'P' && img[2] == 'R' && img[3] == 'M' ) )
    {
        return -2;
    }

    uint16_t count = rd16( &img[6] );
    uint32_t table = 8;
    if( (uint64_t)table + (uint64_t)count * 16u > img_len )
    {
        return -3;
    }

    for( uint16_t i = 0; i < count; i++ )
    {
        const uint8_t *e = &img[ table + (uint32_t)i * 16u ];
        int      kind    = e[0];
        int      slot    = e[1];
        uint32_t length  = rd32( &e[4] );
        uint32_t payoff  = rd32( &e[8] );

        if( length == 0 ||
            (uint64_t)payoff + (uint64_t)length > img_len )
        {
            return -4;
        }
        if( rom_backing_install_region( kind, slot, &img[ payoff ], length ) != 0 )
        {
            return -5;
        }
    }

    return 0;
}

//////////////////////////////////////////////////////////////////////////////

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
    int oob = 0;
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
