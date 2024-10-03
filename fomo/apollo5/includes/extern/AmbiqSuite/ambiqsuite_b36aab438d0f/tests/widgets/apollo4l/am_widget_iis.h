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

extern const unsigned char acstereo_sine_1K_16KHz[];
extern const unsigned char acstereo_sine_1K_48KHz[];

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

#endif // AM_WIDGET_IIS_H
