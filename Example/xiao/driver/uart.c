
#include "uart.h"

bool uart_drv_error_flag = false;

uint32_t uart_get_sercom_index(Sercom *sercom_instance) {
    /* Save all available SERCOM instances for compare. */
    Sercom *sercom_instances[SERCOM_INST_NUM] = SERCOM_INSTS;

    /* Find index for sercom instance. */
    for (uint32_t i = 0; i < SERCOM_INST_NUM; i++) {
        if ((uintptr_t)sercom_instance == (uintptr_t)sercom_instances[i]) {
            return i;
        }
    }

    return 0;
}

static uint64_t long_division(uint64_t n, uint64_t d)
{
    int32_t i;
    uint64_t q = 0, r = 0, bit_shift;
    for (i = 63; i >= 0; i--) {
    bit_shift = (uint64_t)1 << i;
    r = r << 1;
    if (n & bit_shift) {
    r |= 0x01;
    }
    if (r >= d) {
    r = r - d;
    q |= bit_shift;
    }
    }
    return q;
}

#define SHIFT 32
#define USART_SAMPLE_NUM 16

uint16_t calculate_baud_value(
const uint32_t baudrate,
const uint32_t peripheral_clock,
uint8_t sample_num)
{
/* Temporary variables */
    uint64_t ratio = 0;
    uint64_t scale = 0;
    uint64_t baud_calculated = 0;
    uint64_t temp1;
    /* Calculate the BAUD value */
    temp1 = ((sample_num * (uint64_t)baudrate) << SHIFT);
    ratio = long_division(temp1, peripheral_clock);
    scale = ((uint64_t)1 << SHIFT) - ratio;
    baud_calculated = (65536 * scale) >> SHIFT;
    return baud_calculated;
}

void uart_basic_init(Sercom *sercom, uint16_t baud_val, enum uart_pad_settings pad_conf) {
    /* Wait for synchronization */
    while (sercom->USART.SYNCBUSY.bit.ENABLE);

    /* Disable the SERCOM UART module */
    sercom->USART.CTRLA.bit.ENABLE = 0;
    /* Wait for synchronization */
    while (sercom->USART.SYNCBUSY.bit.SWRST);

    /* Perform a software reset */
    sercom->USART.CTRLA.bit.SWRST = 1;
    /* Wait for synchronization */
    while (sercom->USART.CTRLA.bit.SWRST) ;

    /* Wait for synchronization */
    while (sercom->USART.SYNCBUSY.bit.SWRST || sercom->USART.SYNCBUSY.bit.ENABLE)
        ;
    /* Update the UART pad settings, mode and data order settings */
    sercom->USART.CTRLA.reg = pad_conf |
            SERCOM_USART_CTRLA_MODE(1) |
            SERCOM_USART_CTRLA_DORD     |
            SERCOM_USART_CTRLA_SAMPR(0x0)|
            SERCOM_USART_CTRLA_RUNSTDBY |
            SERCOM_USART_CTRLA_MODE_USART_INT_CLK ;

    //

    /* Wait for synchronization */
    while (sercom->USART.SYNCBUSY.bit.CTRLB)
        ;
    /* Enable transmit and receive and set data size to 8 bits */
    sercom->USART.CTRLB.reg =
        SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_CHSIZE(0);
    /* Load the baud value */
    uint16_t baud;
    baud = calculate_baud_value(baud_val,48000000,USART_SAMPLE_NUM);


    //9600
    //(48000000/16)=3000000>=9600

    sercom->USART.BAUD.reg = baud_val;
    /* Wait for synchronization */
    while (sercom->USART.SYNCBUSY.bit.ENABLE)
        ;
    /* Enable SERCOM UART */
    sercom->USART.CTRLA.bit.ENABLE = 1;
    while(sercom->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);
}

void uart_disable(Sercom *sercom) {
    /* Wait for synchronization */
    while (sercom->USART.SYNCBUSY.bit.ENABLE)
        ;
    /* Disable SERCOM UART */
    sercom->USART.CTRLA.bit.ENABLE = 0;
}

void uart_write_byte(Sercom *sercom, uint8_t data) {
    /* Wait for Data Register Empty flag */
    while (!sercom->USART.INTFLAG.bit.DRE)
        ;
    /* Write the data to DATA register */
    sercom->USART.DATA.reg = (uint16_t)data;
    asm("nop");
}

uint8_t uart_read_byte(Sercom *sercom) {
    /* Wait for Receive Complete flag */
    while (!sercom->USART.INTFLAG.bit.RXC)
        ;
    /* Check for errors */
    if (sercom->USART.STATUS.bit.PERR || sercom->USART.STATUS.bit.FERR ||
        sercom->USART.STATUS.bit.BUFOVF)
        /* Set the error flag */
        uart_drv_error_flag = true;
    /* Return the read data */
    return ((uint8_t)sercom->USART.DATA.reg);
}

void uart_write_buffer_polled(Sercom *sercom, uint8_t *ptr, uint16_t length) {
    /* Do the following for specified length */
    do {
        /* Wait for Data Register Empty flag */
        while (!sercom->USART.INTFLAG.bit.DRE)
            ;
        /* Send data from the buffer */
        sercom->USART.DATA.reg = (uint16_t)*ptr++;
    } while (length--);
}

void uart_read_buffer_polled(Sercom *sercom, uint8_t *ptr, uint16_t length) {
    /* Do the following for specified length */
    do {
        /* Wait for Receive Complete flag */
        while (!sercom->USART.INTFLAG.bit.RXC)
            ;
        /* Check for errors */
        if (sercom->USART.STATUS.bit.PERR || sercom->USART.STATUS.bit.FERR ||
            sercom->USART.STATUS.bit.BUFOVF)
            /* Set the error flag */
            uart_drv_error_flag = true;
        /* Store the read data to the buffer */
        *ptr++ = (uint8_t)sercom->USART.DATA.reg;
    } while (length--);
}


static inline void pin_set_peripheral_function(uint32_t pinmux)
{
    uint8_t port = (uint8_t)((pinmux >> 16)/32);
    PORT->Group[port].PINCFG[((pinmux >> 16) - (port*32))].bit.PMUXEN = 1;
    PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg &= ~(0xF << (4 * ((pinmux >>
    16) & 0x01u)));
    PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg |= (uint8_t)((pinmux &
    0x0000FFFF) << (4 * ((pinmux >> 16) & 0x01u)));
}

void uart_clock_init(Sercom *sercom){
        uint32_t port;
        uint8_t pin;

    //PB08 UART_Tx SCOM4PAD0
    //PB09 UART_Rx SCOM4PAD1
    pin_set_peripheral_function(PINMUX_PB08D_SERCOM4_PAD0);
    pin_set_peripheral_function(PINMUX_PB09D_SERCOM4_PAD1);

    uint32_t inst = uart_get_sercom_index(sercom);
    /* Enable clock for BOOT_USART_MODULE */
    PM->APBCSEL.bit.APBCDIV = 0;       //no presescaler
    PM->APBCMASK.reg |= (1u << (inst + PM_APBCMASK_SERCOM0_Pos));

    /* Set GCLK_GEN3 as source for GCLK_ID_SERCOMx_CORE */
    GCLK_CLKCTRL_Type clkctrl = {0};
    uint16_t temp;
    GCLK->CLKCTRL.bit.ID = inst + GCLK_ID_SERCOM0_CORE;
    temp = GCLK->CLKCTRL.reg;
    clkctrl.bit.CLKEN = true;
    clkctrl.bit.WRTLOCK = false;
    clkctrl.bit.GEN = GCLK_CLKCTRL_GEN_GCLK3_Val;
    GCLK->CLKCTRL.reg = (clkctrl.reg | temp);


}
