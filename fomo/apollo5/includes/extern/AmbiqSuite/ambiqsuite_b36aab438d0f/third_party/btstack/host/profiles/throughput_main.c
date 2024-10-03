//*****************************************************************************
//
//! throughput_main.c
//! @file
//!
//! @brief This file provides the main application for the throughput profile.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "am_logger.h"
#include "rpc_common.h"


/*
 * GLOBAL VARIABLE
 ****************************************************************************************
 */


/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */



/*
 * EXPORTED FUNCTION DEFINITIONS
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
int app_throughput_send_data(uint8_t* send_data, uint8_t send_data_len)
{
    if((send_data == NULL) || (send_data_len==0))
    {
        am_error_printf(true, "inalid parameter \r\n");
        return RPMSG_ERR_PARAM;
    }
    return ipc_btdm_data_send(RPMSG_BTDM_THRPUT_DATA_SEND, send_data, send_data_len);
}
