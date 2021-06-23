/**
 * \file
 *
 * \brief Application implement
 *
 * Copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "atmel_start.h"
#include "atmel_start_pins.h"
//#include <hpl_gpio.h>
//#include <hpl_init.h>
//#include <hpl_gclk_base.h>
//#include <hpl_pm_config.h>
//#include <hpl_pm_base.h>

//#include <hpl_dma.h>
//#include <hpl_dmac_config.h>
#define _GCLK_INIT_1ST 0x00000000
/* Not referenced GCLKs, initialized last */
#define _GCLK_INIT_LAST 0x000000FF

void board_init(void)
{
    //hri_nvmctrl_set_CTRLB_RWS_bf(NVMCTRL, 1);
    NVMCTRL->CTRLB.bit.RWS=1;
    _pm_init();
//    PM_CRITICAL_SECTION_ENTER();
//        PM->CPUSEL.bit.CPUDIV =1;
//        PM->APBASEL.bit.APBADIV=1;
//        PM->APBBSEL.bit.APBBDIV=1;
//        PM->APBCSEL.bit.APBCDIV=1;
//    PM_CRITICAL_SECTION_LEAVE();


    _sysctrl_init_sources();
#if _GCLK_INIT_1ST
    _gclk_init_generators_by_fref(_GCLK_INIT_1ST);
#endif
    _sysctrl_init_referenced_generators();
    _gclk_init_generators_by_fref(_GCLK_INIT_LAST);

#if CONF_DMAC_ENABLE
    _pm_enable_bus_clock(PM_BUS_AHB, DMAC);
    _pm_enable_bus_clock(PM_BUS_APBB, DMAC);
    _dma_init();
#endif
}

#define SYSCTRL_FUSES_OSC32K_CAL_ADDR   (NVMCTRL_OTP4 + 4)
#define SYSCTRL_FUSES_OSC32K_CAL_Pos   6
#define 	SYSCTRL_FUSES_OSC32K_ADDR   SYSCTRL_FUSES_OSC32K_CAL_ADDR
#define 	SYSCTRL_FUSES_OSC32K_Pos   SYSCTRL_FUSES_OSC32K_CAL_Pos
#define 	SYSCTRL_FUSES_OSC32K_Msk   (0x7Fu << SYSCTRL_FUSES_OSC32K_Pos)



static void gclk_sync(void) {
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
        ;
}

static void dfll_sync(void) {
    while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) == 0)
        ;
}

#define NVM_SW_CALIB_DFLL48M_COARSE_VAL   58
#define NVM_SW_CALIB_DFLL48M_FINE_VAL     64




void clock_source_init(void){
     NVMCTRL->CTRLB.bit.RWS = 1;

     SYSCTRL->XOSC32K.reg =
           SYSCTRL_XOSC32K_STARTUP(6) | SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K;
       SYSCTRL->XOSC32K.bit.ENABLE = 1;
       while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_XOSC32KRDY) == 0)
           ;

       GCLK->GENDIV.reg = GCLK_GENDIV_ID(1);
       gclk_sync();

       GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(1) | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_GENEN;
       gclk_sync();

       GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0) | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_CLKEN;
       gclk_sync();

       SYSCTRL->DFLLCTRL.bit.ONDEMAND = 0;
       dfll_sync();

       SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP(31) | SYSCTRL_DFLLMUL_FSTEP(511) |
                              SYSCTRL_DFLLMUL_MUL((48000000 / (32 * 1024)));
       dfll_sync();

       SYSCTRL->DFLLCTRL.reg |=
           SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_WAITLOCK | SYSCTRL_DFLLCTRL_QLDIS;
       dfll_sync();

       SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE;

       while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC) == 0 ||
              (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF) == 0)
           ;
       dfll_sync();

       GCLK->GENDIV.reg = GCLK_GENDIV_ID(6);
          gclk_sync();

          // Add GCLK_GENCTRL_OE below to output GCLK0 on the SWCLK pin.
          GCLK->GENCTRL.reg =
              GCLK_GENCTRL_ID(6) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;
          gclk_sync();

}

int main(void)
{
    atmel_start_init();

    board_init();
//    _pm_init();
//    clock_source_init();

    TARGET_USB_init();
    usb_init();

    cdcd_acm_example();






}
