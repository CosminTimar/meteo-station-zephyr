#ifndef MH_RD_HEADER
#define MH_RD_HEADER


#include "util.h"

#define MH_RD_DATA_LENGHT  (1U)

typedef enum{
    DRY = 0,
    LOW,
    MEDIUM,
    HEAVY,
    ILLIGAL_ZONE
}rain_intensity_e;

void rain_sensor_init(void);

void rain_worker(void);


#endif