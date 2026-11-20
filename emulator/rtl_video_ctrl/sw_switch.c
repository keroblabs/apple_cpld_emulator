#include "board.h"
#include "sw_switch.h"
#include "ram_bus_hw.h"
#include "rtl_video.h"

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

uint8_t __sw_switch[SW_MAXINDEX];

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

static io_addr_t sys_ctrl_rd[256];
static io_addr_t sys_ctrl_wr[256];
//static uint8_t ctrl_curr_cpu = CORE_CPU_NONE;

static uint8_t    keycode;
static bool_t     keywaiting;

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t SW_sys_dummy_rd( uint8_t address, uint8_t null1, bool_t is_write )
{
    return 0;
}

static uint8_t SW_sys_dummy_wr( uint8_t address, uint8_t null1, bool_t is_write )
{
    return 0;
}

static uint8_t SW_MemoryPageControl( uint8_t address, uint8_t null1, bool_t is_write )
{
	switch( ( address & 0x0F ) >> 1 )
	{
        case 0x00: SET_SW_SWITCH(SW_80STORE,  address & 0x01 ); break;  // 0xC000 / 0xC001
        case 0x01: SET_SW_SWITCH(SW_RAMRD,    address & 0x01 ); break;  // 0xC002 / 0xC003
        case 0x02: SET_SW_SWITCH(SW_RAMWR,    address & 0x01 ); break;  // 0xC004 / 0xC005
            
        case 0x04: SET_SW_SWITCH(SW_ALTZP,    address & 0x01 ); break;  // 0xC008 / 0xC009
        case 0x06: SET_SW_SWITCH(SW_80COL,    address & 0x01 ); break;  // 0xC00C / 0xC00D
            
        case 0x03: SET_SW_SWITCH(SW_SLOTXROM, address & 0x01 ); break; // 0xC006 / 0xC007
		case 0x05: SET_SW_SWITCH(SW_SLOT3ROM, address & 0x01 ); break; // 0xC00A / 0xC00B
        case 0x07: SET_SW_SWITCH(SW_CHARSET,  address & 0x01 ); break; // 0xC00E / 0xC00F
	}

	//printf( "%s = %d\n", sw_switch_name_01[( address & 0x0F ) >> 1], address & 0x01 );

//	if( ( address & 0x0F ) == 0x01 )
//	{
//		sw_switch.SW_PAGE2 = 0;
//	}

	return( 0x00 );
}

static uint8_t SW_VideoControl( uint8_t address, uint8_t null1, bool_t is_write )
{
	uint8_t check_addr = ( address & 0x0F ) >> 1;
	
	switch( check_addr )
	{
		case 0x00: SET_SW_SWITCH( SW_TEXT,	address & 0x01 ); break; // 0xC050 / 0xC051
		case 0x01: SET_SW_SWITCH( SW_MIXED,	address & 0x01 ); break; // 0xC052 / 0xC053
		case 0x02: SET_SW_SWITCH( SW_PAGE2,	address & 0x01 ); break; // 0xC054 / 0xC055
		case 0x03: SET_SW_SWITCH( SW_HIRES,	address & 0x01 ); break; // 0xC056 / 0xC057
		case 0x04: SET_SW_SWITCH( SW_AN0,	address & 0x01 ); break; // 0xC058 / 0xC059
		case 0x05: SET_SW_SWITCH( SW_AN1,	address & 0x01 ); break; // 0xC05A / 0xC05B
		case 0x06: SET_SW_SWITCH( SW_AN2,	address & 0x01 ); break; // 0xC05C / 0xC05D
		case 0x07: SET_SW_SWITCH( SW_AN3,   address & 0x01 ); break; // 0xC05E / 0xC05F
	}
    
	//printf( "%s = %d\n", sw_switch_name_03[check_addr], address & 0x01 );

	return( 0x00 );
}

#ifdef EMULATOR_C_ONLY

uint8_t SW_MemoryPageStatus( uint8_t address, uint8_t null1 ) 
{
	uint8_t result = 0;

	switch( address & 0x0F )
	{
		//case 0x00: return( __sw_switch[SW_EXP16WRITE]   ? 0x80 : 0x00 ); break;
		case 0x01: result = ( __sw_switch[SW_EXP16BANK]   ? 0x80 : 0x00 ); break;
		case 0x02: result = ( __sw_switch[SW_EXP16RAM]    ? 0x80 : 0x00 ); break;
		case 0x03: result = ( __sw_switch[SW_RAMRD]       ? 0x80 : 0x00 ); break;
		case 0x04: result = ( __sw_switch[SW_RAMWR]       ? 0x80 : 0x00 ); break;
		case 0x05: result = ( __sw_switch[SW_SLOTXROM]    ? 0x80 : 0x00 ); break;
		case 0x06: result = ( __sw_switch[SW_ALTZP]		 ? 0x80 : 0x00 ); break;
		case 0x07: result = ( __sw_switch[SW_SLOT3ROM]    ? 0x80 : 0x00 ); break;
		case 0x08: result = ( __sw_switch[SW_80STORE]     ? 0x80 : 0x00 ); break;
		//case 0x09: return 0;
		case 0x0A: result = ( __sw_switch[SW_TEXT]		 ? 0x80 : 0x00 ); break;
		case 0x0B: result = ( __sw_switch[SW_MIXED]		 ? 0x80 : 0x00 ); break;
		case 0x0C: result = ( __sw_switch[SW_PAGE2]		 ? 0x80 : 0x00 ); break;
		case 0x0D: result = ( __sw_switch[SW_HIRES]		 ? 0x80 : 0x00 ); break;
		case 0x0E: result = ( __sw_switch[SW_CHARSET]	 ? 0x80 : 0x00 ); break;
		case 0x0F: result = ( __sw_switch[SW_80COL]		 ? 0x80 : 0x00 ); break;
	}

	return( result | keycode );
}

static uint8_t SW_ReadKeycode( uint8_t null1, uint8_t null2 ) 
{
	return( keycode | ( keywaiting ? 0x80 : 0 ) );
}

static uint8_t SW_KeyboardClear( uint8_t null1, uint8_t null2 ) 
{
	keywaiting = FALSE;
	return keycode;
}

#endif

void sw_switch_set_keycode( uint8_t new_keycode )
{
	keycode = new_keycode;
	keywaiting = TRUE;
}

void sw_switch_init( void )
{
	uint32_t counter;
    
    for( counter = 0; counter < 256; counter++ )
    {
        sys_ctrl_rd[ counter ] = SW_sys_dummy_rd;
        sys_ctrl_wr[ counter ] = SW_sys_dummy_wr;
    }
	
	for( counter = 0x00; counter <= 0x0F; counter++ )
	{
        ram_bus_hw_reset_set_io_handler( counter, NULL, SW_MemoryPageControl );
	}

	#ifdef EMULATOR_C_ONLY
	for( counter = 0x00; counter <= 0x0F; counter++ )
	{
		ram_bus_hw_reset_set_io_handler( counter, SW_ReadKeycode, NULL );
	}

	ram_bus_hw_reset_set_io_handler( 0x10, SW_KeyboardClear, SW_KeyboardClear );

	for( counter = 0x11; counter <= 0x1F; counter++ )
	{
		if( counter != 0x19 )
			ram_bus_hw_reset_set_io_handler( counter, SW_MemoryPageStatus, NULL );
	}
	#endif

	for( counter = 0x50; counter <= 0x5F; counter++ )
	{
        ram_bus_hw_reset_set_io_handler( counter, SW_VideoControl, SW_VideoControl );
	}

	sw_switch_reset();
}

void sw_switch_reset( void )
{
	keycode = 0;
	keywaiting = FALSE;

    memset( __sw_switch, 0x00, sizeof( __sw_switch ) );
    SET_SW_SWITCH( SW_REG_SCREEN_COLOUR, 0xF0 );   
}
