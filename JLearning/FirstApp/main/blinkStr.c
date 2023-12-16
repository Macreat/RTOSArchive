#include "blinkStr.h"

void setPins()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << button),
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_POSEDGE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE};

    gpio_config(&io_conf);

    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << led1) | (1ULL << cled) | (1ULL << led2) | (1ULL << led3),
        .mode = GPIO_MODE_OUTPUT};

    gpio_config(&led_conf);
}

void turnOnIndicatorLed(int pin, TickType_t delayTime)
{
    gpio_set_level(pin, ledON);
    vTaskDelay(delayTime);
    gpio_set_level(pin, ledOFF);
}
