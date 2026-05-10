#ifndef ML_SENSOR_HEADER
#define ML_SENSOR_HEADER

#include<zephyr/drivers/adc.h>
#include "../util/util.h"

void ml_worker(void);

void ml_init(void);

#endif