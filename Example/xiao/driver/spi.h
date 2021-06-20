

#include "sam.h"
#include "stdint.h"

#ifndef SPI_H
#define SPI_H

#define SPI_CLK_FREQ 8000000
#define SPI_BAUD 50000

#define GPIO_GROUP_SS  0         // PORT group of PA13 (PORTA = PORT group 0)
#define GPIO_MAP_SS    PORT_PA13 // PA13 bit position macro (1<<13)




/**
 * @brief spi_clock_init
 */
void spi_clock_init();

/**
 * @brief spi_init
 */
void spi_init();

/**
 * @brief spi_set_baudrate
 */
void spi_set_baudrate();

/**
 * @brief spi_interrupt_enalbe
 */
void spi_interrupt_enalbe();
#endif // SPI_H
