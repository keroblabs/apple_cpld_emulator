#ifndef instr_defs_am_816_h
#define instr_defs_am_816_h

//addrrec_t addressmode[17]  = {   {TEXT("        ")        ,1, FALSE, SHOW_REG_NONE },        // (implied)
//                                 {TEXT("        ")        ,1, FALSE, SHOW_REG_NONE },        // INVALID1
//                                 {TEXT("        ")        ,2, FALSE, SHOW_REG_NONE },        // INVALID2
//                                 {TEXT("        ")        ,3, FALSE, SHOW_REG_NONE },        // INVALID3
//                                 {TEXT("#$%02X    ")    ,2, FALSE, SHOW_REG_NONE },        // ADDR_IMM
//                                 {TEXT("$%04X   ")        ,3, TRUE , SHOW_MEM | SHOW_REG_NONE },        // ADDR_ABS
//                                 {TEXT("$%02X     ")    ,2, TRUE , SHOW_MEM | SHOW_REG_NONE },        // ADDR_ZPG
//                                 {TEXT("$%04X,X ")        ,3, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_ABSX
//                                 {TEXT("$%04X,Y ")        ,3, TRUE , SHOW_MEM | SHOW_REG_Y    },        // ADDR_ABSY
//                                 {TEXT("$%02X,X   ")    ,2, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_ZPGX
//                                 {TEXT("$%02X,Y   ")    ,2, TRUE , SHOW_MEM | SHOW_REG_Y    },        // ADDR_ZPGY
//                                 {TEXT("$%04X   ")        ,2, TRUE , SHOW_REL | SHOW_FLAGS    },        // ADDR_REL
//                                 {TEXT("($%02X,X)  ")    ,2, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_INDX
//                                 {TEXT("($%04X,X) ")    ,3, TRUE , SHOW_MEM | SHOW_REG_X    },        // ADDR_ABSIINDX
//                                 {TEXT("($%02X),Y ")    ,2, TRUE , SHOW_MEM | SHOW_REG_Y    },        // ADDR_INDY
//                                 {TEXT("($%02X)    ")    ,2, TRUE , SHOW_MEM | SHOW_REG_NONE },        // ADDR_IZPG
//                                 {TEXT("($%04X) ")        ,3, TRUE , SHOW_MEM | SHOW_REG_NONE }         // ADDR_IABS
// };

//#define  ADDR_IMPLIED          0
//#define  ADDR_INVALID1        1
//#define  ADDR_INVALID2        2
//#define  ADDR_INVALID3        3
//#define  ADDR_IMM        4
//#define  ADDR_ABS        5
//#define  ADDR_ZPG        6
//#define  ADDR_ABSX       7
//#define  ADDR_ABSY       8
//#define  ADDR_ZPGX       9
//#define  ADDR_ZPGY       10
//#define  ADDR_REL        11
//#define  ADDR_INDX       12
//#define  ADDR_ABSIINDX   13
//#define  ADDR_INDY       14
//#define  ADDR_IZPG       15
//#define  ADDR_IABS       16
//#define  ADDR_ABSL       17

INLINE static uint32_t am_absl( void )
{
    ADDR_MODE( ADDR_ABS );
    INC_PC( 2 );
    INC_CYCLE( 2 );
    return (0);
}

// Absolute Indexed X - a,X
INLINE static uint32_t am_absx( void )
{
    ADDR_MODE( ADDR_ABSX );
    INC_PC( 2 );
    INC_CYCLE( 2 );
    return (0);
}

// Absolute Indexed Y - a,Y
INLINE static uint32_t am_absy( void )
{
    ADDR_MODE( ADDR_ABSY );
    INC_PC( 2 );
    INC_CYCLE( 2 );
    return (0);
}

// Absolute Indirect - (a)
INLINE static uint32_t am_absi( void )
{
    ADDR_MODE( ADDR_IABS );
    INC_PC( 2 );
    INC_CYCLE( 4 );
    return (0);
}

// Absolute Indexed Indirect - (a,X)
INLINE static uint32_t am_abxi( void )
{
    ADDR_MODE( ADDR_INDX );
    INC_PC( 2 );
    INC_CYCLE( 4 );
    return (0);
}

// Absolute Long - >a
INLINE static uint32_t am_alng( void )
{
    ADDR_MODE( ADDR_ABSL );
    INC_PC( 3 );
    INC_CYCLE( 3 );
    return (0);
}

// Absolute Long Indexed - >a,X
INLINE static uint32_t am_alnx( void )
{
    ADDR_MODE( ADDR_IABSLX );
    INC_PC( 3 );
    INC_CYCLE( 3 );
    return (0);
}

// Absolute Indirect Long - [a]
INLINE static uint32_t am_abil( void )
{
    ADDR_MODE( ADDR_IABSL );
    INC_PC( 2 );
    INC_CYCLE( 5 );
    return (0);
}

// Direct Page - d
INLINE static uint32_t am_dpag( void )
{
    ADDR_MODE( ADDR_ZPG );
    INC_PC( 1 );
    INC_CYCLE( 1 );
    return (0);
}

// Direct Page Indexed X - d,X
INLINE static uint32_t am_dpgx( void )
{
    ADDR_MODE( ADDR_ZPGX );
    INC_PC( 1 );
    INC_CYCLE( 1 );
    return (0);
}

// Direct Page Indexed Y - d,Y
INLINE static uint32_t am_dpgy( void )
{
    ADDR_MODE( ADDR_ZPGY );
    INC_PC( 1 );
    INC_CYCLE( 1 );
    return (0);
}

// Direct Page Indirect - (d)
INLINE static uint32_t am_dpgi( void )
{
    ADDR_MODE( ADDR_IZPG );
    INC_PC( 1 );
    INC_CYCLE( 3 );
    return (0);
}

// Direct Page Indexed Indirect - (d,x)
INLINE static uint32_t am_dpix( void )
{
    ADDR_MODE( ADDR_INDX );
    INC_PC( 1 );
    INC_CYCLE( 3 );
    return (0);
}

// Direct Page Indirect Indexed - (d),Y
INLINE static uint32_t am_dpiy( void )
{
    ADDR_MODE( ADDR_INDY );
    INC_PC( 1 );
    INC_CYCLE( 3 );
    return (0);
}

// Direct Page Indirect Long - [d]
INLINE static uint32_t am_dpil( void )
{
    ADDR_MODE( ADDR_IZPGL );
    INC_PC( 1 );
    INC_CYCLE( 4 );
    return (0);
}

// Direct Page Indirect Long Indexed - [d],Y
INLINE static uint32_t am_dily( void )
{
    ADDR_MODE( ADDR_IZPGLY );
    INC_PC( 1 );
    INC_CYCLE( 4 );
    return (0);
}

// Implied/Stack
INLINE static uint32_t am_impl( void )
{
    ADDR_MODE( ADDR_IMPLIED );
    return (0);
}

// Accumulator
INLINE static uint32_t am_acc( void )
{
    ADDR_MODE( ADDR_IMPLIED );
    return (0);
}

// Immediate uint8_t
INLINE static uint32_t am_immb( void )
{
    ADDR_MODE( ADDR_IMM );
    INC_PC( 1 );
    INC_CYCLE( 0 );
    return (0);
}

// Immediate uint16_t
INLINE static uint32_t am_immw( void )
{
    ADDR_MODE( ADDR_IMMW );
    INC_PC( 2 );
    INC_CYCLE( 1 );
    return (0);
}

// Immediate based on size of A/M
INLINE static uint32_t am_immm( void )
{
    ADDR_MODE( ( op_size == 1 ) ? ADDR_IMM : ADDR_IMMW );
    INC_PC( op_size );
    INC_CYCLE( size - 1 );
    return (0);
}

// Immediate based on size of X/Y
INLINE static uint32_t am_immx( void )
{
    ADDR_MODE( ( op_size == 1 ) ? ADDR_IMM : ADDR_IMMW );
    INC_PC( op_size );
    INC_CYCLE( size - 1 );
    return (0);
}

// Long Relative - d
INLINE static uint32_t am_lrel( void )
{
    ADDR_MODE( ADDR_LREL );
    INC_PC( 2 );
    INC_CYCLE( 2 );
    return (0);
}

// Relative - d
INLINE static uint32_t am_rela( void )
{
    ADDR_MODE( ADDR_REL );
    INC_PC( 1 );
    INC_CYCLE( 1 );
    return (0);
}

// Stack Relative - d,S
INLINE static uint32_t am_srel( void )
{
    ADDR_MODE( ADDR_SREL );
    INC_PC( 1 );
    INC_CYCLE( 1 );
    return (0);
}

// Stack Relative Indirect Indexed Y - (d,S),Y
INLINE static uint32_t am_sriy( void )
{
    ADDR_MODE( ADDR_ABSX );
    INC_PC( 1 );
    INC_CYCLE( 3 );

    return (0);
}

#endif /* instr_defs_am_816_h */
