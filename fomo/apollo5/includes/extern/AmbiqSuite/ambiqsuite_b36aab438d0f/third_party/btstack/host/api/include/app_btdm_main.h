//*****************************************************************************
//
//! @file app_btdm_main.h
//!
//! @brief Function declaration for BT dual mode main state transition.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


#ifndef AM_BTDM_MAIN_H
#define AM_BTDM_MAIN_H


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "gap.h"
#include "am_types.h"
#include "am_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DEFINES
 ****************************************************************************************
 */
#define APP_PROFILE_STATE_SPACE (20u)

// Application transition
enum app_transition
{
    // Application started  - idx meaningless
    APP_STARTED       = 0,
    // Request to start inquiry
    APP_DEVICE_INQUIRY,
    // Connection is established
    APP_BT_CONNECTION_ESTABLISHED,
    // LE Connection is established
    APP_LE_CONNECTION_ESTABLISHED,
    // Pairing is successful
    APP_PAIRING_SUCCEED,
    // Link disconnected
    APP_DISCONNECTED,

    // Inform that inquiry activity is stopped
    APP_INQUIRY_STOPPED,
    // Inform that inquiry scan activity is stopped
    APP_INQUIRY_SCAN_STOPPED,
    // Inform that page activity is stopped
    APP_PAGE_STOPPED,
    // Inform that page scan activity is stopped
    APP_PAGE_SCAN_STOPPED,
    // Advertising Activity created
    APP_ADV_ACTV_CREATED,
    // Set advertising data
    APP_SET_ADV_DATA,
    // Set advertising scan response data
    APP_SET_ADV_SCAN_RSP_DATA,
    // Set periodic advertising data
    APP_SET_PERIOD_ADV_DATA,
    // Advertising Activity stopped
    APP_ADV_ACTV_STOPPED,
    // Scanning Activity stopped
    APP_SCAN_ACTV_STOPPED,
    // initing Activity stopped
    APP_INIT_ACTV_STOPPED,
    // BLE periodic sync established
    APP_BLE_SYNC_ESTAB,
    // BLE pairing succeed
    APP_BLE_PAIRED,

    APP_BLE_ENCRYPTED,
    // First transition value that can be used by a scenario
    APP_FIRST_TRANSITION,
    // First transition value that can be used by A2DP sink profile
    APP_FIRST_A2DP_SINK_TRANSITION =
        APP_FIRST_TRANSITION + APP_PROFILE_STATE_SPACE,
    // First transition value that can be used by A2DP source profile
    APP_FIRST_A2DP_SOURCE_TRANSITION =
        APP_FIRST_A2DP_SINK_TRANSITION + APP_PROFILE_STATE_SPACE,
    // First transition value that can be used by AVRCP profile
    APP_FIRST_AVRCP_TRANSITION =
        APP_FIRST_A2DP_SOURCE_TRANSITION + APP_PROFILE_STATE_SPACE,
    // First transition value that can be used by HFP_UNIT profile
    APP_FIRST_HFP_HF_TRANSITION =
        APP_FIRST_AVRCP_TRANSITION + APP_PROFILE_STATE_SPACE,
    // First transition value that can be used by HFP_GATEWAY profile
    APP_FIRST_HFP_GATEWAY_TRANSITION =
        APP_FIRST_HFP_HF_TRANSITION + APP_PROFILE_STATE_SPACE,
    // First transition value that can be used by AMOTA profile
    APP_FIRST_AMOTA_TRANSITION =
        APP_FIRST_HFP_GATEWAY_TRANSITION + APP_PROFILE_STATE_SPACE,
    // First transition value that can be used by throughput profile
    APP_FIRST_THROUGHPUT_TRANSITION =
        APP_FIRST_AMOTA_TRANSITION + APP_PROFILE_STATE_SPACE,
    // First transition value that can be used by ANCS profile
    APP_FIRST_ANCS_TRANSITION =
         APP_FIRST_THROUGHPUT_TRANSITION + APP_PROFILE_STATE_SPACE,

    APP_TRANSITION_INVALID = 0xFF,
};



/// Pairing mode authorized on the device
/// @verbatim
///    7    6    5    4    3    2    1    0
/// +----+----+----+----+----+----+----+----+
/// |               RFU           | SCP| LP |
/// +----+----+----+----+----+----+----+----+
/// @endverbatim
enum gapm_pairing_mode
{
    /// No pairing authorized
    GAPM_PAIRING_DISABLE = 0,
    /// Legacy pairing Authorized
    GAPM_PAIRING_LEGACY = (1 << 0),
    /// Secure Connection pairing Authorized
    GAPM_PAIRING_SEC_CON = (1 << 1),
};

/// Link Policy Bit Field
enum gapm_link_policy_bf
{
    /// Role Switch enabled
    GAPM_ROLE_SWITCH_ENABLE_BIT = (1 << 0),
    /// Hold Mode enabled
    GAPM_HOLD_MODE_ENABLE_BIT = (1 << 1),
    /// Sniff mode enabled
    GAPM_SNIFF_MODE_ENABLE_BIT = (1 << 2),
};


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * Callback definition used by an application scenario to handle bluetooth event transition
 *
 *
 * @param[in] event     Event transition (see #app_transition)
 * @param[in] idx       Index (connection index or activity index)
 * @param[in] status    Status of event transition
 */
typedef void (*app_state_transition_cb)(uint8_t event, uint16_t status, const void *p_param);

/// Application environment structure
typedef struct app_cfg_env_
{
    /// callback that handle application transition
    app_state_transition_cb cb_transition;
} app_cfg_env_t;




/*
 * MACROS
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */







/*
 ****************************************************************************************
 * @brief Terminate the connection
 *
 * @param[in] conidx       connection index
 *
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_disconnect(uint8_t conidx);


/*
 ****************************************************************************************
 * @brief application initialization for global variant and callback.
 *
 ****************************************************************************************
 */
void app_btdm_init(void);

int app_btdm_start(void);

/*
 ****************************************************************************************
 * @brief Function used to callback transition event to application
 *
 * @param[in] event     application state event to indicate the next transition
 * @param[in] idx       index (connection index or activity index)
 * @param[in] status    Status of event transition
 *
 ****************************************************************************************
 */
void app_state_transition(uint8_t event, uint16_t status, const void *p_param);


/**
 ****************************************************************************************
 * @brief Function used to get application state transition callback.
 *
 ****************************************************************************************
 */
app_state_transition_cb bt_app_get_transition_cb(void);


#ifdef __cplusplus
};
#endif

#endif /* AM_BTDM_MAIN_H */

