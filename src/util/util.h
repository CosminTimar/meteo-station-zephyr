#ifndef UTIL_HEADER
#define UTIL_HEADER

#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>


typedef unsigned char 		uint8;
typedef unsigned short 		uint16;
typedef unsigned int 		uint32;

#define UTIL_DEFAULT_VALUE      		(0xFFU)
#define UTIL_SHIFT_EIGHT                (0X08U)

#define PRINT_DATA 						(1U)

typedef uint8_t (* callback_ptr)(uint8_t*);


typedef enum{
	I2C_NO_ERROR = 0,
	I2C_DEVICE_NOT_READY_ERROR,
	I2C_READ_WRITE_ERROR,
	I2C_CHIP_ID_INVALID,
}i2c_error;

typedef enum
{
    ADC_NO_ERROR = 0,
    ADC_INTERNAL_ERROR,
	ADC_CONVERSION_IN_PROGRESS = 16,
}adc_error;

union float_convert_union
{
	float val;
	uint8_t bytes[sizeof(float)];
};


i2c_error config_i2c_driver(struct i2c_dt_spec dev_i2c);

i2c_error i2c_read_sensor_id(uint8_t* chipIdArray, uint8_t chipId,const struct i2c_dt_spec* dev_i2c);

i2c_error i2c_sensor_config(uint8_t write_reg, uint8_t config_value,const struct i2c_dt_spec* dev_i2c);

i2c_error i2c_burst_read_register(uint8* received_data, uint8 data_lenght,const struct i2c_dt_spec* dev_i2c, uint8 reg);

void util_register_cb(callback_ptr cb);

void util_get_cb_vector(callback_ptr** cb_vector);

void util_float_to_uint8(float float_val, uint8_t* data);

uint8_t util_get_number_of_callbacks(void);

#endif