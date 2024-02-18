#include "uart_control.h"

QueueHandle_t uart_queue; // Varible para la cola de los eventos en el UART

void init_uart(void) // Iniciarlizar el puerto UART
{
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .parity = UART_PARITY_DISABLE,
        .source_clk = UART_SCLK_APB,
        .stop_bits = UART_STOP_BITS_1};

    uart_param_config(UART_NUM, &uart_config);

    uart_set_pin(UART_NUM, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, 5, &uart_queue, 0);

    ESP_LOGI(tag, "Init uart completed");
}

void send_time_UART(void) // Envía la hora actual a través del puerto UART
{
    while (1)
    {
        // wait for time to be set
        static time_t now = 0;
        static struct tm timeinfo = {0};

        // Obtiene la hora actual
        time(&now);

        char strftime_buf[64];

        setenv("TZ", "GTM+5", 1); // Se establece la zona horaria de Colombia
        tzset();
        localtime_r(&now, &timeinfo);                                                 // Se convierta la información de la hora de un formato a otro
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);                // Convierte la hora actual a un formato de cadena legible por humanos
        ESP_LOGI(tag, "The current date/time in Colombia is: %s", strftime_buf);      //
        uart_write_bytes(UART_NUM, strcat(strftime_buf, "\n"), strlen(strftime_buf)); // Se envía la cadena que contiene la hora por el UART

        vTaskDelay(pdMS_TO_TICKS(1000)); // Se espera un segundo
    }
}