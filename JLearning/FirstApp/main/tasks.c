#include "tasks.h"

QueueHandle_t ADC_lecture = 0;           // Se crea variable para almacenar cola de valores medidos por ADC
QueueHandle_t Set_values_queue = 0;      // Se crea variable para almacenar cola de valores leídos por el UART
Dict_set_values Values_set_min_max_LEDs; // Se crea variable con la estructura Dict_set_values

esp_err_t create_task(void) // Función en donde se crean y configuran las tareas
{
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    xTaskCreate(get_ADC,
                "get_ADC",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                2,
                &xHandle);

    xTaskCreate(switch_LED,
                "switch_LED",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                1,
                &xHandle);

    xTaskCreate(uart_task,
                "uart_task",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                5,
                &xHandle);
    ESP_LOGI(tag_task, "Tasks created");

    return ESP_OK;
}

void switch_LED(void *pvParameters) /*Función encargada de leer los valores de la cola del ADC y la cola de
                                    los datos ingresados por comunicación UART. Depués de leerlos y procesarlos
                                    (si es necesario) se realizan las acciones definidas para cada uno*/
{
    char *Mensaje_guia = "¡ERROR SINTAXIS!\nComandos permitidos:\n LED_X_MAX=NUM\n LED_X_MIN=NUM\nEn donde: X->R,G o B\tNUM->Número"; // Mensaje para enviar por UART

    // Se asignan a las variables de límites de temperatura los valores definidos inicialmente
    int Max_ledB = Init_Max_ledB;
    int Min_ledB = Init_Min_ledB;
    int Max_ledG = Init_Max_ledG;
    int Min_ledG = Init_Min_ledG;
    int Max_ledR = Init_Max_ledR;
    int Min_ledR = Init_Min_ledR;

    while (1)
    {
        float receibedValue = 0, Sumatoria_temp = 0;

        // Ciclo for para leer 5 valores de tempertura y sumarlos
        for (size_t i = 0; i < 5; i++)
        {
            Sumatoria_temp += xQueueReceive(ADC_lecture, &receibedValue, pdMS_TO_TICKS(200)) ? receibedValue : printf("\x1b[31mError receiving value from queue\x1b[0m\n");
        }

        float Temperatura = Sumatoria_temp / 5; // Los valores sumados anteriormente se dividen en 5 para hallar su promedio y tener un valor que no sea tan volátil

        Dict_set_values values;
        if (xQueueReceive(Set_values_queue, &values, pdMS_TO_TICKS(5)) == pdTRUE)
        {
            /*Imprimimos los valores
            printf("Key: %s\n", values.key);
            printf("Value: %i\n", values.value);*/

            char *KEY = (const char *)values.key;
            int contador = 0;

            // Se compara el dato que llegó con cada uno de los posibles comandos permitidos para reasignar limite de temperatura
            !strcmp(KEY, "LED_B_MAX") ? Max_ledB = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor máximo de LED BLUE asignado", strlen("CAMBIO CONFIRMADO: Valor máximo de LED BLUE asignado")) : contador++;
            !strcmp(KEY, "LED_B_MIN") ? Min_ledB = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor mínimo de LED BLUE asignado", strlen("CAMBIO CONFIRMADO: Valor mínimo de LED BLUE asignado")) : contador++;
            !strcmp(KEY, "LED_G_MAX") ? Max_ledG = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor máximo de LED GREEN asignado", strlen("CAMBIO CONFIRMADO: Valor máximo de LED GREEN asignado")) : contador++;
            !strcmp(KEY, "LED_G_MIN") ? Min_ledG = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor mínimo de LED GREEN asignado", strlen("CAMBIO CONFIRMADO: Valor mínimo de LED GREEN asignado")) : contador++;
            !strcmp(KEY, "LED_R_MAX") ? Max_ledR = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor máximo de LED RED asignado", strlen("CAMBIO CONFIRMADO: Valor máximo de LED RED asignado")) : contador++;
            !strcmp(KEY, "LED_R_MIN") ? Min_ledR = values.value, uart_write_bytes(UART_NUM_2, "CAMBIO CONFIRMADO: Valor mínimo de LED RED asignado", strlen("CAMBIO CONFIRMADO: Valor mínimo de LED RED asignado")) : contador++;

            // Si no se cumple ninguno, envía el mensaje guía por medio del UART
            contador == 6 ? uart_write_bytes(UART_NUM_2, Mensaje_guia, strlen(Mensaje_guia)) : NULL;
        }

        // Compara temperatura leída con cada uno de los límites
        Temperatura >= Min_ledB &&Temperatura <= Max_ledB ? gpio_set_level(ledB, 1) : gpio_set_level(ledB, 0);
        Temperatura >= Min_ledG &&Temperatura <= Max_ledG ? gpio_set_level(ledG, 1) : gpio_set_level(ledG, 0);
        Temperatura >= Min_ledR &&Temperatura <= Max_ledR ? gpio_set_level(ledR, 1) : gpio_set_level(ledR, 0);

        ESP_LOGI(tag_task, "Temperatura= %.1f °C", Temperatura);
        vTaskDelay(pdMS_TO_TICKS(Delay_Task_Switch_LED));
    }
}

void uart_task(void *pvParameters) /*Tarea para detectar información del UART, leerla,
                                    procesarla para ver si cumple con ciertas caracteristicas
                                     y finalmente almacenarla en una cola*/
{
    uart_event_t event;                                                                                                                               // Variables para almacenar estructura del evento detectado
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);                                                                                                      // Variable para almacenar información recibida
    extern QueueHandle_t uart_queue;                                                                                                                  // Se importa la cola de un archivo externo ya que la vamos a utilizar en este
    char *Mensaje_guia = "¡ESTRUCTURA DE DATOS INCORRECTA!\nComandos permitidos:\n LED_X_MAX=NUM\n LED_X_MIN=NUM\nEn donde: X->R,G o B\tNUM->Número"; // Mensaje par enviar por UART

    while (1)
    {
        if (xQueueReceive(uart_queue, (void *)&event, portMAX_DELAY))
        {
            gpio_set_level(ledUART, 1);
            bzero(data, BUF_SIZE);

            switch (event.type)
            {
            case UART_DATA:                                                      // Si el tipo de evento es "UART_DATA", es decir, si llegó información, se ejecuta lo siguiente
                uart_read_bytes(UART_NUM, data, event.size, pdMS_TO_TICKS(500)); // Lee la información y la guarda en la variable "data"
                uart_write_bytes(UART_NUM, (const char *)data, event.size);      // Retorna lo mismo que llegó, no es necesario, solamentes es de verificación
                uart_flush(UART_NUM);                                            // Limpia el puerto por si queda algo. Esto para no tener problema en la próxima lectura

                char *ptr = strrchr((const char *)data, '='); // Revisa si "data" tiene un "=". Devuelve NULL si no está, de lo contrario devuleve la posición de la última vez que se presentó

                if (ptr != NULL) // Sí si está el caracter "="
                {
                    char *data_whitout_LB = strtok((const char *)data, "\n"); // Quita el salto de línea que enviar el terminal por defecto
                    // printf("data_whitout_LB= %s\n", data_whitout_LB);

                    char *data_key = strtok(data_whitout_LB, "="); // Separa la parte antes del igual (comando para identificar a qué LED se le hace el cambio)
                    Values_set_min_max_LEDs.key = data_key;        // Lo almacena en la estructura creada, exactamente en la varible "key" que contiene internamente
                    // printf("Key= %s\n", Values_set_min_max_LEDs.key);

                    char *data_value = strtok(NULL, "=");           // Separa la parte después del igual (Número a asignar)
                    int data_value_int = atoi(data_value);          // Converte el string en un entero
                    Values_set_min_max_LEDs.value = data_value_int; // Lo almacena en la estructura creada, exactamente en la varible "value" que contiene internamente
                    // printf("Value= %d\n", Values_set_min_max_LEDs.value);

                    // Guarda la estructura en la cola y en caso tal que no pueda, informa por medio de consola
                    !xQueueSend(Set_values_queue, &Values_set_min_max_LEDs, pdMS_TO_TICKS(100)) ? printf("Error cargando valor a la cola\n") : NULL;
                }
                else
                {
                    uart_write_bytes(UART_NUM_2, Mensaje_guia, strlen(Mensaje_guia)); // Envía mensaje de alerta a la terminar
                }

                break;

            default:
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
        gpio_set_level(ledUART, 0);
    }
}

void get_ADC(void *pvParameters) // Tarea para leer temperatura y guardarla en una cola
{
    while (1)
    {
        int adc_val = 0;                                   // Variable para almacenar lectura
        adc_val = adc1_get_raw(NTC_PIN);                   // Funcion para leer el ADC. En este caso solamente nos pide el canal que deseamos leer
        float adc_value = (float)adc_val;                  // Se castea a valor flotante
        float Vol_NTC = (Vol_REF * adc_value) / 4095;      // Se calcula el voltaje que está cayendo en la NTC
        float R_NTC = R_FIXED / ((Vol_REF / Vol_NTC) - 1); // Se calcula la resistencia que tiene la NTC en ese momento. Esto se sacó despejando fórmulas del datasheet
        // float Temperatura_Kelvin = Beta/(log(R_NTC/R0_NTC)+(Beta/Temp0));
        float Temperatura_Kelvin = 1 / ((log(R_NTC / R0_NTC) / Beta) + (1 / Temp0)); // Se calcula la temperatura en grados Kelvin. También se hizo despejando
        float Temperatura_Celcius = Temperatura_Kelvin - 273.15;                     // Se convierte a grados Selcius

        // Guarda la el valor en la cola y en caso tal que no pueda, informa por medio de consola
        xQueueSend(ADC_lecture, &Temperatura_Celcius, pdMS_TO_TICKS(50)) ?: printf("\x1b[31mError writing in queue\x1b[0m\n");

        // ESP_LOGI(tag, "Lectura: %i, VOLTAJE: %f, R_NTC: %f, TEMPERATURA: %f", adc_val, Vol_NTC, R_NTC, Temperatura_Celcius);

        vTaskDelay(pdMS_TO_TICKS(Delay_Task_Switch_LED / 5));
    }
}
