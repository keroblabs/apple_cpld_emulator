#ifndef rtl_interface_h
#define rtl_interface_h

// C++-only interface to the combined Verilator MMU/IOU testbench.
#ifdef SPLIT_CPLD
#error "The state-machine RTL adapter currently requires the combined IOU_MMU_tb model."
#endif

#include "types.h"
#include "rtl_model.h"
#include "VIOU_MMU_tb.h"

typedef VIOU_MMU_tb rtl_model_t;
extern rtl_model_t *p_model;

// No dynamic allocation or implicit cast between the two emulated chip states.
typedef struct cpld_model_t_
{
    cpld_model_MMU_t *mmu;
    cpld_model_IOU_t *iou;
    rtl_model_t *p_model;
} cpld_model_t;

void rtl_interface_init( void );

// CPU-side bus request and data pins.
void cpu_bus_request( cpld_model_t *context, uint32_t address, bool_t cpu_rw, uint8_t data_in );
void cpu_bus_process( cpld_model_t *context );
void cpu_bus_capture( cpld_model_t *context );

// PHI_0 timing and memory transaction completion.
void memory_bus_process( cpld_model_t *context );
void memory_bus_reset( cpld_model_t *context );
void memory_bus_request( cpld_model_t *context );
void memory_bus_latch( cpld_model_t *context );
bool_t memory_bus_is_done( cpld_model_t *context );
void memory_bus_ack( cpld_model_t *context );

// Serial controller advances on its own clock edges, independently of PHI_0.
void spi_slave_process( cpld_model_t *context );
void spi_slave_latch( cpld_model_t *context );
void spi_slave_send_data( cpld_model_t *context, uint8_t addr, uint8_t data_in );
bool_t spi_slave_is_done( cpld_model_t *context );
uint16_t spi_slave_get_received_data( cpld_model_t *context );
void spi_slave_reset( cpld_model_t *context );

// The Verilog testbench wires MMU and IOU together internally; this captures
// its external decoded outputs into the software-side CPLD state.
void rtl_interconnect_capture( cpld_model_t *context );

#endif
