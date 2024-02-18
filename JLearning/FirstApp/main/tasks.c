#include "tasks.h"

QueueHandle_t ADC_lecture = 0;  // Se crea variable para almacenar cola de valores medidos por ADC
QueueHandle_t Temperaturas = 0; // Se crea variable para almacenar cola que contiene temperaturas tomadas en diferentes instantes de tiempo

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

    xTaskCreate(Promedio_temp,
                "Promedio_temp",
                SIZE_BUFFER_TASK,
                &ucParameterToPass,
                1,
                &xHandle);

    ESP_LOGI(tag_task, "Tasks created");

    return ESP_OK;
}

void Promedio_temp(void *pvParameters) // Tarea encargada de tomar 5 valores de lectura de temperatura y promediarlos
{

    while (1)
    {
        float receibedValue = 0, Sumatoria_temp = 0;

        // Ciclo for para leer 5 valores de tempertura y sumarlos
        for (size_t i = 0; i < 5; i++)
        {
            Sumatoria_temp += xQueueReceive(ADC_lecture, &receibedValue, pdMS_TO_TICKS(200)) ? receibedValue : printf("\x1b[31mError receiving value from queue\x1b[0m\n");
        }

        float Temperatura = Sumatoria_temp / 5; // Los valores sumados anteriormente se dividen en 5 para hallar su promedio y tener un valor que no sea tan volátil

        xQueueOverwrite(Temperaturas, &Temperatura); // Se guarda este valor en la cola "Temperaturas"

        vTaskDelay(pdMS_TO_TICKS(Delay_promedio));
    }
}

void get_ADC(void *pvParameters) // Tarea para leer temperatura y guardarla en una cola
{
    int Delay_print_temp_B = Delay_promedio;

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

        vTaskDelay(pdMS_TO_TICKS(Delay_promedio / 5));
    }
}