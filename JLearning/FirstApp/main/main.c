// Libraries to use

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Definition of GPIO ports

#define button 19
#define led1 33
#define led2 25
#define led3 26
#define ledOFF 0
#define ledON 1
#define cled 2

// local variable
int currentLed = 0; // Variable para mantener el estado actual del LED
/*
function to set pins
*/
void SetPines()
{
    gpio_set_direction(button, GPIO_MODE_INPUT);
    gpio_set_pull_mode(button, GPIO_PULLUP_ONLY);
    gpio_set_direction(led1, GPIO_MODE_OUTPUT);
    gpio_set_direction(led2, GPIO_MODE_OUTPUT);
    gpio_set_direction(led3, GPIO_MODE_OUTPUT);
    gpio_set_direction(cled, GPIO_MODE_OUTPUT);
}
/*
function in charge to blink the LED (dosnt work)
*/
void blinkLed(int pin)
{
    bool blinking = true;
    while (blinking)
    {
        int estadoButton = gpio_get_level(button);
        gpio_set_level(pin, ledON);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(pin, ledOFF);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if (estadoButton == 0)
        {
            blinking = false;
        }
    }
}
/*
to control the interruption of my button
*/
void turnOnIndicatorLed(int pin, TickType_t delayTime)
{
    gpio_set_level(pin, ledON);
    vTaskDelay(delayTime);
    gpio_set_level(pin, ledOFF);
}
/*
function to change between leds
 */
void changeLed(void *pvParameters)
{
    while (true)
    {
        // Cambia al siguiente LED cuando se presiona el botón
        int estadoButton = gpio_get_level(button);
        if (estadoButton == 0)
        {
            // Enciende el indicador LED durante 2 segundos
            turnOnIndicatorLed(cled, pdMS_TO_TICKS(1000));

            // Espera un poco para evitar múltiples detecciones del botón
            vTaskDelay(pdMS_TO_TICKS(100));

            // Cambia al siguiente LED
            currentLed = (currentLed + 1) % 3;
        }

        // Enciende el LED correspondiente
        switch (currentLed)
        {
        case 0:
            blinkLed(led1);
            break;
        case 1:
            blinkLed(led2);
            break;
        case 2:
            blinkLed(led3);
            break;
        }
    }
}
// main app
void app_main(void)
{
    SetPines();
    xTaskCreate(changeLed, "change_led_task", 2048, NULL, 10, NULL);
}
