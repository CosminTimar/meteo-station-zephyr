#ifndef CJMCU_SENSOR_HEADER
#define CJMCU_SENSOR_HEADER

#include <zephyr/drivers/i2c.h>
#include "../util/util.h"


#define WH_ID               (0x81)

/* Application registers */
#define STATUS_REG_R        (0x00)
#define MEAS_MODE_RW        (0x01)
#define ALG_RESULT_DATA     (0x02)
#define RAW_DATA_R          (0x03)
#define ENV_DATA_R          (0x05)
#define THRESHOLDS_W        (0x10)
#define BASELINE_RW         (0x11)
#define HW_ID_R             (0x20)
#define HW_VERSION_R        (0x21)
#define FW_BOOT_VERS_R      (0x23)
#define FW_APP_VERS_R       (0x24)
#define ERROR_ID_R          (0xE0)
#define SW_RESET_W          (0xFF)

/* Bootloader registers */
#define BL_START_APP_W      (0xF4)

/* Time sync */
#define CJMCU_START_APPLICATION_TIME        (200U)
#define CJMCU_START_MEASURE_READING_TIME    (1200U)

/* Number of bytes for the measurement register */
#define CJMCU_MESURE_DATA_LENGHT        (0x08U)
/* Number of bytes for the status register */
#define CJMCU_STATUS_RW_LENGHT      	(0x01U)
/* Config for the measurement mode: bit 4 set on 1 represent measurement every second */
#define CJMCU_MEAS_MODE_1S          	(1<<4U)


struct measure_mode{
    uint8 drive_mode;
    uint8 data_ready_int;
    uint8 threshold_int;
};

struct measurement_resut{
    uint16 co2_data;
    uint16 volatile_organic_compound;
    uint8 status;
    uint8 error;
    uint8 raw_data;
};

struct raw_data{
    uint8 current_trough;
    uint8 raw_adc;
};

struct env_data{
    uint8 humidity_low;
    uint8 humidity_high;
    uint8 temperature_low;
    uint8 temperature_high;
};

struct threshold{
    uint16 low_to_medium_thr;
    uint16 medium_to_high_thr;
    uint32 hysteresis;
};

struct baseline{
    uint16 baseline;
};

struct hw_id{
    uint8 hw_id;
};

struct error_id{
    uint8 error_code;
};

void cjmcu_worker(void);

void cjmcu_init(void);




#endif