#include "stm32l4xx.h"
#include "stm32l4xx_hal_conf.h"
#include "stm32l4xx_hal_def.h"
#include "SensorTile.h"
#include "cube_hal.h"



#ifndef __SYS_INIT_H_
#define __SYS_INIT_H_

void BSP_LED_Init(Led_TypeDef Led);
void SystemClock_Config(void);

static void Error_Handler( void );

#endif





