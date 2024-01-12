#include "ConfigStr.h"

void setPins()
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
    adc1_config_channel_atten(NTC, ADC_ATTEN_DB_11);
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

void turnOnIndicatorLed(int pin, TickType_t delayTime)
{
    gpio_set_level(pin, 1);
    vTaskDelay(delayTime);
    gpio_set_level(pin, 0);
}
*/