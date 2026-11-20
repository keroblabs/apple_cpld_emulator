#include "rtl_interface.h"

// The 65816 produces a complete bus request before it can be completed.
// Keep the CPU address distinct from the physical address decoded by the MMU.
void cpu_bus_request( cpld_model_t *context, uint32_t address, bool_t cpu_rw, uint8_t data_in )
{
    cpld_model_MMU_t *mmu = context->mmu;

    mmu->cpu_address = address & 0x0FFFF;
    mmu->cpu_rw = cpu_rw;
    mmu->data_in = data_in;
    mmu->data_out = 0;
    memory_bus_request( context );
}

void cpu_bus_process( cpld_model_t *context )
{
    cpld_model_MMU_t *mmu = context->mmu;
    rtl_model_t *rtl = context->p_model;

    rtl->ADDR = mmu->cpu_address;
    rtl->CPU_RW = mmu->cpu_rw;
    rtl->MODE_SEL = TRUE;

    // Verilator splits the bidirectional DATA net into input, output and
    // enable signals. Preserve the established bus-direction convention.
    rtl->DATA__en = mmu->cpu_rw;
    if( !mmu->cpu_rw )
    {
        rtl->DATA = mmu->data_in;
    }
}

// Called only after evaluating the final high PHI_0 phase.
void cpu_bus_capture( cpld_model_t *context )
{
    cpld_model_MMU_t *mmu = context->mmu;
    rtl_model_t *rtl = context->p_model;

    mmu->data_out = 0;
    context->iou->data_bus = 0;

    if( mmu->cpu_rw && !( rtl->DATA_EN_N & 0x01 ) )
    {
        mmu->data_out = rtl->DATA__out;
        context->iou->data_bus = mmu->data_out;
    }
}
