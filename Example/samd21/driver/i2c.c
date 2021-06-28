#include "i2c.h"

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


//SDA PA8   SCOM2PAD0
//SCL PA9   SCOM2PAD1

void i2c_clock_init(){

    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM2;
    GCLK_CLKCTRL_Type clkctrl = {0};
    uint16_t temp;
    GCLK->CLKCTRL.bit.ID =  SERCOM2_GCLK_ID_CORE;
    temp = GCLK->CLKCTRL.reg;
    clkctrl.bit.CLKEN = 1;
    clkctrl.bit.WRTLOCK = 0;
    clkctrl.bit.GEN = GCLK_CLKCTRL_GEN_GCLK3_Val;   //8Mhz                         //set gclk3 for spi
    GCLK->CLKCTRL.reg = (clkctrl.reg | temp);

    while(GCLK->STATUS.bit.SYNCBUSY);

    pin_set_peripheral_function(PINMUX_PA08D_SERCOM2_PAD0);
    pin_set_peripheral_function(PINMUX_PA09D_SERCOM2_PAD1);





    SERCOM2->I2CM.CTRLA.bit.SWRST = 1;                                          		// Reset the SERCOM
  while (SERCOM2->I2CM.CTRLA.bit.SWRST || SERCOM2->I2CM.SYNCBUSY.bit.SWRST);  		// Wait for synchronization

  SERCOM2->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_MODE_I2C_MASTER;   			// Set I2C master mode
  SERCOM2->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN;   										  			// Enable Smart Mode
  SERCOM2->I2CM.BAUD.bit.BAUD = 48000000 / (2 * 100000) - 7;      			// Set I2C master SCL baud rate
    SERCOM2->I2CM.CTRLA.bit.ENABLE = 1 ;            // Enable SERCOM in I2C master mode
    while (SERCOM2->I2CM.SYNCBUSY.bit.ENABLE);      // Wait for synchronization
    SERCOM2->I2CM.STATUS.bit.BUSSTATE = 0x01;       // Set the I2C bus to IDLE state
    while (SERCOM2->I2CM.SYNCBUSY.bit.SYSOP);       // Wait for synchronization
    SERCOM2->I2CM.INTENSET.bit.ERROR = 1;					 // Enable SERCOM ERROR interrupts



    SERCOM2->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_ENABLE |
    SERCOM_I2CM_CTRLA_MODE_I2C_MASTER |
    SERCOM_I2CM_CTRLA_SDAHOLD(3);
    while (SERCOM2->I2CM.SYNCBUSY.reg);

    SERCOM2->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB;
//    NVIC_EnableIRQ(SERCOM2_IRQn);

    SERCOM2->I2CM.CTRLA.bit.ENABLE = 1 ;
    while ( SERCOM2->I2CM.SYNCBUSY.bit.ENABLE != 0 );

    SERCOM2->I2CM.STATUS.reg |= SERCOM_I2CM_STATUS_BUSSTATE(1);
    while (SERCOM2->I2CM.SYNCBUSY.reg);

    /* Both master on bus and slave on bus interrupt is enabled */
    //SERCOM2->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB;
    /* SERCOM2 handler enabled */
    //system_interrupt_enable(SERCOM2_IRQn);
    //NVIC_EnableIRQ(SERCOM2_IRQn);
}


void i2c_enable(){

    SERCOM2->I2CM.STATUS.bit.BUSSTATE = 1 ;   //write 1 to it to make the bus idle.
    while ( SERCOM2->I2CM.SYNCBUSY.bit.SYSOP != 0 );
}


void i2c_init(){





}


static uint8_t i=0;
static volatile uint8_t tx_done = 0, rx_done = 0;


uint8_t i2c_write_start(uint8_t data)
{
  SERCOM2->I2CM.ADDR.reg = 0x69;

  while (0 == (SERCOM2->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

}

void i2c_write(uint8_t cmd){

    SERCOM2->I2CS.DATA.reg = cmd;

    while (0 == (SERCOM2->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB));

}

void i2c_write_stop(void)
{
  SERCOM2->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
}

uint8_t i2c_read(uint8_t *data, int size)
{
  SERCOM2->I2CM.ADDR.reg = 0x69;

  while (0 == (SERCOM2->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB));

  if (SERCOM2->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
  {
    SERCOM2->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
    //dbg_log("I2C: RXNACK during read (address)\r\n");
    return false;
  }

  SERCOM2->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;

  for (int i = 0; i < size-1; i++)
  {
    data[i] = SERCOM2->I2CM.DATA.reg;
    while (0 == (SERCOM2->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB));
  }

  if (size)
  {
    SERCOM2->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
    SERCOM2->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
    data[size-1] = SERCOM2->I2CM.DATA.reg;
  }

  return true;
}

void i2c_nack(){
   SERCOM2->I2CM.CTRLB.bit.ACKACT = 1;
}


void i2c_ack(){

    SERCOM2->I2CM.CTRLB.bit.ACKACT = 0;
}


//void SERCOM2_Handler(){
// asm("nop");
//}

//void i2c_enable()
//{

//  // Enable the I²C master mode
//  SERCOM2->I2CM.CTRLA.bit.ENABLE = 1 ;

//  while ( SERCOM2->I2CM.SYNCBUSY.bit.ENABLE != 0 )
//  {
//    // Waiting the enable bit from SYNCBUSY is equal to 0;
//  }

//  // Setting bus idle mode
//  SERCOM2->I2CM.STATUS.bit.BUSSTATE = 1 ;

//  while ( SERCOM2->I2CM.SYNCBUSY.bit.SYSOP != 0 )
//  {
//    // Wait the SYSOP bit from SYNCBUSY coming back to 0
//  }
//}

