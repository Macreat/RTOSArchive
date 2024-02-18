// imported libraries
#include "wifiControl.h" // libraries to use wifi
#include "uartControl.h" // personal librarie for UART use

#define STACK_SIZE 1024 // Space memory from tasks

extern void send_time_UART(void); // importing a function which was created on to another file
void app_main(void)
{

    init_uart(); // Initializing uart protocol

    esp_err_t status = WIFI_FAILURE; // variable to control state connection

    // initialize storage
    esp_err_t ret = nvs_flash_init();                                             // initializing no volatile memory
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) // validating space or update from NVS
    {
        ESP_ERROR_CHECK(nvs_flash_erase()); // deleting NVS info
        ret = nvs_flash_init();             // re creating NVS
    }
    ESP_ERROR_CHECK(ret); // to check  NVS created

    // connect to wireless AP
    status = connect_wifi();    // conntecting wifi STA (station mode)
    if (WIFI_SUCCESS != status) // validating if ESP32 cant connect
    {
        ESP_LOGI(TAG, "Failed to associate to AP, dying...");
        return;
    }

    initialize_sntp(); // connecting to SNTP server and update hour

    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED) /*As long as the time is not updated from the server, it will remain at this point
                                                                  This is to prevent the program from having an incorrect time when it is started*/
    {
        ESP_LOGE(TAG, "waiting first hour update ...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle;
    xTaskCreate(send_time_UART, "update_time", STACK_SIZE * 3, ucParameterToPass, 5, &xHandle);
}
