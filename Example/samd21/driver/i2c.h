#include "sam.h"

#ifndef I2C_H
#define I2C_H

#define I2C_CLK_FREQ   8000000  //GCLK_CLKCTRL_GEN_GCLK3_Val
#define I2C_BAUD    50000

void i2c_clock_init();
void i2c_init();
void i2c_write(uint8_t cmd);
uint8_t i2c_read(uint8_t *data, int size);
void i2c_nack();
void i2c_ack();
void i2c_enable();
void i2c_write_stop(void);

uint8_t i2c_write_start(uint8_t data);
#endif // I2C_H
