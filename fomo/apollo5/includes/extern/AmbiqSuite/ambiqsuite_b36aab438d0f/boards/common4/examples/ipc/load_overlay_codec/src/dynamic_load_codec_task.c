//*****************************************************************************
//
//! @file dynamic_load_codec_task.c
//!
//! @brief Task to handle dynamic load codec operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "ipc_service.h"
#include "../../../../../../third_party/open-amp/libmetal/lib/errno.h"
#include "load_overlay_codec.h"

#define CODEC_STATE_SIZE 1
#define CODEC_DATA_SIZE  12
#define CODEC_CODE_SIZE  12

//*****************************************************************************
//
// The memory address of ambt53
//
//*****************************************************************************
#define CODEC_STATE_BASE 0x2c000
#define CODEC_CHECK_CODE_BASE 0x2c010
#define CODEC_CHECK_DATA_BASE 0x2c100
typedef enum
{
    CODEC_INVALID = 0X01,
    CODEC_READY,
} CODEC_STATES;
//*****************************************************************************
//
// Global variable.
//
//*****************************************************************************
SemaphoreHandle_t xSemaphore = NULL;
metal_thread_t load_mp3_task_handle;
metal_thread_t load_aac_task_handle;

//*****************************************************************************
//
// ambt53 MSPI Interrupt handler.
//
//*****************************************************************************
void
ambt53_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(pvRpmsgMspiHandle, &ui32Status, false);
    am_hal_mspi_interrupt_clear(pvRpmsgMspiHandle, ui32Status);
    am_hal_mspi_interrupt_service(pvRpmsgMspiHandle, ui32Status);
}

//*****************************************************************************
//
// Notifies ambt53 of the current codec states
//
//*****************************************************************************
static
void set_codec_state(CODEC_STATES state)
{
    am_devices_ambt53_memory_write(g_RpmsgDevHdl, CODEC_STATE_BASE,
                    CODEC_STATE_SIZE, &state);
}

//*****************************************************************************
//
// Determine the runing codec by checking the overlay data of ambt53
//
//*****************************************************************************
static
void check_codec_data(void)
{
    uint8_t codec_data[CODEC_DATA_SIZE] = {0};
    am_devices_mspi_ambt53_dma_read(g_RpmsgDevHdl, codec_data,
                        CODEC_CHECK_DATA_BASE, CODEC_DATA_SIZE, true);
    am_util_stdio_printf("%s\r\n", codec_data);
}

//*****************************************************************************
//
// Determine the runing codec by checking the overlay code of ambt53
//
//*****************************************************************************
static
void check_codec_code(void)
{
    uint8_t codec_code[CODEC_CODE_SIZE] = {0};
    am_devices_mspi_ambt53_dma_read(g_RpmsgDevHdl, codec_code,
                        CODEC_CHECK_CODE_BASE, CODEC_CODE_SIZE, true);
    am_util_stdio_printf("%s\n", codec_code);
}

//*****************************************************************************
//
// Perform initial ambt53
//
//*****************************************************************************
int
init_ambt53(void)
{
    int status = 0;
    am_util_debug_printf("init ambt53\r\n");
    xSemaphore = xSemaphoreCreateMutex();
    // #### INTERNAL BEGIN ####
#ifdef AM_DEBUG_PRINTF
    //
    // Print some device information.
    //
    am_util_id_t sIdDevice;
    uint32_t ui32StrBuf;
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
    am_util_stdio_printf("Device type: %s\n",
         sIdDevice.pui8DeviceName);

    am_util_stdio_printf("Device Info:\n"
                         "\tPart number: 0x%08X\n"
                         "\tChip ID0:    0x%08X\n"
                         "\tChip ID1:    0x%08X\n"
                         "\tRevision:    0x%08X (Rev%c%c)\n",
                         sIdDevice.sMcuCtrlDevice.ui32ChipPN,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID0,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID1,
                         sIdDevice.sMcuCtrlDevice.ui32ChipRev,
                         sIdDevice.ui8ChipRevMaj, sIdDevice.ui8ChipRevMin );

    //
    // If not a multiple of 1024 bytes, append a plus sign to the KB.
    //
    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32MRAMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tMRAM size:   %7d (%d KB%s)\n",
                         sIdDevice.sMcuCtrlDevice.ui32MRAMSize,
                         sIdDevice.sMcuCtrlDevice.ui32MRAMSize / 1024,
                         &ui32StrBuf);

    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32DTCMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tDTCM size:   %7d (%d KB%s)\n",
                         sIdDevice.sMcuCtrlDevice.ui32DTCMSize,
                         sIdDevice.sMcuCtrlDevice.ui32DTCMSize / 1024,
                         &ui32StrBuf);

    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32SSRAMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tSSRAM size:  %7d (%d KB%s)\n\n",
                         sIdDevice.sMcuCtrlDevice.ui32SSRAMSize,
                         sIdDevice.sMcuCtrlDevice.ui32SSRAMSize / 1024,
                         &ui32StrBuf);
#endif // AM_DEBUG_PRINTF
    // #### INTERNAL END ####

    // Boot the ambt53.
    status = am_devices_ambt53_boot();
    if (status != 0)
    {
        am_util_stdio_printf("am_devices_ambt53_boot failure\n");
        return -1;
    }

    return 0;
}

//*****************************************************************************
//
// Load mp3 codec to ambt53 overlay memory and check the currently running codec
// Please open macro LOAD_AMBT53_FIRMWARE in am_device_ambt53.h before testing example
//
//*****************************************************************************
void
LoadMP3Task(void *pvParameters)
{
    int status = 0;

    while(1)
    {
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        set_codec_state(CODEC_INVALID);
        am_util_stdio_printf("load mp3 code\n");

        #if (LOAD_AMBT53_FIRMWARE == LOAD_ELF)
        status = am_devices_ambt53_firmware_dynamic_load(".overlay.codec.mp3.code");
        if (status != 0)
        {
            am_util_stdio_printf("load mp3 code fail\n");
            break;
        }
        am_util_stdio_printf("load mp3 data\n");
        status = am_devices_ambt53_firmware_dynamic_load(".overlay.codec.mp3.data");
        if (status != 0)
        {
            am_util_stdio_printf("load mp3 data fail\n");
            break;
        }
        #endif
        /*Notifies ambt53 that the codec load is complete*/
        am_util_stdio_printf("mp3 codec ready\n");
        set_codec_state(CODEC_READY);
        /*Wait for ambt53 to run for a while, and then check the currently runing codec*/
        am_util_delay_ms(3000);
        check_codec_code();
        check_codec_data();

        xSemaphoreGive(xSemaphore);
        metal_thread_yield();
    }
}

//*****************************************************************************
//
// Load AAC codec to ambt53 overlay memory and check the currently running codec
// Please open macro LOAD_AMBT53_FIRMWARE in am_device_ambt53.h before testing example
//
//*****************************************************************************
void
LoadAACTask(void *pvParameters)
{
    int status = 0;

    while (1)
    {
        xSemaphoreTake(xSemaphore, portMAX_DELAY);
        set_codec_state(CODEC_INVALID);
        am_util_stdio_printf("load aac code\n");

        #if (LOAD_AMBT53_FIRMWARE == LOAD_ELF)
        status = am_devices_ambt53_firmware_dynamic_load(".overlay.codec.aac.code");
        if (status != 0)
        {
            am_util_stdio_printf("load aac code fail\n");
            break;
        }
        am_util_stdio_printf("load aac data\n");
        status = am_devices_ambt53_firmware_dynamic_load(".overlay.codec.aac.data");
        if (status != 0)
        {
            am_util_stdio_printf("load aac data fail\n");
            break;
        }
        #endif
        /*Notifies ambt53 that the codec load is complete*/
        am_util_stdio_printf("aac codec ready\n");
        set_codec_state(CODEC_READY);
        /*Wait for ambt53 to run for a while, and then check the currently runing codec*/
        am_util_delay_ms(3000);
        check_codec_code();
        check_codec_data();

        xSemaphoreGive(xSemaphore);
        metal_thread_yield();

    }
}
