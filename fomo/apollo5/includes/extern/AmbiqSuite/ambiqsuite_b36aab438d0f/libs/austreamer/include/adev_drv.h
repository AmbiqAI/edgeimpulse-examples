/**
 ****************************************************************************************
 *
 * @file adev_drv.h
 *
 * @brief Declaration of the Audio Device driver common API.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_DRV_H_
#define _ADEV_DRV_H_

#include "adev_comm.h"

#if defined __cplusplus
extern "C"
{
#endif

// Pingpong buffer
#define PINGPONG 2

// Cache line size. Unit is Byte.
#define CACHE_LINE_SIZE (32)

// Bitmap of cache line size
#define CACHE_LINE_SIZE_BITMAP (CACHE_LINE_SIZE - 1)

    // Return value of buffer state machine
    typedef enum
    {
        ADEV_BUF_OK,
        ADEV_BUF_WAIT_FOR_INTERRUPT,
        ADEV_BUF_WAIT_FOR_ACCESS,
        ADEV_BUF_OVER_RUN,
        ADEV_BUF_UNDER_RUN
    } adev_drv_buf_result_t;

    // Driver buffer state
    typedef enum
    {
        ADEV_DRV_BUF_STATE_READY, // Interrupt arrive. Buffer is ready for data
                                  // access.
        ADEV_DRV_BUF_STATE_COMPLETE, // Data access complete.
        ADEV_DRV_BUF_STATE_WAIT // Wait for buffer ready. Wait for interrupt.
    } adev_drv_buf_state_t;

    // Driver buffer data structure
    typedef struct
    {
        uint32_t size; // Number of Bytes
        uint32_t addr; // Address after alignment
        uint32_t priv; // Pointer of buf to be freed
    } adev_drv_buf_t;

    /**
     ****************************************************************************************
     * @brief Allocate driver buffer according device configuration.
     * @param conf Device configuration
     * @return Pointer of driver buffer
     ****************************************************************************************
     */
    adev_drv_buf_t *adev_drv_buf_alloc(const adev_conf_t *conf);

    /**
     ****************************************************************************************
     * @brief Free driver buffer
     * @param buf  Pointer of driver buffer to be freed
     ****************************************************************************************
     */
    void adev_drv_buf_free(adev_drv_buf_t *buf);

    /**
     ****************************************************************************************
     * @brief Buffer state machine. Call this API after interrupt arrive.
     * @param state Buffer state. This parameter will be modified in this
     *function.
     * @return State machine processing result:
     *  ADEV_BUF_OK: Normal state. Nothing to do.
     *  ADEV_BUF_WAIT_FOR_ACCESS: Notify high-level module to access data.
     *  ADEV_BUF_UNDER_RUN: Under run, data access too slow.
     ****************************************************************************************
     */
    adev_drv_buf_result_t
    adev_drv_buf_state_interrupt_arrive(adev_drv_buf_state_t *state);

    /**
     ****************************************************************************************
     * @brief Buffer state machine. Call this API before data access(Read or
     *Write).
     * @param state Buffer state. This parameter will be modified in this
     *function.
     * @return State machine processing result:
     *  ADEV_BUF_OK: Normal state, continue to access data.
     *  ADEV_BUF_WAIT_FOR_INTERRUPT: Wait for interrupt. Should not to access
     *data. ADEV_BUF_OVER_RUN: Over run, data access too fast. Should not to
     *access data.
     ****************************************************************************************
     */
    adev_drv_buf_result_t
    adev_drv_buf_state_data_access(adev_drv_buf_state_t *state);

#if defined __cplusplus
}
#endif
#endif /* _ADEV_DRV_H_ */
