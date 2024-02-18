/*
 * rgb_led.h
 *
 *  Created on: Oct 11, 2021
 *      Author: kjagu
 */

#ifndef MAIN_RGB_LED_H_
#define MAIN_RGB_LED_H_

// RGB LED GPIOs
#define RGB_LED_RED_GPIO_TEXT 13
#define RGB_LED_GREEN_GPIO_TEXT 12
#define RGB_LED_BLUE_GPIO_TEXT 14
#define RGB_LED_RED_GPIO 26
#define RGB_LED_GREEN_GPIO 25
#define RGB_LED_BLUE_GPIO 33

// RGB LED color mix channels
#define RGB_LED_CHANNEL_NUM 3

// RGB LED configuration
typedef struct
{
	int channel;
	int gpio;
	int mode;
	int timer_index;
} ledc_info_t;
// ledc_info_t ledc_ch[RGB_LED_CHANNEL_NUM]; Move this declaration to the top of rgb_led.c to avoid linker errors

/**
 * Color to indicate WiFi application has started.
 */
void rgb_led_wifi_app_started(void);

/**
 * Color to indicate HTTP server has started.
 */
void rgb_led_http_server_started(void);

/**
 * Color to indicate that the ESP32 is connected to an access point.
 */
void rgb_led_wifi_connected(void);

void updateRGB(int R, int G, int B);
void updateRGB2(int R2, int G2, int B2);
void turnOnRedLED();
void turnOnGreenLED();
void turnOnBlueLED();
void setStateLed(int R, int G, int B);
#endif /* MAIN_RGB_LED_H_ */
