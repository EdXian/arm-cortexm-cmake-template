#include "stdio.h"
#include "sam.h"
#include "FreeRTOS.h"
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "uart.h"
#define GENERIC_CLOCK_GENERATOR_MAIN      (0u)
#define GENERIC_CLOCK_GENERATOR_XOSC32K   (1u)
#define GENERIC_CLOCK_GENERATOR_OSCULP32K (2u) /* Initialized at reset for WDT */
#define GENERIC_CLOCK_GENERATOR_OSC8M     (3u)

#define GENERIC_CLOCK_MULTIPLEXER_DFLL48M (0u)

// Constants for DFLL48M
#define MAIN_CLK_FREQ (48000000u)
#define EXT_32K_CLK_FREQ (32768u)

void vAssertCalled(uint8_t* file, uint8_t*  line ){

}

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


SYSCTRL_XOSC32K_Type sysctrl_xosc32k = {
		.bit.WRTLOCK = 0,		/* XOSC32K configuration is not locked */
		.bit.STARTUP = 0x2,		/* 3 cycle start-up time */
		.bit.ONDEMAND = 0,		/* Osc. is always running when enabled */
		.bit.RUNSTDBY = 0,		/* Osc. is disabled in standby sleep mode */
		.bit.AAMPEN = 0,		/* Disable automatic amplitude control */
		.bit.EN32K = 1,			/* 32kHz output is disabled */
		.bit.EN1K =0,
		.bit.XTALEN = 1			/* Crystal connected to XIN32/XOUT32 */
	};

volatile uint32_t count =0;
void SysTick_Handler(){
  count ++;
  if(count >= 1000)
    {
      togle_pin();
      count =0;
      SEGGER_RTT_printf(0,"test\n");
    }
}


void clock_source_init(void){

	uint32_t tempDFLL48CalibrationCoarse;	/* used to retrieve DFLL48 coarse calibration value from NVM */

	/* ----------------------------------------------------------------------------------------------
	* 1) Set Flash wait states for 48 MHz (per Table 37-40 in data sheet)
	*/

	NVMCTRL->CTRLB.bit.RWS = 1;		/* 1 wait state required @ 3.3V & 48MHz */

	/* ----------------------------------------------------------------------------------------------
	* 2) Enable XOSC32K clock (External on-board 32.768kHz oscillator), will be used as DFLL48M reference.
	*/

	// Configure SYSCTRL->XOSC32K settings
	SYSCTRL_XOSC32K_Type sysctrl_xosc32k = {
		.bit.WRTLOCK = 0,		/* XOSC32K configuration is not locked */
		.bit.STARTUP = 0x2,		/* 3 cycle start-up time */
		.bit.ONDEMAND = 0,		/* Osc. is always running when enabled */
		.bit.RUNSTDBY = 0,		/* Osc. is disabled in standby sleep mode */
		.bit.AAMPEN = 0,		/* Disable automatic amplitude control */
		.bit.EN32K = 1,			/* 32kHz output is disabled */
		.bit.XTALEN = 1			/* Crystal connected to XIN32/XOUT32 */
	};
	// Write these settings
	SYSCTRL->XOSC32K.reg = sysctrl_xosc32k.reg;
	// Enable the Oscillator - Separate step per data sheet recommendation (sec 17.6.3)
	SYSCTRL->XOSC32K.bit.ENABLE = 1;

	// Wait for XOSC32K to stabilize
	while(!SYSCTRL->PCLKSR.bit.XOSC32KRDY);

	/* ----------------------------------------------------------------------------------------------
	* 3) Put XOSC32K as source of Generic Clock Generator 1
	*/

	// Set the Generic Clock Generator 1 output divider to 1
	// Configure GCLK->GENDIV settings
	GCLK_GENDIV_Type gclk1_gendiv = {
		.bit.DIV = 1,								/* Set output division factor = 1 */
		.bit.ID = GENERIC_CLOCK_GENERATOR_XOSC32K	/* Apply division factor to Generator 1 */
	};
	// Write these settings
	GCLK->GENDIV.reg = gclk1_gendiv.reg;

	// Configure Generic Clock Generator 1 with XOSC32K as source
	GCLK_GENCTRL_Type gclk1_genctrl = {
		.bit.RUNSTDBY = 0,		/* Generic Clock Generator is stopped in stdby */
		.bit.DIVSEL =  0,		/* Use GENDIV.DIV value to divide the generator */
		.bit.OE = 0,			/* Disable generator output to GCLK_IO[1] */
		.bit.OOV = 0,			/* GCLK_IO[1] output value when generator is off */
		.bit.IDC = 1,			/* Generator duty cycle is 50/50 */
		.bit.GENEN = 1,			/* Enable the generator */
		.bit.SRC = 0x05,		/* Generator source: XOSC32K output */
		.bit.ID = GENERIC_CLOCK_GENERATOR_XOSC32K			/* Generator ID: 1 */
	};
	// Write these settings
	GCLK->GENCTRL.reg = gclk1_genctrl.reg;
	// GENCTRL is Write-Synchronized...so wait for write to complete
	while(GCLK->STATUS.bit.SYNCBUSY);

	/* ----------------------------------------------------------------------------------------------
	* 4) Put Generic Clock Generator 1 as source for Generic Clock Multiplexer 0 (DFLL48M reference)
	*/

	GCLK_CLKCTRL_Type gclk_clkctrl = {
		.bit.WRTLOCK = 0,		/* Generic Clock is not locked from subsequent writes */
		.bit.CLKEN = 1,			/* Enable the Generic Clock */
		.bit.GEN = GENERIC_CLOCK_GENERATOR_XOSC32K, 	/* Generic Clock Generator 1 is the source */
		.bit.ID = 0x00			/* Generic Clock Multiplexer 0 (DFLL48M Reference) */
	};
	// Write these settings
	GCLK->CLKCTRL.reg = gclk_clkctrl.reg;

	/* ----------------------------------------------------------------------------------------------
	* 5) Enable DFLL48M clock
	*/

	// DFLL Configuration in Closed Loop mode, cf product data sheet chapter
	// 17.6.7.1 - Closed-Loop Operation

	// Enable the DFLL48M in open loop mode. Without this step, attempts to go into closed loop mode at 48 MHz will
	// result in Processor Reset (you'll be at the in the Reset_Handler in startup_samd21.c).
	// PCLKSR.DFLLRDY must be one before writing to the DFLL Control register
	// Note that the DFLLRDY bit represents status of register synchronization - NOT clock stability
	// (see Data Sheet 17.6.14 Synchronization for detail)
	while(!SYSCTRL->PCLKSR.bit.DFLLRDY);
	SYSCTRL->DFLLCTRL.reg = (uint16_t)(SYSCTRL_DFLLCTRL_ENABLE);
	while(!SYSCTRL->PCLKSR.bit.DFLLRDY);

	// Set up the Multiplier, Coarse and Fine steps
	SYSCTRL_DFLLMUL_Type sysctrl_dfllmul = {
		.bit.CSTEP = 31,		/* Coarse step - use half of the max value (63) */
		.bit.FSTEP = 511,		/* Fine step - use half of the max value (1023) */
		.bit.MUL = 1465			/* Multiplier = MAIN_CLK_FREQ (48MHz) / EXT_32K_CLK_FREQ (32768 Hz) */
	};
	// Write these settings
	SYSCTRL->DFLLMUL.reg = sysctrl_dfllmul.reg;
	// Wait for synchronization
	while(!SYSCTRL->PCLKSR.bit.DFLLRDY);

	// To reduce lock time, load factory calibrated values into DFLLVAL (cf. Data Sheet 17.6.7.1)
	// Location of value is defined in Data Sheet Table 10-5. NVM Software Calibration Area Mapping

	// Get factory calibrated value for "DFLL48M COARSE CAL" from NVM Software Calibration Area
	tempDFLL48CalibrationCoarse = *(uint32_t*)FUSES_DFLL48M_COARSE_CAL_ADDR;
	tempDFLL48CalibrationCoarse &= FUSES_DFLL48M_COARSE_CAL_Msk;
	tempDFLL48CalibrationCoarse = tempDFLL48CalibrationCoarse>>FUSES_DFLL48M_COARSE_CAL_Pos;
	// Write the coarse calibration value
	SYSCTRL->DFLLVAL.bit.COARSE = tempDFLL48CalibrationCoarse;
	// Switch DFLL48M to Closed Loop mode and enable WAITLOCK
	while(!SYSCTRL->PCLKSR.bit.DFLLRDY);
	SYSCTRL->DFLLCTRL.reg |= (uint16_t) (SYSCTRL_DFLLCTRL_MODE | SYSCTRL_DFLLCTRL_WAITLOCK);

	/* ----------------------------------------------------------------------------------------------
	* 6) Switch Generic Clock Generator 0 to DFLL48M. CPU will run at 48MHz.
	*/

	// Now that DFLL48M is running, switch CLKGEN0 source to it to run the core at 48 MHz.
	// Enable output of Generic Clock Generator 0 (GCLK_MAIN) to the GCLK_IO[0] GPIO Pin
	GCLK_GENCTRL_Type gclk_genctrl0 = {
		.bit.RUNSTDBY = 0,		/* Generic Clock Generator is stopped in stdby */
		.bit.DIVSEL =  0,		/* Use GENDIV.DIV value to divide the generator */
		.bit.OE = 1,			/* Enable generator output to GCLK_IO[0] */
		.bit.OOV = 0,			/* GCLK_IO[0] output value when generator is off */
		.bit.IDC = 1,			/* Generator duty cycle is 50/50 */
		.bit.GENEN = 1,			/* Enable the generator */
		.bit.SRC = 0x07,		/* Generator source: DFLL48M output */
		.bit.ID = GENERIC_CLOCK_GENERATOR_MAIN			/* Generator ID: 0 */
	};
	GCLK->GENCTRL.reg = gclk_genctrl0.reg;
	// GENCTRL is Write-Synchronized...so wait for write to complete
	while(GCLK->STATUS.bit.SYNCBUSY);

	// Direct the GCLK_IO[0] output to PA28
	PORT_WRCONFIG_Type port0_wrconfig = {
		.bit.HWSEL = 1,			/* Pin# (28) - falls in the upper half of the 32-pin PORT group */
		.bit.WRPINCFG = 1,		/* Update PINCFGy registers for all pins selected */
		.bit.WRPMUX = 1,		/* Update PMUXn registers for all pins selected */
		.bit.PMUX = 7,			/* Peripheral Function H selected (GCLK_IO[0]) */
		.bit.PMUXEN = 1,		/* Enable peripheral Multiplexer */
		.bit.PINMASK = (uint16_t)(1 << (28-16)) /* Select the pin(s) to be configured */
	};
	// Write these settings
	PORT->Group[0].WRCONFIG.reg = port0_wrconfig.reg;

	/* ----------------------------------------------------------------------------------------------
	* 7) Modify prescaler value of OSC8M to produce 8MHz output
	*/

	SYSCTRL->OSC8M.bit.PRESC = 0;		/* Prescale by 1 */
	SYSCTRL->OSC8M.bit.ONDEMAND = 0 ;	/* Oscillator is always on if enabled */

	/* ----------------------------------------------------------------------------------------------
	* 8) Put OSC8M as source for Generic Clock Generator 3
	*/

	// Set the Generic Clock Generator 3 output divider to 1
	// Configure GCLK->GENDIV settings
	GCLK_GENDIV_Type gclk3_gendiv = {
		.bit.DIV = 1,								/* Set output division factor = 1 */
		.bit.ID = GENERIC_CLOCK_GENERATOR_OSC8M		/* Apply division factor to Generator 3 */
	};
	// Write these settings
	GCLK->GENDIV.reg = gclk3_gendiv.reg;

	// Configure Generic Clock Generator 3 with OSC8M as source
	GCLK_GENCTRL_Type gclk3_genctrl = {
		.bit.RUNSTDBY = 0,		/* Generic Clock Generator is stopped in stdby */
		.bit.DIVSEL =  0,		/* Use GENDIV.DIV value to divide the generator */
		.bit.OE = 0,			/* Disable generator output to GCLK_IO[1] */
		.bit.OOV = 0,			/* GCLK_IO[2] output value when generator is off */
		.bit.IDC = 1,			/* Generator duty cycle is 50/50 */
		.bit.GENEN = 1,			/* Enable the generator */
		.bit.SRC = 0x06,		/* Generator source: OSC8M output */
		.bit.ID = GENERIC_CLOCK_GENERATOR_OSC8M			/* Generator ID: 3 */
	};
	// Write these settings
	GCLK->GENCTRL.reg = gclk3_genctrl.reg;
	// GENCTRL is Write-Synchronized...so wait for write to complete
	while(GCLK->STATUS.bit.SYNCBUSY);

	/* ----------------------------------------------------------------------------------------------
	* 9) Set CPU and APBx BUS Clocks to 48MHz
	*/
	PM->CPUSEL.reg  = PM_CPUSEL_CPUDIV_DIV1 ;

	PM->APBASEL.reg = PM_APBASEL_APBADIV_DIV1_Val ;
	PM->APBBSEL.reg = PM_APBBSEL_APBBDIV_DIV1_Val ;
	PM->APBCSEL.reg = PM_APBCSEL_APBCDIV_DIV1_Val ;

    PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1;

    GCLK_CLKCTRL_Type clkctrl = {0};

    uint16_t temp;
    GCLK->CLKCTRL.bit.ID = GCLK_CLKCTRL_ID_SERCOM0_CORE;//inst + GCLK_ID_SERCOM0_CORE;
    temp = GCLK->CLKCTRL.reg;
    clkctrl.bit.CLKEN = 1;
    clkctrl.bit.WRTLOCK = 0;
    clkctrl.bit.GEN = GCLK_CLKCTRL_GEN_GCLK0_Val;
    GCLK->CLKCTRL.reg = (clkctrl.reg | temp);

//    PM->AHBMASK.bit.DMAC_=1;
//    PM->AHBMASK.bit.NVMCTRL_=1;
//    PM->AHBMASK.bit.USB_=1;
//    PM->APBAMASK.bit.GCLK_=1;
//    PM->APBAMASK.bit.SYSCTRL_=1;





	//PM->APBBMASK.bit.DMAC_= PM_CPUSEL_CPUDIV_DIV1;

} // ClockSysInit48M()


void dma_enable(){
  DMAC->CTRL.bit.DMAENABLE=1;
  DMAC->CTRL.bit.CRCENABLE = 0;

  DMAC->BASEADDR.reg = 0x20000000;
  DMAC->WRBADDR.reg = 0x20001100;




}

// SERCOM1 pa18 pad[2], pa19 pad[3]
void uart_init(){

    //gpio init

    SERCOM1->USART.CTRLA.bit.ENABLE = 0;
    while(SERCOM1->USART.CTRLA.bit.ENABLE != 0);
    SERCOM1->USART.CTRLA.bit.SWRST = 1;
    while(SERCOM1->USART.CTRLA.bit.ENABLE != 0);


    while (SERCOM1->USART.SYNCBUSY.bit.ENABLE) ;

    /* Disable the SERCOM UART module */
    SERCOM1->USART.CTRLA.bit.ENABLE = 0;
    /* Wait for synchronization */
     while (SERCOM1->USART.SYNCBUSY.bit.SWRST) ;

    /* Perform a software reset */
    SERCOM1->USART.CTRLA.bit.SWRST = 1;
    /* Wait for synchronization */
    while (SERCOM1->USART.CTRLA.bit.SWRST) ;

      /* Wait for synchronization */
    while (SERCOM1->USART.SYNCBUSY.bit.SWRST || SERCOM1->USART.SYNCBUSY.bit.ENABLE)
          ;
      /* Update the UART pad settings, mode and data order settings */
    SERCOM1->USART.CTRLA.reg =  SERCOM_USART_CTRLA_MODE(1) | SERCOM_USART_CTRLA_DORD;
      /* Wait for synchronization */
    while (SERCOM1->USART.SYNCBUSY.bit.CTRLB);

      /* Enable transmit and receive and set data size to 8 bits */
    SERCOM1->USART.CTRLB.reg = SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN | SERCOM_USART_CTRLB_CHSIZE(0);

    /* Load the baud value */
    SERCOM1->USART.BAUD.reg = 0x5555;
    /* Wait for synchronization */
    while (SERCOM1->USART.SYNCBUSY.bit.ENABLE) ;

    /* Enable SERCOM UART */
    SERCOM1->USART.CTRLA.bit.ENABLE = 1;


}


int main(){


  SysTick->CTRL = 0;
  SysTick->LOAD = 47999UL;
  NVIC_SetPriority(SysTick_IRQn, 3);
  SysTick->VAL = 0;
  SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Pos | SysTick_CTRL_TICKINT_Pos | SysTick_CTRL_ENABLE_Pos;
  NVIC_EnableIRQ(SysTick_IRQn);
  clock_source_init();

  uart_get_sercom_index(SERCOM1);

  SystemCoreClock = 48000000;
  SEGGER_RTT_Init();

  REG_PORT_DIRSET0  =  PORT_PA17;
  REG_PORT_OUTSET0 =  PORT_PA17;

  uart_basic_init(SERCOM1,9600,0);


    while(1){
        for (unsigned i = 0; i < (SystemCoreClock/100 ); i++)
        {
            asm("nop");
        }

    }
}
