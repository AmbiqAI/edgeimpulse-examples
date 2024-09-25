//*****************************************************************************
//
//! @file emmc_bm_fatfs.c
//!
//! @brief eMMC bare-metal FatFs example.
//!
//! Purpose: This example demonstrates how to use file system with eMMC device
//! based on the eMMC bare-metal HAL.
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//!
//! AM_DEBUG_PRINTF
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//!
//! Note that when these macros are defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.
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
#include "mmc_apollo5.h"

#define SDIO_TEST_MODULE    0
#if SDIO_TEST_MODULE
#define SDIO_HOST   AM_HAL_SDHC_CARD_HOST1
#else
#define SDIO_HOST   AM_HAL_SDHC_CARD_HOST
#endif

#define ALIGN(x) __attribute__((aligned(x)))

#define BUF_LEN 512*20

#ifdef EMMC_DEEP_SLEEP_TEST
#define GPIO_INDICATOR          13
#define SDIO_EXAMPLE_LOOP_NUM   0x8

#define WAKE_INTERVAL_1S        (32768 * 1000 / 1000)
#define WAKE_INTERVAL_2S        (32768 * 2000 / 1000)
#define WAKE_INTERVAL_5S        (32768 * 5000 / 1000)

void
stimer_init(void)
{
    //
    // Enable compare A interrupt in STIMER
    //
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA);
    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(STIMER_CMPR0_IRQn, 3);
    NVIC_EnableIRQ(STIMER_CMPR0_IRQn);
    //
    // Configure the STIMER and run
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL_1S);
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ |
                         AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);
}

void
am_stimer_cmpr0_isr(void)
{
    //
    // Check the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL_1S);
}
#endif

extern am_hal_card_t eMMC_FatFs[AM_REG_SDIO_NUM_MODULES];

FRESULT res;      /* File function return code */
AM_SHARED_RW FATFS eMMCFatFs ALIGN(32);  /* Filesystem object */
AM_SHARED_RW FIL TestFile ALIGN(32);     /* File object */
AM_SHARED_RW char eMMCPath[4] ALIGN(32); /* eMMC logical drive path */
AM_SHARED_RW uint8_t work_buf[FF_MAX_SS] ALIGN(32); /* working buffer for format process */
AM_SHARED_RW uint8_t read_buf[BUF_LEN] ALIGN(32); /* buffer for file read */
AM_SHARED_RW uint8_t write_buf[BUF_LEN] ALIGN(32) = "This is the content insided the TXT\nApollo5 eMMC File System Example!!!\n"; /* buffer for file write */
uint32_t write_cnt, read_cnt;  /* numbers of bytes to write and read */

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
#ifdef EMMC_DEEP_SLEEP_TEST
        am_hal_gpio_output_set(GPIO_INDICATOR);
        am_hal_gpio_output_clear(GPIO_INDICATOR);
#endif

        am_util_stdio_printf("\nRead write data matched!\neMMC FatFs Test Pass\n");
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
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

#ifdef EMMC_DEEP_SLEEP_TEST
    am_hal_gpio_pinconfig(GPIO_INDICATOR, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_set(GPIO_INDICATOR);
    am_hal_gpio_output_clear(GPIO_INDICATOR);
#endif

    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(SDIO_TEST_MODULE, AM_HAL_HOST_BUS_WIDTH_8);

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

#ifndef EMMC_DEEP_SLEEP_TEST
    //
    // Enable printing to the console.
    //
    am_bsp_itm_printf_enable();
#endif

    //
    // Print the banner.
    //
    am_util_stdio_printf("\nApollo5 eMMC:%d FatFs example\n", SDIO_TEST_MODULE);

    //
    // Config SDIO instance, speed, clcok, buswidth, and UHSmode.
    //
    g_sEMMC_hw.pDevHandle = &eMMC_FatFs[SDIO_TEST_MODULE];
    g_sEMMC_config.eHost = SDIO_HOST;
    g_sEMMC_config.ui32Clock = 48000000;
    g_sEMMC_config.eBusWidth = AM_HAL_HOST_BUS_WIDTH_8;
    g_sEMMC_config.eUHSMode = AM_HAL_HOST_UHS_SDR50;

#ifndef EMMC_DEEP_SLEEP_TEST
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
#endif

#ifdef EMMC_DEEP_SLEEP_TEST
    res = f_mount(&eMMCFatFs, (TCHAR const*)eMMCPath, 0);
    if ( res == FR_OK )
    {
        am_util_stdio_printf("\nFatFs is initialized\n");

        res = f_mkfs((TCHAR const*)eMMCPath, 0, work_buf, sizeof(work_buf));
        if ( res == FR_OK )
        {
            am_util_stdio_printf("\nVolume is created on the logical drive\n");

            stimer_init();

            //
            // Deep sleep for mcu, disable pins for reducing power consumption
            //
            am_bsp_sdio_pins_disable(SDIO_TEST_MODULE, AM_HAL_HOST_BUS_WIDTH_8);
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

            for (uint8_t ui8LoopNum = 0; ui8LoopNum < SDIO_EXAMPLE_LOOP_NUM; ui8LoopNum++)
            {
                am_bsp_sdio_pins_enable(SDIO_TEST_MODULE, AM_HAL_HOST_BUS_WIDTH_8);

                res = f_open(&TestFile, "FatFs.TXT", FA_OPEN_APPEND | FA_WRITE | FA_READ);
                if ( res == FR_OK )
                {
                    am_util_stdio_printf("File FatFs.TXT is opened for write\n");
                    res = f_write(&TestFile, write_buf, sizeof(write_buf), (void *)&write_cnt);
                    if ( (res != FR_OK) || (write_cnt == 0) )
                    {
                        am_util_stdio_printf("File Write Error!\n");
                    }

                    //
                    // Make sure reading operation not fail
                    //
                    res = f_lseek(&TestFile, 0);
                    if (res != FR_OK)
                    {
                        am_util_stdio_printf("F_lseek Error!\n");
                    }

                    res = f_read(&TestFile, read_buf, sizeof(read_buf), (UINT*)&read_cnt);
                    if ( (res != FR_OK) || (read_cnt == 0) )
                    {
                        am_util_stdio_printf("File Read Error!\n");
                    }

                    //
                    // Check whether read data match with write data.
                    //
                    check_if_rwbuf_match(read_buf, write_buf, sizeof(write_buf));
                }
                else
                {
                    am_util_stdio_printf("Fail to open file for write\n");
                }

                res = f_close(&TestFile);
                if ( res != FR_OK )
                {
                    am_util_stdio_printf("Fail to close file\n");
                }

                am_bsp_sdio_pins_disable(SDIO_TEST_MODULE, AM_HAL_HOST_BUS_WIDTH_8);
                am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
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
#endif

    //
    // End banner.
    //
    am_util_stdio_printf("\nApollo5 eMMC:%d FatFs example complete\n", SDIO_TEST_MODULE);

    while (1);
}

