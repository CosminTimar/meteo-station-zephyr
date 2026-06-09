#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include "uart_report.h"
#include "util.h"

#define UART_PAYLOAD_LENGHT (CONFIG_NUMBER_OF_CB + UTIL_BLE_ERRORS)

const struct device *uart0 = DEVICE_DT_GET(DT_NODELABEL(uart0));

static uint8_t uart_payload[UART_PAYLOAD_LENGHT] = {0U};

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
    switch (evt->type)
    {

        case UART_TX_DONE:
        {
        #if IS_ENABLED(CONFIG_PRINTK)
            printk("TX done\n");
        #endif
            break;
        }

        case UART_TX_ABORTED:
        {
        #if IS_ENABLED(CONFIG_PRINTK)
            printk("TX aborted\n");
            break;
        #endif
        }

        default:
        {
            break;
        }
    }
}

void uart_report_add_error(uint8_t error)
{
    static uint8_t error_index = 0;
    uart_payload[error_index] = error;

    if( error_index <= UART_PAYLOAD_LENGHT)
    {
        error_index++;
    }
    else
    {
        error_index = 0;
    }
    
}

void uart_report_worker()
{
    int error = uart_tx(uart0,uart_payload,UART_PAYLOAD_LENGHT,100);

    if(0 != error)
    {
    #if IS_ENABLED(CONFIG_PRINTK)
        printk("Error: %d\n", error);
    #endif
    }
}


void uart_report_init()
{
    if (!device_is_ready(uart0)) 
    {
    #if IS_ENABLED(CONFIG_PRINTK)
        printk("UART device not ready\n");
    #endif
    }

    uart_callback_set(uart0, uart_cb, NULL);
}