//*****************************************************************************
//
//! @file am_devices_st7301.c
//!
//! @brief sitronix TFT display driver.
//!
//! @addtogroup st7301 ST7301 - Sitronix TFT display driver
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

#include <string.h>
#include "am_mcu_apollo.h"
#include "am_devices_st7301.h"
#include "am_bsp.h"
#include "am_util_delay.h"

#ifdef USE_HW_DCX
#define DCX_HIGH()
#define DCX_LOW()
#else
#define DCX_HIGH()  am_hal_gpio_state_write(AM_BSP_GPIO_IOM0_DCX, AM_HAL_GPIO_OUTPUT_SET)
#define DCX_LOW()   am_hal_gpio_state_write(AM_BSP_GPIO_IOM0_DCX, AM_HAL_GPIO_OUTPUT_CLEAR)
#endif

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
typedef struct
{
    uint32_t                    ui32Module;
    void                        *pIomHandle;
    bool                        bOccupied;
} am_devices_iom_st7301_t;

am_devices_iom_st7301_t gAmSt7301[AM_DEVICES_ST7301_MAX_DEVICE_NUM];

static void
nb_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    *(volatile bool *)pCallbackCtxt = true;
}

am_hal_iom_config_t     g_sIomST7301Cfg =
{
    .eInterfaceMode       = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq        = AM_HAL_IOM_24MHZ,
    .eSpiMode             = AM_HAL_IOM_SPI_MODE_3,
    .ui32NBTxnBufLength   = 0,
    .pNBTxnBuf = NULL,
};

//*****************************************************************************
//
//! @brief Generic Command Write function.
//! @param pHandle
//! @param bHiPrio
//! @param ui32Instr
//! @param bdata
//! @return
//
//*****************************************************************************
uint32_t
am_devices_st7301_command_write(void *pHandle,
                                bool bHiPrio,
                                uint32_t ui32Instr,
                                bool bdata)
{
    am_hal_iom_transfer_t Transaction;
    am_devices_iom_st7301_t *pIom = (am_devices_iom_st7301_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = ui32Instr;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = NULL;
    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_DISPLAY_SPI_CS;
    Transaction.bContinue       = (bdata ? true :false);
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    DCX_LOW();
    if (bHiPrio)
    {
        volatile bool bDMAComplete = false;
        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_highprio_transfer(pIom->pIomHandle, &Transaction, nb_complete, (void*)&bDMAComplete))
        {
            return AM_DEVICES_ST7301_STATUS_ERROR;
        }
        while (!bDMAComplete);
    }
    else
    {
        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
        {
            return AM_DEVICES_ST7301_STATUS_ERROR;
        }
    }

    DCX_HIGH();
    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}
//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param ui32Instr
//!
//! @return
//
//*****************************************************************************
static uint32_t
am_devices_st7301_command_read(void *pHandle, uint32_t ui32Instr)
{
    am_hal_iom_transfer_t Transaction;
    uint32_t data[10];
    am_devices_iom_st7301_t *pIom = (am_devices_iom_st7301_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = ui32Instr;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32RxBuffer   = data;
    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_DISPLAY_SPI_CS;
    Transaction.bContinue       = true;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    DCX_LOW();
    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    DCX_HIGH();
    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param bHiPrio
//! @param pData
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
static uint32_t
am_devices_st7301_data_write(void *pHandle, bool bHiPrio, uint8_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    am_devices_iom_st7301_t *pIom = (am_devices_iom_st7301_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 0;
    Transaction.ui32Instr       = 0;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = (uint32_t*)pData;
    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_DISPLAY_SPI_CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    if (bHiPrio)
    {
        volatile bool bDMAComplete = false;
        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_highprio_transfer(pIom->pIomHandle, &Transaction, nb_complete, (void*)&bDMAComplete))
        {
            return AM_DEVICES_ST7301_STATUS_ERROR;
        }
        while (!bDMAComplete);
    }
    else
    {
        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
        {
            return AM_DEVICES_ST7301_STATUS_ERROR;
        }
    }

    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reads the current status of the external display
//
//*****************************************************************************
uint32_t
am_devices_st7301_reset(void *pHandle)
{
    am_hal_gpio_state_write(AM_BSP_GPIO_DISPLAY_RESET, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_util_delay_ms(20);
    am_hal_gpio_state_write(AM_BSP_GPIO_DISPLAY_RESET, AM_HAL_GPIO_OUTPUT_SET);
    am_util_delay_ms(10);

    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_st7301_display_off(void *pHandle)
{
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_DISPLAY_OFF, 0))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_st7301_display_on(void *pHandle)
{
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_DISPLAY_ON, 0))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

#define BYTE_NUM_PER_WRITE  4095
//*****************************************************************************
//
// Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_st7301_blocking_write(void *pHandle, uint8_t *pui8TxBuffer,
                                 uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    am_devices_iom_st7301_t *pIom = (am_devices_iom_st7301_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = AM_DEVICES_ST7301_MEMORY_WRITE_CONTINUE;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_DISPLAY_SPI_CS;
    Transaction.bContinue       = true;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    DCX_LOW();
    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    DCX_HIGH();
    while (ui32NumBytes)
    {
        //
        // Create the transaction.
        //
        Transaction.ui32InstrLen    = 0;
        Transaction.ui32Instr       = 0;
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.ui32NumBytes    = (ui32NumBytes > BYTE_NUM_PER_WRITE) ? BYTE_NUM_PER_WRITE : ui32NumBytes;
        Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer;
        Transaction.bContinue       = (ui32NumBytes > BYTE_NUM_PER_WRITE) ? true : false;
        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
        {
            return AM_DEVICES_ST7301_STATUS_ERROR;
        }
        ui32NumBytes -= Transaction.ui32NumBytes;
        pui8TxBuffer += Transaction.ui32NumBytes;
    }
    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_st7301_nonblocking_write(void *pHandle, uint8_t *pui8TxBuffer,
                                    uint32_t ui32NumBytes,
                                    bool bContinue,
                                    am_hal_iom_callback_t pfnCallback,
                                    void *pCallbackCtxt)
{
    return (am_devices_st7301_nonblocking_write_adv(pHandle, pui8TxBuffer,
                                                    ui32NumBytes,
                                                    bContinue, 0, 0,
                                                    0, 0, pfnCallback, pCallbackCtxt));
}

//*****************************************************************************
//
// Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_st7301_nonblocking_write_adv(void *pHandle, uint8_t *pui8TxBuffer,
                                        uint32_t ui32NumBytes,
                                        bool bContinue,
                                        uint32_t ui32Instr,
                                        uint32_t ui32InstrLen,
                                        uint32_t ui32PauseCondition,
                                        uint32_t ui32StatusSetClr,
                                        am_hal_iom_callback_t pfnCallback,
                                        void *pCallbackCtxt)
{
    am_hal_iom_transfer_t Transaction;
    am_devices_iom_st7301_t *pIom = (am_devices_iom_st7301_t *)pHandle;

    //
    // Common transaction parameters
    //
    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_DISPLAY_SPI_CS;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = ui32PauseCondition;

    if (ui32NumBytes)
    {
        //
        // Create the transaction.
        //
        Transaction.ui32InstrLen    = ui32InstrLen;
        Transaction.ui32Instr       = ui32Instr;
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.ui32NumBytes    = ui32NumBytes;
        Transaction.pui32TxBuffer   = (uint32_t*)pui8TxBuffer;
        Transaction.bContinue       = bContinue,
        Transaction.ui32StatusSetClr = ui32StatusSetClr;

        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, pfnCallback, pCallbackCtxt))
        {
            return AM_DEVICES_ST7301_STATUS_ERROR;
        }
    }

    //
    // Return the status.
    //
    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}
static uint8_t g_ui8TempBuffer[4095] = {0};

//*****************************************************************************
//
// Reads the contents of the fram into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_st7301_blocking_read(void *pHandle, uint8_t *pui8RxBuffer,
                                uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    uint8_t ui8Temp = 0;
    am_devices_iom_st7301_t *pIom = (am_devices_iom_st7301_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = AM_DEVICES_ST7301_MEMORY_READ_CONTINUE;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8RxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_DISPLAY_SPI_CS;
    Transaction.bContinue       = true;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    DCX_LOW();
    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    DCX_HIGH();

    Transaction.ui32NumBytes      = ui32NumBytes + 1;
    Transaction.pui32RxBuffer     = (uint32_t *)g_ui8TempBuffer;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen      = 0;
    Transaction.ui32Instr         = 0;
    Transaction.eDirection        = AM_HAL_IOM_RX;
    Transaction.bContinue         = false;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    for (uint32_t i = 0; i < ui32NumBytes; i++)
    {
        ui8Temp = (g_ui8TempBuffer[i] << 1) | ((g_ui8TempBuffer[i + 1] && 0x80) >> 7);
        g_ui8TempBuffer[i] = ui8Temp;
    }
    memcpy(pui8RxBuffer, g_ui8TempBuffer, ui32NumBytes);

    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reads the contents of the fram into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_st7301_nonblocking_read(void *pHandle, uint8_t *pui8RxBuffer,
                                   uint32_t ui32NumBytes,
                                   am_hal_iom_callback_t pfnCallback,
                                   void *pCallbackCtxt)
{
    am_hal_iom_transfer_t Transaction;
    uint8_t ui8Temp = 0;
    am_devices_iom_st7301_t *pIom = (am_devices_iom_st7301_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = AM_DEVICES_ST7301_MEMORY_READ_CONTINUE;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8RxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_DISPLAY_SPI_CS;
    Transaction.bContinue       = true;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    DCX_LOW();
    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, 0, 0))
    {
      return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    DCX_HIGH();

    Transaction.ui32NumBytes      = ui32NumBytes + 1;
    Transaction.pui32RxBuffer     = (uint32_t *)g_ui8TempBuffer;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen      = 0;
    Transaction.ui32Instr         = 0;
    Transaction.eDirection        = AM_HAL_IOM_RX;
    Transaction.bContinue         = false;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_nonblocking_transfer(pIom->pIomHandle, &Transaction, pfnCallback, pCallbackCtxt))
    {
      return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    for (uint32_t i = 0; i < ui32NumBytes; i++)
    {
      ui8Temp = (g_ui8TempBuffer[i] << 1) | ((g_ui8TempBuffer[i + 1] && 0x80) >> 7);
      g_ui8TempBuffer[i] = ui8Temp;
    }
    memcpy(pui8RxBuffer, g_ui8TempBuffer, ui32NumBytes);

    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

//*****************************************************************************
//
//!
//
//*****************************************************************************
static uint32_t
am_devices_lcm_init(void *pHandle)
{
    uint8_t data[10] = {0};

    data[0] = 0;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_4SPI_INPUT_DATA_SEL, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x02;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_MTP_LOAD_CTRL, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x68;
    if (am_devices_st7301_command_write(pHandle, false, 0xD7, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x01;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_BOOSTER_ENABLE, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x55;
    data[1] = 0x06;
    if (am_devices_st7301_command_write(pHandle, false, 0xE8, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 2))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0xe0;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_GATE_VOL_CTRL, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x01;
    data[1] = 0x02;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_FRAME_RATE_CTRL, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 2))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0xe5;
    data[1] = 0x77;
    data[2] = 0xf1;
    data[3] = 0xff;
    data[4] = 0xff;
    data[5] = 0x4f;
    data[6] = 0xf1;
    data[7] = 0xff;
    data[8] = 0xff;
    data[9] = 0x4f;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_UPDATE_PERIOD_GATE_EQ_CTRL, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 10))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0;
    if (am_devices_st7301_command_write(pHandle, false, 0xB7, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x78;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_DUTY_SETTING, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_SLEEP_OUT, 0))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    am_util_delay_ms(100);

    data[0] = 0x01;
    if (am_devices_st7301_command_write(pHandle, false, 0xBD, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

#if 0 // 48 Hz
    data[0] = 0x80;
    data[1] = 0xe9;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_OSC_ENABLE, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 2))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
#else // 30 Hz
    data[0] = 0xA6;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_OSC_ENABLE, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

#endif

    data[0] = 0x01;
    if (am_devices_st7301_command_write(pHandle, false, 0xD6, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x15;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_VCOMH_VOL_CTRL, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x48;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_MEM_DATA_ACC_CTRL, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x10;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_DATA_FORMAT_SEL, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0;
    data[1] = 0x04;
    data[2] = 0x80;
    data[3] = 0xFA;
    if (am_devices_st7301_command_write(pHandle, false, 0x72, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 4))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x02;
    data[1] = 0;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_DESTRESS_PERIOD_GATE_EQ_CTRL, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 2))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x23;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_SOURCE_SETTING, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x08;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_PANEL_SETTING, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0;
    data[1] = 0x3b;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_COLUMN_ADDR_SETTING, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 2))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0;
    data[1] = 0xef;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_ROW_ADDR_SETTING, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 2))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0;
    data[1] = 0xF0;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_SET_TEAR_SCANLINE, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 2))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0;
    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_TEARING_EFFECT_LINE_ON, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x1f;
    if (am_devices_st7301_command_write(pHandle, false, 0xD0, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_HIGH_POWER_MODE_ON, 0))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    //if (am_devices_st7301_command_write(pHandle, false, AM_DEVICES_ST7301_DISPLAY_ON, 0))
    //{
    //    return AM_DEVICES_ST7301_STATUS_ERROR;
    //}

    data[0] = 0x01;
    if (am_devices_st7301_command_write(pHandle, false, 0xD8, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0xb1;
    if (am_devices_st7301_command_write(pHandle, false, 0xC4, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if ( am_devices_st7301_data_write(pHandle, false, data, 1) )
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x03;
    if (am_devices_st7301_command_write(pHandle, false, 0xD8, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    data[0] = 0x02;
    if (am_devices_st7301_command_write(pHandle, false, 0xE3, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, false, data, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Initialize the st7301 driver.
//
//*****************************************************************************
uint32_t
am_devices_st7301_init(uint32_t ui32Module,
                       am_devices_st7301_config_t *pDevConfig,
                       void **ppHandle,
                       void **ppIomHandle)
{
    uint32_t      ui32Status;
    void *pIomHandle;
    am_hal_iom_config_t     stIOMST7301Settings;

#ifdef USE_HW_DCX
    am_hal_iom_dcx_cfg_t dcxCfg = {AM_BSP_DISPLAY_SPI_CS, AM_BSP_DISPLAY_DCX_CE};

    if (!APOLLO3_GE_B0)
    {
        // Can not use HW DCX
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
#endif

    uint32_t      ui32Index = 0;

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < AM_DEVICES_ST7301_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( gAmSt7301[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == AM_DEVICES_ST7301_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    if ((ui32Module > AM_REG_IOM_NUM_MODULES) || (pDevConfig == NULL) )
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    //
    // Configure the IOM pins.
    //
    am_bsp_iom_display_pins_enable(AM_BSP_4_WIRES_SPI_MODE);

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

    stIOMST7301Settings = g_sIomST7301Cfg;
    stIOMST7301Settings.ui32NBTxnBufLength = pDevConfig->ui32NBTxnBufLength;
    stIOMST7301Settings.pNBTxnBuf = pDevConfig->pNBTxnBuf;
    stIOMST7301Settings.ui32ClockFreq = pDevConfig->ui32ClockFreq;

    //
    // Initialize the IOM instance.
    // Enable power to the IOM instance.
    // Configure the IOM for Serial operation during initialization.
    // Enable the IOM.
    // HAL Success return is 0
    //
    if (am_hal_iom_initialize(ui32Module, &pIomHandle) ||
        am_hal_iom_power_ctrl(pIomHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_iom_configure(pIomHandle, &stIOMST7301Settings) ||
#ifdef USE_HW_DCX
        am_hal_iom_control(pIomHandle, AM_HAL_IOM_REQ_SET_DCX, &dcxCfg) ||
#endif
        am_hal_iom_enable(pIomHandle))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    else
    {
        gAmSt7301[ui32Index].ui32Module = ui32Module;
        *ppIomHandle = gAmSt7301[ui32Index].pIomHandle = pIomHandle;
        *ppHandle = (void *)&gAmSt7301[ui32Index];
        //
        // Device specific TFT display initialization.
        //
        am_util_delay_ms(500);
        am_devices_st7301_reset(pIomHandle);
        ui32Status = am_devices_lcm_init(pIomHandle);
        if (AM_DEVICES_ST7301_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_ST7301_STATUS_ERROR;
        }
        gAmSt7301[ui32Index].bOccupied = true;
        am_hal_gpio_state_write(AM_BSP_GPIO_DISPLAY_BL, AM_HAL_GPIO_OUTPUT_SET);
        //
        // Return the status.
        //
        return AM_DEVICES_ST7301_STATUS_SUCCESS;
    }
}

//*****************************************************************************
//
// De-Initialize the st7301 driver.
//
//*****************************************************************************
uint32_t
am_devices_st7301_term(void *pHandle)
{
    am_devices_iom_st7301_t *pIom = (am_devices_iom_st7301_t *)pHandle;

    if ( pIom->ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_interrupt_clear(  AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_DISPLAY_TE));
    am_hal_gpio_interrupt_disable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_DISPLAY_TE));
    NVIC_DisableIRQ(GPIO_IRQn);

    // Disable the pins
    am_bsp_iom_display_pins_disable(AM_BSP_4_WIRES_SPI_MODE);

    //
    // Disable the IOM.
    //
    am_hal_iom_disable(pIom->pIomHandle);

    //
    // Disable power to and uninitialize the IOM instance.
    //
    am_hal_iom_power_ctrl(pIom->pIomHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);

    am_hal_iom_uninitialize(pIom->pIomHandle);

    // Free this device handle
    pIom->bOccupied = false;

    //
    // Return the status.
    //
    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}
//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_st7301_read_id(void *pHandle, uint32_t *pdata)
{
    am_hal_iom_transfer_t  Transaction;
    am_devices_iom_st7301_t *pIom = (am_devices_iom_st7301_t *)pHandle;

    if (am_devices_st7301_command_read(pHandle, AM_DEVICES_ST7301_READ_ID))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    Transaction.ui32NumBytes      = 3;
    Transaction.pui32RxBuffer     = pdata;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen      = 0;
    Transaction.ui32Instr         = 0;
    Transaction.eDirection        = AM_HAL_IOM_RX;
    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_DISPLAY_SPI_CS;
    Transaction.bContinue         = false;
    Transaction.ui8RepeatCount    = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_st7301_set_transfer_window(void *pHandle,
                                      bool bHiPrio,
                                      uint32_t startRow,
                                      uint32_t startCol,
                                      uint32_t endRow,
                                      uint32_t endCol)
{
    uint8_t data[2];

    data[0] = startRow;
    data[1] = endRow;
    if (am_devices_st7301_command_write(pHandle, bHiPrio, AM_DEVICES_ST7301_ROW_ADDR_SETTING, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, bHiPrio, data, 2))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    data[0] = startCol >> 2;
    data[1] = endCol >> 2;
    if (am_devices_st7301_command_write(pHandle, bHiPrio, AM_DEVICES_ST7301_COLUMN_ADDR_SETTING, 1))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }
    if (am_devices_st7301_data_write(pHandle, bHiPrio, data, 2))
    {
        return AM_DEVICES_ST7301_STATUS_ERROR;
    }

    return AM_DEVICES_ST7301_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

