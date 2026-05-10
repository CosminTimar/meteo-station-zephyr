#ifndef ML_SENSOR_HEADER
#define ML_SENSOR_HEADER

#include<zephyr/drivers/adc.h>
#include "../util/util.h"


static struct ml_internal_struct
{
    bool convertion_done;
    float uv_index;
};

void ml_worker(void);

void ml_init(void);

#endif