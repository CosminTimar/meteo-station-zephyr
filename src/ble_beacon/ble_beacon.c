#include<zephyr/bluetooth/bluetooth.h>
#include<zephyr/bluetooth/gap.h>

#include "ble_beacon.h"
#include "uart_report.h"


#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define BT_DATA_SOMETHING_DATA      (0xA5U)
#define COMPANY_ID_CODE             (0x0059)

static struct bt_le_ext_adv* adv;

static const struct bt_le_adv_param adv_param =
	BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_EXT_ADV, /* No options specified */
			800  , /* Min Advertising Interval 500ms (800*0.625ms) */
			801, /* Max Advertising Interval ~500ms (801*0.625ms) */
			NULL); /* Set to NULL for undirected advertising */

typedef struct adv_mfg_data {
	uint16_t company_code; /* Company Identifier Code. */
} adv_mfg_data_type;

static adv_mfg_data_type adv_mfg_data = { COMPANY_ID_CODE };
static uint8_t environment_mesurements[CONFIG_BLE_DATA_LENGHT] = {0x00};
static uint8_t ble_error;

static struct bt_data bt_extended_adv[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
    BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data)),
    BT_DATA(BT_DATA_SOMETHING_DATA, environment_mesurements, sizeof(environment_mesurements)),
};

void ble_get_env_data(uint8_t* env_data)
{
	memcpy(environment_mesurements,env_data,ARRAY_SIZE(environment_mesurements));
	int error = bt_le_ext_adv_set_data(adv,bt_extended_adv,ARRAY_SIZE(bt_extended_adv),NULL,0);

	if(BLE_NO_ERROR != error)
	{
        ble_error |= 1 << BLE_SET_DATA_ERROR;
    #if IS_ENABLED(CONFIG_UART_REPORT_ENABLE)
        uart_report_add_error(ble_error);
    #endif
		printk("BLE Advertising set of data failed: %d", error);
	}
}

void ble_init()
{
    ble_error = BLE_NO_ERROR;
    int error = bt_enable(NULL);

    if (BLE_NO_ERROR != error)
    {
        ble_error |= 1 << BLE_ENABLE_ERROR;
    #if IS_ENABLED(CONFIG_PRINTK)
        printk("BLE did not started: %d", error);
    #endif
    }

	error = bt_le_ext_adv_create(&adv_param,NULL,&adv);

	if(BLE_NO_ERROR != error)
	{
        ble_error |= 1 << BLE_ADV_CREATE_ERROR;
    #if IS_ENABLED(CONFIG_PRINTK)
		printk("BLE Advertaising creation failed: %d", error);
    #endif
	}

    error = bt_le_ext_adv_set_data(adv, bt_extended_adv, ARRAY_SIZE(bt_extended_adv), NULL, 0);

    if (BLE_NO_ERROR != error)
    {
        ble_error |= 1 << BLE_SET_DATA_ERROR;
    #if IS_ENABLED(CONFIG_PRINTK)
        printk("BLE Advertising setup failed: %d", error);
    #endif
    }

	error = bt_le_ext_adv_start(adv,NULL);

	if (BLE_NO_ERROR != error)
    {
        ble_error |= 1 << BLE_ADV_START_ERROR;
    #if IS_ENABLED(CONFIG_PRINTK)
        printk("BLE Advertising start failed: %d", error);
    #endif
    }

    if(BLE_NO_ERROR != ble_error)
    {
    #if IS_ENABLED(CONFIG_UART_REPORT_ENABLE)
        uart_report_add_error(ble_error);
    #endif
    }
}

