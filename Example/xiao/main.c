#include "stdio.h"
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
#include "spi.h"
#include "tusb_config.h"
#include "tusb.h"

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



void gpio_init(){
    REG_PORT_DIRSET0  |=  PORT_PA17;
    REG_PORT_OUTSET0 |=  PORT_PA17;

//    REG_PORT_DIRSET1 |= PORT_PB08;
//    REG_PORT_OUTSET1 |= PORT_PB08;

//    REG_PORT_DIRSET1 |= PORT_PB09;
//    REG_PORT_OUTSET1 |= PORT_PB09;




}




void USB_Handler(){


    tud_int_handler(0);
}

// SERCOM1 pa18 pad[2], pa19 pad[3]


//void vApplicationTickHook( void ){

//}

//void vApplicationIdleHook(void){

//}

void Sys_init(){
    SysTick->CTRL = 0;
    SysTick->LOAD = 48000-1;

    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Pos | SysTick_CTRL_TICKINT_Pos | SysTick_CTRL_ENABLE_Pos;

    SystemCoreClock = 48000000;

    NVIC_SetPriority(SysTick_IRQn, 3);
    NVIC_EnableIRQ(SysTick_IRQn);
}

void led_task(void *p){
    uint16_t adc_val;
    while(1){
        togle_pin();
//        uart_write_byte(SERCOM4,'a');
//        asm("nop");
//        uart_write_byte(SERCOM4,'b');
//        asm("nop");
//        uart_write_byte(SERCOM4,'c');
//        asm("nop");
//        spiSend(0x66);
//        adc_val = adc_read();
        vTaskDelay(100);
    }
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


void usb_port_init(){
 //   REG_PORT_DIRSET0  |=  PORT_PA17;
//    REG_PORT_OUTSET0 |=  PORT_PA17;

    #define DM_PIN PIN_PA24G_USB_DM
    #define DM_MUX MUX_PA24G_USB_DM
    #define DP_PIN PIN_PA25G_USB_DP
    #define DP_MUX MUX_PA25G_USB_DP

    REG_PORT_DIRSET0 |= PORT_PA24;
    REG_PORT_OUTSET0 &= ~(PORT_PA24);

    REG_PORT_DIRSET0 |= PORT_PA25;
    REG_PORT_OUTSET0 &= ~(PORT_PA25);
/*
  gpio_set_pin_direction(PIN_PA24, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(PIN_PA24, false);
  gpio_set_pin_pull_mode(PIN_PA24, GPIO_PULL_OFF);
  gpio_set_pin_direction(PIN_PA25, GPIO_DIRECTION_OUT);
  gpio_set_pin_level(PIN_PA25, false);
  gpio_set_pin_pull_mode(PIN_PA25, GPIO_PULL_OFF);

  gpio_set_pin_function(PIN_PA24, PINMUX_PA24G_USB_DM);
  gpio_set_pin_function(PIN_PA25, PINMUX_PA25G_USB_DP);
*/


    pin_set_peripheral_function(PINMUX_PA24G_USB_DM);

    pin_set_peripheral_function(PINMUX_PA25G_USB_DP);
}


int main(){


    clock_source_init();
    Sys_init();    // systick init should be later than clocksource init


//    NVIC_EnableIRQ(SERCOM4_IRQn);
//    NVIC_SetPriority(SERCOM4_IRQn,4);
    spi_init();
    //spi_set_baudrate(500);
    gpio_init();
//    spi_init();
//    adc_clok_init();
    tusb_init();
    NVIC_EnableIRQ(USB_IRQn);
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

 xTaskCreate(
          led_task,       /* Function that implements the task. */
          "led_task",          /* Text name for the task. */
          128,      /* Stack size in words, not bytes. */
          ( void * ) 1,    /* Parameter passed into the task. */
          tskIDLE_PRIORITY,/* Priority at which the task is created. */
          &xHandle );      /* Used to pass out the created task's handle. */


    vTaskStartScheduler();
    while(1){
        for(int i=0;i<1000000;i++){
            asm("nop");
        }
        togle_pin();
    }
}
