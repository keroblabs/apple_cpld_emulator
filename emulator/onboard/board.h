#ifndef _board_h_
#define _board_h_

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////////

#include "types.h"

//////////////////////////////////////////////////////////////////////////////////////

#define LCD_WIDTH					_USE_SCREEN_WIDTH
#define LCD_HEIGHT					_USE_SCREEN_HEIGHT

#define MEMORY_AUX_BANK				0x10000
#define MEM_GS_VIDEO_CHAR_MAP       0x20000
#define MEM_IO_REGION_ADDR          0x0C000
#define MEM_ROM_REGION_ADDR         0x40000
#define MEM_KEY_REGION_ADDR         0x50000
#define RAM_GS_MEGA2_ADDR           0

#define RAM_MEMORY_SIZE             0x020000  // 2 * 64 * 1024

#define ENABLE_VIDEO

#define SCREEN_APPLE_TOP_MARGIN     4
#define SCREEN_WIDTH				640
#define SCREEN_HEIGHT				200
#define SCREEN_SWITCH_Y				160
#define SCREEN_APPLE_MARGIN         ( ( SCREEN_WIDTH - 560 ) >> 1 )

#define BUTTON_WIDTH                20

#define STATUS_NONE       0
#define STATUS_IRQ        1
#define STATUS_NMI        2
#define STATUS_BRK        3

#define IRQ_STATE_NONE                  0
#define IRQ_STATE_PENDING               1
#define IRQ_STATE_SERVING               2

#define IRQ_MASK_IRQ                    0x08000000
#define IRQ_MASK_NMI                    0x10000000

enum
{
    CPU_HALT_NONE,
    CPU_HALT_BREAK,
    CPU_HALT_STEP,
    CPU_HALT_SYNC,
    CPU_HALT_SWI,
    CPU_HALT_INVALID
};

enum
{
    CPU_BREAK_NONE,
    CPU_BREAK_STOP,
    CPU_BREAK_RUN
};

enum
{
	LED_ID_LIST = 1000,

	LED_ID_HDD_D1,
	LED_ID_HDD_D2,
	LED_ID_DISC_D1,
	LED_ID_DISC_D2,
	LED_ID_TURBO,
	LED_ID_CAPSLOCK,
	LED_ID_PRINTER,
	LED_ID_DESKTOP_1,
	LED_ID_DESKTOP_2,
	LED_ID_DESKTOP_3,
	LED_ID_DESKTOP_4,
	LED_ID_DESKTOP_5,
	LED_ID_DESKTOP_6,
	LED_ID_DESKTOP_7,
	LED_ID_DESKTOP_8,
	LED_ID_DESKTOP_9,
};

//////////////////////////////////////////////////////////////////////////////////////

typedef void ( *loop_hdl_t )( uint64_t cycle );
typedef void ( *vbl_hdl_t )( void );
typedef void ( *hbl_hdl_t )( void );

typedef uint8_t ( *io_addr_t )( uint8_t addr, uint8_t data, bool_t is_write );

typedef struct spi_data_t_
{
    uint8_t id;
    uint8_t addr;
    uint8_t data;
    uint8_t flags;

} spi_data_t;

//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////

void board_startup( void );
void board_reset( void );
void board_execute( void );
void board_video_execute( void );

uint8_t board_spi_data_exchange( uint8_t addr, uint8_t data_in, uint8_t * data_out );
bool_t board_spi_check_complete( uint8_t id, spi_data_t * spi_resp );

//////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif // _board_h_
