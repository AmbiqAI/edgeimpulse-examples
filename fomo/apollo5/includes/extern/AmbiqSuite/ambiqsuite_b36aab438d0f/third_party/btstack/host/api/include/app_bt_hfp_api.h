//*****************************************************************************
//
//! @file app_bt_hfp_api.h
//!
//! @brief Application API for bt HFP profile activity.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


#ifndef APP_BT_HFP_API_H
#define APP_BT_HFP_API_H

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
#define HFP_VOL_GAIN_MIN  (0)
#define HFP_VOL_GAIN_MAX  (15)
#define HFP_VOL_GAIN_DEF  (5)
/// 24 bytes string + '\0' termination
#define HFP_MAX_PHONE_NUMBER_SIZE (25)

/// Maximum battery value
#define HFP_HF_MAX_BATTERY_VALUE (100)

/// Define function parameter length

#define  BTDM_APP_ANSWER_INCOMING_CALL_REQ_LEN             (0)
#define  BTDM_APP_REJECT_INCOMING_CALL_REQ_LEN             (0)
#define  BTDM_APP_HANGUP_CALL_REQ_LEN             (0)

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
 * @brief Answer incoming call
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int hfp_hf_answer_call(void);

/*
 ****************************************************************************************
 * @brief Reject incoming call
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int hfp_hf_reject_call(void);

/*
 ****************************************************************************************
 * @brief Dial outgoing call
 *
 * @param p_number  the phone number to dial
 * @praam num_len   the length of phone number
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int hfp_hf_initiate_outgoing_call(uint8_t *p_number, uint8_t num_len);

/*
 ****************************************************************************************
 * @brief Hangup call
 *
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int hfp_hf_hang_up_call(void);

/*
 ****************************************************************************************
 * @brief Set HFP speaker volume
 *
 * @param gain range from 0~15
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int hfp_hf_set_speaker_volume(uint8_t gain);

/*
 ****************************************************************************************
 * @brief Update HFP HF battery level
 *
 * @param battery level range from 0~100
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_hfp_hf_chg_bat_lvl(uint8_t bat_lvl);


#ifdef __cplusplus
};
#endif

#endif /* APP_BT_HFP_API_H */
