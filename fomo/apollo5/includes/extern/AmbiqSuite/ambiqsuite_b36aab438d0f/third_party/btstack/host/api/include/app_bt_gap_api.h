//*****************************************************************************
//
//! @file app_bt_gap_api.h
//!
//! @brief Application API to start and stop bt activity.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


#ifndef AM_BT_GAP_API_H
#define AM_BT_GAP_API_H

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "am_types.h"
#include "gap.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DEFINES
 ****************************************************************************************
 */

/// Define function parameter length
#define  BTDM_APP_START_REQ_LEN                      (0)
#define  BTDM_APP_INQ_START_REQ_LEN                  (0)
#define  BTDM_APP_CONN_CREATE_REQ_LEN                (0)
#define  BTDM_APP_INQ_STOP_REQ_LEN                   (0)
#define  BTDM_APP_CONN_CANCEL_REQ_LEN                (0)
#define  BTDM_APP_INQ_SCAN_START_REQ_LEN             (0)
#define  BTDM_APP_INQ_SCAN_STOP_REQ_LEN              (0)
#define  BTDM_APP_PAGE_SCAN_START_REQ_LEN            (0)
#define  BTDM_APP_PAGE_SCAN_STOP_REQ_LEN             (0)


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef struct gapm_inquiry_report
{
    /// Class of device
    uint32_t class_of_device;
    /// Peer address
    gap_addr_t addr;
    /// RSSI  - 0x80 if invalid
    int8_t rssi;
    /// Page Scan Repetition Mode
    uint8_t page_scan_rep_mode;
    /// Clock Offset
    uint16_t clk_off;
} gapm_inquiry_report_t;


typedef void (*app_inquiry_report_received_cb)(uint8_t *inq_report);

// Inquiry application environment  structure
typedef struct app_inquiry_env_
{
    // Report received callback
    app_inquiry_report_received_cb cb_report_received;
} app_inquiry_env_t;

extern app_inquiry_env_t app_inquiry_env;

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
 * @brief Start inquiry
 *
 * @param[in] cb_report_received        callback function
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_inquiry_start(app_inquiry_report_received_cb cb_report_received);

/*
 ****************************************************************************************
 * @brief Stop inquiry
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_inquiry_stop(void);



/*
 ****************************************************************************************
 * @brief Connection establishment with peer device
 *
 * @param[in] p_peer_addr  device address to connect
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_bt_con_estab(gap_addr_t *p_peer_addr);

/*
 ****************************************************************************************
 * @brief Cancel bt connection establishment
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_bt_con_cancel(void);

/*
 ****************************************************************************************
 * @brief Start inquiry scan.
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_inquiry_scan_start(void);



/*
 ****************************************************************************************
 * @brief Stop inquiry scan.
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_inquiry_scan_stop(void);


/*
 ****************************************************************************************
 * @brief Start page scan.
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_page_scan_start(void);

/*
 ****************************************************************************************
 * @brief Stop page scan .
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_page_scan_stop(void);

#ifdef __cplusplus
};
#endif

#endif /* AM_BT_GAP_API_H */
