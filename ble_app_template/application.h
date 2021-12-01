#ifndef APPLICATION_H__
#define APPLICATION_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "nrf_drv_saadc.h"
#include "device_LED_indicator.h"

/********** TWI **********/
#define TWI_INSTANCE_ID             0               /** TWI instance ID. */
#define TWI_ADDRESSES               127             /** Number of possible TWI addresses. */
#define dac_address                 0x30

/********** TIMER **********/
#define TIMER_INSTANCE_ID_3         3               /** Timer instance ID 3. */
// #define TIMER_INSTANCE_ID_2         2               /** Timer instance ID 2. */
#define BTN_INTERVAL                APP_TIMER_TICKS(100) /** Button long push for 100 ms. */

/********** UART **********/
#define MAX_TEST_DATA_BYTES         (15U)           /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE            256             /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE            256
#define UART_HWFC APP_UART_FLOW_CONTROL_DISABLED

// #define BTN_ID_READY                1               /**< ID of the button used to change the readiness to sleep. */
// #define BTN_ID_SLEEP                0               /**< ID of the button used to put the application into sleep/system OFF mode. */
// #define BTN_ID_WAKEUP               0               /**< ID of the button used to wake up the application. */
// #define BTN_ID_RESET                2               /**< ID of the button used to reset the application. */

void battery_level_update(int16_t);

/**
 * @brief GPIOTE initialization.
 */
void gpiote_init (void);

unsigned check_button_time(void);

/**
 * @brief Intensity update and tuning on and off the device with the phone APP.
 * @param[in] value Intensity value/ controlling value sent from the phone App.
 */
void twi_update (uint8_t);

/**
 * @brief TWI initialization.
 */
void twi_init (void);

/**
 * @brief TWI Disabling.
 */
void twi_disable (void);

/**
 * @brief TIMER initialization.
 */
void device_timer_init (void);

/**
 * @brief TIMER update.
 */

void device_timer_update (uint8_t width_us_HILF, uint8_t period_ms_HILF, uint8_t period_ms_LIHF);

/**
 * @brief TIMER disabling.
 */
void device_timer_disable (void);

/**
 * @brief Handle of timers handlers
 */

void device_timer_idle(void);

/**
 * @brief GPIO SIGNAL initialization.
 */
void gpio_signal_init (void);

/**
 * @brief Measurement of the battery capacity at a time instance.
 */

void device_battery_measure (void);

void saadc_init(void);

#endif // APPLICATION_H__
