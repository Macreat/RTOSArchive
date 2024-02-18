// libraries used
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define LM35_ADC_CHANNEL ADC1_CHANNEL_7 // USING GPIO34 pin
#define DEFAULT_VREF 1100               // Using generic value for callibration

void app_main()
{
    // Initializing ADC
    adc1_config_width(ADC_WIDTH_BIT_12);                          // 12 bits configuration
    adc1_config_channel_atten(LM35_ADC_CHANNEL, ADC_ATTEN_DB_11); // Atenuation from maximum 3.3 V input
    while (1)
    {
        // read ADC
        int adc_reading = adc1_get_raw(LM35_ADC_CHANNEL);

        // Convert ADC lecture to temperature in °C
        float voltage = (float)adc_reading / 4095.0 * 3.3; // Converting adc lecture to drop voltage (3.3V is reference)
        float temperature = voltage * 100.0;               // LM35 give 10 mV for °C-

        printf(" room temperature: %.2f°C\n", temperature);

        vTaskDelay(1000 / porTICK_PERIOD_MS); // one second delay
    }
}