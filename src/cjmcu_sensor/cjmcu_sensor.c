#include <zephyr/device.h>

#include "cjmcu_sensor.h"
#include "thread_worker.h"
#include "uart_report.h"

#define I2C_CJMCU_SENSOR   DT_NODELABEL(cjmcu_sensor)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_CJMCU_SENSOR);

static struct measurement_resut fine_data;

static uint8_t standardize_data(uint8_t* env_data);

static void get_status(void);

static void config_1s_reading(void);

static void get_status()
{
    i2c_error error = I2C_NO_ERROR;

    uint8_t status[] = {STATUS_REG_R};
    uint8_t res_status = UTIL_DEFAULT_VALUE;

   error = i2c_write_read_dt(&dev_i2c, status, CJMCU_STATUS_RW_LENGHT, &res_status, CJMCU_STATUS_RW_LENGHT);

    if(I2C_NO_ERROR != error)
    {
        uart_report_add_error(error);
        return;
    }
}

static void config_1s_reading()
{
    i2c_error error = I2C_NO_ERROR;

    error = i2c_sensor_config(MEAS_MODE_RW,CJMCU_MEAS_MODE_1S,&dev_i2c);

    if(I2C_NO_ERROR != error)
    {
        uart_report_add_error(error);
        return;
    }

}

void cjmcu_worker()
{
    i2c_error error = I2C_NO_ERROR;
    uint8_t measured_data[CJMCU_MESURE_DATA_LENGHT] = {0};

    thread_worker_sleep_request(CJMCU_START_MEASURE_READING_TIME);
    
    error = i2c_burst_read_register(&measured_data[0],CJMCU_MESURE_DATA_LENGHT,&dev_i2c,ALG_RESULT_DATA);

    if(I2C_NO_ERROR != error)
    {
        uart_report_add_error(error);
        return;
    }

    fine_data.co2_data = ((measured_data[0]<<UTIL_SHIFT_EIGHT) | measured_data[1]);
    fine_data.volatile_organic_compound = ((measured_data[2]<<UTIL_SHIFT_EIGHT) | measured_data[3]);
#if IS_ENABLED(CONFIG_PRINTK)
    printk("The falue of eCO2 is: %d and the TVOC is: %d\n",fine_data.co2_data ,fine_data.volatile_organic_compound );
#endif
}

static uint8_t standardize_data(uint8_t* env_data)
{
    env_data[0] = ((fine_data.co2_data>>8) & 0xFF);
    env_data[1] = ((fine_data.co2_data) & 0xFF);

    env_data[2] = ((fine_data.volatile_organic_compound >> 8) & 0xFF);
    env_data[3] = ((fine_data.volatile_organic_compound) & 0xFF);

    return CJMCU_DATA_LENGHT;
}

void cjmcu_init()
{
    i2c_error error = I2C_NO_ERROR;
    uint8_t chipId[] = {HW_ID_R};
    
    error = config_i2c_driver(dev_i2c);

    if(I2C_NO_ERROR != error)
    {
        uart_report_add_error(error);
        return;
    }

    error = i2c_read_sensor_id(chipId,WH_ID,&dev_i2c);

    if(I2C_NO_ERROR != error)
    {
        uart_report_add_error(error);
        return;
    }
    uint8_t reg_data = BL_START_APP_W;
    error = i2c_write_dt(&dev_i2c,&reg_data,1);

    if(I2C_NO_ERROR != error)
    {
        uart_report_add_error(error);
        return;
    }

    util_register_cb(&standardize_data);

    k_msleep(CJMCU_START_APPLICATION_TIME);

    get_status();
    config_1s_reading();

}