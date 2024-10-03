//*****************************************************************************
//
//! @file file_writer_task.c
//!
//! @brief Task that writes received and decrypted music data to a file
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include <stdio.h>

#include "complex_test_cases.h"
#include "file_writer_task.h"

#include "funky_trunc.h"
#include "common_emmc_fatfs.h"

#include "FreeRTOS.h"
#include "semphr.h"
extern volatile bool g_bPlaybackTaskComplete;

// Uncomment to print a directory on every file receive
//#define FILEWRITER_LIST_DIR_ON_RECEIVE

//*****************************************************************************
//
// File writer task handle.
//
//*****************************************************************************
TaskHandle_t file_writer_task_handle;
TaskHandle_t file_writer_scaffold_task_handle;

//*****************************************************************************
//
// Global variables and templates
//
//*****************************************************************************
volatile bool g_bFileWriterTaskComplete = false;
FileWriterMetadata_t g_sFileWriterMetadata;
SemaphoreHandle_t g_xFileWriterBufMutex;
extern SemaphoreHandle_t g_xPlaybackFilesInUseMutex;

//*****************************************************************************
//
// Perform initial setup for the file writer task
//
//*****************************************************************************
void
FileWriterTaskSetup(void)
{
    am_util_stdio_printf("File Writer task: setup\r\n");
    g_sFileWriterMetadata.buffer = (uint8_t*)(DECODED_BUFFER_ADDRESS);
    g_sFileWriterMetadata.bytes = 0;

    g_xFileWriterBufMutex = xSemaphoreCreateMutex();
    if(!EMMCFatFsSetup())
    {
        am_util_stdio_printf("File writer setup failed in EMMC fatfs setup\n");
        while(1);
    }
}

//*****************************************************************************
//
// Perform initial setup for the file writer scaffold task
//
//*****************************************************************************
void
FileWriterScaffoldTaskSetup(void)
{
    am_util_stdio_printf("File Writer Scaffold task: setup\r\n");
}


//*****************************************************************************
//
// FileWriterScaffoldTask - Task to feed dummy data into the file writer
//
//*****************************************************************************
void
FileWriterScaffoldTask(void *pvParameters)
{
    uint32_t ui32Remaining;
    uint32_t ui32MP3Index = 0;
    uint32_t file_index = 0;
    am_util_stdio_printf("\r\nRunning File Writer Scaffold Task\r\n");
    // write content to buffer
    while (!g_bFileWriterTaskComplete)
    {
        xSemaphoreTake(g_xFileWriterBufMutex, portMAX_DELAY);
        if(g_sFileWriterMetadata.bytes <= 0) {
            if(ui32MP3Index == 0) {
                // Starting at the beginning of a file, so choose a new filename
                sprintf(g_sFileWriterMetadata.filename, "fnky%04lx.mp3", file_index);
                g_sFileWriterMetadata.op = FILE_WRITER_CREATE;
            } else {
                if(g_sFileWriterMetadata.op == FILE_WRITER_CREATE) {
                    g_sFileWriterMetadata.op = FILE_WRITER_APPEND;
                }
            }
            ui32Remaining = funky_trunc_mp3_len - ui32MP3Index;
            g_sFileWriterMetadata.bytes = ui32Remaining >= DECODED_BUFFER_SIZE ? DECODED_BUFFER_SIZE: ui32Remaining;
            // copy the static file data into the buffer
            memmove(g_sFileWriterMetadata.buffer,
                                funky_trunc_mp3 + ui32MP3Index,
                                g_sFileWriterMetadata.bytes);
            if(ui32Remaining <= DECODED_BUFFER_SIZE) {
                // We transferred our last buffer for this file, close it out
                ui32MP3Index = 0;
                g_sFileWriterMetadata.op = FILE_WRITER_CLOSE;
                file_index = (file_index + 1) & 0xFFFF; // next filename, but wrap at 65536 files
            }
            else
            {
                ui32MP3Index += DECODED_BUFFER_SIZE;
            }
        }
        xSemaphoreGive(g_xFileWriterBufMutex);
    }

    am_util_stdio_printf("\r\nSuspending File Writer Scaffold Task\r\n");
    // Suspend and delete the task
    vTaskDelete(NULL);
}

#ifdef FILEWRITER_LIST_DIR_ON_RECEIVE
//*****************************************************************************
//
// list_files - Print a directory of files in the directory described by path
//
// assumes we hold the g_xFatFsMutex
//
//*****************************************************************************
static void
list_files(char* path)
{
    DIR dir;
    static FILINFO fno;
    FRESULT res;
    res = f_opendir(&dir, path);
    while(res == FR_OK)
    {
        res = f_readdir(&dir, &fno);
        if((strlen(fno.fname) <= 0) || (res != FR_OK))
            break;
        am_util_stdio_printf("%s  %d B\r\n", fno.fname, fno.fsize);
    }
    f_closedir(&dir);
}
#endif //FILEWRITER_LIST_DIR_ON_RECEIVE

//*****************************************************************************
//
// FileWriterTask - Write data from a common buffer to eMMC
//
//*****************************************************************************
void
FileWriterTask(void *pvParameters)
{
    FIL fp;
    TCHAR filename[128];
    TCHAR moved_filename[128];
    UINT bytes_written;
    uint32_t ui32FilesSinceScrub = 0;
    FRESULT res;
    am_util_stdio_printf("\r\nRunning File Writer Task\r\n");
    // write content to file
    while(!g_bPlaybackTaskComplete)
    {
        xSemaphoreTake(g_xFileWriterBufMutex, portMAX_DELAY);
        if(g_sFileWriterMetadata.bytes > 0)
        {
            if(g_sFileWriterMetadata.op == FILE_WRITER_CREATE && ui32FilesSinceScrub > FS_SCRUB_NUM_FILES)
            {
                // It's ok if we can't get the scrub mutex, we'll try next time around
                if(xSemaphoreTake(g_xFsScrubMutex, (TickType_t ) 100) == pdTRUE)
                {
                    am_util_stdio_printf("\nFileWriterTask      - reinitializing eMMC after %d files written\n", ui32FilesSinceScrub);
                    scrub_fs();
                    xSemaphoreGive(g_xFsScrubMutex);
                    ui32FilesSinceScrub = 0;
                }
            }
            strcpy(filename, FILE_DIR_TEMP "/");
            strcat(filename, g_sFileWriterMetadata.filename);
            xSemaphoreTake(g_xFatFsMutex, portMAX_DELAY);
            res = f_open(&fp, filename,
                (g_sFileWriterMetadata.op == FILE_WRITER_CREATE ? FA_CREATE_ALWAYS : FA_OPEN_APPEND)
                | FA_WRITE);
            res = f_write(&fp, g_sFileWriterMetadata.buffer, g_sFileWriterMetadata.bytes, &bytes_written);
            // TODO: do something with the file results
            f_close(&fp);

            if(g_sFileWriterMetadata.op == FILE_WRITER_CLOSE)
            {
                // if we're done writing the file, move it to /music/
                strcpy(moved_filename, FILE_DIR_MUSIC "/");
                strcat(moved_filename, g_sFileWriterMetadata.filename);
                f_unlink(moved_filename);
                res = f_rename(filename, moved_filename);
                ui32FilesSinceScrub++;
                am_util_stdio_printf("FileWriterTask      - received %s\r\n", moved_filename);
#ifdef FILEWRITER_LIST_DIR_ON_RECEIVE
                am_util_stdio_printf("FileWriterTask      - files in %s :\r\n", FILE_DIR_MUSIC);
                am_util_stdio_printf("--------\r\n");
                list_files(FILE_DIR_MUSIC);
                am_util_stdio_printf("--------\r\n");
#endif //FILEWRITER_LIST_DIR_ON_RECEIVE
            }
            // Indicate that the filewriter buffer is free for another payload
            g_sFileWriterMetadata.bytes = 0;
            xSemaphoreGive(g_xFatFsMutex);
        }
        xSemaphoreGive(g_xFileWriterBufMutex);
    }
    am_util_stdio_printf("FileWriterTask      - Suspending File Writer Task\r\n");
    // Indicate the Source Task loops are complete
    g_bFileWriterTaskComplete = true;
    // Suspend and delete the task
    vTaskDelete(NULL);
}
