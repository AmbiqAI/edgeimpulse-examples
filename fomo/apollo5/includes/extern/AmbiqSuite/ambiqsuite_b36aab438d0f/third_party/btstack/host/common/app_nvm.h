//*****************************************************************************
//
//  app_nvm.h
//! @file
//!
//! @brief Application non volatile data management.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef APP_NVM_H
#define APP_NVM_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif



/**************************************************************************************************
  Macros
**************************************************************************************************/
#define  BT_CONN_NUM_MAX             (5)
#define  BT_BOND_KEY_INFO_LEN        (91)
#define  NV_MEMORY_TOTAL_SIZE        (1024)
#define  NV_MEMORY_SYS_SETTING_SIZE  (512)
#define  NV_MEMORY_BT_BOND_KEY_SIZE  (512)
#define  NV_MEMORY_HFP_GAIN_DEFAULT   (9)

#define NV_MEMORY_ADDESS_START       (0x000A0000)
#define NV_MEMORY_ADDRESS_BOND_KEY   (NV_MEMORY_ADDESS_START+NV_MEMORY_SYS_SETTING_SIZE)

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/*! Database record */
typedef struct
{
    // system parameters
    uint8_t hfp_volume;
    uint8_t avrcp_volume;

    // bonded key parameters
    uint8_t entry_num;
    uint16_t total_size;
    // bonded key record
    uint8_t rec[BT_CONN_NUM_MAX*BT_BOND_KEY_INFO_LEN];
} app_nvm_t;



/**************************************************************************************************
  Function Declarations
**************************************************************************************************/
/*************************************************************************************************/
/*!
 *  \brief  Initialize the non volatile device database.
 *
 *  \return None.
 */
/*************************************************************************************************/
void app_nvm_init(void);

/*************************************************************************************************/
/*!
 *  \brief  Update the received non volatile content to flash space
 *
 *  \param  msg      content to update to flash
 *  \param  msg_len  content length.
 *
 */
/*************************************************************************************************/
void app_nvm_update_db(uint8_t *msg, uint16_t msg_len);

int app_nvm_load(uint8_t item_type);

/*************************************************************************************************/
/*!
 *  \brief  Update the HFP volume gain to the specified mram space
 *
 *  \param  gain     updated HFP volume gain, range 0 ~ 15
 *
 */
/*************************************************************************************************/
void app_nvm_update_hfp_vol(uint8_t gain);

/*************************************************************************************************/
/*!
 *  \brief  Get the HFP volume gain stored in mram
 *
 *  \retrun  stored HFP volume gain
 *
 */
/*************************************************************************************************/
uint8_t app_nvm_get_hfp_vol(void);


#ifdef __cplusplus
};
#endif

#endif /* APP_NVM_H */
