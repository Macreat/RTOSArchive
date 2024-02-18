/**
 * Application entry point.
 */

#include "nvs_flash.h"
#include "NTC.h"
#include "wifi_app.h"
#include "uart_control.h"
#include "ButtonTask.h"

// Importamos las variables externas. Serán utilizadas para crear las colas
extern QueueHandle_t ADC_lecture;
extern QueueHandle_t Temperaturas;

void app_main(void)
{

    inicializeNTC();
    button_task_initialize();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Start Wifi
    wifi_app_start();
    init_uart();
    update_leds_from_uart();
}