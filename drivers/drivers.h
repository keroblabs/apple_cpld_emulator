#ifndef _hal_driver_h_
#define _hal_driver_h_

//////////////////////////////////////////////////////////////////////////////////////

#include "types.h"

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

void keypad_driver_init( void );
bool_t keypad_driver_has_data( void );
uint8_t keypad_driver_read_data( void );

void lcd_driver_deinit( void );

void mouse_driver_init( uint8_t index );
bool_t mouse_driver_has_data( uint8_t index );
uint8_t mouse_driver_read_data( uint8_t index );
void mouse_driver_process( void );
void mouse_driver_write_command( uint8_t index, uint8_t *buffer, uint32_t max_size );

void lcd_driver_init( void );
uint16_t lcd_driver_get_width( void );
uint16_t lcd_driver_get_height( void );
void lcd_driver_clear_screen( void );
void lcd_driver_update_window( void );
void lcd_driver_do_scroll( uint16_t height );
void lcd_driver_set_pixel( uint16_t in_posx, uint16_t in_posy, uint16_t colour );
void lcd_driver_show_bitmap_rgb( uint16_t posx, uint16_t posy, uint16_t width, uint16_t height, uint16_t * bitmap, int use_transparent );
void lcd_driver_fill_colour( uint16_t in_posx, uint16_t in_posy, uint16_t in_width, uint16_t in_height, uint16_t colour );

//////////////////////////////////////////////////////////////////////////////////////

#endif // _hal_driver_h_
