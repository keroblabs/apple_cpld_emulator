#include "spi_controller.h"
#include "buffer.h"
#include "rtl_model.h"

//////////////////////////////////////////////////////////////////////////////////////

#define SPI_BUFFER_SIZE     (sizeof( spi_data_t ) * 8)

//////////////////////////////////////////////////////////////////////////////////////

static buffer_data_t spi_buffer_data_tx;
static uint8_t spi_buffer_tx[ SPI_BUFFER_SIZE ];
static buffer_data_t spi_buffer_data_rx;
static uint8_t spi_buffer_rx[ SPI_BUFFER_SIZE ];
static uint8_t spi_id_counter;

//////////////////////////////////////////////////////////////////////////////////////

void spi_controller_init( void )
{
    spi_id_counter = 0;
    buffer_init( &spi_buffer_data_rx, spi_buffer_rx, SPI_BUFFER_SIZE );
    buffer_init( &spi_buffer_data_tx, spi_buffer_tx, SPI_BUFFER_SIZE );
}

uint8_t spi_controller_data_start( uint8_t addr, uint8_t data_in )
{
    spi_data_t spi_req;

    spi_req.id = spi_id_counter;
    spi_req.addr = addr;
    spi_req.data = data_in;
    spi_req.flags = 0;

    spi_id_counter++;

    buffer_write_buffer( &spi_buffer_data_tx, ( uint8_t * )&spi_req, sizeof( spi_data_t ) );

    return spi_req.id;
}

bool_t spi_controller_check_complete( uint8_t id, spi_data_t * spi_resp )
{
    if( buffer_get_size( &spi_buffer_data_rx ) < sizeof( spi_data_t ) )
    {
        return FALSE;
    }

    buffer_read_buffer( &spi_buffer_data_rx, ( uint8_t * )spi_resp, sizeof( spi_data_t ) );

    if( spi_resp->id != id )
    {
        buffer_write_buffer( &spi_buffer_data_rx, ( uint8_t * )spi_resp, sizeof( spi_data_t ) );
        return FALSE;
    }

    return TRUE;
}

void spi_controller_process( void * p_cpld )
{
    cpld_model_t * cpld = ( cpld_model_t * )p_cpld;

    if( cpld->IOU.spi_pending == FALSE )
    {
        if( ( cpld->IOU.spi_state == 0 ) && ( buffer_get_size( &spi_buffer_data_tx ) >= sizeof( spi_data_t ) ) )
        {
            spi_data_t spi_req;

            buffer_read_buffer( &spi_buffer_data_tx, ( uint8_t * )&spi_req, sizeof( spi_data_t ) );
            cpld->IOU.spi_id = spi_req.id;
            cpld->IOU.spi_data_send = spi_req.addr;
            cpld->IOU.spi_data_send = ( cpld->IOU.spi_data_send << 8 ) | spi_req.data;
            cpld->IOU.spi_pending = TRUE;
        }
        else if( cpld->IOU.spi_state == 15 )
        {
            spi_data_t spi_resp;
            spi_resp.id = cpld->IOU.spi_id;
            spi_resp.flags = ( cpld->IOU.spi_data_rcvr >> 8 ) & 0x1F;
            spi_resp.data = cpld->IOU.spi_data_rcvr & 0x0FF;

            buffer_write_buffer( &spi_buffer_data_rx, ( uint8_t * )&spi_resp, sizeof( spi_data_t ) );
            cpld->IOU.spi_state = 0;
        }
    }
}
