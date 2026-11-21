#include "graphics.h"
#include "draw.h"
#include "rtl_model.h"

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
#include "rom_backing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main( int argc, char **argv )
{
    const char *rom_output_path = NULL;
    if( argc == 3 && strcmp( argv[1], "-w" ) == 0 && argv[2][0] != '\0' )
    {
        rom_output_path = argv[2];
    }
    else if( argc != 1 )
    {
        fprintf( stderr, "Usage: %s [-w IMAGEFILE]\n", argv[0] );
        return EXIT_FAILURE;
    }

    rtl_model_init();
    timer_Init();
    
    graphics_init();
    graphics_init_auto_vsync();

    // before board init, run self-test
    //rtl_self_test_run();

    // initialize board
    input_drv_init();
    board_startup();
    if( rom_output_path )
    {
        if( rom_backing_write_image( rom_output_path ) != 0 )
        {
            fprintf( stderr, "Cannot write ROM image '%s'.\n", rom_output_path );
            lcd_driver_deinit();
            heap_mngr_deinit();
            return EXIT_FAILURE;
        }
        printf( "Wrote ROM image '%s' (%u bytes)\n", rom_output_path, ROM_BACKING_SIZE );
    }
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
    return EXIT_SUCCESS;
}
