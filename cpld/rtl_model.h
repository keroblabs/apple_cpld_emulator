#ifndef model_test_hpp
#define model_test_hpp

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////////

#include "types.h"

////////////////////////////////////////////////////////////////////////////////////

#define REGISTER_SET(reg, fw, rtl)              if (fw->reg_##reg##_pending_set) {  rtl->reg_##reg##_pending_set = FALSE;               \
                                                                                    rtl->reg_##reg##_data = fw->reg_##reg##_data; }     \
                                                                            else {  rtl->reg_##reg##_pending_set = TRUE;          }

#define REGISTER_CLEAR(reg, fw, rtl)            if (fw->reg_##reg##_pending_clr) {  rtl->reg_##reg##_pending_clr = FALSE;               \
                                                                                    fw->reg_##reg##_data = rtl->reg_##reg##_data; }     \
                                                                            else {  rtl->reg_##reg##_pending_clr = TRUE;          }

#define REGISTER_SET_DONE(reg, fw, rtl)         fw->reg_##reg##_pending_state = rtl->reg_##reg##_pending_state
#define REGISTER_CLEAR_DONE(reg, fw, rtl)       fw->reg_##reg##_pending_state = rtl->reg_##reg##_pending_state

#define REGISTER_DEFINE(reg)                    bool_t  reg_##reg##_pending_set;     \
                                                bool_t  reg_##reg##_pending_clr;     \
                                                bool_t  reg_##reg##_pending_state;   \
                                                uint8_t reg_##reg##_data

////////////////////////////////////////////////////////////////////////////////////

typedef struct cpld_model_MMU_t_
{
    bool_t DEVSEL_N;
    bool_t IOSEL_N;
    bool_t IOSTB_N;
    bool_t C0XX_N;

    bool_t          EXP16_BANK;
    bool_t          EXP16_RAM;
    bool_t          EXP16_WRITE;

    bool_t          SW_RAMRD;
    bool_t          SW_RAMWR;
    bool_t          SW_SLOTXROM;
    bool_t          SW_SLOT3ROM;
    bool_t          SW_ALTZP;
    bool_t          SW_80STORE;

    bool_t          SW_PAGE2;
    bool_t          SW_HIRES;
    

    bool_t rom_en_n;
    bool_t ram_en_n;

    uint32_t mem_address;

    uint64_t clock_counter;

} cpld_model_MMU_t;

typedef struct cpld_model_IOU_t_
{
    bool_t C0XX_N;

    uint8_t data_bus;
    uint16_t address_in;

    bool_t spk;
    bool_t prev_spk;

    bool_t casout;

    bool_t pdl_strobe;
    bool_t pdl_strobe_prev;

    bool_t devsel_3_n;
    bool_t devsel_6_n;
    bool_t iosel_6_n;

    bool_t   spi_pending;
    uint16_t spi_data_send;
    uint16_t spi_data_rcvr;
    uint8_t  spi_bit_index;
    uint8_t  spi_state;
    uint8_t  spi_id;

    uint64_t cycles_pdl_0;
    uint64_t cycles_pdl_1;
    uint64_t cycles_pdl_2;
    uint64_t cycles_pdl_3;

} cpld_model_IOU_t;

typedef struct cpld_model_t_
{
    cpld_model_MMU_t MMU;
    cpld_model_IOU_t IOU;

} cpld_model_t;

////////////////////////////////////////////////////////////////////////////////////

void rtl_model_reset( void );
void rtl_model_iterate( uint32_t address, bool_t cpu_rw, uint8_t data_in, cpld_model_MMU_t * cpld_model_MMU, cpld_model_IOU_t * cpld_model_IOU );

////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* sdram_ctrl_test_hpp */
