// Importación de librerias utilizadas
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/timers.h"
#include "driver/adc.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_intr_alloc.h"
#include "driver/ledc.h"
// Definición de variables y constantes locales para el trabajo
#define NtcPIN ADC1_CHANNEL_7 // Usamos el GPIO PIN 35
#define CLed 2                // Definimos como led de control al UART 2 : Este indica la intensidad que tiene cada uno de los leds que PUEDE estar encendido.
#define ledR 21
#define ledG 22
#define ledB 23
#define button 27

#define SizeBufferT 1024 * 2
#define DTSL 1000  // DelayTaskSwitchLed
#define RDv 10000  // Valor asociado a la resistencia para el divisor de tensión
#define RNtc 10000 // Valor asociado a la resistencia del termistor NTC
#define Beta 4190
#define Temp0 298.15
#define RefVol 3.3
#define UartNUM UART_NUM_0
#define BufSize 1024

#define RMinLedB 0
#define RMaxLedB 30
#define RMinLedG 30
#define RMaxLedG 50
#define RMinLedR 50
#define RMaxLedR 200
#define MaxLIntensity 100
#define MinLIntensity 0
#define MaxFrecuencyPlot 5
#define ResetFrecuencyPlot 1

#define LenghtADCQ 10
#define LenghtRangeValuesQ 6
#define LenghtButtonQ 1

int CurrentFrequency = 1; // Frecuencia inicial para la impresión de temperatura
int IntensityLed = 50;    // Valor inicial predeterminado para la intensidad del LED

typedef struct
{
    char *key;
    int value;
} Dict_set_values;

// Variables asociadas a esp_log.h

static const char *tag = "ADC";
static const char *tag_UART = "UART EVENT";
static const char *tag_task = "Task";

// Definición de las cuatro diferentes colas necesarias

QueueHandle_t ADCLQ = 0;
QueueHandle_t UartQ = 0;
QueueHandle_t SetValuesQ = 0;
QueueHandle_t ButtonQ = 0;
Dict_set_values RangeValuesRGB;

/* FUNCIONES / TAREAS:
    PROTOTIPADO */

esp_err_t InitLed(void);
esp_err_t InitUART(void);
esp_err_t SetADC(void);
esp_err_t Tasks(void);
// esp_err_t SetIntensity(channel, intensity);
void GetADC(void *pvParameters);
void SwLED(void *pvParameters);
void UartTask(void *pvParameters);
void ButtonTask(void *pvParameters);
void IRAM_ATTR ButtonPress(void *arg);
void app_main(void);

// DEFINICIONES

/**
 * @brief Manejador de interrupciones para el botón.
 *
 * Esta función se activa cuando se presiona el botón. Incrementa la frecuencia actual en 1
 * y la reinicia a 1 si supera 5. Luego, envía la frecuencia actualizada a la tarea principal
 * a través de la cola `ButtonQ`.
 *
 * @param arg Puntero a los argumentos pasados a la función de interrupción (no utilizado aquí).
 */
void IRAM_ATTR ButtonPress(void *arg)
{
    int new_frequency = CurrentFrequency + 1;
    if (new_frequency > MaxFrecuencyPlot)
    {
        new_frequency = ResetFrecuencyPlot;
    }
    xQueueSendFromISR(ButtonQ, &new_frequency, NULL); // Enviamos la frecuencia actualizada por el botón a la tarea principal usando una cola
}
/**
 * @brief Tarea para obtener lecturas del ADC y calcular la temperatura.
 *
 * Esta función obtiene lecturas del ADC, calcula la temperatura utilizando un termistor
 * y envía la temperatura a la cola `ADCLQ` para su procesamiento.
 *
 * @param pvParameters Puntero a los parámetros pasados a la tarea (no utilizado aquí).
 */
void GetADC(void *pvParameters)
{
    while (1)
    {
        int adc_val = 0;
        adc_val = adc1_get_raw(NtcPIN);
        float adc_value = (float)adc_val;
        float VNtc = (RefVol * adc_value) / 4095;
        float Rntc = RDv / ((RefVol / VNtc) - 1);
        float Temperatura_Kelvin = Beta / (log(Rntc / RNtc) + (Beta / Temp0)); // Ecuación que nos relaciona la resistencia del termistor con la temperatura del ambiente
        float Temperatura_Celcius = Temperatura_Kelvin - 273.15;

        xQueueSend(ADCLQ, &Temperatura_Celcius, pdMS_TO_TICKS(50)) ?: printf("\x1b[31mError writing in queue\x1b[0m\n"); // Uso de colas para envío de datos entre tareas

        vTaskDelay(pdMS_TO_TICKS(DTSL / MaxFrecuencyPlot));
    }
}
/**
 * @brief Inicializa el controlador del LED como salida PWM.
 *
 * Configura los pines de los LEDs y el control PWM utilizando el controlador LEDC.
 *
 * @return ESP_OK si la inicialización es exitosa, ESP_FAIL si hay un error.
 */
esp_err_t InitLed(void) // Función para configurar el LED como salida PWM
{
    // Configurar los pines de los LEDs como salidas
    gpio_reset_pin(CLed);
    gpio_set_direction(CLed, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledR);
    gpio_set_direction(ledR, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledG);
    gpio_set_direction(ledG, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledB);
    gpio_set_direction(ledB, GPIO_MODE_OUTPUT);

    // Configurar LEDC para el control PWM de los LEDs
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_10_BIT, // Resolución de 10 bits para un rango de 0-1023
        .freq_hz = 5000,                      // Frecuencia PWM de 5 kHz
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK};

    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = 0, // Valor inicial del ciclo de trabajo
        .gpio_num = CLed,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0};

    ledc_channel_config_t ledc_channelB = {
        .channel = LEDC_CHANNEL_1,
        .duty = 0, // Valor inicial del ciclo de trabajo
        .gpio_num = ledB,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0};

    ledc_channel_config_t ledc_channelG = {
        .channel = LEDC_CHANNEL_2,
        .duty = 0, // Valor inicial del ciclo de trabajo
        .gpio_num = ledG,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0};

    ledc_channel_config_t ledc_channelR = {
        .channel = LEDC_CHANNEL_3,
        .duty = 0, // Valor inicial del ciclo de trabajo
        .gpio_num = ledR,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_0};
    ledc_timer_config(&ledc_timer);
    ledc_channel_config(&ledc_channel);
    ledc_channel_config(&ledc_channelB);
    ledc_channel_config(&ledc_channelG);
    ledc_channel_config(&ledc_channelR);

    ESP_LOGI(tag, "LED's inicializados para control PWM");
    return ESP_OK;
}
/**
 * @brief Establece la intensidad del LED.
 *
 * @param channel Canal del LED.
 * @param intensity Intensidad del LED en porcentaje (de 10 a 100).
 * @return ESP_OK si la configuración es válida, ESP_FAIL si hay un error.
 */
esp_err_t SetIntensity(ledc_channel_t channel, int intensity)
{
    if (intensity < MinLIntensity || intensity > MaxLIntensity)
    {
        ESP_LOGE(tag, "Valor de intensidad fuera de rango");
        return ESP_FAIL;
    }
    else
    {
        uint32_t duty = (intensity * 1023) / MaxLIntensity;

        ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);

        ESP_LOGI(tag, "Intensidad de LED configurada: %d%%", intensity);
        return ESP_OK;
    }
}
/**
 * @brief Tarea para controlar los LEDs basándose en la temperatura y comandos UART.
 *
 * Esta función recibe temperaturas de la cola `ADCLQ` y comandos de la cola `SetValuesQ`
 * para controlar la intensidad de los LEDs según las temperaturas y los límites configurados.
 *
 * @param pvParameters Puntero a los parámetros pasados a la tarea (no utilizado en esta función).
 */
void SwLED(void *pvParameters)
{
    char *Mensaje_guia = "Comandos permitido:\n LED_X_MAX=NUM\n LED_X_MIN=NUM\nEn donde: X->R,G o B\tNUM->Número";

    int Max_ledB = RMaxLedB;
    int Min_ledB = RMinLedB;
    int Max_ledG = RMaxLedG;
    int Min_ledG = RMinLedG;
    int Max_ledR = RMaxLedR;
    int Min_ledR = RMinLedR;

    while (1)
    {
        float receibedValue = 0, Sumatoria_temp = 0;

        for (size_t i = 0; i < MaxFrecuencyPlot; i++)
        {
            Sumatoria_temp += xQueueReceive(ADCLQ, &receibedValue, pdMS_TO_TICKS(200)) ? receibedValue : printf("\x1b[31mError receiving value from queue\x1b[0m\n");
        }
        float Temperatura = Sumatoria_temp / MaxFrecuencyPlot;

        Dict_set_values values;
        if (xQueueReceive(SetValuesQ, &values, pdMS_TO_TICKS(MaxFrecuencyPlot)) == pdTRUE)
        {
            char *KEY = (const char *)values.key;
            int contador = 0;
            !strcmp(KEY, "LED_B_MAX") ? Max_ledB = values.value, uart_write_bytes(UART_NUM_2, "Valor máximo de LED BLUE asignado", strlen("Valor máximo de LED BLUE asignado")) : contador++;
            !strcmp(KEY, "LED_B_MIN") ? Min_ledB = values.value, uart_write_bytes(UART_NUM_2, "Valor mínimo de LED BLUE asignado", strlen("Valor mínimo de LED BLUE asignado")) : contador++;
            !strcmp(KEY, "LED_G_MAX") ? Max_ledG = values.value, uart_write_bytes(UART_NUM_2, "Valor máximo de LED GREEN asignado", strlen("Valor máximo de LED GREEN asignado")) : contador++;
            !strcmp(KEY, "LED_G_MIN") ? Min_ledG = values.value, uart_write_bytes(UART_NUM_2, "Valor mínimo de LED GREEN asignado", strlen("Valor mínimo de LED GREEN asignado")) : contador++;
            !strcmp(KEY, "LED_R_MAX") ? Max_ledR = values.value, uart_write_bytes(UART_NUM_2, "Valor máximo de LED RED asignado", strlen("Valor máximo de LED RED asignado")) : contador++;
            !strcmp(KEY, "LED_R_MIN") ? Min_ledR = values.value, uart_write_bytes(UART_NUM_2, "Valor mínimo de LED RED asignado", strlen("Valor mínimo de LED RED asignado")) : contador++;
            contador == 6 ? uart_write_bytes(UART_NUM_2, Mensaje_guia, strlen(Mensaje_guia)) : NULL;
        }
        vTaskDelay(pdMS_TO_TICKS(CurrentFrequency * 1000)); // Esperar según la frecuencia actual en segundos

        // Control de LEDS :
        SetIntensity(LEDC_CHANNEL_1, (Temperatura >= Min_ledB && Temperatura <= Max_ledB) ? IntensityLed : 0);
        SetIntensity(LEDC_CHANNEL_2, (Temperatura >= Min_ledG && Temperatura <= Max_ledG) ? IntensityLed : 0);
        SetIntensity(LEDC_CHANNEL_3, (Temperatura >= Min_ledR && Temperatura <= Max_ledR) ? IntensityLed : 0);
        SetIntensity(LEDC_CHANNEL_0, ((Temperatura >= Min_ledB && Temperatura <= Max_ledB) || (Temperatura >= Min_ledG && Temperatura <= Max_ledG) || (Temperatura >= Min_ledR && Temperatura <= Max_ledR)) ? IntensityLed : 0);
        gpio_set_level(CLed, (Temperatura >= Min_ledB && Temperatura <= Max_ledB) || (Temperatura >= Min_ledG && Temperatura <= Max_ledG) || (Temperatura >= Min_ledR && Temperatura <= Max_ledR));
        gpio_set_level(ledB, (Temperatura >= Min_ledB && Temperatura <= Max_ledB));
        gpio_set_level(ledG, (Temperatura >= Min_ledG && Temperatura <= Max_ledG));
        gpio_set_level(ledR, (Temperatura >= Min_ledR && Temperatura <= Max_ledR));

        ESP_LOGI(tag, "Temperatura= %.1f °C", Temperatura);
    }
}
/**
 * @brief Tarea para manejar comandos UART.
 *
 * Esta función recibe comandos UART, los interpreta y realiza acciones correspondientes
 * como ajustar la intensidad del LED.
 *
 * @param pvParameters Puntero a los parámetros pasados a la tarea (no utilizado aquí).
 */
void UartTask(void *pvParameters)
{
    uart_event_t event;
    uint8_t *data = (uint8_t *)malloc(BufSize);

    while (1)
    {
        if (xQueueReceive(UartQ, (void *)&event, portMAX_DELAY))
        {
            bzero(data, BufSize);

            switch (event.type)
            {
            case UART_DATA:
                uart_read_bytes(UartNUM, data, event.size, pdMS_TO_TICKS(500));
                uart_write_bytes(UartNUM, (const char *)data, event.size);
                uart_flush(UartNUM);

                char *ptr = strrchr((const char *)data, '=');

                if (ptr != NULL)
                {
                    char *data_whitout_LB = strtok((const char *)data, "\n");
                    char *data_key = strtok(data_whitout_LB, "=");

                    if (strcmp(data_key, "IntensityLed") == MinLIntensity)
                    {
                        char *data_value = strtok(NULL, "=");
                        int new_intensity = atoi(data_value);

                        if (new_intensity >= MinLIntensity && new_intensity <= MaxLIntensity)
                        {
                            IntensityLed = new_intensity;
                            ledc_set_duty(CLed, IntensityLed * 10, 1); // Conversión : intensidad - pwm
                            gpio_set_level(CLed, 1);
                            uart_write_bytes(UartNUM, "LED intensity set", strlen("LED intensity set"));
                        }
                        else
                        {
                            uart_write_bytes(UartNUM, "Valor de intensidad fuera de rango", strlen("Valor de intensidad fuera de rango"));
                        }
                    }
                    else
                    {
                        ESP_LOGE(tag_UART, "Comando no reconocido");
                    }
                }
                else
                {
                    ESP_LOGE(tag_UART, "Estructura de entrada incorrecta!");
                }

                break;

            default:
                break;
            }
        }
    }
}
/**
 * @brief Tarea para manejar las interrupciones del botón.
 *
 * Esta función se activa cuando se presiona el botón y cambia la frecuencia de impresión.
 *
 * @param pvParameters Puntero a los parámetros pasados a la tarea (no utilizado aquí).
 */
void ButtonTask(void *pvParameters)
{
    gpio_set_direction(button, GPIO_MODE_INPUT);     // Configurar el pin del botón como entrada
    gpio_set_pull_mode(button, GPIO_PULLUP_ONLY);    // Resistencia de pull up
    gpio_set_intr_type(button, GPIO_INTR_NEGEDGE);   // Configurar el manejador de interrupciones para flanco de bajada
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);  // Instalar el servicio de manejo de interrupciones GPIO
    gpio_isr_handler_add(button, ButtonPress, NULL); // Adjuntar el manejador de interrupciones al pin del botón

    int new_frequency;
    while (1)
    {
        if (xQueueReceive(ButtonQ, &new_frequency, portMAX_DELAY)) // Esperar a que llegue una nueva frecuencia desde la interrupción del botón
        {
            CurrentFrequency = new_frequency;
            ESP_LOGI(tag, "Frecuencia de impresión cambiada a %d segundos", CurrentFrequency);
        }
    }
}
/**
 * @brief Función principal que inicializa las colas y crea las tareas necesarias para la ejecución del programa.
 */
void app_main(void)
{
    // Creación de colas
    ADCLQ = xQueueCreate(LenghtADCQ, sizeof(float));                       // Cola para la lectura de ADC
    SetValuesQ = xQueueCreate(LenghtRangeValuesQ, sizeof(RangeValuesRGB)); // Cola para el establecimiento de valores
    ButtonQ = xQueueCreate(LenghtButtonQ, sizeof(int));                    // Cola para el botón

    // Llamada de las principales funciones para ejecutar el programa correctamente:
    InitLed();
    SetADC();
    InitUART();
    Tasks();

    xTaskCreate(ButtonTask, "button_task", SizeBufferT, NULL, 3, NULL);
}
/**
 * @brief Inicializa el controlador UART.
 *
 * Esta función configura el controlador UART con la velocidad de baudios, bits de datos, paridad, bits de parada y otros
 * parámetros especificados en la estructura `uart_config`. También configura los pines del transmisor y el receptor UART.
 *
 * @return ESP_OK si la inicialización es exitosa, ESP_FAIL si hay un error.
 */
esp_err_t InitUART(void)
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .parity = UART_PARITY_DISABLE,
        .source_clk = UART_SCLK_APB,
        .stop_bits = UART_STOP_BITS_1};

    uart_param_config(UartNUM, &uart_config);
    uart_set_pin(UartNUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UartNUM, BufSize, BufSize, MaxFrecuencyPlot, &UartQ, 0);

    ESP_LOGI(tag_UART, "Init uart completed");
    return ESP_OK;
}
/**
 * @brief Crea las tareas en el sistema operativo.
 *
 * Esta función crea las tareas necesarias para el funcionamiento del sistema.
 *
 * @return ESP_OK si la creación de tareas es exitosa, ESP_FAIL si hay un error.
 */
esp_err_t Tasks(void)
{
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    xTaskCreate(GetADC,
                "get_ADC",
                SizeBufferT,
                &ucParameterToPass,
                2,
                &xHandle);
    xTaskCreate(SwLED,
                "switch_LED",
                SizeBufferT,
                &ucParameterToPass,
                1,
                &xHandle);
    xTaskCreate(UartTask,
                "uart_task",
                SizeBufferT,
                &ucParameterToPass,
                5,
                &xHandle);

    ESP_LOGI(tag_task, "Tasks created");
    return ESP_OK;
}
/**
 * @brief Configura el canal ADC y la resolución del conversor ADC.
 *
 * Esta función configura el canal de entrada del ADC y la resolución del conversor ADC.
 *
 * @return ESP_OK si la configuración del ADC es exitosa, ESP_FAIL si hay un error.
 */
esp_err_t SetADC(void)
{
    adc1_config_channel_atten(NtcPIN, ADC_ATTEN_DB_11); // Canal de entrada analógica con una atenuación específica
    adc1_config_width(ADC_WIDTH_BIT_12);                // Resolución deseada para el conversor AC/DC

    ESP_LOGI(tag, "ADC configured");
    return ESP_OK;
}