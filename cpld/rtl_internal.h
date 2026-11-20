#ifndef Header_h
#define Header_h

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

////////////////////////////////////////////////////////////////////////////////////

#define SIGNAL_STATE(s)     ((s) ? '*' : '.')

#endif /* Header_h */
