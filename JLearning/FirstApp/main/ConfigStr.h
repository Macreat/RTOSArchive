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
// #define button 19 we dont need a button for this case
#define led1 25
#define led2 26
#define led3 33
#define cled 2
#define stackSize 1024     // in/out data memory
#define UARTNum UART_NUM_2 // initalizating UART PORT
#define Uart_TX 17         // GPIO pin TX uart
#define Uart_RX 16         // GPIO pin RX uart
#define sizeQueueUART 5
#define RDelay 500
#define NLeds 3
#define PwmFreq 5000
#define PwmResolution LEDC_TIMER_8_BIT
#define PwmChannel LEDC_CHANNEL_0
#define ledON 1
#define ledOFF 0

// constants using in task file
#define RFixed 10000 // resistor for tension divisor
#define RNtc 10000   // standar value at 25°C
#define Beta 4190
#define RoomTemperature 298.15
#define VolRef 3.3
#define SizeBufferTask stackSize * 2
#define DelayTaskSwitchLed 1000 // retard to actualizate leds
#define initMinLed1 0
#define initMaxLed1 25

#define initMinLed2 24
#define initMaxLed2 26

#define initMinLed3 27
#define initMaxLed3 30

static const char *tagADC = "ADC"; // label for uart message
static const char *tagUART = "UART EVENT";
static const char *tagLED = "LEDs";
static const char *tagTASK = "Task";
// function prototype
esp_err_t setPins(void);
esp_err_t setADC(void);
esp_err_t setUART(void);
esp_err_t initUart(void);

#endif