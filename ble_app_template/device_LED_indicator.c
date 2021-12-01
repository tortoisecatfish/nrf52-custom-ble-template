#include "device_LED_indicator.h"

#include "board_conf.h"
#include "nrf_gpio.h"
#include "app_timer.h"

// NRF LOG STUFF
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

static uint8_t volatile timer_state = 0;

APP_TIMER_DEF(m_led_timer_id);

static void led_timeout_handler(void * p_context)
{
	UNUSED_PARAMETER(p_context);
	device_indication_set(m_device_led);
}

static void led_timer_init(void) {

	ret_code_t err_code;
	// TODO: Add sanity check for timer init

	err_code = app_timer_create(&m_led_timer_id, APP_TIMER_MODE_SINGLE_SHOT, led_timeout_handler);
	APP_ERROR_CHECK(err_code);
}

void device_indication_clear() {
	app_timer_stop(m_led_timer_id);
}

void device_indication_set(device_led_state_t z_evt) {

	uint32_t next_delay = 0;
	ret_code_t err_code;

	switch (z_evt)
	{
		case DEVICE_INDICATE_ADVERTISING:
			if ((nrf_gpio_pin_out_read(LED_PIN) ? true : false) == true)
			{
				nrf_gpio_pin_clear(LED_PIN);
				next_delay = 200;
			}
			else
			{
				nrf_gpio_pin_set(LED_PIN);
				next_delay = 1800;
			}

			if (timer_state != 0)
				device_indication_clear();

			err_code = app_timer_start(m_led_timer_id, APP_TIMER_TICKS(next_delay), NULL);
			timer_state = 1;
// 			NRF_LOG_INFO("ZENS_INDICATE_ADVERTISING \n");

			APP_ERROR_CHECK(err_code);
			break;

		case DEVICE_INDICATE_CONNECTED:
			if (timer_state != 0)
				device_indication_clear();
			nrf_gpio_pin_clear(LED_PIN);
// 			NRF_LOG_INFO("ZENS_INDICATE_CONNECTED \n");

			break;

		case DEVICE_INDICATE_TREATMENT_ON:
			app_timer_stop(m_led_timer_id);
			if ((nrf_gpio_pin_out_read(LED_PIN) ? true : false) == true)
			{
				nrf_gpio_pin_clear(LED_PIN);
				next_delay = 2000;
			}
			else
			{
				nrf_gpio_pin_set(LED_PIN);
				next_delay = 1000;
			}

			if (timer_state != 0)
				device_indication_clear();

			err_code = app_timer_start(m_led_timer_id, APP_TIMER_TICKS(next_delay), NULL);
			timer_state = 1;
// 			NRF_LOG_INFO("ZENS_INDICATE_TREATMENT_ON \n");

			APP_ERROR_CHECK(err_code);
			break;

		case DEVICE_INDICATE_DISCONNECTED_TREATMENT_ON:
			break;

		case DEVICE_INDICATE_IDLE:
			if (timer_state != 0)
				device_indication_clear();
			nrf_gpio_pin_set(LED_PIN);
// 			NRF_LOG_INFO("ZENS_INDICATE_IDLE \n");

	}

}

void device_indication_init() {
	led_timer_init();
}
