#ifndef UART_REPORT_HEADER
#define UART_REPORT_HEADER


void uart_report_init(void);

void uart_report_add_error(uint8_t error);

void uart_report_worker();

#endif