#include <zephyr/kernel.h>
#include <zephyr/random/random.h>

#include "thread_worker.h"
#include "util.h"
#include "bme_sensor.h"
#include "cjmcu_sensor.h"
#include "ml_sensor.h"
#include "ble_beacon.h"

#define STACKSIZE 1024
#define INIT_THREAD_PRIORITY 1 
#define THREAD1_PRIORITY 2


void init_sensors_thread()
{
    bme_init();

    ml_init();

    ble_init();

    cjmcu_init();
}

void sensor_worker_thread()
{
    bme_worker();

    k_msleep(CJMCU_START_MEASURE_READING_TIME);
    cjmcu_worker();

    ml_worker();
}

void send_ble_data_thread()
{
    
}

void send_uart_thread()
{

}

K_THREAD_DEFINE(init_sensors_thread_id, STACKSIZE, init_sensors_thread, NULL, NULL, NULL, INIT_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(sensor_worker_thread_id, STACKSIZE, sensor_worker_thread, NULL, NULL, NULL,	THREAD1_PRIORITY, 0, 5000);
K_THREAD_DEFINE(send_ble_data_thread_id, STACKSIZE, send_ble_data_thread, NULL, NULL, NULL,	THREAD1_PRIORITY, 0, 5000);
K_THREAD_DEFINE(send_uart_thread_id, STACKSIZE, send_uart_thread, NULL, NULL, NULL,	THREAD1_PRIORITY, 0, 5000);




void worker_thread()
{
    k_thread_start(init_sensors_thread_id);

    for(;;)
    {
        k_thread_start(sensor_worker_thread_id);
    }
}