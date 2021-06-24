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
//#define CONF_CPU_FREQUENCY 48000000
#include "atmel_start.h"
#include "atmel_start_pins.h"
#include "hal_gpio.h"
#include "hal_init.h"
#include "hri_nvmctrl_d21.h"

#include "hpl_gclk_base.h"
#include "hpl_pm_config.h"
#include "hpl_pm_base.h"
#include "hpl_pm_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

#include"tusb_config.h"



#include "tusb.h"

#define _GCLK_INIT_1ST 0x00000000
/* Not referenced GCLKs, initialized last */
#define _GCLK_INIT_LAST 0x000000FF




__STATIC_INLINE uint32_t Sys_Config(uint32_t ticks)
{
  if ((ticks - 1UL) > SysTick_LOAD_RELOAD_Msk)
  {
    return (1UL);                                                   /* Reload value impossible */
  }

  SysTick->LOAD  = (uint32_t)(ticks - 1UL);                         /* set reload register */
  //NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */


  NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
  SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */
  return (0UL);                                                     /* Function successful */
}


#define GENERIC_CLOCK_GENERATOR_MAIN      (0u)
#define GENERIC_CLOCK_GENERATOR_XOSC32K   (1u)
#define GENERIC_CLOCK_GENERATOR_OSCULP32K (2u) /* Initialized at reset for WDT */
#define GENERIC_CLOCK_GENERATOR_OSC8M     (3u)

#define GENERIC_CLOCK_MULTIPLEXER_DFLL48M (0u)

// Constants for DFLL48M
#define MAIN_CLK_FREQ (48000000u)
#define EXT_32K_CLK_FREQ (32768u)


void board_init(void)
{
    //hri_nvmctrl_set_CTRLB_RWS_bf(NVMCTRL, CONF_NVM_WAIT_STATE);
 uint32_t tempDFLL48CalibrationCoarse;
    NVMCTRL->CTRLB.bit.RWS=1;
   _pm_init();
//    __disable_irq();
//    PM->CPUSEL.bit.CPUDIV=1;
//    PM->APBASEL.bit.APBADIV=1;
//    PM->APBBSEL.bit.APBBDIV=1;
//    PM->APBCSEL.bit.APBCDIV=1;


//   __enable_irq();
//    _sysctrl_init_sources();
//#if _GCLK_INIT_1ST
//    _gclk_init_generators_by_fref(_GCLK_INIT_1ST);
//#endif
//    _sysctrl_init_referenced_generators();
//    _gclk_init_generators_by_fref(_GCLK_INIT_LAST);

//#if CONF_DMAC_ENABLE
//    _pm_enable_bus_clock(PM_BUS_AHB, DMAC);
//    _pm_enable_bus_clock(PM_BUS_APBB, DMAC);
//    _dma_init();
//#endif



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


 //    /* ----------------------------------------------------------------------------------------------
 //    * 7) Modify prescaler value of OSC8M to produce 8MHz output
 //    */

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
    _pm_enable_bus_clock(PM_BUS_APBB, USB);
    _pm_enable_bus_clock(PM_BUS_AHB, USB);
    _gclk_enable_channel(USB_GCLK_ID, GCLK_CLKCTRL_GEN_GCLK0_Val);

  gpio_set_pin_direction(PIN_PA24, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(PIN_PA24, false);
  gpio_set_pin_pull_mode(PIN_PA24, GPIO_PULL_OFF);
  gpio_set_pin_direction(PIN_PA25, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(PIN_PA25, false);
  gpio_set_pin_pull_mode(PIN_PA25, GPIO_PULL_OFF);

  gpio_set_pin_function(PIN_PA24, PINMUX_PA24G_USB_DM);
  gpio_set_pin_function(PIN_PA25, PINMUX_PA25G_USB_DP);

  // Output 500hz PWM on D12 (PA19 - TCC0 WO[3]) so we can validate the GCLK0 clock speed with a Saleae.

  _pm_enable_bus_clock(PM_BUS_APBC, TCC0);
  TCC0->PER.bit.PER = 48000000 / 1000;
  TCC0->CC[3].bit.CC = 48000000 / 2000;
  TCC0->CTRLA.bit.ENABLE = true;

//  gpio_set_pin_function(PIN_PA04, PINMUX_PA04E_TCC0_WO0);
//  _gclk_enable_channel(TCC0_GCLK_ID, GCLK_CLKCTRL_GEN_GCLK0_Val);


}
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

// static timer
StaticTimer_t blinky_tmdef;
TimerHandle_t blinky_tm;

// static task for usbd
// Increase stack size when debug log is enabled
#if CFG_TUSB_DEBUG
  #define USBD_STACK_SIZE     (3*configMINIMAL_STACK_SIZE)
#else
  #define USBD_STACK_SIZE     (3*configMINIMAL_STACK_SIZE/2)
#endif

StackType_t  usb_device_stack[USBD_STACK_SIZE];
StaticTask_t usb_device_taskdef;

// static task for cdc
#define CDC_STACK_SZIE      configMINIMAL_STACK_SIZE
StackType_t  cdc_stack[CDC_STACK_SZIE];
StaticTask_t cdc_taskdef;


void led_blinky_cb(TimerHandle_t xTimer);
void usb_device_task(void* param);
void cdc_task(void* params);





void led_task(){
    cdcd_acm_example();
    while (1) {
    //cdcdf_acm_write("hello\n",6);
    gpio_toggle_pin_level(PIN_PA17);
    vTaskDelay(1000);
    }

}

//void USB_Handler(void)
//{
//  tud_int_handler(0);
//} //redefined in hpl_usb.c



#define CONF_CPU_FREQUENCY 48000000
xTaskHandle task_handle;
int main(void)
{
    //atmel_start_init();
    board_init();
    SystemCoreClock = CONF_CPU_FREQUENCY;
     SysTick_Config(CONF_CPU_FREQUENCY / 1000);
    gpio_set_pin_direction(PIN_PA17, GPIO_DIRECTION_OUT);
    gpio_set_pin_level(PIN_PA17, false);
    NVIC_SetPriority(USB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);

    TARGET_USB_init();
    usb_init();
    xTaskCreate(led_task,"task1",128,NULL,2,task_handle);
#if CFG_TUSB_OS  == OPT_OS_FREERTOS
  // If freeRTOS is used, IRQ priority is limit by max syscall ( smaller is higher )

#endif
//    blinky_tm = xTimerCreateStatic(NULL, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), true, NULL, led_blinky_cb, &blinky_tmdef);
//    xTimerStart(blinky_tm, 0);
    // Create a task for tinyusb device stack
//    (void) xTaskCreateStatic( usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES-1, usb_device_stack, &usb_device_taskdef);
//    // Create CDC task
//    (void) xTaskCreateStatic( cdc_task, "cdc", CDC_STACK_SZIE, NULL, configMAX_PRIORITIES-2, cdc_stack, &cdc_taskdef);


    vTaskStartScheduler();
    while(1){

    }
    //_pm_init();
    //clock_source_init();
    //cdcd_acm_example();
//    TARGET_USB_init();
//    usb_init();

//   cdcd_acm_example();
}
void usb_device_task(void* param)
{
  (void) param;

  // This should be called after scheduler/kernel is started.
  // Otherwise it could cause kernel issue since USB IRQ handler does use RTOS queue API.
  tusb_init();

  // RTOS forever loop
  while (1)
  {
    // tinyusb device task
    tud_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), 0);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_SUSPENDED), 0);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  xTimerChangePeriod(blinky_tm, pdMS_TO_TICKS(BLINK_MOUNTED), 0);
}

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void* params)
{
  (void) params;

  // RTOS forever loop
  while ( 1 )
  {
    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    // if ( tud_cdc_connected() )
    {
      // There are data available
      if ( tud_cdc_available() )
      {
        uint8_t buf[64];

        // read and echo back
        uint32_t count = tud_cdc_read(buf, sizeof(buf));
        (void) count;

        // Echo back
        // Note: Skip echo by commenting out write() and write_flush()
        // for throughput test e.g
        //    $ dd if=/dev/zero of=/dev/ttyACM0 count=10000
        tud_cdc_write(buf, count);
        tud_cdc_write_flush();
      }
    }

    // For ESP32-S2 this delay is essential to allow idle how to run and reset wdt
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  (void) itf;
  (void) rts;

  // TODO set some indicator
  if ( dtr )
  {
    // Terminal connected
  }else
  {
    // Terminal disconnected
  }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
  (void) itf;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinky_cb(TimerHandle_t xTimer)
{
  (void) xTimer;
  static bool led_state = false;

  //board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
