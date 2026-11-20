#include "keyboard_controller.h"
#include "spi_controller.h"
#include "board.h"
#include "input_drv.h"

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t keyboard_state;
static uint8_t spi_id;

//////////////////////////////////////////////////////////////////////////////////////

void keyboard_controller_init( void )
{
    keyboard_state = 0;
}

void keyboard_controller_process( void )
{
    uint8_t pending_keycode;
    spi_data_t spi_resp;

    switch( keyboard_state )
    {
        case 0:
            if( input_drv_keyboard_has_data() )
            {
                pending_keycode = input_drv_keyboard_read();
                spi_id = spi_controller_data_start( SPI_REGISTER_KEYBOARD, pending_keycode );
                keyboard_state++;
            }
            break;

        case 1:
            if( spi_controller_check_complete( spi_id, &spi_resp ) )
            {
                keyboard_state++;
            }
            break;

        case 2:
            spi_id = spi_controller_data_start( SPI_REGISTER_NONE, 0 );
            keyboard_state++;            
            break;

        case 3:
            if( spi_controller_check_complete( spi_id, &spi_resp ) )
            {
                if( spi_resp.flags & ( 1 << SPI_REGISTER_KEYBOARD ) )
                {
                    keyboard_state = 2;
                }
                else
                {
                    keyboard_state = 0;
                }
            }
            break;
    }
}
