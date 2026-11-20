#ifndef spi_controller_h
#define spi_controller_h

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////////////

#include "board.h"

//////////////////////////////////////////////////////////////////////////////////////

#if 1 //def SPLIT_CPLD
#define SPI_REGISTER_APL2CTRL           0
#define SPI_REGISTER_CTRL2APL           1
#define SPI_REGISTER_KEYBOARD           2
#define SPI_REGISTER_NONE               5
#else
#define SPI_REGISTER_COMMAND            0
#define SPI_REGISTER_APL2CTRL           1
#define SPI_REGISTER_CTRL2APL           2
#define SPI_REGISTER_KEYBOARD           3
#define SPI_REGISTER_IRQ                4
#define SPI_REGISTER_NONE               5
#endif

//////////////////////////////////////////////////////////////////////////////////////

void spi_controller_init( void );

uint8_t spi_controller_data_start( uint8_t addr, uint8_t data_in );
bool_t spi_controller_check_complete( uint8_t id, spi_data_t * spi_resp );

void spi_controller_process( void * p_cpld );

//////////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* spi_controller_h */
