#ifndef _SW_SWITCH_H__
#define _SW_SWITCH_H__

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////////

#include "board.h"

//////////////////////////////////////////////////////////////////////////////////////

enum
{
	SW_80STORE,
	SW_RAMRD,
	SW_RAMWR,
	SW_ALTZP,
	SW_80COL,
	SW_SLOTXROM,
	SW_SLOT3ROM,
	SW_CHARSET,
	SW_TEXT,
	SW_MIXED,
	SW_PAGE2,
	SW_HIRES,
	SW_AN0,
	SW_AN1,
	SW_AN2,
	SW_AN3,
	SW_EXP16RAM,
	SW_EXP16WRITE,
	SW_EXP16BANK,
    SW_REG_MEMORY,
    SW_SYSTEM_ADDR,

    SW_REG_MONOCOLOR,
    SW_REG_TBCOLOR,
    SW_REG_VGCINT,
    SW_REG_NEWVIDEO,
    SW_REG_LANGSEL,
    SW_REG_SLOTROMSEL,
    SW_REG_SCANINT,
    SW_REG_CLOCKCTRL,
    SW_REG_SHADOW,
    SW_REG_CYAREG,
    SW_REG_STATEREG,
    
    SW_REG_SPEED,
    SW_REG_SCREEN_COLOUR,
    SW_REG_NEW_SCREEN,
    
	SW_MAXINDEX
};

//////////////////////////////////////////////////////////////////////////////////////

#define REG_MONOCOLOR_COLOR         ( 1 << 7 )

#define REG_VGCINT_INTEN_ON         ( 1 << 7 )
#define REG_VGCINT_1SECINT_ON       ( 1 << 6 )
#define REG_VGCINT_SCANINT_ON       ( 1 << 5 )
#define REG_VGCINT_EXTINT_ON        ( 1 << 4 )
#define REG_VGCINT_1SECINT_EN       ( 1 << 2 )
#define REG_VGCINT_SCANINT_EN       ( 1 << 1 )
#define REG_VGCINT_EXTINT_EN        ( 1 << 0 )

#define REG_NEWVIDEO_SUPER          ( 1 << 7 )
#define REG_NEWVIDEO_LINEAR         ( 1 << 6 )
#define REG_NEWVIDEO_HRESBW         ( 1 << 5 )
#define REG_NEWVIDEO_BANK1          ( 1 << 0 )

#define REG_SCANINT_CLEAR_1SEC      ( 1 << 6 )
#define REG_SCANINT_CLEAR_SCANINT   ( 1 << 5 )

#define REG_SHADOW_IO_LC            ( 1 << 6 )
#define REG_SHADOW_LORES2           ( 1 << 5 )
#define REG_SHADOW_HIRES_AUX        ( 1 << 4 )
#define REG_SHADOW_SUPER_HIRES      ( 1 << 3 )
#define REG_SHADOW_HIRES2           ( 1 << 2 )
#define REG_SHADOW_HIRES1           ( 1 << 1 )
#define REG_SHADOW_LORES1           ( 1 << 0 )

#define REG_SPEED_CPU_SPEED         ( 1 << 7 )
#define REG_SPEED_POWER_UP          ( 1 << 6 )
#define REG_SPEED_SHADOW_BANKS      ( 1 << 4 )
#define REG_SPEED_MOTOR3_DET        ( 1 << 3 )
#define REG_SPEED_MOTOR2_DET        ( 1 << 2 )
#define REG_SPEED_MOTOR1_DET        ( 1 << 1 )
#define REG_SPEED_MOTOR0_DET        ( 1 << 0 )

#define REG_STATEREG_ALTZP          ( 1 << 7 )
#define REG_STATEREG_PAGE2          ( 1 << 6 )
#define REG_STATEREG_RAMRD          ( 1 << 5 )
#define REG_STATEREG_RAMRW          ( 1 << 4 )
#define REG_STATEREG_EXPROM         ( 1 << 3 )
#define REG_STATEREG_EXPBANK        ( 1 << 2 )
#define REG_STATEREG_SLOTXROM       ( 1 << 0 )

#define CORE2_RESET_REG_BASE        ( 1 << 5 )
#define CORE1_RESET_REG_BASE        ( 1 << 4 )
#define CORE2_ENABLED_REG_BASE      ( 1 << 3 )
#define CORE1_ENABLED_REG_BASE      ( 1 << 2 )
#define BOOTLOADER_REG_BASE         ( 1 << 1 )
#define MEMPAGE_REG_BASE            ( 1 << 0 )

//////////////////////////////////////////////////////////////////////////////////////

extern uint8_t __sw_switch[];

#define SW_SWITCH(a)                    ( __sw_switch[ a ] )

#define SET_SW_SWITCH(a,s)              __sw_switch[ a ] = ( s )
#define SW_SWITCH_SET_BIT(a,b)          __sw_switch[ a ] |= ( b )
#define SW_SWITCH_CLEAR_BIT(a,b)        __sw_switch[ a ] &= ~( b )

//////////////////////////////////////////////////////////////////////////////////////

void sw_switch_init( void );
void sw_switch_reset( void );
void sw_switch_set_keycode( uint8_t new_keycode );

//////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif // _SW_SWITCH_H__
