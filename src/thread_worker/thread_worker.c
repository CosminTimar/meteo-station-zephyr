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

    //ml_init();

    ble_init();

    //cjmcu_init();
}

void sensor_worker_thread()
{
    bme_worker();

    //k_msleep(CJMCU_START_MEASURE_READING_TIME);
    //cjmcu_worker();

    //ml_worker();
}

void send_ble_data_thread()
{
    uint8_t sensors_data[16] = {0U};
    callback_ptr* cb = NULL;
    uint8_t data_lenght = 0;
    util_get_cb_vector(&cb);
    for(;;)
    {
        for(uint8_t sensor_data_index = 0; sensor_data_index < util_get_number_of_callbacks(); sensor_data_index++)
        {
            data_lenght += cb[sensor_data_index](&sensors_data[data_lenght]);
        }
        k_msleep(1000);
        ble_get_env_data(&sensors_data[0]);
        data_lenght=0;
    }
}

void send_uart_thread()
{

}



K_THREAD_DEFINE(init_sensors_thread_id, STACKSIZE, init_sensors_thread, NULL, NULL, NULL, INIT_THREAD_PRIORITY, 0, 0);
K_THREAD_DEFINE(sensor_worker_thread_id, STACKSIZE, sensor_worker_thread, NULL, NULL, NULL,	THREAD1_PRIORITY, 0, 5000);
K_THREAD_DEFINE(send_ble_data_thread_id, STACKSIZE, send_ble_data_thread, NULL, NULL, NULL,	THREAD1_PRIORITY, 0, 7000);
#if 0
K_THREAD_DEFINE(send_uart_thread_id, STACKSIZE, send_uart_thread, NULL, NULL, NULL,	THREAD1_PRIORITY, 0, 5000);
#endif




void worker_thread()
{
    for(;;)
    {
        //k_thread_start(sensor_worker_thread_id);
    }
}