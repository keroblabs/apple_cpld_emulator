#ifndef rtl_cpu_65816_h_
#define rtl_cpu_65816_h_

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////////

#include "types.h"
#include "board.h"
#include "rtl_cpu_defs_816.h"
#include "timer.h"

//////////////////////////////////////////////////////////////////////////////////////

#define CPU_VECTOR_RESET        0xFFFC

//////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
    Instr_State_Idle,
    Instr_State_Fetch,
    Instr_State_Addressing,
    Instr_State_BusRead,
    Instr_State_BusWrite,
    Instr_State_Execute,
    Instr_State_Internal,
    Instr_State_Done,
} bus_mode_t;

typedef enum
{
    Exec_State_Fetch            = 0,
    Exec_State_Fetch_1          = 1,
    Exec_State_Fetch_2          = 2,
    Exec_State_Fetch_3          = 3,
    Exec_State_Fetch_4          = 4,
    Exec_State_Fetch_5          = 5,
    Exec_State_Fetch_6          = 6,
    Exec_State_Fetch_Last       = 19,

    Exec_State_Addressing       = 20,
    Exec_State_Addressing_1     = 21,
    Exec_State_Addressing_2     = 22,
    Exec_State_Addressing_3     = 23,
    Exec_State_Addressing_4     = 24,
    Exec_State_Addressing_5     = 25,
    Exec_State_Addressing_6     = 26,
    Exec_State_Addressing_7     = 27,
    Exec_State_Addressing_8     = 28,
    Exec_State_Addressing_9     = 29,
    Exec_State_Addressing_Last  = 39,

    Exec_State_Execute          = 40,
    Exec_State_Execute_1        = 41,
    Exec_State_Execute_2        = 42,
    Exec_State_Execute_3        = 43,
    Exec_State_Execute_4        = 44,
    Exec_State_Execute_5        = 45,
    Exec_State_Execute_6        = 46,
    Exec_State_Execute_7        = 47,
    Exec_State_Execute_8        = 48,
    Exec_State_Execute_9        = 49,
    Exec_State_Execute_10       = 50,
    Exec_State_Execute_11       = 51,
    Exec_State_Execute_Last     = 59,

    Exec_State_Reset            = 60,
    Exec_State_Reset_1          = 61,
    Exec_State_Reset_2          = 62,
    Exec_State_Reset_3          = 63,
    Exec_State_Reset_4          = 64,
    Exec_State_Reset_5          = 65,
    Exec_State_Reset_Last       = 79,

} exec_state_t;

typedef struct instr_trace_t_
{
    uint8_t  instr_bytes;
    uint32_t  instr_addr;
    bus_mode_t instr_mode;

} instr_trace_t;

typedef struct rtl_cpu_state_t_
{
    regs_t regs;
    
    exec_state_t    exec_state;   // 0 to n
    uint8_t         exec_opcode;
    bool_t          is_wait;
    bool_t          bus_busy;
    bool_t          is_implicity;
    bool_t          disasm_enabled;
    
    bus_mode_t instr_state;  // idle, fetch, bus_read, bus_write
    uint32_t addr_instr;  // result value for the addressing mode
    uint32_t instr_mask;  // how to execute the instruction

    uint32_t addr_req;
    uint32_t addr_source;
    uint32_t addr_dest;
    uint32_t addr_temp;
    uint8_t  instr_index;
    instr_trace_t  instr_trace[10];

    uint32_t data_req;
    uint32_t data_xfer;
    uint32_t data_temp;
    uint8_t  xfer_size;
    uint64_t cpu_cycles;
    
    timer_data_t cpu_timer;
    uint64_t start_halt_us;
    uint32_t cpu_active_time;
    uint32_t cpu_cycle_ms;
    uint32_t cpu_cycle_counter;
    uint32_t cpu_max_cycles;
    uint8_t cpu_halt;
    bool_t cpu_real_time;
    uint32_t cpu_clock_freq;
    uint32_t benchmark_cycles;

} rtl_access_t;

//////////////////////////////////////////////////////////////////////////////////////

void rtl_init_65816( rtl_access_t * state );
void rtl_cpu_execute( rtl_access_t * state );
void rtl_cpu_complete( rtl_access_t * state );

uint32_t rtl_cpu_get_benchmark_cycles( rtl_access_t * state );
uint32_t rtl_cpu_get_active_time( rtl_access_t * state );

//////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
