#ifndef CONFIG_PERIPHERALS
#define CONFIG_PERIPHERALS

#include <stdio.h>
#include "driver/adc.h" //Librería para configurar y leer por medio de ADC
#include "esp_log.h"    //librería para poder imprimir texto con colores diferentes
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h" //Librería para las colas
#include "driver/uart.h"    //Librería para puertos UART

#define NTC_PIN ADC_CHANNEL_7 // Canal ocupado para la lectura ADC
#define UART_NUM UART_NUM_2   // Puerto UART utilizado
#define UART_TX 17            // GPIO pin TX UART
#define UART_RX 16            // GPIO pin RX UART
#define size_queue_UART 5     // Tamaño de cola para almacenar envento UART
#define BUF_SIZE 1024         // Tamaño de memoria para datos de entrada y salida
#define ledUART 2             // GPIO LED indicador de lectura UART (led amarillo)
#define ledR 33               // GPIO LED rojo
#define ledG 25               // GPIO LED verde
#define ledB 26               // GPIO LED azul

static const char *tag_ADC = "ADC";         // Variable utilzada para etiquetar con la palabra "ADC" el mensaje enviado por medio de ESP_LOG()
static const char *tag_UART = "UART EVENT"; // Variable utilzada para etiquetar con la palabra "UART EVENT" el mensaje enviado por medio de ESP_LOG()
static const char *tag_LED = "LEDs";        // Variable utilzada para etiquetar con la palabra "LEDs" el mensaje enviado por medio de ESP_LOG()

// Prototipado de funciones
esp_err_t set_adc(void);
esp_err_t init_uart(void);
esp_err_t init_led(void);

#endif