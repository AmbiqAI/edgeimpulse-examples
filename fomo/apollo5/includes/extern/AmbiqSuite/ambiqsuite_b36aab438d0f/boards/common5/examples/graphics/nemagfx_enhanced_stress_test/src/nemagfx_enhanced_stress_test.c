//*****************************************************************************
//
//! @file nemagfx_enhanced_stress_test.c
//!
//! @brief NemaGFX example.
//! This example uses MSPI, IOM, DMA, GPU, and DC to read/write data from different
//! ram sections to put enormous pressure on the AXI bus.
//! The GUI task also demonstrates how to use the partial frame buffer and ping-pong
//! buffer features in a pipeline workflow for DMA, GPU, and DC. In GUI task,
//! DMA copy texture from external PSRAM to internal SSRAM or ExtendedRAM,
//! GPU render these textures to frame buffer, and DC transfer the framebuffer to
//! display panel, these three steps are organized in a pipeline to make DMA, GPU,
//! and DC work in parallel.
//!
//! SMALLFB
//! undefine this to disable partial framebuffer features
//! SMALLFB_STRIPES
//! controls how many stripes to divide the whole framebuffer
//!
//! Note: This example needs PSRAM devices connected to MSPI0, if you encounter
//! hardfault, please check your PSRAM setting.You are supposed to see a digital
//! Quartz clock if GUI task runs successfully.
//!
//! Configuration:
//! On FPGA Turbo board: Jumper across FMC GP9-10 (Cayenne GP53-GP55)
//!                      USB, XIP, and ADC tasks disabled due
//!                      to FPGA and board constraints
//!                      If XIP task is necessary, it can be enabled and run
//!                      in IOX Mode 1 using an external display card which has
//!                      been re-worked with a 1.8V AP PSRAM on U2 (MSPI3)
//!                      WARNING!!!: Do not use the display card as-is, since
//!                      by default it is populated with a 1.2V PSRAM!!!
//!
//! On Apollo5_eb board: Jumper across GPIO0-2 for uart task
//!                      Use IOX mode 3 (see CAYNSWS-2685)
//!                      Connect USB C cable to J38 for USB Task
//!                      SW5 should be in USB_AP5 position.
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_enhanced_stress_test.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define AM_DEBUG_PRINTF

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern am_devices_mspi_psram_config_t g_sMspiPsramConfig;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

//We need to set up a small heap to be used by Clib, delete this may cause unexpected error.
__attribute__ ((section(".heap")))
uint32_t clib_heap[100];



am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = false, .bWriteAllocate = false},
    .sInnerAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = false, .bWriteAllocate = false},
    .eDeviceAttr = 0
};

#if (IOMPSRAM_TASK_ENABLE==1) && (MSPI_TASK_ENABLE==1) && ((GUI_TASK_ENABLE==1) || (XIP_TASK_ENABLE==1))

uint32_t g_ui32Count = 3;

am_hal_mpu_region_config_t sMPUCfg[3] =
{
    {//IOM PSRAM
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)ui32DMATCBBuffer2,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)ui32DMATCBBuffer2 + sizeof(ui32DMATCBBuffer2) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    },
    {//MSPI
    .ui32RegionNumber = 7,
    .ui32BaseAddress = (uint32_t)FlashDMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)FlashDMATCBBuffer + sizeof(FlashDMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    },
    {//XIP
    .ui32RegionNumber = 8,
    .ui32BaseAddress = (uint32_t)ui32DMATCBBufferPSRAMXIP,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)ui32DMATCBBufferPSRAMXIP + sizeof(ui32DMATCBBufferPSRAMXIP) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    }
};

#elif (IOMPSRAM_TASK_ENABLE==1) && (MSPI_TASK_ENABLE==0) && ((GUI_TASK_ENABLE==1) || (XIP_TASK_ENABLE==1))
uint32_t g_ui32Count = 2;

am_hal_mpu_region_config_t sMPUCfg[2] =
{
    {//IOM PSRAM
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)ui32DMATCBBuffer2,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)ui32DMATCBBuffer2 + sizeof(ui32DMATCBBuffer2) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    },
    {//XIP
    .ui32RegionNumber = 7,
    .ui32BaseAddress = (uint32_t)ui32DMATCBBufferPSRAMXIP,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)ui32DMATCBBufferPSRAMXIP + sizeof(ui32DMATCBBufferPSRAMXIP) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    }
};
#elif (IOMPSRAM_TASK_ENABLE==0) && (MSPI_TASK_ENABLE==1) && ((GUI_TASK_ENABLE==1) || (XIP_TASK_ENABLE==1))
uint32_t g_ui32Count = 2;

am_hal_mpu_region_config_t sMPUCfg[2] =
{
    {//MSPI
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)FlashDMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)FlashDMATCBBuffer + sizeof(FlashDMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    },
    {//XIP
    .ui32RegionNumber = 7,
    .ui32BaseAddress = (uint32_t)ui32DMATCBBufferPSRAMXIP,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)ui32DMATCBBufferPSRAMXIP + sizeof(ui32DMATCBBufferPSRAMXIP) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    }
};
#elif (IOMPSRAM_TASK_ENABLE==0) && (MSPI_TASK_ENABLE==0) && ((GUI_TASK_ENABLE==1) || (XIP_TASK_ENABLE==1))
uint32_t g_ui32Count = 1;
// XIP
am_hal_mpu_region_config_t sMPUCfg =
{
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)ui32DMATCBBufferPSRAMXIP,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)ui32DMATCBBufferPSRAMXIP + sizeof(ui32DMATCBBufferPSRAMXIP) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
};
#elif (IOMPSRAM_TASK_ENABLE==1) && (MSPI_TASK_ENABLE==1) && ((GUI_TASK_ENABLE==0) && (XIP_TASK_ENABLE==0))
uint32_t g_ui32Count = 2;

am_hal_mpu_region_config_t sMPUCfg[2] =
{
    {//IOM PSRAM
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)ui32DMATCBBuffer2,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)ui32DMATCBBuffer2 + sizeof(ui32DMATCBBuffer2) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    },
    {//MSPI
    .ui32RegionNumber = 7,
    .ui32BaseAddress = (uint32_t)FlashDMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)FlashDMATCBBuffer + sizeof(FlashDMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    }
};
#elif (IOMPSRAM_TASK_ENABLE==1) && (MSPI_TASK_ENABLE==0) && ((GUI_TASK_ENABLE==0) && (XIP_TASK_ENABLE==0))
uint32_t g_ui32Count = 1;

am_hal_mpu_region_config_t sMPUCfg =
{
    //IOM PSRAM
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)ui32DMATCBBuffer2,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)ui32DMATCBBuffer2 + sizeof(ui32DMATCBBuffer2) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    
 };
#elif (IOMPSRAM_TASK_ENABLE==0) && (MSPI_TASK_ENABLE==1) && ((GUI_TASK_ENABLE==0) && (XIP_TASK_ENABLE==0))
uint32_t g_ui32Count = 1;

am_hal_mpu_region_config_t sMPUCfg =
{
    //MSPI
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)FlashDMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)FlashDMATCBBuffer + sizeof(FlashDMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
    
};
#endif

#if (IOMPSRAM_TASK_ENABLE==1) || (MSPI_TASK_ENABLE==1) || ((GUI_TASK_ENABLE==1) || (XIP_TASK_ENABLE==1))	
void MPU_ENABLE(void){

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
    am_hal_mpu_region_configure(&sMPUCfg, g_ui32Count);
    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);
    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);
    am_util_stdio_printf("MPU Enable!\n");
}
#endif

//*****************************************************************************
//
// Error Handling function.
//
//*****************************************************************************
void
vErrorHandler(void)
{
    const char* pTaskName;

    pTaskName = pcTaskGetName(NULL);

    //
    // Called when error occurs in a Task
    //
    am_util_stdio_printf("Error in Task : \r");



    am_util_stdio_printf(" %s \r\n", pTaskName);

    //
    // While loop to prevent other tasks from getting scheduled.
    //
    while (1)
    {
        __asm("BKPT #0\n") ; // Break into the debugger
    }
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    //
    // Add a wait before the program starts.  The debugger/J-Link are being locked out.
    //
    // am_util_delay_ms(5000);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
	
#if (IOMPSRAM_TASK_ENABLE==1) || (MSPI_TASK_ENABLE==1) || ((GUI_TASK_ENABLE==1) || (XIP_TASK_ENABLE==1))	

    MPU_ENABLE();
#endif
	
    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();


#if (MSPI_PSRAM_XIPCODE_MODULE == EMMC_TASK_MSPI_XIP_MODULE)
    am_util_stdio_printf("WARNING: MSPI_PSRAM_XIPCODE_MODULE and EMMC_TASK_MSPI_XIP_MODULE must not be set to the same module!\n");
#endif

#if (GUI_TASK_ENABLE == 1)
    if (MSPI_PSRAM_STATUS_SUCCESS != mspi_psram_init() )
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
        vErrorHandler();
    }
#endif

#if ((XIP_TASK_ENABLE == 1) && (MSPI_PSRAM_XIPCODE_MODULE != MSPI_PSRAM_MODULE))
    if (MSPI_PSRAM_STATUS_SUCCESS != mspi_psram_xip_code_init() )
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
        vErrorHandler();
    }
    am_util_stdio_printf("XIP_TASK_ENABLE PSRAM Device init pass!\n");
#endif


#if (MSPI_TASK_ENABLE == 1)

     // When MSPI0 is at hex mode, we couldn't use MSPI1
    //if ( ((g_sMspiPsramConfig.eDeviceConfig == AM_HAL_MSPI_FLASH_HEX_DDR_CE1) || (g_sMspiPsramConfig.eDeviceConfig == AM_HAL_MSPI_FLASH_HEX_DDR_CE0))
      //  && (MSPI_FLASH_MODULE == 1))
    //{
      //  am_util_stdio_printf("When MSPI0 is at hex mode, we couldn't use MSPI1");
    //}

    if ( MSPI_FLASH_STATUS_SUCCESS != mspi_flash_init() )
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
        vErrorHandler();
    }

#endif

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("nemagfx_stress_test Example\n");
    //
    // Run the application.
    //
    run_tasks();

    //
    // We shouldn't never get here.
    //
    while (1)
    {
    }

}

