#include "ButtonTask.h"

// Definiciones de los pines de los botones
#define BUTTON_1_GPIO 18
#define BUTTON_2_GPIO 19

// Variables para almacenar las pulsaciones de los botones
extern volatile int button_1_press_count = 0;
extern volatile int button_2_press_count = 0;
static bool last_button_1_state = false;
static bool last_button_2_state = false;

#define DEBOUNCE_TIME_MS 50 // tiempo de debounce en milisegundos

// Variables para almacenar el tiempo de la última pulsación
static uint64_t last_button_1_press_time = 0;
static uint64_t last_button_2_press_time = 0;

static void IRAM_ATTR button_1_isr_handler(void *arg)
{
    uint64_t now = esp_timer_get_time() / 1000;              // Tiempo actual en milisegundos
    bool current_state = gpio_get_level(BUTTON_1_GPIO) == 0; // Estado actual del botón

    if (now - last_button_1_press_time > DEBOUNCE_TIME_MS && !last_button_1_state && current_state)
    {
        button_1_press_count++;
        last_button_1_press_time = now;
    }
    last_button_1_state = current_state;
}

static void IRAM_ATTR button_2_isr_handler(void *arg)
{
    uint64_t now = esp_timer_get_time() / 1000;              // Tiempo actual en milisegundos
    bool current_state = gpio_get_level(BUTTON_2_GPIO) == 0; // Estado actual del botón

    if (now - last_button_2_press_time > DEBOUNCE_TIME_MS && !last_button_2_state && current_state)
    {
        button_2_press_count++;
        last_button_2_press_time = now;
    }
    last_button_2_state = current_state;
}

// Función para inicializar la tarea de botones
void button_task_initialize(void)
{
    // Configurar los pines GPIO como entradas con pull-up
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_ANYEDGE,
        .pin_bit_mask = (1ULL << BUTTON_1_GPIO) | (1ULL << BUTTON_2_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0};
    gpio_config(&io_conf);

    // Instalar el servicio ISR y agregar las interrupciones para cada botón
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_1_GPIO, button_1_isr_handler, (void *)BUTTON_1_GPIO);
    gpio_isr_handler_add(BUTTON_2_GPIO, button_2_isr_handler, (void *)BUTTON_2_GPIO);
}

int get_button_1_press_count()
{
    return button_1_press_count;
}

int get_button_2_press_count()
{
    return button_2_press_count;
}
