//#include "stdio.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_def.h"
#include "SensorTile.h"
#include "cube_hal.h"
#include "sys_init.h"
#include "SensorTile.h"


// #include "usbd_desc.h"
// #include "usbd_cdc.h"
// #include "usbd_cdc_interface.h"
// USBD_HandleTypeDef  USBD_Device;
void task_led(void);
void task_led(void){
    BSP_LED_Init(0);
    while(1){
      HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_12);
      vTaskDelay(1000);


    }

}

BaseType_t xReturned;
TaskHandle_t xHandle = NULL;

int main(){

  //
  HAL_Init();
    SystemClock_Config();
SystemCoreClockUpdate();
    BSP_LED_Init(0);
  HAL_PWREx_EnableVddUSB();
  HAL_PWREx_EnableVddIO2();



  xReturned = xTaskCreate(
                     task_led,       /* Function that implements the task. */
                     "NAME",          /* Text name for the task. */
                     256,      /* Stack size in words, not bytes. */
                     ( void * )NULL,    /* Parameter passed into the task. */
                     1,/* Priority at which the task is created. */
                     &xHandle );      /* Used to pass out the created task's handle. */

  if(xReturned = NULL){
    HAL_GPIO_WritePin( GPIOG,GPIO_PIN_12,0);
  }
  // /*** USB CDC Configuration ***/
  // /* Init Device Library */
  // USBD_Init(&USBD_Device, &VCP_Desc, 0);
  // /* Add Supported Class */
  // USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);
  // /* Add Interface callbacks for AUDIO and CDC Class */
  // USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);
  // /* Start Device Process */
  // USBD_Start(&USBD_Device);


    vTaskStartScheduler();
    while(1){
        HAL_GPIO_TogglePin(GPIOG,GPIO_PIN_12);
        HAL_Delay(100);
    }

}
