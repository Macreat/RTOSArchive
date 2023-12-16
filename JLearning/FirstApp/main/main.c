// Libraries to use

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/timers.h"
#include "driver/adc.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_intr_alloc.h"
#include "driver/ledc.h"

// Definition of GPIO ports

#define Pot ADC1_CHANNEL_7
#define button 19
#define led1 25
#define led2 26
#define led3 33
#define cled 2
#define stackSize 1024
#define RDelay 500
#define NLeds 3
#define PwmFreq 5000
#define PwmResolution LEDC_TIMER_8_BIT
#define PwmChannel LEDC_CHANNEL_0

QueueHandle_t buttonQueue = 0;

// global variable

static const char *TAG = "Button Task";
int luminosity[NLeds] = {false, false, false, false};
bool turnOn[NLeds] = {0, 0, 0, 0};
int currentLed = 0; // to save actual led state

// prototype functions
void setPins(void);
esp_err_t initIsr(void);
void setADC(void);
void setPWM(void);
void changeLed(void);
void buttonTask(void *pvParameters);
void setLuminosity(int potValue);
void IRAM_ATTR buttonIsrHandler(void *arg);
/*
function to set interruption
*/
void IRAM_ATTR buttonIsrHandler(void *arg)
{
    int buttonState = gpio_get_level(button);
    xQueueSendFromISR(buttonQueue, &buttonState, NULL);
}
/*
function to set pins
*/

void SetPins()
{
    gpio_set_direction(button, GPIO_MODE_INPUT);
    gpio_set_pull_mode(button, GPIO_PULLUP_ONLY);
    gpio_set_direction(led1, GPIO_MODE_OUTPUT);
    gpio_set_direction(led2, GPIO_MODE_OUTPUT);
    gpio_set_direction(led3, GPIO_MODE_OUTPUT);
    gpio_set_direction(cled, GPIO_MODE_OUTPUT);
}
/*
function to configure ADC
*/
void setADC()
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(Pot, ADC_ATTEN_DB_11);
}
/*
function to configure PWM Channel
*/
void setPWM()
{
    // Configuración del canal PWM
    ledc_timer_config_t timer_conf = {
        .duty_resolution = PwmResolution,
        .freq_hz = PwmFreq,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0};
    ledc_timer_config(&timer_conf);

    // Configuración de los canales PWM
    for (int i = 0; i < NLeds; i++)
    {
        ledc_channel_config_t ledc_conf = {
            .gpio_num = led1 + i,
            .speed_mode = LEDC_HIGH_SPEED_MODE,
            .channel = LEDC_CHANNEL_0 + i,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0};
        ledc_channel_config(&ledc_conf);
    }
    ledc_channel_config_t ledc_conf_3 = {
        .gpio_num = led3,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_2, // Use another channel avaliable
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0};
    ledc_channel_config(&ledc_conf_3);
}
/*
function to use interruption
*/
/*
void turnOnIndicatorLed(int pin, TickType_t delayTime)
{
    gpio_set_level(pin, 1);
    vTaskDelay(delayTime);
    gpio_set_level(pin, 0);
}
*/

/*
function in charge to change between leds
*/
void changeLed()
{
    // turn off leds
    for (int i = 0; i < NLeds; i++)
    {
        gpio_set_level(led1 + i, 0);
        turnOn[i] = false;
    }

    // turn on only current led
    gpio_set_level(led1 + currentLed, 1);
    turnOn[currentLed] = true;
}
/*
function to configurate tasks
*/

void buttonTask(void *pvParameter)
{
    int estado_anterior = 1; // Previous button state (1 = not pressed, 0 = pressed)
    int contador = 0;        // counter for button task

    while (1)
    {
        int estado_pulsador = gpio_get_level(button);

        if (estado_pulsador != estado_anterior)
        {
            ESP_LOGI("TAG", "Button press... changing led");

            vTaskDelay(20 / portTICK_PERIOD_MS); // to evade reboots

            estado_pulsador = gpio_get_level(button);

            if (estado_pulsador != estado_anterior) // Check if the status is stable
            {
                if (estado_pulsador == 0) //  If the button is pressed
                {
                    contador++;

                    if (contador >= 4) // If pressed three times, restart the cycle
                    {
                        contador = 0;
                        currentLed = 0;
                        changeLed();
                    }
                    else // Change to next LED if not pressed three times
                    {
                        currentLed = (currentLed + 1) % NLeds;
                        changeLed();
                    }
                }
            }
        }

        estado_anterior = estado_pulsador; // Update previous button state
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
/*
function to set luminosity in current led
*/
void setLuminosity(int potValue)
{
    int brillo_objetivo = (potValue * 255) / 4095;
    int brillo_actual = turnOn[currentLed] ? luminosity[currentLed] : 0;
    int paso = 0;
    int total_pasos = 50;

    // Smooth the shine transition with 50 steps
    while (paso <= total_pasos)
    {
        int brillo_interpolado = (brillo_actual * (total_pasos - paso) + brillo_objetivo * paso) / total_pasos;
        luminosity[currentLed] = brillo_interpolado;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0 + currentLed, brillo_interpolado);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0 + currentLed);
        paso++;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
// main app
void app_main(void)
{
    setPins();
    setADC();
    setPWM();
    changeLed(); // turn on first led

    xTaskCreate(buttonTask, "boton_task", 2048, NULL, 10, NULL);

    while (1)
    {
        int valor_potenciometro = adc1_get_raw(Pot);
        setLuminosity(valor_potenciometro);

        ESP_LOGI("main", "pottentiometer value: %d", valor_potenciometro);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}