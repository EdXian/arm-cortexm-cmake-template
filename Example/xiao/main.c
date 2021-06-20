#include "stdio.h"
#include "sam.h"
#include "FreeRTOS.h"
#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include "task.h"

#include "uart.h"
#include "spi.h"
#include "clock.h"
#include "adc.h"
#include "pinconfig.h"
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


void dma_enable(){
  DMAC->CTRL.bit.DMAENABLE=1;
  DMAC->CTRL.bit.CRCENABLE = 0;

  DMAC->BASEADDR.reg = 0x20000000;
  DMAC->WRBADDR.reg = 0x20001100;




}

// SERCOM1 pa18 pad[2], pa19 pad[3]


//void vApplicationTickHook( void ){

//}

//void vApplicationIdleHook(void){

//}

void Systick_init(){
    SysTick->CTRL = 0;
    SysTick->LOAD = 48000-1;
    NVIC_SetPriority(SysTick_IRQn, 3);
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Pos | SysTick_CTRL_TICKINT_Pos | SysTick_CTRL_ENABLE_Pos;
    NVIC_EnableIRQ(SysTick_IRQn);
    SystemCoreClock = 48000000;
}

void led_task(void *p){
    uint16_t adc_val;
    while(1){
        togle_pin();
        uart_write_byte(SERCOM4,'a');
        asm("nop");
        uart_write_byte(SERCOM4,'b');
        asm("nop");
        uart_write_byte(SERCOM4,'c');
        asm("nop");
        adc_val = adc_read();
        vTaskDelay(200);
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

int main(){


  Systick_init();
  NVIC_EnableIRQ(SERCOM4_IRQn);
  NVIC_SetPriority(SERCOM4_IRQn,4);

  clock_source_init();

  //spi_init();
  //spi_set_baudrate(500);
  gpio_init();

  adc_clok_init();

  SEGGER_RTT_Init();



  //PB08 UART_Tx SCOM4PAD0
  //PB09 UART_Rx SCOM4PAD1
  uart_clock_init(SERCOM4);
  uart_basic_init(SERCOM4,50436,
                  UART_RX_PAD1_TX_PAD0
                  );
    //gpio_configure_pin


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

    }
}
