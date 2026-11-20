#include "input_drv.h"
#include "board.h"
#include "buffer.h"
#include "drivers.h"

//////////////////////////////////////////////////////////////////////////////////////

#define LOCAL_DISASM_OFF            0
#define LOCAL_DISASM_WAITING        1
#define LOCAL_DISASM_ON             2

//////////////////////////////////////////////////////////////////////////////////////

#define MOUSE_DRIVER_APPLEMOUSE     0
#define MOUSE_DRIVER_JOYSTICK       1

//////////////////////////////////////////////////////////////////////////////////////

#define SHIFT_FLAG                  0x01
#define CAPS_FLAG                   0x02
#define CTRL_FLAG                   0x04
#define ALT_FLAG		            0x08
#define ALTGR_FLAG		            0x10
#define WIN_FLAG	        	    0x20
#define CLOSEAPL_FLAG               0x40
#define OPENAPL_FLAG                0x80

//////////////////////////////////////////////////////////////////////////////////////

#define COMMAND_SPECIAL_KEYS        0x01
#define COMMAND_KEYDOWN             0x02
#define COMMAND_KEYUP               0x03
#define COMMAND_MOUSE_DELTA_X       0x04
#define COMMAND_MOUSE_DELTA_Y       0x05
#define COMMAND_MOUSE_BUTTON        0x06
#define COMMAND_PADDLE_BUTTON       0x07
#define COMMAND_PADDLE_0            0x08
#define COMMAND_PADDLE_1            0x09
#define COMMAND_PADDLE_2            0x0A
#define COMMAND_PADDLE_3            0x0B

#define INPUT_DEVICE_KEYBOARD       0x00
#define INPUT_DEVICE_MOUSE          0x01
#define INPUT_DEVICE_PADDLE         0x02

#define COMMAND_PROBE_REQUEST       0x10
#define COMMAND_PROBE_RESP          0x20

#define COMMAND_PROBE_KEYBOARD      ( COMMAND_PROBE_REQUEST | INPUT_DEVICE_KEYBOARD )
#define COMMAND_PROBE_MOUSE         ( COMMAND_PROBE_REQUEST | INPUT_DEVICE_MOUSE )
#define COMMAND_PROBE_PADDLE        ( COMMAND_PROBE_REQUEST | INPUT_DEVICE_PADDLE )

#define KEYBOARD_BUFFER_SIZE        32

//////////////////////////////////////////////////////////////////////////////////////

enum
{
    KEYPAD_STATE_IDLE,
    KEYPAD_STATE_CONTROL_KEY,
    KEYPAD_STATE_KEY_PRESS,
};

enum
{
    MOUSE_STATE_IDLE,
    MOUSE_STATE_GET_VALUE,
};

//////////////////////////////////////////////////////////////////////////////////////

static buffer_data_t keyboard_buffer_data;
static uint8_t keyboard_buffer[ KEYBOARD_BUFFER_SIZE ];

static int8_t  mouse_delta_x;
static int8_t  mouse_delta_y;
static bool_t  mouse_button;
static uint8_t mouse_rx_state;
static uint8_t mouse_command;

static uint8_t paddle_value_0;
static uint8_t paddle_value_1;
static uint8_t paddle_rx_state;
static uint8_t paddle_command;
static bool_t  paddle_button_0;
static bool_t  paddle_centered;

static uint8_t keypad_rx_state;
static uint8_t keypad_command;
static uint8_t keypad_control;
static uint8_t keypad_is_up;
static uint8_t keypad_value;

static bool_t caps_lock;

static bool_t  open_apple_value;
static bool_t  close_apple_value;

//////////////////////////////////////////////////////////////////////////////////////

static void add_keytroke_to_buffer( uint8_t keycode )
{
    if( ( keypad_control & CTRL_FLAG ) && ( ( ( keycode >= 'A' ) && ( keycode <= 'Z' ) ) ||
                                            ( ( keycode >= 'a' ) && ( keycode <= 'z' ) ) ) )
    {
        keycode &= ~0x20;
        keycode -= 'A' - 1;
    }
    else if( ( keypad_control == CAPS_FLAG ) && ( keycode >= 'A' ) && ( keycode <= 'Z' ) )
    {
        keycode |= 0x20;
    }

    buffer_write( &keyboard_buffer_data, keycode );
}

static void joystick_loop( void )
{
    while( mouse_driver_has_data( MOUSE_DRIVER_JOYSTICK ) != 0 )
    {
        int8_t delta;

        switch( paddle_rx_state )
        {
            case MOUSE_STATE_IDLE:
                paddle_command = mouse_driver_read_data( MOUSE_DRIVER_JOYSTICK );
                paddle_rx_state = MOUSE_STATE_GET_VALUE;
                break;

            case MOUSE_STATE_GET_VALUE:
                if( ( paddle_command == 0x04 ) || ( paddle_command == 0x05 ) )
                {
                    delta = ( int8_t )mouse_driver_read_data( MOUSE_DRIVER_JOYSTICK );
                    int input_paddle = ( paddle_command == 0x04 ) ? ( int )paddle_value_0 : ( int )paddle_value_1;
                    int new_value = paddle_centered ? ( ( int )128 + delta ) : ( input_paddle + delta );
                    if( new_value > 255 ) new_value = 255;
                    else if( new_value < 0 ) new_value = 0;

                    if( paddle_command == 0x04 )
                    {
                        paddle_value_0 = ( uint8_t )new_value;
                    }
                    else
                    {
                        paddle_value_1 = ( uint8_t )new_value;
                    }
                }
                else if( paddle_command == 0x06 )
                {
                    paddle_button_0 = ( bool_t )mouse_driver_read_data( MOUSE_DRIVER_JOYSTICK );
                }

                paddle_rx_state = MOUSE_STATE_IDLE;
                break;
        }
    }
}

static void mouse_loop( void )
{
    while( mouse_driver_has_data( MOUSE_DRIVER_APPLEMOUSE ) != 0 )
    {
        switch( mouse_rx_state )
        {
            case MOUSE_STATE_IDLE:
                mouse_command = mouse_driver_read_data( MOUSE_DRIVER_APPLEMOUSE );
                mouse_rx_state = MOUSE_STATE_GET_VALUE;
                break;

            case MOUSE_STATE_GET_VALUE:
                if( mouse_command == 0x04 )
                {
                    mouse_delta_x = ( int8_t )mouse_driver_read_data( MOUSE_DRIVER_APPLEMOUSE );
                }
                else if( mouse_command == 0x05 )
                {
                    mouse_delta_y = ( int8_t )mouse_driver_read_data( MOUSE_DRIVER_APPLEMOUSE );
                }
                else if( mouse_command == 0x06 )
                {
                    mouse_button = ( bool_t )mouse_driver_read_data( MOUSE_DRIVER_APPLEMOUSE );
                }

                mouse_rx_state = MOUSE_STATE_IDLE;
                break;
        }
    }
}

static void keyboard_loop( void )
{
    while( keypad_driver_has_data() != 0 )
    {
        switch( keypad_rx_state )
        {
            case KEYPAD_STATE_IDLE:
                keypad_command = keypad_driver_read_data();
                if( keypad_command == 0x01 )
                {
                    keypad_rx_state = KEYPAD_STATE_CONTROL_KEY;
                }
                else if( ( keypad_command == 0x02 ) || ( keypad_command == 0x03 ) )
                {
                    keypad_rx_state = KEYPAD_STATE_KEY_PRESS;
                }
                break;

            case KEYPAD_STATE_CONTROL_KEY:
                keypad_control = keypad_driver_read_data();
                keypad_rx_state = KEYPAD_STATE_IDLE;
                open_apple_value = ( ( keypad_control & ALT_FLAG ) ? TRUE : FALSE ) | paddle_button_0;
                close_apple_value = ( ( keypad_control & ALTGR_FLAG ) ? TRUE : FALSE );
                caps_lock = ( keypad_control & CAPS_FLAG ) ? TRUE : FALSE;
                break;

            case KEYPAD_STATE_KEY_PRESS:
                keypad_value = keypad_driver_read_data();
                keypad_is_up = ( keypad_command == 0x02 ) ? FALSE : TRUE;
                keypad_rx_state = KEYPAD_STATE_IDLE;

                if( keypad_is_up == FALSE )
                {
                    add_keytroke_to_buffer( keypad_value & 0x07F );
                }
                break;
        }
    }
}

bool_t input_drv_get_mouse_delta( int8_t * delta_x, int8_t * delta_y )
{
    *delta_x = mouse_delta_x;
    *delta_y = mouse_delta_y;
    return mouse_button;
}

bool_t input_drv_get_paddle_values( uint8_t * paddle_0, uint8_t * paddle_1 )
{
    *paddle_0 = paddle_value_0;
    *paddle_1 = paddle_value_1;
    return TRUE;
}

bool_t input_drv_get_open_apple( void )
{
    return open_apple_value;
}

bool_t input_drv_get_close_apple( void )
{
    return close_apple_value;
}

bool_t input_drv_keyboard_has_data( void )
{
    return buffer_is_empty( &keyboard_buffer_data ) ? FALSE : TRUE;
}

uint8_t input_drv_keyboard_read( void )
{
    return buffer_read( &keyboard_buffer_data );
}

void input_drv_process( void )
{
    keyboard_loop();
    mouse_loop();
    joystick_loop();
}

void input_drv_init( void )
{
    buffer_init( &keyboard_buffer_data, keyboard_buffer, KEYBOARD_BUFFER_SIZE );

    keypad_driver_init();
    mouse_driver_init( MOUSE_DRIVER_APPLEMOUSE );
    mouse_driver_init( MOUSE_DRIVER_JOYSTICK );

    uint8_t joystick_config[] = { 0x50, 0x01, 0x00, 0x00, 0x00, 0x00 };
    uint32_t paddle_scale = 32768;
    memcpy( &joystick_config[ 2 ], &paddle_scale, sizeof( paddle_scale ) );
    mouse_driver_write_command( MOUSE_DRIVER_JOYSTICK, joystick_config, sizeof( joystick_config ) );

    caps_lock = FALSE;    
    paddle_centered = TRUE;
}
