//*****************************************************************************
//
//! @file app_bt_device.h
//!
//! @brief API to get/set device parameters.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


#ifndef AM_BT_DEVICE_H
#define AM_BT_DEVICE_H

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
/// Define device rquest function parameter length

#define DEV_CONFIG_REQ_LEN                          (0)

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/**
 * app_bt_get_ltv_value
 *
 * Get the specified data type value from the given data
 *
 * @param - type, the expected data type, see #enum gap_ad_type
 * @param - data_size, the total size of the input p_data
 * @param - p_data, pointer to the source data
 * @param - p_length, indicate the expected data length if found
 *
 * return - value of expected data type in p_data or NULL if not found
 */
const uint8_t *app_bt_get_ltv_value(uint8_t type, uint16_t data_size,
                                  const uint8_t *p_data, uint8_t *p_length);

#ifdef __cplusplus
};
#endif

#endif /* AM_BT_DEVICE_H */
