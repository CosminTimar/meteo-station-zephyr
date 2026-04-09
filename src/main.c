#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>
#include <zephyr/random/random.h>


#define I2C_NODE DT_NODELABEL(mysensor)

#define CTRLMEAS 0xF4
#define CALIB00	 0x88
#define ID	     0xD0
#define TEMPMSB	 0xFA
#define PRESMSB	 0xF7
#define CHIP_ID  0x60
#define SENSOR_CONFIG_VALUE 0x93
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

/* Data structure to store BME280 data */
struct bme280_data {
	/* Compensation for Temperature */
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
	/* Compensation for Presure*/
	uint16_t dig_p1;
	int16_t dig_p2;
	int16_t dig_p3;
	int16_t dig_p4;
	int16_t dig_p5;
	int16_t dig_p6;
	int16_t dig_p7;
	int16_t dig_p8;
	int16_t dig_p9;
} bmedata;

struct work_info {
    struct k_work work;
    char name[25];
} my_work;

static struct k_work_q offload_work_q = {0};

static K_THREAD_STACK_DEFINE(my_stack_area, WORQ_THREAD_STACK_SIZE);

K_SEM_DEFINE(instance_monitor_sem, 10, 10);

/* Read sensor calibration data and stores these into sensor data */
/*void bme_calibrationdata(const struct i2c_dt_spec *spec, struct bme280_data *sensor_data_ptr)
{
	
	
	uint8_t values[24];

	int ret = i2c_burst_read_dt(spec, CALIB00, values, 24);

	if (ret != 0) {
		printk("Failed to read register %x \n", CALIB00);
		return;
	}

	sensor_data_ptr->dig_t1 = ((uint16_t)values[1]) << 8 | values[0];
	sensor_data_ptr->dig_t2 = ((uint16_t)values[3]) << 8 | values[2];
	sensor_data_ptr->dig_t3 = ((uint16_t)values[5]) << 8 | values[4];
	sensor_data_ptr->dig_p1 = ((uint16_t)values[7]) << 8 | values[6];
	sensor_data_ptr->dig_p2 = ((uint16_t)values[9]) << 8 | values[8];
	sensor_data_ptr->dig_p3 = ((uint16_t)values[11]) << 8 | values[10];
	sensor_data_ptr->dig_p4 = ((uint16_t)values[13]) << 8 | values[12];
	sensor_data_ptr->dig_p5 = ((uint16_t)values[15]) << 8 | values[14];
	sensor_data_ptr->dig_p6 = ((uint16_t)values[17]) << 8 | values[16];
	sensor_data_ptr->dig_p7 = ((uint16_t)values[19]) << 8 | values[18];
	sensor_data_ptr->dig_p8 = ((uint16_t)values[21]) << 8 | values[20];
	sensor_data_ptr->dig_p9 = ((uint16_t)values[23]) << 8 | values[22];

}

int32_t t_fine;

/* Compensate current temperature using previously stored sensor calibration data 
static int32_t bme280_compensate_temp(struct bme280_data *data, int32_t adc_temp)
{
	int32_t var1, var2;

	var1 = (((adc_temp >> 3) - ((int32_t)data->dig_t1 << 1)) * ((int32_t)data->dig_t2)) >> 11;

	var2 = (((((adc_temp >> 4) - ((int32_t)data->dig_t1)) *
		  ((adc_temp >> 4) - ((int32_t)data->dig_t1))) >>
		 12) *
		((int32_t)data->dig_t3)) >>
	       14;

	t_fine = var1 + var2;
	return ((var1 + var2) * 5 + 128) >> 8;
}

/* Compensate current temperature using previously stored sensor calibration data 
static int32_t bme280_compensate_pres(struct bme280_data *data, int32_t adc_pres)
{
	int64_t var1, var2, p;

	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)data->dig_p6;
	var2 = var2 + ((var1*(int64_t)data->dig_p5)<<17);
	var2 = var2 + (((int64_t)data->dig_p4)<<35);
	var1 = ((var1 * var1 * (int64_t)data->dig_p3)>>8) + ((var1 * (int64_t)data->dig_p2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)data->dig_p1)>>33;
	if (var1 == 0)
	{
	return 0; // avoid exception caused by division by zero
	}
	p = 1048576-adc_pres;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((int64_t)data->dig_p9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)data->dig_p8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)data->dig_p7)<<4);

	return (int32_t)p;
}*/

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

K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL,
	THREAD0_PRIORITY, 0, 5000);
K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL,
	THREAD1_PRIORITY, 0, 5000);


int main(void)
{
	/*static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);
	if (!device_is_ready(dev_i2c.bus)) {
		printk("I2C bus %s is not ready!\n\r",dev_i2c.bus->name);
		return -1;
	}

	uint8_t id = 0;
	uint8_t regs[] = {ID};

	int ret = i2c_write_read_dt(&dev_i2c, regs, 1, &id, 1);

	if (ret != 0) {
		printk("Failed to read register %x \n", regs[0]);
		return -1;
	}

	if (id != CHIP_ID) {
		printk("Invalid chip id! %x \n", id);
		return -1;
	}

	bme_calibrationdata(&dev_i2c, &bmedata);

	uint8_t sensor_config[] = {CTRLMEAS, SENSOR_CONFIG_VALUE};

	ret = i2c_write_dt(&dev_i2c, sensor_config, 2);

	if (ret != 0) {
		printk("Failed to write register %x \n", sensor_config[0]);
		return -1;
	}

	while(1)
	{
		uint8_t temp_val[3] = {0};
		int ret = i2c_burst_read_dt(&dev_i2c, TEMPMSB, temp_val, 3);

		if (ret != 0) {
			printk("Failed to read register %x \n", TEMPMSB);
			k_msleep(SLEEP_TIME_MS);
			continue;
		}

		uint8_t press_val[3] = {0};
		ret = i2c_burst_read_dt(&dev_i2c, PRESMSB, press_val, 3);
		if (ret != 0) {
			printk("Failed to read register %x \n", PRESMSB);
			k_msleep(SLEEP_TIME_MS);
			continue;
		}

		int32_t adc_temp =
			(temp_val[0] << 12) | (temp_val[1] << 4) | ((temp_val[2] >> 4) & 0x0F);

		int32_t adc_pres =
			(press_val[0] << 12) | (press_val[1] << 4) | ((press_val[2] >> 4) & 0x0F);

		int32_t comp_temp = bme280_compensate_temp(&bmedata, adc_temp);
		int32_t comp_pres = bme280_compensate_pres(&bmedata, adc_pres);

		float pressure = (float)(comp_pres /256) / 100.0f;

		float temperature = (float)comp_temp / 100.0f;
	
		printk("Temperature in Celsius : %8.2f C\n", (double)temperature);
		printk("Pressure in hPa is : %.2f hPa\n", (double)pressure);

		k_msleep(SLEEP_TIME_MS);
	}*/

	return 0;
}
