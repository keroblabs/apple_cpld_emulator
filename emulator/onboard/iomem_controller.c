#include "keyboard_controller.h"
#include "spi_controller.h"
#include "board.h"
#include "input_drv.h"

//////////////////////////////////////////////////////////////////////////////////////

#define IOMEM_COMMAND_TIMEOUT       100
#define IOMEM_TRANSMIT_TIMEOUT      100

#define IOMEM_INVALID_COMMAND       0x55
#define IOMEM_COMMAND_ACK           0x60

#define IOMEM_STATE_RECEIVE         100
#define IOMEM_STATE_TRANSMIT        150
#define IOMEM_MAX_REQUEST           (512 + 16)

//////////////////////////////////////////////////////////////////////////////////////

typedef uint16_t (*start_func_t)( uint8_t command );
typedef void     (*data_rx_func_t)( uint8_t * data, uint16_t size );
typedef bool_t   (*data_tx_func_t)( uint8_t * data );

typedef struct iomem_cmd_t_
{
    uint8_t command;
    start_func_t start_func;
    data_rx_func_t data_rx_func;
    data_tx_func_t data_tx_func;

    struct iomem_cmd_t_ * p_next;

} iomem_cmd_t;

//////////////////////////////////////////////////////////////////////////////////////

static uint8_t iomem_state;
static uint8_t next_state;
static uint8_t iomem_data;

static bool_t has_data;
static bool_t tx_timeout;

static uint16_t iomem_watchdog;
static uint16_t iomem_tx_timeout;
static uint8_t spi_id;

static uint8_t iomem_request[ IOMEM_MAX_REQUEST ];
static uint16_t iomem_req_index;
static uint16_t iomem_req_size;

static iomem_cmd_t * iomem_cmd_list;
static iomem_cmd_t * curr_iomem;

//////////////////////////////////////////////////////////////////////////////////////

#define RETURN_NEXT_STATE                   ( iomem_state + 1 )
#define RETURN_SAME_STATE                   ( iomem_state )
#define RETURN_RESTART                      ( 0 )

#define IOMEM_CHECK_INCOMING_DATA(n)        next_state = n; iomem_state = IOMEM_STATE_RECEIVE
#define IOMEM_SEND_DATA(d,n)                next_state = n, iomem_state = IOMEM_STATE_TRANSMIT; iomem_data = d

//////////////////////////////////////////////////////////////////////////////////////

static iomem_cmd_t * get_iomem_cmd( uint8_t command )
{
    iomem_cmd_t * search_iomem = iomem_cmd_list;

    while( search_iomem != NULL )
    {
        if( search_iomem->command == command )
        {
            break;
        }

        search_iomem = search_iomem->p_next;
    }

    return search_iomem;
}

void iomem_controller_init( void )
{
    iomem_state = 0;
    iomem_cmd_list = NULL;
}

void iomem_controller_process( void )
{
    spi_data_t spi_resp;

    switch( iomem_state )
    {
        case 0:
            IOMEM_CHECK_INCOMING_DATA( RETURN_NEXT_STATE );
            break;

        case 1:
            if( has_data == TRUE )
            {
                iomem_watchdog = IOMEM_COMMAND_TIMEOUT;
                iomem_req_index = 0;

                curr_iomem = get_iomem_cmd( iomem_data );
                if( curr_iomem == NULL )
                {
                    // unknown command
                    printf( "Unknown 0x%02x\n", iomem_data );
                    IOMEM_SEND_DATA( IOMEM_INVALID_COMMAND, RETURN_RESTART );
                }
                else
                {
                    // valid command
                    IOMEM_SEND_DATA( IOMEM_COMMAND_ACK, RETURN_NEXT_STATE );
                }
            }
            else
            {
                iomem_state--;
            }
            break;

        case 2:
            if( tx_timeout == TRUE )
            {
                iomem_state = 0;
            }
            else
            {
                // iomem_data has the command
                iomem_req_size = curr_iomem->start_func( iomem_data );
                iomem_request[ iomem_req_index++ ] = iomem_data;

                next_state = iomem_state + 1;
                iomem_state = IOMEM_STATE_RECEIVE;
            }
            break;

        case 3:
            if( has_data == TRUE )
            {
                // following bytes of parameters
                iomem_watchdog = IOMEM_COMMAND_TIMEOUT;
                iomem_request[ iomem_req_index++ ] = iomem_data;
                
                if( iomem_req_index >= iomem_req_size )
                {
                    curr_iomem->data_rx_func( iomem_request, iomem_req_size );

                    if( curr_iomem->data_tx_func( &iomem_data ) )
                    {
                        // start sending the response
                        next_state = iomem_state + 1;
                        iomem_state = IOMEM_STATE_TRANSMIT;
                    }
                    else 
                    {
                        iomem_state = 0;
                    }
                }
                else
                {
                    next_state = iomem_state;
                    iomem_state = IOMEM_STATE_RECEIVE;
                }
            }
            else if( iomem_watchdog == 0 )
            {
                iomem_state = 0;
            }
            else
            {
                iomem_watchdog--;
                next_state = iomem_state;
                iomem_state = IOMEM_STATE_RECEIVE;
            }
            break;

        case 4:
            if( tx_timeout == TRUE )
            {
                iomem_state = 0;
            }
            else if( curr_iomem->data_tx_func( &iomem_data ) )
            {
                next_state = iomem_state;
                iomem_state = IOMEM_STATE_TRANSMIT;
            }
            else 
            {
                iomem_state = 0;
            }
            break;

        case IOMEM_STATE_RECEIVE:
            spi_id = spi_controller_data_start( SPI_REGISTER_NONE, 0 );
            iomem_state++;            
            break;

        case IOMEM_STATE_RECEIVE + 1:
            if( spi_controller_check_complete( spi_id, &spi_resp ) )
            {
                if( spi_resp.flags & ( 1 << SPI_REGISTER_APL2CTRL ) )
                {
                    iomem_state++;
                }
                else
                {
                    has_data = FALSE;
                    iomem_state = next_state;
                }
            }
            break;

        case IOMEM_STATE_RECEIVE + 2:
            spi_id = spi_controller_data_start( SPI_REGISTER_APL2CTRL, 0 );
            iomem_state++;
            break;

        case IOMEM_STATE_RECEIVE + 3:
            if( spi_controller_check_complete( spi_id, &spi_resp ) )
            {
                has_data = TRUE;
                iomem_data = spi_resp.data;
                iomem_state = next_state;
            }
            break;

        case IOMEM_STATE_TRANSMIT:
            spi_id = spi_controller_data_start( SPI_REGISTER_CTRL2APL, iomem_data );
            tx_timeout = FALSE;
            iomem_state++;
            break;

        case IOMEM_STATE_TRANSMIT + 1:
            if( spi_controller_check_complete( spi_id, &spi_resp ) )
            {
                iomem_state++;
                iomem_tx_timeout = IOMEM_TRANSMIT_TIMEOUT;
            }
            break;

        case IOMEM_STATE_TRANSMIT + 2:
            spi_id = spi_controller_data_start( SPI_REGISTER_NONE, 0 );
            iomem_state++;            
            break;

        case IOMEM_STATE_TRANSMIT + 3:
            if( spi_controller_check_complete( spi_id, &spi_resp ) )
            {
                if( !( spi_resp.flags & ( 1 << SPI_REGISTER_CTRL2APL ) ) )
                {
                    tx_timeout = FALSE;
                    iomem_state = next_state;
                }
                else if( iomem_tx_timeout == 0 )
                {
                    tx_timeout = TRUE;
                    iomem_state = next_state;
                }
                else
                {
                    iomem_tx_timeout--;
                    iomem_state--;
                }
            }
            break;
        
    }
}
