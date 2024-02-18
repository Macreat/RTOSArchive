#include "motor_control.h"
#include "NTC.h"

void initialize_motors()
{
    gpio_set_direction(motorA1, GPIO_MODE_OUTPUT);
    gpio_set_direction(motorA2, GPIO_MODE_OUTPUT);
    gpio_set_direction(motorB1, GPIO_MODE_OUTPUT);
    gpio_set_direction(motorB2, GPIO_MODE_OUTPUT);
}

void stop_motors()
{
    gpio_set_level(motorA1, LOW);
    gpio_set_level(motorA2, LOW);
    gpio_set_level(motorB1, LOW);
    gpio_set_level(motorB2, LOW);
}
void open_motors()
{
    gpio_set_level(motorA1, LOW);
    gpio_set_level(motorA2, HIGH);
    gpio_set_level(motorB1, LOW);
    gpio_set_level(motorB2, HIGH);
    vTaskDelay(pdMS_TO_TICKS(200));
    stop_motors();
}

void close_motors()
{
    gpio_set_level(motorA1, HIGH);
    gpio_set_level(motorA2, LOW);
    gpio_set_level(motorB1, HIGH);
    gpio_set_level(motorB2, LOW);
    vTaskDelay(pdMS_TO_TICKS(200));
    stop_motors();
}
