#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>

#include "bme_sensor.h"
#include "uart_report.h"

static void calibration_data(struct bme280_data *sensor_data_ptr);

static uint8_t standardize_data(uint8_t* env_data);

static int32_t compensate_temp(struct bme280_data *data, int32_t adc_temp);

static int32_t compensate_pres(struct bme280_data *data, int32_t adc_pres);

#define I2C_BME_NODE DT_NODELABEL(bme_sensor)

#define CTRLMEAS                (0xF4)
#define CALIB00	                (0x88)
#define CALIB01					(0xE1)
#define ID_REG	                (0xD0)
#define TEMPMSB	                (0xFA)
#define PRESMSB	                (0xF7)
#define HUMMSB					(0xFD)
#define CHIP_ID                 (0x60)
#define SENSOR_CONFIG_VALUE     (0x93)

#define BME_DATA_LENGHT					(12U)
#define BME_CALIB00_DATA_LENGHT			(25U)
#define BME_CALIB01_DATA_LENGHT			(8U)
#define BME_ENV_REG_DATA_LENGHT			(3U)
#define BME_HUMIDITY_DATA_LENGHT		(2U)
#define BME_TEMPERATURE_CENTIDEGREE_TO_DEGREE (100.0f)
/* Temperature algo */
#define BME_MAGNITUDE_VALUE_ALGO		(5U)
#define BME_ROUND_UP_VALUE_ALGO			(128U)
/* Pressure algo */
#define BME_NORMALIZE_TEMPERATURE		(128000U)
#define BME_PRESSURE_INVERSE_READING	(1048576U)
#define BME_PRESSURE_BOSH_SCALES		(3125U)
#define BME_PRESSURE_Q24_TO_HPA 		(25600.0f)
/* Humidity algo */
#define BME_HUMIDITY_TEMP				(76800U)
#define BME_HUMIDITY_ROUNDING_0			(16384U)
#define BME_HUMIDITY_ROUNDING_1			(32768U)
#define BME_HUMIDITY_ROUNDING_2			(2097152U)
#define BME_HUMIDITY_ROUNDING_3			(8192U)
#define BME_HUMIDITY_SATURATION			(419430400U)
#define BME_HUMIDITY_Q22_10_SCALE  		(1024.0f)

static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_BME_NODE);

static struct bme280_data bmedata;
static bme_fine_data_type bme_fine_data;
static int32_t temperature_fine;

/* Read sensor calibration data and stores these into sensor data */
static void calibration_data(struct bme280_data *sensor_data_ptr)
{
	
	uint8_t values[(BME_CALIB00_DATA_LENGHT + BME_CALIB01_DATA_LENGHT)];

	int error = i2c_burst_read_dt(&dev_i2c, CALIB00, values, BME_CALIB00_DATA_LENGHT);

	if (error != I2C_NO_ERROR) {
		uart_report_add_error(I2C_READ_WRITE_ERROR);
	#if IS_ENABLED(CONFIG_PRINTK)
		printk("Failed to read register %x \n", CALIB00);
	#endif
		return;
	}

	sensor_data_ptr->dig_t1 = ((((uint16_t)values[1]) << 8) | values[0]);
	sensor_data_ptr->dig_t2 = ((((uint16_t)values[3]) << 8) | values[2]);
	sensor_data_ptr->dig_t3 = ((((uint16_t)values[5]) << 8) | values[4]);

	sensor_data_ptr->dig_p1 = ((((uint16_t)values[7]) << 8) | values[6]);
	sensor_data_ptr->dig_p2 = ((((uint16_t)values[9]) << 8) | values[8]);
	sensor_data_ptr->dig_p3 = ((((uint16_t)values[11]) << 8) | values[10]);
	sensor_data_ptr->dig_p4 = ((((uint16_t)values[13]) << 8) | values[12]);
	sensor_data_ptr->dig_p5 = ((((uint16_t)values[15]) << 8) | values[14]);
	sensor_data_ptr->dig_p6 = ((((uint16_t)values[17]) << 8) | values[16]);
	sensor_data_ptr->dig_p7 = ((((uint16_t)values[19]) << 8) | values[18]);
	sensor_data_ptr->dig_p8 = ((((uint16_t)values[21]) << 8) | values[20]);
	sensor_data_ptr->dig_p9 = ((((uint16_t)values[23]) << 8) | values[22]);

	sensor_data_ptr->dig_h1 = (uint8_t)(values[24]);

	error = i2c_burst_read_dt(&dev_i2c, CALIB01, &values[25], BME_CALIB01_DATA_LENGHT);

	if (error != I2C_NO_ERROR) {
		uart_report_add_error(I2C_READ_WRITE_ERROR);
	#if IS_ENABLED(CONFIG_PRINTK)
		printk("Failed to read register %x \n", CALIB01);
	#endif
		return;
	}
	sensor_data_ptr->dig_h2 = ((((uint16_t)values[26]) << 8) | values[25]);
	sensor_data_ptr->dig_h3 = (uint8_t)(values[27]);
	sensor_data_ptr->dig_h4 = ((((uint16_t)values[29]) << 8) | values[28]);
	sensor_data_ptr->dig_h5 = ((((uint16_t)values[31]) << 8) | values[30]);
	sensor_data_ptr->dig_h6 = ((int8_t)values[32]);
}

/* Compensate current temperature using previously stored sensor calibration data */
static int32_t compensate_temp(struct bme280_data *data, int32_t adc_temp)
{
	int32_t var1, var2;

	var1 = (((adc_temp >> 3) - ((int32_t)data->dig_t1 << 1)) * ((int32_t)data->dig_t2)) >> 11;

	var2 = (((((adc_temp >> 4) - ((int32_t)data->dig_t1)) *
		  	((adc_temp >> 4) - ((int32_t)data->dig_t1))) >> 12) *
			((int32_t)data->dig_t3)) >> 14;

	temperature_fine = var1 + var2;
	return ((var1 + var2) * BME_MAGNITUDE_VALUE_ALGO + BME_ROUND_UP_VALUE_ALGO) >> 8;
}



/* Compensate current temperature using previously stored sensor calibration data */
static int32_t compensate_pres(struct bme280_data *data, int32_t adc_pres)
{
	int64_t var1, var2, pressure;

	var1 = ((int64_t)temperature_fine) - BME_NORMALIZE_TEMPERATURE;
	var2 = var1 * var1 * (int64_t)data->dig_p6;
	var2 = var2 + ((var1*(int64_t)data->dig_p5)<<17);
	var2 = var2 + (((int64_t)data->dig_p4)<<35);
	var1 = ((var1 * var1 * (int64_t)data->dig_p3)>>8) + ((var1 * (int64_t)data->dig_p2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)data->dig_p1)>>33;

	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zerorefactor
	}

	pressure = BME_PRESSURE_INVERSE_READING - adc_pres;
	pressure = (((pressure<<31)-var2)*BME_PRESSURE_BOSH_SCALES)/var1;
	var1 = (((int64_t)data->dig_p9) * (pressure>>13) * (pressure>>13)) >> 25;
	var2 = (((int64_t)data->dig_p8) * pressure) >> 19;
	pressure = ((pressure + var1 + var2) >> 8) + (((int64_t)data->dig_p7)<<4);

	return (int32_t)pressure;
}

/* Compensate current humidity using previously stored sensor calibration data */
static int32_t compensate_hum(struct bme280_data *data, int32_t comp_hum)
{
	int32_t humidity = 0;

	humidity = (temperature_fine - BME_HUMIDITY_TEMP);
	humidity = ((((comp_hum << 14) - (((int32_t)(data->dig_h4)) << 20) - (((int32_t)data->dig_h5) * humidity)) +
				((int32_t)BME_HUMIDITY_ROUNDING_0)) >> 15) * ((((((((humidity * ((int32_t)data->dig_h6)) >> 10 ) * (((humidity * 
				((int32_t)data->dig_h3)) >> 11 ) + ((int32_t)BME_HUMIDITY_ROUNDING_1))) >> 10 ) + ((int32_t)BME_HUMIDITY_ROUNDING_2)) * 
				((int32_t)data->dig_h2) + BME_HUMIDITY_ROUNDING_3) >> 14));

	humidity = (humidity - (((((humidity >> 15) * (humidity >> 15)) >> 7) * ((int32_t)data->dig_h1)) >> 4));

	if(0 >humidity)
	{
		return 0;
	}

	if (BME_HUMIDITY_SATURATION < humidity)
	{
		return ((int32_t)BME_HUMIDITY_SATURATION);
	}

	return (int32_t)(humidity >> 12);
}

void bme_worker(void)
{
	uint8_t temp_val[BME_ENV_REG_DATA_LENGHT] = {0};
	int error = i2c_burst_read_dt(&dev_i2c, TEMPMSB, temp_val, BME_ENV_REG_DATA_LENGHT);

	if (error != 0) {
		uart_report_add_error(I2C_READ_WRITE_ERROR);
	#if IS_ENABLED(CONFIG_PRINTK)
		printk("Failed to read register %x \n", TEMPMSB);
	#endif
		return;
	}

	uint8_t press_val[BME_ENV_REG_DATA_LENGHT] = {0};
	error = i2c_burst_read_dt(&dev_i2c, PRESMSB, press_val, BME_ENV_REG_DATA_LENGHT);

	if (error != 0) {
		uart_report_add_error(I2C_READ_WRITE_ERROR);
	#if IS_ENABLED(CONFIG_PRINTK)
		printk("Failed to read register %x \n", PRESMSB);
	#endif
		return;
	}

	uint8_t hum_val[BME_HUMIDITY_DATA_LENGHT] = {0};
	error = i2c_burst_read_dt(&dev_i2c, HUMMSB, hum_val, BME_HUMIDITY_DATA_LENGHT);

	if (error != 0) {
		uart_report_add_error(I2C_READ_WRITE_ERROR);
	#if IS_ENABLED(CONFIG_PRINTK)
		printk("Failed to read register %x \n", PRESMSB);
	#endif
		return;
	}

	int32_t comp_temp = (temp_val[0] << 12) | (temp_val[1] << 4) | ((temp_val[2] >> 4) & 0x0F);

	int32_t comp_pres = (press_val[0] << 12) | (press_val[1] << 4) | ((press_val[2] >> 4) & 0x0F);

	int32_t comp_hum = ((hum_val[0] << 8) | (hum_val[1] & 0xFF));

	comp_temp = compensate_temp(&bmedata, comp_temp);
	comp_pres = compensate_pres(&bmedata, comp_pres);
	comp_hum  = compensate_hum(&bmedata, comp_hum);
	

	bme_fine_data.presure = (float)(comp_pres / BME_PRESSURE_Q24_TO_HPA);

	bme_fine_data.temperature = (float)comp_temp / BME_TEMPERATURE_CENTIDEGREE_TO_DEGREE;

	bme_fine_data.humidity = (float)(comp_hum) / BME_HUMIDITY_Q22_10_SCALE;

#if IS_ENABLED(CONFIG_PRINTK)
	printk("Temperature in Celsius : %8.2f C\n", (double)bme_fine_data.temperature);
	printk("Pressure in hPa is : %.2f hPa\n", (double)bme_fine_data.presure);
	printk("Humidity in RH is : %.2f %% RH\n", (double)bme_fine_data.humidity);
#endif
}

static uint8_t standardize_data(uint8_t* env_data)
{
	util_float_to_uint8(bme_fine_data.temperature, &env_data[0]);
	util_float_to_uint8(bme_fine_data.presure, &env_data[4]);
	util_float_to_uint8(bme_fine_data.humidity, &env_data[8]);
	return BME_DATA_LENGHT;
}

void bme_init(void)
{
    uint8_t regs[] = {ID_REG};
	i2c_error error = config_i2c_driver(dev_i2c);

    if(I2C_NO_ERROR != error)
	{
		uart_report_add_error(error);
		return;
	}   

    error = i2c_read_sensor_id(regs, CHIP_ID, &dev_i2c);
    if(I2C_NO_ERROR != error)
	{
		uart_report_add_error(error);
		return;
	}   

    calibration_data(&bmedata);

    error = i2c_sensor_config(CTRLMEAS,SENSOR_CONFIG_VALUE, &dev_i2c);
    if(I2C_NO_ERROR != error)
	{
		uart_report_add_error(error);
		return;
	}

	util_register_cb(&standardize_data);
}
