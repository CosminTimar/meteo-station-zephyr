#include <zephyr/device.h>
#include<zephyr/drivers/adc.h>

#include "ml_sensor.h"
#include "uart_report.h"

#define DT_UV_SENSOR	DT_PATH(zephyr_user)

static uint8_t standardize_data(uint8_t* env_data);

static uint8_t voltage_to_uv_intensity(float voltage_mv);

static const struct adc_dt_spec ml_adc_channel = ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), uv_sensor);

static uint16_t ml_sample_buffer;
static struct ml_internal_struct ml_data;

static struct adc_sequence ml_sequance = {
	.buffer = &ml_sample_buffer,
	.buffer_size = sizeof(ml_sample_buffer),
};

/* Volt defines used for offset and span */
#define ML_MV_TO_VOLT			(1000.0f)
#define ML_LOW_VOLT_OUTPUT		(1.0f)
#define ML_HIGH_VOLT_OUTPUT		(3.0f)

/* UV define for the max intensity */
#define ML_HIGH_UV_INTENSITY	(15U)
#define ML_LOW_UV_INTENSITY		(1U)


static uint8_t voltage_to_uv_intensity(float voltage_mv)
{
	/* Transfomr the mV to V */
    float volt = voltage_mv / ML_MV_TO_VOLT;

    if (volt < ML_LOW_VOLT_OUTPUT) 
	{
		return 0;
	}

    /* Linear map: 1.0V -> 0, 3.0V -> 15 mW/cm² */
    uint8_t uv_intensity = ((volt - ML_LOW_VOLT_OUTPUT) * (ML_HIGH_UV_INTENSITY- ML_LOW_UV_INTENSITY))/ (ML_HIGH_VOLT_OUTPUT - ML_LOW_VOLT_OUTPUT);

    return uv_intensity;
}

static uint8_t standardize_data(uint8_t* env_data)
{
	if(true != ml_data.convertion_done)
	{
		env_data[0] = ADC_CONVERSION_IN_PROGRESS;
	}
	else
	{
		env_data[0] = ml_data.uv_index;
	}

	return ML_DATA_LENGHT;
}

void ml_worker()
{
	int error = ADC_NO_ERROR;

	ml_data.convertion_done = false;
	
	error = adc_read(ml_adc_channel.dev, &ml_sequance);
	if(ADC_NO_ERROR != error)
	{
		uart_report_add_error(ADC_CONVERSION_IN_PROGRESS);
		return;
	}

	uint32_t val_mv = ml_sample_buffer;

	error = adc_raw_to_millivolts_dt(&ml_adc_channel, &val_mv);

	if(ADC_NO_ERROR > error)
	{
		uart_report_add_error(ADC_INTERNAL_ERROR);
		return;
	}

	ml_data.uv_index = voltage_to_uv_intensity((float)val_mv);

	ml_data.convertion_done = true;
#if PRINT_DATA == 1
	printk("The UV index is: %d\n", ml_data.uv_index);
#endif

}

void ml_init()
{
	int error = ADC_NO_ERROR;
	error = adc_is_ready_dt(&ml_adc_channel);

	if(true != error)
	{
		uart_report_add_error(ADC_INTERNAL_ERROR);
		return;
	}

	error = adc_channel_setup_dt(&ml_adc_channel);

	if(ADC_NO_ERROR != error)
	{
		uart_report_add_error(ADC_INTERNAL_ERROR);
		return;
	}

	error = adc_sequence_init_dt(&ml_adc_channel, &ml_sequance);
	if(ADC_NO_ERROR != error)
	{
		uart_report_add_error(ADC_INTERNAL_ERROR);
		return;
	}

	util_register_cb(&standardize_data);
}