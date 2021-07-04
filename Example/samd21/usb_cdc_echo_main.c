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

#include "hpl_gclk_base.h"
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
void DMAC_Handler(){
    asm("nop");
    DMAC->CHID.bit.ID=DMAC_CHID_ID(0);
    //trasmit complete
    if(DMAC->CHINTFLAG.reg & DMAC_INTPEND_TCMPL){

    }else if(DMAC->CHINTFLAG.reg & DMAC_INTPEND_SUSP){

    }else if(DMAC->CHINTFLAG.reg & DMAC_INTPEND_TERR){

    }
    asm("nop");
    

	DMAC->INTPEND.bit.TCMPL=1;	
}



int main(){

   clock_source_init();

   for(uint8_t i=0;i<10;i++){
    test1[i] = 0x12345678;
   }

    dma_enable();
    dma_ch_init(0);
//    dma_ch_init(1);
//    dma_ch_init(2);
    dma_add_descriptior(0);
//    dma_add_descriptior(1);
//    dma_add_descriptior(2);
    NVIC_SetPriority(DMAC_IRQn,4);
    NVIC_EnableIRQ(DMAC_IRQn);



    dma_send(0,(uint32_t* )test1,(uint32_t* )test2,40);
    dma_start_transaction(0);
//    dma_send(1,(uint32_t* )test1,(uint32_t* )test3,40);
//    delay_ms(100);
//    dma_send(2,(uint32_t* )test2,(uint32_t* )test4,40);
    delay_ms(50);
    delay_ms(50);
    for(uint8_t i=0;i<10;i++){
     test1[i] = &test1[i];
    }
    test3[3] = 0;
    dma_start_transaction(0);
    delay_ms(50);
    delay_ms(50);
    while(1){
        asm("nop");
        delay_ms(100);
    }
}
