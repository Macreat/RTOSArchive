// importing libraries
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// defining constants
#define Button 18
#define Led 33
#define portTICK_PERIOD_MS 1
#define LedON 1
#define LedOFF 0

void app_main(void)
{
    // configuirn pins
    gpio_set_direction(Button, GPIO_MODE_INPUT);
    gpio_set_direction(Led, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(Button, GPIO_PULLUP_ONLY);
    // variable definition
    bool blinking = true;
    // main bucle
    while (true)
    {
        int buttonState = gpio_get_level(Button);
        buttonState == 0 ? (blinking = !blinking, vTaskDelay(20 / portTICK_PERIOD_MS))
                         : (void)0;
        blinking == 1 ? (gpio_set_level(Led, LedON), vTaskDelay(10 / portTICK_PERIOD_MS), gpio_set_level(Led, LedOFF), vTaskDelay(10 / portTICK_PERIOD_MS))
                      : gpio_set_level(Led, LedOFF);
    }
}