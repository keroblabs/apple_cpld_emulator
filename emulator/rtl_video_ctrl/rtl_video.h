#ifndef _RTL_VIDEO_H__
#define _RTL_VIDEO_H__

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////////

//#include "datatypes.h"
#include "board.h"

//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////

#if LCD_WIDTH == 1280
#define LCD_LEFT_MARGIN       0
#define LCD_TOP_MARGIN        70
#elif LCD_WIDTH == 800
#define LCD_LEFT_MARGIN       80
#define LCD_TOP_MARGIN        30
#elif LCD_WIDTH == 480
#define LCD_LEFT_MARGIN        0
#define LCD_TOP_MARGIN        20
#else
#define LCD_LEFT_MARGIN        0
#define LCD_TOP_MARGIN        0
#endif

//////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
    Video_State_Idle,
    Video_State_BusRead,
    Video_State_BusWrite,
    Video_State_Done,
} video_bus_t;

typedef enum
{
    VideoMode_Margin,
    VideoMode_Text,
    VideoMode_Lores,
    VideoMode_Hires,
    VideoMode_Dhires,
    VideoMode_Super_320,
    VideoMode_Super_640,
} video_mode_t;

typedef struct rtl_video_t_
{
    video_bus_t bus_mode;
    uint32_t addr_req;
    uint32_t data_xfer;
    uint8_t video_state;
    bool_t bus_busy;
    bool_t back_trace;
    
    uint8_t video_char_1;
    uint8_t video_char_2;
    uint32_t temp_pixel;
    
    uint16_t char_addr;
    uint16_t addr_posx;
    uint16_t addr_posy;
    uint16_t vid_posy;
    uint16_t pixel_cntx;

    uint32_t scan_addr;
    uint32_t super_addr;
    
    video_mode_t video_mode;
    
    uint8_t sw_dhires;
    uint8_t sw_80col;

    uint16_t vid_buffer[ SCREEN_WIDTH ];
    
} rtl_video_t;

//////////////////////////////////////////////////////////////////////////////////////

void video_rtl_init( rtl_video_t * state, bool_t zoom );
void video_rtl_dma_pixels( rtl_video_t * state );
void video_rtl_dma_complete( rtl_video_t * state );

uint32_t video_rtl_benchmark_read( void );

bool_t video_rtl_is_zoom( void );
void video_rtl_set_zoom( bool_t state );

vbl_hdl_t video_hook_vbl( vbl_hdl_t new_hook );

bool_t video_get_vlb( void );

//////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif // _RTL_VIDEO_H__
