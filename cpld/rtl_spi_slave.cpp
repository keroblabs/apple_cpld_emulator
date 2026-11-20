#include "rtl_model.h"
#include "rtl_interface.h"

////////////////////////////////////////////////////////////////////////////////////

enum
{
    SPI_SLAVE_IDLE,
    SPI_SLAVE_SHIFT_OUT,
    SPI_SLAVE_SHIFT_IN,
    SPI_SLAVE_WAIT,
    SPI_SLAVE_CLOSE,
    SPI_SLAVE_DONE
};

////////////////////////////////////////////////////////////////////////////////////

void spi_slave_process( cpld_model_t *cpld_model )
{
    cpld_data_t *cpld_data = (cpld_data_t *)cpld_model->p_data;
    
    switch( cpld_data->IOU.spi_slave_state )
    {
        case SPI_SLAVE_IDLE:
            cpld_model->p_model->spi_cs = TRUE;
            cpld_model->p_model->spi_clk = FALSE;
            cpld_model->p_model->spi_mosi = FALSE;

            cpld_data->IOU.spi_data_rcvr = 0;
            cpld_data->IOU.spi_bit_index = 12;
            break;

        case SPI_SLAVE_SHIFT_OUT:
            cpld_model->p_model->spi_cs = FALSE;
            cpld_model->p_model->spi_clk = FALSE;
            cpld_model->p_model->spi_mosi = ( cpld_data->IOU.spi_data_send >> cpld_data->IOU.spi_bit_index ) & 0x01;

            cpld_data->IOU.spi_bit_index--;

            if( cpld_data->IOU.spi_bit_index == 0 )
            {
                // when reached the last bit, skip the next read.
                cpld_data->IOU.spi_slave_state = SPI_SLAVE_WAIT;
            }

            cpld_data->IOU.spi_slave_state = SPI_SLAVE_SHIFT_IN;
            break;

        case SPI_SLAVE_SHIFT_IN:
            cpld_data->IOU.spi_data_rcvr = ( cpld_data->IOU.spi_data_rcvr << 1 ) | ( cpld_model->p_model->spi_miso & 0x01 );

            cpld_data->IOU.spi_bit_index--;

            if( cpld_data->IOU.spi_bit_index == 0 )
            {
                cpld_data->IOU.spi_slave_state = SPI_SLAVE_WAIT;
            }
            break;

        case SPI_SLAVE_WAIT:
            cpld_model->p_model->spi_cs = FALSE;
            cpld_model->p_model->spi_clk = FALSE;
            cpld_model->p_model->spi_mosi = FALSE;
            cpld_data->IOU.spi_slave_state = SPI_SLAVE_CLOSE;
            break;

        case SPI_SLAVE_CLOSE:
            cpld_model->p_model->spi_cs = TRUE;
            cpld_data->IOU.spi_pending = FALSE;
            cpld_data->IOU.spi_slave_state = SPI_SLAVE_DONE;
            break;

        default:
            cpld_data->IOU.spi_slave_state = SPI_SLAVE_IDLE;
            break;
    }
}

void spi_slave_send_data( cpld_model_t *cpld_model, uint8_t addr, uint8_t data_in )
{
    cpld_data_t *cpld_data = (cpld_data_t *)cpld_model->p_data;
    cpld_data->IOU.spi_data_send = addr;
    cpld_data->IOU.spi_data_send = ( cpld_data->IOU.spi_data_send << 8 ) | data_in;
    cpld_data->IOU.spi_pending = TRUE;
}

bool_t spi_slave_is_done( cpld_model_t *cpld_model )
{
    cpld_data_t *cpld_data = (cpld_data_t *)cpld_model->p_data;
    return ( cpld_data->IOU.spi_slave_state == SPI_SLAVE_DONE );
}

uint16_t spi_slave_get_received_data( cpld_model_t *cpld_model )
{
    cpld_data_t *cpld_data = (cpld_data_t *)cpld_model->p_data;
    uint16_t received_data = cpld_data->IOU.spi_data_rcvr;

    if( cpld_data->IOU.spi_slave_state == SPI_SLAVE_DONE )
    {
        cpld_data->IOU.spi_slave_state = SPI_SLAVE_IDLE;
        cpld_data->IOU.spi_pending = FALSE;
    }

    return received_data;
}

void spi_slave_reset( cpld_model_t *cpld_model )
{
    cpld_data_t *cpld_data = (cpld_data_t *)cpld_model->p_data;
    cpld_data->IOU.spi_pending = FALSE;
    cpld_data->IOU.spi_slave_state = SPI_SLAVE_IDLE;
}
