//*****************************************************************************
//
//! @file sd_card_bm_fatfs.c
//!
//! @brief SD Card bare-metal FatFs example.
//!
//! @addtogroup bm_sdmmc_sdio_examples SDIO Examples
//!
//! @defgroup sd_card_bm_fatfs SD Card BM FATFS Example
//! @ingroup bm_sdmmc_sdio_examples
//! @{
//!
//! Purpose: This example demonstrates how to use file system with SD Card device
//! based on the SD Card bare-metal HAL.<br>
//! <br>
//!   1) Initialize for low power<br>
//!   2) Mount Filesystem<br>
//!   3) Create Filesystem using FAT32<br>
//!   4) Open Filesystem<br>
//!   5) Perform Read/Write Test<br>
//!   6) Close Filesystem<br>
//!   7) Unmount Filessytem<br>
//!   8) Sleep Forever
//! <br>
//! Additional Information:
//! Debug messages will be sent over ITM/SWO at 1M Baud.<br>
//! <br>
//! If ITM is not shut down, the device will never achieve deep sleep, only<br>
//! normal sleep, due to the ITM (and thus the HFRC) being enabled.<br>
//
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
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "mmc_apollo4.h"

// #### INTERNAL BEGIN ####
//
// Macro definition for using differernt sd card board
// If the level shifter is txs0206a in board, don't use macro definition
// If the level shifter is NVT4867UK in board, must use macro definition
// The usage of SEL or EN pin should refer to the level shifter spec
//
// #### INTERNAL END ####
#define SD_CARD_BOARD_SUPPORT_1_8_V

FRESULT res;      /* File function return code */
FATFS SDcardFatFs;  /* Filesystem object */
FIL TestFile;     /* File object */
char SDcardPath[4]; /* SD Card logical drive path */
static uint8_t work_buf[FF_MAX_SS]; /* working buffer for format process */
static uint8_t read_buf[256]; /* buffer for file read */
static uint8_t write_buf[256] = "This is the content insided the TXT\nApollo4 SD Card File System Example!!!\n"; /* buffer for file write */
static uint32_t write_cnt, read_cnt;  /* numbers of bytes to write and read */

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
        if (*(wrbuf + i) != *(rdbuf + i) )
        {
            am_util_stdio_printf("Test Fail: read and write buffer does not match from %d\n", i);
            break;
        }
    }

    if (i == len)
    {
        am_util_stdio_printf("\nRead write data matched!\nSD Card FatFs Test Pass\n");
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
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(AM_HAL_HOST_BUS_WIDTH_4);

    //
    // SD card voltage level translator control
    //
#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
    // #### INTERNAL BEGIN ####
    // translator select NVT4867UK.
    // #### INTERNAL END ####
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#else
    // #### INTERNAL BEGIN ####
    // translator select txs0206a.
    // #### INTERNAL END ####
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_set(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#endif

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Enable printing to the console.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_printf("\nApollo4 SD Card FatFs example\n");

    res = f_mount(&SDcardFatFs, (TCHAR const*)SDcardPath, 0);
    if ( res == FR_OK )
    {
        am_util_stdio_printf("\nFatFs is initialized\n");

        res = f_mkfs((TCHAR const*)SDcardPath, 0, work_buf, sizeof(work_buf));
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
    am_util_stdio_printf("\nApollo4 SD Card FatFs example complete\n");

    //
    // Sleep forever
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
