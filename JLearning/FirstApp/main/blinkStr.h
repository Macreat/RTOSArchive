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
#define led1 26
#define led2 25
#define led3 33
#define cled 2
#define ledON 1
#define ledOFF 0

// function prototype
void turnOnIndicatorLed(int pin, TickType_t delayTime);
void setPins(void);

#endif