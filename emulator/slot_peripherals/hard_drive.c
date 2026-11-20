#include "hard_drive.h"
#include "filesys_drv.h"
#include "board.h"
#include "timer.h"

/////////////////////////////////////////////////////////////////////////////////////

#define COMMAND_NONE                0
#define COMMAND_GET_SIZE            1
#define COMMAND_SET_BLOCK_NUM       2
#define COMMAND_READ_BLOCK          3
#define COMMAND_WRITE_BLOCK         4
#define COMMAND_CLEAR_BLOCK         5
#define COMMAND_READ_BLOCK_00       6

#define COMMAND_RESET_FIFO          19
#define COMMAND_GET_RESPONSE        20

/////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_HDD_IMAGE_D1        "hdd/hdd_01.bin"
//#define DEFAULT_HDD_IMAGE_D2        "hdd/apple_desktop.bin"
//#define DEFAULT_HDD_IMAGE_D1        "hdd/hdd_02.bin"
//#define DEFAULT_HDD_IMAGE_D2        "hdd/hdd_prodos_src.bin"
#define DEFAULT_HDD_IMAGE_D2        "hdd/hdd_geos.bin"
//#define DEFAULT_HDD_IMAGE_D2        "inbox/ProgramWriterv2.bin"

#define BLOCK_BUFFER_SIZE           512

/////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

static char hdd_handler_0[256];
static char hdd_handler_1[256];
static char * hdd_handler;

static uint16_t fifo_index = 0;
static uint8_t fifo_buffer[BLOCK_BUFFER_SIZE];
static uint16_t fifo_block_num;

static bool_t command_flag;
static bool_t latch_read_flag;
static bool_t latch_write_flag;
static uint8_t latch_read;
static uint8_t latch_write;
static uint8_t latch_command;

//////////////////////////////////////////////////////////////////////////////////////

static uint16_t get_unit_size( uint8_t unit )
{
    hdd_handler = ( unit & 0x80 ) ? hdd_handler_1 : hdd_handler_0;

    uint32_t max_block = filesys_drv_image_access( hdd_handler, Disc_Mode_Size, 0, NULL, 0 ) >> 9;
    return ( uint16_t )max_block & 0x0FFFF;
}

static void instant_block_read( uint8_t unit, uint16_t block_num, uint8_t * buffer )
{
    hdd_handler = ( unit & 0x80 ) ? hdd_handler_1 : hdd_handler_0;
    filesys_drv_image_access( hdd_handler, Disc_Mode_Read, block_num << 9, buffer, BLOCK_BUFFER_SIZE );
}

static void instant_block_write( uint8_t unit, uint16_t block_num, uint8_t * buffer )
{
    hdd_handler = ( unit & 0x80 ) ? hdd_handler_1 : hdd_handler_0;
    filesys_drv_image_access( hdd_handler, Disc_Mode_Write, block_num << 9, buffer, BLOCK_BUFFER_SIZE );
}

static void hdd_io_latch_process( void )
{
    uint16_t max_block;
    uint8_t param = latch_command & 0xF0;
    
    switch( latch_command & 0x0F)
    {
        case COMMAND_NONE:
            if( latch_write_flag == TRUE )
            {
                fifo_buffer[fifo_index] = latch_write;
                latch_write_flag = FALSE;
                if( ++fifo_index >= BLOCK_BUFFER_SIZE ) fifo_index = 0;
            }
            break;
            
        case COMMAND_GET_SIZE:
            max_block = get_unit_size( param & 0x80 );
            memcpy( fifo_buffer, &max_block, sizeof( max_block ) );
            latch_command = COMMAND_GET_RESPONSE;
            command_flag = FALSE;
            break;
            
        case COMMAND_SET_BLOCK_NUM:
            memcpy( &fifo_block_num, fifo_buffer, sizeof( fifo_block_num ) );
            fifo_buffer[0] = 0; // use this as error code
            latch_command = COMMAND_GET_RESPONSE;
            command_flag = FALSE;
            break;
            
        case COMMAND_READ_BLOCK_00:
            fifo_block_num = 0;
            /* FALLTHROUGH */

        case COMMAND_READ_BLOCK:
            instant_block_read( param & 0x80, fifo_block_num, fifo_buffer );
            latch_command = COMMAND_GET_RESPONSE;
            command_flag = FALSE;
            //trigger_led( param );
            break;

        case COMMAND_WRITE_BLOCK:
            instant_block_write( param & 0x80, fifo_block_num, fifo_buffer );
            latch_command = COMMAND_NONE;
            command_flag = FALSE;
            //trigger_led( param );
            break;

        case COMMAND_CLEAR_BLOCK:
            memset( fifo_buffer, 0x00, BLOCK_BUFFER_SIZE );
            instant_block_write( param ? 0x80 : 0x00, fifo_block_num, fifo_buffer );
            latch_command = COMMAND_GET_RESPONSE;
            command_flag = FALSE;
            //trigger_led( param );
            break;
            
        case COMMAND_GET_RESPONSE:
            if( latch_read_flag == FALSE )
            {
                latch_read = fifo_buffer[ fifo_index ];
                latch_read_flag = TRUE;
                if( ++fifo_index >= BLOCK_BUFFER_SIZE ) fifo_index = 0;
            }
            break;

        case COMMAND_RESET_FIFO:
            if( command_flag == TRUE )
            {
                latch_command = COMMAND_NONE;
                command_flag = FALSE;
            }
            break;
    }
}

uint8_t hdd_io_controller( uint8_t address, uint8_t data, bool_t is_write )
{
    uint8_t ret8 = 0;
    //uint8_t * dma_pointer;
    
    switch( address & 0x0F )
    {
        case 0x00: latch_write = data;      break;
        case 0x01: latch_write_flag = TRUE; break;
        case 0x02: ret8 = latch_read;       break;
        case 0x03: latch_read_flag = FALSE; break;

        case 0x04:
            // wait for the controller to take the command
            ret8 = ( command_flag == TRUE ) ? 0x80 : 0x00;
            hdd_io_latch_process();
            break;

        case 0x05:
            // wait for the controller to take the written data
            ret8 = ( latch_write_flag == TRUE ) ? 0x80 : 0x00;
            hdd_io_latch_process();
            break;

        case 0x06:
            // wait for the controller to load something to read
            ret8 = ( latch_read_flag == TRUE ) ? 0x80 : 0x00;
            hdd_io_latch_process();
            break;
            
        case 0x07:
            // command the controller
            latch_command = data;
            fifo_index = 0;
            command_flag = TRUE;
            latch_write_flag = FALSE;
            latch_read_flag = FALSE;
            break;
    }
    
    return ret8;
}

void hdd_drive_init( void )
{
	strcpy( hdd_handler_0, DEFAULT_HDD_IMAGE_D1 );
	strcpy( hdd_handler_1, DEFAULT_HDD_IMAGE_D2 );

	hdd_handler = hdd_handler_0;
}
