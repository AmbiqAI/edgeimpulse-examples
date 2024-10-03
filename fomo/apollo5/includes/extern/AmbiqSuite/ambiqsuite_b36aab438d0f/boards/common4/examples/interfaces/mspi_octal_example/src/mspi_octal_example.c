//*****************************************************************************
//
//! @file mspi_octal_example.c
//!
//! @brief Example of the MSPI operation with Octal SPI Flash.
//!
//! @addtogroup interfaces_examples Interface Examples
//
//! @defgroup mspi_octal_example MSPI SDR OCTAL Example
//! @ingroup interfaces_examples
//! @{
//!
//! Purpose: This example configures an MSPI connected flash device in Octal SDP
//! mode and performs various operations
//!
//! Initialize the test:
//! 1. Run a timing check to find the best timing for the MSPI clock and chipset
//! 2. Initialize the MSPI instance
//! 3. Apply the timing scan results
//!
//! Test SDR DMA R/W:
//! 1. Write known data to a buffer using DMA
//! 2. Read the data back into another buffer using DMA
//! 3. Compare the results from 1 and 2 immediately above
//!
//! Test SDR XIP R/W:
//! 1. Enable XIP
//! 2. Write known data to a buffer using XIP
//! 3. Read the data back into another buffer using XIP
//! 4. Compare the results from 2 and 3 immediately above
//!
//! Test Function Call after XIP writes function to PSRAM:
//! 1. Place MSPI in Scrambling Mode
//! 2. Write test function to External PSRAM
//! 3. Call function located in PSRAM
//!
//! Deinitialize the MSPI and go to sleep
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//! - #define AM_DEBUG_PRINTF
//!
//! Note that when this macro is defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.
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
#include "am_bsp.h"

#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    #include "am_devices_mspi_is25wx064.h"
    #include "am_devices_mspi_is25wx064.c"
#else
    #include "am_devices_mspi_atxp032.h"
    #include "am_devices_mspi_atxp032.c"
#endif

#include "am_util.h"
#include "mspi_octal_example.h"

#define MSPI_TARGET_SECTOR      (0)
#define MSPI_BUFFER_SIZE        (4*1024)  // 4K example buffer size.
#if defined(apollo4l_eb) || defined(apollo4l_blue_eb)
#define MSPI_TEST_MODULE        AM_BSP_MSPI_NOR_FLASH_U7_MODULE
#else
#define MSPI_TEST_MODULE        AM_BSP_MSPI_FLASH_MODULE
#endif

#if (MSPI_TEST_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS 0x14000000
#elif (MSPI_TEST_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS 0x18000000
#elif (MSPI_TEST_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS 0x1C000000
#endif

#define MSPI_TEST_FREQ          AM_HAL_MSPI_CLK_96MHZ

uint32_t        DMATCBBuffer[4096] AM_BIT_ALIGNED(128);
uint8_t         TestBuffer[2048];
uint8_t         DummyBuffer[1024];
uint8_t         g_SectorTXBuffer[MSPI_BUFFER_SIZE] AM_BIT_ALIGNED(128);
uint8_t         g_SectorRXBuffer[MSPI_BUFFER_SIZE] AM_BIT_ALIGNED(128);
void            *g_MSPIHdl;
void            *g_FlashHdl;

const am_hal_mspi_clock_e eClockFreq = MSPI_TEST_FREQ ;

const am_abstract_mspi_devices_config_t MSPI_Flash_Config =
{
#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
#else
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0,
#endif

    .eClockFreq = MSPI_TEST_FREQ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer) / sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};

//! MSPI interrupts.
static const IRQn_Type mspi_irqs[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define flash_mspi_isr                                                          \
    am_mspi_isr1(MSPI_TEST_MODULE)
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

    am_hal_mspi_interrupt_status_get(g_MSPIHdl, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPIHdl, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPIHdl, ui32Status);
}

//*****************************************************************************
//
// Static function to be executed from external flash device
//
//*****************************************************************************
#if defined(__GNUC_STDC_INLINE__)
__attribute__((naked))
static void xip_test_function(void)
{
    __asm
    (
        "   nop\n"              // Just execute NOPs and return.
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   nop\n"
        "   bx      lr\n"
    );
}

#elif defined(__ARMCC_VERSION)
__asm static void xip_test_function(void)
{
    nop                         // Just execute NOPs and return.
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    bx      lr
}

#elif defined(__IAR_SYSTEMS_ICC__)
__stackless static void xip_test_function(void)
{
    __asm("    nop");           // Just execute NOPs and return.
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    nop");
    __asm("    bx      lr");
}
#endif

#define MSPI_XIP_FUNCTION_SIZE  72
typedef void (*mspi_xip_test_function_t)(void);
am_abstract_mspi_devices_timing_config_t MSPISdrTimingConfig;

mspi_device_func_t mspi_device_func =
{
#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    .devName = "MSPI FLASH IS25WX064",
    .mspi_init = am_devices_mspi_is25wx064_init,
    .mspi_init_timing_check = am_devices_mspi_is25wx064_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_is25wx064_apply_ddr_timing,
    .mspi_term = am_devices_mspi_is25wx064_deinit,
    .mspi_read_id = am_devices_mspi_is25wx064_id,
    .mspi_read = am_devices_mspi_is25wx064_read,
    .mspi_read_adv = am_devices_mspi_is25wx064_read_adv,
// ##### INTERNAL BEGIN #####
    .mspi_read_callback = am_devices_mspi_is25wx064_read_callback,
// ##### INTERNAL END #####
    .mspi_write = am_devices_mspi_is25wx064_write,
    .mspi_mass_erase = am_devices_mspi_is25wx064_mass_erase,
    .mspi_sector_erase = am_devices_mspi_is25wx064_sector_erase,
    .mspi_xip_enable = am_devices_mspi_is25wx064_enable_xip,
    .mspi_xip_disable = am_devices_mspi_is25wx064_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_is25wx064_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_is25wx064_disable_scrambling,
#else
    .devName = "MSPI FLASH ATXP032",
    .mspi_init = am_devices_mspi_atxp032_init,
    .mspi_init_timing_check = am_devices_mspi_atxp032_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_atxp032_apply_sdr_timing,
    .mspi_term = am_devices_mspi_atxp032_deinit,
    .mspi_read_id = am_devices_mspi_atxp032_id,
    .mspi_read = am_devices_mspi_atxp032_read,
    .mspi_read_adv = am_devices_mspi_atxp032_read_adv,
// ##### INTERNAL BEGIN #####
    .mspi_read_callback = am_devices_mspi_atxp032_read_callback,
// ##### INTERNAL END #####
    .mspi_write = am_devices_mspi_atxp032_write,
    .mspi_mass_erase = am_devices_mspi_atxp032_mass_erase,
    .mspi_sector_erase = am_devices_mspi_atxp032_sector_erase,
    .mspi_xip_enable = am_devices_mspi_atxp032_enable_xip,
    .mspi_xip_disable = am_devices_mspi_atxp032_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_atxp032_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_atxp032_disable_scrambling,
#endif
};


//*****************************************************************************
//
// MSPI Example Main.
//
//*****************************************************************************
int
main(void)
{
    uint32_t      ui32Status;
    uint32_t      funcAddr = ((uint32_t)&xip_test_function) & 0xFFFFFFFE;

    am_util_delay_ms(100);

    //
    // Cast a pointer to the begining of the sector as the test function to call.
    //
    mspi_xip_test_function_t test_function = (mspi_xip_test_function_t)((MSPI_XIP_BASE_ADDRESS + (MSPI_TARGET_SECTOR << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT)) | 0x00000001);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 Octal MSPI Example\n\n");

    am_util_debug_printf("Starting MSPI Timing Scan: \n");
    if ( AM_ABSTRACT_MSPI_SUCCESS == mspi_device_func.mspi_init_timing_check(MSPI_TEST_MODULE, (void*)&MSPI_Flash_Config, &MSPISdrTimingConfig) )
    {
#if defined (AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
        am_util_debug_printf("Scan Result: RXDQSDELAY0 = %d \n", MSPISdrTimingConfig.ui32Rxdqsdelay);
        am_util_debug_printf("             TURNAROUND0 = %d \n", MSPISdrTimingConfig.ui32Turnaround);
        am_util_debug_printf("             RXNEG0      = %d \n", MSPISdrTimingConfig.ui32Rxneg);
#elif defined (AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L)
        am_util_debug_printf("Scan Result: TXNEG = %d \n", MSPISdrTimingConfig.bTxNeg);
        am_util_debug_printf("             RXNEG = %d \n", MSPISdrTimingConfig.bRxNeg);
        am_util_debug_printf("             RXCAP = %d \n", MSPISdrTimingConfig.bRxCap);
        am_util_debug_printf("             TURNAROUND = %d \n", MSPISdrTimingConfig.ui8Turnaround);
        am_util_debug_printf("             TXDQSDELAY = %d \n", MSPISdrTimingConfig.ui8TxDQSDelay);
        am_util_debug_printf("             RXDQSDELAY = %d \n", MSPISdrTimingConfig.ui8RxDQSDelay);
#endif
    }
    else
    {
        am_util_stdio_printf("Scan Result: Failed, no valid setting.  \n");
        while(1);
    }

    //
    // Configure the MSPI and Flash Device.
    //
    ui32Status = mspi_device_func.mspi_init(MSPI_TEST_MODULE, (void*)&MSPI_Flash_Config, &g_FlashHdl, &g_MSPIHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
    }
    NVIC_SetPriority(mspi_irqs[MSPI_TEST_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_irqs[MSPI_TEST_MODULE]);
    am_hal_interrupt_master_enable();

    //
    // Set the SDR timing from previous scan.
    //
    // Since the value was already stored off during the timing scan
    // we must overide the store check
    //
    mspi_device_func.mspi_init_timing_apply(g_FlashHdl, &MSPISdrTimingConfig);

    //
    // Generate data into the Sector Buffer
    //
    for (uint32_t i = 0; i < MSPI_BUFFER_SIZE; i++)
    {
        g_SectorTXBuffer[i] = (i & 0xFF);
    }

    //
    // Erase the target sector.
    //
    am_util_stdio_printf("Erasing Sector %d\n", MSPI_TARGET_SECTOR);
    ui32Status = mspi_device_func.mspi_sector_erase(g_FlashHdl, MSPI_TARGET_SECTOR << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to erase Flash Device sector!\n");
    }

    //
    // Make sure we aren't in XIP mode.
    //
    ui32Status = mspi_device_func.mspi_xip_disable(g_FlashHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
    }

    //
    // Write the TX buffer into the target sector.
    //
    am_util_stdio_printf("Writing %d Bytes to Sector %d\n", MSPI_BUFFER_SIZE, MSPI_TARGET_SECTOR);
    ui32Status = mspi_device_func.mspi_write(g_FlashHdl, g_SectorTXBuffer, MSPI_TARGET_SECTOR << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, MSPI_BUFFER_SIZE, true);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to write buffer to Flash Device!\n");
    }

#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    //
    // Read the data back into the RX buffer.
    //
    am_util_stdio_printf("Read %d Bytes from Sector %d\n", MSPI_BUFFER_SIZE, MSPI_TARGET_SECTOR);
    ui32Status = mspi_device_func.mspi_read(g_FlashHdl, g_SectorRXBuffer, MSPI_TARGET_SECTOR << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, MSPI_BUFFER_SIZE, true);
    if ( AM_ABSTRACT_MSPI_SUCCESS != ui32Status )
    {
        am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
    }
#else
    if (eClockFreq != AM_HAL_MSPI_CLK_96MHZ)
    {
        //
        // Read the data back into the RX buffer.
        //
        am_util_stdio_printf("Read %d Bytes from Sector %d\n", MSPI_BUFFER_SIZE, MSPI_TARGET_SECTOR);
        ui32Status = mspi_device_func.mspi_read(g_FlashHdl, g_SectorRXBuffer, MSPI_TARGET_SECTOR << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, MSPI_BUFFER_SIZE, true);
        if ( AM_ABSTRACT_MSPI_SUCCESS != ui32Status )
        {
            am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
        }
    }
    else
    {
        //
        // Read the data back into the RX buffer using XIP
        //
        ui32Status = mspi_device_func.mspi_xip_enable(g_FlashHdl);
        if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to put the MSPI into XIP mode!\n");
        }
        am_hal_daxi_control(AM_HAL_DAXI_CONTROL_FLUSH, NULL);
        am_hal_daxi_control(AM_HAL_DAXI_CONTROL_INVALIDATE, NULL);
        uint8_t *xipPointer = (uint8_t *) (MSPI_XIP_BASE_ADDRESS + (MSPI_TARGET_SECTOR << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT));
        memcpy((uint8_t *) g_SectorRXBuffer, xipPointer, MSPI_BUFFER_SIZE);

        //
        // Quit XIP mode
        //
        ui32Status = mspi_device_func.mspi_xip_disable(g_FlashHdl);
        if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
        }
    }
#endif

    //
    // Compare the buffers
    //
    am_util_stdio_printf("Comparing the TX and RX Buffers\n");
    for (uint32_t i = 0; i < MSPI_BUFFER_SIZE; i++)
    {
        if (g_SectorRXBuffer[i] != g_SectorTXBuffer[i])
        {
            am_util_stdio_printf("TX and RX buffers failed to compare!\n");
            break;
        }
    }

    //
    // Erase the target sector.
    //
    am_util_stdio_printf("Erasing Sector %d\n", MSPI_TARGET_SECTOR);
    ui32Status = mspi_device_func.mspi_sector_erase(g_FlashHdl, MSPI_TARGET_SECTOR << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to erase Flash Device sector!\n");
    }

    //
    // Turn on scrambling operation.
    //
    am_util_stdio_printf("Putting the MSPI into Scrambling mode\n");
    ui32Status = mspi_device_func.mspi_scrambling_enable(g_FlashHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable MSPI scrambling!\n");
    }

    //
    // Write the executable function into the target sector.
    //
    am_util_stdio_printf("Writing Executable function of %d Bytes to Sector %d\n", MSPI_XIP_FUNCTION_SIZE, MSPI_TARGET_SECTOR);
    ui32Status = mspi_device_func.mspi_write(g_FlashHdl, (uint8_t *)funcAddr, MSPI_TARGET_SECTOR << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, MSPI_XIP_FUNCTION_SIZE, true);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to write executable function to Flash Device!\n");
    }

    ui32Status = mspi_device_func.mspi_read(g_FlashHdl, g_SectorRXBuffer, MSPI_TARGET_SECTOR << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, MSPI_XIP_FUNCTION_SIZE, true);
    if ( AM_ABSTRACT_MSPI_SUCCESS != ui32Status )
    {
       am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
    }
    else
    {
        if (memcmp((uint8_t *)funcAddr, g_SectorRXBuffer, MSPI_XIP_FUNCTION_SIZE) != 0)
        {
            am_util_stdio_printf("Flash with XIP data error\n");
        }
    }

    //
    // Set up for XIP operation.
    //
    am_util_stdio_printf("Putting the MSPI and External Flash into XIP mode\n");
    ui32Status = mspi_device_func.mspi_xip_enable(g_FlashHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to put the MSPI into XIP mode!\n");
    }

    // #### INTERNAL BEGIN ####
    // We need to enable scrambling again here if it is enabled previous....
    // The AM_HAL_MSPI_REQ_XIP_CONFIG request to am_hal_mspi_control() will manipulate
    // DEV0SCRAMBLING register, thus may turn scrambling off....
    // Why does AM_HAL_MSPI_REQ_XIP_CONFIG have to change scrambling setting?
    // #### INTERNAL END ####
    ui32Status = mspi_device_func.mspi_scrambling_enable(g_FlashHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable MSPI scrambling!\n");
    }

    //
    // Execute a call to the test function in the sector.
    //
    am_util_stdio_printf("Jumping to function in External Flash\n");
    test_function();
    am_util_stdio_printf("Returned from XIP call\n");

    //
    // Disable XIP operation.
    //
    am_util_stdio_printf("Disabling the MSPI and External Flash from XIP mode\n");
    ui32Status = mspi_device_func.mspi_xip_disable(g_FlashHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
    }

    am_hal_interrupt_master_disable();
    NVIC_DisableIRQ(mspi_irqs[MSPI_TEST_MODULE]);

    //
    // Deinitialize the MSPI before exit.
    //
    ui32Status = mspi_device_func.mspi_term(g_FlashHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to shutdown the MSPI and Flash Device!\n");
    }

    //
    //  End banner.
    //
    am_util_stdio_printf("Apollo4 MSPI Octal Example Complete\n");

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
