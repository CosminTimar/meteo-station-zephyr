#ifndef ML_SENSOR_HEADER
#define ML_SENSOR_HEADER

#include<zephyr/drivers/adc.h>
#include "../util/util.h"


static struct ml_internal_struct
{
    bool convertion_done;
    uint8_t uv_index;
};

static uint8_t standardize_data(uint8_t* env_data);

void ml_worker(void);

void ml_init(void);

#endif