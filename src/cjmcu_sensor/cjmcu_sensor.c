#include <zephyr/device.h>

#include "cjmcu_sensor.h"
#include "thread_worker.h"
#include "uart_report.h"

#define I2C_CJMCU_SENSOR   DT_NODELABEL(cjmcu_sensor)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_CJMCU_SENSOR);

static struct measurement_resut cjmcu_data;

static uint8_t standardize_data(uint8_t* env_data);

static void get_status(void);

static void config_1s_reading(void);

static void get_status()
{
    int error = I2C_NO_ERROR;

    uint8_t status[] = {STATUS_REG_R};
    uint8_t res_status = UTIL_DEFAULT_VALUE;

   error = i2c_write_read_dt(&dev_i2c, status, CJMCU_STATUS_RW_LENGHT, &res_status, CJMCU_STATUS_RW_LENGHT);

    if(I2C_NO_ERROR != error)
    {
        cjmcu_data.error |= 1U << I2C_READ_WRITE_ERROR;
    }
}

static void config_1s_reading()
{
    util_modules_error error = I2C_NO_ERROR;

    error = i2c_sensor_config(MEAS_MODE_RW,CJMCU_MEAS_MODE_1S,&dev_i2c);

    if(I2C_NO_ERROR != error)
    {
        cjmcu_data.error |= 1U << error;
    }

}

void cjmcu_worker()
{
    if(util_get_init_error(cjmcu_data.error))
	{
        util_modules_error error = I2C_NO_ERROR;
        uint8_t measured_data[CJMCU_MESURE_DATA_LENGHT] = {0};

        thread_worker_sleep_request(CJMCU_START_MEASURE_READING_TIME);
        
        error = i2c_burst_read_register(&measured_data[0],CJMCU_MESURE_DATA_LENGHT,&dev_i2c,ALG_RESULT_DATA);

        if(I2C_NO_ERROR != error)
        {
            cjmcu_data.error |= 1U << error;
        }

        cjmcu_data.co2_data = ((measured_data[0]<<UTIL_SHIFT_EIGHT) | measured_data[1]);
        cjmcu_data.volatile_organic_compound = ((measured_data[2]<<UTIL_SHIFT_EIGHT) | measured_data[3]);
    #if IS_ENABLED(CONFIG_PRINTK)
        printk("The falue of eCO2 is: %d and the TVOC is: %d\n",cjmcu_data.co2_data ,cjmcu_data.volatile_organic_compound );
    #endif
        if(I2C_NO_ERROR != cjmcu_data.error)
        {
        #if IS_ENABLED(CONFIG_UART_REPORT_ENABLE)
            uart_report_add_error(cjmcu_data.error);
        #endif
        }
    }
}

static uint8_t standardize_data(uint8_t* env_data)
{
    env_data[0] = ((cjmcu_data.co2_data>>8) & 0xFF);
    env_data[1] = ((cjmcu_data.co2_data) & 0xFF);

    env_data[2] = ((cjmcu_data.volatile_organic_compound >> 8) & 0xFF);
    env_data[3] = ((cjmcu_data.volatile_organic_compound) & 0xFF);

    return CJMCU_DATA_LENGHT;
}

void cjmcu_init()
{
    util_modules_error error = I2C_NO_ERROR;
    cjmcu_data.error = I2C_NO_ERROR;
    uint8_t chipId[] = {HW_ID_R};
    
    error = config_i2c_driver(dev_i2c);

    if(I2C_NO_ERROR != error)
    {
        cjmcu_data.error |= 1U << error;
    }

    error = i2c_read_sensor_id(chipId,WH_ID,&dev_i2c);

    if(I2C_NO_ERROR != error)
    {
        cjmcu_data.error |= 1U << error;
    }
    uint8_t reg_data = BL_START_APP_W;
    error = i2c_write_dt(&dev_i2c,&reg_data,1);

    if(I2C_NO_ERROR != error)
    {
        cjmcu_data.error |= 1U << error;
    }

    if(I2C_NO_ERROR != cjmcu_data.error)
    {
    #if IS_ENABLED(CONFIG_UART_REPORT_ENABLE)
        uart_report_add_error(cjmcu_data.error);
    #endif
        return;
    }

    util_register_cb(&standardize_data);

    k_msleep(CJMCU_START_APPLICATION_TIME);

    get_status();
    config_1s_reading();

}