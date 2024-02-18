#ifndef WIFI_CONTROL
#define WIFI_CONTROL
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_system.h" //Contiene definiciones y funciones para acceder y controlar el sistema operativo ESP32
#include "esp_event.h"  //Contiene definiciones y funciones para registrar y manejar eventos.
#include "esp_log.h"
#include "esp_wifi.h" //Contiene definiciones y funciones para controlar la conexión Wi-Fi.
#include "esp_sntp.h" //Contiene definiciones y funciones para obtener la hora actual de un servidor NTP

#include "lwip/err.h"     //Contiene definiciones de códigos de error para la biblioteca LwIP.
#include "lwip/sockets.h" //Contiene definiciones y funciones para crear y administrar sockets.
#include "lwip/sys.h"     //Contiene definiciones y funciones para el sistema operativo LwIP.
#include "lwip/netdb.h"   // Contiene definiciones y funciones para resolver nombres de dominio a direcciones IP.
#include "lwip/dns.h"     //Contiene definiciones y funciones para el sistema de nombres de dominio (DNS)

// Las líneas siguientes definen constantes para el éxito y el fracaso de la conexión Wi-Fi y de las conexiones TCP
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE 1 << 1
#define MAX_FAILURES 10

static const char *TAG = "example";

// Prototipado de funciones
esp_err_t connect_wifi();
void initialize_sntp(void);
#endif