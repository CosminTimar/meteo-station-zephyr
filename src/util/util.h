#ifndef UTIL_HEADER
#define UTIL_HEADER


typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;


typedef enum{
	I2C_NO_ERROR = 0,
	I2C_DEVICE_NOT_READY_ERROR,
	I2C_READ_WRITE_ERROR,
	I2C_CHIP_ID_INVALID,
}i2c_error;


i2c_error config_i2c_driver(struct i2c_dt_spec dev_i2c);

i2c_error i2c_read_sensor_id(uint8_t* chipIdArray, uint8_t chipId,const struct i2c_dt_spec* dev_i2c);

i2c_error i2c_sensor_config(uint8_t write_reg, uint8_t config_value,const struct i2c_dt_spec* dev_i2c);

i2c_error i2c_burst_read_register(uint8* received_data, uint8 data_lenght,const struct i2c_dt_spec* dev_i2c, uint8 reg);

#endif