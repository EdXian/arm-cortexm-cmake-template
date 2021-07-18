#include "board.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include"tusb_config.h"
#include "atmel_start.h"
#include "atmel_start_pins.h"
#include "hal_gpio.h"
#include "hal_init.h"
#include "hri_nvmctrl_d21.h"

//#include "hpl_gclk_base/.h"
#include "hpl_pm_config.h"
#include "hpl_pm_base.h"
#include "hpl_pm_config.h"



#include "tusb.h"

#include "uart.h"
#include "spi.h"
#include "i2c.h"
#include "flash.h"
#include "clock.h"
#include "dma.h"
uint32_t test1[10]={0};
uint32_t test2[10]={0};
uint32_t test3[10]={0};
uint32_t test4[10]={0};


void tx_cb1(){
    asm("nop");
}
void tx_cb2(){
    asm("nop");
}
void tx_cb3(){
    asm("nop");
}
void tx_cb4(){
    asm("nop");
}
void (*fptr[4])(void);

void DMAC_Handler(){
    asm("nop");
    //DMAC->CHID.bit.ID=DMAC_CHID_ID(0);
    //trasmit complete
     uint8_t channel = DMAC->INTPEND.bit.ID;
     uint8_t flags;
    DMAC->CHID.bit.ID=DMAC_CHID_ID(channel);
    flags = DMAC->CHINTFLAG.reg;
    if(flags & DMAC_CHINTENCLR_TCMPL){
        DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TCMPL;
        for(uint32_t i=0;i<4;i++){
            if(fptr[i])fptr[i]();  //execute call back function
        }
    }else if(flags & DMAC_INTPEND_SUSP){
        DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_SUSP;
    }else if(flags & DMAC_INTPEND_TERR){
        DMAC->CHINTFLAG.reg = DMAC_CHINTENCLR_TERR;
    }
    //asm("nop");


    //DMAC->INTPEND.bit.TCMPL=1;
}



int main(){

   clock_source_init();

   for(uint8_t i=0;i<10;i++){
    test1[i] = 0x12345678;
   }

    dma_enable();
    dma_ch_init(0);
    NVIC_EnableIRQ(DMAC_IRQn);
//    dma_ch_init(1);
//    dma_ch_init(2);
    dma_add_descriptior(0);
//    dma_add_descriptior(1);
//    dma_add_descriptior(2);
    NVIC_SetPriority(DMAC_IRQn,4);
    NVIC_EnableIRQ(DMAC_IRQn);
    dma_send(0,test1,test2,40);
    dma_start_transaction(0);

    fptr[0] = tx_cb1;
    fptr[1] = tx_cb2;
    fptr[2] = tx_cb3;
    //fptr[3] = tx_cb4;
    while(1){
        asm("nop");
        delay_ms(100);
    }
}
