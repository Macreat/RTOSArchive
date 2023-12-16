// Libraries to use

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <driver/gpio.h> // to configure GPIOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"  // to tasks
#include "freertos/queue.h" // to use queues

// Definition of GPIO ports

#define button 19
#define led 26
#define cled 2
#define stackSize 1024
#define RDelay 200

QueueHandle_t GlobalQueue = 0;

// local variable
int currentLed = 0; // Variable para mantener el estado actual del LED

// prototype functions
esp_err_t initLed(void);
esp_err_t initIsr(void);
esp_err_t createTask(void);
void stateChange(void *args);
void BlinkLed(void *pvParameters);
/*
function to set pins
*/
esp_err_t initLed(void) // función para configurar el LED como salida
{
    gpio_reset_pin(led);                        // Se resetea el GPIO a sus valores predeterminados
    gpio_set_direction(led, GPIO_MODE_OUTPUT);  // Se indica que el GPIO se va a utilizar como salida
    gpio_reset_pin(cled);                       // Se resetea el GPIO a sus valores predeterminados
    gpio_set_direction(cled, GPIO_MODE_OUTPUT); // Se indica que el GPIO se va a utilizar como salida

    return ESP_OK;
}
/*
function in charge to blink the LED
*/
void BlinkLed(void *pvParameters) // Función que ejecuta la tarea
{
    static bool state = true; // Variable static de tipo bool para varias los estados de funcionamiento
    while (1)
    {
        int receibedValue = 0;                                                        // Se crea variable para almacenar el valor leido de la cola
        int lecture = xQueueReceive(GlobalQueue, &receibedValue, pdMS_TO_TICKS(100)); // lecture almacena true o false dependiendo si logra leer un valor o no

        if (lecture) // Si lee un valor (es decir, si al presionar el botón se agregó un valor a la cola)
        {
            state = !state; // cambia el estado
        }

        // De acuerdo con el estado realiza la respectiva acción
        switch (state)
        {
        case true:
            gpio_set_level(led, 1);
            vTaskDelay(pdMS_TO_TICKS(RDelay));
            gpio_set_level(led, 0);
            vTaskDelay(pdMS_TO_TICKS(RDelay));
            break;

        default:
            gpio_set_level(led, 1);
            break;
        }
    }
}
/*
to control the interruption of my button
*/

esp_err_t initIsr(void) // Configuración e inicialización de las interrupciones
{
    gpio_config_t pGPIOConfig;                        // Prototipado de variable de tipo gpio_config_t (lo exigue la librería) utilizada para la posterior configuración
    pGPIOConfig.pin_bit_mask = (1ULL << button);      // Se cra la mascara del GPIO
    pGPIOConfig.mode = GPIO_MODE_INPUT;               // Se indica que el GPIO va a estar en modo de entrada
    pGPIOConfig.pull_up_en = GPIO_PULLUP_ENABLE;      // Se deshabilita el pullup porque se implementó de forma física
    pGPIOConfig.pull_down_en = GPIO_PULLDOWN_DISABLE; // Se deshabilida el pulldoun por la misma razón anterior
    pGPIOConfig.intr_type = GPIO_INTR_POSEDGE;        // Se indica qué tipo de interrupción se va a utilizar (rissing/flanco de subida en este caso)

    gpio_config(&pGPIOConfig); // Se carga a la función gpio_config(), la variable pGPIOConfig que fue prototipada anterioremente

    gpio_install_isr_service(0);                     // Se inicia la interrupción 0
    gpio_isr_handler_add(button, stateChange, NULL); /* Se define cómo va a trabajar la interrupción
                                                           gpio_isr_handler_add(pin a utilizar, función que va a ejecutar, NULL)*/

    return ESP_OK;
}
/*
function to configurate tasks
 */

esp_err_t createTask(void) // Función en donde se configuran las tareas
{
    static uint8_t ucParameterToPass; // variable para pasar parámetros a la tarea
    TaskHandle_t xHandle = NULL;      // Identificador de la tarea, en este caso como es uno solo, lo dejamos NULL

    xTaskCreate(BlinkLed, "vTaskR", stackSize, &ucParameterToPass, 1, &xHandle); /*Se crea la tarea con las siguiente caracteristicas
                                                                                    xTaskCreate(función que va a ejecutar,
                                                                                                nombre de la tarea,
                                                                                                tamaño en bit para la tarea,
                                                                                                parámetro a enviar,
                                                                                                prioridad,
                                                                                                identificador)*/

    return ESP_OK;
}

/*
to send the interruption of my button foward the queue
*/
void stateChange(void *args) // esta es la función que va a ejercuar cuando se presente una interrupción
{
    int stateButton = gpio_get_level(button);                  // Leer y guarda el estado lógico del LED en state_button
    xQueueSend(GlobalQueue, &stateButton, pdMS_TO_TICKS(100)); // Agrega a la cola la información del puntero state_button
}
// main app
void app_main(void)
{
    GlobalQueue = xQueueCreate(10, sizeof(uint32_t)); // Se crea la cola con espacio para 10 valores y que van a ser de tipo uint32_t
    initLed();                                        // Se configura el LED
    initIsr();                                        // Se configuran las interrupciones, en este caso es solamente una
    createTask();                                     // Crea las tareas
}