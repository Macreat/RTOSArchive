#ifndef TaskStructure
#define TaskStructure

#include "ConfigStr.h"

// prototipado de funciones

extern QueueHandle_t buttonQueue;
extern int currentLed;
extern const char *TAG;
extern bool turnOn[NLeds];
extern int luminosity[NLeds];

void IRAM_ATTR buttonIsrHandler(void *arg);
void changeLed(void);
void buttonTask(void *pvParameters);
void getLuminosity(int potValue);
void setLuminosity(void);
#endif
