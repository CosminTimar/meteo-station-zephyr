#include <zephyr/kernel.h>
#include <zephyr/random/random.h>

#include "thread_worker.h"
#include "util.h"
#include "bme_sensor.h"
#include "cjmcu_sensor.h"
#include "ml_sensor.h"
#include "ble_beacon.h"
#include "mh_rd.h"
#include "uart_report.h"

/* Stack size for threads */
#define STACKSIZE               (1024)
/* Defines for thread prio */
#define INIT_THREAD_PRIORITY    (1) 
#define BLE_THREAD_PRIORITY     (2)
#define WORKER_THREAD_PRIORITY  (3)
#define UART_REPORT_PRIORITY    (4)

#define BLE_DATA_LENGHT         (16U)

#define THREAD_SLEEP_MS         (1000U)

static void init_sensors_thread(void);
static void send_ble_data_thread(void);

static void init_sensors_thread()
{
    ble_init();

    bme_init();

    ml_init();

    rain_sensor_init();

    cjmcu_init();

    uart_report_init();
}

static void sensor_worker_thread()
{
    for(;;)
    {
        bme_worker();

        cjmcu_worker();

        ml_worker();

        rain_worker();
    }
}

static void send_ble_data_thread()
{
    uint8_t sensors_data[BLE_DATA_LENGHT] = {0U};
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

static void send_uart_thread()
{
    for(;;)
    {
        uart_report_worker();
        thread_worker_sleep_request(THREAD_SLEEP_MS);
    }
}

void thread_worker_sleep_request(uint32_t ms_time)
{
    k_msleep(ms_time);
}

K_THREAD_DEFINE(init_sensors_thread_id, STACKSIZE, init_sensors_thread, NULL, NULL, NULL, INIT_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(sensor_worker_thread_id, STACKSIZE, sensor_worker_thread, NULL, NULL, NULL,	WORKER_THREAD_PRIORITY, 0, 2000);
K_THREAD_DEFINE(send_ble_data_thread_id, STACKSIZE, send_ble_data_thread, NULL, NULL, NULL,	BLE_THREAD_PRIORITY, 0, 3000);
K_THREAD_DEFINE(send_uart_thread_id, STACKSIZE, send_uart_thread, NULL, NULL, NULL,	UART_REPORT_PRIORITY, 0, 1000);
