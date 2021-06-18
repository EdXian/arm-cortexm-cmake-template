#include "stdio.h"
#include "sam.h"


const uint8_t test_data[10]={0};
//drive GPIO PA17
//drive usb

void togle_pin(void);
void togle_pin(void){

    REG_PORT_OUTTGL0 = PORT_PA17;
}

GCLK_GENCTRL_Type gclk1_genctrl = {
    .bit.RUNSTDBY = 0,        /* Generic Clock Generator is stopped in stdby */
    .bit.DIVSEL =  0,            /* Use GENDIV.DIV value to divide the generator */
    .bit.OE = 0,            /* Disable generator output to GCLK_IO[1] */
    .bit.OOV = 0,            /* GCLK_IO[0] output value when generator is off */
    .bit.IDC = 1,            /* Generator duty cycle is 50/50 */
    .bit.GENEN = 1,            /* Enable the generator */
    .bit.SRC = 0x05,            /* Generator source: XOSC32K output */
    .bit.ID = 1            /* Generator ID: 1 */
};

int main(){


   GCLK->GENCTRL.reg = gclk1_genctrl.reg;

   REG_PORT_DIRSET0  =  PORT_PA17;
   REG_PORT_OUTSET0 =  PORT_PA17;


    while(1){
        for (unsigned i = 0; i < (SystemCoreClock / 4U); i++)
        {
            asm("nop");
        }
         togle_pin();
    }
}
