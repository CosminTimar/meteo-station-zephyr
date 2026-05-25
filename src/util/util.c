#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>

#include "util.h"

static callback_ptr util_cb_vector[NUMBER_OF_SENSORS] = {NULL};

static uint8_t number_of_callbacks = 0;


i2c_error config_i2c_driver(struct i2c_dt_spec dev_i2c)
{
	if (!device_is_ready(dev_i2c.bus))
	{
	#if IS_ENABLED(CONFIG_PRINTK)
		printk("I2C bus %s is not ready!\n\r",dev_i2c.bus->name);
	#endif
		return I2C_DEVICE_NOT_READY_ERROR;
	}
	return I2C_NO_ERROR;
}

i2c_error i2c_read_sensor_id(uint8_t* chipIdArray, uint8_t chipId,const struct i2c_dt_spec* dev_i2c)
{
	uint8_t id = 0xFF;
	int error = i2c_write_read_dt(dev_i2c, chipIdArray, 1, &id, 1);

	if (I2C_NO_ERROR != error) 
	{
	#if IS_ENABLED(CONFIG_PRINTK)
		printk("Failed to read register %x \n", chipIdArray[0]);
	#endif
		return I2C_READ_WRITE_ERROR;
	}

	if (id != chipId) 
	{
	#if IS_ENABLED(CONFIG_PRINTK)
		printk("Invalid chip id! %x \n", id);
	#endif
		return I2C_CHIP_ID_INVALID;
	}
	
	return I2C_NO_ERROR;
}

i2c_error i2c_sensor_config(uint8_t write_reg, uint8_t config_value,const struct i2c_dt_spec* dev_i2c)
{
	
    uint8_t sensor_config[] = {write_reg, config_value};

	int error = i2c_write_dt(dev_i2c, sensor_config, 2);

	if (I2C_NO_ERROR != error) 
	{
	#if IS_ENABLED(CONFIG_PRINTK)
		printk("Failed to write register %x \n", sensor_config[0]);
	#endif
		return I2C_READ_WRITE_ERROR;
	}
	return I2C_NO_ERROR;
}

i2c_error i2c_burst_read_register(uint8_t* received_data, uint8_t data_lenght,const struct i2c_dt_spec* dev_i2c, uint8_t reg)
{

	int error = i2c_burst_read_dt(dev_i2c, reg, received_data, data_lenght);

	if (I2C_NO_ERROR != error) 
	{
	#if IS_ENABLED(CONFIG_PRINTK)
		printk("Failed to read register %c \n", reg);
	#endif
		return I2C_READ_WRITE_ERROR;
	}
	return I2C_NO_ERROR;
	
}

void util_register_cb(callback_ptr cb)
{
	if( NULL == cb)
	{
		return;
	}
	util_cb_vector[number_of_callbacks] = cb;
	number_of_callbacks++;
}

uint8_t util_get_number_of_callbacks()
{
	return number_of_callbacks;
}

void util_get_cb_vector(callback_ptr** cb_vector)
{
	*cb_vector = util_cb_vector;
}

void util_float_to_uint8(float float_val, uint8_t* data)
{
	union float_convert_union float_union;
	float_union.val = float_val;
	for(uint8_t index=0; index < (sizeof(float)); index++)
	{
		data[index] = float_union.bytes[index];	
	}
}