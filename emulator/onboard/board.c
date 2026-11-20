#include "board.h"
#include "apple2rom.h"

#include "rtl_cpu_65816.h"
#include "rtl_video.h"
#include "ram_bus_hw.h"
#include "sw_switch.h"

#include "spi_controller.h"
#include "memory_controller.h"
#include "iomem_controller.h"

#include "rtl_model.h"
#include "input_drv.h"

#include "graphics.h"
#include "draw.h"
#include "timer.h"
#include "buffer.h"

//////////////////////////////////////////////////////////////////////////////////////

uint64_t system_timer_get_us( void );

//////////////////////////////////////////////////////////////////////////////////////

#if 0
const static uint8_t disk_drive_fw[256] =
{
    0xA2,0x20,0xA0,0x00,0xA2,0x03,0x86,0x3C,0x8A,0x0A,0x24,0x3C,0xF0,0x10,0x05,0x3C,
    0x49,0xFF,0x29,0x7E,0xB0,0x08,0x4A,0xD0,0xFB,0x98,0x9D,0x56,0x03,0xC8,0xE8,0x10,
    0xE5,0x20,0x58,0xFF,0xBA,0xBD,0x00,0x01,0x0A,0x0A,0x0A,0x0A,0x85,0x2B,0xAA,0xBD,
    0x8E,0xC0,0xBD,0x8C,0xC0,0xBD,0x8A,0xC0,0xBD,0x89,0xC0,0xA0,0x50,0xBD,0x80,0xC0,
    0x98,0x29,0x03,0x0A,0x05,0x2B,0xAA,0xBD,0x81,0xC0,0xA9,0x56,0x20,0xA8,0xFC,0x88,
    0x10,0xEB,0x85,0x26,0x85,0x3D,0x85,0x41,0xA9,0x08,0x85,0x27,0x18,0x08,0xBD,0x8C,
    0xC0,0x10,0xFB,0x49,0xD5,0xD0,0xF7,0xBD,0x8C,0xC0,0x10,0xFB,0xC9,0xAA,0xD0,0xF3,
    0xEA,0xBD,0x8C,0xC0,0x10,0xFB,0xC9,0x96,0xF0,0x09,0x28,0x90,0xDF,0x49,0xAD,0xF0,
    0x25,0xD0,0xD9,0xA0,0x03,0x85,0x40,0xBD,0x8C,0xC0,0x10,0xFB,0x2A,0x85,0x3C,0xBD,
    0x8C,0xC0,0x10,0xFB,0x25,0x3C,0x88,0xD0,0xEC,0x28,0xC5,0x3D,0xD0,0xBE,0xA5,0x40,
    0xC5,0x41,0xD0,0xB8,0xB0,0xB7,0xA0,0x56,0x84,0x3C,0xBC,0x8C,0xC0,0x10,0xFB,0x59,
    0xD6,0x02,0xA4,0x3C,0x88,0x99,0x00,0x03,0xD0,0xEE,0x84,0x3C,0xBC,0x8C,0xC0,0x10,
    0xFB,0x59,0xD6,0x02,0xA4,0x3C,0x91,0x26,0xC8,0xD0,0xEF,0xBC,0x8C,0xC0,0x10,0xFB,
    0x59,0xD6,0x02,0xD0,0x87,0xA0,0x00,0xA2,0x56,0xCA,0x30,0xFB,0xB1,0x26,0x5E,0x00,
    0x03,0x2A,0x5E,0x00,0x03,0x2A,0x91,0x26,0xC8,0xD0,0xEE,0xE6,0x27,0xE6,0x3D,0xA5,
    0x3D,0xCD,0x00,0x08,0xA6,0x2B,0x90,0xDB,0x4C,0x01,0x08,0x00,0x00,0x00,0x00,0x00,
};
#endif

//////////////////////////////////////////////////////////////////////////////////////

static timer_data_t benchmark_ms;

static rtl_access_t rtl_context;
static rtl_video_t  rtl_video;

static cpld_model_t cpld_model;

//////////////////////////////////////////////////////////////////////////////////////

static void board_benchmark( void )
{
    char debug_temp_string[ 256 ];
    
	if( timer_Check( &benchmark_ms ) == TRUE )
	{
		timer_Reload( &benchmark_ms );

        uint32_t video_benchmark = video_rtl_benchmark_read();
        //uint32_t video_update = video_rtl_rate_read();

        uint32_t cpu_active_time = rtl_cpu_get_active_time( &rtl_context );
        uint32_t cpu_benchmark = rtl_cpu_get_benchmark_cycles( &rtl_context );

        //uint32_t cpu_active_time2 = cpu_core2->get_active_time();
        //uint32_t cpu_benchmark2   = cpu_core2->get_benchmark_cycles();

        sprintf( debug_temp_string, "CPU   = %8d Hz (%d/10K)           ", cpu_benchmark, cpu_active_time );
        draw_text( 20, LCD_HEIGHT - 20, debug_temp_string, APP_RGB(192, 255, 192), APP_RGB(0, 0, 0) );

        //sprintf( debug_temp_string, "CPU2  = %8d Hz (%d/10K)           ", cpu_benchmark2, cpu_active_time2 );
        //draw_text( 20, LCD_HEIGHT - 20, debug_temp_string, APP_RGB(192, 255, 192), APP_RGB(0, 0, 0) );

        sprintf( debug_temp_string, "Video = %8d hz (0)              ", video_benchmark );
        draw_text( 20, LCD_HEIGHT - 10, debug_temp_string, APP_RGB(192, 255, 192), APP_RGB(0, 0, 0) );
	}
}

#if 0
static void board_speaker_control( cpld_model_t * cpld )
{
    if( cpld->IOU.prev_spk != cpld->IOU.spk )
    {
        cpld->IOU.prev_spk = cpld->IOU.spk;
        printf( "%ld = %c\n", cpld->MMU.clock_counter, cpld->IOU.spk ? '*' : '.' );
    }
}
#endif

static void board_paddle_control( cpld_model_t * cpld )
{
    uint8_t paddle_0 = 128;
    uint8_t paddle_1 = 128;
    uint8_t paddle_2 = 100;
    uint8_t paddle_3 = 100;

    if( !cpld->IOU.pdl_strobe )
    {
        cpld->IOU.pdl_strobe = TRUE;

        cpld->IOU.cycles_pdl_0 = cpld->MMU.clock_counter + ( 0x0D * ( uint64_t )( paddle_0 & 0x0FF ) );
        cpld->IOU.cycles_pdl_1 = cpld->MMU.clock_counter + ( 0x0D * ( uint64_t )( paddle_1 & 0x0FF ) );
        cpld->IOU.cycles_pdl_2 = cpld->MMU.clock_counter + ( 0x0D * ( uint64_t )( paddle_2 & 0x0FF ) );
        cpld->IOU.cycles_pdl_3 = cpld->MMU.clock_counter + ( 0x0D * ( uint64_t )( paddle_3 & 0x0FF ) );
    }

    if( ( cpld->IOU.cycles_pdl_0 != 0 ) && ( cpld->MMU.clock_counter > cpld->IOU.cycles_pdl_0 ) ) cpld->IOU.cycles_pdl_0 = 0;
    if( ( cpld->IOU.cycles_pdl_1 != 0 ) && ( cpld->MMU.clock_counter > cpld->IOU.cycles_pdl_1 ) ) cpld->IOU.cycles_pdl_1 = 0;
    if( ( cpld->IOU.cycles_pdl_2 != 0 ) && ( cpld->MMU.clock_counter > cpld->IOU.cycles_pdl_2 ) ) cpld->IOU.cycles_pdl_2 = 0;
    if( ( cpld->IOU.cycles_pdl_3 != 0 ) && ( cpld->MMU.clock_counter > cpld->IOU.cycles_pdl_3 ) ) cpld->IOU.cycles_pdl_3 = 0;
}

static void board_bus_address( rtl_access_t * state, cpld_model_t * cpld )
{
    if( state->cpu_halt == CPU_HALT_NONE )
    {
        board_paddle_control( cpld );
        spi_controller_process( cpld );
        iomem_controller_process();
        //board_keyboard_controller( cpld );
        //board_speaker_control( cpld );

        bool_t is_read = ( rtl_context.instr_state == Instr_State_BusWrite ) ? FALSE : TRUE;
        rtl_model_iterate( rtl_context.addr_req, is_read, state->data_xfer, &cpld->MMU, &cpld->IOU );

        //rtl_model_IOU_prefetch( &cpld->IOU, &cpld->MMU );
        //rtl_model_MMU_clock_setup( rtl_context.addr_req, is_read, &cpld->MMU, &cpld->IOU );
        //rtl_model_IOU_clock_setup( rtl_context.addr_req, state->data_xfer, is_read, &cpld->IOU );

        //rtl_model_MMU_clock_done( &cpld->MMU );
        //rtl_model_IOU_clock_done( is_read, &cpld->IOU );
        state->data_xfer = memory_controller_access( cpld, rtl_context.addr_req, is_read, state->data_xfer );
    }
}

void board_startup( void )
{
    rtl_model_reset();
    memset( &cpld_model, 0x00, sizeof( cpld_model ) );

    memory_controller_init();

    // connect the RTL co-simulation
    rtl_init_65816( &rtl_context );

	// display benchmark
    draw_select_font( FONT_8x8 );
    timer_Start( &benchmark_ms, 1000 );

	// init framework
	timer_Init();

    ram_bus_hw_init();
    sw_switch_init();

    video_rtl_init( &rtl_video, TRUE );

    ram_bus_hw_reset();

    spi_controller_init();
    iomem_controller_init();

    cpld_model.IOU.pdl_strobe_prev = TRUE;
    cpld_model.IOU.pdl_strobe = TRUE;
    cpld_model.IOU.C0XX_N = TRUE;
    cpld_model.IOU.spi_pending = FALSE;
    cpld_model.IOU.spi_state = 0;

    cpld_model.MMU.C0XX_N = TRUE;

    //cpld_model.MMU.kbd_strobe = TRUE;
}

void board_reset( void )
{
    rtl_init_65816( &rtl_context );
    sw_switch_reset();
}

void board_video_execute( void )
{
    video_rtl_dma_pixels( &rtl_video );
    
    if( rtl_video.bus_mode == Video_State_BusRead )
    {
        rtl_video.data_xfer = ram_bus_hw_read_16( rtl_video.addr_req );
    }
    
    video_rtl_dma_complete( &rtl_video );
}

void board_execute( void )
{
    int i;
    //uint64_t total_of_cycles = 0;

    board_benchmark();

    for( i = 0; i < 5; i++ )
    {
        rtl_cpu_execute( &rtl_context ); // start of cycle

        board_bus_address( &rtl_context, &cpld_model );

        rtl_cpu_complete( &rtl_context ); // end of cycle

        memory_controller_tick( &cpld_model, rtl_context.cpu_cycles );
    }
}
