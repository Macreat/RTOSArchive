#include "NTC.h"

QueueHandle_t uart_queue = 0;
QueueHandle_t ADC_lecture = 0;  // Se crea variable para almacenar cola de valores medidos por ADC
QueueHandle_t Temperaturas = 0; // Se crea variable para almacenar cola que contiene temperaturas tomadas en diferentes instantes de tiempo

bool inicializeNTChandle = false;
int timerId = 0; // Identificación de cada timer, en este ejemplo no fue tan util ya que solamente hay uno

void inicializeNTC(void)
{
    if (inicializeNTChandle == false)
    {
        set_adc();         // Se configura el ADC
        create_task_NTC(); // Se crean las tareas
        inicializeNTChandle = true;
    }
}

esp_err_t set_adc(void) // Función para configurar el puerto ADC
{
    adc1_config_channel_atten(NTC_PIN, ADC_ATTEN_DB_11); // Aquí se escoge el canar a utilizar y la ateniación que deseamos de acuerdo a nuestra señal
    adc1_config_width(ADC_WIDTH_BIT_12);                 // Aquí se escoge la resolución que deseamos para el ADC
    ESP_LOGI(tag_ADC, "ADC configured");
    return ESP_OK;
}

esp_err_t create_task_NTC(void) // Función en donde se crean y configuran las tareas
{

    ADC_lecture = xQueueCreate(15, sizeof(float)); // Se crea la cola de 10 espacios y valores de tipo flotante
    Temperaturas = xQueueCreate(1, sizeof(float)); // Se crea cola de 1 espacio con valores enteros

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
        xQueueSend(ADC_lecture, &Temperatura_Celcius, pdMS_TO_TICKS(5)) ?: printf("\x1b[31mError writing in queue\x1b[0m\n");

        // ESP_LOGI(tag, "Lectura: %i, VOLTAJE: %f, R_NTC: %f, TEMPERATURA: %f", adc_val, Vol_NTC, R_NTC, Temperatura_Celcius);

        vTaskDelay(pdMS_TO_TICKS(Delay_promedio / 5));
    }
}