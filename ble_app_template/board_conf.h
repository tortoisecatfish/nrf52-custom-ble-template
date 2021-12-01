#ifndef BOARD_CONF_H__
#define BOARD_CONF_H__

// include SDK board definition
#include "pca10040.h"

/** HW pin map */

#define SIGNAL_1	    17             /**< Corresponding pin number to signal 1. */
#define SIGNAL_2	    18             /**< Corresponding pin number to signal 2. */
#define SIGNAL_3	    14             /**< Corresponding pin number to signal 3. */
#define SIGNAL_4	    15             /**< Corresponding pin number to signal 4. */
#define BOOST           20             /**< Corresponding pin number to BOOST EN. */
#define SD              23             /**< Corresponding pin number to shutdown pin on the charger. */
#define PIN_IN          13             /**< Corresponding pin number to the button on the board */
#define LED_PIN         19             /**< Corresponding pin number to the LED on the board. */
#define battery_measure 16             /**< Corresponding pin number to the control of battery measure. */

#endif //BOARD_CONF_H__
