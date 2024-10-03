//*****************************************************************************
//
//! @file app_ble_gap_api.h
//!
//! @brief Application API to start and stop ble activity.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_BLE_GAP_API_H
#define AM_BLE_GAP_API_H

#include "am_types.h"
#include "gap.h"
#include "rpc_client_btdm.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
 * DEFINES
 ****************************************************************************************
 */

#define  ADV_DATA_HEADER_LEN      (2)

#define APP_APPEARANCE_UNKNOWN    (0x00)

/*! advertising data*/
// Ambiq company identifier
#define COMPANY_ID                (0x09AC)
// smartwatch appearance
#define BLE_APPEARANCE            (0x00C2)

/// Define BLE GAP rquest function parameter length
#define  BLE_LEG_ADV_CREATE_REQ_LEN                 (sizeof(gapm_adv_create_param_t)+1)
#define  BLE_EXT_ADV_CREATE_REQ_LEN                 (sizeof(gapm_adv_create_param_t)+sizeof(gapm_adv_second_cfg_t)+1)
#define  BLE_ADV_DATA_SET_REQ_LEN(len)              ((len)+2)
#define  BLE_SCN_RSP_DATA_SET_REQ_LEN(len)          ((len)+2)
#define  BLE_PER_ADV_CREATE_REQ_LEN                 (sizeof(gapm_adv_create_param_t)+sizeof(gapm_adv_second_cfg_t)+sizeof(gapm_adv_period_cfg_t)+1)
#define  BLE_PER_ADV_DATA_SET_REQ_LEN(len)          ((len)+2)
#define  BLE_PER_SYNC_START_REQ_LEN                 (sizeof(gap_bdaddr_t)+4)
#define  BLE_PER_SYNC_CANCEL_REQ_LEN                (0)
#define  BLE_ADV_START_REQ_LEN                      (sizeof(gapm_adv_param_t)+1)
#define  BLE_ADV_STOP_REQ_LEN                       (1)
#define  BLE_SCAN_START_REQ_LEN                     (sizeof(gapm_adv_param_t)+1)
#define  BLE_SCAN_STOP_REQ_LEN                      (0)
#define  BLE_PREF_CONN_PARAM_SET_REQ_LEN            (sizeof(gapm_adv_param_t))
#define  BLE_CONN_CREATE_REQ_LEN                    (sizeof(gap_bdaddr_t)+sizeof(gapm_init_param_t)+1)
#define  BLE_CONN_CANCEL_REQ_LEN                    (1)
#define  BLE_SEC_REQ_SEND_REQ_LEN                   (2)
#define  BLE_PHY_UPDATE_REQ_LEN                     (4)
#define  BLE_PHY_READ_REQ_LEN                       (1)
#define  BLE_PEER_FEA_READ_REQ_LEN                  (1)
#define  BLE_PAIR_START_REQ_LEN                     (1)
#define  BLE_UPDATE_PARAM_REQ_LEN                   (sizeof(struct gapc_conn_param)+1)

#define  BTDM_APP_BLE_ADV_START_REQ_LEN             (0)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

enum le_role {
    /** Only Peripheral role supported. */
    LE_ROLE_PERIPH_ONLY,
    /** Only Central role supported. */
    LE_ROLE_CENTRAL_ONLY,
    /**
    * Peripheral and Central roles supported. Peripheral role preferred for
    * connection establishment.
    */
    LE_ROLE_PERIPH_PREFFERED,
    /**
    * Peripheral and Central roles supported. Central role preferred for
    * connection establishment.
    */
    LE_ROLE_CENTRAL_PREFFERED,
    /** Total number of options. */
    LE_ROLE_OPTIONS_NUM,
};

/// Own BD address source of the device
enum gapm_own_addr
{
    /// Public or Private Static Address according to device address configuration
    GAPM_STATIC_ADDR,
    /// Generated resolvable private random address
    GAPM_GEN_RSLV_ADDR,
    /// Generated non-resolvable private random address
    GAPM_GEN_NON_RSLV_ADDR,
};

/// Option for PHY configuration
enum gapc_phy_option
{
    /// No preference for rate used when transmitting on the LE Coded PHY
    GAPC_PHY_OPT_LE_CODED_ALL_RATES = 0,
    /// 500kbps rate preferred when transmitting on the LE Coded PHY
    GAPC_PHY_OPT_LE_CODED_500K_RATE = 1,
    /// 125kbps  when transmitting on the LE Coded PHY
    GAPC_PHY_OPT_LE_CODED_125K_RATE = 2,
};


/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */


 /// Information about received ADV report
typedef struct gapm_adv_report_info
{
    /// Bit field providing information about the received report (see enum #gapm_adv_report_info_bf)
    uint8_t info;
    /// Transmitter device address
    gap_bdaddr_t trans_addr;
    /// Target address (in case of a directed advertising report)
    gap_bdaddr_t target_addr;
    /// TX power (in dBm)
    int8_t tx_pwr;
    /// RSSI (between -127 and +20 dBm)
    int8_t rssi;
    /// Primary PHY on which advertising report has been received
    uint8_t phy_prim;
    /// Secondary PHY on which advertising report has been received
    uint8_t phy_second;
    /// Advertising SID
    /// Valid only for periodic advertising report
    uint8_t adv_sid;
    /// Periodic advertising interval (in unit of 1.25ms, min is 7.5ms)
    /// Valid only for periodic advertising report
    uint16_t period_adv_intv;
} gapm_adv_report_info_t;

/// Additional advertising parameters
typedef struct gapm_adv_param
{
    /// Advertising duration (in unit of 10ms). 0 means that advertising continues
    /// until the host disable it
    uint16_t duration;
    /// Maximum number of extended advertising events the controller shall attempt to send prior to
    /// terminating the extending advertising
    /// Valid only if extended advertising
    uint8_t max_adv_evt;
} gapm_adv_param_t;

/// Connection Parameter used to update connection parameters
struct gapc_conn_param
{
    /// Connection interval minimum
    uint16_t intv_min;
    /// Connection interval maximum
    uint16_t intv_max;
    /// Latency
    uint16_t latency;
    /// Supervision timeout
    uint16_t time_out;
};

/// Long Term Key information
typedef struct gapc_ltk
{
    /// Long Term Key
    gap_sec_key_t key;
    /// Encryption Diversifier
    uint16_t ediv;
    /// Random Number
    rand_nb_t randnb;
    /// Encryption key size (7 to 16)
    uint8_t key_size;
} gapc_ltk_t;

/// Scan Window operation parameters
typedef struct gapm_scan_wd_op_param
{
    /// Scan interval (N * 0.625 ms)
    uint16_t scan_intv;
    /// Scan window (N * 0.625 ms)
    uint16_t scan_wd;
} gapm_scan_wd_op_param_t;


/// Scanning parameters
typedef struct gapm_scan_param
{
    /// Type of scanning to be started (see enum #gapm_scan_type)
    uint8_t type;
    /// Properties for the scan procedure (see enum #gapm_scan_prop for bit signification)
    uint8_t prop;
    /// Duplicate packet filtering policy
    uint8_t dup_filt_pol;
    /// Reserved for future use
    uint8_t rsvd;
    /// Scan window opening parameters for LE 1M PHY
    gapm_scan_wd_op_param_t scan_param_1m;
    /// Scan window opening parameters for LE Coded PHY
    gapm_scan_wd_op_param_t scan_param_coded;
    /// Scan duration (in unit of 10ms). 0 means that the controller will scan continuously until
    /// reception of a stop command from the application
    uint16_t duration;
    /// Scan period (in unit of 1.28s). Time interval betweem two consequent starts of a scan duration
    /// by the controller. 0 means that the scan procedure is not periodic
    uint16_t period;
} gapm_scan_param_t;

typedef struct gapm_conn_param
{
    /// Minimum value for the connection interval (in unit of 1.25ms). Shall be less than or equal to
    /// conn_intv_max value. Allowed range is 7.5ms to 4s.
    uint16_t conn_intv_min;
    /// Maximum value for the connection interval (in unit of 1.25ms). Shall be greater than or equal to
    /// conn_intv_min value. Allowed range is 7.5ms to 4s.
    uint16_t conn_intv_max;
    /// Slave latency. Number of events that can be missed by a connected slave device
    uint16_t conn_latency;
    /// Link supervision timeout (in unit of 10ms). Allowed range is 100ms to 32s
    uint16_t supervision_to;
    /// Recommended minimum duration of connection events (in unit of 625us)
    uint16_t ce_len_min;
    /// Recommended maximum duration of connection events (in unit of 625us)
    uint16_t ce_len_max;
} gapm_conn_param_t;

typedef struct gapm_init_param
{
    /// Initiating type (see enum #gapm_init_type)
    uint8_t type;
    /// Properties for the initiating procedure (see enum #gapm_init_prop for bit signification)
    uint8_t prop;
    /// Timeout for automatic connection establishment (in unit of 10ms). Cancel the procedure if not all
    /// indicated devices have been connected when the timeout occurs. 0 means there is no timeout
    uint16_t conn_to;
    /// Scan window opening parameters for LE 1M PHY
    gapm_scan_wd_op_param_t scan_param_1m;
    /// Scan window opening parameters for LE Coded PHY
    gapm_scan_wd_op_param_t scan_param_coded;
    /// Connection parameters for LE 1M PHY
    gapm_conn_param_t conn_param_1m;
    /// Connection parameters for LE 2M PHY
    gapm_conn_param_t conn_param_2m;
    /// Connection parameters for LE Coded PHY
    gapm_conn_param_t conn_param_coded;
    /// Address of peer device in case white list is not used for connection
    gap_bdaddr_t peer_addr;
} gapm_init_param_t;

typedef struct gapm_adv_prim_cfg
{
    /// Minimum advertising interval (in unit of 625us). Must be greater than 20ms
    uint32_t adv_intv_min;
    /// Maximum advertising interval (in unit of 625us). Must be greater than 20ms
    uint32_t adv_intv_max;
    /// Bit field indicating the channel mapping
    uint8_t chnl_map;
    /// Indicate on which PHY primary advertising has to be performed (see enum #gapm_phy_type)
    /// Note that LE 2M PHY is not allowed and that legacy advertising only support LE 1M PHY
    uint8_t phy;
} gapm_adv_prim_cfg_t;

typedef struct gapm_adv_create_param
{
    /// Bit field value provided advertising properties
    /// (see enum #gapm_leg_adv_prop, see enum #gapm_ext_adv_prop and see enum #gapm_per_adv_prop for bit signification)
    uint16_t prop;
    /// Discovery mode (see enum #gapm_adv_disc_mode)
    uint8_t disc_mode;
    /// Maximum power level at which the advertising packets have to be transmitted
    /// (between -127 and 126 dBm)
    int8_t max_tx_pwr;
    /// Advertising filtering policy (see enum #gap_adv_filter_policy)
    uint8_t filter_pol;
    /// Peer address configuration (only used in case of directed advertising)
    gap_bdaddr_t peer_addr;
    /// Configuration for primary advertising
    gapm_adv_prim_cfg_t prim_cfg;
} gapm_adv_create_param_t;

typedef struct gapm_adv_second_cfg
{
    /// Maximum number of advertising events the controller can skip before sending the
    /// AUX_ADV_IND packets. 0 means that AUX_ADV_IND PDUs shall be sent prior each
    /// advertising events
    uint8_t max_skip;
    /// Indicate on which PHY secondary advertising has to be performed (see enum #gapm_phy_type)
    uint8_t phy;
    /// Advertising SID
    uint8_t adv_sid;
} gapm_adv_second_cfg_t;

/// Configuration for periodic advertising

typedef struct gapm_adv_period_cfg
{
    // Minimum periodic advertising interval (in unit of 1.25ms). Must be greater than 20ms
    uint16_t interval_min;
    /// Maximum periodic advertising interval (in unit of 1.25ms). Must be greater than 20ms
    uint16_t interval_max;
} gapm_adv_period_cfg_t;


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * Callback executed when get LE connection used PHY value procedure is completed.
 *
 * @param[in] conidx    Connection index
 * @param[in] dummy     Dummy parameter provided by upper layer application
 * @param[in] status    Status of procedure execution (see enum #am_error)
 * @param[in] tx_phy    LE PHY for data transmission (see enum #gap_phy)
 * @param[in] rx_phy    LE PHY for data reception (see enum #gap_phy)
 *
 ****************************************************************************************
 */
typedef void (*gapc_get_le_phy_cmp_cb)(uint8_t conidx, uint32_t dummy, uint16_t status,
                                       uint8_t tx_phy, uint8_t rx_phy);



/*
 ****************************************************************************************
 * Callback executed when get LE peer supported features value procedure is completed.
 *
 * @param[in] conidx      Connection index
 * @param[in] dummy       Dummy parameter provided by upper layer application
 * @param[in] status      Status of procedure execution (see enum #am_error)
 * @param[in] features  Pointer to peer supported feature array
 *
 ****************************************************************************************
 */
typedef void (*gapc_get_le_peer_features_cmp_cb)(uint8_t conidx, uint32_t dummy, uint16_t status,
                                                        const uint8_t *features);


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/*
 ****************************************************************************************
 * @brief Start BLE advertising
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_adv_start(void);

#ifdef __cplusplus
};
#endif

#endif /* AM_BLE_GAP_API_H */
