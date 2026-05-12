#include <zephyr/device.h>
#include <zephyr/kernel.h>

#include<zephyr/bluetooth/bluetooth.h>
#include<zephyr/bluetooth/gap.h>
#include <dk_buttons_and_leds.h>

#include "ble_beacon.h"


#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define USER_BUTTON DK_BTN1_MSK

#define BT_DATA_SOMETHING_DATA 0xA5


static unsigned char url_data[] ={0x17,'/','/','a','c','a','d','e','m','y','.',
                                 'n','o','r','d','i','c','s','e','m','i','.',
                                 'c','o','m'};

static const struct bt_data sd[] = {
    BT_DATA(BT_DATA_URI, url_data,sizeof(url_data)),    
};

static const struct bt_le_adv_param *adv_param =
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_NONE, /* No options specified */
			800, /* Min Advertising Interval 500ms (800*0.625ms) */
			801, /* Max Advertising Interval 500.625ms (801*0.625ms) */
			NULL); /* Set to NULL for undirected advertising */

#define COMPANY_ID_CODE 0x0059

typedef struct adv_mfg_data {
	uint16_t company_code; /* Company Identifier Code. */
	uint16_t number_press; /* Number of times Button 1 is pressed */
} adv_mfg_data_type;

static adv_mfg_data_type adv_mfg_data = { COMPANY_ID_CODE, 0x00 };

typedef struct some_data{
    uint16_t tare;
}some_data_type;

static some_data_type adv_data_Data = { 0xA5};

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data)),
    BT_DATA(BT_DATA_SOMETHING_DATA, (unsigned char*)& adv_data_Data, sizeof(adv_data_Data)),
};

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	if (has_changed & button_state & USER_BUTTON) {
		adv_mfg_data.number_press += 1;
		bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	}
}


static int init_button(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err) {
		printk("Cannot init buttons (err: %d)\n", err);
	}

	return err;
}


void ble_init()
{
    int error = dk_leds_init();
	if (error) {
		printk("LEDs init failed (err %d)\n", error);
	}
	/* STEP 4.2 - Setup buttons on your board  */
	error = init_button();
	if (error) {
		printk("Button init failed (err %d)\n", error);
	}

    error = bt_enable(NULL);

    if (error)
    {
        printk("Nu sa pornit bluethooth: %d", error);
    }

    error = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

    if (error)
    {
        printk("Adv naspa ceva %d", error);
    }

    

}

