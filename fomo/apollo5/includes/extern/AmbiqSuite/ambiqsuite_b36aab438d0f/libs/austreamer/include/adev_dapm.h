/**
 ****************************************************************************************
 *
 * @file adev_dapm.h
 *
 * @brief Declaration of the Audio DAPM(Dynamic Audio Power Management) API.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_DAPM_H_
#define _ADEV_DAPM_H_

#include "adev_comm.h"

#if defined __cplusplus
extern "C"
{
#endif
    // DAPM state machine
    // # is current state; -> or <- is new state
    // OP     |   UN-INIT  |   INIT    |   DOWN    |    UP   |
    // init   |      #     |     ->    |           |         |
    // probe  |            |     #     |     ->    |         |
    // up     |            |           |     #     |    ->   |
    // down   |            |           |     <-    |    #    |
    // deinit |      <-    |     #     |     #     |         |  INIT or DOWN
    typedef enum
    {
        ADEV_DAPM_STATE_UNINIT = 0,
        ADEV_DAPM_STATE_INIT,
        ADEV_DAPM_STATE_DOWN,
        ADEV_DAPM_STATE_UP
    } adev_dapm_state_t;

    // DAPM data structure
    typedef struct
    {
        uint16_t    state;                  // Dapm state
        uint16_t    dev_num;                // Device number added in DAPM
        adev_dev_t *up_list[ADEV_EP_NUM];   // Device active sequence
        adev_dev_t *down_list[ADEV_EP_NUM]; // Deive de-active sequence
    } adev_dapm_t;

    /**
     ****************************************************************************************
     * @brief Init DAPM
     * @param dapm DAPM instance
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_dapm_init(adev_dapm_t *dapm);

    /**
     ****************************************************************************************
     * @brief Add a device into DAPM. Device instance will be freed by
     *adev_dapm_deinit() after adding into DAPM by adev_dapm_add_dev()
     * @param dapm Pointer of DAPM instance
     * @param dev Pointer of device
     * @return ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_dapm_add_dev(adev_dapm_t *dapm, adev_dev_t *dev);

    /**
     ****************************************************************************************
     * @brief DAPM start to probe devices. This API should be called after all
     *the deives are add into DAPM
     * @param dapm Pointer of DAPM instance
     * @return ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_dapm_probe(adev_dapm_t *dapm);

    /**
     ****************************************************************************************
     * @brief DAPM power up
     * @param dapm DAPM instance
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_dapm_up(adev_dapm_t *dapm);

    /**
     ****************************************************************************************
     * @brief DAPM Power down
     * @param dapm DAPM instance
     * @return  ADEV error code
     ****************************************************************************************
     */
    uint32_t adev_dapm_down(adev_dapm_t *dapm);

    /**
     ****************************************************************************************
     * @brief Deinit DAPM. All the device instances will be freed if they are
     *added into DAPM
     * @param dapm DAPM instance
     ****************************************************************************************
     */
    void adev_dapm_deinit(adev_dapm_t *dapm);

#if defined __cplusplus
}
#endif
#endif /* _ADEV_DAPM_H_ */
