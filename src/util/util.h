#ifndef UTIL_HEADER
#define UTIL_HEADER

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#define I2C_DEVICE_NOT_READY_ERROR	(0xA0)
#define I2C_READ_WRITE_ERROR		(0xA1)
#define I2C_CHIP_ID_INVALID			(0xA2)
#define I2C_NO_ERROR				(0x00)


uint8_t config_i2c_driver(struct i2c_dt_spec dev_i2c);

uint8_t i2c_read_sensor_id(uint8_t* chipIdArray, uint8_t chipId, struct i2c_dt_spec* dev_i2c);

uint8_t i2c_sensor_config(uint8_t write_reg, uint8_t config_value, struct i2c_dt_spec* dev_i2c);

#endif