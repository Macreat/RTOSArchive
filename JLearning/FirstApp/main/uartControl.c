#include "uartControl.h"

QueueHandle_t uart_queue; // Variable for the queue of events in the UART

void init_uart(void) // Initialize the UART port
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

void send_time_UART(void) // Send current time through UART port
{
    while (1)
    {
        // wait for time to be set
        static time_t now = 0;
        static struct tm timeinfo = {0};

        // get current hour
        time(&now);

        char strftime_buf[64];

        setenv("TZ", "GTM+5", 1); // Colombia's time zone is established
        tzset();
        localtime_r(&now, &timeinfo); // Convert time information from one format to another

        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);                // Convert the current time to a human-readable string format
        ESP_LOGI(tag, "The current date/time in Colombia is: %s", strftime_buf);      //
        uart_write_bytes(UART_NUM, strcat(strftime_buf, "\n"), strlen(strftime_buf)); // The string containing the time is sent through the UART

        vTaskDelay(pdMS_TO_TICKS(1000)); // delay of one sec
    }
}