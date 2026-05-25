#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include<zephyr/bluetooth/bluetooth.h>
#include<zephyr/bluetooth/gap.h>

#include "ble_beacon.h"


#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define BT_DATA_SOMETHING_DATA 0xA5

static struct bt_le_ext_adv* adv;

/*static unsigned char url_data[] ={0x17,'/','/','a','c','a','d','e','m','y','.',
                                 'n','o','r','d','i','c','s','e','m','i','.',
                                 'c','o','m'};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_URI, url_data,sizeof(url_data)),    
};*/

static const struct bt_le_adv_param adv_param =
	BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_EXT_ADV, /* No options specified */
			800, /* Min Advertising Interval 500ms (800*0.625ms) */
			801, /* Max Advertising Interval 10.23sec (16000*0.625ms) */
			NULL); /* Set to NULL for undirected advertising */

#define COMPANY_ID_CODE 0x0059

typedef struct adv_mfg_data {
	uint16_t company_code; /* Company Identifier Code. */
} adv_mfg_data_type;

static adv_mfg_data_type adv_mfg_data = { COMPANY_ID_CODE };


static uint8_t environment_mesurements[16] = {0x00};

static struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data)),
    BT_DATA(BT_DATA_SOMETHING_DATA, environment_mesurements, sizeof(environment_mesurements)),
};

void ble_get_env_data(uint8_t* env_data)
{
	memcpy(environment_mesurements,env_data,ARRAY_SIZE(environment_mesurements));
	int error = bt_le_ext_adv_set_data(adv,ad,ARRAY_SIZE(ad),NULL,0);

	if(error)
	{
		printk("Error %d", error);
	}
}

void ble_init()
{
    int error = bt_enable(NULL);

    if (error)
    {
    #if IS_ENABLED(CONFIG_PRINTK)
        printk("Nu sa pornit bluethooth: %d", error);
    #endif
    }

	error = bt_le_ext_adv_create(&adv_param,NULL,&adv);

	if(error)
	{
    #if IS_ENABLED(CONFIG_PRINTK)
		printk("Adv create naspa ceva %d", error);
    #endif
	}

    error = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);

    if (error)
    {
    #if IS_ENABLED(CONFIG_PRINTK)
        printk("Adv naspa ceva %d", error);
    #endif
    }

	error = bt_le_ext_adv_start(adv,NULL);

	if (error)
    {
    #if IS_ENABLED(CONFIG_PRINTK)
        printk("Adv naspa ceva start %d", error);
    #endif
    }
}

