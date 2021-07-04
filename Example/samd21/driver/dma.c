#include "dma.h"
#include "sam.h"

volatile DmacDescriptor wrb[DMAC_CH_NUM] __attribute__ ((aligned (16)));
DmacDescriptor descriptor_section[DMAC_CH_NUM] __attribute__ ((aligned (16)));
DmacDescriptor descriptor __attribute__ ((aligned (16)));

void dma_clock_init(){
    PM->AHBMASK.bit.DMAC_=1;                                                // DMAC place holder descriptor
    PM->APBBMASK.bit.DMAC_=1;
}

void dma_start_transaction(uint8_t ch){
    DMAC->CHID.bit.ID=DMAC_CHID_ID(ch);
    DMAC->CHCTRLA.bit.ENABLE=1;
    DMAC->SWTRIGCTRL.reg |= DMAC_SWTRIGCTRL_SWTRIG0;
}

uint32_t dma_channel_ind =0;


void dma_add_descriptior(uint8_t ch){
    if(dma_channel_ind<1){
        //DMAC->CHID.bit.ID=DMAC_CHID_ID(0);
        descriptor_section[0].BTCTRL.bit.STEPSIZE = 0;
        descriptor_section[0].BTCTRL.bit.STEPSEL = 0x00;
        descriptor_section[0].BTCTRL.bit.SRCINC = 1;    //increase 1 byte
        descriptor_section[0].BTCTRL.bit.DSTINC = 1;    //increase 1 byte
        descriptor_section[0].BTCTRL.bit.BEATSIZE = 0;  //1 byte for a beat

        //check the descp is valid.
        descriptor_section[0].DESCADDR.reg = 0x00000000;             //if we dont have the next descp,descaddr = 0
        descriptor_section[0].BTCTRL.bit.VALID=1;
        dma_channel_ind++;
    }else if(dma_channel_ind <DMAC_CH_NUM){
        //append the latest dscrip to the tail
        descriptor_section[dma_channel_ind-1].DESCADDR.reg = (uint32_t*)&descriptor_section[1];
        descriptor_section[dma_channel_ind].BTCTRL.bit.STEPSIZE = 0;
        descriptor_section[dma_channel_ind].BTCTRL.bit.STEPSEL = 0x00;
        descriptor_section[dma_channel_ind].BTCTRL.bit.SRCINC = 1;    //increase 1 byte
        descriptor_section[dma_channel_ind].BTCTRL.bit.DSTINC = 1;    //increase 1 byte
        descriptor_section[dma_channel_ind].BTCTRL.bit.BEATSIZE = 0;  //1 byte for a beat

        //check the descp is valid.
        descriptor_section[dma_channel_ind].DESCADDR.reg = 0x00000000;             //if we dont have the next descp,descaddr = 0
        descriptor_section[dma_channel_ind].BTCTRL.bit.VALID=1;
        dma_channel_ind++;
    }else{

    }
}

void dma_ch_init(uint8_t ch){
    DMAC->CHID.bit.ID=DMAC_CHID_ID(ch);

    DMAC->PRICTRL0.bit.LVLPRI0=1;

//DMAC->CHCTRLB.reg =DMAC_CHCTRLB_TRIGACT_BEAT;
    DMAC->CHID.bit.ID=DMAC_CHID_ID(ch);

//    DMAC->CHCTRLB.reg =DMAC_CHCTRLB_LVL(0) |
//            DMAC_CHCTRLB_TRIGSRC(0x00) |
//             DMAC_CHCTRLB_TRIGACT_BLOCK;
    DMAC->CHINTENSET.bit.SUSP=1;

    DMAC->CHINTENSET.bit.TCMPL=1;
    DMAC->CHINTENSET.bit.TERR=1;
    DMAC->CHCTRLB.reg =DMAC_CHCTRLB_LVL(0) |
            DMAC_CHCTRLB_TRIGSRC(0x00) ;
//// These bits define the channel number that will be affected by the channel registers (CH*). Before reading or writing a
//// channel register, the channel ID bit group must be written first.

 //DMAC->SWTRIGCTRL.reg &= ~(DMAC_SWTRIGCTRL_SWTRIG0);
    DMAC->SWTRIGCTRL.reg &= ~(DMAC_SWTRIGCTRL_SWTRIG(ch));
}

void dma_enable(){
// //enter criticl section
    dma_clock_init();

 __disable_irq();
// //AHB APB clock must be enable

    DMAC->CTRL.bit.DMAENABLE = 0;          		// Disable the DMAC
    while(DMAC->CTRL.bit.DMAENABLE);			 		// Wait for synchronization
    DMAC->CTRL.bit.SWRST = 1;									// Reset the DMAC
    while (DMAC->CTRL.bit.SWRST);							// Wait for synchronization
// //  The SRAM address of where the descriptor memory section is located must be written to the Description Base
// //  Address (BASEADDR) register
    DMAC->BASEADDR.reg = (uint32_t*)&descriptor_section[0];  //it is the address of the dma descriptor
    DMAC->WRBADDR.reg = (uint32_t*)&wrb[0];

//    DMAC->CTRL.bit.LVLEN0 =1;
//    DMAC->CTRL.bit.LVLEN1 =1;
//    DMAC->CTRL.bit.LVLEN2 =1;
//    DMAC->CTRL.bit.LVLEN3 =1;
    DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xf);

// DMAC->CTRL.bit.SWRST=1;
// while (DMAC->CTRL.bit.SWRST);
// //destination & source

        __enable_irq();
}


void dma_send(uint8_t ch, uint32_t src_addr, uint32_t dst_addr, uint16_t count){
//memory to memory
    descriptor_section[ch].BTCNT.reg = count ;//* sizeof (uint32_t) ;
    descriptor_section[ch].SRCADDR.reg = src_addr + count*1;//*(sizeof (uint32_t)) ;   //STEPSEL = 0    the formula is wrong
    descriptor_section[ch].DSTADDR.reg = dst_addr + count*1;//*(sizeof (uint32_t)) ;   //STEPSEL =0     the formula is wrong
    DMAC->CHID.bit.ID=DMAC_CHID_ID(ch);

    //DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK;
    DMAC->CHCTRLA.bit.ENABLE=1;

    //hri_dmac_write_CHCTRLA_ENABLE_bit(DMAC,0,1);        //ch enable
   // //DMAC->SWTRIGCTRL.reg |= (1 << 0);
   // hri_dmac_write_SWTRIGCTRL_SWTRIG0_bit(DMAC,1);  //trig channel 0
    DMAC->SWTRIGCTRL.reg |= DMAC_SWTRIGCTRL_SWTRIG(ch);
}


