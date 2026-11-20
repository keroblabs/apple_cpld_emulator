#include "timer.h"

#include "rtl_cpu_65816.h"
#include "rtl_cpu_defs_816.h"
#include "disasm_816.h"
#include "system_timer.h"
#include "t_threads.h"
//#include "memory_ctrl.h"
#include <unistd.h>

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_CPU_CYCLE_PERIOD        100
#define CPU_REAL_CLOCK_1M               1000000
#define CPU_REAL_CLOCK_2M8              2550000
#define CPU_CYCLE_MS                    10

//////////////////////////////////////////////////////////////////////////////////////

#include "rtl_instr_defs_816.h"

#include "rtl_instr_defs_am_816.h"
#include "rtl_instr_defs_op_816.h"

#include "rtl_instr_code_am_816.h"
#include "rtl_instr_code_op_816.h"

//////////////////////////////////////////////////////////////////////////////////////

static uint32_t debug_line_cnt = 0;

//////////////////////////////////////////////////////////////////////////////////////

#if 0
static void rtl_cpu_get_regs( rtl_access_t * state, debug_regs_t * p_regs )
{
#ifdef direct_page
#undef direct_page
#endif
#ifdef flag_e
#undef flag_e
#endif
#ifdef flag_x
#undef flag_x
#endif
#ifdef flag_m
#undef flag_m
#endif

    p_regs->acu   = state->regs.a.w;
    p_regs->reg_x = state->regs.x.w;
    p_regs->reg_y = state->regs.y.w;
    p_regs->pc    = state->regs.pc;
    p_regs->sp    = state->regs.sp.w;
    
    p_regs->data_page   = state->regs.dbr;
    p_regs->code_page   = state->regs.pbr;
    p_regs->direct_page = state->regs.dp.w;
    
    p_regs->flag_c = ( state->regs.p.f_c ) ? TRUE : FALSE;
    p_regs->flag_n = ( state->regs.p.f_n ) ? TRUE : FALSE;
    p_regs->flag_v = ( state->regs.p.f_v ) ? TRUE : FALSE;
    p_regs->flag_z = ( state->regs.p.f_z ) ? TRUE : FALSE;
    
    p_regs->flag_e = ( state->regs.e     ) ? TRUE : FALSE;
    p_regs->flag_m = ( state->regs.p.f_m ) ? TRUE : FALSE;
    p_regs->flag_x = ( state->regs.p.f_x ) ? TRUE : FALSE;
}
#endif

void rtl_init_65816( rtl_access_t * state )
{
    memset( state, 0x00, sizeof( rtl_access_t ) );
    state->exec_state = Exec_State_Reset;
    
    state->regs.total_cycles   = 0;
    state->regs.cycles         = 0;
    state->regs.stopped        = FALSE;
    state->regs.irq            = IRQ_STATE_NONE;

    state->disasm_enabled      = FALSE;
    //state->disasm_enabled      = TRUE;
    
    state->cpu_halt = CPU_HALT_NONE;
    state->cpu_clock_freq = CPU_REAL_CLOCK_1M;
    state->cpu_real_time = TRUE;
    state->cpu_cycle_ms   = CPU_CYCLE_MS;
    state->benchmark_cycles = 0;

    state->cpu_max_cycles = ( ( state->cpu_clock_freq + 1000 ) / ( 900 / state->cpu_cycle_ms ) );
}

uint32_t rtl_cpu_get_benchmark_cycles( rtl_access_t * state )
{
    uint32_t ret = state->benchmark_cycles;
    state->benchmark_cycles = 0;
    return ret;
}

uint32_t rtl_cpu_get_active_time( rtl_access_t * state )
{
    return state->cpu_active_time;
}

static void rtl_cpu_execute_internal( rtl_access_t * state )
{
    state->instr_state  = Instr_State_Idle;
        
    if( ( state->exec_state >= Exec_State_Reset ) && ( state->exec_state < Exec_State_Reset_Last ) )
    {
        instr_reset( state );
        return;
    }
    
    if( state->is_wait == TRUE )
    {
        state->instr_state = Instr_State_Internal;
    }
    else if( ( state->exec_state >= Exec_State_Fetch ) && ( state->exec_state < Exec_State_Fetch_Last ) )
    {
        if( state->bus_busy )
        {
            if( state->cpu_real_time && ( state->exec_state == Exec_State_Fetch ) && ( state->cpu_cycle_counter >= state->cpu_max_cycles ) )
            {
                if( timer_Check( &state->cpu_timer ) == TRUE )
                {
                    timer_Start( &state->cpu_timer, state->cpu_cycle_ms );
                    state->cpu_halt = CPU_HALT_NONE;

                    state->cpu_cycle_counter = 0;
                    state->start_halt_us     = system_timer_get_us();
                }
                else if( state->cpu_halt == CPU_HALT_NONE ) // && ( turbo_mode == FALSE ) )
                {
                    state->cpu_active_time = ( uint32_t )( system_timer_get_us() - state->start_halt_us );
                    state->cpu_halt = CPU_HALT_SYNC;
                    tthread_sleep_usecs(50);
                }
            }
        }

        if( state->cpu_halt == CPU_HALT_NONE )
        {
            if( state->bus_busy )
            {
                if( state->regs.pc == 0xc300 )
                {
                    //state->disasm_enabled = TRUE;
                }

#ifdef ENABLE_BOOT_TRACE
                if( state->regs.pc == 0x617D )
                {
                    //state->disasm_enabled = TRUE;
                }

                //if( state->regs.pc == 0x6A20 )
                //{
                //    state->disasm_enabled = TRUE;
                //}
                //if( ( state->regs.pc == 0xC71A ) || ( state->regs.pc == 0xFF0B ) || /* ( state->regs.pc == 0x6137 ) || */ ( state->regs.pc == 0xC391 ) || ( state->regs.pc == 0x2AD6 ) )
                //{
                //    state->disasm_enabled = FALSE;
               // }

                //if( ( state->regs.pc == 0xCA07 ) || ( state->regs.pc == 0xFF75 ) || /* ( state->regs.pc == 0x6126 ) || */ ( state->regs.pc == 0xC3AD ) || ( state->regs.pc == 0x2ADE ) )
                //{
                //    state->disasm_enabled = TRUE;
                //}

                #if 0
                if( state->disasm_enabled ) //  && ( state->exec_state == Exec_State_Fetch ) && ( debug_line_cnt >= 0 ) && ( debug_line_cnt < 740000 ) )
                {
                    void rtl_disasm_instruction_816( void * p_regs );

                    debug_regs_t debug_regs;
                    rtl_cpu_get_regs( state, &debug_regs );
                    rtl_disasm_instruction_816( &debug_regs );
                }
                #endif
#endif

                debug_line_cnt++;
            }

            state->regs.cycles = 0;
            instr_fetch( state );
        }
    }
    else
    {
        if( ( state->exec_state >= Exec_State_Addressing ) && ( state->exec_state < Exec_State_Addressing_Last ) )
        {
            RTL_INSTR_AM_DECODE();
            
            // this allow implicity addressings to link directly to the execution pipeline
            if( !state->is_implicity ) return;
        }
        
        if( ( state->exec_state >= Exec_State_Execute ) && ( state->exec_state < Exec_State_Execute_Last ) )
        {
            RTL_INSTR_OP_DECODE();
            
            if( !state->bus_busy && ( state->exec_state == Exec_State_Fetch ) )
            {
                if( state->cpu_real_time )
                {
                    state->cpu_cycle_counter += state->regs.cycles; // for real time control
                }
                
                state->cpu_cycles        += state->regs.cycles; // absolute cpu cycles
                state->benchmark_cycles  += state->regs.cycles; // benchmark cycles

                disasm_instruction_816( state );
                state->instr_index       = 0;
            }
        }
    }
}

void rtl_cpu_execute( rtl_access_t * state )
{
    state->bus_busy = TRUE;
    rtl_cpu_execute_internal( state );
}

void rtl_cpu_complete( rtl_access_t * state )
{
    state->bus_busy = FALSE;
    instruction_trace( state );
    rtl_cpu_execute_internal( state );
}
