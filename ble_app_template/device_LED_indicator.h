#ifndef DEVICE_LED_INDICATOR_H__
#define DEVICE_LED_INDICATOR_H__

//TODO: Feel free to add more states in the enum to indicate more cases if you need to
typedef enum {
	DEVICE_INDICATE_ADVERTISING,
	DEVICE_INDICATE_CONNECTED,
	DEVICE_INDICATE_TREATMENT_ON,
	DEVICE_INDICATE_DISCONNECTED_TREATMENT_ON,
	DEVICE_INDICATE_IDLE
} device_led_state_t;

extern device_led_state_t m_device_led;

void device_indication_set (device_led_state_t z_evt);

void device_indication_init(void);

#endif
