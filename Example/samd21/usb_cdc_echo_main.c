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

#define _GCLK_INIT_1ST (1 << 0 | 1 << 1)

/* Not referenced GCLKs, initialized last */
#define _GCLK_INIT_LAST (~_GCLK_INIT_1ST)


#define CONF_CPU_FREQUENCY 48000000
void board_init(void)
{
  // Clock init ( follow hpl_init.c )
  hri_nvmctrl_set_CTRLB_RWS_bf(NVMCTRL, 2);

  _pm_init();
  _sysctrl_init_sources();

#if _GCLK_INIT_1ST
  _gclk_init_generators_by_fref(_GCLK_INIT_1ST);
#endif
  _sysctrl_init_referenced_generators();
  _gclk_init_generators_by_fref(_GCLK_INIT_LAST);

  // Update SystemCoreClock since it is hard coded with asf4 and not correct
  // Init 1ms tick timer (samd SystemCoreClock may not correct)
  SystemCoreClock = CONF_CPU_FREQUENCY;
  SysTick_Config(CONF_CPU_FREQUENCY / 1000);

  // Led init


  /* USB Clock init
   * The USB module requires a GCLK_USB of 48 MHz ~ 0.25% clock
   * for low speed and full speed operation. */
  PM->APBBMASK.bit.USB_ = 1;
    PM->AHBMASK.bit.USB_=1;


    GCLK_CLKCTRL_Type clkctrl = {0};
    uint16_t temp;
    GCLK->CLKCTRL.bit.ID =  USB_GCLK_ID;
    temp = GCLK->CLKCTRL.reg;
    clkctrl.bit.CLKEN = 1;
    clkctrl.bit.WRTLOCK = 0;
    clkctrl.bit.GEN = GCLK_CLKCTRL_GEN_GCLK6_Val;     // usb 48MHz
    GCLK->CLKCTRL.reg = (clkctrl.reg | temp);

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
//  _pm_enable_bus_clock(PM_BUS_APBC, TCC0);
//  TCC0->PER.bit.PER = 48000000 / 1000;
//  TCC0->CC[3].bit.CC = 48000000 / 2000;
//  TCC0->CTRLA.bit.ENABLE = true;

//  gpio_set_pin_function(PIN_PA19, PINMUX_PA19F_TCC0_WO3);
//  _gclk_enable_channel(TCC0_GCLK_ID, GCLK_CLKCTRL_GEN_GCLK0_Val);

}







int main(void)
{
    atmel_start_init();

//   board_init();
//    _pm_init();
//    clock_source_init();

    TARGET_USB_init();
    usb_init();

    cdcd_acm_example();






}
