#include "ConfigStr.h"

QueueHandle_t uart_queue = 0;
esp_err_t setPins(void)
{

    gpio_reset_pin(cled);
    gpio_set_direction(cled, GPIO_MODE_INPUT);

    gpio_reset_pin(led1);
    gpio_set_direction(led1, GPIO_MODE_OUTPUT);
    gpio_reset_pin(led2);
    gpio_set_direction(led2, GPIO_MODE_OUTPUT);
    gpio_reset_pin(led3);
    gpio_set_direction(led3, GPIO_MODE_OUTPUT);
    return ESP_OK;
}
/*
function to configure ADC
*/
esp_err_t setADC(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(NTC, ADC_ATTEN_DB_11);
    ESP_LOGI(tagADC, "ADC configured...");
    return ESP_OK;
}

/*
function to initializate UART port
*/
esp_err_t initUart(void) //
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .parity = UART_PARITY_DISABLE,
        .source_clk = UART_SCLK_APB,
        .stop_bits = UART_STOP_BITS_1};

    uart_param_config(UARTNum, &uart_config);

    uart_set_pin(UARTNum, Uart_RX, Uart_TX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UARTNum, stackSize, stackSize, sizeQueueUART, &uart_queue, 0);

    ESP_LOGI(tagUART, "Init uart completed");

    return ESP_OK;
}

/*
function to configure PWM Channel

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
*/
/*
function to use interruption

void turnOnIndicatorLed(int pin, TickType_t delayTime)
{
    gpio_set_level(pin, 1);
    vTaskDelay(delayTime);
    gpio_set_level(pin, 0);
}
*/