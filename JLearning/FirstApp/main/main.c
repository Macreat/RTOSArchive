#include "blinkStr.h"
#include "tasks.h"

void app_main(void)
{
    buttonQueue = xQueueCreate(10, sizeof(int));

    xTaskCreate(mainTask, "button Task uart", 2048, NULL, 10, NULL);

    setPins();
}
