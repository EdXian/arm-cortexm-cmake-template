
#include "spi.h"
#include "hal_gpio.h"
static void pin_set_peripheral_function(uint32_t pinmux)
{
 uint8_t port = (uint8_t)((pinmux >> 16)/32);
 PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg &= ~(0xF << (4 * ((pinmux >>
16) & 0x01u)));
 PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg |= (uint8_t)((pinmux &
0x0000FFFF) << (4 * ((pinmux >> 16) & 0x01u)));
 PORT->Group[port].PINCFG[((pinmux >> 16) - (port*32))].bit.PMUXEN = 1;
}


void spi_clock_init(){
    //if sercom 5

    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;


    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(SERCOM0_GCLK_ID_CORE) |
        GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN(0);
    SERCOM0->SPI.CTRLA.bit.ENABLE = 0;

}


void spi_init(){

    while(SERCOM5->SPI.SYNCBUSY.bit.ENABLE);

        /* Disable the SERCOM SPI module */
    SERCOM5->SPI.CTRLA.bit.ENABLE = 0;

        /* Wait for synchronization */
    while(SERCOM5->SPI.SYNCBUSY.bit.SWRST);
            /* Perform a software reset */
    SERCOM5->SPI.CTRLA.bit.SWRST = 1;

        /* Wait for synchronization */
    while(SERCOM5->SPI.CTRLA.bit.SWRST);

        /* Wait for synchronization */
    while(SERCOM5->SPI.SYNCBUSY.bit.SWRST || SERCOM5->SPI.SYNCBUSY.bit.ENABLE);


    pin_set_peripheral_function(PINMUX_PA05D_SERCOM0_PAD1);  //miso  DI
    pin_set_peripheral_function(PINMUX_PA06D_SERCOM0_PAD2);  //mosi  DO
    pin_set_peripheral_function(PINMUX_PA07D_SERCOM0_PAD3);  //sclk  clock


    gpio_set_pin_direction(PORT_PA10,GPIO_DIRECTION_OUT);

    //PORT_PA10
    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;								//Enable the SERCOM 5 under the PM
    GCLK_CLKCTRL_Type clkctrl = {0};
    uint16_t temp;
    GCLK->CLKCTRL.bit.ID =  SERCOM0_GCLK_ID_CORE;
    temp = GCLK->CLKCTRL.reg;
    clkctrl.bit.CLKEN = 1;
    clkctrl.bit.WRTLOCK = 0;
    clkctrl.bit.GEN = GCLK_CLKCTRL_GEN_GCLK3_Val;                           //set gclk3 for spi
    GCLK->CLKCTRL.reg = (clkctrl.reg | temp);

    while(GCLK->STATUS.bit.SYNCBUSY);										//Wait for clock sync

    SPI_SERCOM->SPI.CTRLA.reg = SERCOM_SPI_CTRLA_MODE_SPI_MASTER|			//Configure the Peripheral as SPI Master
    SERCOM_SPI_CTRLA_DOPO(1)| SERCOM_SPI_CTRLA_DIPO(1);                     //DOPO is set to PAD[2,3]


    SPI_SERCOM->SPI.CTRLB.reg = SERCOM_SPI_CTRLB_RXEN;						//Enable receive on SPI

    uint16_t BAUD_REG = ((float)SPI_CLK_FREQ / (float)(2 * SPI_BAUD)) - 1;	//Calculate BAUD value
    SPI_SERCOM->SPI.BAUD.reg =	SERCOM_SPI_BAUD_BAUD(BAUD_REG);				//Set the SPI baud rate
    SPI_SERCOM->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_ENABLE;					//Enable the Sercom SPI
    while(SPI_SERCOM->SPI.SYNCBUSY.bit.ENABLE);

}




uint8_t spiSend(uint8_t data)
{
    while(SPI_SERCOM->SPI.INTFLAG.bit.DRE == 0);
    SPI_SERCOM->SPI.DATA.reg = data;
    while(SPI_SERCOM->SPI.INTFLAG.bit.RXC == 0);
    return (uint8_t)SPI_SERCOM->SPI.DATA.reg;
}



void spi_interrupt_enalbe(){

}

void spi_set_baudrate(uint32_t rate){

//gpio_set_pin_level(PORT_PA10,0);
// Calculate BAUD value
uint16_t BAUD_REG = ((float)SPI_CLK_FREQ / (float)(2 * rate)) - 1;

// Set the SPI baud rate
SERCOM5->SPI.BAUD.reg = SERCOM_SPI_BAUD_BAUD(BAUD_REG);

// Enable the Sercom SPI
SERCOM5->SPI.CTRLA.reg |= SERCOM_SPI_CTRLA_ENABLE;

// What for synchronization of SERCOM SPI registersbwtween the clock domains
while(SERCOM5->SPI.SYNCBUSY.bit.ENABLE);
//gpio_set_pin_level(PORT_PA10,1);
}
