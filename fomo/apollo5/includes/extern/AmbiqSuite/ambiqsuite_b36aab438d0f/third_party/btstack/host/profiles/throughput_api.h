//*****************************************************************************
//
//  throughput_api.h
//! @file
//!
//! @brief Ambiq Micro's demonstration of throughput service.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef THROUGHPUT_API_H
#define THROUGHPUT_API_H

#include "rpc_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * DEFINES
 ****************************************************************************************
 */
#define UPLINK_DATA_CNT      (512)
#define UPLINK_DATA          (0xAA)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */



/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */
//



/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/*
 ****************************************************************************************
 * @brief throughput send notify data to AMBT53 for test
 *
 * @param send_data data to send
 * @param send_data_len lenght of data to send
 * @return >=0 on SUCCESS(length of sent out packet)
 *          RPMSG_ERR_INIT if endpoint is not registered
 *          RPMSG_ERR_NO_MEM if no memory to store the packet.
 ****************************************************************************************
 */
int app_throughput_send_data(uint8_t* send_data, uint8_t send_data_len);


#ifdef __cplusplus
}
#endif

#endif // THROUGHPUT_API_H
