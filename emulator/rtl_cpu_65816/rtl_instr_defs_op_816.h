
#ifndef rtl_instr_ops_h_
#define rtl_instr_ops_h_

INLINE static uint16_t logic_alu( uint16_t op1, uint16_t op2, uint8_t operation, uint8_t mode, bool_t * carry )
{
    switch( operation )
    {
        case Instr_Operation_AND:   return op1 & op2;
        case Instr_Operation_OR:    return op1 | op2;
        case Instr_Operation_EOR:   return op1 ^ op2;

        case Instr_Operation_LSR:   
            *carry = ( op1 & 0x01 );
            return ( op1 >> 1 );
            
        case Instr_Operation_CMP:   *carry = ( op1 >= op2 );  return ( op1 - op2 );
        case Instr_Operation_TRB:   return op1 & ~op2;
        case Instr_Operation_TSB:   return op1 |  op2;

        case Instr_Operation_INC:   op1++; return op1 & ( ( mode == Instr_Mode_8bits ) ? 0x0FF : 0x0FFFF );
        case Instr_Operation_DEC:   op1--; return op1 & ( ( mode == Instr_Mode_8bits ) ? 0x0FF : 0x0FFFF );
            
        case Instr_Operation_ASL:
            if( mode == Instr_Mode_8bits )
            {
                *carry = ( op1 & 0x80 ) ? TRUE : FALSE;
                return ( op1 << 1 ) & 0x0FF;
            }
            else if( mode == Instr_Mode_16bits )
            {
                *carry = ( op1 & 0x8000 ) ? TRUE : FALSE;
                return ( op1 << 1 ) & 0x0FFFF;
            }
            break;

        case Instr_Operation_ROR:
            if( mode == Instr_Mode_8bits )
            {
                uint8_t msb = ( *carry ? 0x80 : 0x00 );
                *carry = op1 & 0x01;
                return ( ( op1 >> 1 ) | msb ) & 0x0FF;
            }
            else if( mode == Instr_Mode_16bits )
            {
                uint16_t msb = ( *carry ? 0x8000 : 0x0000 );
                *carry = op1 & 0x01;
                return ( ( op1 >> 1 ) | msb ) & 0x0FFFF;
            }
            break;

        case Instr_Operation_ROL:
            if( mode == Instr_Mode_8bits )
            {
                uint8_t lsb = ( *carry ? 0x01 : 0x00 );
                *carry = ( op1 & 0x80 ) ? TRUE : FALSE;
                return ( ( op1 << 1 ) | lsb ) & 0x0FF;
            }
            else if( mode == Instr_Mode_16bits )
            {
                uint16_t lsb = ( *carry ? 0x01 : 0x00 );
                *carry = ( op1 & 0x8000 ) ? TRUE : FALSE;
                return ( ( op1 << 1 ) | lsb ) & 0x0FFFF;
            }
            break;
            
        default:
            break;
    }
    
    return 0;
}

INLINE static bool_t check_flag( rtl_access_t * state, uint8_t operation )
{
    switch( operation )
    {
        case Instr_Operation_C0: return (state->regs.p.f_c == 0);
        case Instr_Operation_C1: return (state->regs.p.f_c == 1);
        case Instr_Operation_V0: return (state->regs.p.f_v == 0);
        case Instr_Operation_V1: return (state->regs.p.f_v == 1);
        case Instr_Operation_N0: return (state->regs.p.f_n == 0);
        case Instr_Operation_N1: return (state->regs.p.f_n == 1);
        case Instr_Operation_Z0: return (state->regs.p.f_z == 0);
        case Instr_Operation_Z1: return (state->regs.p.f_z == 1);
        case Instr_Operation_Branch_Always: return TRUE;
        default: return FALSE;
    }
    
    return FALSE;
}

INLINE static void set_flag( rtl_access_t * state, uint8_t operation )
{
    switch( operation )
    {
        case Instr_Operation_I0:  state->regs.p.f_i = 0; break;
        case Instr_Operation_I1:  state->regs.p.f_i = 1; break;
        case Instr_Operation_D0:  state->regs.p.f_d = 0; break;
        case Instr_Operation_D1:  state->regs.p.f_d = 1; break;
        case Instr_Operation_C0:  state->regs.p.f_c = 0; break;
        case Instr_Operation_C1:  state->regs.p.f_c = 1; break;
        case Instr_Operation_V0:  state->regs.p.f_v = 0; break;
        case Instr_Operation_V1:  state->regs.p.f_v = 1; break;
        case Instr_Operation_N0:  state->regs.p.f_n = 0; break;
        case Instr_Operation_N1:  state->regs.p.f_n = 1; break;
        case Instr_Operation_Z0:  state->regs.p.f_z = 0; break;
        case Instr_Operation_Z1:  state->regs.p.f_z = 1; break;
        default: break;
    }
}

INLINE static uint8_t load_reg_8( rtl_access_t * state, reg_instr_t reg_instr )
{
    uint8_t reg_value = 0;
    
    switch( reg_instr )
    {
        case Instr_Mode_ACU:   reg_value = state->regs.a.b; break;
        case Instr_Mode_REGX:  reg_value = state->regs.x.b; break;
        case Instr_Mode_REGY:  reg_value = state->regs.y.b; break;
        case Instr_Mode_DBR:   reg_value = state->regs.dbr; break;
        case Instr_Mode_PBR:   reg_value = state->regs.pbr; break;
        case Instr_Mode_DP:    reg_value = state->regs.dp.b; break;
        case Instr_Mode_FLAGS: reg_value = state->regs.p.b; break;
        case Instr_Mode_ZERO:  reg_value = 0; break;
        default: break;
    }

    return reg_value;
}

INLINE static uint16_t load_reg_16( rtl_access_t * state, reg_instr_t reg_instr )
{
    uint16_t reg_value = 0;
    
    switch( reg_instr )
    {
        case Instr_Mode_ACU:   reg_value = state->regs.a.w; break;
        case Instr_Mode_REGX:  reg_value = state->regs.x.w; break;
        case Instr_Mode_REGY:  reg_value = state->regs.y.w; break;
        case Instr_Mode_DBR:   reg_value = state->regs.dbr; break;
        case Instr_Mode_PBR:   reg_value = state->regs.pbr; break;
        case Instr_Mode_DP:    reg_value = state->regs.dp.w; break;
        case Instr_Mode_FLAGS: reg_value = state->regs.p.b; break;
        case Instr_Mode_ZERO:  reg_value = 0; break;
        default: break;
    }

    return reg_value;
}

INLINE static void store_reg_8( rtl_access_t * state, reg_instr_t reg_instr, uint8_t reg_value )
{
    switch( reg_instr )
    {
        case Instr_Mode_ACU:   state->regs.a.b = reg_value; break;
        case Instr_Mode_REGX:  state->regs.x.b = reg_value; break;
        case Instr_Mode_REGY:  state->regs.y.b = reg_value; break;
        case Instr_Mode_DBR:   state->regs.dbr = reg_value; break;
        case Instr_Mode_PBR:   state->regs.pbr = reg_value; break;
        case Instr_Mode_DP:    state->regs.dp.b = reg_value; break;
        case Instr_Mode_SP:    state->regs.sp.w = reg_value; break;
        case Instr_Mode_FLAGS: state->regs.p.b  = reg_value; break;
        default: break;
    }
}

INLINE static void store_reg_16( rtl_access_t * state, reg_instr_t reg_instr, uint16_t reg_value )
{
    switch( reg_instr )
    {
        case Instr_Mode_ACU:   state->regs.a.w = reg_value; break;
        case Instr_Mode_REGX:  state->regs.x.w = reg_value; break;
        case Instr_Mode_REGY:  state->regs.y.w = reg_value; break;
        case Instr_Mode_DBR:   state->regs.dbr = reg_value & 0x0FF; break;
        case Instr_Mode_PBR:   state->regs.pbr = reg_value & 0x0FF; break;
        case Instr_Mode_DP:    state->regs.dp.w = reg_value; break;
        case Instr_Mode_SP:    state->regs.sp.w = reg_value; break;
        case Instr_Mode_FLAGS: state->regs.p.b  = reg_value & 0x0FF; break;
        default: break;
    }
}

INLINE static void op_adc_sbc( rtl_access_t * state, uint8_t operation )
{
    uint8_t   data_8;
    uint16_t  temp_8;
    int       temp_s16;
    uint16_t  data_16;

    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                if (state->regs.e || state->regs.p.f_m)
                {
                    data_8 = ( operation == Instr_Operation_SBC ) ? ~state->data_xfer : state->data_xfer;
                    temp_8 = state->regs.a.b + data_8 + state->regs.p.f_c;
                    if (state->regs.p.f_d)
                    {
                        if ((temp_8 & 0x0f) > 0x09) temp_8 += 0x06;
                        if ((temp_8 & 0xf0) > 0x90) temp_8 += 0x60;
                    }

                    setc(temp_8 & 0x100);
                    setv((~(state->regs.a.b ^ data_8)) & (state->regs.a.b ^ temp_8) & 0x80);
                    setnz_b(state->regs.a.b = lo(temp_8));
                    TERMINATE_EXECUTE( state );
                }
                else
                {
                    STORE_READ_LSB( state );
                }
            }
            break;
            
        case Exec_State_Execute + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                data_16  = ( operation == Instr_Operation_SBC ) ? ~state->addr_instr : state->addr_instr;
                temp_s16 = state->regs.a.w + data_16 + state->regs.p.f_c;
                
                if (state->regs.p.f_d)
                {
                    if ((temp_s16 & 0x000f) > 0x0009) temp_s16 += 0x0006;
                    if ((temp_s16 & 0x00f0) > 0x0090) temp_s16 += 0x0060;
                    if ((temp_s16 & 0x0f00) > 0x0900) temp_s16 += 0x0600;
                    if ((temp_s16 & 0xf000) > 0x9000) temp_s16 += 0x6000;
                }
                
                setc(temp_s16 & 0x10000);
                setv((~(state->regs.a.w ^ data_16)) & (state->regs.a.w ^ temp_s16) & 0x8000);
                setnz_w(state->regs.a.w = (uint16_t)temp_s16);
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_logic_reg( rtl_access_t * state, uint8_t operation, reg_instr_t reg_instr, bool_t size_is_8 )
{
    bool_t carry = FALSE;
    uint8_t reg_value_8;
    uint16_t reg_value_16;

    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if( size_is_8 )
                {
                    carry = state->regs.p.f_c;
                    reg_value_8 = load_reg_8( state, reg_instr );
                    reg_value_8 = ( uint8_t )logic_alu( reg_value_8, 0, operation, Instr_Mode_8bits, &carry );
                    state->regs.p.f_c = carry;
                    setnz_b( reg_value_8 );
                    store_reg_8( state, reg_instr, reg_value_8 );
                }
                else
                {
                    carry = state->regs.p.f_c;
                    reg_value_16 = load_reg_16( state, reg_instr );
                    reg_value_16 = ( uint16_t )logic_alu( reg_value_16, 0, operation, Instr_Mode_16bits, &carry );
                    state->regs.p.f_c = carry;
                    setnz_w( reg_value_16 );
                    store_reg_16( state, reg_instr, reg_value_16 );
                }

                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_logic_cmp( rtl_access_t * state, reg_instr_t reg_instr, bool_t is_size_8 )
{
    uint8_t  reg_value_8;
    uint8_t  data_8;
    uint16_t temp_8;
    
    uint16_t reg_value_16;
    uint16_t  data_16;
    uint32_t temp_16;

    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                if( is_size_8 )
                {
                    reg_value_8 = load_reg_8( state, reg_instr );

                    data_8 = state->data_xfer;
                    temp_8 = reg_value_8 - data_8;
                                    
                    setc( reg_value_8 >= data_8 );
                    setnz_b( lo( temp_8 ) );
                    TERMINATE_EXECUTE( state );
                }
                else
                {
                    STORE_READ_LSB( state );
                }
            }
            break;
            
        case Exec_State_Execute + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                reg_value_16 = load_reg_8( state, reg_instr );

                data_16 = state->addr_instr;
                temp_16 = reg_value_16 - data_16;
                                
                setc( reg_value_16 >= data_16 );
                setnz_w( ( uint16_t )temp_16 );
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_logic( rtl_access_t * state, uint8_t operation, reg_instr_t reg_instr, bool_t is_size_8 )
{
    uint8_t reg_value_8;
    uint16_t reg_value_16;

    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                if( is_size_8 )
                {
                    reg_value_8 = load_reg_8( state, reg_instr );
                    reg_value_8 = ( uint8_t )logic_alu( reg_value_8, state->data_xfer, operation, Instr_Mode_8bits, NULL );
                    setnz_b(reg_value_8);
                    store_reg_8( state, reg_instr, reg_value_8 );
                    TERMINATE_EXECUTE( state );
                }
                else
                {
                    STORE_READ_LSB( state );
                }
            }
            break;
            
        case Exec_State_Execute + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                reg_value_16 = load_reg_16( state, reg_instr );
                reg_value_16 = ( uint16_t )logic_alu( reg_value_16, state->addr_instr, operation, Instr_Mode_16bits, NULL );
                setnz_w( reg_value_16 );
                store_reg_16( state, reg_instr, reg_value_16 );
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_load( rtl_access_t * state, reg_instr_t reg_instr, bool_t is_size_8 )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                if( is_size_8 )
                {
                    setnz_b( state->data_xfer );
                    store_reg_8( state, reg_instr, state->data_xfer );
                    TERMINATE_EXECUTE( state );
                }
                else
                {
                    STORE_READ_LSB( state );
                }
            }
            
            break;
            
        case Exec_State_Execute + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                setnz_w( state->addr_instr );
                store_reg_16( state, reg_instr, state->addr_instr );
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_branch( rtl_access_t * state, uint8_t operation )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if( !check_flag( state, operation ) )
                {
                    TERMINATE_EXECUTE( state );
                }
            }
            break;

        case Exec_State_Execute + 1:
            SETUP_NOP_CYCLE( state )
            {
                state->regs.pc = (uint16_t)state->addr_instr;

                if( !( state->regs.e && ( ( state->regs.pc ^ state->addr_instr ) & 0xff00 ) ) )
                {
                    TERMINATE_EXECUTE( state );
                }
            }
            break;
            
        case Exec_State_Execute + 2:
            SETUP_NOP_CYCLE( state )
            {
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_jump( rtl_access_t * state, uint8_t operation )
{
    if( ( state->exec_state == Exec_State_Execute ) && ( state->bus_busy == TRUE ) )
    {
        // save the jump address for later
        state->addr_dest = state->addr_instr;
        
        if( operation == Instr_Operation_JMP )
        {
            state->exec_state = Exec_State_Execute + 3;
        }
        else if( operation == Instr_Operation_JSR )
        {
            state->exec_state = Exec_State_Execute + 1;
        }
    }
    
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->regs.pbr )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 1:
            if( state->bus_busy ) state->regs.pc--;
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->regs.pc >> 8 )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 2:
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->regs.pc & 0x0FF )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 3:
            SETUP_NOP_CYCLE( state )
            {
                state->regs.pc = state->addr_dest & 0x0FFFF;
                if( operation != Instr_Operation_JSR )
                {
                    // JSR stays in the same bank
                    state->regs.pbr = lo(state->addr_dest >> 16);
                }
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_return( rtl_access_t * state, bool_t is_long )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            if( state->bus_busy == TRUE )
            {
                if (state->regs.e)
                    ++state->regs.stack_pointer8;
                else
                    ++state->regs.stack_pointer;
            }

            SETUP_READ_BYTE( state, state->regs.stack_pointer )
            {
                state->regs.pc = state->data_xfer & 0x0FF;
            }
            break;

        case Exec_State_Execute + 1:
            if( state->bus_busy == TRUE )
            {
                if (state->regs.e)
                    ++state->regs.stack_pointer8;
                else
                    ++state->regs.stack_pointer;
            }

            SETUP_READ_BYTE( state, state->regs.stack_pointer )
            {
                state->regs.pc += ( state->data_xfer << 8 );
            }
            break;

        case Exec_State_Execute + 2:
            SETUP_NOP_CYCLE( state )
            {
            }
            break;
            
        case Exec_State_Execute + 3:
            if( is_long )
            {
                if( state->bus_busy == TRUE )
                {
                    if (state->regs.e)
                        ++state->regs.stack_pointer8;
                    else
                        ++state->regs.stack_pointer;
                }

                SETUP_READ_BYTE( state, state->regs.stack_pointer )
                {
                    state->regs.pbr = state->data_xfer;
                }
            }
            else
            {
                SETUP_NOP_CYCLE( state )
                {
                }
            }
            break;
            
        case Exec_State_Execute + 4:
            SETUP_NOP_CYCLE( state )
            {
                state->regs.pc++;
                TERMINATE_EXECUTE( state );
            }
            break;

        default:
            break;
    }
}

INLINE static void op_return_rti( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            if( state->bus_busy == TRUE )
            {
                if (state->regs.e)
                    ++state->regs.stack_pointer8;
                else
                    ++state->regs.stack_pointer;
            }

            SETUP_READ_BYTE( state, state->regs.stack_pointer )
            {
                state->regs.p.b = state->data_xfer;
            }
            break;
            
        case Exec_State_Execute + 1:
            SETUP_NOP_CYCLE( state )
            {
            }
            break;

        case Exec_State_Execute + 2:
            if( state->bus_busy == TRUE )
            {
                if (state->regs.e)
                    ++state->regs.stack_pointer8;
                else
                    ++state->regs.stack_pointer;
            }

            SETUP_READ_BYTE( state, state->regs.stack_pointer )
            {
                state->regs.pc = state->data_xfer & 0x0FF;
            }
            break;

        case Exec_State_Execute + 3:
            if( state->bus_busy == TRUE )
            {
                if (state->regs.e)
                    ++state->regs.stack_pointer8;
                else
                    ++state->regs.stack_pointer;
            }

            SETUP_READ_BYTE( state, state->regs.stack_pointer )
            {
                state->regs.pc += ( state->data_xfer << 8 );
            }
            break;

        case Exec_State_Execute + 4:
            SETUP_NOP_CYCLE( state )
            {
                if( state->regs.e )
                {
                    state->exec_state++;
                }
            }
            break;

        case Exec_State_Execute + 5:
            SETUP_NOP_CYCLE( state )
            {
                if( state->bus_busy == TRUE )
                {
                    if (state->regs.e)
                        ++state->regs.stack_pointer8;
                    else
                        ++state->regs.stack_pointer;
                }

                SETUP_READ_BYTE( state, state->regs.stack_pointer )
                {
                    state->regs.pbr = state->data_xfer;
                }
            }
            break;

        case Exec_State_Execute + 6:
            SETUP_NOP_CYCLE( state )
            {
                state->regs.p.f_i = 0;
                state->regs.irq = IRQ_STATE_NONE;
                TERMINATE_EXECUTE( state );
            }
            break;

        default:
            break;
    }
}

INLINE static void op_move_tax( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if( state->regs.e || state->regs.p.f_x )
                {
                    setnz_b( lo( state->regs.x.w = state->regs.a.b ) );
                }
                else
                {
                    setnz_w( state->regs.x.w = state->regs.a.w );
                }

                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_tay( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if( state->regs.e || state->regs.p.f_x )
                {
                    setnz_b( lo( state->regs.y.w = state->regs.a.b ) );
                }
                else
                {
                    setnz_w( state->regs.y.w = state->regs.a.w );
                }
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_tcd( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                state->regs.dp.w = state->regs.a.w;
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_tdc( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if( state->regs.e || state->regs.p.f_m )
                {
                    setnz_b( lo( state->regs.a.w = state->regs.dp.b ) );
                }
                else
                {
                    setnz_w( state->regs.a.w = state->regs.dp.w );
                }
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_tcs( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                state->regs.sp.w = state->regs.e ? (0x0100 | state->regs.a.b) : state->regs.a.w;
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_tsc( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if( state->regs.e || state->regs.p.f_m )
                {
                    setnz_b( lo( state->regs.a.w = state->regs.sp.w ) );
                }
                else
                {
                    setnz_w( state->regs.a.w = state->regs.sp.w );
                }
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_tsx( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if( state->regs.e )
                {
                    setnz_b( state->regs.x.b = state->regs.sp.b );
                }
                else
                {
                    setnz_w( state->regs.x.w = state->regs.sp.w );
                }
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_txa( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if( state->regs.e || state->regs.p.f_m )
                {
                    setnz_b( state->regs.a.b = state->regs.x.b );
                }
                else
                {
                    setnz_w( state->regs.a.w = state->regs.x.w );
                }
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_txs( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if (state->regs.e)
                    state->regs.sp.w = 0x0100 | state->regs.x.b;
                else
                    state->regs.sp.w = state->regs.x.w;

                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_txy( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if (state->regs.e || state->regs.p.f_x)
                {
                    setnz_b(lo(state->regs.y.w = state->regs.x.w));
                }
                else
                {
                    setnz_w(state->regs.y.w = state->regs.x.w);
                }
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_tya( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if (state->regs.e || state->regs.p.f_m)
                {
                    setnz_b(state->regs.a.b = state->regs.y.b);
                }
                else
                {
                    setnz_w(state->regs.a.w = state->regs.y.w);
                }

                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_tyx( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                if (state->regs.e || state->regs.p.f_x)
                {
                    setnz_b(lo(state->regs.x.w = state->regs.y.w));
                }
                else
                {
                    setnz_w(state->regs.x.w = state->regs.y.w);
                }
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_flag( rtl_access_t * state, uint8_t operation )
{
    SETUP_NOP_CYCLE( state )
    {
        set_flag( state, operation );
        TERMINATE_EXECUTE( state );
    }
}

INLINE static void op_logic_bit( rtl_access_t * state, uint8_t operation )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                if( state->regs.e || state->regs.p.f_m )
                {
                    setz((state->regs.a.b & state->data_xfer) == 0);
                    
                    if( operation == Instr_Operation_BitNV )
                    {
                        setn(state->data_xfer & 0x80);
                        setv(state->data_xfer & 0x40);
                    }
                    TERMINATE_EXECUTE( state );
                }
                else
                {
                    STORE_READ_LSB( state );
                }
            }
            break;
            
        case Exec_State_Execute + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                setz((state->regs.a.w & state->addr_instr) == 0);
                
                if( operation == Instr_Operation_BitNV )
                {
                    setn(state->addr_instr & 0x8000);
                    setv(state->addr_instr & 0x4000);
                }
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_brl( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_NOP_CYCLE( state )
            {
                state->regs.pc = (uint16_t)state->addr_instr;
            }
            break;

        case Exec_State_Execute + 1:
            SETUP_NOP_CYCLE( state )
            {
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_soft_irq( rtl_access_t * state, uint8_t operation )
{
    uint8_t flags = 0;
    
    if( ( state->exec_state == Exec_State_Execute ) && ( state->bus_busy == TRUE ) && state->regs.e )
    {
        state->exec_state = Exec_State_Execute + 1;
    }

    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->regs.pbr )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 1:
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->regs.pc >> 8 )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 2:
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->regs.pc & 0x0FF )
            {
                state->addr_req = state->regs.stack_pointer;
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 3:
            flags = state->regs.p.b | ( ( operation == Instr_Operation_BRK ) ? 0x10 : 0x00 );

            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, flags )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
                
                state->addr_dest = (state->regs.e) ? ( ( operation == Instr_Operation_BRK ) ? 0xfffe : 0xfff4 ) : ( ( operation == Instr_Operation_BRK ) ? 0xffe6 : 0xffe4 );
            }
            break;

        case Exec_State_Addressing + 4:
            SETUP_READ_BYTE( state, state->addr_dest )
            {
                STORE_READ_LSB( state );
            }
            break;
            
        case Exec_State_Addressing + 5:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                state->regs.pc = state->addr_instr;
                state->regs.p.f_i = 1;
                state->regs.p.f_d = 0;
                state->regs.pbr = 0;
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_move_data( rtl_access_t * state, uint8_t operation )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                if( operation == Instr_Operation_MVN )
                {
                    state->addr_source = state->data_xfer;
                }
                else
                {
                    state->addr_dest = state->data_xfer;
                }
            }
            break;
            
        case Exec_State_Execute + 1:
            SETUP_NEXT_READ( state )
            {
                if( operation == Instr_Operation_MVN )
                {
                    state->addr_dest = state->data_xfer;
                }
                else
                {
                    state->addr_source = state->data_xfer;
                }
            }
            break;
            
        case Exec_State_Execute + 2:
            SETUP_NOP_CYCLE( state )
            {
            }
            break;
            
        case Exec_State_Execute + 3:
            SETUP_READ_BYTE( state, bank( state->addr_source ) | state->regs.x.w )
            {
            }
            break;
            
        case Exec_State_Execute + 4:
            SETUP_WRITE_BYTE( state, bank( state->addr_dest ) | state->regs.y.w, state->data_xfer )
            {
            }
            break;

        case Exec_State_Execute + 5:
            SETUP_NOP_CYCLE( state )
            {
                if( operation == Instr_Operation_MVN )
                {
                    state->regs.x.w++;
                    state->regs.y.w++;
                }
                else
                {
                    state->regs.x.w--;
                    state->regs.y.w--;
                }

                state->regs.dbr = state->addr_dest;
                
                if( --state->regs.a.w != 0xffff )
                {
                    state->regs.pc -= 3;
                }
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_store_reg( rtl_access_t * state, uint8_t operation, bool_t size_is_8 )
{
    uint16_t reg_value = 0;

    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            if( state->bus_busy == TRUE )
            {
                reg_value = size_is_8 ? load_reg_8( state, operation ) : load_reg_16( state, operation );
            }
            SETUP_WRITE_BYTE( state, state->addr_instr, reg_value )
            {
                if( size_is_8 )
                {
                    TERMINATE_EXECUTE( state );
                }
            }
            break;

        case Exec_State_Execute + 1:
            SETUP_NEXT_WRITE( state )
            {
                TERMINATE_EXECUTE( state );
            }
            break;

        default:
            break;
    }
}

INLINE static void op_read_mod_write( rtl_access_t * state, uint8_t operation )
{
    bool_t carry;
    
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                if( state->regs.e || state->regs.p.f_m )
                {
                    carry = state->regs.p.f_c;
                    state->data_xfer = ( uint8_t )logic_alu( state->data_xfer, 0, operation, Instr_Mode_8bits, &carry );
                    setnz_b(state->data_xfer);
                    state->regs.p.f_c = carry;

                    state->exec_state = Exec_State_Execute + 2;
                }
                else
                {
                    STORE_READ_LSB( state );
                }
            }
            break;

        case Exec_State_Execute + 1:
            SETUP_NEXT_READ( state )
            {
                carry = state->regs.p.f_c;
                state->data_xfer = ( uint16_t )logic_alu( state->addr_instr, 0, operation, Instr_Mode_16bits, &carry );
                setnz_b(state->data_xfer);
                state->regs.p.f_c = carry;
            }
            break;
            
        case Exec_State_Execute + 2:
            SETUP_WRITE_BYTE( state, state->addr_instr, state->data_xfer )
            {
                if( state->regs.e || state->regs.p.f_m )
                {
                    TERMINATE_EXECUTE( state );
                }
            }
            break;

        case Exec_State_Execute + 3:
            SETUP_NEXT_WRITE( state )
            {
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_trb_tsb( rtl_access_t * state, uint8_t operation )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                if( state->regs.e || state->regs.p.f_m )
                {
                    state->data_xfer = ( uint8_t )logic_alu( state->data_xfer, state->regs.a.b, operation, Instr_Mode_8bits, NULL );
                    setnz_b( ( state->data_xfer & state->regs.a.b ) == 0 );

                    state->exec_state = Exec_State_Execute + 2;
                }
                else
                {
                    STORE_READ_LSB( state );
                }
            }
            break;
            
        case Exec_State_Execute + 1:
            SETUP_NEXT_READ( state )
            {
                state->data_xfer = ( uint16_t )logic_alu( state->addr_instr, state->regs.a.w, operation, Instr_Mode_16bits, NULL );
                setnz_w( ( state->data_xfer & state->regs.a.w ) == 0 );
            }
            break;
            
        case Exec_State_Execute + 2:
            SETUP_WRITE_BYTE( state, state->addr_instr, state->data_xfer )
            {
                if( state->regs.e || state->regs.p.f_m )
                {
                    TERMINATE_EXECUTE( state );
                }
            }
            break;

        case Exec_State_Execute + 3:
            SETUP_NEXT_WRITE( state )
            {
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_rep( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                state->regs.p.b &= ~state->data_xfer;
                if (state->regs.e) state->regs.p.f_m = state->regs.p.f_x = 1;
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}


INLINE static void op_sep( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                state->regs.p.b |= state->data_xfer;
                if (state->regs.e) state->regs.p.f_m = state->regs.p.f_x = 1;

                if (state->regs.p.f_x)
                {
                    state->regs.x.w = state->regs.x.b;
                    state->regs.y.w = state->regs.y.b;
                }
                TERMINATE_EXECUTE( state );
            }
            break;

        default:
            break;
    }
}

INLINE static void op_push_indirect( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            SETUP_READ_BYTE( state, state->addr_instr )
            {
                STORE_READ_LSB( state );
            }
            break;
            
        case Exec_State_Execute + 1:
            SETUP_NEXT_READ( state )
            {
                STORE_READ_MSB( state );
                state->data_temp = state->addr_instr;
            }
            break;

        case Exec_State_Execute + 2:
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->data_temp >> 8 )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 3:
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->data_temp & 0x0FF )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 4:
            SETUP_NOP_CYCLE( state )
            {
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_push_operand( rtl_access_t * state )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            if( state->bus_busy )
            {
                state->data_temp = state->addr_instr;
            }
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->data_temp >> 8 )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 1:
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->data_temp & 0x0FF )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;
            
        case Exec_State_Execute + 2:
            SETUP_NOP_CYCLE( state )
            {
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_push( rtl_access_t * state, uint8_t operation, bool_t size_is_8 )
{
    switch( state->exec_state )
    {
        case Exec_State_Execute + 0:
            if( state->bus_busy )
            {
                state->data_temp = size_is_8 ? load_reg_8( state, operation ) : load_reg_16( state, operation );
            }
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->data_temp )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 1:
            SETUP_NOP_CYCLE( state )
            {
                if( size_is_8 )
                {
                    TERMINATE_EXECUTE( state );
                }
            }
            break;
            
        case Exec_State_Execute + 2:
            SETUP_WRITE_BYTE( state, state->regs.stack_pointer, state->data_temp >> 8 )
            {
                if (state->regs.e)
                    --state->regs.stack_pointer8;
                else
                    --state->regs.stack_pointer;
            }
            break;

        case Exec_State_Execute + 3:
            SETUP_NOP_CYCLE( state )
            {
                TERMINATE_EXECUTE( state );
            }
            break;
            
        default:
            break;
    }
}

INLINE static void op_pop( rtl_access_t * state, reg_instr_t reg_instr, bool_t size_is_8 )
{
    if( size_is_8 )
    {
        switch( state->exec_state )
        {
            case Exec_State_Execute + 0:
                if( state->bus_busy )
                {
                    if (state->regs.e)
                        ++state->regs.stack_pointer8;
                    else
                        ++state->regs.stack_pointer;
                }

                SETUP_READ_BYTE( state, state->regs.stack_pointer )
                {
                    if( size_is_8)
                    {
                        if( ( reg_instr == Instr_Mode_REGX ) || ( reg_instr == Instr_Mode_REGY ) )
                        {
                            store_reg_16( state, reg_instr, state->data_xfer );
                        }
                        else
                        {
                            store_reg_8( state, reg_instr, state->data_xfer );
                        }
                        
                        if( reg_instr == Instr_Mode_FLAGS )
                        {
                            state->regs.p.b |= 0x30;
                        }
                        else
                        {
                            setnz_b( lo( state->data_xfer ) );
                        }

                        TERMINATE_EXECUTE( state );
                    }
                    else
                    {
                        state->data_temp = state->data_xfer;
                    }
                }
                break;
                
            case Exec_State_Execute + 1:
                if( state->bus_busy )
                {
                    if (state->regs.e)
                        ++state->regs.stack_pointer8;
                    else
                        ++state->regs.stack_pointer;
                }
                SETUP_READ_BYTE( state, state->regs.stack_pointer )
                {
                    state->data_xfer = ( state->data_xfer << 8 ) | state->data_temp;
                    store_reg_16( state, reg_instr, state->addr_instr );
                    setnz_w( state->data_xfer );
                    
                    TERMINATE_EXECUTE( state );
                }
                break;
                
            default:
                break;
        }
    }
}

INLINE static void op_xba( rtl_access_t * state )
{
    SETUP_NOP_CYCLE( state )
    {
        state->regs.a.w = swap(state->regs.a.w);
        setnz_b(state->regs.a.b);
        
        TERMINATE_EXECUTE( state );
    }
}

INLINE static void op_xce( rtl_access_t * state )
{
    SETUP_NOP_CYCLE( state )
    {
        uint8_t oe = state->regs.e;
        
        state->regs.e = state->regs.p.f_c;
        state->regs.p.f_c = oe;
        
        if (state->regs.e)
        {
            state->regs.p.b |= 0x30;
            state->regs.sp.w = 0x0100 | state->regs.sp.b;
        }
        
        TERMINATE_EXECUTE( state );
    }
}

#endif

