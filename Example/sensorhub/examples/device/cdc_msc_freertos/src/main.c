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
#include "math.h"
#include "bsp/board.h"
#include "tusb.h"

#include "arm_math.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
SemaphoreHandle_t usb_rx_semaphore;
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

//--------------------------------------------------------------------+
// Main
//--------------------------------------------------------------------+
void USB_Handler(void)
{
  //tud_int_handler(0);
    dcd_int_handler(0);
}

//a = signal.firwin(100,[1.6, 31.5],pass_zero=False,fs=256)

float32_t fir_taps[]={


    -0.00020170992588542015,-0.0006102604818090372,-0.0010038944270775953,-0.0011831110426271175,
    -0.001035157048802475,-0.0006141670770164509,-0.0001583155603573578,-3.253010410796381e-06,
    -0.00040148725881828756,-0.001331255141976387,-0.0024237610785579487,-0.0031031046970776506,
    -0.0029133982374278267,-0.001863918610066117,-0.0005564204396155583,5.752772025129389e-05,
    -0.0007755056134713423,-0.0030525876399422358,-0.005824526638350609,-0.007594588889524958,
    -0.00717832868473774,-0.004555876212797487,-0.0011477231688400457,0.0007950429744862871,
    -0.0005253706572907836,-0.005193983659632256,-0.011171884782909742,-0.015169429272532352,
    -0.014495223486761955,-0.008887135905953483,-0.0011564022166899498,0.003987777844734601,
    0.002534405381154022,-0.006173556660977727,-0.01836679665036295,-0.027318554037222737,
    -0.02692457540679899,-0.01560441212521708,0.0017744203706998457,0.015474282042505947,
    0.015702008577773298,-0.0015169429391628556,-0.030537403188627438,-0.05683458468957305,
    -0.06264223975501425,-0.035282617187437436,0.02569445429463989,0.10626066706005483,
    0.18214232388674761,0.22807068567792313,0.22807068567792313,0.18214232388674761,
    0.10626066706005483,0.02569445429463989,-0.03528261718743743,-0.06264223975501425,
    -0.05683458468957305,-0.030537403188627438,-0.0015169429391628556,0.015702008577773298,
    0.015474282042505946,0.0017744203706998457,-0.015604412125217074,-0.02692457540679899,
    -0.027318554037222734,-0.01836679665036295,-0.006173556660977723,0.002534405381154021,
    0.0039877778447346005,-0.0011564022166899498,-0.00888713590595348,-0.014495223486761953,
    -0.015169429272532349,-0.011171884782909742,-0.005193983659632254,-0.0005253706572907834,
    0.0007950429744862871,-0.001147723168840046,-0.004555876212797484,-0.007178328684737738,
    -0.007594588889524958,-0.005824526638350605,-0.0030525876399422345,-0.0007755056134713423,
    5.752772025129389e-05,-0.0005564204396155576,-0.0018639186100661161,-0.0029133982374278267,
    -0.0031031046970776506,-0.002423761078557947,-0.0013312551419763859,-0.00040148725881828756,
    -3.253010410796381e-06,-0.00015831556035735767,-0.0006141670770164506,-0.001035157048802475,
    -0.001183111042627116,-0.0010038944270775953,-0.0006102604818090367,-0.00020170992588542015

};

float32_t fir_state[103]={0.0f};
arm_fir_instance_f32 s;
uint8_t a,b;
StaticSemaphore_t  xSemaphoreBuffer;
int main(void)
{
  board_init();
  usb_rx_semaphore = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);
  // soft timer for blinky
  blinky_tm = xTimerCreateStatic(NULL, pdMS_TO_TICKS(BLINK_NOT_MOUNTED), true, NULL, led_blinky_cb, &blinky_tmdef);
  xTimerStart(blinky_tm, 0);
  arm_fir_init_f32(&s,100,fir_taps,fir_state,4);
  // Create a task for tinyusb device stack
  (void) xTaskCreateStatic( usb_device_task, "usbd", USBD_STACK_SIZE, NULL, configMAX_PRIORITIES-1, usb_device_stack, &usb_device_taskdef);

  // Create CDC task
  (void) xTaskCreateStatic( cdc_task, "cdc", CDC_STACK_SZIE, NULL, configMAX_PRIORITIES-2, cdc_stack, &cdc_taskdef);
    NVIC_SetPriority(USB_IRQn,5);
  // skip starting scheduler (and return) for ESP32-S2 or ESP32-S3
#if CFG_TUSB_MCU != OPT_MCU_ESP32S2 && CFG_TUSB_MCU != OPT_MCU_ESP32S3
  vTaskStartScheduler();
#endif
    while(1)asm("nop");
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

#pragma pack(push, 1)

typedef struct sample{
    uint8_t head;
    uint8_t len;
    float a;
    float b;
    float c;
    float d;
    uint16_t sum;
}sample_t;
#pragma pack(pop)
//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
int32_t val;
float v;
sample_t sample_data;
sample_t sample_test;
float32_t a_vector[4];
float32_t b_vector[4];
void cdc_task(void* params)
{
  (void) params;

  // RTOS forever loop
    uint8_t buf[64]="test\n";

    uint32_t count=0;

  while ( 1 )
  {
    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    // if ( tud_cdc_connected() )
    // if(xSemaphoreTake( usb_rx_semaphore, 0xffff ) == pdTRUE )
    {
      // There are data available
      if ( tud_cdc_available() )
      {

        // read and echo back
        uint32_t count = tud_cdc_read(buf, sizeof(buf));
        (void) count;
        memcpy(a_vector,&buf[2],4*sizeof (float32_t));
       // sprintf(buf,"test\n");
        // Echo back
        // Note: Skip echo by commenting out write() and write_flush()
        // for throughput test e.g
        //    $ dd if=/dev/zero of=/dev/ttyACM0 count=10000
        //        v=3.15f;
        //          count++;
        //            val =  (int32_t) 1000*cosf(2*3.14*(float)count/20);
        //          //sprintf(buf,"%d\n",val);
        //          tud_cdc_write(buf, strlen(buf));
        //          tud_cdc_write_flush();
        //          vTaskDelay(30);
        memset(&sample_test,0,sizeof (sample_test));

        arm_fir_f32(&s,a_vector,b_vector,4);
        sample_test.head = 0x55;
        sample_test.len = 0x12;
        sample_test.a = b_vector[0];
        sample_test.b = b_vector[1];
        sample_test.c = b_vector[2];
        sample_test.d = b_vector[3];
        tud_cdc_write(&sample_test, sizeof (sample_test));
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
    asm("nop");
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
