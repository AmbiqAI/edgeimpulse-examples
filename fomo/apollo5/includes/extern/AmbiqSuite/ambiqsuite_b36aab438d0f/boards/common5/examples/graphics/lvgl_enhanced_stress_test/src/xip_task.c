//*****************************************************************************
//
//! @file gui_task.c
//!
//! @brief Task to handle XIP operations.
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "lvgl_enhanced_stress_test.h"

#if (XIP_TASK_ENABLE == 1)

#define MAX_XIP_TASK_DELAY    (100 * 2)

#if defined(__ICCARM__)
#error "not supported compiler"
#elif defined(__GNUC__)
  /* defined on linker script file */
    extern uint32_t _sxipcode;
    extern uint32_t _exipcode;
    extern uint32_t __xip_text_load_addr;
#endif

//*****************************************************************************
//
// XIP task handle.
//
//*****************************************************************************
TaskHandle_t XipTaskHandle;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
#if defined(XIP_IN_FLASH)
extern void *g_pFlashHandle;
extern mspi_flash_device_func_t mspi_flash_device_func;
#endif

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
extern void loops_all_mid_10k_sp_main(int argc, char *argv[]);

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
XipTask(void *pvParameters)
{
    int argc = 4;
    char* argv[4];
    argv[0] = "loops_all_mid_10k_sp";
    argv[1] = "-c1";
    argv[2] = "-w1";
    argv[3] = "-v1";

    uint32_t randomDelay;
    uint32_t ui32CodeSectionLength = 0;
    uint32_t ui32CodeSectionLoadAddr = 0;
    uint32_t ui32CodeRunAddr = 0;

    ui32CodeSectionLength = (uint32_t)&_exipcode - (uint32_t)&_sxipcode;
    ui32CodeSectionLoadAddr = (uint32_t)&__xip_text_load_addr;
    ui32CodeRunAddr = (uint32_t)&_sxipcode;
#if defined(XIP_IN_FLASH)
    if (mspi_flash_device_func.mspi_xip_enable == NULL)
    {
        am_util_stdio_printf(">> Flash Not supported XIP operations! <<\n");
        //Suspend and exit from current task.
        vTaskDelete(NULL);
    }
    uint32_t i;
    uint32_t ui32Status;
    uint32_t ui32SectorSize = (ui32CodeSectionLength + MSPI_FLASH_SECTOR_SIZE) / MSPI_FLASH_SECTOR_SIZE ;

    am_util_stdio_printf("Writing %d Bytes to flash\n", ui32CodeSectionLength);
    am_util_stdio_printf("Need to erased the sector size is %d.\n", ui32SectorSize);
    //
    // Erase the target sector.
    //
    for ( i = 0; i < ui32SectorSize; i++ )
    {
        am_util_stdio_printf("Erasing Sector %d\n", i);
        ui32Status = mspi_flash_device_func.mspi_sector_erase(g_pFlashHandle, i * MSPI_FLASH_SECTOR_SIZE);
        if (MSPI_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to erase Flash Device sector!\n");

            //Suspend and exit from current task.
            vTaskDelete(NULL);
        }
    }

    //
    // Copy the code section from MRAM to FLASH.
    //
    ui32Status = mspi_flash_device_func.mspi_write(g_pFlashHandle,
                                               (uint8_t*)ui32CodeSectionLoadAddr,
                                               0,
                                               ui32CodeSectionLength,
                                               true);
    if (MSPI_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to write the Flash Device!\n");

        //Suspend and exit from current task.
        vTaskDelete(NULL);
    }
    am_util_stdio_printf("Success to write the Flash Device!\n");
    //
    // Enable XIP.
    //
    ui32Status = mspi_flash_device_func.mspi_xip_enable(g_pFlashHandle);
    if (MSPI_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");

        //Suspend and exit from current task.
        vTaskDelete(NULL);
    }
    am_util_stdio_printf("XIP tasks in external flash..\r\n");
#elif defined(XIP_IN_PSRAM)
    //
    // Copy the code section from MRAM to PSRAM.
    //
    am_util_stdio_printf("Copy %d Bytes to psram, Code run address: 0x%x, Code loader address: 0x%x \n", ui32CodeSectionLength, ui32CodeRunAddr, ui32CodeSectionLoadAddr);

    memcpy((void *)ui32CodeRunAddr, (void *)ui32CodeSectionLoadAddr, ui32CodeSectionLength);
    am_util_stdio_printf("XIP tasks in external PSRAM..\r\n");
#else
    #error "Plaease select a memory to handle the XIP operations.!"
#endif
    while (1)
    {
        am_util_stdio_printf("Running XIP loops_all_mid_10k_sp tasks...\r\n");

        //
        // Run the coremarkPro workload in XIP mode.
        //
        loops_all_mid_10k_sp_main(argc, argv);

        am_util_stdio_printf("Running XIP loops_all_mid_10k_sp tasks done...\r\n");

        srand(xTaskGetTickCount());
        randomDelay = rand() % MAX_XIP_TASK_DELAY;
        vTaskDelay(randomDelay);
    }
}

#endif  //XIP_TASK_ENABLE
