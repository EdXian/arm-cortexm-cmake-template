#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include "sam.h"
#include "FreeRTOS.h"
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "task.h"
#include "dma.h"
#include "uart.h"
#include "spi.h"
#include "clock.h"
#include "adc.h"
#include "pinconfig.h"

#include "tusb_config.h"
#include "tusb.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
void vAssertCalled(uint8_t* file, uint8_t*  line ){

}

const uint8_t test_data[10]={0};
//drive GPIO PA17
//drive usb

void togle_pin(void);
void togle_pin(void){

    REG_PORT_OUTTGL0 = PORT_PA17;
    REG_PORT_OUTTGL1 = PORT_PB08;
}


volatile uint32_t count =0;

#define USBD_STACK_SIZE     (3*configMINIMAL_STACK_SIZE)


void gpio_init(){
    REG_PORT_DIRSET0  |=  PORT_PA17;
    REG_PORT_OUTSET0 |=  PORT_PA17;
}

//enum  {
//  BLINK_NOT_MOUNTED = 250,
//  BLINK_MOUNTED = 1000,
//  BLINK_SUSPENDED = 2500,
//};


void USB_Handler(){


    tud_int_handler(0);
}

void Sys_init(){
    SysTick->CTRL = 0;
    SysTick->LOAD = 48000-1;

    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Pos | SysTick_CTRL_TICKINT_Pos | SysTick_CTRL_ENABLE_Pos;

    SystemCoreClock = 48000000;

    NVIC_SetPriority(SysTick_IRQn, 3);
    NVIC_EnableIRQ(SysTick_IRQn);
}


void SERCOM4_Handler(void){

   	if (SERCOM4->USART.INTFLAG.bit.RXC)
	{
		// Got a character
        uint16_t rxData = SERCOM4->USART.DATA.reg;
		return;
	}

}
TaskHandle_t xHandle = NULL;


static inline void pin_set_peripheral_function(uint32_t pinmux)
{
    uint8_t port = (uint8_t)((pinmux >> 16)/32);
    PORT->Group[port].PINCFG[((pinmux >> 16) - (port*32))].bit.PMUXEN = 1;
    PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg &= ~(0xF << (4 * ((pinmux >>
    16) & 0x01u)));
    PORT->Group[port].PMUX[((pinmux >> 16) - (port*32))/2].reg |= (uint8_t)((pinmux &
    0x0000FFFF) << (4 * ((pinmux >> 16) & 0x01u)));
}

static inline void pin_set_pull_up(uint32_t pinmux,uint8_t set)
{
    uint8_t port = (uint8_t)((pinmux >> 16)/32);
    PORT->Group[port].PINCFG[((pinmux >> 16) - (port*32))].bit.PMUXEN = 1;
    PORT->Group[port].PINCFG[((pinmux >> 16) - (port*32))].bit.PULLEN = set;
}
void usb_port_init(){


    REG_PORT_DIRSET0 |= PORT_PA24;
    REG_PORT_OUTSET0 &= ~(PORT_PA24);
    //pin_set_pull_up(PORT_PA24,0);no pull up

    REG_PORT_DIRSET0 |= PORT_PA25;
    REG_PORT_OUTSET0 &= ~(PORT_PA25);
    //pin_set_pull_up(PORT_PA25,0); no pull up

    pin_set_peripheral_function(PINMUX_PA24G_USB_DM);

    pin_set_peripheral_function(PINMUX_PA25G_USB_DP);
}

#define CDC_STACK_SZIE      configMINIMAL_STACK_SIZE
StackType_t  cdc_stack[CDC_STACK_SZIE];
StaticTask_t cdc_taskdef;

StaticTimer_t blinky_tmdef;
TimerHandle_t blinky_tm;

void led_blinky_cb(TimerHandle_t xTimer);
void usb_device_task(void* param);
void cdc_task(void* params);





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


void led_blinky_cb(TimerHandle_t xTimer);
void usb_device_task(void* param);
void cdc_task(void* params);




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
    vTaskDelay(pdMS_TO_TICKS(100));
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

  togle_pin();

}






int main(){


    clock_source_init();
    Sys_init();    // systick init should be later than clocksource init


//    NVIC_EnableIRQ(SERCOM4_IRQn);
//    NVIC_SetPriority(SERCOM4_IRQn,4);
    //spi_init();
    //spi_set_baudrate(500);
    gpio_init();
//    spi_init();
//    adc_clok_init();

    usb_port_init();

    //tusb_init();

     NVIC_SetPriority(USB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
//  SEGGER_RTT_Init();
//    dma_enable();


  //PB08 UART_Tx SCOM4PAD0
  //PB09 UART_Rx SCOM4PAD1
  //uart_clock_init(SERCOM4);
//  uart_basic_init(SERCOM4,50436,
//                  UART_RX_PAD1_TX_PAD0
//                  );
//    //gpio_configure_pin


/* Create the task, storing the handle. */

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
    while(1){

    }
}
