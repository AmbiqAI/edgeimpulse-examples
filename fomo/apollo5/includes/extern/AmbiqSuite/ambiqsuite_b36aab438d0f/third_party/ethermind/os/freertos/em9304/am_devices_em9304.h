//*****************************************************************************
//
//! @file am_devices_em9304.h
//!
//! @brief Support functions for the EM Micro EM9304 BTLE radio.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_DEVICES_EM9304_H
#define AM_DEVICES_EM9304_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// EM9304 vendor specific events
//
//*****************************************************************************

//*****************************************************************************
//
// EM9304 device structure
//
//*****************************************************************************
typedef struct
{
    //
    // MODE UART vs IOM SPI
    //
    uint32_t ui32Mode;

    //
    // IOM Module #
    //
    uint32_t ui32IOMModule;

    //
    // IOM Chip Select NOTE: this driver uses GPIO for chip selects
    //
    uint32_t ui32IOMChipSelect;

    //
    // GPIO # for EM9304 DREADY signal
    //
    uint32_t ui32DREADY;
}
am_devices_em9304_t;

#if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
typedef enum
{
    AM_DEVICES_EM9304_STATUS_SUCCESS,
    AM_DEVICES_EM9304_STATUS_ERROR
} am_devices_em9304_status_t;
#endif // defined(AM_PART_APOLLO3)

extern const am_devices_em9304_t g_sEm9304;
#if defined(AM_PART_APOLLO) || defined(AM_PART_APOLLO2)
extern const am_hal_iom_config_t g_sEm9304IOMConfigSPI;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern bool am_devices_em9304_mac_set(const uint8_t *pui8MacAddress);
extern uint32_t am_devices_em9304_block_read(const am_devices_em9304_t *psDevice,
                                             uint32_t *pui32Values,
                                             uint32_t ui32NumBytes);
extern void am_devices_em9304_block_write(const am_devices_em9304_t *psDevice,
                                              uint8_t type,
                                              uint8_t *pui8Values,
                                              uint32_t ui32NumBytes);

extern void am_devices_em9304_spi_init(uint32_t ui32Module, const am_hal_iom_config_t *psIomConfig);
extern uint8_t am_devices_em9304_tx_starts(const am_devices_em9304_t *psDevice);
extern void am_devices_em9304_tx_ends(void);
extern void am_devices_em9304_config_pins(void);
extern void am_devices_em9304_spi_sleep(uint32_t ui32Module);
extern void am_devices_em9304_spi_awake(uint32_t ui32Module);
extern void am_devices_em9304_enable_interrupt(void);
extern void am_devices_em9304_disable_interrupt(void);
#endif // defined(AM_PART_APOLLO) || defined(AM_PART_APOLLO2)

#if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
extern /*const*/ am_hal_iom_config_t g_sEm9304IOMConfigSPI;

extern uint32_t am_devices_em9304_spi_init(uint32_t ui32Module, am_hal_iom_config_t *psIomConfig);
// #### INTERNAL BEGIN ####
extern uint32_t am_devices_em9304_fullduplex(uint8_t *pui8TxBuffer,
                                             uint32_t ui32WriteAddress,
                                             uint32_t ui32TxNumBytes,
                                             am_hal_iom_callback_t pfnTxCallback,
                                             uint8_t *pui8RxBuffer,
                                             uint32_t ui32ReadAddress,
                                             uint32_t ui32RxNumBytes,
                                             am_hal_iom_callback_t pfnRxCallback);
extern void am_devices_em9304_fullduplex_ends(void);
// #### INTERNAL END ####

#endif // defined(AM_PART_APOLLO3)

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_EM9304_H
