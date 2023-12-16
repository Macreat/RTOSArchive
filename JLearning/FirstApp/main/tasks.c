#include "tasks.h"

bool blinking = true;
static const char *TAG = "Button_Task";
QueueHandle_t buttonQueue;
int currentLed = 0;

void IRAM_ATTR buttonPress(void *arg)
{

    int buttonState = gpio_get_level(button);
    xQueueSendFromISR(buttonQueue, &buttonState, NULL);
    vTaskDelay(pdMS_TO_TICKS(10)); // Pequeño retardo para evitar lecturas múltiples del botón
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

void mainTask(void *pvParameters)
{
    gpio_install_isr_service(0);
    gpio_isr_handler_add(button, buttonPress, (void *)button);
    while (true)
    {
        int estadoButton;
        int buttonState = xQueueReceive(buttonQueue, &estadoButton, portMAX_DELAY);
        if (buttonState) // changing to do while;
        {
            turnOnIndicatorLed(cled, pdMS_TO_TICKS(1000));
            // Espera un poco para evitar múltiples detecciones del botón
            vTaskDelay(pdMS_TO_TICKS(100));
            currentLed = *((int *)pvParameters);
            currentLed = (currentLed + 1) % 3;
            *((int *)pvParameters) = currentLed;
        }
        switch (*((int *)pvParameters))
        {
        case 0:
            blinkLed(led1, true);
            break;
        case 1:
            blinkLed(led2, true);
            break;
        case 2:
            blinkLed(led3, true);
            break;
        }
    }
}
