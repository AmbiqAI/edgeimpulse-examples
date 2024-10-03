/**
 ****************************************************************************************
 *
 * @file adev_drv_i2s.h
 *
 * @brief Declaration of the Audio Device I2S Driver API.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_DRV_I2S_H_
#define _ADEV_DRV_I2S_H_

#include "adev_comm.h"

#if defined __cplusplus
extern "C"
{
#endif
    //                    ====  I2S Wave  ====
    //
    //              frame width
    //       |=======================|                       |====
    //       |------------------|    |------------------|    |
    //       |   sample width   |    |   sample width   |    |
    //   ====|                       |=======================|
    //                                     frame width
    //
    // I2S frame width. Frame width >= sample width.
    //
    // Common Configuration
    // Sample width  |  Frame width
    //   8                  8
    //   16                 16
    //   16                 32
    //   24                 32
    //   32                 32
    typedef enum
    {
        ADEV_FRAME_WIDTH_8_BITS  = 8,  // I2S frame width 8bit
        ADEV_FRAME_WIDTH_16_BITS = 16, // I2S frame width 16bit
        ADEV_FRAME_WIDTH_32_BITS = 32  // I2S frame width 32bit. I2S 24 sample
                                       // width uses 32bit frame width.
    } adev_i2s_frame_width_t;

    // I2S role, master or slave.
    // Master: I2S BCK and LRCLK are output
    // Slave: I2S BCK and LRCLK are input
    typedef enum
    {
        ADEV_I2S_SLAVE, // I2S Slave mode
        ADEV_I2S_MASTER // I2S Master mode
    } adev_i2s_role_t;

    // I2S Data alignment.
    typedef enum
    {
        ADEV_I2S_STANDARD, // I2S standard format is left-justified with 1bit
                           // delay
        ADEV_I2S_LEFT_JUSTIFIED, // Left-justified with 0 bit delay
        ADEV_I2S_RIGHT_JUSTIFIED // Right-justified with 0 bit delay
    } adev_i2s_align_t;

    // I2S Configuration
    typedef struct
    {
        adev_conf_t comm;        // ADEV confiuration common header
        uint8_t     frame_width; // I2S frame bit number. Value defined by
                                 // adev_i2s_frame_width_t
        uint8_t role;            // I2S role. Value defined by adev_i2s_role_t.
        uint8_t align;  // I2S alignment. Value defined by adev_i2s_align_t.
        uint8_t rsv[1]; // Reserve for 32bit alignment
    } adev_conf_i2s_t;

    // ADEV I2S Driver
    extern const adev_drv_t adev_drv_i2s;

#if defined __cplusplus
}
#endif
#endif /* _ADEV_DRV_I2S_H_ */
