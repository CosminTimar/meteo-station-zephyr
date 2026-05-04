#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include "bme_sensor/bme_sensor.h"



#define SLEEP_TIME_MS 1000

#define STACKSIZE 1024
#define WORQ_THREAD_STACK_SIZE 512
#define THREAD0_PRIORITY 4 
#define THREAD1_PRIORITY 4
#define WORKQ_PRIORITY   4

#define PRODUCER_PRIORITY        5 
#define CONSUMER_PRIORITY        5

/* Mutex needed*/
#define COMBINED_TOTAL   40
int32_t increment_count = 0; 
int32_t decrement_count = COMBINED_TOTAL; 
K_MUTEX_DEFINE(test_mutex);



volatile uint32_t available_instance_count = 10;



struct work_info {
    struct k_work work;
    char name[25];
} my_work;

static struct k_work_q offload_work_q = {0};

static K_THREAD_STACK_DEFINE(my_stack_area, WORQ_THREAD_STACK_SIZE);

K_SEM_DEFINE(instance_monitor_sem, 10, 10);


static inline void emulate_work()
{
	for(volatile int count_out = 0; count_out < 300000; count_out ++);
}


void offload_function(struct k_work *work_term)
{
	emulate_work();
}

void shared_code_section(void)
{
	uint8_t race_condition = 0;
	int32_t increment_count_copy = 0;
	int32_t decrement_count_copy = 0;

	k_mutex_lock(&test_mutex, K_FOREVER);
	increment_count += 1;
	increment_count = increment_count % COMBINED_TOTAL; 

	decrement_count -= 1;
	if (decrement_count == 0) 
	{
		decrement_count = COMBINED_TOTAL;
	}

	if (increment_count + decrement_count != COMBINED_TOTAL) {
        race_condition = 1;

		increment_count_copy = increment_count;
        decrement_count_copy = decrement_count;
    }
	k_mutex_unlock(&test_mutex);
    
    if( race_condition ){
        printk("Race condition happend!\n");
        printk("Increment_count (%d) + Decrement_count (%d) = %d \n", increment_count_copy,
            decrement_count_copy, (increment_count_copy + decrement_count_copy));
        k_msleep(400 + sys_rand32_get() % 10);
    }
}


void thread0(void)
{
	/*uint64_t time_stamp;
    int64_t delta_time;

	k_work_queue_start(&offload_work_q, my_stack_area,
                   K_THREAD_STACK_SIZEOF(my_stack_area), WORKQ_PRIORITY,
                   NULL);

	strcpy(my_work.name, "Thread0 emulate_work()");
	k_work_init(&my_work.work, offload_function);

    while (1) {
        time_stamp = k_uptime_get();
        k_work_submit_to_queue(&offload_work_q, &my_work.work);
		delta_time = k_uptime_delta(&time_stamp);
		printk("thread0 yielding this round in %lld ms\n", delta_time);
		k_msleep(20);
    }   */

	printk("Thread 0 started\n");
	while (1) {
		shared_code_section(); 
	}  
}

void thread1(void)
{
	/*uint64_t time_stamp;
    int64_t delta_time;

    while (1) {
        time_stamp = k_uptime_get();
        emulate_work();
        delta_time = k_uptime_delta(&time_stamp);

        printk("thread1 yielding this round in %lld ms\n", delta_time);
        k_msleep(20);
    } */
   	printk("Thread 1 started\n");
	while (1) {
		shared_code_section(); 
	}  
}

void get_access(void)
{
	k_sem_take(&instance_monitor_sem, K_FOREVER);
	printk("Resource taken and available_instance_count = %d\n", k_sem_count_get(&instance_monitor_sem));
}

void release_access(void)
{
	k_sem_give(&instance_monitor_sem);
	printk("Resource given and available_instance_count = %d\n", k_sem_count_get(&instance_monitor_sem));
}

void consumer(void)
{
	printk("Consumer thread started\n");
	while (1) {
		get_access();
		k_msleep((sys_rand32_get() % 10) * 1000);
	}
}

void producer(void)
{
	printk("Producer thread started\n");
	while (1) {
		release_access();
		k_msleep((sys_rand32_get() % 10) * 1000);
	}
}

//K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL, THREAD0_PRIORITY, 0, 5000);
//K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL,	THREAD1_PRIORITY, 0, 5000);


#include<zephyr/drivers/adc.h>

static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));

static int16_t sample_buffer;

static struct adc_sequence sequance = {
	.buffer = &sample_buffer,
	.buffer_size = sizeof(sample_buffer),
};

volatile uint8_t error;

static float voltage_to_uv(float voltage_mv)
{
    float v = voltage_mv / 1000.0f;
    if (v < 1.0f) return 0.0f;
    /* Linear map: 1.0V -> 0, 2.8V -> 15 mW/cm² */
    float uv = (v - 1.0f) * (15.0f / (2.8f - 1.0f));
    if (uv < 0.0f) uv = 0.0f;
    return uv;
}

int main(void)
{

	//bme_init();
	
	error = adc_is_ready_dt(&adc_channel);

	if(1 != error)
	{
		return -1;
	}

	error = adc_channel_setup_dt(&adc_channel);

	if(0 != error)
	{
		return -1;
	}

	error = adc_sequence_init_dt(&adc_channel, &sequance);
	if(0 != error)
	{
		return -1;
	}

	while(1)
	{
		error = adc_read(adc_channel.dev, &sequance);
		if(0 != error)
		{
			return -1;
		}

		int32_t val_mv = sample_buffer;

		error = adc_raw_to_millivolts_dt(&adc_channel, &val_mv);

		if(0 > error)
		{
			return -1;
		}

		float uv_intensity = voltage_to_uv((float)val_mv);

		printk("MV from sensor : %.2f \n", uv_intensity);
	}

	

	return 0;
}
