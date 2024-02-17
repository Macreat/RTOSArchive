#include "tasks.h"

QueueHandle_t ADCLecture = 0; // creating variables to save queue of measured values
QueueHandle_t SetValuesQueue = 0;
Dict_set_values Values_set_min_max_LEDs;

/*
function to create and configurate tasks
*/
esp_err_t createTask(void)
{
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    xTaskCreate(getADC,
                "getADC",
                SizeBufferTask,
                &ucParameterToPass,
                2,
                &xHandle);

    xTaskCreate(switchLED,
                "switchLED",
                SizeBufferTask,
                &ucParameterToPass,
                1,
                &xHandle);

    xTaskCreate(uartTask,
                "uartTask",
                SizeBufferTask,
                &ucParameterToPass,
                5,
                &xHandle);
    ESP_LOGI(tagTASK, "Tasks created");

    return ESP_OK;
}

/*
function in charge to change between leds
also, responsible for reading the values ​​from the ADC queue and the
                                    the data entered by UART communication. After reading and processing them
                                    (if necessary) the actions defined for each one are carried out*/

void switchLED(void *pvParameters)
{
    char *Mssg = "¡SYNTAX ERROR!\nAllow commands: :\n LED_X_MAX=NUM\n LED_X_MIN=NUM\nEn donde: X->1,2 o 3\tNUM->Número"; // To send via UART

    // Assigning limit and minim values
    int Max_ledB = initMaxLed1;
    int Min_ledB = initMinLed1;
    int Max_ledG = initMaxLed2;
    int Min_ledG = initMinLed2;
    int Max_ledR = initMaxLed3;
    int Min_ledR = initMinLed3;

    while (1)
    {
        float receibedValue = 0, averageTemp = 0;

        // for cicle to read and sum 5 values
        for (size_t i = 0; i < 5; i++)
        {
            averageTemp += xQueueReceive(ADCLecture, &receibedValue, pdMS_TO_TICKS(200)) ? receibedValue : printf("\x1b[31mError receiving value from queue\x1b[0m\n");
        }

        float temp = averageTemp / 5; // taking average

        Dict_set_values values;
        if (xQueueReceive(SetValuesQueue, &values, pdMS_TO_TICKS(5)) == pdTRUE)
        {

            char *KEY = (const char *)values.key;
            int count = 0;

            // The data that arrived is compared with each of the possible commands allowed to reassign the temperature limit.
            !strcmp(KEY, "LED_B_MAX") ? Max_ledB = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor máximo de LED BLUE asignado", strlen("CAMBIO CONFIRMADO: Valor máximo de LED BLUE asignado")) : count++;
            !strcmp(KEY, "LED_B_MIN") ? Min_ledB = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor mínimo de LED BLUE asignado", strlen("CAMBIO CONFIRMADO: Valor mínimo de LED BLUE asignado")) : count++;
            !strcmp(KEY, "LED_G_MAX") ? Max_ledG = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor máximo de LED GREEN asignado", strlen("CAMBIO CONFIRMADO: Valor máximo de LED GREEN asignado")) : count++;
            !strcmp(KEY, "LED_G_MIN") ? Min_ledG = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor mínimo de LED GREEN asignado", strlen("CAMBIO CONFIRMADO: Valor mínimo de LED GREEN asignado")) : count++;
            !strcmp(KEY, "LED_R_MAX") ? Max_ledR = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor máximo de LED RED asignado", strlen("CAMBIO CONFIRMADO: Valor máximo de LED RED asignado")) : count++;
            !strcmp(KEY, "LED_R_MIN") ? Min_ledR = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor mínimo de LED RED asignado", strlen("CAMBIO CONFIRMADO: Valor mínimo de LED RED asignado")) : count++;

            // if anyone happens, we send an uart warning
            count == 6 ? uart_write_bytes(UARTNum, Mssg, strlen(Mssg)) : NULL;
        }

        // Checking temperature limit
        temp >= Min_ledB &&temp <= Max_ledB ? gpio_set_level(led1, 1) : gpio_set_level(led1, 0);
        temp >= Min_ledG &&temp <= Max_ledG ? gpio_set_level(led2, 1) : gpio_set_level(led2, 0);
        temp >= Min_ledR &&temp <= Max_ledR ? gpio_set_level(led3, 1) : gpio_set_level(led3, 0);

        ESP_LOGI(tagTASK, "current temperature= %.1f °C", temp);
        vTaskDelay(pdMS_TO_TICKS(DelayTaskSwitchLed));
    }
}

/*Task to detect UART information, read it,
process it to see if it meets certain characteristic
and finally store it in a queue*/
void uartTask(void *pvParameters)
{
    uart_event_t event;                                                                                                                            // Variables to store structure of the detected event
    uint8_t *data = (uint8_t *)malloc(stackSize);                                                                                                  // Variable to store received information
    extern QueueHandle_t uart_queue;                                                                                                               // The queue is imported from an external file since we are going to use it in this
    char *SccndMssg = "¡ESTRUCTURA DE DATOS INCORRECTA!\nComandos permitidos:\n LED_X_MAX=NUM\n LED_X_MIN=NUM\nEn donde: X->R,G o B\tNUM->Número"; // Message to send from UART

    while (1)
    {
        if (xQueueReceive(uart_queue, (void *)&event, portMAX_DELAY))
        {
            gpio_set_level(cled, 1);
            bzero(data, stackSize);

            switch (event.type)
            {
            case UART_DATA:                                                     // If the event type is "UART_DATA", that is, if information arrived, the following is executed
                uart_read_bytes(UARTNum, data, event.size, pdMS_TO_TICKS(500)); // Reads the information and saves it in the "data" variable
                uart_write_bytes(UARTNum, (const char *)data, event.size);      // Returns the same as it arrived, it is not necessary, it is only for verification
                uart_flush(UARTNum);                                            // Clean the port in case anything remains. This so as not to have a problem in the next reading.

                char *ptr = strrchr((const char *)data, '='); // Check if "data" has a "=". Returns NULL if not present, otherwise returns the position of the last time it occurred

                if (ptr != NULL) // codintion about characther "="
                {
                    char *data_whitout_LB = strtok((const char *)data, "\n"); // Removes the line break that the terminal sends by default
                    // printf("data_whitout_LB= %s\n", data_whitout_LB);

                    char *data_key = strtok(data_whitout_LB, "="); // Separa la parte antes del igual (comando para identificar a qué LED se le hace el cambio)
                    Values_set_min_max_LEDs.key = data_key;        // It stores it in the created structure, exactly in the "key" variable that it contains internally
                    // printf("Key= %s\n", Values_set_min_max_LEDs.key);

                    char *data_value = strtok(NULL, "=");           // Separate the part after the equal (Number to assign)
                    int data_value_int = atoi(data_value);          // Converte el string en un entero
                    Values_set_min_max_LEDs.value = data_value_int; // Stores it in the created structure, exactly in the "value" variable that it contains internally
                    // printf("Value= %d\n", Values_set_min_max_LEDs.value);

                    // Save the structure in the queue and if it cannot, report it through the console
                    !xQueueSend(SetValuesQueue, &Values_set_min_max_LEDs, pdMS_TO_TICKS(100)) ? printf("Error chargin value to queue\n") : NULL;
                }
                else
                {
                    uart_write_bytes(UARTNum, SccndMssg, strlen(SccndMssg)); // Send alert message upon completion
                }

                break;

            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        gpio_set_level(cled, 0);
    }
}
/*
This function read and save the ADC value on a queue*/
void getADC(void *pvParameters)
{
    while (1)
    {
        int adc_val = 0;                                 // Variable to save adc value
        adc_val = adc1_get_raw(NTC);                     // to read ADC, in this case only have to give the ADC channel
        float adc_value = (float)adc_val;                // Changing to float value
        float Vol_NTC = (VolRef * adc_value) / 4095;     // Calculing  drop tension on NTC resistor
        float RNtcc = RFixed / ((VolRef / Vol_NTC) - 1); // This is for calucate RNTC value using datasheet
        // float Temperatura_Kelvin = Beta/(log(R_NTC/R0_NTC)+(Beta/Temp0));
        float Temperatura_Kelvin = 1 / ((log(RNtcc / RNtc) / Beta) + (1 / RoomTemperature)); // Calculatin temperature using Kelvin
        float Temperatura_Celcius = Temperatura_Kelvin - 273.15;                             // converting to celsius

        // Save the value in the queue and if it cannot, report it through the console
        xQueueSend(ADCLecture, &Temperatura_Celcius, pdMS_TO_TICKS(50)) ?: printf("\x1b[31mError writing in queue\x1b[0m\n");

        // ESP_LOGI(tag, "Lectura: %i, VOLTAJE: %f, R_NTC: %f, TEMPERATURA: %f", adc_val, Vol_NTC, R_NTC, Temperatura_Celcius);

        vTaskDelay(pdMS_TO_TICKS(DelayTaskSwitchLed / 5));
    }
}
