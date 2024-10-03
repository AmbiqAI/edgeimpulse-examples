//*****************************************************************************
//
//! @file app_bt_avrcp_api.h
//!
//! @brief Application API for BT AVRCP activity.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


#ifndef APP_BT_AVRCP_API_H
#define APP_BT_AVRCP_API_H

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "am_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DEFINES
 ****************************************************************************************
 */

/// Define function parameter length
#define BTDM_APP_AVRCP_PLAY_REQ_LEN        (0)
#define BTDM_APP_AVRCP_STOP_REQ_LEN        (0)
#define BTDM_APP_AVRCP_PLAY_BACK_REQ_LEN   (0)
#define BTDM_APP_AVRCP_PLAY_FORW_REQ_LEN   (0)


#define AVRCP_VOL_MIN                      (0)
#define AVRCP_VOL_MAX                      (127)
#define AVRCP_VOL_DEF                      (10)
#define AVRCP_VOL_STEP                     (6)


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
 * @brief AVRCP control music play
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_avrcp_play(void);

/*
 ****************************************************************************************
 * @brief AVRCP control music stop
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_avrcp_stop(void);

/*
 ****************************************************************************************
 * @brief AVRCP control to play backward music
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_avrcp_play_backward(void);

/*
 ****************************************************************************************
 * @brief AVRCP control to play forward music
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_avrcp_play_forward(void);

/*
 ****************************************************************************************
 * @brief AVRCP control music stop
 *
 * @param vol the absolute volume to set, range 0~127
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_avrcp_set_volume(uint8_t vol);



#ifdef __cplusplus
};
#endif

#endif /* AM_BT_GAP_API_H */
