#include "rtl_interface.h"

// The combined Verilog top already connects IOU switches into the MMU.
// Only project the externally visible signals into the emulator's bus state.
void rtl_interconnect_capture( cpld_model_t *context )
{
    cpld_model_MMU_t *mmu = context->mmu;
    cpld_model_IOU_t *iou = context->iou;
    rtl_model_t *rtl = context->p_model;

    // Reconstruct the lower bits for the 5-pin MMU. A slot IOSEL page uses
    // only A7..A0. C6XX_N also decodes internal Apple ROM at C6xx, where the
    // full CPU offset is needed.
#ifdef FULL_MMU_ADDR
    mmu->mem_address = rtl->MEM_ADDR_OUT;
#else
    const bool slot_entry = mmu->cpu_rw &&
                            ( !rtl->IOSEL_OUT_N || ( rtl->ROM_EN_N && !rtl->C6XX_N ) );
    const bool address_valid = !rtl->RAM_EN_N || !rtl->ROM_EN_N || slot_entry ||
                               ( mmu->cpu_rw && !rtl->IOSTB_N );
    const uint32_t offset_mask = slot_entry ? 0x00FF : 0x0FFF;
    mmu->mem_address = ( rtl->MEM_ADDR_OUT << 12 ) |
                       ( address_valid ? ( mmu->cpu_address & offset_mask ) : 0 );
#endif

    mmu->IOSEL_N = rtl->IOSEL_OUT_N;
    mmu->IOSTB_N = rtl->IOSTB_N;
    mmu->DEVSEL_N = rtl->DEVSEL_n;
    mmu->rom_en_n = rtl->ROM_EN_N;
    mmu->ram_en_n = rtl->RAM_EN_N;
    mmu->bus_rd_n = rtl->BUS_RD_N;
    mmu->bus_wr_n = rtl->BUS_WR_N;

    iou->devsel_3_n = rtl->C0BX_N;
    iou->devsel_6_n = rtl->C0EX_N;
    iou->iosel_6_n = rtl->C6XX_N;
    iou->spk = rtl->SPK_OUT;
    iou->casout = rtl->CAS_OUT;
}
