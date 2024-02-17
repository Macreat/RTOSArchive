#ifndef TaskStructure
#define TaskStructure

#include "ConfigStr.h"

// prototipado de funciones

extern QueueHandle_t buttonQueue;
extern int currentLed;
extern const char *TAG;
extern bool turnOn[NLeds];
extern int luminosity[NLeds];

typedef struct
{
    char *key;
    int value;
} Dict_set_values; // structure for organizate respective UART information

esp_err_t createTask(void);
void switchLED(void *pvParameters);
void uartTask(void *pvParameters);
void getADC(void *pvParameters);
#endif
