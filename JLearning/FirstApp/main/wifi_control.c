#include "wifi_control.h"

EventGroupHandle_t wifi_event_group;
int s_retry_num = 0;

// Controlador eventos para los eventos del wifi
void wifi_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) /*Verifica si hay un evento con el ID WIFI_EVENT_STA_START
                                                                        y si lo es, se conecta al wifi*/
    {
        ESP_LOGI(TAG, "Connecting to AP...");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) /*Verifica si hay un evento con el ID WIFI_EVENT_STA_DISCONNECTED
                                                                                    y si lo es, intenta reconectarse al wifi*/
    {
        if (s_retry_num < MAX_FAILURES)
        {
            ESP_LOGI(TAG, "Reconnecting to AP...");
            esp_wifi_connect();
            s_retry_num++;
        }
        else
        {
            xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
        }
    }
}

// Controlador de eventos para eventos de IP
void ip_event_handler(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) /*Si el ID del evento es IP_EVENT_STA_GOT_IP, imprime
                                                                    Imprime la IP del ESP y asigna el bit del evento a
                                                                    wifi_event_group inidicando que ya se conectó*/
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }
}

void time_sync_notification_cb(struct timeval *tv) // Función que se ejecuta cada vez que se detecta un envío de información del SNTP
{
    ESP_LOGW(TAG, "Notification of a time synchronization event");
}

void initialize_sntp(void) // Inicialización del SNTP
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL); /*Se indica que va a trabajar en modo SNTP_OPMODE_POLL. significa que SNTP
                                             se ejecutará en modo de sondeo. En este modo, SNTP se conectará a un servidor
                                              NTP cada cierto tiempo para sincronizar la hora*/

    sntp_set_sync_interval(15000);                                 // Se define cada cuanto tiempo se va a actualizar la hora del ESP32 tomándola desde el SNTP
    sntp_setservername(0, "pool.ntp.org");                         // Se utiliza para configurar el nombre del servidor NTP que SNTP utilizará para sincronizar la hora
    sntp_set_time_sync_notification_cb(time_sync_notification_cb); // Se utiliza para registrar una función de devolución de llamada que será notificada cuando SNTP sincronice la hora.
    sntp_init();                                                   // Se inicializa el cliente o servicio SNTP
}

// Conectar a Wifi y retornar un resultado
esp_err_t connect_wifi()
{
    int status = WIFI_FAILURE;

    /** INITIALIZE ALL THE THINGS **/
    // initialize the esp network interface
    ESP_ERROR_CHECK(esp_netif_init());

    // initialize default esp event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // create wifi station in the wifi driver
    esp_netif_create_default_wifi_sta();

    // setup wifi station with the default wifi configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /** EVENT LOOP CRAZINESS **/
    wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t wifi_handler_event_instance;
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

    /** START THE WIFI DRIVER **/
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "Internet para Ricos",//"Internet para Ricos" "EstudiantesUN" "Gustavo Pisso" "Claro_6ACE68",
            .password = "estaesmiclave04",//"estaesmiclave04" "RedEstudiantes" "12345678" "N2Y4N8R2E9P4",
            .threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };

    // set the wifi controller to be a station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // set the wifi config
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    // start the wifi driver
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA initialization complete");

    /** NOW WE WAIT **/
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_SUCCESS | WIFI_FAILURE,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_SUCCESS)
    {
        ESP_LOGI(TAG, "Connected to ap");
        status = WIFI_SUCCESS;
    }
    else if (bits & WIFI_FAILURE)
    {
        ESP_LOGI(TAG, "Failed to connect to ap");
        status = WIFI_FAILURE;
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        status = WIFI_FAILURE;
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(wifi_event_group);

    return status;
}