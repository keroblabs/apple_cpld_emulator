#include "rtl_model.h"
#include "rtl_internal.h"

////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////

#ifdef SPLIT_CPLD
void rtl_model_reset( void )
{
    int i;
    
    p_model_MMU->PHI_0 = FALSE;
    p_model_MMU->RESET = FALSE;
    p_model_MMU->ADDR  = 0x0FFFF;

    p_model_IOU->PHI_0    = FALSE;
    p_model_IOU->RESET    = FALSE;
    p_model_IOU->C0XX_N   = TRUE;
    p_model_IOU->ADDR     = 0x0FF;

    for( i = 0; i < 2; i++ )
    {
        p_model_MMU->PHI_0   = FALSE;
        p_model_MMU->eval();

        p_model_IOU->PHI_0 = FALSE;
        p_model_IOU->spi_clk = FALSE;
        p_model_IOU->eval();


        p_model_MMU->PHI_0   = TRUE;
        p_model_MMU->eval();

        p_model_IOU->PHI_0 = TRUE;
        p_model_IOU->spi_clk = TRUE;
        p_model_IOU->eval();
    }

    p_model_IOU->RESET    = TRUE;

    p_model_MMU->RESET = TRUE;
    p_model_MMU->DMA_N = TRUE;
    p_model_IOU->SW0 = FALSE;
    p_model_IOU->SW1 = FALSE;
    p_model_IOU->SW2 = FALSE;

    //p_model_MMU->MODE_SEL = FALSE;
    p_model_MMU->MODE_SEL = TRUE;

    p_model_IOU->PDL0 = TRUE;
    p_model_IOU->PDL1 = TRUE;
    p_model_IOU->PDL2 = TRUE;
    p_model_IOU->PDL3 = TRUE;

    p_model_MMU->EXP16_BANK     = p_model_IOU->EXP16_BANK;
    p_model_MMU->EXP16_RAM      = p_model_IOU->EXP16_RAM;
    p_model_MMU->EXP16_WRITE    = p_model_IOU->EXP16_WRITE;
    p_model_MMU->SW_RAMRD       = p_model_IOU->SW_RAMRD;
    p_model_MMU->SW_RAMWR       = p_model_IOU->SW_RAMWR;
    p_model_MMU->SW_SLOTXROM    = p_model_IOU->SW_SLOTXROM;
    p_model_MMU->SW_SLOT3ROM    = p_model_IOU->SW_SLOT3ROM;
    p_model_MMU->SW_ALTZP       = p_model_IOU->SW_ALTZP;
    p_model_MMU->SW_80STORE     = p_model_IOU->SW_80STORE;
    p_model_MMU->SW_PAGE2       = p_model_IOU->SW_PAGE2;
    p_model_MMU->SW_HIRES       = p_model_IOU->SW_HIRES;

}

void rtl_model_iterate( uint32_t address, bool_t cpu_rw, uint8_t data_in, cpld_model_MMU_t * cpld_model_MMU, cpld_model_IOU_t * cpld_model_IOU )
{
    p_model_MMU->PHI_0     = FALSE;
    p_model_MMU->ADDR      = address;
    p_model_MMU->CPU_RW    = cpu_rw;
    p_model_MMU->MODE_SEL  = TRUE;

    p_model_MMU->eval();

    ////////////////////////////

    p_model_IOU->PHI_0     = FALSE;
    p_model_IOU->ADDR      = address & 0x0FF;
    p_model_IOU->CPU_RW    = cpu_rw;
    p_model_IOU->DATA      = data_in;
    p_model_IOU->C0XX_N    = cpld_model_IOU->C0XX_N      = p_model_MMU->C0XX_N_OUT;

    if( cpld_model_IOU->spi_pending )
    {
        if( cpld_model_IOU->spi_state == 0 )
        {
            p_model_IOU->spi_cs = TRUE;
            p_model_IOU->spi_clk = FALSE;
            p_model_IOU->spi_mosi = FALSE;
            cpld_model_IOU->spi_data_rcvr = 0;
            cpld_model_IOU->spi_bit_index = 12;
        }
        else if( ( cpld_model_IOU->spi_state >= 1 ) && ( cpld_model_IOU->spi_state <= 13 ) )
        {
            p_model_IOU->spi_cs = FALSE;
            p_model_IOU->spi_clk = FALSE;
            p_model_IOU->spi_mosi = ( cpld_model_IOU->spi_data_send >> cpld_model_IOU->spi_bit_index ) & 0x01;
            cpld_model_IOU->spi_bit_index--;
        }
        else if( cpld_model_IOU->spi_state == 14 )
        {
            p_model_IOU->spi_cs = FALSE;
            p_model_IOU->spi_clk = FALSE;
            p_model_IOU->spi_mosi = FALSE;
        }
        else if( cpld_model_IOU->spi_state == 15 )
        {
            p_model_IOU->spi_cs = TRUE;
            p_model_IOU->spi_clk = FALSE;
            p_model_IOU->spi_mosi = FALSE;
            cpld_model_IOU->spi_pending = FALSE;
        }
    }

    p_model_IOU->eval();

    ////////////////////////////

    p_model_IOU->PHI_0          = TRUE;

    if( cpld_model_IOU->spi_pending )
    {
        if( cpld_model_IOU->spi_state == 0 )
        {
            p_model_IOU->spi_cs = TRUE;
            p_model_IOU->spi_clk = TRUE;
            //cpld_model_IOU->spi_state++;
        }
        else if( ( cpld_model_IOU->spi_state >= 1 ) && ( cpld_model_IOU->spi_state <= 13 ) )
        {
            p_model_IOU->spi_clk = TRUE;
        }
        else if( cpld_model_IOU->spi_state == 14 )
        {
            p_model_IOU->spi_clk = TRUE;
            cpld_model_IOU->spi_state++;
        }
    }

    p_model_IOU->eval();

    if( cpld_model_IOU->spi_pending )
    {
        if( ( cpld_model_IOU->spi_state >= 1 ) && ( cpld_model_IOU->spi_state <= 13 ) )
        {
            if( cpld_model_IOU->spi_state <= 12 )
            {
                cpld_model_IOU->spi_data_rcvr = ( cpld_model_IOU->spi_data_rcvr << 1 ) | ( p_model_IOU->spi_miso & 0x01 );
            }

            cpld_model_IOU->spi_state++;
        }
    }

    if( cpld_model_IOU->spi_pending )
    {
        if( cpld_model_IOU->spi_state == 0 )
        {
            cpld_model_IOU->spi_state++;
        }
    }

    cpld_model_IOU->data_bus = 0;

    if( !( p_model_IOU->DATA_EN_N & 0x01 ) && cpu_rw )
    {
        cpld_model_IOU->data_bus = p_model_IOU->DATA;
    }

    cpld_model_IOU->devsel_3_n  = p_model_IOU->C0BX_N;
    cpld_model_IOU->devsel_6_n  = p_model_IOU->C0EX_N;
    cpld_model_IOU->iosel_6_n   = p_model_MMU->C6XX_N;

    cpld_model_IOU->spk         = p_model_IOU->SPK_OUT;

    ////////////////////////////

    p_model_MMU->PHI_0     = TRUE;
    p_model_MMU->eval();

    cpld_model_MMU->clock_counter++;

    //if( cpld_model_MMU->clock_counter > 1000000 )
    //{
    //    p_model_MMU->SW1 = FALSE;
    //}

    cpld_model_MMU->mem_address = ( p_model_MMU->MEM_ADDR_16_12 & 0x01F );
    cpld_model_MMU->mem_address = ( cpld_model_MMU->mem_address << 12 ) | ( address & 0x0FFF );

    //std::cout.flush();

    cpld_model_MMU->IOSEL_N     = p_model_MMU->IOSEL_OUT_N;
    cpld_model_MMU->IOSTB_N     = p_model_MMU->IOSTB_N;

    cpld_model_MMU->DEVSEL_N    = p_model_IOU->DEVSEL_n;
    cpld_model_MMU->rom_en_n    = p_model_MMU->ROM_EN_N;
    cpld_model_MMU->ram_en_n    = p_model_MMU->RAM_EN_N;
    
    ////////////////////////////

    p_model_MMU->EXP16_BANK     = cpld_model_MMU->EXP16_BANK     = p_model_IOU->EXP16_BANK;
    p_model_MMU->EXP16_RAM      = cpld_model_MMU->EXP16_RAM      = p_model_IOU->EXP16_RAM;
    p_model_MMU->EXP16_WRITE    = cpld_model_MMU->EXP16_WRITE    = p_model_IOU->EXP16_WRITE;
    p_model_MMU->SW_RAMRD       = cpld_model_MMU->SW_RAMRD       = p_model_IOU->SW_RAMRD;
    p_model_MMU->SW_RAMWR       = cpld_model_MMU->SW_RAMWR       = p_model_IOU->SW_RAMWR;
    p_model_MMU->SW_SLOTXROM    = cpld_model_MMU->SW_SLOTXROM    = p_model_IOU->SW_SLOTXROM;
    p_model_MMU->SW_SLOT3ROM    = cpld_model_MMU->SW_SLOT3ROM    = p_model_IOU->SW_SLOT3ROM;
    p_model_MMU->SW_ALTZP       = cpld_model_MMU->SW_ALTZP       = p_model_IOU->SW_ALTZP;
    p_model_MMU->SW_80STORE     = cpld_model_MMU->SW_80STORE     = p_model_IOU->SW_80STORE;
    p_model_MMU->SW_PAGE2       = cpld_model_MMU->SW_PAGE2       = p_model_IOU->SW_PAGE2;
    p_model_MMU->SW_HIRES       = cpld_model_MMU->SW_HIRES       = p_model_IOU->SW_HIRES;
}

#else

void rtl_model_reset( void )
{
    int i;
    
    p_model->PHI_0 = FALSE;
    p_model->RESET = FALSE;
    p_model->ADDR  = 0x0FFFF;
    p_model->spi_cs = TRUE;

    for( i = 0; i < 2; i++ )
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
    p_model->SW2 = FALSE;

    p_model->PDL0 = TRUE;
    p_model->PDL1 = TRUE;
    p_model->PDL2 = TRUE;
    p_model->PDL3 = TRUE;

    //p_model->MODE_SEL = FALSE;
    p_model->MODE_SEL = TRUE;
}

#if 0
void rtl_model_spi_iterate( cpld_model_IOU_t * cpld_model_IOU )
{
    if( cpld_model_IOU->spi_pending )
    {
        if( cpld_model_IOU->spi_state == 0 )
        {
            p_model->spi_cs = TRUE;
            p_model->spi_clk = FALSE;
            p_model->spi_mosi = FALSE;

            cpld_model_IOU->spi_data_rcvr = 0;
            cpld_model_IOU->spi_bit_index = 12;
        }
        else if( ( cpld_model_IOU->spi_state >= 1 ) && ( cpld_model_IOU->spi_state <= 13 ) )
        {
            p_model->spi_cs = FALSE;
            p_model->spi_clk = FALSE;
            p_model->spi_mosi = ( cpld_model_IOU->spi_data_send >> cpld_model_IOU->spi_bit_index ) & 0x01;

            cpld_model_IOU->spi_bit_index--;
        }
        else if( cpld_model_IOU->spi_state == 14 )
        {
            p_model->spi_cs = FALSE;
            p_model->spi_clk = FALSE;
            p_model->spi_mosi = FALSE;
        }
        else if( cpld_model_IOU->spi_state == 15 )
        {
            p_model->spi_cs = TRUE;
            p_model->spi_clk = FALSE;
            p_model->spi_mosi = FALSE;

            cpld_model_IOU->spi_pending = FALSE;
        }
    }    

    p_model->eval();

    if( cpld_model_IOU->spi_pending )
    {
        if( cpld_model_IOU->spi_state == 0 )
        {
            p_model->spi_cs = TRUE;
            p_model->spi_clk = TRUE;
            //cpld_model_IOU->spi_state++;
        }
        else if( ( cpld_model_IOU->spi_state >= 1 ) && ( cpld_model_IOU->spi_state <= 13 ) )
        {
            p_model->spi_clk = TRUE;
        }
        else if( cpld_model_IOU->spi_state == 14 )
        {
            p_model->spi_clk = TRUE;
            cpld_model_IOU->spi_state++;
        }
    }

    p_model->eval();

    if( cpld_model_IOU->spi_pending )
    {
        if( ( cpld_model_IOU->spi_state >= 1 ) && ( cpld_model_IOU->spi_state <= 13 ) )
        {
            if( cpld_model_IOU->spi_state <= 12 )
            {
                cpld_model_IOU->spi_data_rcvr = ( cpld_model_IOU->spi_data_rcvr << 1 ) | ( p_model->spi_miso & 0x01 );
            }

            cpld_model_IOU->spi_state++;
        }

        if( cpld_model_IOU->spi_state == 0 )
        {
            cpld_model_IOU->spi_state++;
        }
    }    
}
#endif

void rtl_model_iterate( uint32_t address, bool_t cpu_rw, uint8_t data_in, cpld_model_MMU_t * cpld_model_MMU, cpld_model_IOU_t * cpld_model_IOU )
{
    p_model->PHI_0     = FALSE;
    p_model->ADDR      = address;
    p_model->CPU_RW    = cpu_rw;
    p_model->MODE_SEL  = TRUE;

    p_model->DATA__en = cpu_rw;
    if( cpu_rw == FALSE )
    {
        p_model->DATA = data_in;
    }

    if( address == 0xC0F1 )
    {
        p_model->MODE_SEL  = TRUE;
    }

    #if 1
    if( cpld_model_IOU->spi_pending )
    {
        if( cpld_model_IOU->spi_state == 0 )
        {
            p_model->spi_cs = TRUE;
            p_model->spi_clk = FALSE;
            p_model->spi_mosi = FALSE;

            cpld_model_IOU->spi_data_rcvr = 0;
            cpld_model_IOU->spi_bit_index = 12;
        }
        else if( ( cpld_model_IOU->spi_state >= 1 ) && ( cpld_model_IOU->spi_state <= 13 ) )
        {
            p_model->spi_cs = FALSE;
            p_model->spi_clk = FALSE;
            p_model->spi_mosi = ( cpld_model_IOU->spi_data_send >> cpld_model_IOU->spi_bit_index ) & 0x01;

            cpld_model_IOU->spi_bit_index--;
        }
        else if( cpld_model_IOU->spi_state == 14 )
        {
            p_model->spi_cs = FALSE;
            p_model->spi_clk = FALSE;
            p_model->spi_mosi = FALSE;
        }
        else if( cpld_model_IOU->spi_state == 15 )
        {
            p_model->spi_cs = TRUE;
            p_model->spi_clk = FALSE;
            p_model->spi_mosi = FALSE;

            cpld_model_IOU->spi_pending = FALSE;
        }
    }    

    p_model->eval();
    #else

    rtl_model_spi_iterate( cpld_model_IOU );
    //rtl_model_spi_iterate( cpld_model_IOU );
    //rtl_model_spi_iterate( cpld_model_IOU );
    //rtl_model_spi_iterate( cpld_model_IOU );
    #endif

    p_model->PHI_0          = TRUE;

    #if 1
    if( cpld_model_IOU->spi_pending )
    {
        if( cpld_model_IOU->spi_state == 0 )
        {
            p_model->spi_cs = TRUE;
            p_model->spi_clk = TRUE;
            //cpld_model_IOU->spi_state++;
        }
        else if( ( cpld_model_IOU->spi_state >= 1 ) && ( cpld_model_IOU->spi_state <= 13 ) )
        {
            p_model->spi_clk = TRUE;
        }
        else if( cpld_model_IOU->spi_state == 14 )
        {
            p_model->spi_clk = TRUE;
            cpld_model_IOU->spi_state++;
        }
    }
    
    p_model->eval();

    #else

    rtl_model_spi_iterate( cpld_model_IOU );
    //rtl_model_spi_iterate( cpld_model_IOU );
    //rtl_model_spi_iterate( cpld_model_IOU );
    //rtl_model_spi_iterate( cpld_model_IOU );
    #endif

    #if 1
    if( cpld_model_IOU->spi_pending )
    {
        if( ( cpld_model_IOU->spi_state >= 1 ) && ( cpld_model_IOU->spi_state <= 13 ) )
        {
            if( cpld_model_IOU->spi_state <= 12 )
            {
                cpld_model_IOU->spi_data_rcvr = ( cpld_model_IOU->spi_data_rcvr << 1 ) | ( p_model->spi_miso & 0x01 );
            }

            cpld_model_IOU->spi_state++;
        }

        if( cpld_model_IOU->spi_state == 0 )
        {
            cpld_model_IOU->spi_state++;
        }
    }
    #endif

    p_model->PHI_0 = FALSE;
    p_model->eval();

    cpld_model_IOU->data_bus = 0;

    if( !( p_model->DATA_EN_N & 0x01 ) && cpu_rw )
    {
        cpld_model_IOU->data_bus = p_model->DATA__out;
    }

    cpld_model_IOU->devsel_3_n  = p_model->C0BX_N;
    cpld_model_IOU->devsel_6_n  = p_model->C0EX_N;
    cpld_model_IOU->iosel_6_n   = p_model->C6XX_N;

    cpld_model_IOU->spk         = p_model->SPK_OUT;

    cpld_model_MMU->clock_counter++;

    //if( cpld_model_MMU->clock_counter > 1000000 )
    //{
    //    p_model->SW1 = FALSE;
    //}

    cpld_model_MMU->mem_address = ( p_model->MEM_ADDR_OUT & 0x01F );
    cpld_model_MMU->mem_address = ( cpld_model_MMU->mem_address << 12 ) | ( address & 0x0FFF );

    //std::cout.flush();

    cpld_model_MMU->IOSEL_N     = p_model->IOSEL_OUT_N;
    cpld_model_MMU->IOSTB_N     = p_model->IOSTB_N;

    cpld_model_MMU->DEVSEL_N    = p_model->DEVSEL_n;
    cpld_model_MMU->rom_en_n    = p_model->ROM_EN_N;
    cpld_model_MMU->ram_en_n    = p_model->RAM_EN_N;
    
}
#endif
