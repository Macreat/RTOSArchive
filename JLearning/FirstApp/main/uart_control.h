#ifndef UART_CONTROL
#define UART_CONTROL

#include <string.h>
#include <time.h>     //Contiene funciones para trabajar con el tiempo
#include <sys/time.h> //Contiene funciones para trabajar con el tiempo de forma más baja
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"

#define UART_NUM UART_NUM_2 // Define el número de puerto UART que se utilizará
#define BUF_SIZE 1024       // Se define el tamaño del buffer para RX y TX del UART

static const char *tag = "UART EVENT";

// Prototipado de fucniones
void send_time_UART(void);
void init_uart(void);

#endif