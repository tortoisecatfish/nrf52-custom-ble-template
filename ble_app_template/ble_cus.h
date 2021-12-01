#ifndef BLE_CUS_H__
#define BLE_CUS_H__

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

/**@brief   Macro for defining a ble_hrs instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_CUS_DEF(_name)                                                                          \
static ble_cus_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_cus_on_ble_evt, &_name)



// CUSTOM_SERVICE_UUID_BASE f37aa659-d864-4bed-8173-47d46f0c18e3


#define CUSTOM_SERVICE_UUID_BASE         {0xE3, 0x18, 0x0C, 0x6F, 0xD4, 0x47, 0x73, 0x81,           \
                                          0xED, 0x4B, 0x64, 0xD8, 0x59, 0xA6, 0x7A, 0xF3}

#define CUSTOM_SERVICE_UUID               0x1400
#define INTENSITY_VALUE_CHAR_UUID         0x1401
#define HILF_PW_VALUE_CHAR_UUID           0x1402
#define HILF_PERIOD_VALUE_CHAR_UUID       0x1403
#define LIHF_PERIOD_VALUE_CHAR_UUID       0x1404

extern volatile uint8_t intensity_data_received;
extern volatile uint8_t HILF_pulse_width_data_received;
extern volatile uint8_t HILF_period_data_received;
extern volatile uint8_t LIHF_period_data_received;
																					
/**@brief Custom Service event type. */
typedef enum
{
    BLE_CUS_EVT_NOTIFICATION_ENABLED,                              /**< Custom value notification enabled event. */
    BLE_CUS_EVT_NOTIFICATION_DISABLED,                             /**< Custom value notification disabled event. */
    BLE_CUS_EVT_DISCONNECTED,
    BLE_CUS_EVT_CONNECTED,
    BLE_CUS_EVT_GOTO_IDLE,                                         /**< Goto idle state.   */
    BLE_CUS_EVT_GOTO_ACTIVE,                                       /**< Goto active state. */
    BLE_CUS_EVT_TIMER_UPDATE,                                      /**< Update Signal.     */
    BLE_CUS_EVT_GOTO_SLEEP                                         /**< Goto sleep state.  */
} ble_cus_evt_type_t;

/**@brief Custom Service event. */
typedef struct
{
    ble_cus_evt_type_t evt_type;                                  /**< Type of event. */
} ble_cus_evt_t;

// Forward declaration of the ble_cus_t type.
typedef struct ble_cus_s ble_cus_t;


/**@brief Custom Service event handler type. */
typedef void (*ble_cus_evt_handler_t) (ble_cus_t * p_bas, ble_cus_evt_t * p_evt);

/**@brief Battery Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_cus_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint8_t                       initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_cus_init_t;

/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_cus_s
{
    ble_cus_evt_handler_t         evt_handler;                    /**< Event handler to be called for handling events in the Custom Service. */
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      intensity_value_handles;        /**< Handles related to the Intensity Value characteristic. */
    ble_gatts_char_handles_t      HILF_pw_value_handles;          /**< Handles related to the HILF Pulse Width Value characteristic. */
    ble_gatts_char_handles_t      HILF_period_value_handles;      /**< Handles related to the HILF Period Value characteristic. */
    ble_gatts_char_handles_t      LIHF_period_value_handles;      /**> Handles related to the LIHF Period Value characteristic. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};

/**@brief Function for initializing the Custom Service.
 *
 * @param[out]  p_cus       Custom Service structure. This structure will have to be supplied by
 *                          the application. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_cus_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init);

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note 
 *
 * @param[in]   p_cus      Custom Service structure.
 * @param[in]   p_ble_evt  Event received from the BLE stack.
 */
void ble_cus_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context);

/**@brief Function for updating the custom value.
 *
 * @details The application calls this function when the cutom value should be updated. If
 *          notification has been enabled, the custom value characteristic is sent to the client.
 *
 * @note 
 *       
 * @param[in]   p_bas          Custom Service structure.
 * @param[in]   Custom value 
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */

uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t custom_value);

#endif // BLE_CUS_H__
