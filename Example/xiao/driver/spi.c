
#include "spi.h"

void spi_clock_init(){
    //if sercom 5

    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM5;


    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM5_GCLK_ID_CORE) |
        GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);
    SERCOM5->SPI.CTRLA.bit.ENABLE = 0;

}


void spi_init(){

    // Pre-setting Slave Select (SS) high
    PORT->Group[GPIO_GROUP_SS].OUTSET.reg = GPIO_MAP_SS;

    // Setting SPI Slave Select as an output
    PORT->Group[GPIO_GROUP_SS].DIRSET.reg = GPIO_MAP_SS;


    spi_clock_init();

    SERCOM5->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE_SPI_MASTER|
        SERCOM_SPI_CTRLA_DOPO(1);
}

void spi_interrupt_enalbe(){

}

void spi_set_baudrate(uint32_t rate){


// Calculate BAUD value
uint16_t BAUD_REG = ((float)SPI_CLK_FREQ / (float)(2 * rate)) - 1;

// Set the SPI baud rate
SERCOM5->SPI.BAUD.reg = SERCOM_SPI_BAUD_BAUD(BAUD_REG);

// Enable the Sercom SPI
SERCOM5->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_ENABLE;

// What for synchronization of SERCOM SPI registersbwtween the clock domains
while(SERCOM5->SPI.SYNCBUSY.bit.ENABLE);
}
