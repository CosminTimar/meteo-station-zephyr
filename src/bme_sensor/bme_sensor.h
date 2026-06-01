#ifndef BME_SENSORS_HEADER
#define BME_SENSORS_HEADER

#include "util.h"

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
	uint8_t dig_h1;
	int16_t dig_h2;
	uint8_t dig_h3;
	int16_t dig_h4;
	int16_t dig_h5;
	int8_t  dig_h6;
};

typedef struct bme_fine_data
{
	float temperature;
	float presure; 
	float humidity;
}bme_fine_data_type;

/* BME sensor is read and the values put in the struct */
void bme_worker(void);

/* Initialize the bme280 sensor */
void bme_init(void);

#endif