//*****************************************************************************
//
//! @file am_devices_tsl2540.h
//!
//! @brief Device Driver for the TSL2540 Ambient Light Sensor
//!
//! @addtogroup tsl2540 TSL2540 Driver
//! @ingroup devices
//! @{
//
//**************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_DEVICES_TSL2540_H
#define AM_DEVICES_TSL2540_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_hal_iom.h"
#include "stdint.h"

typedef enum
{
    AM_DEVICES_TSL2540_STATUS_SUCCESS,
    AM_DEVICES_TSL2540_STATUS_ERROR
} am_devices_tls2540_status_e;

//*****************************************************************************
//
//! @name Global definitions for the commands
//! @{
//
//*****************************************************************************
#define TSL2540_ADDR            0x39
#define AM_DEVICES_TSL2540_ID   0xE4

#define TSL2540_ENABLE_REG      0x80 //!< ENABLE R/W Enables states and functions 0x00
#define TSL2540_ATIME_REG       0x81 //!< ATIME R/W ALS integration time 0x00
#define TSL2540_WTIME_REG       0x83 //!< WTIME R/W Wait time 0x00
#define TSL2540_AILTL_REG       0x84 //!< AILTL R/W ALS interrupt low threshold low byte 0x00
#define TSL2540_AILTH_REG       0x85 //!< AILTH R/W ALS interrupt low threshold high byte 0x00
#define TSL2540_AIHTL_REG       0x86 //!< AIHTL R/W ALS interrupt high threshold low byte 0x00
#define TSL2540_AIHTH_REG       0x87 //!< AIHTH R/W ALS interrupt high threshold high byte 0x00
#define TSL2540_PERS_REG        0x8C //!< PERS R/W ALS interrupt persistence filters 0x00
#define TSL2540_CFG0_REG        0x8D //!< CFG0 R/W Configuration register zero 0x80
#define TSL2540_CFG1_REG        0x90 //!< CFG1 R/W Configuration register one 0x00
#define TSL2540_REVID_REG       0x91 //!< REVID R Revision ID 0x61
#define TSL2540_ID_REG          0x92 //!< ID R Device ID 0xE4
#define TSL2540_STATUS_REG      0x93 //!< STATUS R Device status register 0x00
#define TSL2540_VISDATAL_REG    0x94 //!< VISDATAL R Visible channel data low byte 0x00
#define TSL2540_VISDATAH_REG    0x95 //!< VISDATAH R Visible channel data high byte 0x00
#define TSL2540_IRDATAL_REG     0x96 //!< IRDATAL R IR channel data low byte 0x00
#define TSL2540_IRDATAH_REG     0x97 //!< IRDATAH R IR channel data high byte 0x00
#define TSL2540_REVID2_REG      0x9E //!< REVID2 R Auxiliary ID REVID2
#define TSL2540_CFG2_REG        0x9F //!< CFG2 R/W Configuration register two 0x04
#define TSL2540_CFG3_REG        0xAB //!< CFG3 R/W Configuration register three 0x0C
#define TSL2540_AZ_CONFIG_REG   0xD6 //!< AZ_CONFIG R/W Autozero configuration 0x7F
#define TSL2540_INTENAB_REG     0xDD //!< INTENAB R/W Interrupt enables 0x00
//! @}

//*****************************************************************************
//
//! @name Global type definitions.
//! @{
//
//*****************************************************************************

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} am_devices_tsl2540_config_t;
//! @}

#define AM_DEVICES_TSL2540_MAX_DEVICE_NUM    1

typedef struct
{
    uint32_t                    ui32Module;
    uint32_t                    ui32CS;
    uint32_t                    ui32MaxTransSize;
    void                        *pIomHandle;
    bool                        bOccupied;
} am_devices_iom_tsl2540_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief DeInitialize the TLS2540 driver.
//!
//! @param *pHandle - Pointer to the Handle for the IOM Device Instance.
//!
//! @return status.
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_term(void *pHandle);

//*****************************************************************************
//
//! @brief Init the Device Driver without IOM for the TMA525 Touch Sensor
//!
//! @param ui32Module - IOM Module Number.
//! @param *pDevConfig - Pointer to the IOM Settings.
//! @param **ppHandle - Pointer to the Handle for the IOM Device Instance.
//! @param **ppIomHandle - Pointer to the Handle for the IOM Instance.
//!
//! Initialize the TMA525 driver for Multidrop if another device has already
//!  initialized the IOM for DMA and multidrop
//!
//! @return status - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_init(uint32_t ui32Module,
                                        am_hal_iom_config_t *pDevConfig,
                                        void **ppHandle,
                                        void **ppIomHandle);

//*****************************************************************************
//
//! @brief Initialize the TLS2540 driver without initialiizing a new IOM
//!
//! @param ui32Module - IOM Module Number.
//! @param *pDevConfig - Pointer to the IOM Settings.
//! @param **ppHandle - Pointer to the Handle for the IOM Device Instance.
//! @param **ppIomHandle - Pointer to the Handle for the IOM Instance.
//!
//!   This should only be used in the multidrop case where another device
//!         driver has initialized the IOM.
//!
//! @returns Status of i2c init
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_no_iom_init(uint32_t ui32Module,
                                               am_hal_iom_config_t *pDevConfig,
                                               void **ppHandle,
                                               void **ppIomHandle);

//*****************************************************************************
//
//! @brief Read Device ID for TLS2540
//!
//! @param pHandle
//! @param ui32data_p
//!
//! This function will return the 8-bit data for the TSL2540_ID_REG register
//!
//! @returns Status of i2c transfer from data get
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_read_id(void *pHandle,
                                           uint32_t *ui32data_p);

//*****************************************************************************
//
//! @brief
//! @param pHandle
//! @param ui32data_p
//! @param pFunctionCallback
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_nonblocking_read_id(void *pHandle,
                                                       uint32_t *ui32data_p,
                                                       am_hal_iom_callback_t pFunctionCallback);
//*****************************************************************************
//
//! @brief
//! @param pHandle
//! @param ui8data_p
//! @param length_of_read
//! @param pFunctionCallback
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_nonblocking_read_regs(void *pHandle,
                                                         uint8_t *ui8data_p,
                                                         uint8_t length_of_read,
                                                         am_hal_iom_callback_t pFunctionCallback);

//*****************************************************************************
//
//! @brief Read Revision ID for TLS2540
//!
//! @param pHandle
//! @param ui32data_p
//!
//! This function will return the 8-bit data for the TSL2540_REVID_REG register
//!
//! @returns Status of i2c transfer from data get
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_read_revision_id(void *pHandle,
                                                    uint32_t *ui32data_p);

//*****************************************************************************
//
//! @brief Read Revision Auxiliary ID for TLS2540
//!
//! @param pHandle
//! @param ui32data_p
//!
//! This function will return the 8-bit data for the TSL2540_REVID2_REG register
//!
//! @returns Status of i2c transfer from data get
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_read_revision_id2(void *pHandle,
                                                     uint32_t *ui32data_p);

//*****************************************************************************
//
//! @brief Read Status from TLS2540
//!
//! @param pHandle
//! @param ui32data_p
//!
//! This function will return the 8-bit data for the STATUS register
//!
//! @returns Status of i2c transfer from data get
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_read_status(void *pHandle,
                                               uint32_t *ui32data_p);

//*****************************************************************************
//
//! @brief Read the Low Byte of Visual Data from TLS2540
//!
//! @param pHandle
//! @param ui32data_p
//!
//! This function will return the low byte of the 16-bit visible channel data.
//!
//! @returns Status of i2c transfer from data get
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_read_visual_data_low_byte(void *pHandle,
                                                             uint32_t *ui32data_p);

//*****************************************************************************
//
//! @brief Read the High Byte of Visual Data from TLS2540
//!
//! @param pHandle
//! @param ui32data_p
//!
//! This function will return the high byte of the 16-bit visible channel data.
//!
//! @returns Status of i2c transfer from data get
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_read_visual_data_high_byte(void *pHandle,
                                                              uint32_t *ui32data_p);

//*****************************************************************************
//
//! @brief Read the Low Byte of IR Data from TLS2540
//!
//! @param pHandle
//! @param ui32data_p
//!
//! This function will return the low byte of the 16-bit IR channel data.
//!
//! @returns Status of i2c transfer from data get
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_read_is_data_low_byte(void *pHandle,
                                                         uint32_t *ui32data_p);

//*****************************************************************************
//
//! @brief Read the High Byte of IR Data from TLS2540
//!
//! @param pHandle
//! @param ui32data_p
//!
//! This function will return the high byte of the 16-bit IR channel data.
//!
//! @returns Status of i2c transfer from data get
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_read_ir_data_high_byte(void *pHandle,
                                                          uint32_t *ui32data_p);

//*****************************************************************************
//
//! @brief Read the ENABLE register from TLS2540
//!
//! @param pHandle
//! @param ui32data_p
//!
//! This function will return the status of the TSL2540_ENABLE_REG register
//!
//! @returns Status of i2c transfer from data get
//
//*****************************************************************************
extern uint32_t am_devices_tsl2540_read_enable(void *pHandle,
                                               uint32_t *ui32data_p);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_TSL3540_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

