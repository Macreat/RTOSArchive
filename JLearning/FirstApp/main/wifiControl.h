#ifndef WIFI_CONTROL
#define WIFI_CONTROL
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h" //Librerarie to controll vololatile memory from ESP32

#include "freertos/event_groups.h"
#include "esp_system.h" // Contains definitions and functions to access and control the ESP32 operating system
#include "esp_event.h"  //Contains definitions and functions to register and handle events.
#include "esp_log.h"
#include "esp_wifi.h" //Contiene definiciones y funciones para controlar la conexión Wi-Fi.
#include "esp_sntp.h" //Contains definitions and functions to obtain the current time from an NTP server

#include "lwip/err.h"     //Contains error code definitions for the LwIP library.
#include "lwip/sockets.h" //Contains definitions and functions for creating and managing sockets.
#include "lwip/sys.h"     //Contains definitions and functions for the LwIP operating system.
#include "lwip/netdb.h"   // Contains definitions and functions to resolve domain names to IP addresses.
#include "lwip/dns.h"     //Contains definitions and functions for the domain name system (DNS)

// The following lines define constants for success and failure of Wi-Fi connection and TCP connections
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE 1 << 1
#define MAX_FAILURES 10

static const char *TAG = "example";

// function prototype
esp_err_t connect_wifi();
void initialize_sntp(void);
#endif
