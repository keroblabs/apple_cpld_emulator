#include "graphics.h"
#include "draw.h"
#include "rtl_interface.h"

#include "board.h"
#include "sw_switch.h"
#include "input_drv.h"
#include "drivers.h"
#include "t_threads.h"
#include "heap_mngr.h"
#include "timer.h"
//#include "rtl_self_test.h"
#include "sdl_main.h"

#include "keyboard_controller.h"

////////////////////////////////////////////////////////////////////////

static void * video_thread;

////////////////////////////////////////////////////////////////////////

static void video_thread_handler( void )
{
    while( sdl_is_running() )
    {
        board_video_execute();
        //tthread_sleep_usecs( 1 );
    }
}

int main( void )
{
    rtl_interface_init();
    timer_Init();
    
    graphics_init();
    graphics_init_auto_vsync();

    // before board init, run self-test
    //rtl_self_test_run();

    // initialize board
    input_drv_init();
    board_startup();
    keyboard_controller_init();

    video_thread = tthread_create( video_thread_handler );

    while( sdl_is_running() )
    {
        graphics_process();
        board_execute();
        input_drv_process();
        keyboard_controller_process();
        mouse_driver_process();
        //system_Events();
    }

    tthread_sleep_ms( 1000 );
    lcd_driver_deinit();
    heap_mngr_deinit();
}
