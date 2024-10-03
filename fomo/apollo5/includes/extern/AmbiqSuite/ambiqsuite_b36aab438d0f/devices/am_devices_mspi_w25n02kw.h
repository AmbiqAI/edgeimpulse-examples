//*****************************************************************************
//
//! @file am_devices_mspi_w25n02kw.h
//!
//! @brief Multibit SPI w25n02kw NAND flash driver.
//!
//! @addtogroup mspi_w25n02kw W25N02KW MSPI Driver
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

#ifndef AM_DEVICES_MSPI_W25N02KW_H
#define AM_DEVICES_MSPI_W25N02KW_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Global definitions for flash commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_W25N02KW_PROGRAM_LOAD_X1 0x02
#define AM_DEVICES_MSPI_W25N02KW_PROGRAM_LOAD_X4 0x32
#define AM_DEVICES_MSPI_W25N02KW_PROGRAM_EXECUTE 0x10

#define AM_DEVICES_MSPI_W25N02KW_READ_CELL_ARRAY 0x13
#define AM_DEVICES_MSPI_W25N02KW_READ_BUFFER_X1  0x03
#define AM_DEVICES_MSPI_W25N02KW_READ_BUFFER_X2  0x3B
#define AM_DEVICES_MSPI_W25N02KW_READ_BUFFER_X4  0x6B

#define AM_DEVICES_MSPI_W25N02KW_WRITE_DISABLE   0x04
#define AM_DEVICES_MSPI_W25N02KW_WRITE_ENABLE    0x06

#define AM_DEVICES_MSPI_W25N02KW_READ_ID         0x9F

#define AM_DEVICES_MSPI_W25N02KW_BLOCK_ERASE     0xD8

#define AM_DEVICES_MSPI_W25N02KW_SET_FEATURE     0x1F
#define AM_DEVICES_MSPI_W25N02KW_GET_FEATURE     0x0F
#define AM_DEVICES_MSPI_W25N02KW_FEATURE_STATUS  0xC0
#define AM_DEVICES_MSPI_W25N02KW_FEATURE_B0      0xB0
#define AM_DEVICES_MSPI_W25N02KW_FEATURE_A0      0xA0

#define AM_DEVICES_MSPI_W25N02KW_RESET           0xFF
//! @}

//*****************************************************************************
//
//! @name Device specific identification.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_W25N02KW_ID             0xEFBA22
#define AM_DEVICES_MSPI_W25N02KW_ID_MASK        0xFFFFFF
//! @}
//*****************************************************************************
//
//! @name Device specific definitions for the flash size information
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_W25N02KW_PAGE_DATA_SIZE 2048
#define AM_DEVICES_MSPI_W25N02KW_PAGE_OOB_SIZE  64
#define AM_DEVICES_MSPI_W25N02KW_PAGE_FULL_SIZE 2112 //Internal ECC is enabled, default; 64 bytes are used for redundancy or for other uses
#define AM_DEVICES_MSPI_W25N02KW_BLOCK_SIZE     0x20000  //128K bytes
#define AM_DEVICES_MSPI_W25N02KW_MAX_BLOCKS     2048
#define AM_DEVICES_MSPI_W25N02KW_MAX_PAGES      131072
//! @}
//*****************************************************************************
//
//! @name Global definitions for the flash status register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_W25N02KW_ECCS  0x30   // ECC Status[1:0]
#define AM_DEVICES_W25N02KW_PRG_F 0x08   // Program Fail
#define AM_DEVICES_W25N02KW_ERS_F 0x04   // Erase Fail
#define AM_DEVICES_W25N02KW_WEL   0x02   // Write enable latch
#define AM_DEVICES_W25N02KW_OIP   0x01   // Operation in progress

#define AM_DEVICES_W25N02KW_ECCS_NO_BIT_FLIPS            0x00
#define AM_DEVICES_W25N02KW_ECCS_BIT_FLIPS_CORRECTED     0x10
#define AM_DEVICES_W25N02KW_ECCS_BIT_FLIPS_NOT_CORRECTED 0x20
#define AM_DEVICES_W25N02KW_ECCS_BIT_FLIPS_CORRECTED_THR 0x30 // more than the threshold bit
//! @}

//*****************************************************************************
//
//! @name Global definitions for the flash OTP register
//! @{
//
//*****************************************************************************
#define AM_DEVICES_W25N02KW_OTP_DATA_LOCK   0x80
#define AM_DEVICES_W25N02KW_ENTER_OTP       0x40
#define AM_DEVICES_W25N02KW_ECC_EN          0x10
#define AM_DEVICES_W25N02KW_BUFFER_MODE     0x08    //not supported for KW
#define AM_DEVICES_W25N02KW_HOLD_DISABLE    0x01
//! @}

//*****************************************************************************
//
//! @name Global definitions for the MSPI instance to use.
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_W25N02KW_MSPI_INSTANCE  0
#define AM_DEVICES_MSPI_W25N02KW_MAX_DEVICE_NUM 1
//! @}
//*****************************************************************************
//
//! @name Global type definitions.
//! @{
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_MSPI_W25N02KW_STATUS_SUCCESS,
    AM_DEVICES_MSPI_W25N02KW_STATUS_ERROR
} am_devices_mspi_w25n02kw_status_t;

typedef struct
{
    am_hal_mspi_device_e eDeviceConfig;
    am_hal_mspi_clock_e eClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
    uint32_t ui32ScramblingStartAddr;
    uint32_t ui32ScramblingEndAddr;
} am_devices_mspi_w25n02kw_config_t;

typedef struct
{
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    uint32_t ui32Turnaround;
    uint32_t ui32Rxneg;
    uint32_t ui32Rxdqsdelay;
#elif defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_APOLLO5_API)
    bool            bTxNeg;
    bool            bRxNeg;
    bool            bRxCap;
    uint8_t         ui8TxDQSDelay;
    uint8_t         ui8RxDQSDelay;
    uint8_t         ui8Turnaround;
#endif
} am_devices_mspi_w25n02kw_sdr_timing_config_t;

typedef enum
{
    AM_DEVICES_MSPI_W25N02KW_ECC_STATUS_NO_BIT_FLIPS = 0,
    AM_DEVICES_MSPI_W25N02KW_ECC_STATUS_BIT_FLIPS_CORRECTED,
    AM_DEVICES_MSPI_W25N02KW_ECC_STATUS_BIT_FLIPS_NOT_CORRECTED
} am_devices_mspi_w25n02kw_ecc_status_t;
//! @}
//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief
//! @param ui32Module
//! @param psMSPISettings
//! @param ppHandle
//! @param ppMspiHandle
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_mspi_w25n02kw_init(uint32_t ui32Module,
                                                const am_devices_mspi_w25n02kw_config_t *psMSPISettings,
                                                void **ppHandle, void **ppMspiHandle);

//*****************************************************************************
//
//! @brief
//! @param pHandle
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_mspi_w25n02kw_deinit(void *pHandle);

//*****************************************************************************
//
//! @brief Read the ID of the NAND flash.
//!
//! @param pHandle - W25N02KW device handle.
//! @param pui32DeviceID - a variable pointer to store the ID of the NAND flash.
//!
//! This function reads ID of the external flash.
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_w25n02kw_id(void *pHandle,
                                            uint32_t *pui32DeviceID);

//*****************************************************************************
//
//! @brief Read the contents of a certain page from the NAND flash into a buffer.
//!
//! @param pHandle - W25N02KW device handle.
//! @param ui32PageNum - Page number, for this NAND, valid value from 0 to 65535.
//! @param pui8DataBuffer - Buffer to store the data read from the NAND data region.
//! @param ui32DataLen - Number of bytes to read from the NAND data region.
//! @param pui8OobBuffer - Buffer to store the data read from the NAND oob region.
//! @param ui32OobLen - Number of bytes to read from the NAND oob region.
//! @param pui32EccResult - Return ECC result according ECCS[1:0] value.
//! 0--no bit flips; 1--bit flips corrected; 2--bit flips not corrected.
//!
//! This function reads the external flash at the certain page and stores
//! the received data into the provided buffer location. This function will
//! only store ui32DataLen bytes of data region and ui32OobLen bytes of oob region.
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_w25n02kw_read(void *pHandle, uint32_t ui32PageNum,
                                                uint8_t *pui8DataBuffer,
                                                uint32_t ui32DataLen,
                                                uint8_t *pui8OobBuffer,
                                                uint32_t ui32OobLen,
                                                uint8_t *pui32EccResult);

//*****************************************************************************
//
//! @brief Write the contents of a certain page to the NAND flash.
//!
//! @param pHandle - W25N02KW device handle.
//! @param ui32PageNum - Page number, for this NAND, valid value from 0 to 65535.
//! @param pui8DataBuffer - Buffer to store the data writen to the NAND data region.
//! @param ui32DataLen - Number of bytes to write to the NAND data region.
//! @param pui8OobBuffer - Buffer to store the data write to the NAND oob region.
//! @param ui32OobLen - Number of bytes to write to the NAND oob region.
//!
//! This function writes the data to the certain page on the external flash.
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_w25n02kw_write(void *pHandle,
                                                uint32_t ui32PageNum,
                                                uint8_t *pui8DataBuffer,
                                                uint32_t ui32DataLen,
                                                uint8_t *pui8OobBuffer,
                                                uint32_t ui32OobLen);

//*****************************************************************************
//
//! @brief Erase a certain page of the NAND flash.
//!
//! @param pHandle - W25N02KW device handle.
//! @param ui32BlockNum - block number, for this NAND, valid value from 0 to 1024.
//!
//! This function erases a certain block of the external flash.
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_w25n02kw_block_erase(void *pHandle,
                                                     uint32_t ui32BlockNum);

//*****************************************************************************
//
//! @brief Apply given SDR timing settings to target MSPI instance.
//!
//! @param pHandle - Handle to the NandFlash.
//! @param pDevSdrCfg - Pointer to the ddr timing config structure
//!
//! This function applies the ddr timing settings to the selected mspi instance.
//! This function must be called after MSPI instance is initialized into
//! ENABLEFINEDELAY0 = 1 mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t
am_devices_mspi_w25n02kw_apply_sdr_timing(void *pHandle,
                                         am_devices_mspi_w25n02kw_sdr_timing_config_t *pDevSdrCfg);
//*****************************************************************************
//
//! @brief Checks W25N02KW timing and determine a delay setting.
//!
//! @param module
//! @param pDevCfg
//! @param pDevSdrCfg
//!
//! This function scans through the delay settings of MSPI SDR mode and selects
//! the best parameter to use by tuning TURNAROUND/RXNEG/RXDQSDELAY0 values.
//! This function is only valid in SDR mode and ENABLEDQS0 = 0.
//!
//! @return 32-bit status, scan result in structure type
//
//*****************************************************************************
extern uint32_t
am_devices_mspi_w25n02kw_sdr_init_timing_check(uint32_t module,
                                              const am_devices_mspi_w25n02kw_config_t *pDevCfg,
                                              am_devices_mspi_w25n02kw_sdr_timing_config_t *pDevSdrCfg);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_MSPI_W25N02KW_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
