//*****************************************************************************
//
//! @file am_devices_rm67162.c
//!
//! @brief Generic Raydium OLED display driver.
//!
//! @addtogroup rm67162 RM67162 Display Driver
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
#include "am_devices_rm67162.h"
#include "am_bsp.h"
#include "am_util_delay.h"
#include "am_util.h"

#ifdef USE_HW_DCX
#define DCX_HIGH()
#define DCX_LOW()
#else
#define DCX_HIGH()  am_hal_gpio_state_write(AM_BSP_GPIO_IOM0_DCX, AM_HAL_GPIO_OUTPUT_SET)
#define DCX_LOW()   am_hal_gpio_state_write(AM_BSP_GPIO_IOM0_DCX, AM_HAL_GPIO_OUTPUT_CLEAR)
#endif

//*****************************************************************************
//
//! @hame Global variables.
//! @{
//
//*****************************************************************************
static void *g_pRM67162IOMHandle;

static volatile bool bNbDone = false;

//! @}

static struct
{
    uint32_t row_start;
    uint32_t row_end;
    uint32_t col_start;
    uint32_t col_end;
}gs_display_info;

//*****************************************************************************
//
//! @brief
//! @param pCallbackCtxt
//! @param transactionStatus
//
//*****************************************************************************
static void
nb_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    bNbDone = true;
}

//*****************************************************************************
//
// Generic Command Write function.
//
//*****************************************************************************
uint32_t
am_devices_rm67162_command_write(bool bHiPrio, uint32_t ui32Instr, bool bdata)
{
    am_hal_iom_transfer_t Transaction;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = ui32Instr;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = NULL;
    Transaction.uPeerInfo.ui32SpiChipSelect = 1;
    Transaction.bContinue       = (bdata ? true :false);
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    DCX_LOW();
    if (bHiPrio)
    {
        bNbDone = false;
        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_highprio_transfer(g_pRM67162IOMHandle, &Transaction, nb_complete, 0))
        {
            return AM_DEVICES_RM67162_STATUS_ERROR;
        }
        while (!bNbDone);
    }
    else
    {
        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_blocking_transfer(g_pRM67162IOMHandle, &Transaction))
        {
            return AM_DEVICES_RM67162_STATUS_ERROR;
        }
    }

    DCX_HIGH();
    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//! @param ui32Instr
//! @return
//
//*****************************************************************************
static uint32_t
am_devices_rm67162_command_read(uint32_t ui32Instr)
{
    am_hal_iom_transfer_t Transaction;
    uint32_t data[10];

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = ui32Instr;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32RxBuffer   = data;
    Transaction.uPeerInfo.ui32SpiChipSelect = 1;
    Transaction.bContinue       = true;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    DCX_LOW();
    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(g_pRM67162IOMHandle, &Transaction))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }
    DCX_HIGH();
    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//! @param bHiPrio
//! @param pData
//! @param ui32NumBytes
//! @return
//
//*****************************************************************************
static uint32_t
am_devices_rm67162_data_write(bool bHiPrio, uint8_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 0;
    Transaction.ui32Instr       = 0;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = (uint32_t*)pData;
    Transaction.uPeerInfo.ui32SpiChipSelect = 1;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    if (bHiPrio)
    {
        bNbDone = false;
        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_highprio_transfer(g_pRM67162IOMHandle, &Transaction, nb_complete, 0))
        {
            return AM_DEVICES_RM67162_STATUS_ERROR;
        }
        while (!bNbDone);
    }
    else
    {
        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_blocking_transfer(g_pRM67162IOMHandle, &Transaction))
        {
            return AM_DEVICES_RM67162_STATUS_ERROR;
        }
    }

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reads the current status of the external display
//
//*****************************************************************************
uint32_t
am_devices_rm67162_reset(void)
{
    am_hal_gpio_state_write(AM_BSP_GPIO_DISPLAY_RESET, AM_HAL_GPIO_OUTPUT_SET);
    am_util_delay_ms(20);
    am_hal_gpio_state_write(AM_BSP_GPIO_DISPLAY_RESET, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_util_delay_ms(20);
    am_hal_gpio_state_write(AM_BSP_GPIO_DISPLAY_RESET, AM_HAL_GPIO_OUTPUT_SET);
    am_util_delay_ms(20);

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_rm67162_display_off(void)
{
    // if ( am_devices_rm67162_command_write(false, AM_DEVICES_RM67162_DISPLAY_OFF, 0) )
    // {
    //     return AM_DEVICES_RM67162_STATUS_ERROR;
    // }

    if ( am_devices_rm67162_command_write(false, 0x28, 0) )
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    if ( am_devices_rm67162_command_write(false, 0x10, 0) )
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_rm67162_display_on(void)
{
    // if ( am_devices_rm67162_command_write(false, AM_DEVICES_RM67162_DISPLAY_ON, 0) )
    // {
    //     return AM_DEVICES_RM67162_STATUS_ERROR;
    // }

    if ( am_devices_rm67162_command_write(false, 0x29, 0) )
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

#define BYTE_NUM_PER_WRITE  4095
//*****************************************************************************
//
//  Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_rm67162_blocking_write(uint8_t *pui8TxBuffer,
                                  uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = 0x2c; //AM_DEVICES_RM67162_MEMORY_WRITE_CONTINUE;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8TxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = 1;
    Transaction.bContinue       = true;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    DCX_LOW();
    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(g_pRM67162IOMHandle, &Transaction))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
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
        if (am_hal_iom_blocking_transfer(g_pRM67162IOMHandle, &Transaction))
        {
            return AM_DEVICES_RM67162_STATUS_ERROR;
        }
        ui32NumBytes -= Transaction.ui32NumBytes;
        pui8TxBuffer += Transaction.ui32NumBytes;
    }
    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_rm67162_nonblocking_write(uint8_t *pui8TxBuffer,
                                     uint32_t ui32NumBytes,
                                     bool bContinue,
                                     am_hal_iom_callback_t pfnCallback,
                                     void *pCallbackCtxt)
{
    return (am_devices_rm67162_nonblocking_write_adv(pui8TxBuffer,
                                                     ui32NumBytes,
                                                     bContinue,
                                                     0, 0, pfnCallback, pCallbackCtxt));
}

//*****************************************************************************
//
// Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_rm67162_nonblocking_write_adv(uint8_t *pui8TxBuffer,
                                         uint32_t ui32NumBytes,
                                         bool bContinue,
                                         uint32_t ui32PauseCondition,
                                         uint32_t ui32StatusSetClr,
                                         am_hal_iom_callback_t pfnCallback,
                                         void *pCallbackCtxt)
{
    am_hal_iom_transfer_t Transaction;

    //
    // Common transaction parameters
    //
    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.uPeerInfo.ui32SpiChipSelect = 1;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = ui32PauseCondition;

    if (ui32NumBytes)
    {
        //
        // Create the transaction.
        //
        Transaction.ui32InstrLen    = 0;
        Transaction.ui32Instr       = 0;
        Transaction.eDirection      = AM_HAL_IOM_TX;
        Transaction.ui32NumBytes    = ui32NumBytes;
        Transaction.pui32TxBuffer   = (uint32_t*)pui8TxBuffer;
        Transaction.bContinue       = bContinue,
        Transaction.ui32StatusSetClr = ui32StatusSetClr;

        //
        // Execute the transction over IOM.
        //
        if (am_hal_iom_nonblocking_transfer(g_pRM67162IOMHandle, &Transaction, pfnCallback, pCallbackCtxt))
        {
            return AM_DEVICES_RM67162_STATUS_ERROR;
        }
    }

    //
    // Return the status.
    //
    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

static uint8_t g_ui8TempBuffer[4095] = {0};
//*****************************************************************************
//
// Reads the contents of the fram into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_rm67162_blocking_read(uint8_t *pui8RxBuffer,
                                 uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    uint8_t ui8Temp = 0;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = AM_DEVICES_RM67162_MEMORY_READ_CONTINUE;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8RxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = 1;
    Transaction.bContinue       = true;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    DCX_LOW();
    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(g_pRM67162IOMHandle, &Transaction))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
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
    if (am_hal_iom_blocking_transfer(g_pRM67162IOMHandle, &Transaction))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    for (uint32_t i = 0; i < ui32NumBytes; i++)
    {
        ui8Temp = (g_ui8TempBuffer[i] << 1) | ((g_ui8TempBuffer[i + 1] && 0x80) >> 7);
        g_ui8TempBuffer[i] = ui8Temp;
    }
    memcpy(pui8RxBuffer, g_ui8TempBuffer, ui32NumBytes);

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reads the contents of the fram into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_rm67162_nonblocking_read(uint8_t *pui8RxBuffer,
                                    uint32_t ui32NumBytes,
                                    am_hal_iom_callback_t pfnCallback,
                                    void *pCallbackCtxt)
{
    am_hal_iom_transfer_t Transaction;
    uint8_t ui8Temp = 0;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = 1;
    Transaction.ui32Instr       = AM_DEVICES_RM67162_MEMORY_READ_CONTINUE;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = 0;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8RxBuffer;
    Transaction.uPeerInfo.ui32SpiChipSelect = 1;
    Transaction.bContinue       = true;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    DCX_LOW();
    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_nonblocking_transfer(g_pRM67162IOMHandle, &Transaction, 0, 0))
    {
      return AM_DEVICES_RM67162_STATUS_ERROR;
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
    if (am_hal_iom_nonblocking_transfer(g_pRM67162IOMHandle, &Transaction, pfnCallback, pCallbackCtxt))
    {
      return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    for (uint32_t i = 0; i < ui32NumBytes; i++)
    {
      ui8Temp = (g_ui8TempBuffer[i] << 1) | ((g_ui8TempBuffer[i + 1] && 0x80) >> 7);
      g_ui8TempBuffer[i] = ui8Temp;
    }
    memcpy(pui8RxBuffer, g_ui8TempBuffer, ui32NumBytes);

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//!
//! @param cmd
//! @param data
//! @param len
//!
//! @return
//
//*****************************************************************************
static uint32_t
am_devices_rm67162_reg_write(uint8_t cmd, uint8_t *data, uint8_t len)
{
    if (am_devices_rm67162_command_write(false, cmd, 1))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    if (len > 0)
    {
        if (am_devices_rm67162_data_write(false, data, len))
        {
            return AM_DEVICES_RM67162_STATUS_ERROR;
        }
    }

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//!
//! @param psGraphic_conf
//!
//! @return
//
//*****************************************************************************
static uint32_t
am_devices_lcm_init(am_devices_rm67162_graphic_conf_t *psGraphic_conf)
{
    uint8_t data[4] = {0};

    gs_display_info.row_start = psGraphic_conf->row_offset;
    gs_display_info.row_end = psGraphic_conf->max_row + psGraphic_conf->row_offset - 1;
    gs_display_info.col_start = psGraphic_conf->col_offset;
    gs_display_info.col_end = psGraphic_conf->max_col + psGraphic_conf->col_offset - 1;

    data[0] = 0x0;
    am_devices_rm67162_reg_write(AM_DEVICES_RM67162_TEARING_EFFECT_LINE_ON, data, 1);

    data[0] = psGraphic_conf->bus_mode;
    am_devices_rm67162_reg_write(AM_DEVICES_RM67162_SET_DSPI_MODE, data, 1);

    data[0] = psGraphic_conf->color_mode ;
    am_devices_rm67162_reg_write(AM_DEVICES_RM67162_DATA_FORMAT_SEL, data, 1);

    data[0] = psGraphic_conf->scan_mode;
    am_devices_rm67162_reg_write(AM_DEVICES_RM67162_SCAN_MODE, data, 1);

    data[0] = 0x20;
    am_devices_rm67162_reg_write(AM_DEVICES_RM67162_SET_WRITE_DISPLAY_CTRL, data, 1);

    am_devices_rm67162_setting_address(0, 0);

    data[0] = 0x00;
    data[1] = 0x28;
    am_devices_rm67162_reg_write(AM_DEVICES_RM67162_SET_TEAR_SCANLINE, data, 2);

    am_devices_rm67162_reg_write(AM_DEVICES_RM67162_SLEEP_OUT, NULL, 0);

    am_util_delay_ms(130);

    am_devices_rm67162_reg_write(AM_DEVICES_RM67162_DISPLAY_ON, NULL, 0);

    am_util_delay_ms(200);

    am_hal_gpio_state_write(AM_BSP_GPIO_DISPLAY_PWRCTRL, AM_HAL_GPIO_OUTPUT_CLEAR);

    am_util_stdio_printf("AM_BSP_GPIO_DISPLAY_PWRCTRL set on \n");

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Initialize the rm67162 driver.
//
//*****************************************************************************
uint32_t
am_devices_rm67162_init(uint32_t ui32Module,
                        am_hal_iom_config_t *psIOMSettings,
                        am_devices_rm67162_graphic_conf_t *psGraphic_conf)
{
    uint32_t      ui32Status;

    if ((ui32Module > AM_REG_IOM_NUM_MODULES) || (psIOMSettings == NULL))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    // power ctrl
    am_hal_gpio_pinconfig(AM_BSP_GPIO_1V8_CTRL, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(AM_BSP_GPIO_1V8_CTRL, AM_HAL_GPIO_OUTPUT_SET);
    am_util_delay_us(15);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_2V8_CTRL, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(AM_BSP_GPIO_2V8_CTRL, AM_HAL_GPIO_OUTPUT_SET);

    am_util_delay_ms(5);

    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISPLAY_PWRCTRL, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(AM_BSP_GPIO_DISPLAY_PWRCTRL, AM_HAL_GPIO_OUTPUT_SET);
    am_util_delay_ms(5);

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

    //
    // Initialize the IOM instance.
    // Enable power to the IOM instance.
    // Configure the IOM for Serial operation during initialization.
    // Enable the IOM.
    // HAL Success return is 0
    //
    if (am_hal_iom_initialize(ui32Module, &g_pRM67162IOMHandle) ||
        am_hal_iom_power_ctrl(g_pRM67162IOMHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_iom_configure(g_pRM67162IOMHandle, psIOMSettings) ||
        am_hal_iom_enable(g_pRM67162IOMHandle))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }
    else
    {
/*         *ppIomHandle = g_pRM67162IOMHandle; */
        //
        // Device specific OLED display initialization.
        //
        am_util_delay_ms(10);
        am_devices_rm67162_reset();
        ui32Status = am_devices_lcm_init(psGraphic_conf);
        if (AM_DEVICES_RM67162_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_RM67162_STATUS_ERROR;
        }
        //
        // Return the status.
        //
        return AM_DEVICES_RM67162_STATUS_SUCCESS;
    }
}

//*****************************************************************************
//
// De-Initialize the rm67162 driver.
//
//*****************************************************************************
uint32_t
am_devices_rm67162_term(uint32_t ui32Module)
{
    if ( ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_DISPLAY_TE));
    am_hal_gpio_interrupt_disable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_DISPLAY_TE));
    NVIC_DisableIRQ(GPIO_IRQn);

    // Disable the pins
    am_bsp_iom_display_pins_disable(AM_BSP_4_WIRES_SPI_MODE);

    //
    // Disable the IOM.
    //
    am_hal_iom_disable(g_pRM67162IOMHandle);

    //
    // Disable power to and uninitialize the IOM instance.
    //
    am_hal_iom_power_ctrl(g_pRM67162IOMHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);

    am_hal_iom_uninitialize(g_pRM67162IOMHandle);

    //
    // Return the status.
    //
    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_rm67162_read_id(uint32_t *pdata)
{
    am_hal_iom_transfer_t  Transaction;

    if (am_devices_rm67162_command_read(AM_DEVICES_RM67162_READ_ID))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }
    Transaction.ui32NumBytes      = 3;
    Transaction.pui32RxBuffer     = pdata;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen      = 0;
    Transaction.ui32Instr         = 0;
    Transaction.eDirection        = AM_HAL_IOM_RX;
    Transaction.uPeerInfo.ui32SpiChipSelect = 1;
    Transaction.bContinue         = false;
    Transaction.ui8RepeatCount    = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(g_pRM67162IOMHandle, &Transaction))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_rm67162_set_transfer_window(bool bHiPrio, uint32_t startRow, uint32_t startCol, uint32_t endRow, uint32_t endCol)
{
    uint8_t data[4];

/*
    if ((endRow>=AM_DEVICES_RM67162_MAX_ROW)||(endCol>=AM_DEVICES_RM67162_MAX_COL))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }
*/

    data[0] = 0x00;
    data[1] = startRow;
    data[2] = 0x00;
    data[3] = endRow;
    if (am_devices_rm67162_command_write(bHiPrio, AM_DEVICES_RM67162_ROW_ADDR_SETTING, 1))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }
    if (am_devices_rm67162_data_write(bHiPrio, data, 4))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    data[0] = 0x00;
    data[1] = startCol;
    data[2] = 0x00;
    data[3] = endCol;
    if (am_devices_rm67162_command_write(bHiPrio, AM_DEVICES_RM67162_COLUMN_ADDR_SETTING, 1))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }
    if (am_devices_rm67162_data_write(bHiPrio, data, 4))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//*****************************************************************************
uint32_t
am_devices_rm67162_setting_address(uint32_t x, uint32_t y)
{
    uint8_t data[4] = {0};
    x += gs_display_info.col_start;
    y += gs_display_info.row_start;

/* set column start address */
    data[0] = (x / 256);
    data[1] = (x % 256);
    data[2] = (gs_display_info.col_end / 256);
    data[3] = (gs_display_info.col_end % 256);

    if (am_devices_rm67162_command_write(false, 0x2A, 1))//Column
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }
    if (am_devices_rm67162_data_write(false, data, 4))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

/* set row start address */
    data[0] = (y / 256);
    data[1] = (y % 256);
    data[2] = (gs_display_info.row_end / 256);
    data[3] = (gs_display_info.row_end % 256);
    if (am_devices_rm67162_command_write(false, 0x2B, 1))//row
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }
    if (am_devices_rm67162_data_write(false, data, 4))
    {
        return AM_DEVICES_RM67162_STATUS_ERROR;
    }

    return AM_DEVICES_RM67162_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

