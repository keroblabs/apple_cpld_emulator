#include "ram_bus_hw.h"
#include "board.h"
#include "sw_switch.h"
#include "char_map.h"
#include "t_threads.h"

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t video_ram_memory[RAM_MEMORY_SIZE];

static io_addr_t p_io_read[256];
static io_addr_t p_io_write[256];

static void * vram_mutex;

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t dummy_io( uint8_t address, uint8_t null1, bool_t is_write )
{
    return 0;
}

uint8_t ram_bus_read_sw_switch( uint16_t index )
{
    uint8_t ret;
    tthread_mutex_lock( vram_mutex );
    ret = SW_SWITCH( index );
    tthread_mutex_unlock( vram_mutex );

    return ret;
}

uint8_t ram_bus_hw_access( uint32_t address, uint8_t data_in, bool_t is_read )
{
    uint8_t data = data_in;

    tthread_mutex_lock( vram_mutex );

    if( is_read )
    {
        if( ( address & 0x0FF00 ) == MEM_IO_REGION_ADDR )
        {
            data = p_io_read[ address & 0x0FF ]( ( uint8_t )( address & 0x0FF ), 0, FALSE );
        }
        else if( ( address & 0xFF0000 ) == MEM_GS_VIDEO_CHAR_MAP )
        {
            data = char_map[ address & 0x0FFF ];
        }
        else if( address != NEG_U32 )
        {
            uint32_t ram_address = ( ( address & 0x0FFFF ) << 1 ) | ( address & 0xFE0000 ) | ( ( address & 0x010000 ) ? 0x01 : 0x00 );
            data = video_ram_memory[ ram_address ];
        }
    }
    else
    {
        if( ( address & 0x0FF00 ) == MEM_IO_REGION_ADDR )
        {
            p_io_write[ address & 0x0FF ]( ( uint8_t )( address & 0x0FF ), data, TRUE );
        }
        else if( address != NEG_U32 )
        {
            uint32_t ram_address = ( ( address & 0x0FFFF ) << 1 ) | ( address & 0xFE0000 ) | ( ( address & 0x010000 ) ? 0x01 : 0x00 );
            video_ram_memory[ ram_address ] = data;
        }
    }

    tthread_mutex_unlock( vram_mutex );
    
    return data;
}

uint16_t ram_bus_hw_read_16( uint32_t address )
{
    uint16_t * video_ram_16 = ( uint16_t * )video_ram_memory;
    return video_ram_16[ address & 0x0FFFF ];
}

void ram_bus_hw_reset_set_io_handler( uint8_t page, io_addr_t io_read, io_addr_t io_write )
{
    if( ( io_read == NULL ) && ( io_write == NULL ) )
    {
        io_write = dummy_io;
        io_read = dummy_io;
    }

    if( io_read != NULL ) p_io_read[ page ] = ( io_addr_t )io_read;
    if( io_write != NULL ) p_io_write[ page ] = ( io_addr_t )io_write;
}

void ram_bus_hw_reset( void )
{
    // reset main memory
    memset( &video_ram_memory[0], 0xA0, ( 64 * 1024 ) );

    // reset aux memory
    memset( &video_ram_memory[( 64 * 1024 )], 0xA0, ( 64 * 1024 ) );
}

void ram_bus_hw_init( void )
{
    vram_mutex = tthread_mutex_init();

    // reset all memory
    for( int counter = 0; counter < 256; counter++ )
    {
        p_io_read[counter] = dummy_io;
        p_io_write[counter] = dummy_io;
    }
}


