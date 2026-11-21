#include "rtl_interface.h"

// Four scheduler ticks per SPI half-clock, matching the PHI_0 half-period.
// This keeps the serial link independent from a CPU request's control flow.
#define SPI_HALF_PERIOD_TICKS 4

void spi_slave_process( cpld_model_t *context )
{
    cpld_model_IOU_t *iou = context->iou;
    rtl_model_t *rtl = context->p_model;

    if( iou->spi_slave_state == SPI_SLAVE_STATE_IDLE && iou->spi_pending )
    {
        iou->spi_data_rcvr = 0;
        iou->spi_bit_index = 12; // 5 command/status bits + 8 data bits
        iou->spi_clock_div = 0;
        iou->spi_miso_latched = 0;
        iou->spi_slave_state = SPI_SLAVE_STATE_RESET_LOW;
    }

    switch( iou->spi_slave_state )
    {
        case SPI_SLAVE_STATE_RESET_LOW:
        case SPI_SLAVE_STATE_IDLE:
        case SPI_SLAVE_STATE_DONE:
            rtl->spi_cs = TRUE;
            rtl->spi_clk = FALSE;
            rtl->spi_mosi = FALSE;
            break;

        case SPI_SLAVE_STATE_RESET_HIGH:
            // IOU_spi_iface loads the status shift register on a rising
            // SPI clock with CS deasserted; CS alone is not sufficient.
            rtl->spi_cs = TRUE;
            rtl->spi_clk = TRUE;
            rtl->spi_mosi = FALSE;
            break;

        case SPI_SLAVE_STATE_SHIFT_LOW:
            rtl->spi_cs = FALSE;
            rtl->spi_clk = FALSE;
            rtl->spi_mosi = ( iou->spi_data_send >> iou->spi_bit_index ) & 1;
            break;

        case SPI_SLAVE_STATE_SHIFT_HIGH:
            rtl->spi_cs = FALSE;
            rtl->spi_clk = TRUE;
            rtl->spi_mosi = ( iou->spi_data_send >> iou->spi_bit_index ) & 1;
            break;

        case SPI_SLAVE_STATE_WAIT_LOW:
            rtl->spi_cs = FALSE;
            rtl->spi_clk = FALSE;
            rtl->spi_mosi = FALSE;
            break;

        case SPI_SLAVE_STATE_WAIT_HIGH:
            // Retain the legacy trailing SPI clock.
            rtl->spi_cs = FALSE;
            rtl->spi_clk = TRUE;
            rtl->spi_mosi = FALSE;
            break;

        case SPI_SLAVE_STATE_CLOSE:
            rtl->spi_cs = TRUE;
            rtl->spi_clk = FALSE;
            rtl->spi_mosi = FALSE;
            break;

        default:
            iou->spi_slave_state = SPI_SLAVE_STATE_IDLE;
            break;
    }
}

// Called after eval(), so MISO and edge-triggered Verilog outputs are valid.
// Capture MISO during the low phase, BEFORE the RTL advances its shift
// register on the rising edge. Do not drop the thirteenth response bit.
void spi_slave_latch( cpld_model_t *context )
{
    cpld_model_IOU_t *iou = context->iou;
    const uint8_t state = iou->spi_slave_state;

    if( state == SPI_SLAVE_STATE_IDLE || state == SPI_SLAVE_STATE_DONE )
        return;

    if( ++iou->spi_clock_div < SPI_HALF_PERIOD_TICKS )
        return;

    iou->spi_clock_div = 0;
    switch( state )
    {
        case SPI_SLAVE_STATE_RESET_LOW:
            iou->spi_slave_state = SPI_SLAVE_STATE_RESET_HIGH;
            break;

        case SPI_SLAVE_STATE_RESET_HIGH:
            iou->spi_slave_state = SPI_SLAVE_STATE_SHIFT_LOW;
            break;

        case SPI_SLAVE_STATE_SHIFT_LOW:
            iou->spi_miso_latched = context->p_model->spi_miso & 1;
            iou->spi_slave_state = SPI_SLAVE_STATE_SHIFT_HIGH;
            break;

        case SPI_SLAVE_STATE_SHIFT_HIGH:
            iou->spi_data_rcvr = ( iou->spi_data_rcvr << 1 ) | iou->spi_miso_latched;
            if( iou->spi_bit_index == 0 )
            {
                iou->spi_slave_state = SPI_SLAVE_STATE_WAIT_LOW;
            }
            else
            {
                --iou->spi_bit_index;
                iou->spi_slave_state = SPI_SLAVE_STATE_SHIFT_LOW;
            }
            break;

        case SPI_SLAVE_STATE_WAIT_LOW:
            iou->spi_slave_state = SPI_SLAVE_STATE_WAIT_HIGH;
            break;

        case SPI_SLAVE_STATE_WAIT_HIGH:
            iou->spi_slave_state = SPI_SLAVE_STATE_CLOSE;
            break;

        case SPI_SLAVE_STATE_CLOSE:
            // Completion remains observable until the queue consumes it.
            iou->spi_slave_state = SPI_SLAVE_STATE_DONE;
            break;
    }
}

void spi_slave_send_data( cpld_model_t *context, uint8_t addr, uint8_t data_in )
{
    cpld_model_IOU_t *iou = context->iou;
    if( iou->spi_pending || iou->spi_slave_state != SPI_SLAVE_STATE_IDLE )
        return;

    iou->spi_data_send = ( (uint16_t)addr << 8 ) | data_in;
    iou->spi_pending = TRUE;
}

bool_t spi_slave_is_done( cpld_model_t *context )
{
    return context->iou->spi_slave_state == SPI_SLAVE_STATE_DONE;
}

uint16_t spi_slave_get_received_data( cpld_model_t *context )
{
    cpld_model_IOU_t *iou = context->iou;
    uint16_t received = iou->spi_data_rcvr;

    if( spi_slave_is_done( context ) )
    {
        iou->spi_pending = FALSE;
        iou->spi_slave_state = SPI_SLAVE_STATE_IDLE;
    }
    return received;
}

void spi_slave_reset( cpld_model_t *context )
{
    cpld_model_IOU_t *iou = context->iou;
    iou->spi_pending = FALSE;
    iou->spi_slave_state = SPI_SLAVE_STATE_IDLE;
    iou->spi_clock_div = 0;
    iou->spi_miso_latched = 0;
    iou->spi_data_rcvr = 0;
    iou->spi_bit_index = 0;
}
