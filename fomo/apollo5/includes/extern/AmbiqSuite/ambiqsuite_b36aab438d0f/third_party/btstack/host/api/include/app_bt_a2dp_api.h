//*****************************************************************************
//
//! @file app_bt_a2dp_api.h
//!
//! @brief Application API for bt a2dp sink/source profile activity.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


#ifndef APP_BT_A2DP_API_H
#define APP_BT_A2DP_API_H

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
#define BTDM_APP_A2DP_SRC_START_REQ_LEN          (0)
#define BTDM_APP_A2DP_SRC_SUSPEND_REQ_LEN        (0)
#define BTDM_APP_A2DP_SRC_CLOSE_REQ_LEN          (0)
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


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
 * @brief Start Streaming onto the connection (required to send Audio Data)
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_a2dp_src_stream_start(void);

/*
 ****************************************************************************************
 * @brief Stop / Suspend stream onto the connection
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_a2dp_src_stream_suspend(void);

/*
 ****************************************************************************************
 * @brief  Close the stream on the ACL connection.
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_a2dp_src_stream_close(void);

#ifdef __cplusplus
};
#endif

#endif /* APP_BT_A2DP_API_H */
