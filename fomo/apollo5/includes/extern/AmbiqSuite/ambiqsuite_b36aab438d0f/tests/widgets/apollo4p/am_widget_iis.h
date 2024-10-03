//*****************************************************************************
//
//! @file am_widget_iis.h
//!
//! @brief Test widget for testing IIS channels by data transfer.
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

#ifndef AM_WIDGET_IIS_H
#define AM_WIDGET_IIS_H

#define AM_WIDGET_IIS_SUCCESS   AM_HAL_STATUS_SUCCESS

#define I2S_CHANNEL_0   0
#define I2S_CHANNEL_1   1

extern const unsigned char acstereo_sine_1K_16KHz[];
extern const unsigned char acstereo_sine_1K_48KHz[];

typedef struct i2s_asrc_half_duplex_settings
{
    uint32_t                    sample_rate;
    uint32_t                    master_channel; // I2S0 or I2S1, set the other one as slave
    am_hal_i2s_xfer_dir_e       master_role;    // master direction: halfduplex tx or rx, or fullduplex
                                                // slave applies setting accordingly
    am_hal_i2s_clksel_e m_clock_source;
    uint32_t  m_div3;

    am_hal_i2s_clksel_e s_clock_source;
    uint32_t      s_div3;

    float conversion;

}i2s_asrc_test_settings_t;

typedef struct i2s_loopback_test_settings
{
    uint32_t                    master_channel; // I2S0 or I2S1, set the other one as slave
    am_hal_i2s_xfer_dir_e       master_role;    // master direction: halfduplex tx or rx, or fullduplex
                                                // slave applies setting accordingly
    uint32_t                    clock_source;   // clock source selection
    uint32_t                    sclk_hz;        // sclk/bclk frequency
#ifdef TDM_MODE
    uint32_t                    ui32ChannelNumbersPhase1;
    uint32_t                    ui32ChannelNumbersPhase2;
#endif
    am_hal_i2s_channel_length_e data_width_1;   // data width in bits for phase 1
    am_hal_i2s_channel_length_e data_width_2;   // data width in bits for phase 2
#ifdef TDM_MODE
    am_hal_i2s_sample_length_e  eSampleLenPhase1;
    am_hal_i2s_sample_length_e  eSampleLenPhase2;
#endif
    am_hal_i2s_data_phase_e     phase;          // single or dual
    am_hal_i2s_data_justified_e justify;        // left or right justified
    am_hal_i2s_io_rx_cpol_e     rx_cpol;        // rx at falling or rising edge
    am_hal_i2s_io_tx_cpol_e     tx_cpol;        // tx at falling or rising edge
    am_hal_i2s_io_fsync_cpol_e  fsync_cpol;     // fsync polarity, high or low
}i2s_loopback_test_settings_t;

extern void am_widget_iis_handler_deinitialize(void);
extern uint32_t am_widget_iis_pin_enable(void);
extern uint32_t am_widget_iis_build_configuration(uint32_t iis_channel,
                                                  i2s_loopback_test_settings_t* test_setting,
                                                  am_hal_i2s_config_t* config);
extern uint32_t am_widget_iis_build_asrc_configuration(uint32_t iis_channel,
                                                       i2s_asrc_test_settings_t* test_setting,
                                                       am_hal_i2s_config_t* config);

/* #define AM_WIDGET_IOM_ERRMASK_NAK_DEVADDR       0x1
#define AM_WIDGET_IOM_ERRMASK_NAK_OFFSETSTART   0x2
#define AM_WIDGET_IOM_ERRMASK_NAK_OFFSETMID     0x4
#define AM_WIDGET_IOM_ERRMASK_NAK_CONT          0x8
#define AM_WIDGET_IOM_ERRMASK_DMA_START         0x10
#define AM_WIDGET_IOM_ERRMASK_DMA_MID           0x20

typedef enum
{
    AM_WIDGET_SUCCESS,
    AM_WIDGET_ERROR
} am_widget_iom_status_t;

typedef enum
{
    AM_WIDGET_IOM_BLOCKING,
    AM_WIDGET_IOM_NONBLOCKING
} am_widget_iom_mode_t;

typedef enum
{
    AM_WIDGET_DEVICE_MB85RS64V,     // SPI FRAM
    AM_WIDGET_DEVICE_MB85RC256V,    // I2C FRAM
    AM_WIDGET_DEVICE_EM9304,        // EM9304 (for duplex testing)
    AM_WIDGET_DEVICE_PSRAM          // PSRAM
} am_widget_iom_device_t;

typedef struct
{
    uint32_t              ui32Module;
    uint32_t              ui32DeviceType;
    am_hal_iom_config_t   IOMConfig;
} am_widget_iom_config_t;

typedef struct
{
    uint32_t                      ui32BufferSize;
    uint32_t                      ui32ByteOffset;
    uint32_t                      ui32QueueSize; // Relevant only for am_widget_iom_test_queue_write_read
    am_hal_sysctrl_power_state_e  sleepMode; // Relevant only for nonblocking tests
} am_widget_iom_test_t;

extern volatile uint32_t g_IOMInterruptStatus;
extern uint32_t am_widget_iom_test_setup(am_widget_iom_config_t *pTestCfg,
                             void **ppWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_cleanup(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_read_id(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_status_get(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_blocking_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_nonblocking_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_queue_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_duplex(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_power_saverestore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode);
extern uint32_t am_widget_iom_test_blocking_write_read_error(void *pWidget, void *pCfg, char *pErrStr, uint32_t errorMask);
extern uint32_t am_widget_iom_test_queue_write_read_error(void *pWidget, void *pCfg, char *pErrStr, uint32_t errorMask );*/

#endif // AM_WIDGET_IIS_H
