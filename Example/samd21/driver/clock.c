#include "clock.h"
#include "hal_gpio.h"
#include "hal_init.h"
#include "hri_nvmctrl_d21.h"


#include "hpl_pm_config.h"
#include "hpl_pm_base.h"
#include "hpl_pm_config.h"
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
#define CPU_FREQUENCY 48000000

#define _GCLK_INIT_1ST (1 << 0 | 1 << 1)

/* Not referenced GCLKs, initialized last */
#define _GCLK_INIT_LAST (~_GCLK_INIT_1ST)



#define CLK_CONFIG 0
#if CLK_CONFIG ==1
void clock_source_init(void) {

  NVMCTRL->CTRLB.bit.RWS = 1;

  SYSCTRL->XOSC32K.reg =SYSCTRL_XOSC32K_STARTUP(6) | SYSCTRL_XOSC32K_XTALEN | SYSCTRL_XOSC32K_EN32K;

  SYSCTRL->XOSC32K.bit.ENABLE = 1;
  while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_XOSC32KRDY) == 0);


      GCLK->GENDIV.reg = GCLK_GENDIV_ID(1);
      gclk_sync();

      GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(1) | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_GENEN;
      gclk_sync();

      GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0) | GCLK_CLKCTRL_GEN_GCLK1 | GCLK_CLKCTRL_CLKEN;
      gclk_sync();

      SYSCTRL->DFLLCTRL.bit.ONDEMAND = 0;
      dfll_sync();

      SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP(31) | SYSCTRL_DFLLMUL_FSTEP(511) |
                             SYSCTRL_DFLLMUL_MUL((CPU_FREQUENCY / (32 * 1024)));
      dfll_sync();

      SYSCTRL->DFLLCTRL.reg |=
          SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_WAITLOCK | SYSCTRL_DFLLCTRL_QLDIS;
      dfll_sync();

      SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ENABLE;

      while ((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKC) == 0 ||
             (SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLLCKF) == 0)
          ;
      dfll_sync();



      // Configure DFLL48M as source for GCLK_GEN 0
      GCLK->GENDIV.reg = GCLK_GENDIV_ID(0);
      gclk_sync();

      // Add GCLK_GENCTRL_OE below to output GCLK0 on the SWCLK pin.
      GCLK->GENCTRL.reg =
          GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_IDC | GCLK_GENCTRL_GENEN;
      gclk_sync();

      SysTick_Config(1000);

}
#else
void clock_source_init(void){

    uint32_t tempDFLL48CalibrationCoarse;
       NVMCTRL->CTRLB.bit.RWS=1;
      _pm_init();
   //    __disable_irq();
   //    PM->CPUSEL.bit.CPUDIV=1;
   //    PM->APBASEL.bit.APBADIV=1;
   //    PM->APBBSEL.bit.APBBDIV=1;
   //    PM->APBCSEL.bit.APBCDIV=1;


   //   __enable_irq();
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


     _pm_enable_bus_clock(PM_BUS_APBC, TCC0);
     TCC0->PER.bit.PER = 48000000 / 1000;
     TCC0->CC[3].bit.CC = 48000000 / 2000;
     TCC0->CTRLA.bit.ENABLE = true;


} // ClockSysInit48M()
#endif
static void pin_set_peripheral_function(uint32_t pinmux)
{
 uint8_t port = (uint8_t)((pinmux >> 16)/32);
 PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg &= ~(0xF << (4 * ((pinmux >>
16) & 0x01u)));
 PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg |= (uint8_t)((pinmux &
0x0000FFFF) << (4 * ((pinmux >> 16) & 0x01u)));
 PORT->Group[port].PINCFG[((pinmux >> 16) - (port*32))].bit.PMUXEN = 1;
}

void clock_pin_test(){


    PM->APBCMASK.bit.TCC0_ = 1;
    TCC0->PER.bit.PER = 48000000 / 1000;
    TCC0->CC[3].bit.CC = 48000000 / 2000;
    TCC0->CTRLA.bit.ENABLE = 1;
    pin_set_peripheral_function(PINMUX_PA04E_TCC0_WO0);
    //gpio_set_pin_function(PIN_PA04, PINMUX_PA04E_TCC0_WO0);


    //_gclk_enable_channel(TCC0_GCLK_ID, GCLK_CLKCTRL_GEN_GCLK0_Val);
    GCLK_CLKCTRL_Type clkctrl = {0};
    uint16_t temp;
    GCLK->CLKCTRL.bit.ID = TCC0_GCLK_ID;//inst + GCLK_ID_SERCOM0_CORE;
    temp = GCLK->CLKCTRL.reg;
    clkctrl.bit.CLKEN = 1;
    clkctrl.bit.WRTLOCK = 0;
    clkctrl.bit.GEN = GCLK_CLKCTRL_GEN_GCLK0_Val;
    GCLK->CLKCTRL.reg = (clkctrl.reg | temp);


}
