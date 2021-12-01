#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sdk_common.h"
#include "application.h"

#include "nrf.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_twi.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_timer.h"
#include "nrf_gpio.h"
#include "bsp.h"
#include "nordic_common.h"
#include "board_conf.h"
#include "device_LED_indicator.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "app_timer.h"
#include "bsp_nfc.h"
#include "nrf_drv_clock.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_log.h"
#include "ble_cus.h"

#include "nrf_delay.h"
#include "bsp.h"
#include "RingBuffer.h"

static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
static const nrf_drv_timer_t TIMER_SIGNAL_3 = NRF_DRV_TIMER_INSTANCE(TIMER_INSTANCE_ID_3);
// static const nrf_drv_timer_t TIMER_SIGNAL_2 = NRF_DRV_TIMER_INSTANCE(TIMER_INSTANCE_ID_2);

APP_TIMER_DEF(m_btn_timer_id);

static volatile unsigned button_hold_time = 0;      	// Number of cycles the button has been held for.
static unsigned cycle_count               = 0;			// Number of pulses
static unsigned _button_hold_time         = 0;      	// Number of cycles the button has been held for (counter).
#define time_us_on_HILF                100          	/**< Signal on time in us. */
#define time_us_period_HILF            700          	/**< Signal period in us. */
#define SAMPLES_IN_BUFFER                1				/**< Number of samples in the buffer. */
static nrf_saadc_value_t m_buffer_pool[2][SAMPLES_IN_BUFFER];
static volatile unsigned LIHF_flag             = false; /**< flag for triggering LIHF signal */
static volatile unsigned counter               = 0;     /**< counter for how many pulses happened in 25ms */
static volatile uint8_t  check_saadc_enable    = 0;     /**< check whether saadc is enabled or not cuz it sure cannot be enbaled twice. */

// static enum {
//     POSITIVE,
//     NEGATIVE
// } polarity;
//
// static enum {
//     OFF,
//     ON
// } toggle;

// inverted logic, see schematic
#define PIN_set(pin)   nrf_gpio_pin_clear(pin)
#define PIN_clear(pin) nrf_gpio_pin_set(  pin)


/**
 * @brief Function for defining the input pin handler - turning on and off the device
 */
static void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
    _button_hold_time = 0;
    app_timer_start(m_btn_timer_id, BTN_INTERVAL, NULL);
}

/**@brief Function for handling the Battery measurement timer timeout.
 *
 * @details This function will be called each time the button timer expires.
 *
 * @param[in] p_context  Pointer used for passing some arbitrary information (context) from the
 *                       app_start_timer() call to the timeout handler.
 */
static void btn_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
//     ret_code_t err_code;

    if (nrf_gpio_pin_read(PIN_IN)) {
        if (_button_hold_time < 40)
            _button_hold_time++;
        else {
// 			NRF_LOG_INFO("turning off the LED");
			m_device_led = DEVICE_INDICATE_IDLE;
			device_indication_set(m_device_led);
		}
    }
    else {
        app_timer_stop(m_btn_timer_id);
        button_hold_time = _button_hold_time;
        _button_hold_time = 0;
    }
    return;
}

unsigned check_button_time() {
    unsigned r = button_hold_time;
    button_hold_time = 0;
    return r;
}

/**
 * @brief GPIO initialization.
 */
void gpiote_init (void){
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

//     nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(true);

//     err_code = nrf_drv_gpiote_out_init(PIN_OUT, &out_config);
//     APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config.pull = NRF_GPIO_PIN_NOPULL;

    err_code = nrf_drv_gpiote_in_init(PIN_IN, &in_config, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(PIN_IN, true); // for when NRF is running


    // APP_TIMER should be initialized already
    err_code = app_timer_create(&m_btn_timer_id, APP_TIMER_MODE_REPEATED, btn_timeout_handler);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("GPIOTE is enabled successfully. \r\n");

}

/**
 * @brief GPIO SIGNAL initialization.
 */
void gpio_signal_init (void){
    nrf_gpio_cfg_output(SIGNAL_1);
    nrf_gpio_cfg_output(SIGNAL_2);
    nrf_gpio_cfg_output(SIGNAL_3);
    nrf_gpio_cfg_output(SIGNAL_4);
    nrf_gpio_cfg_output(SD);
    nrf_gpio_cfg_output(BOOST);
	nrf_gpio_cfg_output(LED_PIN);
	nrf_gpio_cfg_output(battery_measure);

    // Set all the signal pins to 0 at their init states
    PIN_clear(SIGNAL_1);
    PIN_clear(SIGNAL_2);
    PIN_clear(SIGNAL_3);
    PIN_clear(SIGNAL_4);
    nrf_gpio_pin_clear(BOOST);
    nrf_gpio_pin_clear(SD);
	nrf_gpio_pin_set(LED_PIN);
	nrf_gpio_pin_clear(battery_measure);

    NRF_LOG_INFO("GPIOs are enabled successfully. \r\n");
}

/**
 * @brief TWI initialization.
 */
void twi_init (void){
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
        .scl                = ARDUINO_SCL_PIN,
        .sda                = ARDUINO_SDA_PIN,
        .frequency          = NRF_TWI_FREQ_400K,
        .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
        .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);

    // Send initial value to disable DAC and wait for instruction
//     uint8_t * buff = (0<<2);
//     err_code = nrf_drv_twi_rx(&m_twi, dac_address, buff, sizeof(buff));
//     APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("DAC is enabled successfully. \r\n");
}

/**
 * @brief Intensity update and tuning on and off the device with the phone APP.
 * @param[in] value Intensity value/ controlling value sent from the phone App.
 */
void twi_update (uint8_t value){
    ret_code_t err_code;

    err_code = nrf_drv_twi_tx(&m_twi, dac_address, &value, 1, false);
    APP_ERROR_CHECK(err_code);

//     NRF_LOG_INFO("Intensity updated successfully. \r\n");
}

/**
 * @brief TWI Disabling.
 */
void twi_disable (void){
    ret_code_t err_code;

    uint8_t buff[1];
    err_code = nrf_drv_twi_rx(&m_twi, dac_address, buff, 1);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("DAC is disabled successfully. \r\n");
}


// volatile uint8_t hilf_timer_event = 0;

/**
 * @brief Handler for timer events.
 */
static void device_timer_handler_HILF(nrf_timer_event_t event_type, void* p_context)
{

// 	__asm("bkpt");
//     if (event_type == NRF_TIMER_EVENT_COMPARE0) {
//         hilf_timer_event = 1;
//     }
//     else if (event_type == NRF_TIMER_EVENT_COMPARE1) {
//         hilf_timer_event = 2;
//     }
//     else if (event_type == NRF_TIMER_EVENT_COMPARE3) {
//         hilf_timer_event = 3;
//     }

	if (event_type == NRF_TIMER_EVENT_COMPARE0) {
		cycle_count++;

		PIN_clear(SIGNAL_1);
		PIN_clear(SIGNAL_3);
		PIN_clear(SIGNAL_2);
		PIN_clear(SIGNAL_4);
// 		NRF_LOG_INFO("Beginning of timer1 \n");
	}
	if (cycle_count & 4) {
		switch (event_type) {
			case NRF_TIMER_EVENT_COMPARE0:
				nrf_gpio_pin_set(BOOST);
// 				NRF_LOG_INFO("Set Boost Converter \n");
				break;
			case NRF_TIMER_EVENT_COMPARE1:
				PIN_set(SIGNAL_1);
				PIN_set(SIGNAL_3);
				break;
			case NRF_TIMER_EVENT_COMPARE2:
				PIN_clear(SIGNAL_1);
				PIN_clear(SIGNAL_3);

				PIN_set(SIGNAL_2);
				PIN_set(SIGNAL_4);
				break;
			case NRF_TIMER_EVENT_COMPARE3:
				PIN_clear(SIGNAL_1);
				PIN_clear(SIGNAL_3);
				PIN_clear(SIGNAL_2);
				PIN_clear(SIGNAL_4);
// 				nrf_gpio_pin_clear(BOOST);
				break;
			case NRF_TIMER_EVENT_COMPARE4:
// 				NRF_LOG_INFO("Clear Timer \n");
				break;
			case NRF_TIMER_EVENT_COMPARE5:
				break;
		}
	} else {
		nrf_gpio_pin_clear(BOOST);
// 		NRF_LOG_INFO("Clear boost \n");
	}
}

void device_timer_idle() {
}

/**
 * @brief TIMER initialization.
 */
void device_timer_init (void){
    ret_code_t err_code;

    err_code = NRF_SUCCESS;

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;

    err_code = nrf_drv_timer_init(&TIMER_SIGNAL_3, &timer_cfg, device_timer_handler_HILF);
	APP_ERROR_CHECK(err_code);
//     err_code = nrf_drv_timer_init(&TIMER_SIGNAL_2, &timer_cfg, zens_timer_handler_LIHF);
//     APP_ERROR_CHECK(err_code);

    NRF_LOG_INFO("TIMER is configured");
}

/**
 * for LIHF the dutycycle is 50% for now
 */
static void device_timer_enable(unsigned width_us_HILF, unsigned period_ms_HILF, unsigned period_ms_LIHF) {
    ASSERT(period_ms_HILF > width_us_HILF);

    // sanity check
    nrf_drv_timer_disable(&TIMER_SIGNAL_3);
//     nrf_drv_timer_disable(&TIMER_SIGNAL_2);
	//TODO: Change the magic numbers below to set the frequency for the waveform.

	nrf_drv_timer_compare(          &TIMER_SIGNAL_3, NRF_TIMER_CC_CHANNEL0, nrf_drv_timer_us_to_ticks(&TIMER_SIGNAL_3, 2             ),                                      true );
	nrf_drv_timer_compare(          &TIMER_SIGNAL_3, NRF_TIMER_CC_CHANNEL1, nrf_drv_timer_us_to_ticks(&TIMER_SIGNAL_3, 2002            ),                                    true );
    nrf_drv_timer_compare(          &TIMER_SIGNAL_3, NRF_TIMER_CC_CHANNEL2, nrf_drv_timer_us_to_ticks(&TIMER_SIGNAL_3, width_us_HILF ),                                      true );
    nrf_drv_timer_compare(          &TIMER_SIGNAL_3, NRF_TIMER_CC_CHANNEL3, nrf_drv_timer_us_to_ticks(&TIMER_SIGNAL_3, 2202           ),                                     true );
    nrf_drv_timer_extended_compare( &TIMER_SIGNAL_3, NRF_TIMER_CC_CHANNEL4, nrf_drv_timer_ms_to_ticks(&TIMER_SIGNAL_3, period_ms_HILF), NRF_TIMER_SHORT_COMPARE4_CLEAR_MASK, true );

    nrf_drv_timer_enable(&TIMER_SIGNAL_3);
//     nrf_drv_timer_enable(&TIMER_SIGNAL_2);

    NRF_LOG_INFO("TIMER is enabled successfully. \r\n");
    NRF_LOG_INFO("Current HILF PW is %d \n", width_us_HILF);
    NRF_LOG_INFO("Current HILF Period is %d \n", period_ms_HILF);
    NRF_LOG_INFO("Current LIHF PW is %d \n", period_ms_LIHF);
}

void device_timer_update(uint8_t width_us_HILF, uint8_t period_ms_HILF, uint8_t period_ms_LIHF) {
    if (width_us_HILF != 0 && period_ms_HILF != 0 && period_ms_LIHF != 0) {
        device_timer_enable(width_us_HILF, period_ms_HILF, period_ms_LIHF);
    }
    else {
        device_timer_enable(2102, 25, 100); // 100us width and 700us period
    }
}



void device_timer_disable (void){
    nrf_drv_timer_disable(&TIMER_SIGNAL_3);
//     nrf_drv_timer_disable(&TIMER_SIGNAL_2);

    PIN_clear(SIGNAL_1);
    PIN_clear(SIGNAL_3);
    PIN_clear(SIGNAL_2);
    PIN_clear(SIGNAL_4);

    NRF_LOG_INFO("TIMER is disabled successfully. \r\n");
}

static void saadc_callback (nrf_drv_saadc_evt_t const* p_event) {
// 	NRF_LOG_INFO("ADC evt %d\n", p_event->type);
	if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
	{
// 		ret_code_t err_code;
// 		for (int i = 0; i < SAMPLES_IN_BUFFER; i++)
// 		{
// 			NRF_LOG_INFO("ADC data %d read as follows: %d", i, p_event->data.done.p_buffer[i]);
// 		}

// 		NRF_LOG_INFO("Start battery level update. \n");
		battery_level_update(p_event->data.done.p_buffer[0]);

		// Disable saadc
		nrf_saadc_disable();

		// Clear the saadc enbale state
		check_saadc_enable = 0;

		// Clear the battery_measure pin
		nrf_gpio_pin_clear(battery_measure);
	}
}

void saadc_init(void){
	ret_code_t err_code;
	nrf_saadc_channel_config_t channel_config =
	NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);

	err_code = nrf_drv_saadc_init(NULL, saadc_callback);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_saadc_channel_init(0, &channel_config);
	APP_ERROR_CHECK(err_code);
}

void device_battery_measure (void) {
	ret_code_t err_code;

	nrf_gpio_pin_set(battery_measure);
	if (check_saadc_enable == 0)
		nrf_saadc_enable();
	check_saadc_enable = 1;
// 	NRF_LOG_INFO("ADC is enabled to take a snapshot of the current battery capacity. \n");

	err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_saadc_sample();
	APP_ERROR_CHECK(err_code);

// 	NRF_LOG_INFO("Asked for sampling. \n");

}
