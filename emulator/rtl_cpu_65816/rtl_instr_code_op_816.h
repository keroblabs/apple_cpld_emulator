#ifndef rtl_instr_exec_816_h
#define rtl_instr_exec_816_h

#define NO_REG              0
#define NO_BANK             0

#define rtl_op_adc( state )         op_adc_sbc( state, Instr_Operation_ADC )
#define rtl_op_and( state )         op_logic    ( state, Instr_Operation_AND, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_asl( state )         op_read_mod_write( state, Instr_Operation_ASL )
#define rtl_op_asla( state )        op_logic_reg( state, Instr_Operation_ASL, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_bcc( state )         op_branch( state, Instr_Operation_C0 )
#define rtl_op_bcs( state )         op_branch( state, Instr_Operation_C1 )
#define rtl_op_beq( state )         op_branch( state, Instr_Operation_Z1 )
#define rtl_op_bit( state )         op_logic_bit( state, Instr_Operation_BitNV )
#define rtl_op_biti( state )        op_logic_bit( state, Instr_Operation_Biti )
#define rtl_op_bmi( state )         op_branch( state, Instr_Operation_N1 )
#define rtl_op_bne( state )         op_branch( state, Instr_Operation_Z0 )
#define rtl_op_bpl( state )         op_branch( state, Instr_Operation_N0 )
#define rtl_op_bra( state )         op_branch( state, Instr_Operation_Branch_Always )
#define rtl_op_brk( state )         op_soft_irq ( state, Instr_Operation_COP )
#define rtl_op_brl( state )         op_brl( state )
#define rtl_op_bvc( state )         op_branch( state, Instr_Operation_V0 )
#define rtl_op_bvs( state )         op_branch( state, Instr_Operation_V1 )
#define rtl_op_clc( state )         op_flag( state, Instr_Operation_C0 )
#define rtl_op_cld( state )         op_flag( state, Instr_Operation_D0 )
#define rtl_op_cli( state )         op_flag( state, Instr_Operation_I0 )
#define rtl_op_clv( state )         op_flag( state, Instr_Operation_V0 )
#define rtl_op_cmp( state )         op_logic_cmp( state, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_cop( state )         op_soft_irq ( state, Instr_Operation_COP )
#define rtl_op_cpx( state )         op_logic_cmp( state, Instr_Mode_REGX, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_cpy( state )         op_logic_cmp( state, Instr_Mode_REGY, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_dec( state )         op_read_mod_write( state, Instr_Operation_DEC )
#define rtl_op_deca( state )        op_logic_reg( state, Instr_Operation_DEC, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_dex( state )         op_logic_reg( state, Instr_Operation_DEC, Instr_Mode_REGX, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_dey( state )         op_logic_reg( state, Instr_Operation_DEC, Instr_Mode_REGY, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_eor( state )         op_logic    ( state, Instr_Operation_EOR, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_inc( state )         op_read_mod_write( state, Instr_Operation_INC )
#define rtl_op_inca( state )        op_logic_reg( state, Instr_Operation_INC, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_inx( state )         op_logic_reg( state, Instr_Operation_INC, Instr_Mode_REGX, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_iny( state )         op_logic_reg( state, Instr_Operation_INC, Instr_Mode_REGY, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_jmp( state )         op_jump     ( state, Instr_Operation_JMP );
#define rtl_op_jsl( state )         op_jump     ( state, Instr_Operation_JSL );
#define rtl_op_jsr( state )         op_jump     ( state, Instr_Operation_JSR );
#define rtl_op_lda( state )         op_load     ( state, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_ldx( state )         op_load     ( state, Instr_Mode_REGX, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_ldy( state )         op_load     ( state, Instr_Mode_REGY, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_lsr( state )         op_read_mod_write( state, Instr_Operation_LSR )
#define rtl_op_lsra( state )        op_logic_reg( state, Instr_Operation_LSR, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_mvn( state )         op_move_data( state, Instr_Operation_MVN )
#define rtl_op_mvp( state )         op_move_data( state, Instr_Operation_MVP )
#define rtl_op_nop( state )         op_flag     ( state, Instr_Operation_Branch_Always )
#define rtl_op_ora( state )         op_logic    ( state, Instr_Operation_OR, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_pea( state )         op_push_indirect( state )
#define rtl_op_pei( state )         op_push_indirect( state )
#define rtl_op_per( state )         op_push_operand ( state )
#define rtl_op_pha( state )         op_push     ( state, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_phb( state )         op_push     ( state, Instr_Mode_DBR, TRUE )
#define rtl_op_phd( state )         op_push     ( state, Instr_Mode_DP, FALSE )
#define rtl_op_phk( state )         op_push     ( state, Instr_Mode_PBR, TRUE )
#define rtl_op_php( state )         op_push     ( state, Instr_Mode_FLAGS, TRUE )
#define rtl_op_phx( state )         op_push     ( state, Instr_Mode_REGX, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_phy( state )         op_push     ( state, Instr_Mode_REGY, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_pla( state )         op_pop      ( state, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_plb( state )         op_pop      ( state, Instr_Mode_DBR, TRUE )
#define rtl_op_pld( state )         op_pop      ( state, Instr_Mode_DP, FALSE )
#define rtl_op_plk( state )         op_pop      ( state, Instr_Mode_PBR, TRUE )
#define rtl_op_plp( state )         op_pop      ( state, Instr_Mode_FLAGS, TRUE )
#define rtl_op_plx( state )         op_pop      ( state, Instr_Mode_REGX, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_ply( state )         op_pop      ( state, Instr_Mode_REGY, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_rep( state )         op_rep      ( state )
#define rtl_op_rol( state )         op_read_mod_write( state, Instr_Operation_ROL )
#define rtl_op_rola( state )        op_logic_reg     ( state, Instr_Operation_ROL, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_ror( state )         op_read_mod_write( state, Instr_Operation_ROR )
#define rtl_op_rora( state )        op_logic_reg     ( state, Instr_Operation_ROR, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_rti( state )         op_return_rti ( state );
#define rtl_op_rtl( state )         op_return     ( state, TRUE );
#define rtl_op_rts( state )         op_return     ( state, FALSE );
#define rtl_op_sbc( state )         op_adc_sbc    ( state, Instr_Operation_SBC )
#define rtl_op_sec( state )         op_flag( state, Instr_Operation_C1 )
#define rtl_op_sed( state )         op_flag( state, Instr_Operation_D1 )
#define rtl_op_sei( state )         op_flag( state, Instr_Operation_I1 )
#define rtl_op_sep( state )         op_sep ( state )
#define rtl_op_sta( state )         op_store_reg( state, Instr_Mode_ACU, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_stp( state )         op_flag     ( state, Instr_Operation_Branch_Always )
#define rtl_op_stx( state )         op_store_reg( state, Instr_Mode_REGX, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_sty( state )         op_store_reg( state, Instr_Mode_REGY, ( state->regs.e || state->regs.p.f_x ) )
#define rtl_op_stz( state )         op_store_reg( state, Instr_Mode_ZERO, ( state->regs.e || state->regs.p.f_m ) )
#define rtl_op_tax( state )         op_move_tax ( state )
#define rtl_op_tay( state )         op_move_tay ( state )
#define rtl_op_tcd( state )         op_move_tcd ( state )
#define rtl_op_tdc( state )         op_move_tdc ( state )
#define rtl_op_tcs( state )         op_move_tcs ( state )
#define rtl_op_trb( state )         op_trb_tsb  ( state, Instr_Operation_TRB )
#define rtl_op_tsb( state )         op_trb_tsb  ( state, Instr_Operation_TSB )
#define rtl_op_tsc( state )         op_move_tsc ( state )
#define rtl_op_tsx( state )         op_move_tsx ( state )
#define rtl_op_txa( state )         op_move_txa ( state )
#define rtl_op_txs( state )         op_move_txs ( state )
#define rtl_op_txy( state )         op_move_txy ( state )
#define rtl_op_tya( state )         op_move_tya ( state )
#define rtl_op_tyx( state )         op_move_tyx ( state )
#define rtl_op_wai( state )         op_flag     ( state, Instr_Operation_Branch_Always )
#define rtl_op_wdm( state )         op_flag     ( state, Instr_Operation_Branch_Always )
#define rtl_op_xba( state )         op_xba      ( state )
#define rtl_op_xce( state )         op_xce      ( state )


#define RTL_INSTR_OP_DECODE()   switch ( state->exec_opcode ) {                    \
                                    case 0x00:    rtl_op_brk ( state );    break;  \
                                    case 0x01:    rtl_op_ora ( state );    break;  \
                                    case 0x02:    rtl_op_cop ( state );    break;  \
                                    case 0x03:    rtl_op_ora ( state );    break;  \
                                    case 0x04:    rtl_op_tsb ( state );    break;  \
                                    case 0x05:    rtl_op_ora ( state );    break;  \
                                    case 0x06:    rtl_op_asl ( state );    break;  \
                                    case 0x07:    rtl_op_ora ( state );    break;  \
                                    case 0x08:    rtl_op_php ( state );    break;  \
                                    case 0x09:    rtl_op_ora ( state );    break;  \
                                    case 0x0a:    rtl_op_asla( state );    break;  \
                                    case 0x0b:    rtl_op_phd ( state );    break;  \
                                    case 0x0c:    rtl_op_tsb ( state );    break;  \
                                    case 0x0d:    rtl_op_ora ( state );    break;  \
                                    case 0x0e:    rtl_op_asl ( state );    break;  \
                                    case 0x0f:    rtl_op_ora ( state );    break;  \
                                    case 0x10:    rtl_op_bpl ( state );    break;  \
                                    case 0x11:    rtl_op_ora ( state );    break;  \
                                    case 0x12:    rtl_op_ora ( state );    break;  \
                                    case 0x13:    rtl_op_ora ( state );    break;  \
                                    case 0x14:    rtl_op_trb ( state );    break;  \
                                    case 0x15:    rtl_op_ora ( state );    break;  \
                                    case 0x16:    rtl_op_asl ( state );    break;  \
                                    case 0x17:    rtl_op_ora ( state );    break;  \
                                    case 0x18:    rtl_op_clc ( state );    break;  \
                                    case 0x19:    rtl_op_ora ( state );    break;  \
                                    case 0x1a:    rtl_op_inca( state );    break;  \
                                    case 0x1b:    rtl_op_tcs ( state );    break;  \
                                    case 0x1c:    rtl_op_trb ( state );    break;  \
                                    case 0x1d:    rtl_op_ora ( state );    break;  \
                                    case 0x1e:    rtl_op_asl ( state );    break;  \
                                    case 0x1f:    rtl_op_ora ( state );    break;  \
                                    case 0x20:    rtl_op_jsr ( state );    break;  \
                                    case 0x21:    rtl_op_and ( state );    break;  \
                                    case 0x22:    rtl_op_jsl ( state );    break;  \
                                    case 0x23:    rtl_op_and ( state );    break;  \
                                    case 0x24:    rtl_op_bit ( state );    break;  \
                                    case 0x25:    rtl_op_and ( state );    break;  \
                                    case 0x26:    rtl_op_rol ( state );    break;  \
                                    case 0x27:    rtl_op_and ( state );    break;  \
                                    case 0x28:    rtl_op_plp ( state );    break;  \
                                    case 0x29:    rtl_op_and ( state );    break;  \
                                    case 0x2a:    rtl_op_rola( state );    break;  \
                                    case 0x2b:    rtl_op_pld ( state );    break;  \
                                    case 0x2c:    rtl_op_bit ( state );    break;  \
                                    case 0x2d:    rtl_op_and ( state );    break;  \
                                    case 0x2e:    rtl_op_rol ( state );    break;  \
                                    case 0x2f:    rtl_op_and ( state );    break;  \
                                    case 0x30:    rtl_op_bmi ( state );    break;  \
                                    case 0x31:    rtl_op_and ( state );    break;  \
                                    case 0x32:    rtl_op_and ( state );    break;  \
                                    case 0x33:    rtl_op_and ( state );    break;  \
                                    case 0x34:    rtl_op_bit ( state );    break;  \
                                    case 0x35:    rtl_op_and ( state );    break;  \
                                    case 0x36:    rtl_op_rol ( state );    break;  \
                                    case 0x37:    rtl_op_and ( state );    break;  \
                                    case 0x38:    rtl_op_sec ( state );    break;  \
                                    case 0x39:    rtl_op_and ( state );    break;  \
                                    case 0x3a:    rtl_op_deca( state );    break;  \
                                    case 0x3b:    rtl_op_tsc ( state );    break;  \
                                    case 0x3c:    rtl_op_bit ( state );    break;  \
                                    case 0x3d:    rtl_op_and ( state );    break;  \
                                    case 0x3e:    rtl_op_rol ( state );    break;  \
                                    case 0x3f:    rtl_op_and ( state );    break;  \
                                    case 0x40:    rtl_op_rti ( state );    break;  \
                                    case 0x41:    rtl_op_eor ( state );    break;  \
                                    case 0x42:    rtl_op_wdm ( state );    break;  \
                                    case 0x43:    rtl_op_eor ( state );    break;  \
                                    case 0x44:    rtl_op_mvp ( state );    break;  \
                                    case 0x45:    rtl_op_eor ( state );    break;  \
                                    case 0x46:    rtl_op_lsr ( state );    break;  \
                                    case 0x47:    rtl_op_eor ( state );    break;  \
                                    case 0x48:    rtl_op_pha ( state );    break;  \
                                    case 0x49:    rtl_op_eor ( state );    break;  \
                                    case 0x4a:    rtl_op_lsra( state );    break;  \
                                    case 0x4b:    rtl_op_phk ( state );    break;  \
                                    case 0x4c:    rtl_op_jmp ( state );    break;  \
                                    case 0x4d:    rtl_op_eor ( state );    break;  \
                                    case 0x4e:    rtl_op_lsr ( state );    break;  \
                                    case 0x4f:    rtl_op_eor ( state );    break;  \
                                    case 0x50:    rtl_op_bvc ( state );    break;  \
                                    case 0x51:    rtl_op_eor ( state );    break;  \
                                    case 0x52:    rtl_op_eor ( state );    break;  \
                                    case 0x53:    rtl_op_eor ( state );    break;  \
                                    case 0x54:    rtl_op_mvn ( state );    break;  \
                                    case 0x55:    rtl_op_eor ( state );    break;  \
                                    case 0x56:    rtl_op_lsr ( state );    break;  \
                                    case 0x57:    rtl_op_eor ( state );    break;  \
                                    case 0x58:    rtl_op_cli ( state );    break;  \
                                    case 0x59:    rtl_op_eor ( state );    break;  \
                                    case 0x5a:    rtl_op_phy ( state );    break;  \
                                    case 0x5b:    rtl_op_tcd ( state );    break;  \
                                    case 0x5c:    rtl_op_jmp ( state );    break;  \
                                    case 0x5d:    rtl_op_eor ( state );    break;  \
                                    case 0x5e:    rtl_op_lsr ( state );    break;  \
                                    case 0x5f:    rtl_op_eor ( state );    break;  \
                                    case 0x60:    rtl_op_rts ( state );    break;  \
                                    case 0x61:    rtl_op_adc ( state );    break;  \
                                    case 0x62:    rtl_op_per ( state );    break;  \
                                    case 0x63:    rtl_op_adc ( state );    break;  \
                                    case 0x64:    rtl_op_stz ( state );    break;  \
                                    case 0x65:    rtl_op_adc ( state );    break;  \
                                    case 0x66:    rtl_op_ror ( state );    break;  \
                                    case 0x67:    rtl_op_adc ( state );    break;  \
                                    case 0x68:    rtl_op_pla ( state );    break;  \
                                    case 0x69:    rtl_op_adc ( state );    break;  \
                                    case 0x6a:    rtl_op_rora( state );    break;  \
                                    case 0x6b:    rtl_op_rtl ( state );    break;  \
                                    case 0x6c:    rtl_op_jmp ( state );    break;  \
                                    case 0x6d:    rtl_op_adc ( state );    break;  \
                                    case 0x6e:    rtl_op_ror ( state );    break;  \
                                    case 0x6f:    rtl_op_adc ( state );    break;  \
                                    case 0x70:    rtl_op_bvs ( state );    break;  \
                                    case 0x71:    rtl_op_adc ( state );    break;  \
                                    case 0x72:    rtl_op_adc ( state );    break;  \
                                    case 0x73:    rtl_op_adc ( state );    break;  \
                                    case 0x74:    rtl_op_stz ( state );    break;  \
                                    case 0x75:    rtl_op_adc ( state );    break;  \
                                    case 0x76:    rtl_op_ror ( state );    break;  \
                                    case 0x77:    rtl_op_adc ( state );    break;  \
                                    case 0x78:    rtl_op_sei ( state );    break;  \
                                    case 0x79:    rtl_op_adc ( state );    break;  \
                                    case 0x7a:    rtl_op_ply ( state );    break;  \
                                    case 0x7b:    rtl_op_tdc ( state );    break;  \
                                    case 0x7c:    rtl_op_jmp ( state );    break;  \
                                    case 0x7d:    rtl_op_adc ( state );    break;  \
                                    case 0x7e:    rtl_op_ror ( state );    break;  \
                                    case 0x7f:    rtl_op_adc ( state );    break;  \
                                    case 0x80:    rtl_op_bra ( state );    break;  \
                                    case 0x81:    rtl_op_sta ( state );    break;  \
                                    case 0x82:    rtl_op_brl ( state );    break;  \
                                    case 0x83:    rtl_op_sta ( state );    break;  \
                                    case 0x84:    rtl_op_sty ( state );    break;  \
                                    case 0x85:    rtl_op_sta ( state );    break;  \
                                    case 0x86:    rtl_op_stx ( state );    break;  \
                                    case 0x87:    rtl_op_sta ( state );    break;  \
                                    case 0x88:    rtl_op_dey ( state );    break;  \
                                    case 0x89:    rtl_op_biti( state );    break;  \
                                    case 0x8a:    rtl_op_txa ( state );    break;  \
                                    case 0x8b:    rtl_op_phb ( state );    break;  \
                                    case 0x8c:    rtl_op_sty ( state );    break;  \
                                    case 0x8d:    rtl_op_sta ( state );    break;  \
                                    case 0x8e:    rtl_op_stx ( state );    break;  \
                                    case 0x8f:    rtl_op_sta ( state );    break;  \
                                    case 0x90:    rtl_op_bcc ( state );    break;  \
                                    case 0x91:    rtl_op_sta ( state );    break;  \
                                    case 0x92:    rtl_op_sta ( state );    break;  \
                                    case 0x93:    rtl_op_sta ( state );    break;  \
                                    case 0x94:    rtl_op_sty ( state );    break;  \
                                    case 0x95:    rtl_op_sta ( state );    break;  \
                                    case 0x96:    rtl_op_stx ( state );    break;  \
                                    case 0x97:    rtl_op_sta ( state );    break;  \
                                    case 0x98:    rtl_op_tya ( state );    break;  \
                                    case 0x99:    rtl_op_sta ( state );    break;  \
                                    case 0x9a:    rtl_op_txs ( state );    break;  \
                                    case 0x9b:    rtl_op_txy ( state );    break;  \
                                    case 0x9c:    rtl_op_stz ( state );    break;  \
                                    case 0x9d:    rtl_op_sta ( state );    break;  \
                                    case 0x9e:    rtl_op_stz ( state );    break;  \
                                    case 0x9f:    rtl_op_sta ( state );    break;  \
                                    case 0xa0:    rtl_op_ldy ( state );    break;  \
                                    case 0xa1:    rtl_op_lda ( state );    break;  \
                                    case 0xa2:    rtl_op_ldx ( state );    break;  \
                                    case 0xa3:    rtl_op_lda ( state );    break;  \
                                    case 0xa4:    rtl_op_ldy ( state );    break;  \
                                    case 0xa5:    rtl_op_lda ( state );    break;  \
                                    case 0xa6:    rtl_op_ldx ( state );    break;  \
                                    case 0xa7:    rtl_op_lda ( state );    break;  \
                                    case 0xa8:    rtl_op_tay ( state );    break;  \
                                    case 0xa9:    rtl_op_lda ( state );    break;  \
                                    case 0xaa:    rtl_op_tax ( state );    break;  \
                                    case 0xab:    rtl_op_plb ( state );    break;  \
                                    case 0xac:    rtl_op_ldy ( state );    break;  \
                                    case 0xad:    rtl_op_lda ( state );    break;  \
                                    case 0xae:    rtl_op_ldx ( state );    break;  \
                                    case 0xaf:    rtl_op_lda ( state );    break;  \
                                    case 0xb0:    rtl_op_bcs ( state );    break;  \
                                    case 0xb1:    rtl_op_lda ( state );    break;  \
                                    case 0xb2:    rtl_op_lda ( state );    break;  \
                                    case 0xb3:    rtl_op_lda ( state );    break;  \
                                    case 0xb4:    rtl_op_ldy ( state );    break;  \
                                    case 0xb5:    rtl_op_lda ( state );    break;  \
                                    case 0xb6:    rtl_op_ldx ( state );    break;  \
                                    case 0xb7:    rtl_op_lda ( state );    break;  \
                                    case 0xb8:    rtl_op_clv ( state );    break;  \
                                    case 0xb9:    rtl_op_lda ( state );    break;  \
                                    case 0xba:    rtl_op_tsx ( state );    break;  \
                                    case 0xbb:    rtl_op_tyx ( state );    break;  \
                                    case 0xbc:    rtl_op_ldy ( state );    break;  \
                                    case 0xbd:    rtl_op_lda ( state );    break;  \
                                    case 0xbe:    rtl_op_ldx ( state );    break;  \
                                    case 0xbf:    rtl_op_lda ( state );    break;  \
                                    case 0xc0:    rtl_op_cpy ( state );    break;  \
                                    case 0xc1:    rtl_op_cmp ( state );    break;  \
                                    case 0xc2:    rtl_op_rep ( state );    break;  \
                                    case 0xc3:    rtl_op_cmp ( state );    break;  \
                                    case 0xc4:    rtl_op_cpy ( state );    break;  \
                                    case 0xc5:    rtl_op_cmp ( state );    break;  \
                                    case 0xc6:    rtl_op_dec ( state );    break;  \
                                    case 0xc7:    rtl_op_cmp ( state );    break;  \
                                    case 0xc8:    rtl_op_iny ( state );    break;  \
                                    case 0xc9:    rtl_op_cmp ( state );    break;  \
                                    case 0xca:    rtl_op_dex ( state );    break;  \
                                    case 0xcb:    rtl_op_wai ( state );    break;  \
                                    case 0xcc:    rtl_op_cpy ( state );    break;  \
                                    case 0xcd:    rtl_op_cmp ( state );    break;  \
                                    case 0xce:    rtl_op_dec ( state );    break;  \
                                    case 0xcf:    rtl_op_cmp ( state );    break;  \
                                    case 0xd0:    rtl_op_bne ( state );    break;  \
                                    case 0xd1:    rtl_op_cmp ( state );    break;  \
                                    case 0xd2:    rtl_op_cmp ( state );    break;  \
                                    case 0xd3:    rtl_op_cmp ( state );    break;  \
                                    case 0xd4:    rtl_op_pei ( state );    break;  \
                                    case 0xd5:    rtl_op_cmp ( state );    break;  \
                                    case 0xd6:    rtl_op_dec ( state );    break;  \
                                    case 0xd7:    rtl_op_cmp ( state );    break;  \
                                    case 0xd8:    rtl_op_cld ( state );    break;  \
                                    case 0xd9:    rtl_op_cmp ( state );    break;  \
                                    case 0xda:    rtl_op_phx ( state );    break;  \
                                    case 0xdb:    rtl_op_stp ( state );    break;  \
                                    case 0xdc:    rtl_op_jmp ( state );    break;  \
                                    case 0xdd:    rtl_op_cmp ( state );    break;  \
                                    case 0xde:    rtl_op_dec ( state );    break;  \
                                    case 0xdf:    rtl_op_cmp ( state );    break;  \
                                    case 0xe0:    rtl_op_cpx ( state );    break;  \
                                    case 0xe1:    rtl_op_sbc ( state );    break;  \
                                    case 0xe2:    rtl_op_sep ( state );    break;  \
                                    case 0xe3:    rtl_op_sbc ( state );    break;  \
                                    case 0xe4:    rtl_op_cpx ( state );    break;  \
                                    case 0xe5:    rtl_op_sbc ( state );    break;  \
                                    case 0xe6:    rtl_op_inc ( state );    break;  \
                                    case 0xe7:    rtl_op_sbc ( state );    break;  \
                                    case 0xe8:    rtl_op_inx ( state );    break;  \
                                    case 0xe9:    rtl_op_sbc ( state );    break;  \
                                    case 0xea:    rtl_op_nop ( state );    break;  \
                                    case 0xeb:    rtl_op_xba ( state );    break;  \
                                    case 0xec:    rtl_op_cpx ( state );    break;  \
                                    case 0xed:    rtl_op_sbc ( state );    break;  \
                                    case 0xee:    rtl_op_inc ( state );    break;  \
                                    case 0xef:    rtl_op_sbc ( state );    break;  \
                                    case 0xf0:    rtl_op_beq ( state );    break;  \
                                    case 0xf1:    rtl_op_sbc ( state );    break;  \
                                    case 0xf2:    rtl_op_sbc ( state );    break;  \
                                    case 0xf3:    rtl_op_sbc ( state );    break;  \
                                    case 0xf4:    rtl_op_pea ( state );    break;  \
                                    case 0xf5:    rtl_op_sbc ( state );    break;  \
                                    case 0xf6:    rtl_op_inc ( state );    break;  \
                                    case 0xf7:    rtl_op_sbc ( state );    break;  \
                                    case 0xf8:    rtl_op_sed ( state );    break;  \
                                    case 0xf9:    rtl_op_sbc ( state );    break;  \
                                    case 0xfa:    rtl_op_plx ( state );    break;  \
                                    case 0xfb:    rtl_op_xce ( state );    break;  \
                                    case 0xfc:    rtl_op_jsr ( state );    break;  \
                                    case 0xfd:    rtl_op_sbc ( state );    break;  \
                                    case 0xfe:    rtl_op_inc ( state );    break;  \
                                    case 0xff:    rtl_op_sbc ( state );    break;  \
                                    }

#endif /* rtl_instr_exec_816_h */
