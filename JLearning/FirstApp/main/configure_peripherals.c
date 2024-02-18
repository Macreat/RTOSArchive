#include "configure_peripherals.h"

QueueHandle_t uart_queue = 0;
int timerId = 0; // Identificación de cada timer, en este ejemplo no fue tan util ya que solamente hay uno

esp_err_t set_adc(void) // Función para configurar el puerto ADC
{
    adc1_config_channel_atten(NTC_PIN, ADC_ATTEN_DB_11); // Aquí se escoge el canar a utilizar y la ateniación que deseamos de acuerdo a nuestra señal
    adc1_config_width(ADC_WIDTH_BIT_12);                 // Aquí se escoge la resolución que deseamos para el ADC
    ESP_LOGI(tag_ADC, "ADC configured");
    return ESP_OK;
}