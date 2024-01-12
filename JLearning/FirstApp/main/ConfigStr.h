#ifndef ConfigStructure
#define ConfigStructure

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

#define NTC ADC1_CHANNEL_7
#define button 19
#define led1 25
#define led2 26
#define led3 33
#define cled 2
#define stackSize 1024
#define RDelay 500
#define NLeds 3
#define PwmFreq 5000
#define PwmResolution LEDC_TIMER_8_BIT
#define PwmChannel LEDC_CHANNEL_0
#define ledON 1
#define ledOFF 0

// function prototype
void turnOnIndicatorLed(int pin, TickType_t delayTime);
void setPins(void);
void setADC(void);
void setPWM(void);
#endif