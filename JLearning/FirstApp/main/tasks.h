#ifndef TaskStructure
#define TaskStructure

#include "blinkStr.h"

// prototipado de funciones

extern QueueHandle_t buttonQueue;
extern bool blinking;

void mainTask(void *pvParameters);
void IRAM_ATTR buttonPress(void *arg);
void blinkLed(int pin, bool blink);
#endif
