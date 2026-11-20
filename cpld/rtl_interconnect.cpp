#include "rtl_interface.h"

// The combined Verilog top already connects IOU switches into the MMU.
// Only project the externally visible signals into the emulator's bus state.
void rtl_interconnect_capture( cpld_model_t *context )
{
    cpld_model_MMU_t *mmu = context->mmu;
    cpld_model_IOU_t *iou = context->iou;
    rtl_model_t *rtl = context->p_model;

    // The default testbench exports upper address bits [16:12], not the
    // entire address (unless FULL_MMU_ADDR is enabled at Verilator build).
#ifdef FULL_MMU_ADDR
    mmu->mem_address = rtl->MEM_ADDR_OUT & 0x1FFFF;
#else
    mmu->mem_address = ( ( rtl->MEM_ADDR_OUT & 0x01F ) << 12 ) |
                       ( mmu->cpu_address & 0x0FFF );
#endif

    mmu->IOSEL_N = rtl->IOSEL_OUT_N;
    mmu->IOSTB_N = rtl->IOSTB_N;
    mmu->DEVSEL_N = rtl->DEVSEL_n;
    mmu->rom_en_n = rtl->ROM_EN_N;
    mmu->ram_en_n = rtl->RAM_EN_N;

    iou->devsel_3_n = rtl->C0BX_N;
    iou->devsel_6_n = rtl->C0EX_N;
    iou->iosel_6_n = rtl->C6XX_N;
    iou->spk = rtl->SPK_OUT;
    iou->casout = rtl->CAS_OUT;
}
