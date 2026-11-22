#include "rtl_model.h"
#include "rtl_interface.h"
#include <cstdlib>

// The coordinator owns Verilator eval() and no bus implementation does.
// One CPU access still completes synchronously for the current 65816 core,
// while the MMU bus, SPI link and output capture advance independently.
void rtl_model_init( void )
{
    rtl_interface_init();
}

void rtl_model_reset( void )
{
    p_model->PHI_0 = FALSE;
    p_model->RESET = FALSE;
    p_model->ADDR = 0xFFFF;
    p_model->spi_cs = TRUE;
    p_model->CPU_RW = TRUE;
    p_model->DATA__en = TRUE;

    for( int i = 0; i < 2; ++i )
    {
        p_model->PHI_0 = FALSE;
        p_model->spi_clk = FALSE;
        p_model->eval();

        p_model->PHI_0 = TRUE;
        p_model->spi_clk = TRUE;
        p_model->eval();
    }

    p_model->RESET = TRUE;
    p_model->DMA_N = TRUE;
    p_model->SW0 = FALSE;
    p_model->SW1 = FALSE;
    //p_model->SW2 = FALSE;
    p_model->PDL0 = TRUE;
    p_model->PDL1 = TRUE;
    //p_model->PDL2 = TRUE;
    //p_model->PDL3 = TRUE;
    p_model->MODE_SEL = TRUE;

    p_model->PHI_0 = FALSE;
    p_model->spi_clk = FALSE;
    p_model->eval();
}

// Called after the board clears its software-side CPLD state, so all
// state machines and inactive-high chip selects have deterministic values.
void rtl_model_state_reset( cpld_data_t *cpld_data )
{
    cpld_model_t context = { &cpld_data->MMU, &cpld_data->IOU, p_model };
    memory_bus_reset( &context );
    spi_slave_reset( &context );
}

void rtl_model_iterate( uint32_t address, bool_t cpu_rw, uint8_t data_in,
                        cpld_model_MMU_t *mmu, cpld_model_IOU_t *iou )
{
    cpld_model_t context = { mmu, iou, p_model };
    cpu_bus_request( &context, address, cpu_rw, data_in );

    // The present divider takes 4 low + 4 high + 1 release ticks.
    // Wait on the state machine rather than hard-coding that schedule.
    // The watchdog prevents a regression from silently wedging the CPU.
    const unsigned max_bus_ticks = 32;
    for( unsigned tick = 0; tick < max_bus_ticks &&
                            !memory_bus_is_done( &context ); ++tick )
    {
        cpu_bus_process( &context );
        memory_bus_process( &context );
        spi_slave_process( &context );

        p_model->eval();

        memory_bus_latch( &context );
        spi_slave_latch( &context );
    }

    if( !memory_bus_is_done( &context ) )
        std::abort(); // Never release the CPU before its bus cycle finishes.

    memory_bus_ack( &context );
    ++mmu->clock_counter;
}
