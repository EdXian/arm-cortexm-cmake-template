#include "dma.h"




void dma_enable(){
 //enter criticl section

 __disable_irq();
 //AHB APB clock must be enable
 PM->AHBMASK.bit.DMAC_=1;
 PM->APBBMASK.bit.DMAC_=1;

 //  The SRAM address of where the descriptor memory section is located must be written to the Description Base
 //  Address (BASEADDR) register
 DMAC->BASEADDR.reg = 0x20000000;  //it is the address of the dma descriptor
 DMAC->WRBADDR.reg = 0x20001100;

 DMAC->CTRL.bit.LVLEN0 =1;
 DMAC->CTRL.bit.DMAENABLE=0;
 DMAC->CTRL.bit.CRCENABLE = 0;

 DMAC->CTRL.bit.SWRST=1;
 while (DMAC->CTRL.bit.SWRST);
 //destination & source


// These bits define the channel number that will be affected by the channel registers (CH*). Before reading or writing a
// channel register, the channel ID bit group must be written first.
   DMAC->CHID.bit.ID=0;




  //CHCTRLA


   //CHCTRLB
  //DMAC->CHCTRLB.bit.TRIGACT = 1; //one trigger required for each beat/block/transaction.
  //DMAC->CHCTRLB.bit.TRIGSRC = SERCOM5_DMAC_ID_TX;  //for example trigger source


   //CHINTENCLR
   //CHINTENSET
   //CHINTFLAG
   //CHSTATUS

  //set decriptor
  DmacDescriptor descript_dma={
      .BTCNT=1,
      .DSTADDR = 0x20000100,
      .SRCADDR = 0x20000000,
  };



  DMAC_SRCADDR_Type src_addr;
  DMAC_DSTADDR_Type dst_addr;
  dst_addr.reg = 0x20000100;
  src_addr.reg = 0x20000000;




  __enable_irq();
 //exit criticl section
}
