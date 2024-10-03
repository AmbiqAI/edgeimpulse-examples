//*****************************************************************************
//
//! @file mspi_mspi_display.c
//!
//! @brief Example demonstrating the scrolling display of two images.
//!
//! Purpose: This example demonstrates transferring a large buffer from a FLASH device
//! connected on MSPI, to a Display device connected to another MSPI.
//!
//! At initialization, both the Display and FLASH are initialized and a set image
//! data is written to the FLASH for transfer to Display.
//!
//!
//! @endverbatim
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "mspi_mspi_display.h"

#define FLASH_DATA_INIT

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
extern const uint8_t g_ui8AcambiqLogo[];
extern const uint8_t g_ui8WatchBackground[];
uint8_t                    *img0 = (uint8_t *)g_ui8AcambiqLogo;
uint8_t                    *img1 = (uint8_t *)g_ui8WatchBackground;

volatile bool   g_bNewDisplay = false;
volatile bool   g_bNewFB = false;

void            *g_FlashHandle;
void            *g_MSPIFlashHandle;

uint8_t g_FrameBuffer[FRAME_SIZE];

//*****************************************************************************
//
// FLASH MSPI Configuration
//
//*****************************************************************************
uint32_t        DMATCBBuffer[2560];
const am_devices_mspi_atxp128_config_t MSPI_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0,
    .eClockFreq = AM_HAL_MSPI_CLK_48MHZ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer) / sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
#if defined(AM_PART_APOLLO3P)
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define flash_mspi_isr                                                          \
    am_mspi_isr1(FLASH_MSPI_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr


//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void flash_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPIFlashHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPIFlashHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPIFlashHandle, ui32Status);
}

// Initialization
uint32_t
mspi_flash_init(void)
{
    uint32_t ui32Status;

    //
    // Configure the MSPI and Flash Device.
    //
    ui32Status = am_devices_mspi_atxp128_init(FLASH_MSPI_MODULE, (void *)&MSPI_Flash_Config, &g_FlashHandle, &g_MSPIFlashHandle);
    if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
    }

    NVIC_EnableIRQ(mspi_interrupts[FLASH_MSPI_MODULE]);
    am_hal_interrupt_master_enable();

#ifdef  FLASH_XIP_ENABLE
    //
    // Set up for XIP operation.
    //
    am_util_stdio_printf("Putting the MSPI and External Flash into XIP mode\n");
    ui32Status = am_devices_mspi_atxp128_enable_xip(g_FlashHandle);
    if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to put the MSPI into XIP mode!\n");
    }
#endif

    return ui32Status;
}

uint32_t
mspi_flash_data_init(void)
{
    uint32_t      ui32Status;
    uint32_t address;

    DEBUG_PRINT("Writing a known pattern to psram!\n");
    //
    // Erase the target sector.
    //
    for (address = 0; address < FRAME_SIZE; address += AM_DEVICES_MSPI_ATXP128_SECTOR_SIZE)
    {
        ui32Status = am_devices_mspi_atxp128_sector_erase(g_FlashHandle, address +  FLASH_IMAGE1_OFFSET);
        if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to erase Flash Device sector!\n");
            return -1;
        }
    }
    //
    // Write the image0 data to flash.
    //
    ui32Status = am_devices_mspi_atxp128_write(g_FlashHandle, img0, FLASH_IMAGE1_OFFSET, FRAME_SIZE, true);
    if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
    {
        DEBUG_PRINT("Failed to write buffer to FALSH Device!\n");
        return -1;
    }
#ifdef VERIFY_SRCBUF
    uint8_t rxBuf[TEMP_BUFFER_SIZE];
    for (uint32_t address = 0; address < FRAME_SIZE; address += TEMP_BUFFER_SIZE)
    {
        //
        // Read the data back into the RX buffer.
        //
        ui32Status = am_devices_mspi_atxp128_read(g_FlashHandle, rxBuf, (uint32_t)(address + FLASH_IMAGE1_OFFSET), TEMP_BUFFER_SIZE, true);
        if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to read buffer to FALSH Device!\n");
            return -1;
        }

        uint32_t size = ((address + TEMP_BUFFER_SIZE) > FRAME_SIZE) ? (FRAME_SIZE - address) : TEMP_BUFFER_SIZE;
        //
        // Compare the buffers
        //
        for (uint32_t i = 0; i < size; i++)
        {
            if (rxBuf[i] != img0[i + address])
            {
                DEBUG_PRINT("TX and RX buffers failed to compare!\n");
                return -1;
            }
        }
    }
#endif
    //
    // Erase the target sector.
    //
    for (address = 0; address < FRAME_SIZE; address += AM_DEVICES_MSPI_ATXP128_SECTOR_SIZE)
    {
        ui32Status = am_devices_mspi_atxp128_sector_erase(g_FlashHandle, address +  FLASH_IMAGE2_OFFSET);
        if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to erase Flash Device sector!\n");
            return -1;
        }
    }
    //
    // Write the image2 data to flash.
    //
    ui32Status = am_devices_mspi_atxp128_write(g_FlashHandle, img1, FLASH_IMAGE2_OFFSET, FRAME_SIZE, true);
    if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
    {
        DEBUG_PRINT("Failed to write buffer to FALSH Device!\n");
        return -1;
    }

#ifdef VERIFY_SRCBUF
    uint8_t rxBuf[TEMP_BUFFER_SIZE];
    for (uint32_t address = 0; address < FRAME_SIZE; address += TEMP_BUFFER_SIZE)
    {
        //
        // Read the data back into the RX buffer.
        //
        ui32Status = am_devices_mspi_atxp128_read(g_FlashHandle, rxBuf, (uint32_t)(address + FLASH_IMAGE2_OFFSET), TEMP_BUFFER_SIZE, true);
        if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to read buffer to FALSH Device!\n");
            return -1;
        }

        uint32_t size = ((address + TEMP_BUFFER_SIZE) > FRAME_SIZE) ? (FRAME_SIZE - address) : TEMP_BUFFER_SIZE;
        //
        // Compare the buffers
        //
        for (uint32_t i = 0; i < size; i++)
        {
            if (rxBuf[i] != img0[i + address])
            {
                DEBUG_PRINT("TX and RX buffers failed to compare!\n");
                return -1;
            }
        }
    }
#endif

    return 0;
}
// Callbacks
static void
mspi_flash_read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    DEBUG_GPIO_HIGH(DBG2_GPIO);
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        DEBUG_PRINT("\nMSPI FLASH Read Failed 0x%x\n", transactionStatus);
    }
    else
    {
        DEBUG_PRINT("\nMSPI FLASH Read Done 0x%x\n", transactionStatus);
        g_bNewDisplay = true;
    }
    DEBUG_GPIO_LOW(DBG2_GPIO);
}

void
compose_image(uint32_t pSrcAddr0, uint32_t pSrcAddr1, uint8_t *pDestBuf, uint32_t offset)
{
    uint32_t      ui32Status;
    am_hal_mspi_callback_t  mspiSourceCb = 0;
    mspiSourceCb = mspi_flash_read_complete;
    uint32_t column_index = 0;
    uint32_t buffer_index = 0;
    uint32_t flash_offset;

    if (offset == 0)
    {
        ui32Status = am_devices_mspi_atxp128_read_cb(g_FlashHandle,
                     pDestBuf,
                     pSrcAddr0,
                     FRAME_SIZE,
                     mspiSourceCb,
                     0);
        if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to read buffer to FALSH Device!\n");
        }
    }
    else if (offset >= COLUMN_NUM)
    {
        ui32Status = am_devices_mspi_atxp128_read_cb(g_FlashHandle,
                     pDestBuf,
                     pSrcAddr1,
                     FRAME_SIZE,
                     mspiSourceCb,
                     0);
        if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to read buffer to FALSH Device!\n");
        }
    }
    else
    {
        for (column_index = 0; column_index < COLUMN_NUM; column_index++)
        {

             buffer_index = column_index * ROW_NUM * PIXEL_SIZE;

            flash_offset = buffer_index + offset * PIXEL_SIZE;
            ui32Status = am_devices_mspi_atxp128_read(g_FlashHandle,
                         pDestBuf + buffer_index,
                         pSrcAddr0 + flash_offset,
                         (ROW_NUM - offset) * PIXEL_SIZE,
                         true);
            if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
            {
                DEBUG_PRINT("Failed to read buffer to FALSH Device!\n");
            }

            flash_offset = buffer_index ;
            buffer_index += (ROW_NUM - offset) * PIXEL_SIZE;
            if (column_index == COLUMN_NUM - 1)
            {
                ui32Status = am_devices_mspi_atxp128_read_cb(g_FlashHandle,
                             pDestBuf + buffer_index,
                             pSrcAddr1 + flash_offset,
                             offset * PIXEL_SIZE, mspiSourceCb,
                             0);
                if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
                {
                    DEBUG_PRINT("Failed to read buffer to FALSH Device!\n");
                }
            }
            else
            {
                ui32Status = am_devices_mspi_atxp128_read(g_FlashHandle,
                             pDestBuf + buffer_index,
                             pSrcAddr1 + flash_offset,
                             offset * PIXEL_SIZE,
                             true);
                if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
                {
                    DEBUG_PRINT("Failed to read buffer to FALSH Device!\n");
                }
            }
        }
    }
}

int
main(void)
{
    int iRet;
    int32_t newOffset = 0;
    int32_t oldOffset = COLUMN_NUM;
    uint32_t ui32Step;
    bool bSign = 0;
    //
    // Set the clock frequency
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

#ifndef NOFPU
    //
    // Enable the floating point module, and configure the core for lazy
    // stacking.
    //
    am_hal_sysctrl_fpu_enable();
    am_hal_sysctrl_fpu_stacking_enable(true);
#else
    am_hal_sysctrl_fpu_disable();
#endif

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#endif

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_itm_printf_enable();
    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo3 evb cygnus disply demo\n");

    iRet = mspi_flash_init();
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize Display\n");
        while (1);
    }
#ifdef FLASH_DATA_INIT
    iRet = mspi_flash_data_init();
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize Display\n");
        while (1);
    }
#endif
    //
    // Run display
    //
    iRet = display_init();
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize Display\n");
        while (1);
    }
    am_hal_gpio_pinconfig(DBG1_GPIO, g_AM_HAL_GPIO_OUTPUT_12);
    DEBUG_GPIO_LOW(DBG1_GPIO);
    am_hal_gpio_pinconfig(DBG2_GPIO, g_AM_HAL_GPIO_OUTPUT_12);
    DEBUG_GPIO_LOW(DBG2_GPIO);

    // Render first FB
    init_mspi_mspi_xfer();
    g_bDisplayDone = true;
    g_bNewDisplay = false;
    ui32Step = 10;
    while (1)
    {
        if (oldOffset != newOffset)
        {
            //DEBUG_GPIO_TOGGLE(DBG1_GPIO);
            DEBUG_GPIO_HIGH(DBG2_GPIO);
            compose_image(FLASH_IMAGE1_OFFSET, FLASH_IMAGE2_OFFSET, g_FrameBuffer, newOffset);
            oldOffset = newOffset;
        }

        if (g_bTEInt)
        {
            DEBUG_GPIO_LOW(DBG2_GPIO);
            g_bNewDisplay = false;
            g_bDisplayDone = false;
            g_bTEInt = false;
            start_mspi_xfer((uint32_t)g_FrameBuffer, FRAME_SIZE);
            newOffset = bSign ? (newOffset - ui32Step) : (newOffset + ui32Step);
        }

        if (newOffset >= COLUMN_NUM)
        {
            //! minus
            bSign  = 1;
        }

        if (newOffset <= 0)
        {
            //! plus
            bSign = 0;
        }
    }
}
