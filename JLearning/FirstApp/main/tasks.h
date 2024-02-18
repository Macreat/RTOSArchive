#ifndef CONFIG_TASK #
#define CONFIG_TASK

#include "configure_peripherals.h" //Se inluye la librería de configuración para no volver a definir las constantes que se reutilizarán
#include <stdio.h>
#include <stdlib.h>
#include <math.h>       //Librería para hacer operaciones matemáticas. En este caso se utilizo para el logaritmo natural
#include <string.h>     //Librería para manejo de cadenas de caracteres
#include "driver/adc.h" //Librería para configurar y leer por medio de ADC
#include "esp_log.h"    //librería para poder imprimir texto con colores diferentes
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h" //Librería para las colas
#include "driver/uart.h"    //Librería para puertos UART
#include "freertos/task.h"  //Librería para las Tareas

#define R_FIXED 100000             // Valor de resistencia adicional que se pone para hacer el valor de tensión con la NTC
#define R0_NTC 100000              // Valor de NTC a 25°C
#define Beta 4190                  // Factor Beta de la NTC
#define Temp0 298.15               // Valor de temperatura a temperatura ambiente, en °Kelvin. (25°C+273.15=298.15°K)
#define Vol_REF 3.3                // Voltaje aplicado al divisor de tensión
#define SIZE_BUFFER_TASK 1024 * 2  // valor de espacio de memoria para las tareas (si se pone un valor muy pequeño se va a reiniciar el uC)
#define Delay_Task_Switch_LED 1000 // Valor de retardo para la tarea de actualizar los LEDs
#define Init_Min_ledB 0            // Valor mínimo de temperatura inicial para el LEB BLUE
#define Init_Max_ledB 28           // Valor máximo de temperatura inicial para el LEB BLUE
#define Init_Min_ledG 28           // Valor mínimo de temperatura inicial para el LEB GREEN
#define Init_Max_ledG 35           // Valor máximo de temperatura inicial para el LEB GREEN
#define Init_Min_ledR 35           // Valor mínimo de temperatura inicial para el LEB RED
#define Init_Max_ledR 50           // Valor máximo de temperatura inicial para el LEB RED

typedef struct
{
    char *key;
    int value;
} Dict_set_values; // Estructua para manejar de una mejor forma la información que llega por el UART

static const char *tag_task = "Task"; // Variable utilzada para etiquetar con la palabra "Task" el mensaje enviado por medio de ESP_LOG()

// Prototipado de funciones
esp_err_t create_task(void);
void get_ADC(void *pvParameters);
void switch_LED(void *pvParameters);
void uart_task(void *pvParameters);

#endif
