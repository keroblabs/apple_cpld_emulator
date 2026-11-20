//
//  rtl_instr_code_am_816.h
//  AppleIIGS Emulator
//
//  Created by Marcio Cherobim on 24/04/2024.
//  Copyright © 2024 Marcio Cherobim. All rights reserved.
//

#ifndef rtl_instr_code_am_816_h
#define rtl_instr_code_am_816_h

#define rtl_am_absl( state )        am_absl_xy( state, NO_REG )
#define rtl_am_absx( state )        am_absl_xy( state, CPU_REG_X )
#define rtl_am_absy( state )        am_absl_xy( state, CPU_REG_Y )
#define rtl_am_absi( state )        am_absl_indirect( state, NO_BANK, NO_REG )
#define rtl_am_abxi( state )        am_absl_indirect( state, bank_pbr, CPU_REG_X )
#define rtl_am_alng( state )        am_alng_x( state, NO_REG )
#define rtl_am_alnx( state )        am_alng_x( state, CPU_REG_X )
#define rtl_am_abil( state )        am_absl_indirect_long( state )
#define rtl_am_dpag( state )        am_direct_page( state, NO_REG )
#define rtl_am_dpgx( state )        am_direct_page( state, CPU_REG_X_8 )
#define rtl_am_dpgy( state )        am_direct_page( state, CPU_REG_Y_8 )
#define rtl_am_dpgi( state )        am_direct_page_pre_indexed( state, NO_REG )
#define rtl_am_dpix( state )        am_direct_page_pre_indexed( state, CPU_REG_X )
#define rtl_am_dpiy( state )        am_direct_page_indexed( state, CPU_REG_Y )
#define rtl_am_dpil( state )        am_direct_page_long_indirect( state, NO_REG )
#define rtl_am_dily( state )        am_direct_page_long_indirect( state, CPU_REG_Y )
#define rtl_am_impl( state )        am_implicity( state )
#define rtl_am_iacc( state )        am_implicity( state )
#define rtl_am_immb( state )        am_immediate_b( state )
#define rtl_am_immw( state )        am_immediate_w( state )
#define rtl_am_immm( state )        am_immediate( state )
#define rtl_am_immx( state )        am_immediate_xy( state )
#define rtl_am_lrel( state )        am_relative( state, TRUE )
#define rtl_am_rela( state )        am_relative( state, FALSE )
#define rtl_am_srel( state )        am_stack_relative( state, FALSE )
#define rtl_am_sriy( state )        am_stack_relative( state, TRUE )


#define RTL_INSTR_AM_DECODE()   switch ( state->exec_opcode ) {                    \
                                    case 0x00:    rtl_am_immb( state );    break;  \
                                    case 0x01:    rtl_am_dpix( state );    break;  \
                                    case 0x02:    rtl_am_immb( state );    break;  \
                                    case 0x03:    rtl_am_srel( state );    break;  \
                                    case 0x04:    rtl_am_dpag( state );    break;  \
                                    case 0x05:    rtl_am_dpag( state );    break;  \
                                    case 0x06:    rtl_am_dpag( state );    break;  \
                                    case 0x07:    rtl_am_dpil( state );    break;  \
                                    case 0x08:    rtl_am_impl( state );    break;  \
                                    case 0x09:    rtl_am_immm( state );    break;  \
                                    case 0x0a:    rtl_am_iacc( state );    break;  \
                                    case 0x0b:    rtl_am_impl( state );    break;  \
                                    case 0x0c:    rtl_am_absl( state );    break;  \
                                    case 0x0d:    rtl_am_absl( state );    break;  \
                                    case 0x0e:    rtl_am_absl( state );    break;  \
                                    case 0x0f:    rtl_am_alng( state );    break;  \
                                    case 0x10:    rtl_am_rela( state );    break;  \
                                    case 0x11:    rtl_am_dpiy( state );    break;  \
                                    case 0x12:    rtl_am_dpgi( state );    break;  \
                                    case 0x13:    rtl_am_sriy( state );    break;  \
                                    case 0x14:    rtl_am_dpag( state );    break;  \
                                    case 0x15:    rtl_am_dpgx( state );    break;  \
                                    case 0x16:    rtl_am_dpgx( state );    break;  \
                                    case 0x17:    rtl_am_dily( state );    break;  \
                                    case 0x18:    rtl_am_impl( state );    break;  \
                                    case 0x19:    rtl_am_absy( state );    break;  \
                                    case 0x1a:    rtl_am_iacc( state );    break;  \
                                    case 0x1b:    rtl_am_impl( state );    break;  \
                                    case 0x1c:    rtl_am_absl( state );    break;  \
                                    case 0x1d:    rtl_am_absx( state );    break;  \
                                    case 0x1e:    rtl_am_absx( state );    break;  \
                                    case 0x1f:    rtl_am_alnx( state );    break;  \
                                    case 0x20:    rtl_am_absl( state );    break;  \
                                    case 0x21:    rtl_am_dpix( state );    break;  \
                                    case 0x22:    rtl_am_alng( state );    break;  \
                                    case 0x23:    rtl_am_srel( state );    break;  \
                                    case 0x24:    rtl_am_dpag( state );    break;  \
                                    case 0x25:    rtl_am_dpag( state );    break;  \
                                    case 0x26:    rtl_am_dpag( state );    break;  \
                                    case 0x27:    rtl_am_dpil( state );    break;  \
                                    case 0x28:    rtl_am_impl( state );    break;  \
                                    case 0x29:    rtl_am_immm( state );    break;  \
                                    case 0x2a:    rtl_am_iacc( state );    break;  \
                                    case 0x2b:    rtl_am_impl( state );    break;  \
                                    case 0x2c:    rtl_am_absl( state );    break;  \
                                    case 0x2d:    rtl_am_absl( state );    break;  \
                                    case 0x2e:    rtl_am_absl( state );    break;  \
                                    case 0x2f:    rtl_am_alng( state );    break;  \
                                    case 0x30:    rtl_am_rela( state );    break;  \
                                    case 0x31:    rtl_am_dpiy( state );    break;  \
                                    case 0x32:    rtl_am_dpgi( state );    break;  \
                                    case 0x33:    rtl_am_sriy( state );    break;  \
                                    case 0x34:    rtl_am_dpgx( state );    break;  \
                                    case 0x35:    rtl_am_dpgx( state );    break;  \
                                    case 0x36:    rtl_am_dpgx( state );    break;  \
                                    case 0x37:    rtl_am_dily( state );    break;  \
                                    case 0x38:    rtl_am_impl( state );    break;  \
                                    case 0x39:    rtl_am_absy( state );    break;  \
                                    case 0x3a:    rtl_am_iacc( state );    break;  \
                                    case 0x3b:    rtl_am_impl( state );    break;  \
                                    case 0x3c:    rtl_am_absx( state );    break;  \
                                    case 0x3d:    rtl_am_absx( state );    break;  \
                                    case 0x3e:    rtl_am_absx( state );    break;  \
                                    case 0x3f:    rtl_am_alnx( state );    break;  \
                                    case 0x40:    rtl_am_impl( state );    break;  \
                                    case 0x41:    rtl_am_dpix( state );    break;  \
                                    case 0x42:    rtl_am_immb( state );    break;  \
                                    case 0x43:    rtl_am_srel( state );    break;  \
                                    case 0x44:    rtl_am_immw( state );    break;  \
                                    case 0x45:    rtl_am_dpag( state );    break;  \
                                    case 0x46:    rtl_am_dpag( state );    break;  \
                                    case 0x47:    rtl_am_dpil( state );    break;  \
                                    case 0x48:    rtl_am_impl( state );    break;  \
                                    case 0x49:    rtl_am_immm( state );    break;  \
                                    case 0x4a:    rtl_am_impl( state );    break;  \
                                    case 0x4b:    rtl_am_impl( state );    break;  \
                                    case 0x4c:    rtl_am_absl( state );    break;  \
                                    case 0x4d:    rtl_am_absl( state );    break;  \
                                    case 0x4e:    rtl_am_absl( state );    break;  \
                                    case 0x4f:    rtl_am_alng( state );    break;  \
                                    case 0x50:    rtl_am_rela( state );    break;  \
                                    case 0x51:    rtl_am_dpiy( state );    break;  \
                                    case 0x52:    rtl_am_dpgi( state );    break;  \
                                    case 0x53:    rtl_am_sriy( state );    break;  \
                                    case 0x54:    rtl_am_immw( state );    break;  \
                                    case 0x55:    rtl_am_dpgx( state );    break;  \
                                    case 0x56:    rtl_am_dpgx( state );    break;  \
                                    case 0x57:    rtl_am_dpil( state );    break;  \
                                    case 0x58:    rtl_am_impl( state );    break;  \
                                    case 0x59:    rtl_am_absy( state );    break;  \
                                    case 0x5a:    rtl_am_impl( state );    break;  \
                                    case 0x5b:    rtl_am_impl( state );    break;  \
                                    case 0x5c:    rtl_am_alng( state );    break;  \
                                    case 0x5d:    rtl_am_absx( state );    break;  \
                                    case 0x5e:    rtl_am_absx( state );    break;  \
                                    case 0x5f:    rtl_am_alnx( state );    break;  \
                                    case 0x60:    rtl_am_impl( state );    break;  \
                                    case 0x61:    rtl_am_dpix( state );    break;  \
                                    case 0x62:    rtl_am_lrel( state );    break;  \
                                    case 0x63:    rtl_am_srel( state );    break;  \
                                    case 0x64:    rtl_am_dpag( state );    break;  \
                                    case 0x65:    rtl_am_dpag( state );    break;  \
                                    case 0x66:    rtl_am_dpag( state );    break;  \
                                    case 0x67:    rtl_am_dpil( state );    break;  \
                                    case 0x68:    rtl_am_impl( state );    break;  \
                                    case 0x69:    rtl_am_immm( state );    break;  \
                                    case 0x6a:    rtl_am_impl( state );    break;  \
                                    case 0x6b:    rtl_am_impl( state );    break;  \
                                    case 0x6c:    rtl_am_absi( state );    break;  \
                                    case 0x6d:    rtl_am_absl( state );    break;  \
                                    case 0x6e:    rtl_am_absl( state );    break;  \
                                    case 0x6f:    rtl_am_alng( state );    break;  \
                                    case 0x70:    rtl_am_rela( state );    break;  \
                                    case 0x71:    rtl_am_dpiy( state );    break;  \
                                    case 0x72:    rtl_am_dpgi( state );    break;  \
                                    case 0x73:    rtl_am_sriy( state );    break;  \
                                    case 0x74:    rtl_am_dpgx( state );    break;  \
                                    case 0x75:    rtl_am_dpgx( state );    break;  \
                                    case 0x76:    rtl_am_dpgx( state );    break;  \
                                    case 0x77:    rtl_am_dily( state );    break;  \
                                    case 0x78:    rtl_am_impl( state );    break;  \
                                    case 0x79:    rtl_am_absy( state );    break;  \
                                    case 0x7a:    rtl_am_impl( state );    break;  \
                                    case 0x7b:    rtl_am_impl( state );    break;  \
                                    case 0x7c:    rtl_am_abxi( state );    break;  \
                                    case 0x7d:    rtl_am_absx( state );    break;  \
                                    case 0x7e:    rtl_am_absx( state );    break;  \
                                    case 0x7f:    rtl_am_alnx( state );    break;  \
                                    case 0x80:    rtl_am_rela( state );    break;  \
                                    case 0x81:    rtl_am_dpix( state );    break;  \
                                    case 0x82:    rtl_am_lrel( state );    break;  \
                                    case 0x83:    rtl_am_srel( state );    break;  \
                                    case 0x84:    rtl_am_dpag( state );    break;  \
                                    case 0x85:    rtl_am_dpag( state );    break;  \
                                    case 0x86:    rtl_am_dpag( state );    break;  \
                                    case 0x87:    rtl_am_dpil( state );    break;  \
                                    case 0x88:    rtl_am_impl( state );    break;  \
                                    case 0x89:    rtl_am_immm( state );    break;  \
                                    case 0x8a:    rtl_am_impl( state );    break;  \
                                    case 0x8b:    rtl_am_impl( state );    break;  \
                                    case 0x8c:    rtl_am_absl( state );    break;  \
                                    case 0x8d:    rtl_am_absl( state );    break;  \
                                    case 0x8e:    rtl_am_absl( state );    break;  \
                                    case 0x8f:    rtl_am_alng( state );    break;  \
                                    case 0x90:    rtl_am_rela( state );    break;  \
                                    case 0x91:    rtl_am_dpiy( state );    break;  \
                                    case 0x92:    rtl_am_dpgi( state );    break;  \
                                    case 0x93:    rtl_am_sriy( state );    break;  \
                                    case 0x94:    rtl_am_dpgx( state );    break;  \
                                    case 0x95:    rtl_am_dpgx( state );    break;  \
                                    case 0x96:    rtl_am_dpgy( state );    break;  \
                                    case 0x97:    rtl_am_dily( state );    break;  \
                                    case 0x98:    rtl_am_impl( state );    break;  \
                                    case 0x99:    rtl_am_absy( state );    break;  \
                                    case 0x9a:    rtl_am_impl( state );    break;  \
                                    case 0x9b:    rtl_am_impl( state );    break;  \
                                    case 0x9c:    rtl_am_absl( state );    break;  \
                                    case 0x9d:    rtl_am_absx( state );    break;  \
                                    case 0x9e:    rtl_am_absx( state );    break;  \
                                    case 0x9f:    rtl_am_alnx( state );    break;  \
                                    case 0xa0:    rtl_am_immx( state );    break;  \
                                    case 0xa1:    rtl_am_dpix( state );    break;  \
                                    case 0xa2:    rtl_am_immx( state );    break;  \
                                    case 0xa3:    rtl_am_srel( state );    break;  \
                                    case 0xa4:    rtl_am_dpag( state );    break;  \
                                    case 0xa5:    rtl_am_dpag( state );    break;  \
                                    case 0xa6:    rtl_am_dpag( state );    break;  \
                                    case 0xa7:    rtl_am_dpil( state );    break;  \
                                    case 0xa8:    rtl_am_impl( state );    break;  \
                                    case 0xa9:    rtl_am_immm( state );    break;  \
                                    case 0xaa:    rtl_am_impl( state );    break;  \
                                    case 0xab:    rtl_am_impl( state );    break;  \
                                    case 0xac:    rtl_am_absl( state );    break;  \
                                    case 0xad:    rtl_am_absl( state );    break;  \
                                    case 0xae:    rtl_am_absl( state );    break;  \
                                    case 0xaf:    rtl_am_alng( state );    break;  \
                                    case 0xb0:    rtl_am_rela( state );    break;  \
                                    case 0xb1:    rtl_am_dpiy( state );    break;  \
                                    case 0xb2:    rtl_am_dpgi( state );    break;  \
                                    case 0xb3:    rtl_am_sriy( state );    break;  \
                                    case 0xb4:    rtl_am_dpgx( state );    break;  \
                                    case 0xb5:    rtl_am_dpgx( state );    break;  \
                                    case 0xb6:    rtl_am_dpgy( state );    break;  \
                                    case 0xb7:    rtl_am_dily( state );    break;  \
                                    case 0xb8:    rtl_am_impl( state );    break;  \
                                    case 0xb9:    rtl_am_absy( state );    break;  \
                                    case 0xba:    rtl_am_impl( state );    break;  \
                                    case 0xbb:    rtl_am_impl( state );    break;  \
                                    case 0xbc:    rtl_am_absx( state );    break;  \
                                    case 0xbd:    rtl_am_absx( state );    break;  \
                                    case 0xbe:    rtl_am_absy( state );    break;  \
                                    case 0xbf:    rtl_am_alnx( state );    break;  \
                                    case 0xc0:    rtl_am_immx( state );    break;  \
                                    case 0xc1:    rtl_am_dpix( state );    break;  \
                                    case 0xc2:    rtl_am_immb( state );    break;  \
                                    case 0xc3:    rtl_am_srel( state );    break;  \
                                    case 0xc4:    rtl_am_dpag( state );    break;  \
                                    case 0xc5:    rtl_am_dpag( state );    break;  \
                                    case 0xc6:    rtl_am_dpag( state );    break;  \
                                    case 0xc7:    rtl_am_dpil( state );    break;  \
                                    case 0xc8:    rtl_am_impl( state );    break;  \
                                    case 0xc9:    rtl_am_immm( state );    break;  \
                                    case 0xca:    rtl_am_impl( state );    break;  \
                                    case 0xcb:    rtl_am_impl( state );    break;  \
                                    case 0xcc:    rtl_am_absl( state );    break;  \
                                    case 0xcd:    rtl_am_absl( state );    break;  \
                                    case 0xce:    rtl_am_absl( state );    break;  \
                                    case 0xcf:    rtl_am_alng( state );    break;  \
                                    case 0xd0:    rtl_am_rela( state );    break;  \
                                    case 0xd1:    rtl_am_dpiy( state );    break;  \
                                    case 0xd2:    rtl_am_dpgi( state );    break;  \
                                    case 0xd3:    rtl_am_sriy( state );    break;  \
                                    case 0xd4:    rtl_am_dpag( state );    break;  \
                                    case 0xd5:    rtl_am_dpgx( state );    break;  \
                                    case 0xd6:    rtl_am_dpgx( state );    break;  \
                                    case 0xd7:    rtl_am_dily( state );    break;  \
                                    case 0xd8:    rtl_am_impl( state );    break;  \
                                    case 0xd9:    rtl_am_absy( state );    break;  \
                                    case 0xda:    rtl_am_impl( state );    break;  \
                                    case 0xdb:    rtl_am_impl( state );    break;  \
                                    case 0xdc:    rtl_am_abil( state );    break;  \
                                    case 0xdd:    rtl_am_absx( state );    break;  \
                                    case 0xde:    rtl_am_absx( state );    break;  \
                                    case 0xdf:    rtl_am_alnx( state );    break;  \
                                    case 0xe0:    rtl_am_immx( state );    break;  \
                                    case 0xe1:    rtl_am_dpix( state );    break;  \
                                    case 0xe2:    rtl_am_immb( state );    break;  \
                                    case 0xe3:    rtl_am_srel( state );    break;  \
                                    case 0xe4:    rtl_am_dpag( state );    break;  \
                                    case 0xe5:    rtl_am_dpag( state );    break;  \
                                    case 0xe6:    rtl_am_dpag( state );    break;  \
                                    case 0xe7:    rtl_am_dpil( state );    break;  \
                                    case 0xe8:    rtl_am_impl( state );    break;  \
                                    case 0xe9:    rtl_am_immm( state );    break;  \
                                    case 0xea:    rtl_am_impl( state );    break;  \
                                    case 0xeb:    rtl_am_impl( state );    break;  \
                                    case 0xec:    rtl_am_absl( state );    break;  \
                                    case 0xed:    rtl_am_absl( state );    break;  \
                                    case 0xee:    rtl_am_absl( state );    break;  \
                                    case 0xef:    rtl_am_alng( state );    break;  \
                                    case 0xf0:    rtl_am_rela( state );    break;  \
                                    case 0xf1:    rtl_am_dpiy( state );    break;  \
                                    case 0xf2:    rtl_am_dpgi( state );    break;  \
                                    case 0xf3:    rtl_am_sriy( state );    break;  \
                                    case 0xf4:    rtl_am_immw( state );    break;  \
                                    case 0xf5:    rtl_am_dpgx( state );    break;  \
                                    case 0xf6:    rtl_am_dpgx( state );    break;  \
                                    case 0xf7:    rtl_am_dily( state );    break;  \
                                    case 0xf8:    rtl_am_impl( state );    break;  \
                                    case 0xf9:    rtl_am_absy( state );    break;  \
                                    case 0xfa:    rtl_am_impl( state );    break;  \
                                    case 0xfb:    rtl_am_impl( state );    break;  \
                                    case 0xfc:    rtl_am_abxi( state );    break;  \
                                    case 0xfd:    rtl_am_absx( state );    break;  \
                                    case 0xfe:    rtl_am_absx( state );    break;  \
                                    case 0xff:    rtl_am_alnx( state );    break;  \
                                    }

#endif /* rtl_instr_code_am_816_h */
