#ifndef BUTTON_TASK_H
#define BUTTON_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void button_task_initialize(void);
    int get_button_1_press_count(void);
    int get_button_2_press_count(void);
#ifdef __cplusplus
}
#endif

#endif // BUTTON_TASK_H
