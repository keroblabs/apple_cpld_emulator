#include "memory_controller.h"
#include "buffer.h"
#include "rtl_model.h"
#include "apple2rom.h"
#include "ram_bus_hw.h"
#include "disc_drive.h"

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t ram_memory[ RAM_MEMORY_SIZE ];
static uint8_t rom_memory[ ROM_SIZE * 2 ];

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t board_read_bus( cpld_model_t * cpld, uint32_t cpu_address )
{
    uint8_t data_xfer = 0;

    if( !cpld->MMU.ram_en_n )
    {
        data_xfer = ram_memory[ cpld->MMU.mem_address ];
    }
    else if( !cpld->MMU.rom_en_n )
    {
        data_xfer = rom_memory[ cpld->MMU.mem_address & 0x0FFFF ];
    }
    else if( !cpld->MMU.IOSEL_N )
    {

    }
    else if( !cpld->IOU.iosel_6_n )
    {
        //data_xfer = disc_rom_access( cpu_address );
        //printf( "Slot 6 ROM: 0x%08X\n", cpu_address );        
        //data_xfer = disk_drive_fw[ cpu_address & 0x0FF ];
    }
    else if( !cpld->MMU.IOSTB_N )
    {

    }
    else if( !cpld->MMU.DEVSEL_N )
    {

    }
    else if( !cpld->IOU.devsel_3_n )
    {
        
    }
    else if( !cpld->IOU.devsel_6_n )
    {
        data_xfer = disc_io_access( cpu_address, 0, FALSE );
        //printf( "Slot 6 IO: 0x%08X\n", cpu_address );        
    }
    else
    {
        ram_bus_hw_access( cpu_address, data_xfer, TRUE );      
    }

    data_xfer |= cpld->IOU.data_bus;

    return data_xfer;
}

static void board_write_bus( cpld_model_t * cpld, uint32_t cpu_address, uint8_t data_xfer )
{
    if( !cpld->MMU.ram_en_n )
    {
        ram_memory[ cpld->MMU.mem_address ] = data_xfer;
        ram_bus_hw_access( cpld->MMU.mem_address, data_xfer, FALSE );             
    }
    else if( !cpld->MMU.DEVSEL_N )
    {

    }
    else if( !cpld->IOU.devsel_3_n )
    {
        
    }
    else if( !cpld->IOU.devsel_6_n )
    {
        disc_io_access( cpu_address, data_xfer, TRUE );
        //printf( "Slot 6 IO: 0x%08X <= 0x%02X\n", cpu_address, data_xfer );
    }
    else
    {
        ram_bus_hw_access( cpu_address, data_xfer, FALSE );      
    }
}

void memory_controller_init( void )
{
    // reset main memory
    memset( &ram_memory[0], 0xA0, RAM_MEMORY_SIZE );

    // set default values for rom
    memset( &rom_memory[0], 0xA0, ROM_SIZE * 2 );

    // load Apple firmware ROM
    memcpy( &rom_memory[32 * 1024], apple_rom_bin,  sizeof( apple_rom_bin ) );

    disc_drive_init();
}

uint8_t memory_controller_access( void * p_cpld, uint32_t cpu_address, bool_t is_read, uint8_t data_in )
{
    uint8_t data_out = 0;
    cpld_model_t * cpld = ( cpld_model_t * )p_cpld;

    if( !is_read )
    {
        board_write_bus( cpld, cpu_address, data_in );
    }
    else
    {
        data_out = board_read_bus( cpld, cpu_address );
    }

    return data_out;
}

void memory_controller_tick( void * p_cpld, uint64_t cycles )
{
    // cpld_model_t * cpld = ( cpld_model_t * )p_cpld;

    disc_process( cycles );
}