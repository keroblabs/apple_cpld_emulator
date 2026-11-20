#include "board.h"
#include "disasm_816.h"

#define CPU_REG_X_16            ( 0 )
#define CPU_REG_Y_16            ( 0 )
#define CPU_REG_X_8             ( 0 )
#define CPU_REG_Y_8             ( 0 )

#define TEXT(a)                 a

#define CPU_REG_X               ( 0 )
#define CPU_REG_Y               ( 0 )

#define CLAMP_ADDR              ( 0 )

#define stack_pointer8          sp & 0x0FF
#define stack_pointer           sp

#define lo(b)                   ((uint8_t)((b) & 0x0FF))
#define hi(b)                   ((uint8_t)(((b) >> 8) & 0x0FF))
#define swap(v)                 ((((uint16_t)(v)) << 8) | (((uint16_t)(v)) >> 8))
#define join16(h,l)             (((uint16_t)((h)&0x0FF)<<8)  | ((l)&0x0FF))
#define bank(b)                 ((uint32_t)((b)&0x0FF)<<16)

#define INC_PC(i)               instr_num_bytes += (i)
#define INC_CYCLE(i)

#ifdef ADDR_MODE
#undef ADDR_MODE
#endif
#define ADDR_MODE(a)            instr_addr_mode = (a)

#define SHOW_REG_NONE           0
#define SHOW_FLAGS              (1 << 0)
#define SHOW_REG_ACC            (1 << 1)
#define SHOW_REG_X              (1 << 2)
#define SHOW_REG_Y              (1 << 3)
#define SHOW_MEM                (1 << 4)
#define SHOW_STACK              (1 << 5)
#define SHOW_REL                (1 << 6)
#define SHOW_JUMP               (1 << 7)
#define SHOW_RETURN             (1 << 8)

#define  ADDR_IMPLIED           0
#define  ADDR_INVALID1          1
#define  ADDR_INVALID2          2
#define  ADDR_INVALID3          3
#define  ADDR_IMM               4
#define  ADDR_ABS               5
#define  ADDR_ZPG               6
#define  ADDR_ABSX              7
#define  ADDR_ABSY              8
#define  ADDR_ZPGX              9
#define  ADDR_ZPGY              10
#define  ADDR_REL               11
#define  ADDR_INDX              12
#define  ADDR_ABSIINDX          13
#define  ADDR_INDY              14
#define  ADDR_IZPG              15
#define  ADDR_IABS              16
#define  ADDR_ABSL              17
#define  ADDR_ABSLX             18
#define  ADDR_ABSLY             19
#define  ADDR_IABSL             20
#define  ADDR_IABSLX            21
#define  ADDR_IZPGL             22
#define  ADDR_IZPGLY            23
#define  ADDR_IMMW              24
#define  ADDR_LREL              25
#define  ADDR_SREL              26
#define  ADDR_YREL              27

typedef struct _addrrec
{
    const char format[12];
    uint8_t    bytes;
    bool_t     show_address;
    uint32_t   show_comment;
} addrrec_t;

typedef struct _general_regs_t_
{
    uint16_t acu;
    uint16_t reg_x;
    uint16_t reg_y;
    uint32_t pc;
    uint32_t sp;
    
    uint8_t data_page;
    uint8_t code_page;
    uint16_t direct_page;

    bool_t flag_c;
    bool_t flag_n;
    bool_t flag_v;
    bool_t flag_z;
    
    bool_t flag_e;
    bool_t flag_m;
    bool_t flag_x;

} debug_regs_t;

static INLINE uint8_t cpu_read_8( uint32_t addr )
{
    return 0;
}

static INLINE uint16_t cpu_read_16( uint32_t addr )
{
    return 0;
}

static INLINE uint32_t cpu_read_24( uint32_t addr )
{
    return 0;
}

static addrrec_t addressmode[]    = {    {TEXT("        ")        ,1, FALSE, SHOW_REG_NONE },        // (implied)
                                         {TEXT("        ")        ,1, FALSE, SHOW_REG_NONE },        // INVALID1
                                         {TEXT("        ")        ,2, FALSE, SHOW_REG_NONE },        // INVALID2
                                         {TEXT("        ")        ,3, FALSE, SHOW_REG_NONE },        // INVALID3
                                         {TEXT("#$%02X    ")      ,2, FALSE, SHOW_REG_NONE },        // ADDR_IMM
                                         {TEXT("$%04X   ")        ,3, TRUE , SHOW_MEM | SHOW_REG_NONE },        // ADDR_ABS
                                         {TEXT("$%02X     ")      ,2, TRUE , SHOW_MEM | SHOW_REG_NONE },        // ADDR_ZPG
                                         {TEXT("$%04X,X ")        ,3, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_ABSX
                                         {TEXT("$%04X,Y ")        ,3, TRUE , SHOW_MEM | SHOW_REG_Y    },        // ADDR_ABSY
                                         {TEXT("$%02X,X   ")      ,2, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_ZPGX
                                         {TEXT("$%02X,Y   ")      ,2, TRUE , SHOW_MEM | SHOW_REG_Y    },        // ADDR_ZPGY
                                         {TEXT("$%04X   ")        ,2, TRUE , SHOW_REL | SHOW_FLAGS    },        // ADDR_REL
                                         {TEXT("($%02X,X)  ")     ,2, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_INDX
                                         {TEXT("($%04X,X) ")      ,3, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_ABSIINDX
                                         {TEXT("($%02X),Y ")      ,2, TRUE , SHOW_MEM | SHOW_REG_Y    },        // ADDR_INDY
                                         {TEXT("($%02X)    ")     ,2, TRUE , SHOW_MEM | SHOW_REG_NONE },        // ADDR_IZPG
                                         {TEXT("($%04X) ")        ,3, TRUE , SHOW_MEM | SHOW_REG_NONE },        // ADDR_IABS

                                         {TEXT("$%06X ")          ,4, TRUE , SHOW_MEM | SHOW_REG_NONE },        // ADDR_ABSL
                                         {TEXT("($%06X),X ")      ,4, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_ABSLX
                                         {TEXT("($%06X),Y ")      ,4, TRUE , SHOW_MEM | SHOW_REG_Y    },        // ADDR_ABSLY
                                         {TEXT("($%06X) ")        ,4, TRUE , SHOW_MEM | SHOW_REG_NONE },        // ADDR_IABSL
                                         {TEXT("$%06X,X ")        ,4, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_IABSLX
                                         {TEXT("[$%02X]    ")     ,2, TRUE , SHOW_MEM | SHOW_REG_NONE },        // ADDR_IZPGL
                                         {TEXT("[$%02X],Y ")      ,2, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_IZPGLX
                                         {TEXT("#$%04X    ")      ,3, FALSE, SHOW_REG_NONE },                   // ADDR_IMMW
                                         {TEXT("$%06X   ")        ,2, TRUE , SHOW_REL | SHOW_FLAGS    },        // ADDR_LREL
                                         {TEXT("$%04X,S ")        ,2, TRUE , SHOW_REL | SHOW_FLAGS    },        // ADDR_SREL
                                         {TEXT("($%04X,S),Y ")    ,2, TRUE , SHOW_REL | SHOW_REG_Y    },        // ADDR_YREL
};

static uint8_t instr_num_bytes;
static uint8_t instr_addr_mode;
static uint32_t disasm_comment;
static uint32_t comment_addr;
// static rtl_access_t *disasm_state;
static uint8_t op_size;

//static debug_regs_t regs;
static char str_opcode[10];

extern char debug_temp_string[];

#include "instr_defs_am_816.h"
#include "instr_disasm_816.h"
#include "instr_code_816.h"


static void generate_comment_abs_addr( rtl_access_t * state, bool_t show_address, char * output )
{
    char reg_str[40];

    if( show_address )
    {
        uint8_t index = state->instr_index - 1;
        sprintf( output, "$%04x - 0x%02x", state->instr_trace[index].instr_addr, 
                                        state->instr_trace[index].instr_bytes );
    }

    if( disasm_comment & SHOW_REG_X )
    {
        sprintf( reg_str, " (REG.X = 0x%02X)", state->regs.x.b );
        strcat( output, reg_str );
    }
    
    if( disasm_comment & SHOW_REG_Y )
    {
        sprintf( reg_str, " (REG.Y = 0x%02X)", state->regs.y.b );
        strcat( output, reg_str );
    }
    
    if( disasm_comment & SHOW_REG_ACC )
    {
        sprintf( reg_str, " (ACC = 0x%02X)", state->regs.a.b );
        strcat( output, reg_str );
    }
    
    if( disasm_comment & SHOW_FLAGS )
    {
        sprintf( reg_str, " (C=%d S=%d V=%d Z=%d)", state->regs.p.f_c, state->regs.p.f_n, state->regs.p.f_v, state->regs.p.f_z );
        strcat( output, reg_str );
    }

}

void disasm_instruction_816( rtl_access_t * state )
{
    char addr_str[20];
    char asm_code[40];
    char comment_str[80];

    if( state->disasm_enabled == FALSE ) return;

    uint32_t curr_pc = state->instr_trace[0].instr_addr;

    uint8_t opcode = state->instr_trace[0].instr_bytes;
    uint8_t byte1 = state->instr_trace[1].instr_bytes;
    uint8_t byte2 = state->instr_trace[2].instr_bytes;
    uint8_t byte3 = state->instr_trace[3].instr_bytes;

    instr_num_bytes = 0;
    disasm_comment = 0;
    instr_addr_mode = 0;
    comment_addr = 0;
    op_size = ( state->regs.e || state->regs.p.f_m ) ? 1 : 2;
    
    INSTR_DECODE( opcode );
    instr_num_bytes++;
    
    switch( instr_num_bytes )
    {
        case 1:
            sprintf( asm_code, "$%02X.%04X: %02X         ", state->regs.pbr, curr_pc, opcode );
            sprintf( addr_str, addressmode[instr_addr_mode].format );
            break;

        case 2:
            sprintf( asm_code, "$%02X.%04X: %02X %02X      ", state->regs.pbr, curr_pc, opcode, byte1 );

            if( instr_addr_mode == ADDR_REL )
            {
                int target_addr = ( int )curr_pc + ( char )byte1 + instr_num_bytes;
                sprintf( addr_str, addressmode[instr_addr_mode].format, target_addr );
            }
            else
            {
                sprintf( addr_str, addressmode[instr_addr_mode].format, byte1 );
            }
            break;

        case 3:
            sprintf( asm_code, "$%02X.%04X: %02X %02X %02X   ", state->regs.pbr, curr_pc, opcode, byte1, byte2 );

            {
                uint16_t val16;

                val16 = byte2;
                val16 = ( val16 << 8 ) + byte1;
                sprintf( addr_str, addressmode[instr_addr_mode].format, val16 );
            }
            break;

        case 4:
            sprintf( asm_code, "$%02X.%04X: %02X %02X %02X %02X", state->regs.pbr, curr_pc, opcode, byte1, byte2, byte3 );

            {
                uint32_t val24;

                val24 = byte3;
                val24 = ( val24 << 8 ) + byte2;
                val24 = ( val24 << 8 ) + byte1;
                sprintf( addr_str, addressmode[instr_addr_mode].format, val24 );
            }
            break;
    }
    
    strcpy( comment_str, "" );
    generate_comment_abs_addr( state, addressmode[instr_addr_mode].show_address, comment_str );
    
    printf( "%s %s %s ;%s\n", asm_code, str_opcode, addr_str, comment_str );
}
