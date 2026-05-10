#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include "ml_sensor.h"

#define DT_UV_SENSOR	DT_PATH(zephyr_user)

static const struct adc_dt_spec ml_adc_channel = ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), uv_sensor);

static uint16 ml_sample_buffer;
static struct ml_internal_struct ml_data;

static struct adc_sequence ml_sequance = {
	.buffer = &ml_sample_buffer,
	.buffer_size = sizeof(ml_sample_buffer),
};

/* Volt defines used for offset and span */
#define ML_MV_TO_VOLT			(1000.0f)
#define ML_LOW_VOLT_OUTPUT		(1.0f)
#define ML_HIGH_VOLT_OUTPUT		(3.3f)

/* UV define for the max intensity */
#define ML_HIGH_UV_INTENSITY	(15.0f)
/* UV define for index transformation */
#define UV_INDEX_NOAA			(2.5f)


static float voltage_to_uv_intensity(float voltage_mv)
{
	/* Transfomr the mV to V */
    float volt = voltage_mv / ML_MV_TO_VOLT;

    if (volt < ML_LOW_VOLT_OUTPUT) 
	{
		return 0.0f;
	}

    /* Linear map: 1.0V -> 0, 3.3V -> 15 mW/cm² */
    float uv_intensity = (volt - ML_LOW_VOLT_OUTPUT) * (ML_HIGH_UV_INTENSITY / (ML_HIGH_VOLT_OUTPUT - ML_LOW_VOLT_OUTPUT));

    if (uv_intensity < 0.0f)
 	{ 
		uv_intensity = 0.0f;
	}
    return uv_intensity;
}

/* Can be an inline*/
static float uv_intensity_to_index(float uv_intensity)
{
	return uv_intensity * UV_INDEX_NOAA;
}

void ml_worker()
{
	int error = ADC_NO_ERROR;

	ml_data.convertion_done = false;
	
	error = adc_read(ml_adc_channel.dev, &ml_sequance);
	if(ADC_NO_ERROR != error)
	{
		return;
	}

	uint32 val_mv = ml_sample_buffer;

	error = adc_raw_to_millivolts_dt(&ml_adc_channel, &val_mv);

	if(ADC_NO_ERROR > error)
	{
		return;
	}

	float uv_intensity = voltage_to_uv_intensity((float)val_mv);

	ml_data.uv_index = uv_intensity_to_index(uv_intensity);

	ml_data.convertion_done = true;

}

float ml_get_uv_index()
{
	if(ADC_NO_ERROR != ml_data.convertion_done)
	{
		return ADC_CONVERSION_IN_PROGRESS;
	}
	return ml_data.uv_index;
}

void ml_init()
{
	int error = ADC_NO_ERROR;
	error = adc_is_ready_dt(&ml_adc_channel);

	if(true != error)
	{
		return;
	}

	error = adc_channel_setup_dt(&ml_adc_channel);

	if(ADC_NO_ERROR != error)
	{
		return;
	}

	error = adc_sequence_init_dt(&ml_adc_channel, &ml_sequance);
	if(ADC_NO_ERROR != error)
	{
		return;
	}
}