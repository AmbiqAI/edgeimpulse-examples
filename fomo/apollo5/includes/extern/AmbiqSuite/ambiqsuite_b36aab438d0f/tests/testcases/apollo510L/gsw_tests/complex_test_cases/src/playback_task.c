//*****************************************************************************
//
//! @file playback_task.c
//!
//! @brief Task to handle reading and decoding MP3 files from eMMC
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "complex_test_cases.h"
#include "playback_task.h"

#include "FreeRTOS.h"

// MP3 decoder and related config flags
#define MINIMP3_ONLY_MP3
#define MINIMP3_NO_SIMD
#define MINIMP3_IMPLEMENTATION

#define MP3_VALID_DECODE_FRAME_SAMPLES  1152

#include "minimp3.h"

//*****************************************************************************
//
// Playback task handle.
//
//*****************************************************************************
TaskHandle_t playback_task_handle;

//*****************************************************************************
//
// Global variables and templates
//
//*****************************************************************************
volatile bool g_bPlaybackTaskComplete = false;

mp3dec_t* psMP3d;

//*****************************************************************************
//
// Perform initial setup for the playback task.
//
//*****************************************************************************
void
PlaybackTaskSetup(void)
{
    am_util_stdio_printf("Playback task: setup\r\n");
}

//*****************************************************************************
//
// find_mp3_file - Select an MP3 filename from the directory in "path".
//
// Selection will be the "index"-th mp3 file listed in the directory.
// Note that this index is filesystem-sequential and not filename-sequential.
// If there are less than "index" files in the directory, we wrap around to the
// first file in the directory. This is a low-cost way of generating eMMC file
// read traffic that's similar to a real world player with a playlist without
// needing the memory resources to handle a real playlist.
//
// Returns chosen file's filename in "*fileinfo.fname" and the current index in
// "*index"
// If no file is found in the directory, "*fileinfo.fname" will be an empty string.
// Returns true if all file ops were successful, false otherwise.
//
// NOTE: Assumes the caller holds the g_xFatFsMutex
//
//*****************************************************************************
static bool
find_mp3_file(char *path, uint32_t* index, FILINFO* fileinfo)
{
    DIR dir;
    FRESULT res;
    uint32_t filecount = 0;
    res = f_findfirst(&dir, fileinfo, path, PLAYBACK_FILE_MATCH_PATTERN);

    while((res == FR_OK) && (strlen(fileinfo->fname) > 0))
    {
        // did we find it?
        if(filecount == *index)
            break;

        res = f_findnext(&dir, fileinfo);

        if(res != FR_OK)
            break;

        filecount++;
        if(strlen(fileinfo->fname) <= 0)
        {
            if(strlen(fileinfo->fname) > 64)
            {
                am_util_stdio_printf("Filename error!\n");
                while(1);
            }
            // we're at the end of the directory or something went wrong, start over
            f_closedir(&dir);
            res = f_findfirst(&dir, fileinfo, path, PLAYBACK_FILE_MATCH_PATTERN);
            *index = *index % filecount; // Wrap the index so we don't have to start over more than once
            filecount = 0;
        }
    }
    f_closedir(&dir);
    return(res == FR_OK);
}

//*****************************************************************************
//
// PlaybackTask - Play an MP3 file from eMMC using a buffer in XIPMM space
//
//*****************************************************************************
void
PlaybackTask(void *pvParameters)
{
    uint32_t ui32BytesInBuf = 0;
    uint8_t *file_buf = (uint8_t*)(PLAYBACK_FILE_BUFFER_ADDRESS);
    UINT bytes_read;
    uint32_t file_index;
    int isamples;
    uint32_t buf_decoded_bytes;
    uint32_t ui32DecodeFails = 0;
    mp3dec_frame_info_t sinfo;
    short *pcm;

    FIL fp;
    FILINFO fileinfo;
    TCHAR current_filename[128];
    FRESULT res;
    uint32_t dir_index = 0;
    uint32_t played_files = 0;
    am_util_stdio_printf("\r\nRunning Playback Task\r\n");
    psMP3d = pvPortMalloc(sizeof(mp3dec_t));
    pcm = pvPortMalloc(sizeof(short) * MINIMP3_MAX_SAMPLES_PER_FRAME);
    current_filename[0] = 0;
    mp3dec_init(psMP3d);
#ifdef RUN_FOREVER
    while(1)
#else
    while(played_files < PLAYBACK_NUM_FILES)
#endif
    {
        // Don't allow the file writer task to scrub the filesystem until we're
        // done finding the current file and playing it back
        xSemaphoreTake(g_xFsScrubMutex, portMAX_DELAY);
        xSemaphoreTake(g_xFatFsMutex, portMAX_DELAY);

        res = find_mp3_file(FILE_DIR_MUSIC, &dir_index, &fileinfo);

        if(strlen(fileinfo.fname) <= 0)
        {
            // No files to play right now
            am_util_stdio_printf("PlaybackTask        - no files to play\r\n");
            xSemaphoreGive(g_xFatFsMutex);
            xSemaphoreGive(g_xFsScrubMutex);
            vTaskDelay(100 / portTICK_PERIOD_MS); // check for new files in 100ms
            continue;
        }
        xSemaphoreGive(g_xFatFsMutex);

        dir_index++;
        played_files++;
        file_index = 0;
        while(fileinfo.fsize > file_index)
        {
            strcpy(current_filename, FILE_DIR_MUSIC "/");
            strcat(current_filename, fileinfo.fname);
            xSemaphoreTake(g_xFatFsMutex, portMAX_DELAY);
            res = f_open(&fp, current_filename, FA_READ);
            if (res != FR_OK)
            {
                xSemaphoreGive(g_xFatFsMutex);
                TEST_FAIL();
                break;
            }
            res = f_lseek(&fp, file_index);
            if (res != FR_OK)
            {
                xSemaphoreGive(g_xFatFsMutex);
                TEST_FAIL();
                break;
            }
            res = f_read(&fp, file_buf + ui32BytesInBuf, PLAYBACK_FILE_BUFFER_SIZE - ui32BytesInBuf, &bytes_read);
            if (res != FR_OK)
            {
                xSemaphoreGive(g_xFatFsMutex);
                TEST_FAIL();
                break;
            }
            xSemaphoreGive(g_xFatFsMutex);
            file_index += bytes_read;
            am_util_stdio_printf("PlaybackTask        - read %d bytes from %s dir_index %d idx %d bufpos %d\r\n", bytes_read, fileinfo.fname, dir_index, file_index, ui32BytesInBuf);
            ui32BytesInBuf += bytes_read;
            buf_decoded_bytes = 0;
            while(1)
            {
                if(buf_decoded_bytes > PLAYBACK_FILE_BUFFER_SIZE)
                {
                    am_util_stdio_printf("\n     ****ERROR**** Playback buffer decode pointer out of bounds\r\n");
                    TEST_FAIL();
                    while(1);
                }
                isamples = mp3dec_decode_frame(psMP3d, file_buf + buf_decoded_bytes, ui32BytesInBuf, pcm, &sinfo);
                if(isamples != MP3_VALID_DECODE_FRAME_SAMPLES)
                    ui32DecodeFails++;
                am_util_stdio_printf("PlaybackTask        - decoded samples: %d  frame_bytes: %d  in buf: %d total decode fails: %d unity failed: %d\r\n", isamples, sinfo.frame_bytes, ui32BytesInBuf, ui32DecodeFails, Unity.CurrentTestFailed);
                TEST_ASSERT_EQUAL(MP3_VALID_DECODE_FRAME_SAMPLES, isamples);

                // Note: I'm not in love with doing a block copy when the
                // low water mark is hit in the playback buffer, but the
                // decoder can only deal with memory sequential buffers
                // and gets upset for the next couple of calls if we point
                // it at a buffer with no valid frames
                if(ui32BytesInBuf < (PLAYBACK_FILE_BUFFER_SIZE / 4)) // low water mark at 1/4 file buffer
                {
                    memmove(file_buf, file_buf + buf_decoded_bytes, ui32BytesInBuf);
                    break;
                }
                ui32BytesInBuf -= sinfo.frame_bytes;
                buf_decoded_bytes += sinfo.frame_bytes;
                // Yield for approx. the amount of time it would take to drain a
                // sample buffer
                vTaskDelay((isamples * 1000) / (44100 * portTICK_PERIOD_MS));
            }
        }
        xSemaphoreGive(g_xFsScrubMutex);
        taskYIELD();
        // if we exited the file read loop due to file error
        if(res != FR_OK)
        {
            am_util_stdio_printf("PlaybackTask        - ERROR during file read\r\n");
            TEST_FAIL();
        }
    }
    am_util_stdio_printf("PlaybackTask        - Suspending Playback task\r\n");
    vPortFree(psMP3d);
    vPortFree(pcm);
    // Indicate that the playback task is complete
    g_bPlaybackTaskComplete = true;
    // Suspend and delete the task
    vTaskDelete(NULL);
}
