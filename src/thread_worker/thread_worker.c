#include <zephyr/kernel.h>
#include <zephyr/random/random.h>

#include "thread_worker.h"
#include "util.h"
#if IS_ENABLED(CONFIG_BME_SENSOR_ENABLE)
    #include "bme_sensor.h"
#endif
#if IS_ENABLED(CONFIG_CJMCU_SENSOR_ENABLE)
    #include "cjmcu_sensor.h"
#endif
#if IS_ENABLED(CONFIG_ML_SENSOR_ENABLE)
    #include "ml_sensor.h"
#endif
    #include "ble_beacon.h"
#if IS_ENABLED(CONFIG_MH_RD_SENSOR_ENABLE)
    #include "mh_rd.h"
#endif
#if IS_ENABLED(CONFIG_UART_REPORT_ENABLE)
    #include "uart_report.h"
#endif

/* Stack size for threads */
#define STACKSIZE               (1024)
/* Defines for thread prio */
#define INIT_THREAD_PRIORITY    (1) 
#define BLE_THREAD_PRIORITY     (2)
#define WORKER_THREAD_PRIORITY  (3)
#define UART_REPORT_PRIORITY    (4)

#define THREAD_SLEEP_MS         (1000U)

static void init_sensors_thread(void);
static void send_ble_data_thread(void);
static void sensor_worker_thread(void);
static void send_uart_thread(void);

static void init_sensors_thread()
{
    ble_init();
#if IS_ENABLED(CONFIG_BME_SENSOR_ENABLE)
    bme_init();
#endif
#if IS_ENABLED(CONFIG_ML_SENSOR_ENABLE)
    ml_init();
#endif
#if IS_ENABLED(CONFIG_MH_RD_SENSOR_ENABLE)
    rain_sensor_init();
#endif
#if IS_ENABLED(CONFIG_CJMCU_SENSOR_ENABLE)
    cjmcu_init();
#endif
#if IS_ENABLED(CONFIG_UART_REPORT_ENABLE)
    uart_report_init();
#endif
}

static void sensor_worker_thread()
{
    for(;;)
    {
    #if IS_ENABLED(CONFIG_BME_SENSOR_ENABLE)
        bme_worker();
    #endif
    #if IS_ENABLED(CONFIG_CJMCU_SENSOR_ENABLE)
        cjmcu_worker();
    #endif
    #if IS_ENABLED(CONFIG_ML_SENSOR_ENABLE)
        ml_worker();
    #endif
    #if IS_ENABLED(CONFIG_MH_RD_SENSOR_ENABLE)
        rain_worker();
    #endif
        thread_worker_sleep_request(THREAD_SLEEP_MS);
    }
}

static void send_ble_data_thread()
{
    uint8_t sensors_data[CONFIG_BLE_DATA_LENGHT] = {0U};
    callback_ptr* cb = NULL;
    uint8_t data_lenght = 0;
    util_get_cb_vector(&cb);
    for(;;)
    {
        for(uint8_t sensor_data_index = 0; sensor_data_index < util_get_number_of_callbacks(); sensor_data_index++)
        {
            data_lenght += cb[sensor_data_index](&sensors_data[data_lenght]);
        }
        ble_get_env_data(&sensors_data[0]);
        data_lenght=0;

        thread_worker_sleep_request(THREAD_SLEEP_MS);
    }
}
#if IS_ENABLED(CONFIG_UART_REPORT_ENABLE)
static void send_uart_thread()
{
    for(;;)
    {
        uart_report_worker();
        thread_worker_sleep_request(THREAD_SLEEP_MS);
    }
}
#endif

void thread_worker_sleep_request(uint32_t ms_time)
{
    k_msleep(ms_time);
}

K_THREAD_DEFINE(init_sensors_thread_id, STACKSIZE, init_sensors_thread, NULL, NULL, NULL, INIT_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(sensor_worker_thread_id, STACKSIZE, sensor_worker_thread, NULL, NULL, NULL,	WORKER_THREAD_PRIORITY, 0, 2000);
K_THREAD_DEFINE(send_ble_data_thread_id, STACKSIZE, send_ble_data_thread, NULL, NULL, NULL,	BLE_THREAD_PRIORITY, 0, 3000);
#if IS_ENABLED(CONFIG_UART_REPORT_ENABLE)
K_THREAD_DEFINE(send_uart_thread_id, STACKSIZE, send_uart_thread, NULL, NULL, NULL,	UART_REPORT_PRIORITY, 0, 1000);
#endif
