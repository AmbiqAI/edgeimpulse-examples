//*****************************************************************************
//
//! @file mspi_task.c
//!
//! @brief Task to handle MSPI operations.
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

#ifndef BAREMETAL
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FLASH_TARGET_ADDRESS     (MSPI_FLASH_SECTOR_SIZE * 0)
#define MSPI_BUFFER_SIZE        (2*1024)  // 2K example buffer size.

//Undefine this if you want do dummy read.
#define MSPI_DATA_CHECK
//#define MSPI_TASK_DEBUG_LOG

//MSPI task loop delay
#define MAX_MSPI_TASK_DELAY    (100 * 2)

//Delay to wait read operation complete.
#define MSPI_READ_DELAY        (10)

//*****************************************************************************
//
// MSPI task handle.
//
//*****************************************************************************
TaskHandle_t MspiTaskHandle;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
extern void *g_pFlashHandle;
extern mspi_flash_device_func_t mspi_flash_device_func;
//*****************************************************************************
//
// MSPI Buffer.
//
//*****************************************************************************
#ifdef MSPI_DATA_CHECK
static uint8_t  gFlashTXBuffer[MSPI_BUFFER_SIZE];
#endif

AM_SHARED_RW static uint8_t  gFlashRXBufferSSRAM[MSPI_BUFFER_SIZE];
static uint8_t  gFlashRXBufferTCM[MSPI_BUFFER_SIZE];

//*****************************************************************************
//
// MSPI task.
//
//*****************************************************************************
void
MspiTask(void *pvParameters)
{
    uint32_t      ui32Status;
    uint8_t*      ptrBuffer;
    uint32_t      randomDelay;

#ifdef MSPI_DATA_CHECK
    //
    // Erase the target sector.
    //
    am_util_stdio_printf("Erasing Sector %d\n", FLASH_TARGET_ADDRESS);
    ui32Status = mspi_flash_device_func.mspi_sector_erase (g_pFlashHandle, FLASH_TARGET_ADDRESS);
    if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to erase Flash Device sector!\n");
    }

    //
    // Generate data into the Sector Buffer
    //
    for (uint32_t i = 0; i < MSPI_BUFFER_SIZE; i++)
    {
        gFlashTXBuffer[i] = (i & 0xFF);
    }

    //
    // Write the TX buffer into the target sector.
    //
    am_util_stdio_printf("Writing %d Bytes to Sector %d\n", MSPI_BUFFER_SIZE, FLASH_TARGET_ADDRESS);
    ui32Status = mspi_flash_device_func.mspi_write(g_pFlashHandle, gFlashTXBuffer, FLASH_TARGET_ADDRESS, MSPI_BUFFER_SIZE, true);
    if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to write buffer to Flash Device!\n");
    }
#endif


    ptrBuffer = gFlashRXBufferSSRAM;

    while(1)
    {
        //
        //Clear RX buffer
        //
        memset(ptrBuffer, 0, MSPI_BUFFER_SIZE);

        //
        // Read the data back into the RX buffer in SSRAM.
        //
#if defined(MSPI_TASK_DEBUG_LOG)
        char* ptrLocationString;
        ptrLocationString = (ptrBuffer == gFlashRXBufferSSRAM) ? "SSRAM" : "TCM";
        am_util_stdio_printf("Read %d Bytes from address %d to %s\n", MSPI_BUFFER_SIZE, FLASH_TARGET_ADDRESS, ptrLocationString);
        am_util_delay_ms(100);
#else
        am_util_stdio_printf("[TASK] : MSPI\n");
#endif

        ui32Status = mspi_flash_device_func.mspi_read(g_pFlashHandle, ptrBuffer, FLASH_TARGET_ADDRESS , MSPI_BUFFER_SIZE, false);
#if defined(MSPI_TASK_DEBUG_LOG)
        if ( AM_HAL_MSPI_FIFO_FULL_CONDITION == ui32Status )
        {
          am_util_stdio_printf("FIFO full condition is detected!\n");
        }
        else if (MSPI_FLASH_STATUS_SUCCESS != ui32Status)
        {
          am_util_stdio_printf("Failed to read buffer from Flash Device!\n");
        }
#endif

#ifdef MSPI_DATA_CHECK
        //Wait until the read operation is complete.
        vTaskDelay(MSPI_READ_DELAY);

        //
        //Data check
        //
        for (uint32_t i = 0; i < MSPI_BUFFER_SIZE; i++ )
        {
            if ( gFlashTXBuffer[i] != ptrBuffer[i] )
            {
                am_util_stdio_printf("Data compare failed at index:%d, address:0x%X, expect:0x%2X, received:0x%2X\n",
                                      i,
                                      &ptrBuffer[i],
                                      gFlashTXBuffer[i],
                                      ptrBuffer[i]);
                break;
            }
        }
#endif
        srand(xTaskGetTickCount());
        randomDelay = rand() % MAX_MSPI_TASK_DELAY;
        vTaskDelay(randomDelay);

        //
        //swap rx buffer
        //
        if ( ptrBuffer == gFlashRXBufferSSRAM )
        {
            ptrBuffer = gFlashRXBufferTCM;
        }
        else
        {
            ptrBuffer = gFlashRXBufferSSRAM;
        }

    }
}
#endif
