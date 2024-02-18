/*
 * rgb_led.c
 *
 *  Created on: Oct 11, 2021
 *      Author: kjagu
 */

#include <stdbool.h>

#include "driver/ledc.h"
#include "rgb_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h" //Librería para configurar los timers

// RGB LED Configuration Array
ledc_info_t ledc_ch[RGB_LED_CHANNEL_NUM];

// handle for rgb_led_pwm_init
bool g_pwm_init_handle = false;

// handle for rgb_led_init

bool g_LEDs_init_handle = false;

/**
 * Initializes the RGB LED settings per channel, including
 * the GPIO for each color, mode and timer configuration.
 */
static void rgb_led_pwm_init(void)
{
	int rgb_ch;

	// Red
	ledc_ch[0].channel = LEDC_CHANNEL_0;
	ledc_ch[0].gpio = RGB_LED_RED_GPIO;
	ledc_ch[0].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[0].timer_index = LEDC_TIMER_0;

	// Green
	ledc_ch[1].channel = LEDC_CHANNEL_1;
	ledc_ch[1].gpio = RGB_LED_GREEN_GPIO;
	ledc_ch[1].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[1].timer_index = LEDC_TIMER_0;

	// Blue
	ledc_ch[2].channel = LEDC_CHANNEL_2;
	ledc_ch[2].gpio = RGB_LED_BLUE_GPIO;
	ledc_ch[2].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[2].timer_index = LEDC_TIMER_0;

	// Red
	ledc_ch[3].channel = LEDC_CHANNEL_3;
	ledc_ch[3].gpio = RGB_LED_RED_GPIO_TEXT;
	ledc_ch[3].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[3].timer_index = LEDC_TIMER_0;

	// Green
	ledc_ch[4].channel = LEDC_CHANNEL_4;
	ledc_ch[4].gpio = RGB_LED_GREEN_GPIO_TEXT;
	ledc_ch[4].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[4].timer_index = LEDC_TIMER_0;

	// Blue
	ledc_ch[5].channel = LEDC_CHANNEL_4;
	ledc_ch[5].gpio = RGB_LED_BLUE_GPIO_TEXT;
	ledc_ch[5].mode = LEDC_HIGH_SPEED_MODE;
	ledc_ch[5].timer_index = LEDC_TIMER_0;

	// Configure timer zero
	ledc_timer_config_t ledc_timer =
		{
			.duty_resolution = LEDC_TIMER_8_BIT,
			.freq_hz = 100,
			.speed_mode = LEDC_HIGH_SPEED_MODE,
			.timer_num = LEDC_TIMER_0};
	ledc_timer_config(&ledc_timer);

	// Configure channels
	for (rgb_ch = 0; rgb_ch < 6; rgb_ch++)
	{
		ledc_channel_config_t ledc_channel =
			{
				.channel = ledc_ch[rgb_ch].channel,
				.duty = 0,
				.hpoint = 0,
				.gpio_num = ledc_ch[rgb_ch].gpio,
				.intr_type = LEDC_INTR_DISABLE,
				.speed_mode = ledc_ch[rgb_ch].mode,
				.timer_sel = ledc_ch[rgb_ch].timer_index,
			};
		ledc_channel_config(&ledc_channel);
	}
	g_pwm_init_handle = true;
}

static void toggleLed(int GPIO)
{
	bool state = 0;
	for (int i = 0; i < 6; i++)
	{
		state = !state;
		gpio_set_level(GPIO, state);
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
void setStateLed(int R, int G, int B)
{
	gpio_set_level(RGB_LED_RED_GPIO_TEXT, R);
	gpio_set_level(RGB_LED_GREEN_GPIO_TEXT, G);
	gpio_set_level(RGB_LED_BLUE_GPIO_TEXT, B);

	//(R == 1) ? toggleLed(RGB_LED_RED_GPIO_TEXT) : NULL;
	//(G == 1) ? toggleLed(RGB_LED_GREEN_GPIO_TEXT) : NULL;
	//(B == 1) ? toggleLed(RGB_LED_BLUE_GPIO_TEXT) : NULL;
}
/**
 * Sets the RGB color.
 */
static void rgb_led_set_pwm_color(uint8_t red, uint8_t green, uint8_t blue)
{
	// Value should be 0 - 255 for 8 bit number
	ledc_set_duty(ledc_ch[0].mode, ledc_ch[0].channel, red);
	ledc_update_duty(ledc_ch[0].mode, ledc_ch[0].channel);

	ledc_set_duty(ledc_ch[1].mode, ledc_ch[1].channel, green);
	ledc_update_duty(ledc_ch[1].mode, ledc_ch[1].channel);

	ledc_set_duty(ledc_ch[2].mode, ledc_ch[2].channel, blue);
	ledc_update_duty(ledc_ch[2].mode, ledc_ch[2].channel);
}

static void rgb_led_set_pwm_color2(uint8_t red2, uint8_t green2, uint8_t blue2)
{
	// Value should be 0 - 255 for 8 bit number
	ledc_set_duty(ledc_ch[3].mode, ledc_ch[3].channel, red2);
	ledc_update_duty(ledc_ch[3].mode, ledc_ch[3].channel);

	ledc_set_duty(ledc_ch[4].mode, ledc_ch[4].channel, green2);
	ledc_update_duty(ledc_ch[4].mode, ledc_ch[4].channel);

	ledc_set_duty(ledc_ch[5].mode, ledc_ch[5].channel, blue2);
	ledc_update_duty(ledc_ch[5].mode, ledc_ch[5].channel);
}
static void rgb_led_init(void)
{
	gpio_reset_pin(RGB_LED_RED_GPIO);
	gpio_reset_pin(RGB_LED_GREEN_GPIO);
	gpio_reset_pin(RGB_LED_BLUE_GPIO);

	gpio_set_direction(RGB_LED_RED_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_direction(RGB_LED_GREEN_GPIO, GPIO_MODE_OUTPUT);
	gpio_set_direction(RGB_LED_BLUE_GPIO, GPIO_MODE_OUTPUT);

	gpio_reset_pin(RGB_LED_RED_GPIO_TEXT);
	gpio_reset_pin(RGB_LED_GREEN_GPIO_TEXT);
	gpio_reset_pin(RGB_LED_BLUE_GPIO_TEXT);

	gpio_set_direction(RGB_LED_RED_GPIO_TEXT, GPIO_MODE_OUTPUT);
	gpio_set_direction(RGB_LED_GREEN_GPIO_TEXT, GPIO_MODE_OUTPUT);
	gpio_set_direction(RGB_LED_BLUE_GPIO_TEXT, GPIO_MODE_OUTPUT);

	g_LEDs_init_handle = true;
}

void rgb_led_wifi_app_started(void)
{
	(g_LEDs_init_handle == false) ? rgb_led_init() : NULL;
	setStateLed(1, 0, 0);
}

void rgb_led_http_server_started(void)
{
	(g_LEDs_init_handle == false) ? rgb_led_init() : NULL;
	setStateLed(0, 0, 1);
}

void rgb_led_wifi_connected(void)
{
	(g_LEDs_init_handle == false) ? rgb_led_init() : NULL;
	setStateLed(0, 1, 0);
}

void updateRGB(int R, int G, int B)
{
	(g_pwm_init_handle == false) ? rgb_led_pwm_init() : NULL;
	int RValue = (R * 255) / 100;
	int GValue = (G * 255) / 100;
	int BValue = (B * 255) / 100;
	rgb_led_set_pwm_color(RValue, GValue, BValue);
}

void updateRGB2(int R2, int G2, int B2)
{
	(g_pwm_init_handle == false) ? rgb_led_pwm_init() : NULL;
	int RValue2 = (R2 * 255) / 100;
	int GValue2 = (G2 * 255) / 100;
	int BValue2 = (B2 * 255) / 100;
	rgb_led_set_pwm_color2(RValue2, GValue2, BValue2);
}

void turnOnRedLED()
{
	gpio_set_level(RGB_LED_RED_GPIO_TEXT, 1);
	gpio_set_level(RGB_LED_GREEN_GPIO_TEXT, 0);
	gpio_set_level(RGB_LED_BLUE_GPIO_TEXT, 0);
}

void turnOnGreenLED()
{
	gpio_set_level(RGB_LED_RED_GPIO_TEXT, 0);
	gpio_set_level(RGB_LED_GREEN_GPIO_TEXT, 1);
	gpio_set_level(RGB_LED_BLUE_GPIO_TEXT, 0);
}

void turnOnBlueLED()
{
	gpio_set_level(RGB_LED_RED_GPIO_TEXT, 0);
	gpio_set_level(RGB_LED_GREEN_GPIO_TEXT, 0);
	gpio_set_level(RGB_LED_BLUE_GPIO_TEXT, 1);
}