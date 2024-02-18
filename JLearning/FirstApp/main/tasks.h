#ifndef CONFIG_TASK
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
#include "freertos/task.h"  //Librería para las Tareas

#define R_FIXED 10000             // Valor de resistencia adicional que se pone para hacer el valor de tensión con la NTC
#define R0_NTC 10000              // Valor de NTC a 25°C
#define Beta 4190                 // Factor Beta de la NTC
#define Temp0 298.15              // Valor de temperatura a temperatura ambiente, en °Kelvin. (25°C+273.15=298.15°K)
#define Vol_REF 3.3               // Voltaje aplicado al divisor de tensión
#define SIZE_BUFFER_TASK 1024 * 2 // valor de espacio de memoria para las tareas (si se pone un valor muy pequeño se va a reiniciar el uC)
#define Delay_promedio 500

static const char *tag_task = "Task"; // Variable utilzada para etiquetar con la palabra "Task" el mensaje enviado por medio de ESP_LOG()

// Prototipado de funciones
esp_err_t create_task(void);
void get_ADC(void *pvParameters);
void Promedio_temp(void *pvParameters);

#endif