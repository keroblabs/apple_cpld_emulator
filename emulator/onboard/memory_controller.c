#include "memory_controller.h"
#include "rtl_model.h"
#include "ram_bus_hw.h"
#include "disc_drive.h"
#include "rom_backing.h"
#include <string.h>

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t ram_memory[ RAM_MEMORY_SIZE ];

//////////////////////////////////////////////////////////////////////////////////////

static bool_t flash_selected( const cpld_data_t *cpld )
{
    return !cpld->MMU.rom_en_n || !cpld->MMU.IOSEL_N ||
           !cpld->MMU.IOSTB_N || !cpld->IOU.iosel_6_n;
}

static bool_t board_read_memory( cpld_data_t *cpld, uint32_t cpu_address,
                                 uint8_t *data_out )
{
    if( !cpld->MMU.bus_rd_n && !cpld->MMU.ram_en_n )
    {
        *data_out = ram_memory[ cpld->MMU.mem_address ];
        return TRUE;
    }

    if( !cpld->MMU.bus_rd_n && flash_selected( cpld ) )
    {
        // The MMU supplies an address for Apple ROM and every flash-owned
        // card-ROM path, including the dedicated slot-6 selection.
        *data_out = rom_backing_read_traced( cpu_address,
                                             cpld->MMU.mem_address,
                                             cpld->MMU.rom_en_n );
        return TRUE;
    }

    return FALSE;
}

static bool_t board_write_memory( cpld_data_t *cpld, uint8_t data_xfer )
{
    if( !cpld->MMU.bus_wr_n && !cpld->MMU.ram_en_n )
    {
        ram_memory[ cpld->MMU.mem_address ] = data_xfer;
        ram_bus_hw_access( cpld->MMU.mem_address, data_xfer, FALSE );
        return TRUE;
    }

    if( !cpld->MMU.bus_wr_n && flash_selected( cpld ) )
    {
        // No authorised flash-program selection or mutable NOR command model
        // exists yet. RTL keeps BUS_WR_N inactive for flash until that route
        // is separately reviewed; ordinary ROM writes never reach this path.
        return TRUE;
    }

    return FALSE;
}

static uint8_t board_read_other( cpld_data_t *cpld, uint32_t cpu_address )
{
    uint8_t data_xfer = 0;

    if( !cpld->MMU.DEVSEL_N )
    {
    }
    else if( !cpld->IOU.devsel_3_n )
    {
    }
    else if( !cpld->IOU.devsel_6_n )
    {
        data_xfer = disc_io_access( cpu_address, 0, FALSE );
    }
    else
    {
        data_xfer = ram_bus_hw_access( cpu_address, data_xfer, TRUE );
    }

    return data_xfer;
}

static void board_write_other( cpld_data_t *cpld, uint32_t cpu_address,
                               uint8_t data_xfer )
{
    if( !cpld->MMU.DEVSEL_N )
    {
    }
    else if( !cpld->IOU.devsel_3_n )
    {
    }
    else if( !cpld->IOU.devsel_6_n )
    {
        disc_io_access( cpu_address, data_xfer, TRUE );
    }
    else
    {
        ram_bus_hw_access( cpu_address, data_xfer, FALSE );
    }
}

void memory_controller_init( void )
{
    rom_backing_init();
    memset( ram_memory, 0xA0, RAM_MEMORY_SIZE );
    disc_drive_init();
}

uint8_t memory_controller_access( void *p_cpld, uint32_t cpu_address,
                                  bool_t is_read, uint8_t data_in )
{
    uint8_t data_out = 0;
    cpld_data_t *cpld = ( cpld_data_t * )p_cpld;

    // Memory operations are selected by the sampled RTL strobe plus its
    // existing device select. CPU direction is only used for I/O fallback.
    if( !board_read_memory( cpld, cpu_address, &data_out ) &&
        !board_write_memory( cpld, data_in ) )
    {
        if( is_read )
        {
            data_out = board_read_other( cpld, cpu_address );
        }
        else
        {
            board_write_other( cpld, cpu_address, data_in );
        }
    }

    data_out |= cpld->IOU.data_bus;
    return data_out;
}

void memory_controller_tick( void *p_cpld, uint64_t cycles )
{
    (void)p_cpld;
    disc_process( cycles );
}
