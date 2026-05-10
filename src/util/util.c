#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#include "util.h"


i2c_error config_i2c_driver(struct i2c_dt_spec dev_i2c)
{
	if (!device_is_ready(dev_i2c.bus)) {
		printk("I2C bus %s is not ready!\n\r",dev_i2c.bus->name);
		return I2C_DEVICE_NOT_READY_ERROR;
	}
	return I2C_NO_ERROR;
}

i2c_error i2c_read_sensor_id(uint8_t* chipIdArray, uint8_t chipId,const struct i2c_dt_spec* dev_i2c)
{
	uint8_t id = 0xFF;
	int error = i2c_write_read_dt(dev_i2c, chipIdArray, 1, &id, 1);

	if (I2C_NO_ERROR != error) {
		printk("Failed to read register %x \n", chipIdArray[0]);
		return I2C_READ_WRITE_ERROR;
	}

	if (id != chipId) {
		printk("Invalid chip id! %x \n", id);
		return I2C_CHIP_ID_INVALID;
	}
	
	return I2C_NO_ERROR;
}

i2c_error i2c_sensor_config(uint8_t write_reg, uint8_t config_value,const struct i2c_dt_spec* dev_i2c)
{
	
    uint8_t sensor_config[] = {write_reg, config_value};

	int error = i2c_write_dt(dev_i2c, sensor_config, 2);

	if (I2C_NO_ERROR != error) {
		printk("Failed to write register %x \n", sensor_config[0]);
		return I2C_READ_WRITE_ERROR;
	}
	return I2C_NO_ERROR;
}

i2c_error i2c_burst_read_register(uint8* received_data, uint8 data_lenght,const struct i2c_dt_spec* dev_i2c, uint8 reg)
{

	int error = i2c_burst_read_dt(dev_i2c, reg, received_data, data_lenght);

	if (I2C_NO_ERROR != error) 
	{
		printk("Failed to read register %c \n", reg);
		return I2C_READ_WRITE_ERROR;
	}
	return I2C_NO_ERROR;
	
}