#include "ConfigStr.h"
#include "tasks.h"

// main app
void app_main(void)
{
    setPins();
    setADC();
    setPWM();
    changeLed(); // turn on first led
    xTaskCreate(buttonTask, "Button task", 2048, NULL, 10, NULL);
    setLLuminosity();
}
