#include "tasks.h"

QueueHandle_t buttonQueue;
int currentLed = 0;
const char *TAG = "Button task";
bool turnOn[NLeds] = {false, false, false};
int luminosity[NLeds] = {0, 0, 0};

/*
function to set interruption
*/
void IRAM_ATTR buttonIsrHandler(void *arg)
{
    int buttonState = gpio_get_level(button);
    xQueueSendFromISR(buttonQueue, &buttonState, NULL);
}
/*
function in charge to change between leds
*/
void changeLed()
{
    // turn off leds
    for (int i = 0; i < NLeds; i++)
    {
        gpio_set_level(led1 + i, 0);
        turnOn[i] = false;
    }

    // turn on only current led
    gpio_set_level(led1 + currentLed, 1);
    turnOn[currentLed] = true;
    // turn on interruption control led
    gpio_set_level(cled, 1);
    vTaskDelay(RDelay);
    gpio_set_level(cled, 0);
}
/*
function to configurate button tasks
*/

void buttonTask(void *pvParameter)
{
    int previousState = 1; // Previous button state (1 = not pressed, 0 = pressed)
    int counter = 0;       // counter for button task

    while (1)
    {
        int buttonState = gpio_get_level(button);

        if (buttonState != previousState)
        {
            ESP_LOGI("TAG", "Button press... changing led");

            vTaskDelay(20 / portTICK_PERIOD_MS); // to evade reboots

            buttonState = gpio_get_level(button);

            if (buttonState != previousState) // Check if the status is stable
            {
                if (buttonState == 0) //  If the button is pressed
                {
                    counter++;

                    if (counter >= 3) // If pressed three times, restart the cycle
                    {
                        counter = 0;
                        currentLed = 0;
                        changeLed();
                    }
                    else // Change to next LED if not pressed three times
                    {
                        currentLed = (currentLed + 1) % NLeds;
                        changeLed();
                    }
                }
            }
        }

        previousState = buttonState; // Update previous button state
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/*
function to get luminosity in current led
*/
void getLuminosity(int potValue)
{
    int BrightnessObjective = (potValue * 255) / 4095;                      // scalling 0 to 255 the shine
    int actualBrightness = turnOn[currentLed] ? luminosity[currentLed] : 0; // assign luminosity of current led
    int step = 0;
    int totalSteps = 50;

    // Smooth the shine transition with 50 steps
    while (step <= totalSteps)
    {
        int interpolatedBrightness = (actualBrightness * (totalSteps - step) + BrightnessObjective * step) / totalSteps;
        luminosity[currentLed] = interpolatedBrightness;
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0 + currentLed, interpolatedBrightness);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0 + currentLed);
        step++;
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
/*
function to set luminosity in current led
*/
void setLLuminosity()
{
    while (1)
    {
        int potVal = adc1_get_raw(Pot);
        getLuminosity(potVal);

        ESP_LOGI("ADC lecture", "pottentiometer value: %d", potVal);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
