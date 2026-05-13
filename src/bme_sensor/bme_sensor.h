#ifndef BME_SENSORS_HEADER
#define BME_SENSORS_HEADER

#include <zephyr/drivers/i2c.h>
#include "../util/util.h"

#define CTRLMEAS                (0xF4)
#define CALIB00	                (0x88)
#define ID_REG	                (0xD0)
#define TEMPMSB	                (0xFA)
#define PRESMSB	                (0xF7)
#define CHIP_ID                 (0x60)
#define SENSOR_CONFIG_VALUE     (0x93)

#define SLEEP_TIME_MS           1000

/* Data structure to store BME280 data */
struct bme280_data 
{
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

typedef struct bme_fine_data
{
	float temperature;
	float presure; 
}bme_fine_data_type;

/* Get a reference to the local struct that keeps the sensor data */
void get_bme_data(bme_fine_data_type* bme_data);

/* BME sensor is read and the values put in the struct */
void bme_worker(void);

/* Initialize the bme280 sensor */
void bme_init(void);

#endif