// Libraries to use
#include <stdio.h>
#include <stdbool.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// Definition of GPIO ports
#define Pulsador 18
#define Led 33
#define LedON 1
#define LedOFF 0
// Main program
void app_main(void)
{
    // pins configuration
    gpio_set_direction(Pulsador, GPIO_MODE_INPUT);
    gpio_set_direction(Led, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(Pulsador, GPIO_PULLUP_ONLY);
    gpio_set_level(Led, LedON); //
    bool boton_presionado = false;
    while (true)
    {
        // To obtain the state of the button
        int EstadoPulsador = gpio_get_level(Pulsador);
        //  To define Led's state, with the condition of the button's state
        gpio_set_level(Led, EstadoPulsador == 0 ? (boton_presionado = true, LedOFF) : (boton_presionado = false, LedON));
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}