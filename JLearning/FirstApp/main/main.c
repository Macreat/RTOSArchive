#include "ConfigStr.h"
#include "tasks.h"

// main app just controlling state of a different LEDS with an NTC termistor.
void app_main(void)
{
    setPins();
    setADC();
    setPWM();
    changeLed();
    xTaskCreate(buttonTask, "Button task", 2048, NULL, 10, NULL);
    setLuminosity();
}
