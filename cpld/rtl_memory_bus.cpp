#include "rtl_model.h"
#include "rtl_interface.h"

////////////////////////////////////////////////////////////////////////////////////

enum
{
    BUS_STATE_IDLE,
    BUS_STATE_PHASE_0,
    BUS_STATE_PHASE_1,
};

enum
{
    BUS_XFER_PHASE_IDLE,
    BUS_XFER_PHASE_REQ,
    BUS_XFER_PHASE_PROCESS,
    BUS_XFER_PHASE_DONE,
};

#define PHASE_CLOCK_DIVIDER  4

////////////////////////////////////////////////////////////////////////////////////

void memory_bus_process( cpld_model_t *cpld_model )
{
    cpld_data_t *cpld_data = (cpld_data_t *)cpld_model->p_data;
    
    switch( cpld_data->MMU.bus_state )
    {
        case BUS_STATE_IDLE:
            cpld_data->MMU.clock_div = 0;
            break;

        case BUS_STATE_PHASE_0:
            cpld_model->p_model->DATA__en = cpld_data->MMU.cpu_rw;

            cpld_model->p_model->PHI_0     = FALSE;
            cpld_model->p_model->ADDR      = cpld_data->MMU.mem_address;
            cpld_model->p_model->CPU_RW    = cpld_data->MMU.cpu_rw;
            cpld_model->p_model->MODE_SEL  = TRUE;

            if( cpld_data->MMU.bus_xfer_phase == BUS_XFER_PHASE_REQ )
            {
                cpld_data->MMU.bus_xfer_phase = BUS_XFER_PHASE_PROCESS;
            }

            if( cpld_data->MMU.cpu_rw == FALSE )
            {
                cpld_model->p_model->DATA = cpld_data->MMU.data_in;
            }

            if( cpld_data->MMU.mem_address == 0xC0F1 )
            {
                cpld_model->p_model->MODE_SEL  = TRUE;
            }

            if( ++cpld_data->MMU.clock_div >= PHASE_CLOCK_DIVIDER )
            {
                cpld_data->MMU.clock_div   = 0;
                cpld_data->MMU.bus_state   = BUS_STATE_PHASE_1;
                cpld_model->p_model->PHI_0  = TRUE;
            }
            break;

        case BUS_STATE_PHASE_1:
            if( ++cpld_data->MMU.clock_div >= PHASE_CLOCK_DIVIDER )
            {
                if( !( cpld_model->p_model->DATA_EN_N & 0x01 ) && cpld_data->MMU.cpu_rw )
                {
                    cpld_data->IOU.data_bus = cpld_model->p_model->DATA__out;
                }     

                cpld_data->MMU.mem_address = ( cpld_model->p_model->MEM_ADDR_OUT & 0x01F );
                cpld_data->MMU.mem_address = ( cpld_data->MMU.mem_address & 0x0FFF );

                cpld_data->MMU.IOSEL_N     = cpld_model->p_model->IOSEL_OUT_N;
                cpld_data->MMU.IOSTB_N     = cpld_model->p_model->IOSTB_N;

                cpld_data->MMU.DEVSEL_N    = cpld_model->p_model->DEVSEL_n;
                cpld_data->MMU.rom_en_n    = cpld_model->p_model->ROM_EN_N;
                cpld_data->MMU.ram_en_n    = cpld_model->p_model->RAM_EN_N;
                
                cpld_data->IOU.devsel_3_n  = cpld_model->p_model->C0BX_N;
                cpld_data->IOU.devsel_6_n  = cpld_model->p_model->C0EX_N;
                cpld_data->IOU.iosel_6_n   = cpld_model->p_model->C6XX_N;

                cpld_data->IOU.spk         = cpld_model->p_model->SPK_OUT;

                cpld_data->MMU.clock_div   = 0;
                cpld_data->MMU.bus_state   = BUS_STATE_PHASE_0;
                cpld_model->p_model->PHI_0  = FALSE;

                if( cpld_data->MMU.bus_xfer_phase == BUS_XFER_PHASE_PROCESS )
                {
                    cpld_data->MMU.bus_xfer_phase = BUS_XFER_PHASE_DONE;
                }
            }
            break;
    } 
}

void memory_bus_reset( cpld_model_t *cpld_model )
{
    
    cpld_data_t *cpld_data = (cpld_data_t *)cpld_model->p_data;
    cpld_data->MMU.clock_div       = 0;
    cpld_data->MMU.bus_state       = BUS_STATE_IDLE;
    cpld_data->MMU.bus_xfer_phase  = BUS_XFER_PHASE_IDLE;
    cpld_data->MMU.cpu_rw          = FALSE;
    cpld_data->MMU.data_in         = 0;
    cpld_data->MMU.data_out        = 0;
    cpld_data->MMU.mem_address     = 0;
    cpld_data->MMU.IOSEL_N         = TRUE;
    cpld_data->MMU.IOSTB_N         = TRUE;
    cpld_data->MMU.DEVSEL_N        = TRUE;
    cpld_data->MMU.rom_en_n        = TRUE;
    cpld_data->MMU.ram_en_n        = TRUE;
}

void memory_bus_request( cpld_model_t *cpld_model, uint32_t address, bool_t cpu_rw, uint8_t data_in )
{
    cpld_data_t *cpld_data = (cpld_data_t *)cpld_model->p_data;
    cpld_data->MMU.mem_address     = address;
    cpld_data->MMU.cpu_rw          = cpu_rw;
    cpld_data->MMU.data_in         = data_in;
    cpld_data->MMU.bus_xfer_phase  = BUS_XFER_PHASE_REQ;
}

bool_t memory_bus_is_done( cpld_model_t *cpld_model )
{
    cpld_data_t *cpld_data = (cpld_data_t *)cpld_model->p_data;
    bool_t done = ( cpld_data->MMU.bus_xfer_phase == BUS_XFER_PHASE_DONE );
    cpld_data->MMU.bus_xfer_phase  = BUS_XFER_PHASE_IDLE;    
    return done;
}

