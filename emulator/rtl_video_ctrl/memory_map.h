#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

//////////////////////////////////////////////////////////////////////////////////////

static void memory_map_pg0(     bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_map_lores(   bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_map_hires(   bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_map_lores2(  bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_map_hires2(  bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_map_normal(  bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_map_iosel(   bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_map_iostb(   bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_map_rom_0(   bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_map_rom_1(   bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_io_space(    bool_t * ram_sw, ram_aux_main_data_t * data );
static void memory_map_pagable( bool_t * ram_sw, ram_aux_main_data_t * data );

//////////////////////////////////////////////////////////////////////////////////////

typedef void ( *address_map_t )( bool_t * ram_sw, ram_aux_main_data_t * data );

//////////////////////////////////////////////////////////////////////////////////////

static const address_map_t ram_map[] =
{
    memory_map_pg0,  memory_map_pg0,        // 0x0000 - 0x1FF
    
    memory_map_normal, memory_map_normal,   // 0x0200 - 0x3FF
    
    memory_map_lores, memory_map_lores, memory_map_lores, memory_map_lores,         // 0x0400 - 0x07FF
    memory_map_lores2, memory_map_lores2, memory_map_lores2, memory_map_lores2,     // 0x0800 - 0x0BFF
    
    memory_map_pagable, memory_map_pagable, memory_map_pagable, memory_map_pagable,     // 0x0C00 - 0x1FFF
    memory_map_pagable, memory_map_pagable, memory_map_pagable, memory_map_pagable,
    memory_map_pagable, memory_map_pagable, memory_map_pagable, memory_map_pagable,
    memory_map_pagable, memory_map_pagable, memory_map_pagable, memory_map_pagable,
    memory_map_pagable, memory_map_pagable, memory_map_pagable, memory_map_pagable,

    memory_map_hires, memory_map_hires, memory_map_hires, memory_map_hires,         // 0x2000 - 0x3FFF
    memory_map_hires, memory_map_hires, memory_map_hires, memory_map_hires,
    memory_map_hires, memory_map_hires, memory_map_hires, memory_map_hires,
    memory_map_hires, memory_map_hires, memory_map_hires, memory_map_hires,
    memory_map_hires, memory_map_hires, memory_map_hires, memory_map_hires,
    memory_map_hires, memory_map_hires, memory_map_hires, memory_map_hires,
    memory_map_hires, memory_map_hires, memory_map_hires, memory_map_hires,
    memory_map_hires, memory_map_hires, memory_map_hires, memory_map_hires,

    memory_map_hires2, memory_map_hires2, memory_map_hires2, memory_map_hires2,     // 0x4000 - 0x5FFF
    memory_map_hires2, memory_map_hires2, memory_map_hires2, memory_map_hires2,
    memory_map_hires2, memory_map_hires2, memory_map_hires2, memory_map_hires2,
    memory_map_hires2, memory_map_hires2, memory_map_hires2, memory_map_hires2,
    memory_map_hires2, memory_map_hires2, memory_map_hires2, memory_map_hires2,
    memory_map_hires2, memory_map_hires2, memory_map_hires2, memory_map_hires2,
    memory_map_hires2, memory_map_hires2, memory_map_hires2, memory_map_hires2,
    memory_map_hires2, memory_map_hires2, memory_map_hires2, memory_map_hires2,

    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,     // 0x6000 - 0xBFFF
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    memory_map_normal, memory_map_normal, memory_map_normal, memory_map_normal,
    
    memory_io_space,                                                                // 0xC0xx
                      memory_map_iosel, memory_map_iosel, memory_map_iosel,         // 0xC100 - 0xC7FF
    memory_map_iosel, memory_map_iosel, memory_map_iosel, memory_map_iosel,
    
    memory_map_iostb, memory_map_iostb, memory_map_iostb, memory_map_iostb,         // 0xC800 - 0xCFFF
    memory_map_iostb, memory_map_iostb, memory_map_iostb, memory_map_iostb,
    
    memory_map_rom_0, memory_map_rom_0, memory_map_rom_0, memory_map_rom_0,         // 0xD000 - 0xDFFF
    memory_map_rom_0, memory_map_rom_0, memory_map_rom_0, memory_map_rom_0,
    memory_map_rom_0, memory_map_rom_0, memory_map_rom_0, memory_map_rom_0,
    memory_map_rom_0, memory_map_rom_0, memory_map_rom_0, memory_map_rom_0,
    
    memory_map_rom_1, memory_map_rom_1, memory_map_rom_1, memory_map_rom_1,         // 0xC000 - 0xFFFF
    memory_map_rom_1, memory_map_rom_1, memory_map_rom_1, memory_map_rom_1,
    memory_map_rom_1, memory_map_rom_1, memory_map_rom_1, memory_map_rom_1,
    memory_map_rom_1, memory_map_rom_1, memory_map_rom_1, memory_map_rom_1,
    memory_map_rom_1, memory_map_rom_1, memory_map_rom_1, memory_map_rom_1,
    memory_map_rom_1, memory_map_rom_1, memory_map_rom_1, memory_map_rom_1,
    memory_map_rom_1, memory_map_rom_1, memory_map_rom_1, memory_map_rom_1,
    memory_map_rom_1, memory_map_rom_1, memory_map_rom_1, memory_map_rom_1,
};

#endif // MEMORY_MAP_H