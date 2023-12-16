#ifndef BlinkStructure
#define BlinkStructure

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/timers.h"
#include "driver/adc.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_intr_alloc.h"
#include "driver/ledc.h"

#define button 19
#define led 26
#define cled 2
// #define portTICK_PERIOD_MS 1
#define ledON 1
#define ledOFF 0

// Prototipado de funciones
void turnOnIndicatorLed(int pin, TickType_t delayTime);
void setPins(void);

#endif