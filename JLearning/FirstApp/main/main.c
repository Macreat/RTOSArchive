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

#define button 19
#define led1 25
#define led2 26
#define led3 33
#define cled 2
#define stackSize 1024
#define RDelay 500

QueueHandle_t buttonQueue = 0;

// local variable

static const char *TAG = "Button Task";
extern bool blinking = true;
int currentLed = 0; // Variable para mantener el estado actual del LED

// prototype functions
void setPins(void);
esp_err_t initIsr(void);
void buttonTask(void *pvParameters);
void BlinkLed(void *pvParameters);
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
function to use interruption
*/
void turnOnIndicatorLed(int pin, TickType_t delayTime)
{
    gpio_set_level(pin, 1);
    vTaskDelay(delayTime);
    gpio_set_level(pin, 0);
}

/*
function in charge to blink the LED
*/
void blinkLed(int pin)
{
    bool blinking = true; // definir al principio
    while (blinking)
    {
        int estadoButton = gpio_get_level(button);
        gpio_set_level(pin, 1);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(pin, 0);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if (estadoButton == 0)
        {
            blinking = false;
        }
    }
}
/*
function to configurate tasks
 */

void buttonTask(void *pvParameter)
{
    while (true)
    {
        int estadoButton = gpio_get_level(button);
        xQueueSend(buttonQueue, &estadoButton, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100)); // Pequeño retardo para evitar lecturas múltiples del botón
    }
}

/*
function to manipulate the change on to LEDs
*/
void changeLed(void *pvParameters)
{
    int estadoButton;

    while (true)
    {
        xQueueReceive(buttonQueue, &estadoButton, portMAX_DELAY);

        if (estadoButton == 0)
        {
            // turn on the indicator led of an interruption
            turnOnIndicatorLed(cled, pdMS_TO_TICKS(1000));

            // delay to evite reboot button
            vTaskDelay(pdMS_TO_TICKS(100));

            // Change to next LED
            currentLed = *((int *)pvParameters);
            currentLed = (currentLed + 1) % 3;
            *((int *)pvParameters) = currentLed;
        }

        // turning on the correspondient led
        switch (*((int *)pvParameters))
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
    SetPins();

    // creating queue for button interruption
    buttonQueue = xQueueCreate(1, sizeof(int));

    // creating tasks
    xTaskCreate(buttonTask, "button_task", 2048, NULL, 10, NULL);
    xTaskCreate(changeLed, "change_led_task", 2048, &currentLed, 10, NULL);
}