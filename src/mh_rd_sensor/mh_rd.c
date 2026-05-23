#include <zephyr/device.h>
#include<zephyr/drivers/adc.h>

#include "mh_rd.h"

#define DT_UV_SENSOR	DT_PATH(zephyr_user)

static uint8_t standardize_data(uint8_t* env_data);

static const struct adc_dt_spec rain_adc_channel = ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), rain_sensor);

static uint16 rain_sample_buffer;
static struct rain_internal_struct rain_data;

static struct adc_sequence rain_sequance = {
	.buffer = &rain_sample_buffer,
	.buffer_size = sizeof(rain_sample_buffer),
};