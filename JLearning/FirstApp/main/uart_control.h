#ifndef UART_CONTROL
#define UART_CONTROL

#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"

#define UART_NUM UART_NUM_2
#define BUF_SIZE 1024

static const char *tag = "UART EVENT";

void send_time_UART(void);
esp_err_t init_uart(void);
void update_leds_from_uart(void);
bool uart_control_send_rgb_command(const char *command);

#endif