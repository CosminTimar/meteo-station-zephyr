#ifndef ML_SENSOR_HEADER
#define ML_SENSOR_HEADER

#include "util.h"

#define ML_DATA_LENGHT  (1U)


static struct ml_internal_struct
{
    bool convertion_done;
    uint8_t uv_index;
};

void ml_worker(void);

void ml_init(void);

#endif