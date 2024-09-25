//*****************************************************************************
//
//! @file mspi_octal_example.c
//!
//! @brief Example of the MSPI operation with Octal SPI Flash.
//! @{
//! Purpose: This example configures an MSPI connected flash device in Octal DDR
//! mode and performs various operations - verifying the correctness of the same
//! Operations include - Erase, Write, Read, and XIP
//!
//! Starting from apollo5b, the MSPI XIP read bandwidth is boosted by the
//! ARM CM55 cache and the MSPI CPU read combine feature. By default,
//! the CPU read queue is on(CPURQEN). Cache prefetch(RID3) and cache miss(RID2)
//! requests deemed appropriate by MSPI hardware are combined and processed
//! with a 2:1 ratio(GQARB) between general queue and CPU read queue.
//!
//! am_hal_mspi_cpu_read_burst_t default =
//! {
//!     .eGQArbBais                         = AM_HAL_MSPI_GQARB_2_1_BAIS,
//!     .bCombineRID2CacheMissAccess        = true,
//!     .bCombineRID3CachePrefetchAccess    = true,
//!     .bCombineRID4ICacheAccess           = false,
//!     .bCPUReadQueueEnable                = true,
//! }
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//!
//! AM_DEBUG_PRINTF
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//!
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"

#define AM_BSP_MSPI_FLASH_DEVICE_IS25WX064
//#define AM_BSP_MSPI_FLASH_DEVICE_ATXP032

#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    #include "am_devices_mspi_is25wx064.h"
#else
    #include "am_devices_mspi_atxp032.h"
#endif

#include "am_util.h"
#include "mspi_octal_example.h"

#define MSPI_TARGET_SECTOR      (0)
#define MSPI_BUFFER_SIZE        (4*1024)  // 16K example buffer size.

#define DEFAULT_TIMEOUT         10000

#define MSPI_TEST_MODULE        1

#if (MSPI_TEST_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS MSPI0_APERTURE_START_ADDR
#elif (MSPI_TEST_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS MSPI1_APERTURE_START_ADDR
#elif (MSPI_TEST_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS MSPI2_APERTURE_START_ADDR
#elif (MSPI_TEST_MODULE == 3)
#define MSPI_XIP_BASE_ADDRESS MSPI3_APERTURE_START_ADDR
#endif

AM_SHARED_RW uint32_t        DMATCBBuffer[256];
AM_SHARED_RW uint8_t         g_SectorTXBuffer[MSPI_BUFFER_SIZE] __attribute__((aligned(32)));   //algined 32 byte to match a cache line
AM_SHARED_RW uint8_t         g_SectorRXBuffer[MSPI_BUFFER_SIZE] __attribute__((aligned(32)));   //algined 32 byte to match a cache line

#define MSPI_TIMING_SCAN

#define MSPI_TEST_FREQ          AM_HAL_MSPI_CLK_96MHZ

void            *g_FlashHdl;
void            *g_MSPIHdl;
const am_hal_mspi_clock_e eClockFreq = MSPI_TEST_FREQ ;

const am_abstract_mspi_devices_config_t MSPI_Flash_Config =
{
#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8,
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
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
    MSPI3_IRQn,
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
    .mspi_write = am_devices_mspi_atxp032_write,
    .mspi_mass_erase = am_devices_mspi_atxp032_mass_erase,
    .mspi_sector_erase = am_devices_mspi_atxp032_sector_erase,
    .mspi_xip_enable = am_devices_mspi_atxp032_enable_xip,
    .mspi_xip_disable = am_devices_mspi_atxp032_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_atxp032_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_atxp032_disable_scrambling,
#endif
};
am_hal_mpu_region_config_t sMPUCfg =
{
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)DMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)DMATCBBuffer + sizeof(DMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
};
am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .sInnerAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .eDeviceAttr = 0,
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
    am_hal_cachectrl_range_t sRange;

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
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);
    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttr, 1);
    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();
    //
    // Set up the regions.
    //
    am_hal_mpu_region_configure(&sMPUCfg, 1);
    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();

    am_util_stdio_printf("Apollo5 MSPI Octal Example\n\n");
#if defined(MSPI_TIMING_SCAN)
    am_util_debug_printf("Starting MSPI Timing Scan: \n");
    if ( AM_ABSTRACT_MSPI_SUCCESS == mspi_device_func.mspi_init_timing_check(MSPI_TEST_MODULE, (void*)&MSPI_Flash_Config, &MSPISdrTimingConfig) )
    {
        am_util_debug_printf("Scan Result: TXNEG = %d \n", MSPISdrTimingConfig.bTxNeg);
        am_util_debug_printf("             RXNEG = %d \n", MSPISdrTimingConfig.bRxNeg);
        am_util_debug_printf("             RXCAP = %d \n", MSPISdrTimingConfig.bRxCap);
        am_util_debug_printf("             TURNAROUND = %d \n", MSPISdrTimingConfig.ui8Turnaround);
        am_util_debug_printf("             TXDQSDELAY = %d \n", MSPISdrTimingConfig.ui8TxDQSDelay);
        am_util_debug_printf("             RXDQSDELAY = %d \n", MSPISdrTimingConfig.ui8RxDQSDelay);
    }
    else
    {
        am_util_stdio_printf("Scan Result: Failed, no valid setting.  \n");
        return -1;
    }
#endif
    //
    // Configure the MSPI and Flash Device.
    //
    ui32Status = mspi_device_func.mspi_init(MSPI_TEST_MODULE, (void*)&MSPI_Flash_Config, &g_FlashHdl, &g_MSPIHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_TEST_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_TEST_MODULE]);
    am_hal_interrupt_master_enable();

#if defined(MSPI_TIMING_SCAN)
    //
    //  Set the timing from previous scan.
    //
    mspi_device_func.mspi_init_timing_apply(g_FlashHdl, &MSPISdrTimingConfig);
#endif

    //
    // Generate data into the Sector Buffer
    //
    for (uint32_t i = 0; i < MSPI_BUFFER_SIZE; i++)
    {
        g_SectorTXBuffer[i] = (i & 0xFF);
    }

    sRange.ui32StartAddr = (uint32_t)g_SectorTXBuffer;
    sRange.ui32Size = MSPI_BUFFER_SIZE;
    am_hal_cachectrl_dcache_clean(&sRange);
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
        am_hal_cachectrl_dcache_invalidate(NULL, true);
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
            am_util_stdio_printf("Flash with XiP data error\n");
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
    // Shutdown XIP operation.
    //
    am_util_stdio_printf("Disabling the MSPI and External Flash from XIP mode\n");
    ui32Status = mspi_device_func.mspi_xip_disable(g_FlashHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
    }

    am_hal_interrupt_master_disable();
    NVIC_DisableIRQ(mspi_interrupts[MSPI_TEST_MODULE]);
    //
    // Clean up the MSPI before exit.
    //
    ui32Status = mspi_device_func.mspi_term(g_FlashHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to shutdown the MSPI and Flash Device!\n");
    }

    //
    //  End banner.
    //
    am_util_stdio_printf("\nApollo5 MSPI Octal Example Complete\n");

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
    }
}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
