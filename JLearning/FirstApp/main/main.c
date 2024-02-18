// Wifi and tcp controller on ESP32 (connect to a network wwhich is connect to a host and send data over TCP )
//  Importación de librerias utilizadas
#include <stdio.h>
#include <string.h>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

/** DEFINES **/

#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE 1 << 1
#define MAX_FAILURES 10
#define WIFI_CONNECT_TIMEOUT pdMS_TO_TICKS(10000) // 10-second timeout

/** GLOBALS **/

// event group to contain status information
static EventGroupHandle_t wifi_event_group;

// retry  tracker
static int s_retry_num = 0;

// task tag
static const char *TAG = "WIFI";

/** FUNCTIONS **/

// prototipado

// event handler for wifi events
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Connecting to AP...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < MAX_FAILURES)
        {
            ESP_LOGI(TAG, "Reconnecting top AP...");
            esp_wifi_connect();
            s_retry_num++;
        }
        else
        {
            xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
        }
    }
}

// event handler for ip events

static void ip_event_handler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }
}

// connect to wifi and return the result
esp_err_t connect_wifi()
{
    int status = WIFI_FAILURE;

    /*INITIALIZE ALL THE THINGS, LIKE EVENTS its gonna happen*/
    // initialize the esp network interface
    ESP_ERROR_CHECK(esp_netif_init());
    // initialize default event loop,
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // create wifi station in the wifi driver
    esp_netif_create_default_wifi_sta();

    // setup wifi station with the default wifi configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // Correctly pass the config to the init function

    /**EVENT LOOP CRAZINESS**/
    wifi_event_group = xEventGroupCreate(); // where the output of the event is going to live, GLOBAL VARIABLE

    esp_event_handler_instance_t wifi_handler_event_instance; // just checking and call the wifi_event_handler
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &wifi_handler_event_instance));

    esp_event_handler_instance_t got_ip_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &ip_event_handler,
                                                        NULL,
                                                        &got_ip_event_instance));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "IphMat", // Here i can put my real stuff
            .password = "110623Al",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };

    // set the wifi controller to be a station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // set the wifi config
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // start the wifi driver , hhere the event loop for the wifi controller kicks off
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA initialization complete");

    /**NOW WE WAIT**/
    ESP_LOGI(TAG, "Waiting for WiFi connection...");

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_SUCCESS | WIFI_FAILURE,
                                           pdFALSE,
                                           pdFALSE,
                                           WIFI_CONNECT_TIMEOUT); // 10-second timeout
    ESP_LOGI(TAG, "Done waiting for WiFi connection.");

    /* After timeout or event happens, we check which one it was. */
    if (bits & WIFI_SUCCESS)
    {
        ESP_LOGI(TAG, "Succeded connected to AP");
        status = WIFI_SUCCESS;
    }
    else if (bits & WIFI_FAILURE)
    {
        ESP_LOGI(TAG, "Failed to connect to AP");
        status = WIFI_FAILURE;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to connect to AP (timeout)");
        status = WIFI_FAILURE;
    }

    /* The event will not be processed after unregister */

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(wifi_event_group);

    return status;
}

// connect to the server and return the result

esp_err_t connect_tcp_server(void)
{
    struct sockaddr_in serverInfo = {0};
    char readBuffer[1024] = {0};

    serverInfo.sin_family = AF_INET;
    serverInfo.sin_addr.s_addr = htonl(0x8200140a); // Use htonl to ensure proper byte order
    serverInfo.sin_port = htons(12345);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Failed to create a socket..?");
        return TCP_FAILURE;
    }

    if (connect(sock, (struct sockaddr *)&serverInfo, sizeof(serverInfo)) != 0)
    {
        ESP_LOGE(TAG, "Faild to connect to %s!", inet_ntoa(serverInfo.sin_addr.s_addr));
        return TCP_FAILURE;
    }

    ESP_LOGI(TAG, "Connected to TCP server");
    bzero(readBuffer, sizeof(readBuffer));
    int r = read(sock, readBuffer, sizeof(readBuffer) - 1);
    for (int i = 0; i < r; i++)
    {
        putchar(readBuffer[i]);
    }
    if (strcmp(readBuffer, "HELLO") == 0)
    {
        ESP_LOGI(TAG, "NICE,CONNECTED DID IT");
    }
    return TCP_SUCCESS;
}

void app_main(void)
{
    esp_err_t status = WIFI_FAILURE;

    // initialize sotrage ; nonvoltagie storie
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // connect to wireless AP
    status = connect_wifi();
    if (WIFI_SUCCESS != status)
    {
        ESP_LOGI(TAG, "Failed to associate to AP, exiting...");
        return;
    }

    status = connect_tcp_server();
    if (TCP_SUCCESS != status)
    {
        ESP_LOGI(TAG, "Failed to connect to remote server, exiting...");
        return;
    }
}
