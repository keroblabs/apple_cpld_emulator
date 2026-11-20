#include "sw_switch.h"
#include "mmu_controller.h"

//////////////////////////////////////////////////////////////////////////////////////

typedef struct ram_aux_main_data_t_
{
    uint8_t *sw_data;

    uint32_t input_address;

    uint32_t mem_address;
    bool_t is_io_space;

} ram_aux_main_data_t;

//////////////////////////////////////////////////////////////////////////////////////

#define RAM_AUX_PAGE                0x10000

#define ROM_GS_256_FIRMWARE_ADDR    0xFF0000
#define MEM_DRV_ROM_GS_ADDR         0xEC0000
#define MEM_IO_REGION_GS_ADDR       0xE60000
//#define ROM_DRV_REGION_ADDR         0x180000

#define RAM_READ_NOT_AVAILABLE      ( ( uint32_t )( -2 ) )
#define RAM_WRITE_NOT_AVAILABLE     ( ( uint32_t )( -3 ) )

//#define ROM_IO_MEMORY_ADDR(s)       ( ROM_DRV_REGION_ADDR | ( ( uint32_t )( ( s ) << 12 ) ) )               //  | ( ( uint32_t )( ( s ) << 8 ) ) )

#define ROM_GS_IO_MEMORY_ADDR(s)    ( MEM_DRV_ROM_GS_ADDR | ( ( uint32_t )( ( s ) << 12 ) ) )               //  | ( ( uint32_t )( ( s ) << 8 ) ) )
#define ROM_GS_IO_IOSTB_ADDR(s)     ( MEM_DRV_ROM_GS_ADDR | ( ( ( uint32_t )( ( s ) << 12 ) ) + 2048 ) )    //  | ( ( uint32_t )( 0x08  << 8 ) ) )

#define RAM_GET_BANK(a)             ( (a) >> 16 )

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t slot_index;
static uint8_t iostb_index;
//static bool_t  is_io_space;

//////////////////////////////////////////////////////////////////////////////////////

#include "memory_map.h"

//////////////////////////////////////////////////////////////////////////////////////

static void memory_io_space( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    // is_io_space = TRUE;
    data->mem_address = MEM_IO_REGION_GS_ADDR | ( data->input_address & 0x0FFFF );
    data->is_io_space = TRUE;
}

static void memory_map_pg0( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    data->mem_address = ( data->input_address & 0x0FFFF ) | ( ( data->sw_data[SW_ALTZP] == TRUE ) ? RAM_AUX_PAGE : 0 );
}

static void memory_map_lores( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    if( data->sw_data[SW_80STORE] == TRUE )
    {
        data->mem_address = ( data->input_address & 0x0FFFF ) | ( ( data->sw_data[SW_PAGE2] == TRUE ) ? RAM_AUX_PAGE : 0 );
    }
    else if( *ram_switch == TRUE )
    {
        data->mem_address = ( data->input_address & 0x0FFFF ) | RAM_AUX_PAGE;
    }
    else
    {
        data->mem_address = data->input_address;
    }
}

static void memory_map_lores2( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    data->mem_address = ( data->input_address & 0x0FFFF ) | ( ( *ram_switch == TRUE ) ? RAM_AUX_PAGE : 0 );
}

static void memory_map_hires( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    if( ( data->sw_data[SW_80STORE] == TRUE ) && ( data->sw_data[SW_HIRES] == TRUE ) )
    {
        data->mem_address = ( data->input_address & 0x0FFFF ) | ( ( data->sw_data[SW_PAGE2] == TRUE ) ? RAM_AUX_PAGE : 0 );
    }
    else if( *ram_switch == TRUE )
    {
        data->mem_address = ( data->input_address & 0x0FFFF ) | RAM_AUX_PAGE;
    }
    else
    {
        data->mem_address = data->input_address;
    }
}

static void memory_map_hires2( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    data->mem_address = ( data->input_address & 0x0FFFF ) | ( ( *ram_switch == TRUE ) ? RAM_AUX_PAGE : 0 );
}

static void memory_map_pagable( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    data->mem_address = ( data->input_address & 0x0FFFF ) | ( ( *ram_switch == TRUE ) ? RAM_AUX_PAGE : 0 );
}

static void memory_map_normal( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    data->mem_address = ( data->input_address & 0x0FFFF ) | ( ( *ram_switch == TRUE ) ? RAM_AUX_PAGE : 0 );
}

static void memory_map_iosel( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    uint8_t index = ( data->input_address & 0x0F00 ) >> 8;

    if( ram_switch != &data->sw_data[SW_RAMRD] )
    {
        data->mem_address = RAM_READ_NOT_AVAILABLE;
    }
    else if( ( data->sw_data[SW_SLOTXROM] == TRUE ) ||
           ( ( data->sw_data[SW_SLOT3ROM] == FALSE ) && ( index == 3 ) ) )
    {
        iostb_index = 0;
        slot_index = 0;
        data->mem_address = ROM_GS_256_FIRMWARE_ADDR + ( data->input_address & 0x0FFFF );
    }
//    else if( index == 3 )
//    {
//        iostb_index = 0;
//        slot_index = 0;
//        return GET_MEMORY_PTR( ROM_GS_256_FIRMWARE_ADDR + address - 0xC000 + 0x1000 - 0x1100 );
//    }
    else
    {
        iostb_index = index;
        slot_index = index;
        // return GET_MEMORY_PTR( ROM_IO_MEMORY_ADDR( index ) + ( address & 0x0FF ) );
        data->mem_address = ROM_GS_IO_MEMORY_ADDR( index ) + ( data->input_address & 0x0FF );
    }
}

static void memory_map_iostb( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    if( data->sw_data[SW_REG_SHADOW] & REG_SHADOW_IO_LC )
    {
        data->mem_address = data->input_address;
    }
    else if( ram_switch == &data->sw_data[SW_RAMRD] )
    {
        data->mem_address =   ( ( ( data->sw_data[SW_SLOTXROM] == TRUE ) || ( data->sw_data[SW_SLOT3ROM] == FALSE ) ) && ( ( iostb_index == 0 ) || ( iostb_index == 3 ) ) )
                            ? ROM_GS_256_FIRMWARE_ADDR + ( data->input_address )
                            : ROM_GS_IO_IOSTB_ADDR( iostb_index ) + ( data->input_address & 0x07FF );
                          //: ROM_IO_MEMORY_ADDR( iostb_index ) + ( ROM_IO_BLOCK_SIZE >> 1 ) + address & 0x7FF;
    }
    else
    {
        data->mem_address = RAM_READ_NOT_AVAILABLE;
    }

    if( ( data->input_address & 0x0FFFF ) == 0xCFFF )
    {
        slot_index = 0;
        iostb_index = 0;
    }
}

static void memory_map_rom_0( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    bool_t * exp_switch = ( ram_switch == &data->sw_data[SW_RAMRD] ) ? &data->sw_data[SW_EXP16RAM] : &data->sw_data[SW_EXP16WRITE];

    if( data->sw_data[SW_REG_SHADOW] & REG_SHADOW_IO_LC )
    {
        data->mem_address = data->input_address & 0x0FFFF;
    }
    if( *exp_switch == TRUE )
    {
        uint32_t mem_address = ( data->input_address ) | ( ( data->sw_data[SW_ALTZP] == TRUE ) ? RAM_AUX_PAGE : 0 );
        
        if( !( data->sw_data[SW_REG_SHADOW] & ( 1 << 6 ) ) )
        {
            mem_address -= ( data->sw_data[SW_EXP16BANK] == FALSE ) ? 0x1000 : 0;
        }

        data->mem_address = mem_address;
    }
    else if( ram_switch == &data->sw_data[SW_RAMRD] )
    {
        data->mem_address = ROM_GS_256_FIRMWARE_ADDR + ( data->input_address & 0x0FFFF );
    }
    else
    {
        data->mem_address = RAM_WRITE_NOT_AVAILABLE;
    }
}

static void memory_map_rom_1( bool_t * ram_switch, ram_aux_main_data_t * data )
{
    bool_t * exp_switch = ( ram_switch == &data->sw_data[SW_RAMRD] ) ? &data->sw_data[SW_EXP16RAM] : &data->sw_data[SW_EXP16WRITE];
    uint32_t address = data->input_address & 0x0FFFF;
    
    if( *exp_switch == TRUE )
    {
        data->mem_address = address | ( ( data->sw_data[SW_ALTZP] == TRUE ) ? RAM_AUX_PAGE : 0 );
    }
    else if( ram_switch == &data->sw_data[SW_RAMRD] )
    {
        data->mem_address = ROM_GS_256_FIRMWARE_ADDR + address;
    }
    else
    {
        data->mem_address = RAM_WRITE_NOT_AVAILABLE;
    }
}

uint32_t mmu_controller_get_address( uint16_t address, uint32_t sw_ram )
{
    ram_aux_main_data_t data;
    bool_t * p_sw_ram;

    data.sw_data = sw_switch_get_data();
    data.is_io_space = FALSE;
    data.input_address = address;
    data.mem_address = RAM_READ_NOT_AVAILABLE;

    if( sw_ram == SW_RAMRD )
    {
        p_sw_ram = &data.sw_data[SW_RAMRD];
    }
    else if( sw_ram == SW_RAMWR )
    {
        p_sw_ram = &data.sw_data[SW_RAMWR];
    }
    else
    {
        return RAM_READ_NOT_AVAILABLE;
    }

    ram_map[ ( address & 0x0FF00 ) >> 8 ]( p_sw_ram, &data );

    return data.mem_address;
}

uint8_t mmu_controller_read_memory( uint16_t address )
{
    uint32_t mem_address = mmu_controller_get_address( address, SW_RAMRD );
    uint8_t data = 0xA0;

    if( ( mem_address & 0xFF0000 ) == MEM_DRV_ROM_GS_ADDR )
    {
        // slot ROM
    }
    else if( ( mem_address & 0xFF0000 ) == ROM_GS_256_FIRMWARE_ADDR )
    {
        // firmware ROM
    }
    else if( ( mem_address & 0xFF0000 ) == MEM_IO_REGION_GS_ADDR )
    {
        // IO space
    }
    else
    {
        // RAM
    }

    return data;
}

void mmu_controller_init( void )
{
    slot_index = 0;
    iostb_index = 0;
}