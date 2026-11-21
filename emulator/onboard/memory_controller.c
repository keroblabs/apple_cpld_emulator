#include "memory_controller.h"
#include "buffer.h"
#include "rtl_model.h"
#include "apple2rom.h"
#include "ram_bus_hw.h"
#include "disc_drive.h"
#include "rom_backing.h"

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t ram_memory[ RAM_MEMORY_SIZE ];

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t board_read_bus( cpld_data_t * cpld, uint32_t cpu_address )
{
    uint8_t data_xfer = 0;

    if( !cpld->MMU.ram_en_n )
    {
        data_xfer = ram_memory[ cpld->MMU.mem_address ];
    }
    else if( !cpld->MMU.rom_en_n )
    {
        // ROM path is addressed by the MMU's translated physical address
        // (MEM_ADDR_OUT) with explicit bounds; see rom_backing / ROM-0003.
        data_xfer = rom_backing_read_traced( cpu_address,
                                             cpld->MMU.mem_address,
                                             cpld->MMU.rom_en_n );
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

static void board_write_bus( cpld_data_t * cpld, uint32_t cpu_address, uint8_t data_xfer )
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

    // Initialise the logical ROM space and load the Apple firmware image at its
    // CPU base ($8000). Per-slot IO firmware (e.g. slot 7 ProDOS) is installed
    // from a packaged *.romimg via memory_controller_load_rom_image() once an
    // image exists (ROM-0002 format / ROM-0003 loader).
    rom_backing_init( apple_rom_bin, sizeof( apple_rom_bin ), 32 * 1024, 0xA0 );

    // Preserve existing Apple-visible behaviour under the 4 KiB-per-slot layout:
    // migrate the internal $C100-$CFFF firmware (as previously served from the
    // raw ROM image) into each per-slot window as a BASELINE. This keeps the
    // slot-6 Disk II boot ROM and other internal firmware byte-identical at the
    // Apple address level, while the MMU now maps ROM reads to per-slot windows.
    // A packaged image loaded later may override any of these baselines.
    {
        // apple_rom_bin covers $8000-$FFFF; $C000 is at image offset 0x4000.
        const uint8_t *cpage = &apple_rom_bin[ 0xC000 - 0x8000 ];
        int slot;
        for( slot = 1; slot <= 7; slot++ )
        {
            // IOSEL entry page $Cn00-$CnFF (256 bytes).
            rom_backing_fill_baseline( rom_backing_slot_entry_phys( slot ),
                                       &cpage[ slot * 0x100 ], 0x100 );
            // Shared IOSTB expansion $C800-$CFFF (2 KiB) replicated per slot,
            // matching the previous raw-address behaviour for any selected slot.
            rom_backing_fill_baseline( rom_backing_slot_expansion_phys( slot ),
                                       &cpage[ 0x800 ], 0x800 );
        }
    }

    disc_drive_init();
}

int memory_controller_load_rom_image( const void * image, uint32_t image_len )
{
    // Documented startup route for installing per-slot IO firmware ROM from a
    // packed image. Validation (window geometry, overlap, bounds) is enforced
    // by the rom_backing loader.
    return rom_backing_load_image( ( const uint8_t * )image, image_len );
}

uint8_t memory_controller_access( void * p_cpld, uint32_t cpu_address, bool_t is_read, uint8_t data_in )
{
    uint8_t data_out = 0;
    cpld_data_t * cpld = ( cpld_data_t * )p_cpld;

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