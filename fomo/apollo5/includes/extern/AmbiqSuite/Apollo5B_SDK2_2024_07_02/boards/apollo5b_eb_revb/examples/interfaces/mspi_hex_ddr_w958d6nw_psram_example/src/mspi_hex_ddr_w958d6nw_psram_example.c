//*****************************************************************************
//
//! @file mspi_hex_ddr_w958d6nw_psram_example.c
//!
//! @brief Example of the MSPI operation with DDR HEX SPI PSRAM W958D6NW.
//!
//! Purpose: This example demonstrates MSPI DDR HEX operation using the W958D6NW
//! PSRAM device.
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
//
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

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_devices_mspi_psram_w958d6nw.h"
#include "am_util.h"

#define HYBRIDSLEEP_TEST
#define MSPI_PSRAM_TIMING_CHECK

#define ENABLE_XIPMM
#define MSPI_INT_TIMEOUT        (100)

#define DDR_PSRAM_TARGET_ADDRESS 0
#define MSPI_BUFFER_SIZE        (4*1024)  // 4K example buffer size.

#define DEFAULT_TIMEOUT         10000

#define AM_BSP_HEX_CE0
#if defined(AM_BSP_HEX_CE0)
#define MSPI_PSRAM_HEX_CONFIG MSPI_PSRAM_HexCE0MSPIConfig
#else
#define MSPI_PSRAM_HEX_CONFIG MSPI_PSRAM_HexCE1MSPIConfig
#endif

AM_SHARED_RW uint32_t        DMATCBBuffer[256];
AM_SHARED_RW uint8_t         g_TXBuffer[MSPI_BUFFER_SIZE] __attribute__((aligned(32)));   //algined 32 byte to match a cache line
AM_SHARED_RW uint8_t         g_RXBuffer[MSPI_BUFFER_SIZE] __attribute__((aligned(32)));   //algined 32 byte to match a cache line
void            *g_pDevHandle;
void            *g_pHandle;

am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE0MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

am_devices_mspi_psram_config_t MSPI_PSRAM_HexCE1MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE1,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

#if defined(AM_PART_APOLLO5B)
am_hal_mspi_cpu_read_burst_t sCPURQCFG =
{
    .eGQArbBais                         = AM_HAL_MSPI_GQARB_2_1_BAIS,
    .bCombineRID2CacheMissAccess        = true,
    .bCombineRID3CachePrefetchAccess    = true,
    .bCombineRID4ICacheAccess           = false,
    .bCPUReadQueueEnable                = true,
};
#endif

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

#define MSPI_TEST_MODULE              0

#if (MSPI_TEST_MODULE == 0)
#define MSPI_XIP_BASE_ADDRESS MSPI0_APERTURE_START_ADDR
#elif (MSPI_TEST_MODULE == 1)
#define MSPI_XIP_BASE_ADDRESS MSPI1_APERTURE_START_ADDR
#elif (MSPI_TEST_MODULE == 2)
#define MSPI_XIP_BASE_ADDRESS MSPI2_APERTURE_START_ADDR
#elif (MSPI_TEST_MODULE == 3)
#define MSPI_XIP_BASE_ADDRESS MSPI3_APERTURE_START_ADDR
#endif // #if (MSPI_TEST_MODULE == 0)

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
#define psram_mspi_isr                                                          \
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
void psram_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pHandle, ui32Status);
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

#ifdef ENABLE_XIPMM
//*****************************************************************************
//
// XIPMM check
//
//*****************************************************************************
#define MSPI_XIPMM_BASE_ADDRESS MSPI_XIP_BASE_ADDRESS

bool
run_mspi_xipmm(uint32_t block, bool bUseWordAccesses)
{
    uint32_t ix;

    if ( bUseWordAccesses )
    {
        // Use word accesses if scrambled.
        uint32_t *pAddr1 = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS);
        uint32_t *pAddr2 = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS  + 512);

        // Initialize a pattern
        for (ix = 0; ix < 512 / 4; ix++)
        {
            *pAddr1++ = ix;
            *pAddr2++ = ix ^ 0xFFFFFFFF;
        }
        pAddr1 = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS);
        pAddr2 = (uint32_t *)(MSPI_XIPMM_BASE_ADDRESS + 512);

        // Verify the pattern
        for (ix = 0; ix < 512 / 4; ix++)
        {
            if ( (*pAddr1++ != ix) || (*pAddr2++ != (ix ^ 0xFFFFFFFF)) )
            {
                return false;
            }
        }
    }
    else
    {
        // Use byte accesses.
        uint8_t *pAddr1 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS);
        uint8_t *pAddr2 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS  + 512);

        // Initialize a pattern
        for (ix = 0; ix < 512; ix++)
        {
            *pAddr1++ = (uint8_t)(ix & 0xFF);
            *pAddr2++ = (uint8_t)((ix & 0xFF) ^ 0xFF);
        }
        pAddr1 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS);
        pAddr2 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS + 512);

        // Verify the pattern
        for (ix = 0; ix < 512; ix++)
        {
            if ( (*pAddr1++ != (uint8_t)(ix & 0xFF)) || (*pAddr2++ != (uint8_t)((ix & 0xFF) ^ 0xFF)) )
            {
                return false;
            }
        }
    }
    return true;
}
#endif

#ifdef MSPI_PSRAM_TIMING_CHECK
am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
#endif

typedef struct
{
    am_hal_gpio_drivestrength_ext_e eD0DS;
    am_hal_gpio_drivestrength_ext_e eD1DS;
    am_hal_gpio_drivestrength_ext_e eD2DS;
    am_hal_gpio_drivestrength_ext_e eD3DS;
    am_hal_gpio_drivestrength_ext_e eD4DS;
    am_hal_gpio_drivestrength_ext_e eD5DS;
    am_hal_gpio_drivestrength_ext_e eD6DS;
    am_hal_gpio_drivestrength_ext_e eD7DS;
    am_hal_gpio_drivestrength_ext_e eD8DS;
    am_hal_gpio_drivestrength_ext_e eD9DS;
    am_hal_gpio_drivestrength_ext_e eD10DS;
    am_hal_gpio_drivestrength_ext_e eD11DS;
    am_hal_gpio_drivestrength_ext_e eD12DS;
    am_hal_gpio_drivestrength_ext_e eD13DS;
    am_hal_gpio_drivestrength_ext_e eD14DS;
    am_hal_gpio_drivestrength_ext_e eD15DS;
    am_hal_gpio_drivestrength_ext_e eSCKDS;
    am_hal_gpio_drivestrength_ext_e eDQS0DS;
    am_hal_gpio_drivestrength_ext_e eDQS1DS;
    am_hal_gpio_drivestrength_e     eCEDS;
}mspi_io_drive_strength_t;


#if defined(apollo5_eb) || defined(apollo5b_eb_revb)
void mspi_config_drive_strength(uint32_t ui32Module,
                                am_hal_mspi_device_e  eMSPIDevice,
                                mspi_io_drive_strength_t* pMspiIODS)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    g_AM_BSP_GPIO_MSPI0_D8.GP.cfg_b.eDriveStrength = pMspiIODS->eD8DS;
                    g_AM_BSP_GPIO_MSPI0_D9.GP.cfg_b.eDriveStrength = pMspiIODS->eD9DS;
                    g_AM_BSP_GPIO_MSPI0_D10.GP.cfg_b.eDriveStrength = pMspiIODS->eD10DS;
                    g_AM_BSP_GPIO_MSPI0_D11.GP.cfg_b.eDriveStrength = pMspiIODS->eD11DS;
                    g_AM_BSP_GPIO_MSPI0_D12.GP.cfg_b.eDriveStrength = pMspiIODS->eD12DS;
                    g_AM_BSP_GPIO_MSPI0_D13.GP.cfg_b.eDriveStrength = pMspiIODS->eD13DS;
                    g_AM_BSP_GPIO_MSPI0_D14.GP.cfg_b.eDriveStrength = pMspiIODS->eD14DS;
                    g_AM_BSP_GPIO_MSPI0_D15.GP.cfg_b.eDriveStrength = pMspiIODS->eD15DS;
                    g_AM_BSP_GPIO_MSPI0_DQS1DM1.GP.cfg_b.eDriveStrength = pMspiIODS->eDQS1DS;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    g_AM_BSP_GPIO_MSPI0_DQSDM.GP.cfg_b.eDriveStrength = pMspiIODS->eDQS0DS;
                    g_AM_BSP_GPIO_MSPI0_D4.GP.cfg_b.eDriveStrength = pMspiIODS->eD4DS;
                    g_AM_BSP_GPIO_MSPI0_D5.GP.cfg_b.eDriveStrength = pMspiIODS->eD5DS;
                    g_AM_BSP_GPIO_MSPI0_D6.GP.cfg_b.eDriveStrength = pMspiIODS->eD6DS;
                    g_AM_BSP_GPIO_MSPI0_D7.GP.cfg_b.eDriveStrength = pMspiIODS->eD7DS;
                    g_AM_BSP_GPIO_MSPI0_D2.GP.cfg_b.eDriveStrength = pMspiIODS->eD2DS;
                    g_AM_BSP_GPIO_MSPI0_D3.GP.cfg_b.eDriveStrength = pMspiIODS->eD3DS;
                    g_AM_BSP_GPIO_MSPI0_CE0.GP.cfg_b.eDriveStrength = pMspiIODS->eCEDS;
                    g_AM_BSP_GPIO_MSPI0_D0.GP.cfg_b.eDriveStrength = pMspiIODS->eD0DS;
                    g_AM_BSP_GPIO_MSPI0_D1.GP.cfg_b.eDriveStrength = pMspiIODS->eD1DS;
                    g_AM_BSP_GPIO_MSPI0_SCK.GP.cfg_b.eDriveStrength = pMspiIODS->eSCKDS;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    g_AM_BSP_GPIO_MSPI3_D8.GP.cfg_b.eDriveStrength = pMspiIODS->eD8DS;
                    g_AM_BSP_GPIO_MSPI3_D9.GP.cfg_b.eDriveStrength = pMspiIODS->eD9DS;
                    g_AM_BSP_GPIO_MSPI3_D10.GP.cfg_b.eDriveStrength = pMspiIODS->eD10DS;
                    g_AM_BSP_GPIO_MSPI3_D11.GP.cfg_b.eDriveStrength = pMspiIODS->eD11DS;
                    g_AM_BSP_GPIO_MSPI3_D12.GP.cfg_b.eDriveStrength = pMspiIODS->eD12DS;
                    g_AM_BSP_GPIO_MSPI3_D13.GP.cfg_b.eDriveStrength = pMspiIODS->eD13DS;
                    g_AM_BSP_GPIO_MSPI3_D14.GP.cfg_b.eDriveStrength = pMspiIODS->eD14DS;
                    g_AM_BSP_GPIO_MSPI3_D15.GP.cfg_b.eDriveStrength = pMspiIODS->eD15DS;
                    g_AM_BSP_GPIO_MSPI3_DQS1DM1.GP.cfg_b.eDriveStrength = pMspiIODS->eDQS1DS;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    g_AM_BSP_GPIO_MSPI3_DQSDM.GP.cfg_b.eDriveStrength = pMspiIODS->eDQS0DS;
                    g_AM_BSP_GPIO_MSPI3_D4.GP.cfg_b.eDriveStrength = pMspiIODS->eD4DS;
                    g_AM_BSP_GPIO_MSPI3_D5.GP.cfg_b.eDriveStrength = pMspiIODS->eD5DS;
                    g_AM_BSP_GPIO_MSPI3_D6.GP.cfg_b.eDriveStrength = pMspiIODS->eD6DS;
                    g_AM_BSP_GPIO_MSPI3_D7.GP.cfg_b.eDriveStrength = pMspiIODS->eD7DS;
                    g_AM_BSP_GPIO_MSPI3_D2.GP.cfg_b.eDriveStrength = pMspiIODS->eD2DS;
                    g_AM_BSP_GPIO_MSPI3_D3.GP.cfg_b.eDriveStrength = pMspiIODS->eD3DS;
                    g_AM_BSP_GPIO_MSPI3_CE0.GP.cfg_b.eDriveStrength = pMspiIODS->eCEDS;
                    g_AM_BSP_GPIO_MSPI3_D0.GP.cfg_b.eDriveStrength = pMspiIODS->eD0DS;
                    g_AM_BSP_GPIO_MSPI3_D1.GP.cfg_b.eDriveStrength = pMspiIODS->eD1DS;
                    g_AM_BSP_GPIO_MSPI3_SCK.GP.cfg_b.eDriveStrength = pMspiIODS->eSCKDS;
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
}
#endif

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
    bool bDoScrambling = true;
    am_hal_cachectrl_range_t sRange;

    //
    // Cast a pointer to the begining of the sector as the test function to call.
    //
    mspi_xip_test_function_t test_function = (mspi_xip_test_function_t)((MSPI_XIP_BASE_ADDRESS) | 0x00000001);

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
    am_util_stdio_printf("Apollo5 MSPI HEX DDR W958D6NW PSRAM Example\n\n");

#ifdef apollo5_eb
    mspi_io_drive_strength_t sMspiIODS =
    {
        .eD0DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD1DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD2DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD3DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD4DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD5DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD6DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD7DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD8DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD9DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD10DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD11DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD12DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD13DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD14DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD15DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eSCKDS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P75X,
        .eDQS0DS = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
        .eDQS1DS = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
        .eCEDS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    };
    mspi_config_drive_strength(MSPI_TEST_MODULE, MSPI_PSRAM_HEX_CONFIG.eDeviceConfig,
                               &sMspiIODS);
#elif apollo5b_eb_revb
    mspi_io_drive_strength_t sMspiIODS =
    {
        .eD0DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD1DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD2DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD3DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD4DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD5DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD6DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD7DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD8DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD9DS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD10DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD11DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD12DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD13DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD14DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eD15DS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .eSCKDS  = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
        .eDQS0DS = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
        .eDQS1DS = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
        .eCEDS   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    };
    mspi_config_drive_strength(MSPI_TEST_MODULE, MSPI_PSRAM_HEX_CONFIG.eDeviceConfig,
                               &sMspiIODS);
#endif

#ifdef MSPI_PSRAM_TIMING_CHECK
    am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_hex_ddr_init_timing_check(MSPI_TEST_MODULE, &MSPI_PSRAM_HEX_CONFIG, &MSPIDdrTimingConfig) )
    {
        am_util_stdio_printf("==== Scan Result: PASS \n");
    }
    else
    {
        am_util_stdio_printf("==== Scan Result: Failed, no valid setting.  \n");
        while(1);
    }
#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_init(MSPI_TEST_MODULE, &MSPI_PSRAM_HEX_CONFIG, &g_pDevHandle, &g_pHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
        am_util_stdio_printf("Must ensure Device is Connected and Properly Initialized!\n");
        while(1);
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_TEST_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_TEST_MODULE]);

    am_hal_interrupt_master_enable();

#ifdef MSPI_PSRAM_TIMING_CHECK
    //
    //  Set the DDR timing from previous scan.
    //
    am_devices_mspi_psram_apply_hex_ddr_timing(g_pDevHandle, &MSPIDdrTimingConfig);
#endif

    am_devices_mspi_psram_info_t sDeviceInfo;
    am_devices_mspi_psram_w958d6nw_ddr_info(g_pDevHandle, &sDeviceInfo);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to get device info!\n");
    }
    else
    {
        am_util_stdio_printf("Vendor ID: %02X\n", sDeviceInfo.ui8VendorId);
        am_util_stdio_printf("Device ID: %02X\n", sDeviceInfo.ui8DeviceId);
        am_util_stdio_printf("Aperture Base Address: 0x%08x\n", sDeviceInfo.ui32BaseAddr);
        am_util_stdio_printf("Device Size: %dKB\n", sDeviceInfo.ui32DeviceSizeKb);
    }

    //
    // Generate data into the Sector Buffer
    //
    for (uint32_t i = 0; i < MSPI_BUFFER_SIZE; i++)
    {
        g_TXBuffer[i] = (i & 0xFF);
    }
    sRange.ui32StartAddr = (uint32_t)g_TXBuffer;
    sRange.ui32Size = MSPI_BUFFER_SIZE;
    am_hal_cachectrl_dcache_clean(&sRange);

#if defined(HYBRIDSLEEP_TEST)
    {
        uint32_t ui32DelayMs = 3000;
        am_util_stdio_printf("Entering Hybrid Sleep Mode\n\n");
        am_devices_mspi_psram_w958d6nw_enter_hybridsleep(g_pDevHandle);
        am_util_stdio_printf("Delaying for %lu ms\n\n", ui32DelayMs);
        am_util_delay_ms(ui32DelayMs);
        am_util_stdio_printf("Exiting Hybrid Sleep Mode\n\n");
        am_devices_mspi_psram_w958d6nw_exit_hybridsleep(g_pDevHandle);
    }
#endif

    //
    // Make sure we aren't in XIP mode.
    //
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_disable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
    }

    //
    // Write the TX buffer into the target sector.
    //
    am_util_stdio_printf("Writing %d Bytes to Address 0x%X\n", MSPI_BUFFER_SIZE, DDR_PSRAM_TARGET_ADDRESS);
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_write(g_pDevHandle, g_TXBuffer, DDR_PSRAM_TARGET_ADDRESS, MSPI_BUFFER_SIZE, true);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to write buffer to Flash Device!\n");
    }

    //
    // Read the data back into the RX buffer.
    //
    am_util_stdio_printf("Read %d Bytes to Address 0x%X\n", MSPI_BUFFER_SIZE, DDR_PSRAM_TARGET_ADDRESS);
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_read(g_pDevHandle, g_RXBuffer, DDR_PSRAM_TARGET_ADDRESS, MSPI_BUFFER_SIZE, true);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
    }

    //
    // Compare the buffers
    //
    am_util_stdio_printf("Comparing the TX and RX Buffers\n");
    for (uint32_t i = 0; i < MSPI_BUFFER_SIZE; i++)
    {
        if (g_RXBuffer[i] != g_TXBuffer[i])
        {
            am_util_stdio_printf("TX and RX buffers failed to compare!\n");
            am_util_stdio_printf("g_TXBuffer[%d] = 0x%X\n", i, g_TXBuffer[i]);
            am_util_stdio_printf("g_RXBuffer[%d] = 0x%X\n", i, g_RXBuffer[i]);
            break;
        }
    }

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_enable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

    //
    // Verify the content using XIP aperture
    //
    uint8_t    *pui8Address = (uint8_t *)(MSPI_XIP_BASE_ADDRESS + DDR_PSRAM_TARGET_ADDRESS);
    for (uint32_t i = 0; i < MSPI_BUFFER_SIZE; i++)
    {
        uint8_t val = *pui8Address;
        if (val != g_TXBuffer[i])
        {
            am_util_stdio_printf("TX and XIP failed to compare!\n");
            break;
        }
        pui8Address++;
    }

    if ( bDoScrambling )
    {
        //
        // Turn on scrambling operation.
        //
        am_util_stdio_printf("Putting the MSPI into Scrambling mode\n");
        ui32Status = am_devices_mspi_psram_w958d6nw_ddr_enable_scrambling(g_pDevHandle);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to enable MSPI scrambling!\n");
        }
    }

    //
    // Write the executable function into the target sector.
    //
    am_util_stdio_printf("Writing Executable function of %d Bytes to Sector %d\n", MSPI_XIP_FUNCTION_SIZE, 0);
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_write(g_pDevHandle, (uint8_t *)funcAddr, 0, MSPI_XIP_FUNCTION_SIZE, true);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to write executable function to Flash Device!\n");
    }

    //
    // Set up for XIP operation.
    //
    am_util_stdio_printf("Putting the MSPI and External PSRAM into XIP mode\n");
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_enable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to put the MSPI into XIP mode!\n");
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
#ifndef ENABLE_XIPMM
    am_util_stdio_printf("Disabling the MSPI and External Flash from XIP mode\n");
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_disable_xip(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
    }
#endif

#ifdef ENABLE_XIPMM
    //
    // If scrambling on, force word accesses in XIPMM.
    //
#if defined(AM_PART_APOLLO5B)
    //
    // Turn off CPU Read queue feature when scrambling is on.
    //
    sCPURQCFG.bCPUReadQueueEnable = false;
    am_hal_mspi_control(g_pHandle, AM_HAL_MSPI_REQ_CPU_READ_COMBINE, &sCPURQCFG);
#endif
    if ( run_mspi_xipmm(0, bDoScrambling) )
    {
        am_util_stdio_printf("XIPMM aperature is working!\n");
    }
    else
    {
        am_util_stdio_printf("XIPMM aperature is NOT working!\n");
    }
#endif
    //
    // Clean up the MSPI before exit.
    //
    am_hal_interrupt_master_disable();
    NVIC_DisableIRQ(mspi_interrupts[MSPI_TEST_MODULE]);

    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_deinit(g_pDevHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to shutdown the MSPI and Flash Device!\n");
    }

    //
    //  End banner.
    //
    am_util_stdio_printf("\nApollo5 MSPI HEX DDR W958D6NW PSRAM Example Complete\n");

    while (1);
}
