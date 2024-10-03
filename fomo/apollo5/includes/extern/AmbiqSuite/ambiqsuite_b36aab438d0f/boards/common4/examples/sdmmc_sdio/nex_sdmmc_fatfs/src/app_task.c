//*****************************************************************************
//
//! @file app_task.c
//!
//! @brief FatFs example based on NEX stack.
//!
//! This example This example demonstrates how to use FatFs based on NEX stack.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "nex_api.h"

#include "am_bsp.h"
#include "am_mcu_apollo.h"
#include "am_util.h"

#include "mmc_apollo4.h"

TaskHandle_t app_task_handle;   /* task handle */

FRESULT res;      /* File function return code */
FATFS eMMCFatFs;  /* Filesystem object */
FIL TestFile;     /* File object */
char eMMCPath[4]; /* eMMC logical drive path */
static uint8_t work_buf[FF_MAX_SS]; /* working buffer for format process */
static uint8_t read_buf[256]; /* buffer for file read */
static uint8_t write_buf[256] = "This is the content insided the TXT\nApollo4 eMMC File System Example!!!\n"; /* buffer for file write */
static uint32_t write_cnt, read_cnt;  /* numbers of bytes to write and read */

//*****************************************************************************
//
// Interrupt handler for sdio.
//
//*****************************************************************************
void am_sdio_isr(void)
{
    nex_irqhandler(SDHC, SLOT);
}

//*****************************************************************************
//
// Check whether read data match with write data.
//
//*****************************************************************************
static void check_if_rwbuf_match(uint8_t *rdbuf, uint8_t *wrbuf, uint32_t len)
{
    int i;
    for ( i = 0; i < len; i++ )
    {
        if (*(wrbuf + i) != *(rdbuf + i))
        {
          am_util_stdio_printf("Test Fail: read and write buffer does not match from %d\n", i);
        break;
        }
    }

    if (i == len)
    {
        am_util_stdio_printf("\nRead write data matched!\neMMC FatFs Test Pass\n");
    }
}

//*****************************************************************************
//
// FatFs task.
//
//*****************************************************************************
void app_sdmmc_fatfs_task(void *pvParameters)
{
    am_util_stdio_printf("\nFatFs Test Start...\n");

    res = f_mount(&eMMCFatFs, (TCHAR const*)eMMCPath, 0);
    if ( res == FR_OK )
    {
      am_util_stdio_printf("\nFatFs is initialized\n");

      res = f_mkfs((TCHAR const*)eMMCPath, 0, work_buf, sizeof(work_buf));
      if ( res == FR_OK )
      {
        am_util_stdio_printf("\nVolume is created on the logical drive\n");
        res = f_open(&TestFile, "FatFs.TXT", FA_CREATE_ALWAYS | FA_WRITE);
        if ( res == FR_OK )
        {
          am_util_stdio_printf("File FatFs.TXT is opened for write\n");
          res = f_write(&TestFile, write_buf, sizeof(write_buf), (void *)&write_cnt);
          if ( (res != FR_OK) || (write_cnt == 0) )
          {
            am_util_stdio_printf("File Write Error!\n");
          }
          else
          {
            res = f_close(&TestFile); //close the file to flush data into device
            if ( res == FR_OK )
            {
              am_util_stdio_printf("File is closed\n");
            }
            else
            {
              am_util_stdio_printf("Fail to close file\n");
            }

            res = f_open(&TestFile, "FatFs.TXT", FA_READ);
            if ( res == FR_OK )
            {
              am_util_stdio_printf("File FatFs.TXT is opened for read\n");
              res = f_read(&TestFile, read_buf, sizeof(read_buf), (UINT*)&read_cnt);
              if ( (res != FR_OK) || (read_cnt == 0) )
              {
                am_util_stdio_printf("File Read Error!\n");
              }
              else
              {
                res = f_close(&TestFile);
                if ( res != FR_OK )
                {
                  am_util_stdio_printf("Fail to close file\n");
                }

                //
                // Check whether read data match with write data.
                //
                check_if_rwbuf_match(read_buf, write_buf, sizeof(write_buf));
              }
            }
            else
            {
              am_util_stdio_printf("Fail to open file for read\n");
            }
          }
        }
        else
        {
          am_util_stdio_printf("Fail to open file for write\n");
        }
      }
      else
      {
        am_util_stdio_printf("FatFs Format Fail\n");
      }

    }
    else
    {
      am_util_stdio_printf("FatFs Initialization Fail\n");
    }

    //
    // End banner.
    //
    am_util_stdio_printf("\nFatFs Test End...\n");

    vTaskSuspend(NULL);
}

