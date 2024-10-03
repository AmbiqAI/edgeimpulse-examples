//*****************************************************************************
//
//! @file am_devices_tsl2540.c
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

#include "am_mcu_apollo.h"
#include "am_devices_tsl2540.h"
#include "am_bsp.h"
#include "string.h"
#include "am_util_delay.h"

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************

//
//! TSL2540 IOM Device instance
//
am_devices_iom_tsl2540_t g_AmTsl2540[AM_DEVICES_TSL2540_MAX_DEVICE_NUM] = {0};

//
//! TSL2540 IOM Device Config instance
//
am_hal_iom_config_t     g_sI2cTsl2540Cfg =
{
    .eInterfaceMode       = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq        = AM_HAL_IOM_400KHZ,
    .ui32NBTxnBufLength   = 0,
    .pNBTxnBuf = NULL,
};

//*****************************************************************************
//
// Init function for TLS2540
// This function will initialize the IOM for the TSL2540 Ambient Light Sensor
//      and will check the Device ID register to confirm the chip is correct
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_init(uint32_t ui32Module,
                        am_hal_iom_config_t *pDevConfig,
                        void **ppHandle,
                        void **ppIomHandle)
{
    void *pIomHandle;
    uint32_t ui32Status = AM_DEVICES_TSL2540_STATUS_SUCCESS;
    am_hal_iom_config_t     sIomTls2540Settings;
    uint32_t CS[AM_REG_IOM_NUM_MODULES] =
#ifdef APOLLO3P_EVB_CYGNUS
    {
        0,
        AM_BSP_IOM1_CS_CHNL,
        0,
        AM_BSP_IOM3_CS_CHNL,
        AM_BSP_IOM4_CS_CHNL,
        0
    };
#elif defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L)
    { 0, 0, 0, 0, 0, 0, 0, 0 };
#endif

    uint32_t ui32Index = 0;

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < AM_DEVICES_TSL2540_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( g_AmTsl2540[ui32Index].bOccupied == false )
        {
            break;
        }
    }

    if ( ui32Index == AM_DEVICES_TSL2540_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    if ( (ui32Module > AM_REG_IOM_NUM_MODULES)  || (pDevConfig == NULL) )
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    sIomTls2540Settings = g_sI2cTsl2540Cfg;
    sIomTls2540Settings.ui32NBTxnBufLength = pDevConfig->ui32NBTxnBufLength;
    sIomTls2540Settings.pNBTxnBuf = pDevConfig->pNBTxnBuf;
    sIomTls2540Settings.ui32ClockFreq = pDevConfig->ui32ClockFreq;

    //
    // Look up the Max sTransaction size to fit into 8usec for CE asserted
    //
    g_AmTsl2540[ui32Index].ui32MaxTransSize = 0;

    //
    // Configure the IOM pins.
    //
    am_bsp_iom_pins_enable(ui32Module, g_sI2cTsl2540Cfg.eInterfaceMode);

    //
    // Enable fault detection.
    //
#if !defined(AM_PART_APOLLO5_API)
#if defined(AM_PART_APOLLO4_API)
    am_hal_fault_capture_enable();
#elif AM_PART_APOLLO3_API
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_ENABLE, 0);
#else
    am_hal_mcuctrl_fault_capture_enable();
#endif
#endif

    //
    // Initialize the IOM instance.
    // Enable power to the IOM instance.
    // Configure the IOM for Serial operation during initialization.
    // Enable the IOM.
    //
    if (am_hal_iom_initialize(ui32Module, &pIomHandle) ||
        am_hal_iom_power_ctrl(pIomHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_iom_configure(pIomHandle, &sIomTls2540Settings) ||
        am_hal_iom_enable(pIomHandle))
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }
    else
    {
        am_util_delay_us(150);
        g_AmTsl2540[ui32Index].ui32Module = ui32Module;
        g_AmTsl2540[ui32Index].ui32CS = CS[ui32Module];
        *ppIomHandle = g_AmTsl2540[ui32Index].pIomHandle = pIomHandle;
        *ppHandle = (void *)&g_AmTsl2540[ui32Index];
    }

    return ui32Status;
}

//*****************************************************************************
//
// Initialize the TLS2540 driver without initialiizing a new IOM
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_no_iom_init(uint32_t ui32Module, am_hal_iom_config_t *pDevConfig, void **ppHandle, void **ppIomHandle)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)ppHandle;
    g_AmTsl2540[0] = *pIom;
    g_AmTsl2540[0].pIomHandle = *ppIomHandle;

    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
// DeInitialize the TSL2540 driver.
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_term(void *pHandle)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    if ( pIom->ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    // Disable the pins
    am_bsp_iom_pins_disable(pIom->ui32Module, AM_HAL_IOM_SPI_MODE);

    //
    // Disable the IOM.
    //
    if ( AM_DEVICES_TSL2540_STATUS_SUCCESS != am_hal_iom_disable(pIom->pIomHandle) )
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    //
    // Disable power to and uninitialize the IOM instance.
    //
    if ( AM_DEVICES_TSL2540_STATUS_SUCCESS != am_hal_iom_power_ctrl(pIom->pIomHandle, AM_HAL_SYSCTRL_DEEPSLEEP, true) )
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    if ( AM_DEVICES_TSL2540_STATUS_SUCCESS != am_hal_iom_uninitialize(pIom->pIomHandle) )
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    // Free this device handle
    pIom->bOccupied = false;

    //
    // Return the status.
    //
    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//! @param pHandle
//! @param pui8RxBuffer
//! @param ui32ReadAddress
//! @param ui32NumBytes
//! @param pfnCallback
//! @param pCallbackCtxt
//! @return
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_nonblocking_read(void *pHandle, uint8_t *pui8RxBuffer,
                                    uint32_t ui32ReadAddress,
                                    uint32_t ui32NumBytes,
                                    am_hal_iom_callback_t pfnCallback,
                                    void * pCallbackCtxt)
{
    am_hal_iom_transfer_t         Transaction;

    //
    // Set up the IOM transaction.
    //
    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32InstrLen    = 1;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L))
    Transaction.ui64Instr       = (ui32ReadAddress & 0x0000FFFF);
#else
    Transaction.ui32Instr       = (ui32ReadAddress & 0x0000FFFF);
#endif
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32RxBuffer   = (uint32_t *)pui8RxBuffer;
    Transaction.uPeerInfo.ui32I2CDevAddr = TSL2540_ADDR;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    Transaction.bContinue       = false;

    //
    // Start the transaction.
    //
    if (am_hal_iom_nonblocking_transfer(pHandle, &Transaction, pfnCallback, pCallbackCtxt))
    {
        return AM_HAL_STATUS_FAIL;
    }

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Generic Command Read function.
//!
//! @param pHandle
//! @param ui8DevAddr
//! @param ui32InstrLen
//! @param ui64Instr
//! @param bCont
//! @param pData
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
static uint32_t
am_device_tsl2540_command_read(void *pHandle, uint8_t ui8DevAddr, uint32_t ui32InstrLen, uint64_t ui64Instr,
                               bool bCont, uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t  sTransaction;

    //
    // Create the sTransaction.
    //
    sTransaction.ui32InstrLen    = ui32InstrLen;
#if (defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L))
    sTransaction.ui64Instr       = ui64Instr;
#else
    sTransaction.ui32Instr       = (uint32_t)ui64Instr;
#endif
    sTransaction.eDirection      = AM_HAL_IOM_RX;
    sTransaction.ui32NumBytes    = ui32NumBytes;
    sTransaction.pui32RxBuffer   = pData;
    sTransaction.uPeerInfo.ui32I2CDevAddr = ui8DevAddr;
    sTransaction.bContinue       = bCont;
    sTransaction.ui8RepeatCount  = 0;
    sTransaction.ui32PauseCondition = 0;
    sTransaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pHandle, &sTransaction))
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }
    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_nonblocking_read_id(void *pHandle, uint32_t *ui32data_p, am_hal_iom_callback_t pFunctionCallback)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    //
    // Send the command sequence to read the Device ID.
    //
    if (am_devices_tsl2540_nonblocking_read(pIom->pIomHandle, (uint8_t *)ui32data_p, TSL2540_ID_REG, 1, pFunctionCallback, 0))
    {
        return AM_HAL_STATUS_FAIL;
    }

    *ui32data_p &= 0xFF;

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}
//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_nonblocking_read_regs(void *pHandle, uint8_t *ui8data_p, uint8_t length_of_read, am_hal_iom_callback_t pFunctionCallback)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    //
    // Send the command sequence to read the Device ID.
    //
    if (am_devices_tsl2540_nonblocking_read(pIom->pIomHandle, ui8data_p, TSL2540_REVID_REG, length_of_read, pFunctionCallback, 0))
    {
        return AM_HAL_STATUS_FAIL;
    }

    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reads the ID of the external flash and returns the value.
// This function reads the device ID register of the external flash and returns
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_read_id(void *pHandle, uint32_t *ui32data_p)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    //
    // Send the command sequence to read the Device ID.
    //
    if (am_device_tsl2540_command_read(pIom->pIomHandle, TSL2540_ADDR, 1,
                           TSL2540_ID_REG,
                           false, ui32data_p, 1))
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    *ui32data_p &= 0xFF;

    //
    // Return the status.
    //
    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read Revision ID for TLS2540
// This function will return the 8-bit data for the TSL2540_REVID_REG register
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_read_revision_id(void *pHandle, uint32_t *ui32data_p)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    am_util_delay_us(10);

    if (am_device_tsl2540_command_read(pIom->pIomHandle, TSL2540_ADDR, 1,
                            TSL2540_REVID_REG, false, ui32data_p, 1))
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    *ui32data_p &= 0xFF;

    //
    // Return the status.
    //
    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read Revision Auxiliary ID for TLS2540
// This function will return the 8-bit data for the TSL2540_REVID2_REG register
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_read_revision_id2(void *pHandle, uint32_t *ui32data_p)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    am_util_delay_us(10);

    if (am_device_tsl2540_command_read(pIom->pIomHandle, TSL2540_ADDR, 1,
                            TSL2540_REVID2_REG, false, ui32data_p, 1))
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    *ui32data_p &= 0xFF;

    //
    // Return the status.
    //
    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read Status from TLS2540
// This function will return the 8-bit data for the STATUS register
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_read_status(void *pHandle, uint32_t *ui32data_p)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    am_util_delay_us(10);

    if (am_device_tsl2540_command_read(pIom->pIomHandle, TSL2540_ADDR, 1,
                            TSL2540_STATUS_REG, false, ui32data_p, 1))
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    *ui32data_p &= 0xFF;

    //
    // Return the status.
    //
    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read the Low Byte of Visual Data from TLS2540
// This function will return the low byte of the 16-bit visible channel data.
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_read_visual_data_low_byte(void *pHandle, uint32_t *ui32data_p)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    am_util_delay_us(10);

    if (am_device_tsl2540_command_read(pIom->pIomHandle, TSL2540_ADDR, 1,
                            TSL2540_VISDATAL_REG, false, ui32data_p, 1))
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    *ui32data_p &= 0xFF;

    //
    // Return the status.
    //
    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read the High Byte of Visual Data from TLS2540
// This function will return the high byte of the 16-bit visible channel data.
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_read_visual_data_high_byte(void *pHandle, uint32_t *ui32data_p)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    am_util_delay_us(10);

    if (am_device_tsl2540_command_read(pIom->pIomHandle, TSL2540_ADDR, 1,
                            TSL2540_VISDATAH_REG, false, ui32data_p, 1))
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    *ui32data_p &= 0xFF;

    //
    // Return the status.
    //
    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read the Low Byte of IR Data from TLS2540
// This function will return the low byte of the 16-bit IR channel data.
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_read_is_data_low_byte(void *pHandle, uint32_t *ui32data_p)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    am_util_delay_us(10);

    if (am_device_tsl2540_command_read(pIom->pIomHandle, TSL2540_ADDR, 1,
                            TSL2540_IRDATAL_REG, false, ui32data_p, 1))
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    *ui32data_p &= 0xFF;

    //
    // Return the status.
    //
    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read the High Byte of IR Data from TLS2540
// This function will return the high byte of the 16-bit IR channel data.
//
//*****************************************************************************
uint32_t
am_devices_tsl2540_read_ir_data_high_byte(void *pHandle, uint32_t *ui32data_p)
{
    am_devices_iom_tsl2540_t *pIom = (am_devices_iom_tsl2540_t *)pHandle;

    am_util_delay_us(10);

    if (am_device_tsl2540_command_read(pIom->pIomHandle, TSL2540_ADDR, 1,
                            TSL2540_IRDATAH_REG, false, ui32data_p, 1))
    {
        return AM_DEVICES_TSL2540_STATUS_ERROR;
    }

    *ui32data_p &= 0xFF;

    //
    // Return the status.
    //
    return AM_DEVICES_TSL2540_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

