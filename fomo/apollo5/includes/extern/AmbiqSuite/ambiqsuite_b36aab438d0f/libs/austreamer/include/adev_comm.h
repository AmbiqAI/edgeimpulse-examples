/**
 ****************************************************************************************
 *
 * @file adev_comm.h
 *
 * @brief Declaration Audio Device common definition.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ADEV_COMM_H_
#define _ADEV_COMM_H_

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "acore_log.h"

#if defined __cplusplus
extern "C"
{
#endif

#define ADEV_LOG_TRACE(x, ...)   ACORE_LOG_TRACE(x, ##__VA_ARGS__);
#define ADEV_LOG_INFO(x, ...)    ACORE_LOG_INFO(x, ##__VA_ARGS__);
#define ADEV_LOG_WARNING(x, ...) ACORE_LOG_WARNING(x, ##__VA_ARGS__);
#define ADEV_LOG_ERROR(x, ...)   ACORE_LOG_ERROR(x, ##__VA_ARGS__);

// ADEV state code
#define ADEV_OK                   0
#define ADEV_WAIT_FOR_READY_EVENT 1
#define ADEV_ERR                  2
#define ADEV_ERR_EP_ID            3
#define ADEV_ERR_OP_INACTIVE      4
#define ADEV_ERR_OP_ACTIVE        5
#define ADEV_ERR_DAPM             6
#define ADEV_ERR_ALLOCATE         7

// Bit number of one Byte
#define BITS_PER_BYTE 8

// All the endpoint number
#define ADEV_EP_NUM 4

// I2S endpoint number
#define I2S_EP_NUM 2

// PDM endpoint number
#define PDM_EP_NUM 1

// ADC endpoint number
#define ADC_EP_NUM 1

    // Apollo5a endpoint definition
    typedef enum
    {
        ADEV_EP_I2S_0, // Endpoint I2S-0
        ADEV_EP_I2S_1, // Endpoint I2S-1
        ADEV_EP_PDM_0, // Endpoint PDM-0
        ADEV_EP_ADC_0, // Endpoint ADC-0
    } adev_ep_id_t;

    // DAPM device types
    typedef enum
    {
        ADEV_DAPM_ADC,     // analog to digital converter
        ADEV_DAPM_DAC,     // digital to analog converter
        ADEV_DAPM_MIC,     // microphone
        ADEV_DAPM_HP,      // headphones
        ADEV_DAPM_SPK,     // speaker
        ADEV_DAPM_LINE,    // line input/output
        ADEV_DAPM_SWITCH,  // analog switch
        ADEV_DAPM_SUPPLY,  // power/clock supply
        ADEV_DAPM_PINCTRL, // pinctrl
        ADEV_DAPM_CLOCK,   // external clock
        ADEV_DAPM_AIF,     // audio interface

        ADEV_DAPM_TYPE_COUNT // End flag. Don't add any item below
    } adev_dapm_type_t;

    // Endpoint types
    typedef enum
    {
        ADEV_EP_TYPE_I2S = ADEV_EP_I2S_0, // Endpoint type I2S
        ADEV_EP_TYPE_PDM = ADEV_EP_PDM_0, // Endpoint type PDM
        ADEV_EP_TYPE_ADC = ADEV_EP_ADC_0  // Endpoint type ADC
    } adev_ep_type_t;

    // Endpoint notification type. I2S endpoint supports RX and TX full-duplex
    typedef enum
    {
        ADEV_NOTIFY_RX, // RX Notify
        ADEV_NOTIFY_TX  // TX Notify
    } adev_notify_rxtx_t;

    // Audio sample rate
    typedef enum
    {
        ADEV_SAMPLE_RATE_8000_HZ  = 8000,  // Sample rate 8kHz
        ADEV_SAMPLE_RATE_16000_HZ = 16000, // Sample rate 16kHz
        ADEV_SAMPLE_RATE_48000_HZ = 48000, // Sample rate 48kHz
        ADEV_SAMPLE_RATE_96000_HZ = 96000  // Sample rate 96kHz
    } adev_sample_rate_t;

    // Audio sample width. Bit number of valid data
    typedef enum
    {
        ADEV_SAMPLE_WIDTH_8_BITS  = 8,  // Sample width 8 bits
        ADEV_SAMPLE_WIDTH_16_BITS = 16, // Sample width 16 bits
        ADEV_SAMPLE_WIDTH_24_BITS = 24, // Sample width 24 bits
        ADEV_SAMPLE_WIDTH_32_BITS = 32  // Sample width 32 bits
    } adev_sample_width_t;

    // Audio channel number. MONO is 1 channel, Stereo is 2 channel
    typedef enum
    {
        ADEV_CHANNEL_MONO   = 1, // 1 channel
        ADEV_CHANNEL_STEREO = 2  // 2 channel
    } adev_channels_t;

    // Audio clock source
    typedef enum
    {
        ADEV_CLKSRC_XTAL = 0, // External 32MHz XTAL with HFRC2Adj.
        ADEV_CLKSRC_MCLK = 1, // External MCLK connect to GPIO15.
        ADEV_CLKSRC_HFRC = 2, // Internal HFRC clock. Low power consumption.
        ADEV_CLKSRC_PLL  = 3  // External 32MHz XTAL with PLL. High precision.
    } adev_clksrc_t;

    // Audio data direction
    typedef enum
    {
        ADEV_DATA_UNKNOWN = 0, // Audio streaming direction is unknown
        ADEV_DATA_RX      = 1, // Audio streaming RX only
        ADEV_DATA_TX      = 2, // Audio streaming TX only
        ADEV_DATA_RX_TX   = 3  // Audio streaming are full-duplex, RX and TX.
    } adev_data_dir_t;

    // Declaration of ADEV device data struct
    typedef struct adev_dev_s adev_dev_t;

    // Declaration of notify callback function.
    // @param dev Device pointer
    typedef void (*adev_notify_f)(adev_dev_t *dev);

    // ADEV Configuration data structure
    typedef struct
    {
        uint8_t id;     // Index in its type. id of I2S-0 is 0, ADC-0 is 0.
        uint8_t type;   // Device type. Value defined by adev_ep_type_t.
        uint8_t enable; // Enable flag. 1 is enable, 0 is disable.
        uint8_t clksrc; // Clock source. Value defined by adev_clksrc_t
        uint8_t dir;    // Streaming direction. Value defined by adev_data_dir_t
        uint8_t channels;     // Channel type. Value defined by adev_channels_t
        uint8_t sample_width; // Bit number of valid data. Value defined by
                              // adev_sample_width_t
        uint8_t rsv[1];       // Reserve for 32bit alignment
        uint32_t
            sample_rate;    // Sample rate. Value defined by adev_sample_rate_t
        uint32_t mclk_freq; // MCLK frequency.
        uint32_t duration_us; // Duration of notify
    } adev_conf_t;

    // Operation of driver
    // Driver state machine
    // # is current state; -> or <- is new state
    // OP      |   Init     |   Probed  |   Ready  |   Active  |
    // probe   |     #      |     ->    |          |           |
    // prepare |            |     #     |     ->   |           |
    // start   |            |           |     #    |     ->    |
    // read    |            |           |          |     #     |
    // write   |            |           |          |     #     |
    // stop    |            |     <-    |          |     #     |
    // remove  |     <-     |     #     |     #    |           |
    typedef struct
    {
        // Probe operation will generate private data instance, parse
        // configuration, enable pinctrl for this device, initialize data
        // structure
        uint32_t (*probe)(adev_dev_t *dev);

        // Prepare operation will allocate buffer, configure and enable low
        // level device, enable clock source
        uint32_t (*prepare)(adev_dev_t *dev);

        // Start operation will enable DMA transfer
        uint32_t (*start)(adev_dev_t *dev);

        // Read operation run after start
        uint32_t (*read)(adev_dev_t *dev, char *buf, uint32_t size);

        // Write operation run after start
        uint32_t (*write)(adev_dev_t *dev, char *buf, uint32_t size);

        // Stop operation will disable DMA transfer, free buffer, low level
        // device, and disable clock source
        void (*stop)(adev_dev_t *dev);

        // Remove operation will remove private data instance, disable pinctrl
        // for this device
        void (*remove)(adev_dev_t *dev);

    } adev_ops_t;

    // ADEV Driver data structure
    typedef struct
    {
        adev_dapm_type_t type; // DAPM type
        adev_ops_t       ops;  // Driver operations
    } adev_drv_t;

    // ADEV Device data structure
    struct adev_dev_s
    {
        const adev_conf_t *conf;   // Driver configuration
        const adev_drv_t  *drv;    // Driver
        void              *priv;   // Driver private runtime data
        adev_notify_f      notify; // Notify callback
    };

#if defined __cplusplus
}
#endif
#endif /* _ADEV_COMM_H_ */
