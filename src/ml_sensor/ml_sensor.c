#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "ml_sensor.h"

#define DT_UV_SENSOR	DT_PATH(zephyr_user)

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), uv_sensor);

static int16_t sample_buffer;

static struct adc_sequence sequance = {
	.buffer = &sample_buffer,
	.buffer_size = sizeof(sample_buffer),
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
	while(1)
	{
		error = adc_read(adc_channel.dev, &sequance);
		if(0 != error)
		{
			return;
		}

		int32_t val_mv = sample_buffer;

		error = adc_raw_to_millivolts_dt(&adc_channel, &val_mv);

		if(0 > error)
		{
			return;
		}

		float uv_intensity = voltage_to_uv_intensity((float)val_mv);

		printk("MV from sensor : %.2f \n", uv_intensity);
	}
}

void ml_init()
{
	error = adc_is_ready_dt(&adc_channel);

	if(1 != error)
	{
		return;
	}

	error = adc_channel_setup_dt(&adc_channel);

	if(0 != error)
	{
		return;
	}

	error = adc_sequence_init_dt(&adc_channel, &sequance);
	if(0 != error)
	{
		return;
	}
}