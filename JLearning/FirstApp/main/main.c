/**
 * Application entry point.
 */

#include "nvs_flash.h"
#include "wifi_app.h"
#include "configure_peripherals.h" //Se importa la librería en donde se configuran todos los periféricos utilizados
#include "tasks.h"                 // Se importa librería en donde se crearon y estructuraron todas las tareas

// Importamos las variables externas. Serán utilizadas para crear las colas
extern QueueHandle_t ADC_lecture;
extern QueueHandle_t Temperaturas;

void app_main(void)
{

    ADC_lecture = xQueueCreate(15, sizeof(float)); // Se crea la cola de 10 espacios y valores de tipo flotante
    Temperaturas = xQueueCreate(1, sizeof(float)); // Se crea cola de 1 espacio con valores enteros
    set_adc();                                     // Se configura el ADC                                                  // Se configura e inicializa la comunicación UART
    create_task();                                 // Se crean las tareas

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
}
