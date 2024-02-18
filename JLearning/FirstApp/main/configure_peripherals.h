#ifndef CONFIG_PERIPHERALS
#define CONFIG_PERIPHERALS

#include <stdio.h>
#include "driver/adc.h" //Librería para configurar y leer por medio de ADC
#include "esp_log.h"    //librería para poder imprimir texto con colores diferentes
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h" //Librería para las colas

#define NTC_PIN ADC1_CHANNEL_7 // Canal ocupado para la lectura ADC

static const char *tag_ADC = "ADC"; // Variable utilzada para etiquetar con la palabra "ADC" el mensaje enviado por medio de ESP_LOG()

// Prototipado de funciones
esp_err_t set_adc(void);

#endif