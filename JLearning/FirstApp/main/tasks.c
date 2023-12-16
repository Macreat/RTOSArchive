#include "tasks.h"

bool blinking = true;
static const char *TAG = "Button_Task";
QueueHandle_t buttonQueue;

void IRAM_ATTR buttonPress(void *arg)
{

    int buttonState = gpio_get_level(button);
    xQueueSendFromISR(buttonQueue, &buttonState, NULL);
}

void blinkLed(int pin, bool blink)
{
    while (blink)
    {
        gpio_set_level(pin, ledON);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Encendido durante 100 ms
        gpio_set_level(pin, ledOFF);
        vTaskDelay(100 / portTICK_PERIOD_MS); // Apagado durante 100 ms
        int currentButtonState = gpio_get_level(button);
        if (currentButtonState == 0)
        {
            break; // blin = false
        }
    }
}

void mainTask(void *pvParameter) // changing main task name
{
    bool blink;
    int buttonState;             // redunding
    gpio_install_isr_service(0); // inicializar una sola vez
    gpio_isr_handler_add(button, buttonPress, (void *)button);
    while (true)
    {

        int bqEl = 0;
        int buttonState = xQueueReceive(buttonQueue, &bqEl, portMAX_DELAY);
        do
        {
            turnOnIndicatorLed(cled, pdMS_TO_TICKS(1000));
            ESP_LOGI(TAG, "Button pressed. Blinking ");
            blinkLed(led, blinking);
            blink = blinking;
            if (buttonState)
            {
                blink = false;
            }
        } while (blink);

        turnOnIndicatorLed(cled, pdMS_TO_TICKS(500));
        ESP_LOGI(TAG, "Button pressed. Blinking disabled. Rebooting...");
        gpio_set_level(led, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        break;
    }
}
