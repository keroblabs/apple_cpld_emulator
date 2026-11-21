#include "rtl_interface.h"

// Each state is processed by one call to the central Verilator scheduler.
// Clock phases are deliberately explicit rather than hiding eval() in here.
enum
{
    BUS_STATE_IDLE = 0,
    BUS_STATE_PHASE_0,
    BUS_STATE_PHASE_1,
    BUS_STATE_RELEASE
};

enum
{
    BUS_XFER_PHASE_IDLE = 0,
    BUS_XFER_PHASE_REQ,
    BUS_XFER_PHASE_PROCESS,
    BUS_XFER_PHASE_DONE
};

#define PHASE_CLOCK_DIVIDER 4

void memory_bus_process( cpld_model_t *context )
{
    cpld_model_MMU_t *mmu = context->mmu;

    switch( mmu->bus_state )
    {
        case BUS_STATE_IDLE:
            context->p_model->PHI_0 = FALSE;
            break;

        case BUS_STATE_PHASE_0:
            context->p_model->PHI_0 = FALSE;
            mmu->bus_xfer_phase = BUS_XFER_PHASE_PROCESS;
            if( ++mmu->clock_div >= PHASE_CLOCK_DIVIDER )
            {
                mmu->clock_div = 0;
                mmu->bus_state = BUS_STATE_PHASE_1;
            }
            break;

        case BUS_STATE_PHASE_1:
            context->p_model->PHI_0 = TRUE;
            if( ++mmu->clock_div >= PHASE_CLOCK_DIVIDER )
            {
                mmu->clock_div = 0;
                mmu->bus_state = BUS_STATE_RELEASE;
            }
            break;

        case BUS_STATE_RELEASE:
            // Settle the falling edge before handing the cycle to the CPU.
            context->p_model->PHI_0 = FALSE;
            mmu->bus_state = BUS_STATE_IDLE;
            mmu->bus_xfer_phase = BUS_XFER_PHASE_DONE;
            break;

        default:
            mmu->bus_state = BUS_STATE_IDLE;
            break;
    }
}

// During RELEASE the last high phase has already been driven, but the
// falling edge has not yet been evaluated. Capture the translated address
// and device outputs once, after eval() of that last high phase.
void memory_bus_latch( cpld_model_t *context )
{
    if( context->mmu->bus_state == BUS_STATE_RELEASE &&
        context->mmu->bus_xfer_phase == BUS_XFER_PHASE_PROCESS )
    {
        cpu_bus_capture( context );
        rtl_interconnect_capture( context );
    }
}

void memory_bus_reset( cpld_model_t *context )
{
    cpld_model_MMU_t *mmu = context->mmu;
    mmu->clock_div = 0;
    mmu->bus_state = BUS_STATE_IDLE;
    mmu->bus_xfer_phase = BUS_XFER_PHASE_IDLE;
    mmu->cpu_address = 0;
    mmu->cpu_rw = FALSE;
    mmu->data_in = 0;
    mmu->data_out = 0;
    mmu->mem_address = 0;
    mmu->IOSEL_N = TRUE;
    mmu->IOSTB_N = TRUE;
    mmu->DEVSEL_N = TRUE;
    mmu->rom_en_n = TRUE;
    mmu->ram_en_n = TRUE;
    mmu->bus_rd_n = TRUE;
    mmu->bus_wr_n = TRUE;
}

void memory_bus_request( cpld_model_t *context )
{
    cpld_model_MMU_t *mmu = context->mmu;
    mmu->clock_div = 0;
    mmu->bus_state = BUS_STATE_PHASE_0;
    mmu->bus_xfer_phase = BUS_XFER_PHASE_REQ;
}

bool_t memory_bus_is_done( cpld_model_t *context )
{
    return context->mmu->bus_xfer_phase == BUS_XFER_PHASE_DONE;
}

void memory_bus_ack( cpld_model_t *context )
{
    if( memory_bus_is_done( context ) )
    {
        context->mmu->bus_xfer_phase = BUS_XFER_PHASE_IDLE;
    }
}
