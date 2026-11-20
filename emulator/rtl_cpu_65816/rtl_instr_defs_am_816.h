#ifndef instr_defs_am_816_h
#define instr_defs_am_816_h

INLINE static void instruction_trace( rtl_access_t * state )
{
    if( ( state->bus_busy == FALSE ) && ( ( state->instr_state == Instr_State_BusRead ) || ( state->instr_state == Instr_State_BusWrite ) ) )
    {
        state->instr_trace[ state->instr_index ].instr_bytes = state->data_xfer;
        state->instr_trace[ state->instr_index ].instr_addr  = state->addr_req;
        state->instr_trace[ state->instr_index ].instr_mode  = state->instr_state;
        state->instr_index++;
    }
}

INLINE static void instr_fetch( rtl_access_t * state )
{
    if( state->exec_state == Exec_State_Fetch )
    {
        SETUP_FETCH_BYTE( state, bank_pbr | state->regs.pc )
        {
            if( state->data_xfer == 0xA0 )
            {
                state->instr_state  = Instr_State_Done; 
            }
            TERMINATE_FETCH( state, 1 );
        }
    }
}

INLINE static void instr_reset( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Reset + 0:
            SETUP_READ_BYTE( state, CPU_VECTOR_RESET )
            {
                state->regs.pc = state->data_xfer;
            }
            break;
            
        case Exec_State_Reset + 1:
            SETUP_READ_BYTE( state, CPU_VECTOR_RESET + 1 )
            {
                state->regs.pc     |= ( state->data_xfer << 8 );
                state->exec_state  = Exec_State_Fetch;
                state->instr_state = Instr_State_Done;
                state->bus_busy    = FALSE;
                state->is_implicity = FALSE;

                state->regs.e      = TRUE;
                state->regs.pbr    = 0x00;
                state->regs.dbr    = 0x00;
                state->regs.dp.w   = 0x0000;
                state->regs.sp.w   = 0x01ff;
                state->regs.p.b    = 0x34;
                
                state->regs.sp.w   = 0x0100 | ((state->regs.sp.w - 3) & 0xFF);
                
                timer_Start( &state->cpu_timer, state->cpu_cycle_ms );

                state->cpu_cycle_counter = 0;
                state->start_halt_us     = system_timer_get_us();
                state->instr_index       = 0;
            }
            break;
            
        default:
            break;
    }
}

// Absolute a
// Absolute Indexed X - a,X
// Absolute Indexed Y - a,Y
INLINE static void am_absl_xy( rtl_access_t * state, uint32_t reg )
{
    switch( state->exec_state )
    {
        case Exec_State_Addressing + 0:
            SETUP_READ_BYTE( state, bank_pbr | state->regs.pc )
            {
                STORE_READ_LSB( state );
            }
            break;
            
        case Exec_State_Addressing + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                state->addr_instr   |= bank_dbr;
                state->addr_instr   += reg;
                TERMINATE_ADDRESSING( state, 2 );
            }
            break;
            
        default:
            break;
    }
}

// Absolute Indirect - (a)
// Absolute Indexed Indirect - (a,X)
INLINE static void am_absl_indirect( rtl_access_t * state, uint32_t post_bank, uint32_t reg )
{
    switch( state->exec_state )
    {
        case Exec_State_Addressing + 0:
            SETUP_READ_BYTE( state, bank_pbr | state->regs.pc )
            {
                STORE_READ_LSB( state );
            }
            break;
            
        case Exec_State_Addressing + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
            }
            break;

        case Exec_State_Addressing + 2:
            SETUP_READ_BYTE( state, state->addr_instr + reg )
            {
                STORE_READ_LSB( state );
            }
            break;

        case Exec_State_Addressing + 3:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                state->addr_instr   |= post_bank;
                TERMINATE_ADDRESSING( state, 2 );
            }
            break;
            
        default:
            break;
    }
}

// Absolute Indirect Long - [a]
INLINE static void am_absl_indirect_long( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Addressing + 0:
            SETUP_READ_BYTE( state, bank_pbr | state->regs.pc )
            {
                STORE_READ_LSB( state );
            }
            break;
            
        case Exec_State_Addressing + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
            }
            break;
            
        case Exec_State_Addressing + 2:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                STORE_READ_LSB( state );
            }
            break;
            
        case Exec_State_Addressing + 3:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
            }
            break;

        case Exec_State_Addressing + 4:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_HSB( state );
                TERMINATE_ADDRESSING( state, 2 );
            }
            break;
            
        default:
            break;
    }
}

// Absolute Long - >a
// Absolute Long Indexed - >a,X
INLINE static void am_alng_x( rtl_access_t * state, uint32_t reg )
{
    switch( state->exec_state )
    {
        case Exec_State_Addressing + 0:
            SETUP_READ_BYTE( state, bank_pbr | state->regs.pc )
            {
                STORE_READ_LSB( state );
            }
            break;
            
        case Exec_State_Addressing + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
            }
            break;
            
        case Exec_State_Addressing + 2:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_HSB( state );
                state->addr_instr   += reg;
                TERMINATE_ADDRESSING( state, 3 );
            }
            break;
            
        default:
            break;
    }
}

// Direct Page - d
// Direct Page Indexed X - d,X
// Direct Page Indexed Y - d,Y
INLINE static void am_direct_page( rtl_access_t * state, uint8_t reg )
{
    uint8_t offset;
    
    switch( state->exec_state )
    {
        case Exec_State_Addressing + 0:
            SETUP_READ_BYTE( state, bank_pbr | state->regs.pc )
            {
                offset = ( uint8_t )( state->data_xfer + reg );
                state->addr_instr    = ( state->regs.direct_page + offset ) & 0x0FFFF;
                TERMINATE_ADDRESSING( state, 1 );
            }
            break;
            
        default:
            break;
    }
}

// Direct Page Indirect - (d)
// Direct Page Indexed Indirect - (d,x)
INLINE static void am_direct_page_pre_indexed( rtl_access_t * state, uint32_t reg )
{
    switch( state->exec_state )
    {
        case Exec_State_Addressing + 0:
            SETUP_READ_BYTE( state, bank_pbr | state->regs.pc )
            {
                state->addr_instr  = state->regs.direct_page + state->data_xfer + reg;
                state->addr_instr &= ( ( reg != 0 ) && state->regs.e ) ? 0x0FF : 0x0FFFF;
            }
            break;
            
        case Exec_State_Addressing + 1:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                STORE_READ_LSB( state );
            }
            break;
            
        case Exec_State_Addressing + 2:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                state->addr_instr   |= bank_dbr;
                TERMINATE_ADDRESSING( state, 1 );
            }
            break;

        default:
            break;
    }
}

// Direct Page Indirect Indexed - (d),Y
INLINE static void am_direct_page_indexed( rtl_access_t * state, uint32_t reg )
{
    switch( state->exec_state )
    {
        case Exec_State_Addressing + 0:
            SETUP_READ_BYTE( state, bank_pbr | state->regs.pc )
            {
            }
            break;
            
        case Exec_State_Addressing + 1:
            SETUP_READ_BYTE( state, state->regs.direct_page + state->data_xfer )
            {
                STORE_READ_LSB( state );
            }
            break;

        case Exec_State_Addressing + 2:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );

                state->addr_instr   += reg;
                state->addr_instr   |= bank_dbr;
                TERMINATE_ADDRESSING( state, 1 );
            }
            break;
            
        default:
            break;
    }
}

// Direct Page Indirect Long - [d]
// Direct Page Indirect Long Indexed - [d],Y
INLINE static void am_direct_page_long_indirect( rtl_access_t * state, uint32_t reg )
{
    switch( state->exec_state )
    {
        case Exec_State_Addressing + 0:
            SETUP_READ_BYTE( state, bank_pbr | state->regs.pc )
            {
                state->addr_instr  = state->regs.direct_page + state->data_xfer;
            }
            break;
            
        case Exec_State_Addressing + 1:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                STORE_READ_LSB( state );
            }
            break;

        case Exec_State_Addressing + 2:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
            }
            break;
            
        case Exec_State_Addressing + 3:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_HSB( state );
                state->addr_instr   += reg;
                TERMINATE_ADDRESSING( state, 1 );
            }
            break;
            
        default:
            break;
    }
}

// Relative - d
// Long Relative - d
INLINE static void am_relative( rtl_access_t * state, bool_t is_long )
{
    switch( state->exec_state )
    {
        case Exec_State_Addressing + 0:
            SETUP_READ_BYTE( state, bank_pbr | state->regs.pc )
            {
                if( is_long )
                {
                    STORE_READ_LSB( state );
                }
                else
                {
                    state->addr_instr   = ( state->regs.pc + 1 ) + ( signed char )state->data_xfer;
                    TERMINATE_ADDRESSING( state, 1 );
                }
            }
            break;

        case Exec_State_Addressing + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                state->addr_instr   = ( state->regs.pc + 2 ) + ( signed short )state->addr_instr;
                TERMINATE_ADDRESSING( state, 2 );
            }
            break;

        default:
            break;
    }
}

// Stack Relative - d,S
// Stack Relative Indirect Indexed Y - (d,S),Y
INLINE static void am_stack_relative( rtl_access_t * state, bool_t is_indexed_y )
{
    switch( state->exec_state )
    {
        case Exec_State_Addressing + 0:
            SETUP_READ_BYTE( state, bank_pbr | state->regs.pc )
            {
                STORE_READ_LSB( state );

                if (state->regs.flag_e)
                    state->addr_instr = ((bank(0) | join16(hi(state->regs.stack_pointer), state->regs.stack_pointer8 + state->addr_instr)));
                else
                    state->addr_instr =  (bank_dbr | (uint16_t)(state->regs.stack_pointer + state->addr_instr));

                if( !is_indexed_y )
                {
                    TERMINATE_ADDRESSING( state, 1 );
                }
            }
            break;

        case Exec_State_Addressing + 1:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                STORE_READ_LSB( state );
            }
            break;
            
        case Exec_State_Addressing + 2:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                state->addr_instr   = ( state->addr_instr + CPU_REG_Y_16 ) & 0x0FFFF;
                TERMINATE_ADDRESSING( state, 1 );
            }
            break;

        default:
            break;
    }
}

INLINE static void am_implicity( rtl_access_t * state )
{
    state->is_implicity = TRUE;
    TERMINATE_ADDRESSING( state, 0 );
}

// Immediate uint8_t
INLINE static void am_immediate_b( rtl_access_t * state )
{
    state->addr_instr = bank_pbr | state->regs.pc;
    state->is_implicity = TRUE;
    TERMINATE_ADDRESSING( state, 1 );
}

// Immediate uint16_t
INLINE static void am_immediate_w( rtl_access_t * state )
{
    state->addr_instr = bank_pbr | state->regs.pc;
    state->is_implicity = TRUE;
    TERMINATE_ADDRESSING( state, 2 );
}

// Immediate based on size of A/M
INLINE static void am_immediate( rtl_access_t * state )
{
    state->addr_instr = bank_pbr | state->regs.pc;
    state->is_implicity = TRUE;
    unsigned int size = (state->regs.flag_e || state->regs.flag_m) ? 1 : 2;
    TERMINATE_ADDRESSING( state, size );
}

// Immediate based on size of X/Y
INLINE static void am_immediate_xy( rtl_access_t * state )
{
    state->addr_instr = bank_pbr | state->regs.pc;
    state->is_implicity = TRUE;
    unsigned int size = (state->regs.flag_e || state->regs.flag_x) ? 1 : 2;
    TERMINATE_ADDRESSING( state, size );
}

#endif // 
