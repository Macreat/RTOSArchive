#include <stdio.h>
#include <stdbool.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/ledc.h"
//  Definición de pines
#define Potenciometro ADC1_CHANNEL_7 // Pin
#define Pulsador 18                  // Pin
#define Led1 25                      // Pin
#define Led2 26                      // Pin
#define Led3 33                      // Pin

// Definición de constantes
#define NUM_LEDS 3
#define PWM_FREQ 5000                   // Frecuencia PWM (en Hz)
#define PWM_RESOLUTION LEDC_TIMER_8_BIT // Resolución del PWM (8 bits para LEDC_TIMER_8_BIT)
#define PWM_CHANNEL LEDC_CHANNEL_0      // Canal PWM utilizado (0 a 7 para LEDC_TIMER_8_BIT)
#define portTICK_PERIOD_MS 10           // Período de tiempo del tick en milisegundos (cambiado a 10ms)

// Variables globales
int led_actual = 0;
bool encendido[NUM_LEDS] = {false, false, false};
int luminosidad[NUM_LEDS] = {0, 0, 0};

void configurar_pines()
{
    gpio_set_direction(Pulsador, GPIO_MODE_INPUT);
    gpio_set_direction(Led1, GPIO_MODE_OUTPUT);
    gpio_set_direction(Led2, GPIO_MODE_OUTPUT);
    gpio_set_direction(Led3, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(Pulsador, GPIO_PULLUP_ONLY);
}

void configurar_adc()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(Potenciometro, ADC_ATTEN_DB_11);
}

void configurar_pwm()
{
    // Configuración del canal PWM
    ledc_timer_config_t timer_conf = {
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0};
    ledc_timer_config(&timer_conf);

    // Configuración de los canales PWM
    for (int i = 0; i < NUM_LEDS; i++)
    {
        ledc_channel_config_t ledc_conf = {
            .gpio_num = Led1 + i,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .channel = LEDC_CHANNEL_0 + i,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0};
        ledc_channel_config(&ledc_conf);
    }
    ledc_channel_config_t ledc_conf_3 = {
        .gpio_num = Led3,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_2, // Usar el siguiente canal disponible
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0};
    ledc_channel_config(&ledc_conf_3);
}

void cambiar_led()
{
    // Apagar todos los LEDs
    for (int i = 0; i < NUM_LEDS; i++)
    {
        gpio_set_level(Led1 + i, 0);
        encendido[i] = false;
    }

    // Encender el LED actual
    gpio_set_level(Led1 + led_actual, 1);
    encendido[led_actual] = true;
}
void boton_task(void *pvParameter)
{
    int estado_anterior = 1; // Estado anterior del botón (1 = no presionado, 0 = presionado)
    int contador = 0;        // Contador para contar las veces que se presiona el botón

    while (1)
    {
        int estado_pulsador = gpio_get_level(Pulsador);

        if (estado_pulsador != estado_anterior)
        {
            vTaskDelay(20 / portTICK_PERIOD_MS); // Esperar un corto período para evitar rebotes

            estado_pulsador = gpio_get_level(Pulsador); // Leer el estado nuevamente

            if (estado_pulsador != estado_anterior) // Verificar si el estado es estable
            {
                if (estado_pulsador == 0) // Si el botón está presionado
                {
                    contador++;

                    if (contador >= 4) // Si se presiona tres veces, reiniciar el ciclo
                    {
                        contador = 0;
                        led_actual = 0;
                        cambiar_led();
                    }
                    else // Cambiar al siguiente LED si no se ha presionado tres veces
                    {
                        led_actual = (led_actual + 1) % NUM_LEDS;
                        cambiar_led();
                    }
                }
            }
        }

        estado_anterior = estado_pulsador; // Actualizar el estado anterior del botón
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
void actualizar_luminosidad(int valor_potenciometro)
{
    int brillo_objetivo = (valor_potenciometro * 255) / 4095;
    int brillo_actual = encendido[led_actual] ? luminosidad[led_actual] : 0;
    int paso = 0;
    int total_pasos = 50;

    // Suaviza la transición de brillo con 50 pasos
    while (paso <= total_pasos)
    {
        int brillo_interpolado = (brillo_actual * (total_pasos - paso) + brillo_objetivo * paso) / total_pasos;
        luminosidad[led_actual] = brillo_interpolado;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0 + led_actual, brillo_interpolado);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0 + led_actual);
        paso++;
        vTaskDelay(10 / portTICK_PERIOD_MS); // Pequeña pausa entre pasos para suavizar la transición
    }
}

void app_main(void)
{
    configurar_pines();
    configurar_adc();
    configurar_pwm();
    cambiar_led(); // Enciende el primer LED al inicio

    xTaskCreate(boton_task, "boton_task", 2048, NULL, 10, NULL);

    while (1)
    {
        int valor_potenciometro = adc1_get_raw(Potenciometro);
        actualizar_luminosidad(valor_potenciometro);

        ESP_LOGI("main", "Valor del potenciómetro: %d", valor_potenciometro);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
