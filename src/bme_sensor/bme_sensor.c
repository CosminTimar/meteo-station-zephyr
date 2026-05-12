#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "bme_sensor.h"

#define I2C_BME_NODE DT_NODELABEL(bme_sensor)

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_BME_NODE);

struct bme280_data bmedata;
static bme_fine_data_type bme_fine_data;
static int32_t t_fine;

/* Read sensor calibration data and stores these into sensor data */
static void bme_calibrationdata(struct bme280_data *sensor_data_ptr)
{
	
	uint8_t values[24];

	int ret = i2c_burst_read_dt(&dev_i2c, CALIB00, values, 24);

	if (ret != 0) {
		printk("Failed to read register %x \n", CALIB00);
		return;
	}

	sensor_data_ptr->dig_t1 = ((uint16_t)values[1]) << 8 | values[0];
	sensor_data_ptr->dig_t2 = ((uint16_t)values[3]) << 8 | values[2];
	sensor_data_ptr->dig_t3 = ((uint16_t)values[5]) << 8 | values[4];
	sensor_data_ptr->dig_p1 = ((uint16_t)values[7]) << 8 | values[6];
	sensor_data_ptr->dig_p2 = ((uint16_t)values[9]) << 8 | values[8];
	sensor_data_ptr->dig_p3 = ((uint16_t)values[11]) << 8 | values[10];
	sensor_data_ptr->dig_p4 = ((uint16_t)values[13]) << 8 | values[12];
	sensor_data_ptr->dig_p5 = ((uint16_t)values[15]) << 8 | values[14];
	sensor_data_ptr->dig_p6 = ((uint16_t)values[17]) << 8 | values[16];
	sensor_data_ptr->dig_p7 = ((uint16_t)values[19]) << 8 | values[18];
	sensor_data_ptr->dig_p8 = ((uint16_t)values[21]) << 8 | values[20];
	sensor_data_ptr->dig_p9 = ((uint16_t)values[23]) << 8 | values[22];

}

/* Compensate current temperature using previously stored sensor calibration data */
static int32_t bme280_compensate_temp(struct bme280_data *data, int32_t adc_temp)
{
	int32_t var1, var2;

	var1 = (((adc_temp >> 3) - ((int32_t)data->dig_t1 << 1)) * ((int32_t)data->dig_t2)) >> 11;

	var2 = (((((adc_temp >> 4) - ((int32_t)data->dig_t1)) *
		  ((adc_temp >> 4) - ((int32_t)data->dig_t1))) >>
		 12) *
		((int32_t)data->dig_t3)) >>
	       14;

	t_fine = var1 + var2;
	return ((var1 + var2) * 5 + 128) >> 8;
}

/* Compensate current temperature using previously stored sensor calibration data */
static int32_t bme280_compensate_pres(struct bme280_data *data, int32_t adc_pres)
{
	int64_t var1, var2, p;

	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)data->dig_p6;
	var2 = var2 + ((var1*(int64_t)data->dig_p5)<<17);
	var2 = var2 + (((int64_t)data->dig_p4)<<35);
	var1 = ((var1 * var1 * (int64_t)data->dig_p3)>>8) + ((var1 * (int64_t)data->dig_p2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)data->dig_p1)>>33;
	if (var1 == 0)
	{
	return 0; // avoid exception caused by division by zerorefactor
	}
	p = 1048576-adc_pres;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((int64_t)data->dig_p9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)data->dig_p8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)data->dig_p7)<<4);

	return (int32_t)p;
}

void bme_worker(void)
{
	uint8_t temp_val[3] = {0};
	int ret = i2c_burst_read_dt(&dev_i2c, TEMPMSB, temp_val, 3);

	if (ret != 0) {
		printk("Failed to read register %x \n", TEMPMSB);
		k_msleep(SLEEP_TIME_MS);
	}

	uint8_t press_val[3] = {0};
	ret = i2c_burst_read_dt(&dev_i2c, PRESMSB, press_val, 3);

	if (ret != 0) {
		printk("Failed to read register %x \n", PRESMSB);
		k_msleep(SLEEP_TIME_MS);
	}

	int32_t adc_temp =
		(temp_val[0] << 12) | (temp_val[1] << 4) | ((temp_val[2] >> 4) & 0x0F);

	int32_t adc_pres =
		(press_val[0] << 12) | (press_val[1] << 4) | ((press_val[2] >> 4) & 0x0F);

	int32_t comp_temp = bme280_compensate_temp(&bmedata, adc_temp);
	int32_t comp_pres = bme280_compensate_pres(&bmedata, adc_pres);

	bme_fine_data.presure = (float)(comp_pres /256) / 100.0f;

	bme_fine_data.temperature = (float)comp_temp / 100.0f;

	printk("Temperature in Celsius : %8.2f C\n", (double)bme_fine_data.temperature);
	printk("Pressure in hPa is : %.2f hPa\n", (double)bme_fine_data.presure);

	k_msleep(SLEEP_TIME_MS);
}

void get_bme_data(bme_fine_data_type* bme_data)
{
	bme_data = &bme_fine_data;
}

void bme_init(void)
{
    uint8_t regs[] = {ID_REG};
	uint8_t error = config_i2c_driver(dev_i2c);

    if(I2C_NO_ERROR != error)
	{
		return;
	}   

    error = i2c_read_sensor_id(regs, CHIP_ID, &dev_i2c);
    if(I2C_NO_ERROR != error)
	{
		return;
	}   

    bme_calibrationdata(&bmedata);

    error = i2c_sensor_config(CTRLMEAS,SENSOR_CONFIG_VALUE, &dev_i2c);
    if(I2C_NO_ERROR != error)
	{
		return;
	}   

    bme_worker();   
}
