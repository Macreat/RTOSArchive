#include "configure_peripherals.h"

QueueHandle_t uart_queue = 0;

esp_err_t init_led(void) // función para configurar los LEDs de acuerdo a la necesidad
{
    gpio_reset_pin(ledUART);
    gpio_set_direction(ledUART, GPIO_MODE_OUTPUT);

    gpio_reset_pin(ledR);
    gpio_set_direction(ledR, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledG);
    gpio_set_direction(ledG, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledB);
    gpio_set_direction(ledB, GPIO_MODE_OUTPUT);
    ESP_LOGI(tag_LED, "LED's inicialized");
    return ESP_OK;
}

esp_err_t set_adc(void) // Función para configurar el puerto ADC
{
    adc1_config_channel_atten(NTC_PIN, ADC_ATTEN_DB_11); // Aquí se escoge el canar a utilizar y la ateniación que deseamos de acuerdo a nuestra señal
    adc1_config_width(ADC_WIDTH_BIT_12);                 // Aquí se escoge la resolución que deseamos para el ADC
    ESP_LOGI(tag_ADC, "ADC configured");
    return ESP_OK;
}

esp_err_t init_uart(void) // Función para configurar e inicializar puerto UART
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .parity = UART_PARITY_DISABLE,
        .source_clk = UART_SCLK_APB,
        .stop_bits = UART_STOP_BITS_1};

    uart_param_config(UART_NUM, &uart_config);

    uart_set_pin(UART_NUM, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, size_queue_UART, &uart_queue, 0);

    ESP_LOGI(tag_UART, "Init uart completed");

    return ESP_OK;
}
