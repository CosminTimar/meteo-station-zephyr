#ifndef UTIL_HEADER
#define UTIL_HEADER

#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2c.h>

#define UTIL_DEFAULT_VALUE      		(0xFFU)
#define UTIL_SHIFT_EIGHT                (0X08U)

#define UTIL_BLE_ERRORS					(0x01U)

typedef uint8_t (* callback_ptr)(uint8_t*);
typedef void    (* work_callback_ptr)(void);


typedef enum
{
    I2C_NO_ERROR = 0,
    I2C_READ_WRITE_ERROR,
    I2C_DEVICE_NOT_READY_ERROR,
    I2C_CHIP_ID_INVALID,
    ADC_NO_ERROR = 0,
    ADC_INTERNAL_ERROR,
    ADC_CONVERSION_IN_PROGRESS,
    BLE_NO_ERROR = 0,
    BLE_ENABLE_ERROR,
    BLE_ADV_CREATE_ERROR,
    BLE_SET_DATA_ERROR,
    BLE_ADV_START_ERROR,

}util_modules_error;

union float_convert_union
{
	float val;
	uint8_t bytes[sizeof(float)];
};


util_modules_error config_i2c_driver(struct i2c_dt_spec dev_i2c);

util_modules_error i2c_read_sensor_id(uint8_t* chipIdArray, uint8_t chipId,const struct i2c_dt_spec* dev_i2c);

util_modules_error i2c_sensor_config(uint8_t write_reg, uint8_t config_value,const struct i2c_dt_spec* dev_i2c);

util_modules_error i2c_burst_read_register(uint8_t* received_data, uint8_t data_lenght,const struct i2c_dt_spec* dev_i2c, uint8_t reg);

void util_register_cb(callback_ptr cb);

void util_get_cb_vector(callback_ptr** cb_vector);

void util_float_to_uint8(float float_val, uint8_t* data);

uint8_t util_get_number_of_callbacks(void);

inline bool util_get_init_error(uint8_t error){return (error & 0x01U ? false : true);}

#endif