#ifndef rtl_cpu_defs_h_
#define rtl_cpu_defs_h_

typedef struct _regs
{
    union FLAGS {
        struct {
            bool_t             f_c : 1;
            bool_t             f_z : 1;
            bool_t             f_i : 1;
            bool_t             f_d : 1;
            bool_t             f_x : 1;
            bool_t             f_m : 1;
            bool_t             f_v : 1;
            bool_t             f_n : 1;
        };
        uint8_t            b;
    }   p;

    bool_t     e;
    bool_t     stopped;

    union REGS {
        uint8_t  b;
        uint16_t w;
    }   a, x, y, sp, dp;

    uint16_t   pc;
    uint8_t    pbr;
    uint8_t    dbr;

    uint8_t    irq;
    uint32_t   cycles;
    uint64_t   total_cycles;

} regs_t;

#define join16(h,l)			(((uint16_t)((h)&0x0FF)<<8)  | ((l)&0x0FF))
#define bank(b)				((uint32_t)((b)&0x0FF)<<16)
#define bank_pbr			bank(state->regs.pbr)
#define bank_dbr			bank(state->regs.dbr)
#define lo(b)				((uint8_t)((b) & 0x0FF))
#define hi(b)				((uint8_t)(((b) >> 8) & 0x0FF))
#define swap(v)				((((uint16_t)(v)) << 8) | (((uint16_t)(v)) >> 8))

#define PC(c)               state->regs.pc += (c)
#define CYCLE(c)            state->regs.cycles += (c)
#define ADDR_MODE(a)

#define setn( flag )            state->regs.p.f_n = ( flag ) ? 1 : 0;
#define setv( flag )            state->regs.p.f_v = ( flag ) ? 1 : 0;
#define setd( flag )            state->regs.p.f_d = ( flag ) ? 1 : 0;
#define seti( flag )            state->regs.p.f_i = ( flag ) ? 1 : 0;
#define setz( flag )            state->regs.p.f_z = ( flag ) ? 1 : 0;
#define setc( flag )            state->regs.p.f_c = ( flag ) ? 1 : 0;

#define setnz_b( value )        { setn( ( value ) & 0x80);     setz( ( value ) == 0); }
#define setnz_w( value )        { setn( ( value ) & 0x80000);  setz( ( value ) == 0); }

#endif
