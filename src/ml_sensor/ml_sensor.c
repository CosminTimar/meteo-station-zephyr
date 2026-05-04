#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "ml_sensor.h"

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

static int16_t sample_buffer;

static struct adc_sequence sequance = {
	.buffer = &sample_buffer,
	.buffer_size = sizeof(sample_buffer),
};

volatile uint8_t error;

static float voltage_to_uv(float voltage_mv)
{
    float v = voltage_mv / 1000.0f;
    if (v < 1.0f) return 0.0f;
    /* Linear map: 1.0V -> 0, 2.8V -> 15 mW/cm² */
    float uv = (v - 1.0f) * (15.0f / (2.8f - 1.0f));
    if (uv < 0.0f) uv = 0.0f;
    return uv;
}

void ml_worker()
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

		float uv_intensity = voltage_to_uv((float)val_mv);

		printk("MV from sensor : %.2f \n", uv_intensity);
	}
}