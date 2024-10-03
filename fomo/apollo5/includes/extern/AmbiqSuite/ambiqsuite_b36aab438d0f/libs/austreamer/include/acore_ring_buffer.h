/****************************************************************************************
 *
 * @file acore_ring_buffer.h
 *
 * @brief Audio core DMA ring buffer is design for audio DMA to detect xrun
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ACORE_RING_BUF_H_
#define _ACORE_RING_BUF_H_
#if defined __cplusplus
extern "C"
{
#endif

#include <stdint.h>
    typedef struct
    {
        char    *buf;      // Buffer address
        uint32_t buf_size; // Buffer size
        uint32_t cl_size;  // Cluster size
        uint32_t rw_pos;   // dring read/write position
        uint64_t cl_cnt;   // Cluster counter
        uint64_t rw_cnt;   // read/write counter
        int64_t  xrun_cnt; // underrun counter
    } dring_t;

    /**
     ****************************************************************************************
     * @brief Init a DMA ring buffer
     *
     * @param  dr        Provide a dring_t struct to be initialized
     * @param  buf       DMA buffer address
     * @param  cl_size   DMA cluster size
     * @param  cl_num    Cluster number of the buffer
     *
     ****************************************************************************************
     */
    void dring_init(dring_t *dr, char *buf, uint32_t cl_size, uint32_t cl_num);

    /**
     ****************************************************************************************
     * @brief Reset dring
     *
     * @param  dr        Dring instance
     *
     ****************************************************************************************
     */
    void dring_reset(dring_t *dr);

    /**
     ****************************************************************************************
     * @brief Read data from DMA ring buffer. This API is used in audio record.
     * Hardware cycle DMA send data into buffer, software application call
     *dring_read to receive data.
     *
     * @param  dr        Dring instance
     * @param  buf       Memory address to store output data
     * @param  size      Data size to read
     *
     * @return   0: Normal; DRING_OVER_RUN: Read too fast; DRING_UNDER_RUN: Read
     *to slow
     *
     ****************************************************************************************
     */
    uint32_t dring_read(dring_t *dr, char *buf, uint32_t size);

    /**
     ****************************************************************************************
     * @brief Get available data size to read
     *
     * @param  dr        Dring instance
     *
     * @return Available data size. Unit: Byte
     *
     ****************************************************************************************
     */
    uint32_t dring_get_read_size(dring_t *dr);

    /**
     ****************************************************************************************
     * @brief Write data into DMA ring buffer. This API is used for audio
     *playback. Hardware cycle DMA get data from buffer, software application
     *call dring_write to send data into buffer.
     *
     * @param  dr        Dring instance
     * @param  buf       Memory address of input data. NULL means write silent
     *data.
     * @param  size      Input data size
     *
     * @return   0: Normal; DRING_OVER_RUN: Write too fast; DRING_UNDER_RUN:
     *Write to slow
     *
     ****************************************************************************************
     */
    uint32_t dring_write(dring_t *dr, char *buf, uint32_t size);

    /**
     ****************************************************************************************
     * @brief Update cluster counter for XRUN detection. Called by DMA cluster
     *ISR.
     *
     * @param  dr        Dring instance
     *
     ****************************************************************************************
     */
    void dring_cluster_count(dring_t *dr);
#if defined __cplusplus
}
#endif
#endif // _ACORE_RING_BUF_H_
