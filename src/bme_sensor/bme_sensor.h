#ifndef BME_SENSORS
#define BME_SENSORS

#include <zephyr/drivers/i2c.h>

#define CTRLMEAS                0xF4
#define CALIB00	                0x88
#define ID	                    0xD0
#define TEMPMSB	                0xFA
#define PRESMSB	                0xF7
#define CHIP_ID                 0x60
#define SENSOR_CONFIG_VALUE     0x93

#define SLEEP_TIME_MS           1000

/* Data structure to store BME280 data */
struct bme280_data {
	/* Compensation for Temperature */
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
	/* Compensation for Presure*/
	uint16_t dig_p1;
	int16_t dig_p2;
	int16_t dig_p3;
	int16_t dig_p4;
	int16_t dig_p5;
	int16_t dig_p6;
	int16_t dig_p7;
	int16_t dig_p8;
	int16_t dig_p9;
};


/* Read sensor calibration data and stores these into sensor data */
void bme_calibrationdata(const struct i2c_dt_spec *spec, struct bme280_data *sensor_data_ptr);

/* Compensate current temperature using previously stored sensor calibration data */
static int32_t bme280_compensate_temp(struct bme280_data *data, int32_t adc_temp);

/* Compensate current temperature using previously stored sensor calibration data */
static int32_t bme280_compensate_pres(struct bme280_data *data, int32_t adc_pres);

#endif