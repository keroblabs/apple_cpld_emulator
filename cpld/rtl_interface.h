#ifndef rtl_interface_h
#define rtl_interface_h

////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
//extern "C" {
#endif

////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <cstdio>
#include <cmath>

#include "types.h"

// include from models
#ifdef SPLIT_CPLD
#include "VMMU.h"
#include "VIOU.h"
//#include "VP65C816.h"
#else
#include "VIOU_MMU_tb.h"
#endif

#include "types.h"

////////////////////////////////////////////////////////////////////////////////////

// typedef of models
#ifdef SPLIT_CPLD
typedef VMMU            rtl_model_MMU_t;
typedef VIOU            rtl_model_IOU_t;
#else
typedef VIOU_MMU_tb           rtl_model_t;
#endif

// extern instance of models
#ifdef SPLIT_CPLD
extern rtl_model_MMU_t         *p_model_MMU;
extern rtl_model_IOU_t         *p_model_IOU;
#else
extern rtl_model_t               *p_model;
#endif

typedef struct cpld_model_t_
{
    void * p_data;
    rtl_model_t  *p_model;

} cpld_model_t;

////////////////////////////////////////////////////////////////////////////////////

void rtl_interface_init( void );

void spi_slave_process( cpld_model_t *cpld_model );
void spi_slave_send_data( cpld_model_t *cpld_model, uint8_t addr, uint8_t data_in );
bool_t spi_slave_is_done( cpld_model_t *cpld_model );
uint16_t spi_slave_get_received_data( cpld_model_t *cpld_model );
void spi_slave_reset( cpld_model_t *cpld_model );

void memory_bus_process( cpld_model_t *cpld_model );
void memory_bus_reset( cpld_model_t *cpld_model );
void memory_bus_request( cpld_model_t *cpld_model, uint32_t address, bool_t cpu_rw, uint8_t data_in );
bool_t memory_bus_is_done( cpld_model_t *cpld_model );

////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
// }
#endif

////////////////////////////////////////////////////////////////////////////////////

#endif /* rtl_interface_h */
