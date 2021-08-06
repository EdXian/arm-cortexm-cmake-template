#include "board.h"

#include"tusb_config.h"
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

#include "tusb.h"
#include "mavlink.h"
#include "uart.h"
#include "spi.h"
#include "i2c.h"
#include "flash.h"
mavlink_heartbeat_t pack;
mavlink_message_t msg;

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

void USB_Handler(void)
{
     gpio_toggle_pin_level(PIN_PA17);
  tud_int_handler(0);
} //redefined in hpl_usb.c



#define CONF_CPU_FREQUENCY 48000000

volatile uint32_t system_ticks = 0;
uint16_t byte_num = 0;
uint8_t a,b,c;
uint8_t data;
void SysTick_Handler (void)
{
  system_ticks++;
  if(system_ticks %1000 ==0){

      gpio_toggle_pin_level(GPIO(GPIO_PORTA, 17));
//      pack.custom_mode=0x55;
//      pack.mavlink_version=0xaa;
//      pack.base_mode = 0x63;
//      pack.type = 0x07;
//      byte_num = mavlink_msg_heartbeat_encode(1,1,&msg,&pack);
//      gpio_set_pin_level(PORT_PA10,0);
//      a = spiSend(0xAA);
//      b = spiSend(0x00);
//      c = spiSend(0x00);
//      c = spiSend(0x00);
//      gpio_set_pin_level(PORT_PA10,1);
//      tud_cdc_write(&msg, byte_num);
//      tud_cdc_write_flush();
        //01101001
    //uint8_t address = 0b110100;   //read 1 write 0
      i2c_enable();
      SERCOM2->I2CM.ADDR.bit.ADDR = 0x69;
      while(SERCOM2->I2CM.SYNCBUSY.bit.SYSOP);



      i2c_write(0x0f);

      data = SERCOM2->I2CM.DATA.reg;
      while (0 == (SERCOM2->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB));

    SERCOM2->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
    SERCOM2->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
      i2c_enable();



    asm("nop");
//    while(SERCOM2->I2CM.STATUS.reg &SERCOM_I2CM_INTFLAG_MB);

//    i2c_write(0x0f);
//    while(SERCOM2->I2CM.STATUS.reg &SERCOM_I2CM_INTFLAG_MB);
//    uint8_t data;
//    data = i2c_read();
//    SERCOM2->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
    //i2c_nack();
  }
}

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void cdc_task(void);




/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  tusb_init();
  SysTick_Config(48000);
  NVIC_EnableIRQ(USB_IRQn);
  NVIC_SetPriority(USB_IRQn,3);
  gpio_set_pin_direction(PIN_PA17,GPIO_DIRECTION_OUT);
  gpio_set_pin_pull_mode(PIN_PA17,GPIO_PULL_UP);

    i2c_clock_init();
    i2c_enable();

//    asm("nop");

//    spi_clock_init();
//    spi_init();
//    gpio_set_pin_level(PORT_PA10,1);
//    uart_clock_init(SERCOM4);
//    uart_basic_init(SERCOM4,50834,UART_RX_PAD1_TX_PAD0);
    //PA05
    //PA06
    //PA07
//    uint32_t test = 0xffffffff;
//    flash_write((uint32_t*)0x8000,&test,1);



  while (1)
  {

    tud_task(); // tinyusb device task
    led_blinking_task();

    cdc_task();
//    uart_write_byte(SERCOM4,0xaa);

//    delay_ms(20);
  }

  return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}


//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
void cdc_task(void)
{
  // connected() check for DTR bit
  // Most but not all terminal client set this when making connection
  // if ( tud_cdc_connected() )
  {
    // connected and there are data available
    //if ( tud_cdc_available() )
    {
      // read datas
//      char buf[64];
//      uint32_t count = tud_cdc_read(buf, sizeof(buf));
//      (void) count;

      // Echo back
      // Note: Skip echo by commenting out write() and write_flush()
      // for throughput test e.g
      //    $ dd if=/dev/zero of=/dev/ttyACM0 count=10000


//         delay_ms(10);
    }



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
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  //Blink every interval ms
  //if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  //board_led_write(led_state);
  //led_state = 1 - led_state; // toggle
}