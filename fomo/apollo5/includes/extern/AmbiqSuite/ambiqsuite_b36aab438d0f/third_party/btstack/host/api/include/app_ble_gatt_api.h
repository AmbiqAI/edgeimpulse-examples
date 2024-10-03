//*****************************************************************************
//
//! @file app_ble_gatt_api.h
//!
//! @brief Application GATT API to for BLE GATT activities.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_BLE_GATT_API_H
#define AM_BLE_GATT_API_H

#include "am_types.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
 * DEFINES
 ****************************************************************************************
 */
/// Maximum battery value
#define BAS_MAX_BATTERY_VALUE (100)




/*
 * ENUMERATIONS
 ****************************************************************************************
 */




/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */





/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/*
 ****************************************************************************************
 * @brief Update HFP HF battery level
 *
 * @param bat_lvl battery level range from 0~100
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_bass_chg_bat_lvl(uint8_t bat_lvl);



#ifdef __cplusplus
};
#endif

#endif /* AM_BLE_GATT_API_H */
