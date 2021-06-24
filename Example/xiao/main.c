/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

#include "hal/include/hal_gpio.h"
#include "hal/include/hal_init.h"
#include "hri/hri_nvmctrl_d21.h"

#include "hpl/gclk/hpl_gclk_base.h"
#include "hpl_pm_config.h"
#include "hpl/pm/hpl_pm_base.h"

#include "tusb.h"


#define LED_PIN               17
#define LED_STATE_ON          0

// Button
#define BUTTON_PIN            9 // PA4 pin D1 on seed input
#define BUTTON_STATE_ACTIVE   0

// UART
#define UART_RX_PIN           4
#define UART_TX_PIN           5
//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
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


void USB_Handler(void)
{
  tud_int_handler(0);
}

//--------------------------------------------------------------------+
// MACRO TYPEDEF CONSTANT ENUM DECLARATION
//--------------------------------------------------------------------+

/* Referenced GCLKs, should be initialized firstly */
#define _GCLK_INIT_1ST (1 << 0 | 1 << 1)

/* Not referenced GCLKs, initialized last */
#define _GCLK_INIT_LAST (~_GCLK_INIT_1ST)


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
    uint32_t tempDFLL48CalibrationCoarse;
  // Clock init ( follow hpl_init.c )
  hri_nvmctrl_set_CTRLB_RWS_bf(NVMCTRL, 2);

  _pm_init();


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





  // Update SystemCoreClock since it is hard coded with asf4 and not correct
  // Init 1ms tick timer (samd SystemCoreClock may not correct)
  SystemCoreClock = CONF_CPU_FREQUENCY;
  SysTick_Config(CONF_CPU_FREQUENCY / 1000);

 

#if CFG_TUSB_OS  == OPT_OS_FREERTOS
  // If freeRTOS is used, IRQ priority is limit by max syscall ( smaller is higher )
  NVIC_SetPriority(USB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
#endif

  /* USB Clock init
   * The USB module requires a GCLK_USB of 48 MHz ~ 0.25% clock
   * for low speed and full speed operation. */
  _pm_enable_bus_clock(PM_BUS_APBB, USB);
  _pm_enable_bus_clock(PM_BUS_AHB, USB);
  _gclk_enable_channel(USB_GCLK_ID, GCLK_CLKCTRL_GEN_GCLK0_Val);

  // USB Pin Init
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

  gpio_set_pin_function(PIN_PA04, PINMUX_PA04E_TCC0_WO0);
  _gclk_enable_channel(TCC0_GCLK_ID, GCLK_CLKCTRL_GEN_GCLK0_Val);
}

//--------------------------------------------------------------------+
// Board porting API
//--------------------------------------------------------------------+

void board_led_write(uint8_t state)
{
  gpio_set_pin_level(LED_PIN, state ? LED_STATE_ON : (1-LED_STATE_ON));
}

uint32_t board_button_read(void)
{
  return BUTTON_STATE_ACTIVE == gpio_get_pin_level(BUTTON_PIN);
}

int board_uart_read(uint8_t* buf, int len)
{
  (void) buf; (void) len;
  return 0;
}

int board_uart_write(void const * buf, int len)
{
  (void) buf; (void) len;
  return 0;
}


void led_blinky_cb(TimerHandle_t xTimer);
void usb_device_task(void* param);
void cdc_task(void* params);

//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+

int main(void)
{
  board_init();

  // soft timer for blinky
  blinky_tm = xTimerCreateStatic(NULL, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), true, NULL, led_blinky_cb, &blinky_tmdef);
  xTimerStart(blinky_tm, 0);

  // Create a task for tinyusb device stack
  (void) xTaskCreateStatic( usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES-1, usb_device_stack, &usb_device_taskdef);

  // Create CDC task
  (void) xTaskCreateStatic( cdc_task, "cdc", CDC_STACK_SZIE, NULL, configMAX_PRIORITIES-2, cdc_stack, &cdc_taskdef);

  // skip starting scheduler (and return) for ESP32-S2 or ESP32-S3
#if CFG_TUSB_MCU != OPT_MCU_ESP32S2 && CFG_TUSB_MCU != OPT_MCU_ESP32S3
  vTaskStartScheduler();
#endif

  return 0;
}

#if CFG_TUSB_MCU == OPT_MCU_ESP32S2 || CFG_TUSB_MCU == OPT_MCU_ESP32S3
void app_main(void)
{
  main();
}
#endif

// USB Device Driver task
// This top level thread process all usb events and invoke callbacks
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

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
