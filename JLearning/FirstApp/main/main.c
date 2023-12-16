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
#define led 26
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
esp_err_t buttonTask(void *pvParameters);
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
void setPins()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << button),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_ANYEDGE,
        .pull_up_en = GPIO_PULLUP_ENABLE};

    gpio_config(&io_conf);

    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << led) | (1ULL << cled),
        .mode = GPIO_MODE_OUTPUT};

    gpio_config(&led_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(button, buttonIsrHandler, (void *)button);
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
void blinkLed(int pin, bool blink)
{
    while (blink) // Mientras el botón no se presione
    {

        gpio_set_level(pin, 1);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Encendido durante 100 ms
        gpio_set_level(pin, 0);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Apagado durante 100 ms
        int buttonState = gpio_get_level(button);
        if (buttonState == 0)
        {
            ESP_LOGI(TAG, "Button pressed. Blinking stopped ...");

            break; // blin = false
        }
    }
    gpio_set_level(led, 0);
}
/*
function to configurate tasks
 */

esp_err_t buttonTask(void *pvParameters)
{
    int buttonState;
    bool blink;
    while (true)
    {
        if (xQueueReceive(buttonQueue, &buttonState, portMAX_DELAY))
        {
            buttonState == 0 ? (blinking = !blinking,
                                turnOnIndicatorLed(cled, pdMS_TO_TICKS(1000)), vTaskDelay(1 / portTICK_PERIOD_MS))
                             : ((void)0, vTaskDelay(1 / portTICK_PERIOD_MS));

            do
            {
                ESP_LOGI(TAG, "Button pressed...");
                blinkLed(led, blinking);
                blink = blinking;
                int bt = gpio_get_level(button);
                if (bt == 0)
                {
                    blink = false;
                }
            } while (blink); // La condición para salir del bucle

            printf("Blinking end...\n");
            gpio_set_level(led, 0);
        }
    }
}
// main app
void app_main(void)
{
    setPins();
    buttonQueue = xQueueCreate(1, sizeof(int));

    xTaskCreate(buttonTask, "button_task", 2048, NULL, 10, NULL);
}