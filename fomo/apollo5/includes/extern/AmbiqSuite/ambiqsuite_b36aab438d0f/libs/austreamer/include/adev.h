/**
 ****************************************************************************************
 *
 * @file adev.h
 *
 * @brief Declaration of the Audio Device API.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_H_
#define _ADEV_H_

#include "adev_comm.h"
#include "adev_setting.h"

#if defined __cplusplus
extern "C"
{
#endif

    /**
     ****************************************************************************************
     * @brief Audio Device Init
     ****************************************************************************************
     */
    void adev_init(void);

    /**
     ****************************************************************************************
     * @brief Enable an endpoint
     * @param epid Endpoint ID
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_ep_enable(adev_ep_id_t epid);

    /**
     ****************************************************************************************
     * @brief Active Audio Device
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_active(void);

    /**
     ****************************************************************************************
     * @brief Read data from ADEV
     * @param epid Endpoint ID
     * @param buf Pointer of buffer to store data
     * @param size Size of the buffer, unit is Byte
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_read_data(adev_ep_id_t epid, void *buf, uint32_t size);

    /**
     ****************************************************************************************
     * @brief Write data to ADEV
     * @param epid Endpoint ID
     * @param buf Pointer of buffer storing the data
     * @param size Size of the buffer, unit is Byte
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_write_data(adev_ep_id_t epid, void *buf, uint32_t size);

    /**
     ****************************************************************************************
     * @brief De-active Audio Device
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_deactive(void);

    /**
    ****************************************************************************************
    * @brief Disable an endpoint
    * @param epid Endpoint ID
    * @return  ADEV error code
    ****************************************************************************************
    */
    uint32_t adev_ep_disable(adev_ep_id_t epid);

    /**
     ****************************************************************************************
     * @brief Reset ADEV configuration
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_ep_reset(adev_ep_id_t epid);

    /**
     ****************************************************************************************
     * @brief Audio Device Deinit
     ****************************************************************************************
     */
    void adev_deinit(void);

#if defined __cplusplus
}
#endif
#endif /* _ADEV_H_ */
