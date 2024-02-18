#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define motorA1 4
#define motorA2 5
#define motorB1 12
#define motorB2 13
#define HIGH 1
#define LOW 0

void initialize_motors(void);
void open_motors(void);
void close_motors(void);
void stop_motors(void);

#endif // MOTOR_CONTROL_H
