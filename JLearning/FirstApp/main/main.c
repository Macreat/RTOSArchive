#include "configure_peripherals.h" //Se importa la librería en donde se configuran todos los periféricos utilizados
#include "tasks.h"                 // Se importa librería en donde se crearon y estructuraron todas las tareas

extern QueueHandle_t ADC_lecture;      // Se crea variable para la cola de lectura del ADC, indicando que se está tomando de un archivo externo
extern QueueHandle_t Set_values_queue; // Se crea variable para la cola en donde se almacenan los límites ingresados por UART, indicando que se está tomando de un archivo externo

void app_main(void)
{
    ADC_lecture = xQueueCreate(10, sizeof(float));               // Se crea la cola de 10 espacios y valores de tipo flotante
    Set_values_queue = xQueueCreate(6, sizeof(Dict_set_values)); // Se cree la cola de 6 espacios con valores de tipo Dict_set_values (ESTRUCTURA CREADA)
    init_led();                                                  // Se inicializan los LEDs
    set_adc();                                                   // Se configura el ADC
    init_uart();                                                 // Se configura e inicializa la comunicación UART
    create_task();                                               // Se crean las tareas
}
