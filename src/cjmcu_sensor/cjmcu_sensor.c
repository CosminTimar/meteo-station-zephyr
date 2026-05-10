#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "cjmcu_sensor.h"

#define I2C_CJMCU_SENSOR   DT_NODELABEL(cjmcu_sensor)
static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_CJMCU_SENSOR);

static struct measurement_resut fine_data;


static void cjmcu_get_status()
{
    i2c_error error = I2C_NO_ERROR;

    uint8 status[] = {STATUS_REG_R};
    uint8 res_status = 0xFF;

   error = i2c_write_read_dt(&dev_i2c, status, 1, &res_status, 1);

    if(I2C_NO_ERROR != error)
    {
        return;
    }

   printk("Status register is: %d", res_status);
}

static void cjmcu_config_1s_reading()
{
    i2c_error error = I2C_NO_ERROR;

    error = i2c_sensor_config(MEAS_MODE_RW,0x10,&dev_i2c);

    if(I2C_NO_ERROR != error)
    {
        return;
    }

}

static void cjmcu_read_eCO2_TVOC()
{
    i2c_error error = I2C_NO_ERROR;
    uint8 measured_data[8] = {0};
    
    error = i2c_burst_read_register(&measured_data[0],8,&dev_i2c,ALG_RESULT_DATA);

    if(I2C_NO_ERROR != error)
    {
        return;
    }

    fine_data.co2_data = ((measured_data[0]<<8) | measured_data[1]);
    fine_data.volatile_organic_compound = ((measured_data[3]<<8) | measured_data[4]);

    printk("The falue of eCO2 is: %d and the TVOC is: %d\n",fine_data.co2_data ,fine_data.volatile_organic_compound );
}

void cjmcu_worker()
{
    cjmcu_read_eCO2_TVOC();
}



void cjmcu_init()
{
    i2c_error error = I2C_NO_ERROR;
    uint8 chipId[] = {HW_ID_R};
    
    error = config_i2c_driver(dev_i2c);

    if(I2C_NO_ERROR != error)
    {
        return;
    }

    error = i2c_read_sensor_id(chipId,WH_ID,&dev_i2c);

    if(I2C_NO_ERROR != error)
    {
        return;
    }
    uint8 reg_data = BL_START_APP_W;
    error = i2c_write_dt(&dev_i2c,&reg_data,1);

    if(I2C_NO_ERROR != error)
    {
        return;
    }

    k_msleep(200);

    cjmcu_get_status();
    cjmcu_config_1s_reading();

    while(1)
    {
        k_msleep(1200);
        cjmcu_worker();
    }
}