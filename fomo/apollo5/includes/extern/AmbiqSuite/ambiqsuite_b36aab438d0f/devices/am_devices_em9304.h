//*****************************************************************************
//
//! @file am_devices_em9304.h
//!
//! @brief Support functions for the EM Micro EM9304 BTLE radio.
//!
//! @addtogroup em9304 EM9304 BTLE Radio Device Driver
//! @ingroup devices
//! @{
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
//! EM9304 device structure
//
//*****************************************************************************
typedef struct
{
    //
    //! MODE UART vs IOM SPI
    //
    uint32_t ui32Mode;

    //
    //! IOM Module #
    //
    uint32_t ui32IOMModule;

    //
    //! IOM Chip Select NOTE: this driver uses GPIO for chip selects
    //
    uint32_t ui32IOMChipSelect;

    //
    //! GPIO # for EM9304 DREADY signal
    //
    uint32_t ui32DREADY;
}
am_devices_em9304_t;

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

#if (defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P))

//
//! module status enum definitions
//
typedef enum
{
    AM_DEVICES_EM9304_STATUS_SUCCESS,
    AM_DEVICES_EM9304_STATUS_ERROR
} am_devices_em9304_status_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} am_devices_em9304_config_t;

#define AM_DEVICES_EM9304_MAX_DEVICE_NUM    1

//*****************************************************************************
//
//! @brief EM9304 SPI/IOM initialization function.
//!
//! @param ui32Module - The IOM module to be used for EM9304.
//! @param pDevConfig - The configuration information for the IOM.
//! @param ppHandle   -
//! @param ppIomHandle -
//!
//! This function initializes the IOM for operation with the EM9304.
//!
//! @return  value from am_devices_em9304_status_t
//
//*****************************************************************************
extern uint32_t am_devices_em9304_init(uint32_t ui32Module,
                                       am_devices_em9304_config_t *pDevConfig,
                                       void **ppHandle,
                                       void **ppIomHandle);

//*****************************************************************************
//
//! @brief EM9304 SPI/IOM de-initialization function.
//!
//! @param pHandle     - Device handle#
//!
//! @return status.
//
//*****************************************************************************
extern uint32_t am_devices_em9304_term(void *pHandle);

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
//! @brief Apollo3 full duplex SPI transaction.
//!
//! @param pHandle
//! @param ui32WriteAddress
//! @param pui8TxBuffer
//! @param pui8RxBuffer
//! @param ui32TxNumBytes
//!
//! This function handles a full duplex transaction on the EM9304.
//!
//! @return status
//
//*****************************************************************************
extern uint32_t am_devices_em9304_fullduplex(void *pHandle, uint32_t ui32WriteAddress,
                             uint8_t *pui8TxBuffer,
                             uint8_t *pui8RxBuffer,
                             uint32_t ui32TxNumBytes);
// #### INTERNAL END ####

#endif // defined(AM_PART_APOLLO3)

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_EM9304_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

