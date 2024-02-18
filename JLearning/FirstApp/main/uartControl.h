
#ifndef UART_CONTROL
#define UART_CONTROL
#include "wifiControl.h"
#include "driver/uart.h"

#define UART_NUM UART_NUM_2 // Defining uart channel for comumunication d
#define BUF_SIZE 1024       // defining buffer size for RX and TX

static const char *tag = "UART EVENT";
// function prototype
void send_time_UART(void);
void init_uart(void);

#endif