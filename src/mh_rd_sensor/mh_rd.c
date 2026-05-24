#include <zephyr/device.h>
#include<zephyr/drivers/adc.h>

#include "mh_rd.h"

#define DT_UV_SENSOR	DT_PATH(zephyr_user)

#define DRY_THRESHOLD           (2800U)
#define LOW_THRESHOLD           (1800U)
#define MEDIUM_THRESHOLD        (1000U)
#define HEAVY_THRESHOLD         (0U)

static rain_intensity_e voltage_to_rain_intensity(uint32_t voltage_mv);


static uint8_t standardize_data(uint8_t* env_data);

static const struct adc_dt_spec rain_adc_channel = ADC_DT_SPEC_GET_BY_NAME(DT_PATH(zephyr_user), rain_sensor);

static uint16 rain_sample_buffer;
static struct rain_internal_struct rain_data;

static struct adc_sequence rain_sequance = {
	.buffer = &rain_sample_buffer,
	.buffer_size = sizeof(rain_sample_buffer),
};

static uint8_t standardize_data(uint8_t* env_data)
{
    env_data[0] = (uint8_t)rain_data.rain_adc_conv;

    return MH_RD_DATA_LENGHT;
}

static rain_intensity_e voltage_to_rain_intensity(uint32_t voltage_mv)
{
    /*
        3300 - 2800 mV  - dry
        2800 - 1800 mv  - low rain
        1800 - 1000 mV  - medium 
        1000 -          - heavy
    */
   const uint16_t rain_intensity_zone[] = {DRY_THRESHOLD,LOW_THRESHOLD,MEDIUM_THRESHOLD,HEAVY_THRESHOLD};

   for(uint8_t zone_index = 0; zone_index < ARRAY_SIZE(rain_intensity_zone); zone_index++)
   {
        if(voltage_mv > rain_intensity_zone[zone_index])
        {
            return (rain_intensity_e)zone_index;
        }
   }
   return ILLIGAL_ZONE;

}

void rain_worker()
{
	int error = ADC_NO_ERROR;

	rain_data.convertion_done = false;
	
	error = adc_read(rain_adc_channel.dev, &rain_sequance);
	if(ADC_NO_ERROR != error)
	{
		return;
	}

	uint32 val_mv = rain_sample_buffer;

	error = adc_raw_to_millivolts_dt(&rain_adc_channel, &val_mv);

	if(ADC_NO_ERROR > error)
	{
		return;
	}

	rain_data.rain_adc_conv = voltage_to_rain_intensity(val_mv);

	rain_data.convertion_done = true;
#if 1 == PRINT_DATA
	printk("The voltige output is: %d\n", rain_data.rain_adc_conv);
#endif

}

void rain_sensor_init()
{
	int error = ADC_NO_ERROR;
	error = adc_is_ready_dt(&rain_adc_channel);

	if(true != error)
	{
		return;
	}

	error = adc_channel_setup_dt(&rain_adc_channel);

	if(ADC_NO_ERROR != error)
	{
		return;
	}

	error = adc_sequence_init_dt(&rain_adc_channel, &rain_sequance);
	if(ADC_NO_ERROR != error)
	{
		return;
	}

	util_register_cb(&standardize_data);
}