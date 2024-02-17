#include "ConfigStr.h"
#include "tasks.h"

extern QueueHandle_t ADCLecture;     // Se crea variable para la cola de lectura del ADC, indicando que se está tomando de un archivo externo
extern QueueHandle_t SetValuesQueue; // Se crea variable para la cola en donde se almacenan los límites ingresados por UART, indicando que se está tomando de un archivo externo

// main app just controlling state of a different LEDS with an NTC termistor.
void app_main(void)
{
    ADCLecture = xQueueCreate(10, sizeof(float));              // Se crea la cola de 10 espacios y valores de tipo flotante
    SetValuesQueue = xQueueCreate(6, sizeof(Dict_set_values)); // Se cree la cola de 6 espacios con valores de tipo Dict_set_values (ESTRUCTURA CREADA)

    setPins();
    setADC();
    initUart();
    createTask();
}
