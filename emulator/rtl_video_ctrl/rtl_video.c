#include "sw_switch.h"
#include "timer.h"
#include "rtl_video.h"
#include "graphics.h"
#include "draw.h"
#include "ram_bus_hw.h"
#include "t_threads.h"

//////////////////////////////////////////////////////////////////////////////////////

#define ENABLE_ZOOM
#define VIDEO_HZ                    35

#if LCD_WIDTH == 1280
#define ENABLE_80COL_VIDEO
#define VIDEO_CENTRED
#define VIDEO_MASK_SCALE            2,2,2,2,2,2,2, 2,2,2,2,2,2,2, 2,2,2,2,2,2,2, 2,2,2,2,2,2,2,

#elif LCD_WIDTH == 1024
#define ENABLE_80COL_VIDEO
#define VIDEO_CENTRED
#define VIDEO_MASK_SCALE            2,2,2,2,1,2,2, 2,2,1,2,2,2,2, 1,2,2,2,2,1,2, 2,2,1,2,2,1,2,

#elif LCD_WIDTH == 800
#define ENABLE_80COL_VIDEO
#define VIDEO_CENTRED
//#define VIDEO_MASK_SCALE            2,1,1,2,1,1,2, 1,1,2,1,1,2,1, 2,1,1,2,1,1,2, 1,1,2,1,1,2,1,
#define VIDEO_MASK_SCALE            1,1,1,1,1,1,1, 1,1,1,1,1,1,1, 1,1,1,1,1,1,1, 1,1,1,1,1,1,1,

#elif LCD_WIDTH == 640
#define ENABLE_80COL_VIDEO
#define VIDEO_CENTRED
#define VIDEO_MASK_SCALE            1,1,1,1,1,1,1, 1,1,1,1,1,1,1, 1,1,1,1,1,1,1, 1,1,1,1,1,1,1,

#elif LCD_WIDTH == 480
#define ENABLE_80COL_VIDEO
#define VIDEO_CENTRED
#define VIDEO_MASK_SCALE            1,1,1,1,0,1,1, 1,1,0,1,1,1,1, 0,1,1,1,1,0,1, 1,1,1,0,1,1,1,

#elif LCD_WIDTH == 320
#define DONT_ENABLE_80COL_VIDEO
#define VIDEO_MASK_SCALE            1,1,1,1,1,1,1, 1,1,1,1,1,1,1, 1,1,1,1,1,1,1, 1,1,1,1,1,1,1,

#else
#error "Unsupported LCD"
#endif

#define VIDEO_RATE_MAX_ITEM             100

#if LCD_HEIGHT == 800
#define VIDEO_ZOOM_HEIGHT               2
#define VIDEO_ZOOM_INCREMENT            1
#elif LCD_HEIGHT == 600
#define VIDEO_ZOOM_HEIGHT               2
#define VIDEO_ZOOM_INCREMENT            1
#elif LCD_HEIGHT == 720
#define VIDEO_ZOOM_HEIGHT               3
#define VIDEO_ZOOM_INCREMENT            0
#elif LCD_HEIGHT == 480
#define VIDEO_ZOOM_HEIGHT               1
#define VIDEO_ZOOM_INCREMENT            1
#elif    (LCD_HEIGHT == 240)
      || (LCD_HEIGHT == 200)
      || (LCD_HEIGHT == 272)
#define VIDEO_ZOOM_HEIGHT               1
#define VIDEO_ZOOM_INCREMENT            0
#else
#error "Unsupported LCD"
#endif

#define _USE_BITMAP_DISPLAY

#define BLACK_COLOUR_DEFAULT                APP_RGB(  32,  32,  32 )
#define WHITE_COLOUR_DEFAULT                APP_RGB( 255, 255, 255 )

//////////////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_VIDEO

#ifdef VIDEO_MASK_SCALE
static const uint8_t zoom_mask_index[28] = { VIDEO_MASK_SCALE };
#endif

static const uint32_t base_address[] =   { 0x000,  0x080,  0x100,  0x180,  0x200,  0x280,  0x300,  0x380 };
static const uint32_t offset_address[] = { 0x000,  0x028,  0x050 };
static const uint32_t offset_hires[] =   { 0x0000, 0x0400, 0x0800, 0x0C00, 0x1000, 0x1400, 0x1800, 0x1C00 };

static const uint8_t expand_byte[] =     { 0x00,   0x03,   0x0C,   0x0F,   0x30,   0x33,   0x3C,   0x3F,
                                           0xC0,   0xC3,   0xCC,   0xCF,   0xF0,   0xF3,   0xFC,   0xFF };

/*
static const uint16_t super_table_low[8]     = { 0x0000, 0x00A0, 0x0140, 0x01E0, 0x0280, 0x0320, 0x03C0, 0x0460 };
static const uint16_t super_table_offset[16] = { 0x0000, 0x0500, 0x0A00, 0x0F00, 0x1400, 0x1900, 0x1E00, 0x2300,
                                                 0x2800, 0x2D00, 0x3200, 0x3700, 0x3C00, 0x4100, 0x4600, 0x4B00 };

static const uint8_t palete_640_lut[] = { 0x8, 0xC, 0x0, 0x4 };
*/

static const uint16_t pixel_mono[2] =
{
    BLACK_COLOUR_DEFAULT,
    WHITE_COLOUR_DEFAULT
};

static const uint16_t pixel_colour[4] =
{
    APP_RGB( 255,   0, 255 ),  // pink
    APP_RGB(   0, 255,   0 ),  // green
    APP_RGB(  64, 128, 255 ),  // blue
    APP_RGB( 255, 128,  64 ),  // orange
};

static const uint16_t pixel_lores[16] =
{
    BLACK_COLOUR_DEFAULT,
    APP_RGB( 128,  25,  96 ),
    APP_RGB(  65,  45, 225 ),
    APP_RGB( 200,  60, 255 ), // pink

    APP_RGB(   0, 128,  16 ),
    APP_RGB( 128, 128, 128 ),
    APP_RGB(  36, 128, 255 ), // blue
    APP_RGB( 160, 160, 255 ),

    APP_RGB(  80,  80,   0 ),
    APP_RGB( 240,  96,   0 ), // orange
    APP_RGB( 190, 190, 190 ),
    APP_RGB( 255, 128, 255 ),

    APP_RGB(   0, 200,   0 ), // green
    APP_RGB( 200, 200,   0 ),
    APP_RGB(  80, 255, 128 ),
    WHITE_COLOUR_DEFAULT
};

//////////////////////////////////////////////////////////////////////////////////////

#define RAM_SW_SWITCH           ram_bus_read_sw_switch

extern const unsigned char char_map[];

//////////////////////////////////////////////////////////////////////////////////////

static timer_data_t     scan_ms;
static vbl_hdl_t        p_vbl;

static uint32_t frame_counter;

static bool_t video_zoom;
static bool_t video_zoom_req;
static bool_t end_v_scan;

static uint32_t video_frame_ms;
static uint32_t update_count;

static void * rtl_video_mutex;

//static uint8_t  super_line_ctrl;
//static uint16_t  super_palette[16];

//////////////////////////////////////////////////////////////////////////////////////

static bool_t is_video_backtrace( rtl_video_t * state )
{
    if( state->bus_busy == TRUE )
    {
        state->back_trace = (    ( state->pixel_cntx < SCREEN_APPLE_MARGIN )
                              || ( state->pixel_cntx >= ( SCREEN_WIDTH - SCREEN_APPLE_MARGIN ) ) );
    }
    
    return state->back_trace;
}

static void video_dma_backtrace( rtl_video_t * state )
{
    uint8_t  bit_cnt;

    if( state->bus_busy == TRUE )
    {
        for( bit_cnt = 0; bit_cnt < 8; bit_cnt++ )
        {
            state->vid_buffer[ state->pixel_cntx++ ] = 0;
        }
    }
    else
    {
        state->bus_mode = Video_State_Done;
    }
}

static void video_dma_text_pixels( rtl_video_t * state )
{
    uint32_t addr = state->scan_addr + state->addr_posx;
    uint16_t posy = state->addr_posy - SCREEN_APPLE_TOP_MARGIN;
    uint8_t  bit_cnt;

    if( state->bus_busy == TRUE )    
    {
        state->addr_req = addr;
        state->bus_mode = Video_State_BusRead;
    }
    else
    {
        if( state->sw_80col )
        {
            state->video_char_1 = ( state->data_xfer >> 8 ) & 0x0FF;
            state->video_char_2 = state->data_xfer & 0x0FF;

            uint32_t char_map_addr;

            // 80 col - decode main char map first
            char_map_addr = ( state->char_addr + ( state->video_char_2 << 3 ) + ( posy & 0x07 ) );
            state->temp_pixel = char_map[ char_map_addr ];

            // get next pixel map
            char_map_addr = ( state->char_addr + ( state->video_char_1 << 3 ) + ( posy & 0x07 ) );
            state->temp_pixel = ( state->temp_pixel << 7 ) | char_map[ char_map_addr ];
        }
        else
        {
            // 40 col, store main char 1
            uint32_t char_map_addr = ( state->char_addr + ( ( state->data_xfer & 0x0FF ) << 3 ) + ( posy & 0x07 ) );
            uint8_t char_xfer = char_map[ char_map_addr ];
            state->temp_pixel = expand_byte[ char_xfer & 0x0F ] | ( expand_byte[ char_xfer >> 4 ] << 8 );
        }

        for( bit_cnt = 0; bit_cnt < 14; bit_cnt++ )
        {
            uint8_t text_index =   ( state->temp_pixel & 0x01 )
                                 ? ( RAM_SW_SWITCH( SW_REG_SCREEN_COLOUR ) >> 4 )
                                 : ( RAM_SW_SWITCH( SW_REG_SCREEN_COLOUR ) & 0x0F );
            
            state->vid_buffer[ state->pixel_cntx ] = pixel_lores[ text_index ];  // pixel_mono[ local_pixel & 0x01 ];
            state->temp_pixel >>= 1;
            state->pixel_cntx++;
        }
        
        state->addr_posx++;
        state->bus_mode = Video_State_Done;
    }
}

static void video_dma_lores_pixels( rtl_video_t * state )
{
    uint8_t bit_cnt;
    uint32_t addr = state->scan_addr + state->addr_posx;
    uint16_t posy = state->addr_posy - SCREEN_APPLE_TOP_MARGIN;

    if( state->bus_busy == TRUE )
    {
        state->addr_req = addr;
        state->bus_mode = Video_State_BusRead;
    }
    else
    {
        if( state->sw_dhires )
        {
            // 80 col, split 16 bits
            state->video_char_1 = state->data_xfer >> 8;
            state->video_char_2 = state->data_xfer & 0x0FF;
        }
        else
        {
            // 40 col, store in both slots
            state->video_char_1 = state->data_xfer;
            state->video_char_2 = state->data_xfer;
        }

        if( posy & 0x04 )
        {
            state->video_char_1 >>= 4;
            state->video_char_2 >>= 4;
        }
        else
        {
            state->video_char_1 &= 0x0F;
            state->video_char_2 &= 0x0F;
        }
        
        for( bit_cnt = 0; bit_cnt < 7; bit_cnt++ )
        {
            state->vid_buffer[ state->pixel_cntx     ] = pixel_lores[ state->video_char_1 ];
            state->vid_buffer[ state->pixel_cntx + 7 ] = pixel_lores[ state->video_char_2 ];
            state->pixel_cntx++;
        }
        
        state->pixel_cntx += 7;
        state->addr_posx++;
        state->bus_mode = Video_State_Done;
    }
}

static void video_dma_hires_pixels( rtl_video_t * state )
{
    uint32_t addr = state->scan_addr + state->addr_posx;
    uint8_t bit_cnt;

    if( state->bus_busy == TRUE )
    {
        state->addr_req = addr;
        state->bus_mode = Video_State_BusRead;
    }
    else
    {
        for( bit_cnt = 0; bit_cnt < 7; bit_cnt++ )
        {
            if( ( state->data_xfer & ( 1 << bit_cnt ) ) )
            {
                if(    ( state->pixel_cntx >= ( SCREEN_APPLE_MARGIN + 2 ) )
                    && ( state->vid_buffer[ state->pixel_cntx - 2 ] != pixel_mono[0] ) )
                {
                    state->vid_buffer[ state->pixel_cntx - 2 ] = pixel_mono[1];
                    state->vid_buffer[ state->pixel_cntx - 1 ] = pixel_mono[1];
                    state->vid_buffer[ state->pixel_cntx     ] = pixel_mono[1];
                    state->vid_buffer[ state->pixel_cntx + 1 ] = pixel_mono[1];
                }
                else
                {
                    state->vid_buffer[ state->pixel_cntx     ] = pixel_colour[ ( ( state->data_xfer >> 6 ) & 0x02 ) | ( ( state->pixel_cntx >> 1 ) & 0x01 ) ];
                    state->vid_buffer[ state->pixel_cntx + 1 ] = pixel_colour[ ( ( state->data_xfer >> 6 ) & 0x02 ) | ( ( state->pixel_cntx >> 1 ) & 0x01 ) ];
                }

                if(      ( state->pixel_cntx >= ( SCREEN_APPLE_MARGIN + 4 ) )
                      && ( state->vid_buffer[ state->pixel_cntx - 2 ] == pixel_mono[0] )
                      && ( state->vid_buffer[ state->pixel_cntx - 4 ] != pixel_mono[0] ) )
                {
                    state->vid_buffer[ state->pixel_cntx - 2 ] = state->vid_buffer[ state->pixel_cntx ];
                    state->vid_buffer[ state->pixel_cntx - 1 ] = state->vid_buffer[ state->pixel_cntx ];
                }
            }
            else
            {
                state->vid_buffer[ state->pixel_cntx     ] = pixel_mono[0];
                state->vid_buffer[ state->pixel_cntx + 1 ] = pixel_mono[0];
            }
            
            state->pixel_cntx += 2;
        }
        
        state->addr_posx++;
        state->bus_mode = Video_State_Done;
    }
}

static void video_dma_dhires_pixel( rtl_video_t * state )
{
    uint32_t addr = state->scan_addr + state->addr_posx;
    uint8_t bit_cnt;
    
    if( state->bus_busy == TRUE )
    {
        state->addr_req = addr;
        state->bus_mode = Video_State_BusRead;
    }
    else
    {
        state->video_char_1 = state->data_xfer >> 8;
        state->video_char_2 = state->data_xfer & 0x0FF;
                
        for( bit_cnt = 0; bit_cnt < 7; bit_cnt++ )
        {
            state->vid_buffer[ state->pixel_cntx     ] = pixel_mono[ ( state->video_char_1 >> bit_cnt ) & 0x01 ];
            state->vid_buffer[ state->pixel_cntx + 7 ] = pixel_mono[ ( state->video_char_2 >> bit_cnt ) & 0x01 ];
            state->pixel_cntx++;
        }
        
        state->pixel_cntx += 7;
        state->addr_posx++;
        state->bus_mode = Video_State_Done;
    }
}

#if 0
static void video_dma_super_320_pixels( uint32_t scan_addr, pixel_t * vid_buffer )
{
    uint8_t  bit_cnt;
    uint8_t  byte_cnt;
    uint8_t  video_data;
    uint32_t addr = scan_addr + addr_posx;

    for( byte_cnt = 0; byte_cnt < 4; byte_cnt++ )
    {
        video_data = ram_bus_hw_read_bank_E0( addr + byte_cnt );
        addr_posx++;
        
        for( bit_cnt = 0; bit_cnt < 2; bit_cnt++ )
        {
            if( ( pixel_cntx > 1 ) && ( super_line_ctrl & 0x20 ) && ( ( video_data & 0xF0 ) == 0x00 ) )
            {
                vid_buffer[ pixel_cntx     ] = vid_buffer[ pixel_cntx - 1 ];
                vid_buffer[ pixel_cntx + 1 ] = vid_buffer[ pixel_cntx - 1 ];
            }
            else
            {
                vid_buffer[ pixel_cntx     ] = super_palette[ video_data >> 4 ] ;
                vid_buffer[ pixel_cntx + 1 ] = vid_buffer[ pixel_cntx ];
            }
                
            pixel_cntx += 2;
            video_data <<= 4;
        }
    }
}

static void video_dma_super_640_pixels( uint32_t scan_addr, pixel_t * vid_buffer )
{
    uint8_t bit_cnt;
    uint8_t byte_cnt;
    uint8_t video_data;
    uint32_t addr = scan_addr + addr_posx;

    for( byte_cnt = 0; byte_cnt < 4; byte_cnt++ )
    {
        video_data = ram_bus_hw_read_bank_E0( addr + byte_cnt );
        addr_posx++;

        for( bit_cnt = 0; bit_cnt < 4; bit_cnt++ )
        {
            vid_buffer[ pixel_cntx++ ] = super_palette[ palete_640_lut[ bit_cnt ] + ( video_data >> 6 ) ];
            video_data <<= 2;
        }
    }
}
#endif

static uint32_t super_video_get_address( rtl_video_t * state )
{
    uint32_t addr = NEG_U32;
#if 0
    uint32_t line_ctrl_addr;
    uint32_t palette_addr;
    uint8_t  loop;
    uint8_t  gs_palette_gb;
    uint8_t  gs_palette_r;

    addr = ( input_posy & 0x80 ) ? 0x17000 : 0x12000;
    addr += super_table_offset[ ( input_posy >> 3 ) & 0x0F ];
    addr += super_table_low[ input_posy & 0x07 ];
    
    line_ctrl_addr       = 0x19D00 + input_posy;
    super_line_ctrl      = ram_bus_hw_read_bank_E0( line_ctrl_addr );
    video_mode           = ( super_line_ctrl & 0x80 ) ? VideoMode_Super_640 : VideoMode_Super_320;
    
    palette_addr = 0x19E00 + ( input_posy << 5 );
    for( loop = 0; loop < 32; loop += 2 )
    {
        gs_palette_gb = ram_bus_hw_read_bank_E0( palette_addr + loop     );
        gs_palette_r  = ram_bus_hw_read_bank_E0( palette_addr + loop + 1 );
        
        super_palette[ loop >> 1 ] = APP_RGB( gs_palette_r << 4, gs_palette_gb & 0xF0, gs_palette_gb << 4 );
    }
#endif
    return addr;
}

static uint32_t apple_video_get_address( rtl_video_t * state )
{
    uint8_t sw_altchar;
    uint8_t sw_80store;
    uint8_t sw_an3;
    uint8_t sw_text;
    uint8_t sw_page2;
    uint8_t sw_hires;
    uint16_t temp_posy;
    uint16_t posy;
    uint32_t addr = NEG_U32;
    
    if( ( state->addr_posy < SCREEN_APPLE_TOP_MARGIN ) || ( state->addr_posy >= ( SCREEN_HEIGHT - SCREEN_APPLE_TOP_MARGIN ) ) )
    {
        state->video_mode = VideoMode_Margin;
    }
    else
    {
        posy = state->addr_posy - SCREEN_APPLE_TOP_MARGIN;
        
        sw_hires        = RAM_SW_SWITCH( SW_HIRES );
        sw_text         = RAM_SW_SWITCH( SW_TEXT );
        state->sw_80col = RAM_SW_SWITCH( SW_80COL );
        sw_altchar      = RAM_SW_SWITCH( SW_CHARSET );
        sw_page2        = RAM_SW_SWITCH( SW_PAGE2 );
        sw_80store      = RAM_SW_SWITCH( SW_80STORE );
        sw_an3          = RAM_SW_SWITCH( SW_AN3 );

        if( RAM_SW_SWITCH( SW_MIXED ) && ( posy >= SCREEN_SWITCH_Y ) )
        {
            sw_hires = FALSE;
            sw_text  = TRUE;
        }

        if( sw_80store == TRUE ) sw_page2 = FALSE;

        if( sw_hires && state->sw_80col && !sw_an3 ) state->sw_dhires = TRUE;
        else if( !sw_hires && state->sw_80col )      state->sw_dhires = TRUE;
        else                                         state->sw_dhires = FALSE;

        //if( local_dhires == TRUE )                 sw_page2 = FALSE;

        if( sw_text || ( !sw_text && !sw_hires ) )
        {
            temp_posy = posy >> 3;
            addr      =   base_address[ temp_posy & 0x07 ]
                        + offset_address[ ( temp_posy >> 3 ) & 0x03 ]
                        + ( sw_page2 ? 0x0800 : 0x0400 );
            
            //state->video_mode = sw_text ? VideoMode_Text : VideoMode_Lores;
            if( sw_text )
            {
                state->video_mode = VideoMode_Text;
            }
            else
            {
                state->video_mode = VideoMode_Lores;
            }
            
            if( state->video_mode == VideoMode_Text )
            {
                state->char_addr = sw_altchar ? 2048 : 0;
            }
        }
        else
        {
            temp_posy = posy >> 3;
            addr      =   base_address[ temp_posy & 0x07 ]
                        + offset_address[ ( temp_posy >> 3 ) & 0x07 ]
                        + offset_hires[ posy & 0x07 ]
                        + ( sw_page2 ? 0x4000 : 0x2000 );

            state->video_mode = state->sw_dhires ? VideoMode_Dhires : VideoMode_Hires;
        }
    }
    
    return addr;
}

static void video_show_line( rtl_video_t * state )
{
    uint16_t zoom_height = VIDEO_ZOOM_HEIGHT;
    uint16_t left_margin = LCD_LEFT_MARGIN;
    uint16_t top_margin  = LCD_TOP_MARGIN;

    #ifdef VIDEO_MASK_SCALE
    if( video_zoom == TRUE )
    {
        #ifndef VIDEO_CENTRED
        left_margin = 0;
        top_margin  = 0;
        #endif
        zoom_height += VIDEO_ZOOM_INCREMENT;
    }
    #endif

    uint16_t posx = left_margin;
    uint16_t posy = state->vid_posy + top_margin;
    uint16_t pixel_source_x = 0;

    for( pixel_source_x = 0; pixel_source_x < SCREEN_WIDTH; pixel_source_x++ )
    {
        uint16_t pixel_temp = state->vid_buffer[ pixel_source_x ];
        int zoom_w_cnt =
                            #ifdef VIDEO_MASK_SCALE
                            ( video_zoom == TRUE ) ? zoom_mask_index[pixel_source_x % 28] :
                            #endif
                        1;

        while( zoom_w_cnt )
        {
            for( uint16_t zoom_h_cnt = 0; zoom_h_cnt < zoom_height; zoom_h_cnt++ )
            {
                graphics_set_pixel(left_margin + posx, posy + zoom_h_cnt, pixel_temp);
            }
            
            posx++;
            zoom_w_cnt--;
        }
    }
}

static void default_video_vbl( void )
{
}

static void video_v_scan( rtl_video_t * state )
{
    tthread_mutex_lock( rtl_video_mutex );
    frame_counter++;
    tthread_mutex_unlock( rtl_video_mutex );

    state->vid_posy = 0;
    state->addr_posy = 0;
    state->super_addr = 0;

    #ifdef ENABLE_ZOOM
    if( video_zoom != video_zoom_req )
    {
        if( video_zoom == TRUE )
        {
            uint16_t posx, posy;

            // reducing - need to clean screen
            for( posy = 0; posy < LCD_HEIGHT; posy++ )
            {
                for( posx = 0; posx < ( LCD_WIDTH - BUTTON_WIDTH ); posx++ )
                {
                    graphics_set_pixel( posx, posy, APP_RGB( 0, 0, 0) );
                }
            }
        }

        video_zoom = video_zoom_req;
    }
    #endif
}

static uint32_t video_h_scan( rtl_video_t * state )
{
    state->addr_posx = 0;

    state->vid_posy += VIDEO_ZOOM_HEIGHT;

    if( video_zoom == TRUE )
    {
        state->vid_posy += VIDEO_ZOOM_INCREMENT;
    }

    if( ++state->addr_posy >= SCREEN_HEIGHT )
    {
        end_v_scan = TRUE;
        video_v_scan( state );
        p_vbl();
    }
    
    return ( RAM_SW_SWITCH( SW_REG_NEW_SCREEN ) & REG_NEWVIDEO_SUPER )
                ? super_video_get_address( state )
                : apple_video_get_address( state );
}

void static rtl_dma_pixels( rtl_video_t * state )
{
    if( is_video_backtrace( state ) || ( state->video_mode == VideoMode_Margin ) )
    {
        video_dma_backtrace( state );
    }
    else
    {
        switch( state->video_mode )
        {
            case VideoMode_Text:
                video_dma_text_pixels( state );
                break;
                
            case VideoMode_Lores:
                video_dma_lores_pixels( state );
                break;
                
            case VideoMode_Hires:
                video_dma_hires_pixels( state );
                break;
                
            case VideoMode_Dhires:
                video_dma_dhires_pixel( state );
                break;
                
    //        case VideoMode_Super_320:
    //            video_dma_super_320_pixels( state );
    //            break;
    //
    //        case VideoMode_Super_640:
    //            video_dma_super_640_pixels( state );
    //            break;
                
            default:
                break;
        }
    }
}

void video_rtl_dma_pixels( rtl_video_t * state )
{
    state->bus_mode = Video_State_Idle;
    
    if( end_v_scan == TRUE )
    {
        if( timer_Check( &scan_ms ) == TRUE )
        {
            timer_Start( &scan_ms, video_frame_ms );
            end_v_scan = FALSE;
        }
        else
        {
            tthread_sleep_usecs( 10 );
            return;
        }
    }

    state->bus_busy = TRUE;
    rtl_dma_pixels( state );
}

void video_rtl_dma_complete( rtl_video_t * state )
{
    state->bus_busy = FALSE;
    rtl_dma_pixels( state );
    
    if( state->bus_mode == Video_State_Done )
    {
        state->video_state = Video_State_Idle;
        state->video_state = 0;

        if( state->pixel_cntx >= SCREEN_WIDTH )
        {
            video_show_line( state );

            state->pixel_cntx   = 0;
            
            state->scan_addr = video_h_scan( state );
        }
    }
}

uint32_t video_rtl_benchmark_read( void )
{
    tthread_mutex_lock( rtl_video_mutex );
    
    uint32_t ret = frame_counter;
    frame_counter = 0;

    tthread_mutex_unlock( rtl_video_mutex );

    return ret;
}

bool_t video_rtl_is_zoom( void )
{
    return video_zoom;
}

void video_rtl_set_zoom( bool_t state )
{
    video_zoom_req = state;
}

void video_rtl_init( rtl_video_t * state, bool_t zoom )
{
    p_vbl = default_video_vbl;

    rtl_video_mutex = tthread_mutex_init();

    video_frame_ms = 1000 / VIDEO_HZ;
    frame_counter = 0;
    timer_Start( &scan_ms, video_frame_ms );
    end_v_scan = FALSE;

    video_v_scan( state );
    state->scan_addr = video_h_scan( state );
    state->pixel_cntx = 0;
    state->video_state = Video_State_Idle;
    state->bus_busy = TRUE;
    
    video_zoom = video_zoom_req = zoom;

    update_count = 0;
}

#endif
