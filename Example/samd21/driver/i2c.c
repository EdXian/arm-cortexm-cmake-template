#include "i2c.h"
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

    while(SERCOM2->I2CM.SYNCBUSY.bit.ENABLE);

    SERCOM2->I2CM.CTRLA.bit.ENABLE = 0;

    while(SERCOM2->I2CM.SYNCBUSY.bit.SWRST);

    SERCOM2->I2CM.CTRLA.bit.SWRST = 1;

        /* Wait for synchronization */
    while(SERCOM2->I2CM.CTRLA.bit.SWRST);

        /* Wait for synchronization */
    while(SERCOM2->I2CM.SYNCBUSY.bit.SWRST || SERCOM2->I2CM.SYNCBUSY.bit.ENABLE);

    pin_set_peripheral_function(PINMUX_PA08D_SERCOM2_PAD0);
    pin_set_peripheral_function(PINMUX_PA09D_SERCOM2_PAD1);

    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM2;								//Enable the SERCOM 5 under the PM
    GCLK_CLKCTRL_Type clkctrl = {0};
    uint16_t temp;
    GCLK->CLKCTRL.bit.ID =  SERCOM2_GCLK_ID_CORE;
    temp = GCLK->CLKCTRL.reg;
    clkctrl.bit.CLKEN = 1;
    clkctrl.bit.WRTLOCK = 0;
    clkctrl.bit.GEN = GCLK_CLKCTRL_GEN_GCLK3_Val;                           //set gclk3 for spi
    GCLK->CLKCTRL.reg = (clkctrl.reg | temp);

    while(GCLK->STATUS.bit.SYNCBUSY);										//Wait for clock sync

    SERCOM2->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_SPEED (1) |
                                SERCOM_I2CM_CTRLA_SDAHOLD(0x2) |
                                SERCOM_I2CM_CTRLA_RUNSTDBY |
                                SERCOM_I2CM_CTRLA_MODE_I2C_MASTER;

    SERCOM2->I2CM.CTRLB.reg = SERCOM_I2CM_CTRLB_SMEN;
    /* synchronization busy */
    while(SERCOM2->I2CM.SYNCBUSY.bit.SYSOP);
    /* BAUDLOW is non-zero, and baud register is loaded */
    SERCOM2->I2CM.BAUD.reg = SERCOM_I2CM_BAUD_BAUD(11) | SERCOM_I2CM_BAUD_BAUDLOW(22);
    /* synchronization busy */
    while(SERCOM2->I2CM.SYNCBUSY.bit.SYSOP);
    /* SERCOM2 peripheral enabled by setting the ENABLE bit as 1*/
    SERCOM2->I2CM.CTRLA.reg |= SERCOM_I2CM_CTRLA_ENABLE;
    /* SERCOM Enable synchronization busy */
    while((SERCOM2->I2CM.SYNCBUSY.reg & SERCOM_I2CM_SYNCBUSY_ENABLE));
    /* bus state is forced into idle state */
    SERCOM2->I2CM.STATUS.bit.BUSSTATE = 0x1;
    /* synchronization busy */
    while(SERCOM2->I2CM.SYNCBUSY.bit.SYSOP);
    /* Both master on bus and slave on bus interrupt is enabled */
    SERCOM2->I2CM.INTENSET.reg = SERCOM_I2CM_INTENSET_MB | SERCOM_I2CM_INTENSET_SB;

}

void i2c_init(){





}

void SERCOM2_Handler(){

}
#define SLAVE_ADDR 0x7f

uint8_t i=0,tx_done,rx_done;
void i2c_master_transact(void)
{
i = 0;
/* Acknowledge section is set as ACK signal by
 writing 0 in ACKACT bit */
SERCOM2->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
while(SERCOM2->I2CM.SYNCBUSY.bit.SYSOP);
/* slave address with Write(0) */
SERCOM2->I2CM.ADDR.reg = (SLAVE_ADDR << 1) | 0;
while(!tx_done);
i = 0;
/* Acknowledge section is set as ACK signal by
 writing 0 in ACKACT bit */
SERCOM2->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;
while(SERCOM2->I2CM.SYNCBUSY.bit.SYSOP);
/* slave address with read (1) */
SERCOM2->I2CM.ADDR.reg = (SLAVE_ADDR << 1) | 1;
while(!rx_done);
/*interrupts are cleared */
SERCOM2->I2CM.INTENCLR.reg = SERCOM_I2CM_INTENCLR_MB | SERCOM_I2CM_INTENCLR_SB;
}
