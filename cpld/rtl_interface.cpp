#include <iostream>

#include <cstdio>
#include <cmath>

#include "rtl_internal.h"

#include "verilated.h"
//#include "verilated_vcd_c.h"
#include "rtl_interface.h"

////////////////////////////////////////////////////////////////////////////////////

// model declarations
#ifdef SPLIT_CPLD
rtl_model_MMU_t             *p_model_MMU;
rtl_model_IOU_t             *p_model_IOU;
#else
rtl_model_t                 *p_model;
#endif

////////////////////////////////////////////////////////////////////////////////////

static const char * test_argv[] =
{
    "/Users/marciocherobim/apple_soc",
    NULL
};

////////////////////////////////////////////////////////////////////////////////////

double sc_time_stamp()
{
    // this is needed by verilator threads
    return 0;
}

void rtl_interface_init( void )
{
    std::ios::sync_with_stdio(true);

    // Init verilator
    Verilated::commandArgs(1, test_argv);

    // create instance of models
    #ifdef SPLIT_CPLD
    p_model_MMU        = new rtl_model_MMU_t;
    p_model_IOU        = new rtl_model_IOU_t;
    #else
    p_model            = new rtl_model_t;
    #endif
}
