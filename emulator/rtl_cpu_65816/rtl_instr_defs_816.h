#ifndef instr_defs_h_
#define instr_defs_h_

#define CPU_REG_X           ( state->regs.e ? CPU_REG_X_8 : state->regs.x.w )
#define CPU_REG_Y           ( state->regs.e ? state->regs.y.b : CPU_REG_Y_16 )
#define CPU_REG_X_16        ( state->regs.x.w )
#define CPU_REG_Y_16        ( state->regs.y.w )
#define CPU_REG_X_8         ( state->regs.x.b )
#define CPU_REG_Y_8         ( state->regs.y.b )
#define CLAMP_ADDR          ( state->regs.e ? 0x0FF : 0x0FFFFFF )

#define direct_page         dp.w
#define stack_pointer       sp.w
#define stack_pointer8      sp.b

#define flag_e              e
#define flag_x              p.f_x
#define flag_m              p.f_m

#define INC_PC(i)           state->regs.pc += (i)
#define INC_CYCLE(i)        state->regs.cycles += (i)

//////////////////////////////////////////////////////////////////////////////////////

#define SETUP_FETCH_BYTE( state, address )                              \
    state->regs.cycles = 0;                                             \
    state->is_implicity = FALSE;                                        \
    if( !state->bus_busy ) state->exec_state++;                         \
    if( state->bus_busy == TRUE ) {                                     \
        state->instr_state  = Instr_State_BusRead;                      \
        state->xfer_size    = 1;                                        \
        state->addr_req     = address; } else

#define SETUP_READ_BYTE( state, address )                               \
    if( !state->bus_busy ) state->exec_state++;                         \
    if( state->bus_busy == TRUE ) {                                     \
        state->instr_state  = Instr_State_BusRead;                      \
        state->xfer_size    = 1;                                        \
        state->addr_req     = address; } else

#define SETUP_WRITE_BYTE( state, address, data )                        \
    if( !state->bus_busy ) state->exec_state++;                         \
    if( state->bus_busy == TRUE ) {                                     \
        state->instr_state  = Instr_State_BusWrite;                     \
        state->xfer_size    = 1;                                        \
        state->data_req     = ( data ) >> 8;                            \
        state->data_xfer    = ( data ) & 0x0FF;                         \
        state->addr_req     = address; } else

#define SETUP_NOP_CYCLE( state )                                        \
    if( !state->bus_busy ) state->exec_state++;                         \
    if( state->bus_busy == TRUE ) {                                     \
        state->instr_state  = Instr_State_Internal; } else

#define STORE_READ_LSB( state )                                         \
    state->addr_instr   = state->data_xfer

#define STORE_READ_MSB( state )                                         \
    state->addr_instr   |= ( state->data_xfer << 8 );

#define STORE_READ_HSB( state )                                         \
    state->addr_instr   |= ( state->data_xfer << 16 );

#define SETUP_NEXT_WRITE( stata )                                       \
    if( !state->bus_busy ) state->exec_state++;                         \
    if( state->bus_busy == TRUE ) {                                     \
        state->data_xfer    = state->data_req & 0x0FF;                  \
        state->instr_state  = Instr_State_BusWrite;                     \
        state->data_req     >>= 8;                                      \
        state->addr_req++; } else

#define SETUP_NEXT_READ( stata )                                        \
    if( !state->bus_busy ) state->exec_state++;                         \
    if( state->bus_busy == TRUE ) {                                     \
        state->instr_state  = Instr_State_BusRead;                      \
        state->addr_req++; } else

#define  TERMINATE_FETCH( state, add_pc )                                           \
    state->instr_state  = Instr_State_Done;                                         \
    state->regs.cycles = ( state->exec_state + 1 - Exec_State_Fetch ) >> 1;         \
    state->exec_state = Exec_State_Addressing;                                      \
    state->exec_opcode = state->data_xfer;                                          \
    INC_PC( add_pc )

#define  TERMINATE_ADDRESSING( state, add_pc )                                      \
    state->instr_state  = Instr_State_Done;                                         \
    state->regs.cycles  += ( state->exec_state + 1 - Exec_State_Addressing ) >> 1;  \
    state->exec_state = Exec_State_Execute;                                         \
    INC_PC( add_pc )

#define TERMINATE_EXECUTE( state )                                                  \
    state->instr_state  = Instr_State_Done;                                         \
    state->regs.cycles += ( state->exec_state + 1 - Exec_State_Execute ) >> 1;      \
    state->exec_state   = Exec_State_Fetch

//////////////////////////////////////////////////////////////////////////////////////

enum
{
    Instr_Operation_ADC,
    Instr_Operation_SBC,
    Instr_Operation_AND,
    Instr_Operation_OR,
    Instr_Operation_EOR,
    Instr_Operation_ASL,
    Instr_Operation_LSR,
    Instr_Operation_ROR,
    Instr_Operation_ROL,
    Instr_Operation_CMP,
    Instr_Operation_INC,
    Instr_Operation_DEC,
    Instr_Operation_JMP,
    Instr_Operation_JSL,
    Instr_Operation_JSR,
    Instr_Operation_MVN,
    Instr_Operation_MVP,
    Instr_Operation_RTS,
    Instr_Operation_RTL,
    Instr_Operation_RTI,
    Instr_Operation_BRK,
    Instr_Operation_COP,
    Instr_Operation_TRB,
    Instr_Operation_TSB,

    Instr_Operation_TAX,
    Instr_Operation_TAY,
    Instr_Operation_TCD,
    Instr_Operation_TDC,
    Instr_Operation_TCS,
    Instr_Operation_TSC,
    Instr_Operation_TSX,
    Instr_Operation_TXA,
    Instr_Operation_TXS,
    Instr_Operation_TXY,
    Instr_Operation_TYA,
    Instr_Operation_TYX,

    Instr_Operation_C0,
    Instr_Operation_C1,
    Instr_Operation_V0,
    Instr_Operation_V1,
    Instr_Operation_N0,
    Instr_Operation_N1,
    Instr_Operation_Z0,
    Instr_Operation_Z1,
    Instr_Operation_I0,
    Instr_Operation_I1,
    Instr_Operation_D0,
    Instr_Operation_D1,
    Instr_Operation_Branch_Always,

    Instr_Mode_SizeAuto,
    Instr_Mode_8bits,
    Instr_Mode_16bits,
    Instr_Operation_Biti,
    Instr_Operation_BitNV,

    Instr_Mode_Operand,
    Instr_Mode_Operand_Indirect,
};

typedef enum
{
    Instr_Mode_ACU,
    Instr_Mode_REGX,
    Instr_Mode_REGY,
    Instr_Mode_SP,
    Instr_Mode_FLAGS,
    Instr_Mode_DP,
    Instr_Mode_DBR,
    Instr_Mode_PBR,
    Instr_Mode_ZERO,
    
} reg_instr_t;

#endif
