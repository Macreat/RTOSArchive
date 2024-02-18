#include "ConfigStr.h"
#include "tasks.h"

extern QueueHandle_t ADCLecture;     // Variable is created for the ADC read queue, indicating that it is being taken from an external file
extern QueueHandle_t SetValuesQueue; // A variable is created for the queue where the limits entered by UART are stored, indicating that it is being taken from an external file
// main app just controlling state of a different LEDS with an NTC termistor.
void app_main(void)
{
    ADCLecture = xQueueCreate(10, sizeof(float));              // The queue of 10 spaces and floating type values ​​is created
    SetValuesQueue = xQueueCreate(6, sizeof(Dict_set_values)); // The queue of 6 spaces is created with values ​​of type Dict_set_values ​​(STRUCTURE CREATED)

    setPins();
    setADC();
    initUart();
    createTask();
}
