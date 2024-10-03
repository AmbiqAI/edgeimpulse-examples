/**
 ****************************************************************************************
 *
 * @file adev_dev.h
 *
 * @brief Declaration of the ADEV Device API.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_DEV_H_
#define _ADEV_DEV_H_

#include "adev_comm.h"

#if defined __cplusplus
extern "C"
{
#endif

    /**
     ****************************************************************************************
     * @brief ADEV device module initial
     * @param ep_confs List of endpoint configuration pointer
     * @param ep_num   Endpoint number in the list ep_confs
     * @param clk_conf Pointer of clock configuration
     * @return ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_dev_init(adev_conf_t **ep_confs, uint16_t ep_num,
                           const adev_conf_t *clk_conf);

    /**
     ****************************************************************************************
     * @brief Set endpoint notify callback
     * @param ep Endpoint ID
     * @param notify Notify callback
     ****************************************************************************************
     */
    void adev_dev_set_notify(uint32_t ep, adev_notify_f notify);

    /**
     ****************************************************************************************
     * @brief ADEV device module deinit
     ****************************************************************************************
     */
    void adev_dev_deinit(void);

    /**
     ****************************************************************************************
     * @brief ADEV device module enable
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_dev_enable(void);

    /**
     ****************************************************************************************
     * @brief ADEV devices disable
     ****************************************************************************************
     */
    void adev_dev_disable(void);

    /**
     ****************************************************************************************
     * @brief Read data from a device
     * @param ep Endpoint ID
     * @param buf Pointer of buffer to store data
     * @param size Size of the buffer, unit is Byte. Size expected to read
     * @return Data size read actually
     ****************************************************************************************
     */
    uint32_t adev_dev_read(uint32_t ep, char *buf, uint32_t size);

    /**
     ****************************************************************************************
     * @brief Write data to a device
     * @param ep Endpoint ID
     * @param buf Pointer of buffer storing the data
     * @param size Size of the buffer, unit is Byte. Size expected to write
     * @return  Data size has been written actually
     ****************************************************************************************
     */
    uint32_t adev_dev_write(uint32_t ep, char *buf, uint32_t size);

#if defined __cplusplus
}
#endif
#endif /* _ADEV_DEV_H_ */
