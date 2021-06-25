

#include "sam.h"
#include "stdint.h"

#ifndef SPI_H
#define SPI_H

//#define SPI_CLK_FREQ 8000000
//#define SPI_BAUD 50000

//#define GPIO_GROUP_SS  0         // PORT group of PA13 (PORTA = PORT group 0)
//#define GPIO_MAP_SS    PORT_PA13 // PA13 bit position macro (1<<13)


#define GPIO_MAP_SS						PORT_PA13
#define GPIO_MAP_LED					PORT_PB30
#define GPIO_MAP_MISO					PORT_PB16
#define GPIO_MAP_MOSI					PORT_PB22
#define GPIO_MAP_SCK					PORT_PB23

#define GPIO_GROUP_SS					0
#define GPIO_GROUP_LED					1
#define GPIO_GROUP_SPI					1

#define GPIO_PIN_NUM_MISO				PIN_PA16

#define GPIO_LED_CLR()					PORT->Group[GPIO_GROUP_LED].OUTSET.reg = GPIO_MAP_LED;
#define GPIO_LED_SET()					PORT->Group[GPIO_GROUP_LED].OUTCLR.reg = GPIO_MAP_LED;
#define GPIO_LED_TGL()					PORT->Group[GPIO_GROUP_LED].OUTTGL.reg = GPIO_MAP_LED;

#define GPIO_SS_LOW()					PORT->Group[GPIO_GROUP_SS].OUTCLR.reg = GPIO_MAP_SS;
#define GPIO_SS_HIGH()					PORT->Group[GPIO_GROUP_SS].OUTSET.reg = GPIO_MAP_SS;


#define SPI_SERCOM_PINS_PORT_GROUP	1
#define SPI_MISO_PORT_PMUX			PORT_PMUX_PMUXE_C_Val
#define SPI_SERCOM_MISO_PIN			PORT_PB16
#define SPI_SCK_MOSI_PORT_PMUX		PORT_PMUX_PMUXE_D_Val
#define SPI_SERCOM_MOSI_PIN			PORT_PB22
#define SPI_SERCOM_SCK_PIN			PORT_PB23

#define SPI_SERCOM_CLK_GEN			0
#define SPI_SERCOM					SERCOM0
#define SPI_CLK_FREQ				8000000
#define SPI_BAUD					500000















void SPI_init(void);
uint8_t spiSend(uint8_t data);




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
