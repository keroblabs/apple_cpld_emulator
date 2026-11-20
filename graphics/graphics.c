#include "graphics.h"
#include "timer.h"
#include "t_threads.h"
#include "drivers.h"

///////////////////////////////////////////////////////////////

enum
{
	LCD_MODE_DISABLED,
	LCD_MODE_API,
	LCD_MODE_FRAMEBUFFER,
};

enum
{
	LCD_VIEW_LANDSCAPE,
	LCD_VIEW_PORTRAIT,
};

///////////////////////////////////////////////////////////////

#define VSYNC_TIME_MS		50

///////////////////////////////////////////////////////////////

//static bool_t lcd_ready = FALSE;
//static bool_t lcd_init_done = FALSE;
static timer_data_t vsync_timer;
static void * lcd_mutex;

///////////////////////////////////////////////////////////////

//static uint8_t lcd_mode;
//static uint8_t lcd_view;

///////////////////////////////////////////////////////////////

void graphics_init( void )
{
	lcd_driver_init();
	lcd_driver_clear_screen();

	lcd_mutex = tthread_mutex_init();

	timer_Start( &vsync_timer, VSYNC_TIME_MS );
	timer_Stop( &vsync_timer );
}

void graphics_init_auto_vsync( void )
{
	timer_Start( &vsync_timer, VSYNC_TIME_MS );
}

void graphics_process( void )
{
	if( timer_Check( &vsync_timer ) )
	{
		timer_Reload( &vsync_timer );

		graphics_flip_frame();
	}
}

void graphics_flip_frame( void )
{
	tthread_mutex_lock( lcd_mutex );
	lcd_driver_update_window();
	tthread_mutex_unlock( lcd_mutex );
}

void graphics_do_scroll( uint16_t height )
{
	tthread_mutex_lock( lcd_mutex );
	lcd_driver_do_scroll( height );
	tthread_mutex_unlock( lcd_mutex );
}

uint16_t graphics_width( void )
{
	return lcd_driver_get_width();
}

uint16_t graphics_height( void )
{
	return lcd_driver_get_height();
}

void graphics_clear_screen( void )
{
	tthread_mutex_lock( lcd_mutex );
	lcd_driver_clear_screen();
	tthread_mutex_unlock( lcd_mutex );
}

void graphics_set_pixel( uint16_t posx, uint16_t posy, uint16_t colour )
{
	tthread_mutex_lock( lcd_mutex );
	lcd_driver_set_pixel( posx, posy, colour );
	tthread_mutex_unlock( lcd_mutex );
}

void graphics_show_bitmap( uint16_t posx, uint16_t posy, const bitmap_t * bitmap, bool_t use_transparent )
{
	tthread_mutex_lock( lcd_mutex );
	int transparent;
	transparent = -1;
	if( use_transparent ) transparent = (int)bitmap->background_colour;
	graphics_set_bitmap_rgb( posx, posy, bitmap->width, bitmap->height, bitmap->buffer, transparent );
	tthread_mutex_unlock( lcd_mutex );
}

void graphics_set_bitmap_rgb( uint16_t posx, uint16_t posy, uint16_t width, uint16_t height, uint16_t * data, int i_transparent )
{
	tthread_mutex_lock( lcd_mutex );
	lcd_driver_show_bitmap_rgb( posx, posy, width, height, data, FALSE );
	tthread_mutex_unlock( lcd_mutex );
}

void graphics_fill_box( uint16_t posx, uint16_t posy, uint16_t width, uint16_t height, uint16_t colour )
{
	tthread_mutex_lock( lcd_mutex );
	lcd_driver_fill_colour( posx, posy, width, height, colour );
	tthread_mutex_unlock( lcd_mutex );
}

void graphics_bitmap_1bb( uint16_t posx, uint16_t posy, uint16_t width, uint16_t heigth, 
					      uint8_t *input, uint16_t back_colour, uint16_t fore_colour )
{
	uint16_t x;
	uint16_t y;
	uint16_t bit_mask;
	uint8_t * ptr_pixel;

	tthread_mutex_lock( lcd_mutex );

	for( y = 0; y < heigth; y++ )
	{
		if( y > lcd_driver_get_height() ) break;

		ptr_pixel = &input[ ( ( y >> 3 ) * width ) ];
		bit_mask = 0x01 << ( y & 0x07 );
		
		for( x = 0; x < width; x++ )
		{
			if( x > lcd_driver_get_width() ) break;

			if( ( ptr_pixel[ x ] & bit_mask ) != 0 )
			{
				lcd_driver_set_pixel( posx + x, posy + y, fore_colour );
			}
			else
			{
                lcd_driver_set_pixel( posx + x, posy + y, back_colour );
			}		
		}
	}
	tthread_mutex_unlock( lcd_mutex );
}
