//*****************************************************************************
//
//! @file emmc_bm_fatfs.c
//!
//! @brief eMMC bare-metal FatFs example.
//!
//! Purpose: This example demonstrates how to use file system with eMMC device
//! based on the eMMC bare-metal HAL.
//! The following steps are performed..
//! a. FatFS is initialize by mounting a single partition and formatting the media
//! b. a directory tree is created
//! c. within that directory tree files are created
//! d. within the directory tree for each file created reading and writing
//! e. operations are benchmarked using the System Timer
//! f. The directory tree is then scanned and all found directories and files are reported
//! g. a summary of performance metrics along with a count of directories and files found
//!      is reported
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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "mmc_apollo4.h"
#include "ff.h"


#define LOOP_COUNT              2 //! DDR_8bit SDR_8bit

#define OneK                    1024
#define FourK                   4096
#define EightK                  8192
#define SixteenK                16384
#define Thirty32K               32768
#define SixtyFourK              65536
#define One28K                  131072
#define TwoFiftySixK            262144
#define OneMeg                  1048576

// #### INTERNAL BEGIN ####
//
// depth of the directory tree is dependent on how many buffer sizes
// we wish to test.
// depth of 10 was ok for 3 smaller buffer size types.   4,8 & 16k
// 12 was too deep for 3 buffer sizes.
// 10 too deep for 4 buffer sizes (4 to 32k)
// this is due to the amount of SRAM required by the filesystem
//
// #### INTERNAL END ####

#define TREE_DEPTH              6

#define NUM_WRITES              100
#define START_BLK               3000

//! #define DDR_4bit          1
#define DDR_8bit          0
//! #define SDR_4bit          1
#define SDR_8bit          2

// #### INTERNAL BEGIN ####
// DaveC changed in ffconf.h #define FF_FS_RPATH       2
//   1: Enable relative path. f_chdir() and f_chdrive() are available.
//   2: f_getcwd() function is available in addition to 1.
//
// DaveC changed in ffconf.h #define FF_USE_LFN        2
// Enable LFN with dynamic working buffer on the STACK

// #### INTERNAL END ####

#define FMC_BOARD_EMMC_TEST

#define EMMC_DDR50_TEST

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//
// at the moment results under 1ms are not reliable.
//

#define WAKE_INTERVAL_IN_MS     1

#define XT_PERIOD               32768

#define WAKE_INTERVAL           (XT_PERIOD * WAKE_INTERVAL_IN_MS / 1000)

// #### INTERNAL BEGIN ####
//
// there is some discussion as to whether we continue support for a FPGA platform
// for this example
//
// #### INTERNAL END ####

#ifdef APOLLO4_FPGA
// The FPGA XT appears to run at about 1.5MHz (1.5M / 32K = 48).
#undef  WAKE_INTERVAL
#define WAKE_INTERVAL           (XT_PERIOD * WAKE_INTERVAL_IN_MS / 1000 * 48)
#endif

// #### INTERNAL BEGIN ####

// if we enable console output is very verbose
// of course this depends on AM_DEBUG_PRINTF being define
//
// #### INTERNAL END ####

// #define EMMC_DEBUG_PRINT


int memory_index = 0; //! moved here due to compiler optimizations if variable stays stack based
int loop_index;  //! also moved here due to IAR optimization

uint32_t g_ui32TimerTickBefore;
uint32_t g_ui32TimerTickAfter;

uint32_t NumberOfBlocks = 0;

uint32_t g_ui32Count = 0;  //! for System Timer interrupt counter

uint8_t DDR8_flag = 0;  // DDR or SDR?

uint8_t ui8TxRxDelays[2]; //! Tx and Rx delays that are the result of DDR based calibration

FRESULT res;                                  //! Filesystem function return code
FATFS eMMCFatFs;                              //! Filesystem object
FIL TestFile;                                 //! File object
char eMMCPath[4];                             //! eMMC logical drive path
uint8_t Mbytes_per_second_write_list[60];     //! list of performance metrics for each write operation
uint8_t Mbytes_per_second_read_list[60];      //! list of performance metrics for each read operation
int num_of_dirs = 0;                          //! the number of directories found when scanning the filesystem
int num_of_files = 0;                         //! the number of files found when scanning the filesystem
FILINFO fno;                                  //! handle for file access for FatFS
DIR     dir;                                  //! handle for directory access for FatFS
TCHAR  cwd[32];                              //! array to hold the name of a directory name from a f_getcwd call

uint8_t work_buf[FF_MAX_SS] AM_BIT_ALIGNED(128);              //! working buffer for format process

uint8_t read_buf[Thirty32K] AM_BIT_ALIGNED(128);             //! buffer for file read
uint8_t write_buf[Thirty32K] AM_BIT_ALIGNED(128);            //! buffer for file write

uint32_t write_cnt;
uint32_t read_cnt;            //! numbers of bytes to write and read



//*****************************************************************************
//
//! @brief Check whether read data matches with write data.
//!
//! @param rdbuf ptr to read buffer
//!
//! @param wrbuf ptr to write buffer
//!
//! @param len lenght of data to be compared
//!
//! @return none
//
//*****************************************************************************

void
check_if_rwbuf_match(uint8_t *rdbuf, uint8_t *wrbuf, uint32_t len)
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
#ifdef EMMC_DEBUG_PRINT
        am_util_stdio_printf("\nRead write data matched!\neMMC FatFs data verification passed\n");
#endif
    }
}


//*****************************************************************************
//
//! @brief scans a directory tree and reports all directories and subdirectories
//! along with all files found in the directory tree
//!
//! @param path the path for the top level directory
//!
//! @return result of filesystem access
//
//*****************************************************************************

FRESULT
scan_files(char *path)
{
    FRESULT res;
    FILINFO fno;
    DIR     dir;
    int32_t i = 0;
    char* path_final;

    res = f_opendir(&dir, path);                        //! at 28 levels deep this fails
    if ( res == FR_OK )
    {
        for ( ; ; )
        {
            res = f_readdir(&dir, &fno);                   //! Read a directory item
            if ( res != FR_OK || fno.fname[0] == 0 )
            {
                break;  //! Break on error or end of dir
            }
            if ( fno.fattrib & AM_DIR )
            {                                             //! it is a directory
                num_of_dirs++;
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                path_final = path + 1;                      //! dropping first back slash
                am_util_stdio_printf("\n scanning directory tree and current directory  is %s \n", path_final);
                res = scan_files(path);                     //! Enter the directory
                if ( res != FR_OK )
                {
                    break;
                }
                path[i] = 0;
            }
            else
            {                                             //! It is a file;
                path_final = path + 1;
                am_util_stdio_printf("\n scanning directory tree and file is found, directory is  %s\n", path_final);
                am_util_stdio_printf("filename is %s \n", fno.fname);
                num_of_files++;
            }
        }
        f_closedir(&dir);
    }

    return res;
}



//*****************************************************************************
//
//! @brief creates a file and measures write and read times for the file
//!
//! @param dirname the path defining where the file to be created will reside
//!
//! @param index for storing write and read performance numbers
//!
//! @param NumBlks number of blocks to be written or read for each write or
//! read operation
//!
//! @return None
//
//*****************************************************************************
FRESULT
file_create(char* dirname, int index, uint32_t NumBlks)
{
    int i, j;
    char local_file[32];
    uint32_t file_size, volume_data_written;
    uint32_t Bytes_per_ms;
    uint32_t num_of_ms;
    uint32_t volume_data_read;
    uint32_t Mbytes_per_second_write;
    uint32_t Mbytes_per_second_read;

    memset(local_file, 0, sizeof(local_file));
    strcpy (local_file, dirname);

    for ( i = 0, j = 0; local_file[i] != '\0'; ++i )
    {
      if ( local_file[i] != 0x2F )  //! Ascii value for '/'
      {
         local_file[j] = local_file[i];
         local_file[j] = toupper(local_file[j]);   //! want upper case in path name
         ++j;
      }
    }

    local_file[j] = '\0'; // need terminator!!
#ifdef EMMC_DEBUG_PRINT
    am_util_stdio_printf("\n now creating/writing & then reading back to verify contents of files \n");
#endif
    res = f_open(&TestFile, local_file, FA_CREATE_ALWAYS | FA_WRITE );

    if ( res != FR_OK )
    {
        am_util_stdio_printf("\nProblem opening file, returning\n");
        return(res);
    }

    // prep the read and write buffers

    for ( i = 0; i < (NumBlks * 512); i++ )
    {
        write_buf[i] = rand() % 256;
        read_buf[i] = 0;
    }

    file_size = NumBlks*512;

#ifdef EMMC_DEBUG_PRINT
    am_util_stdio_printf("\nFile %s is opened for write in directory %s, writing %d bytes %d times\n", local_file, dirname, file_size, NUM_WRITES);
#endif
    // Calculating total M
    g_ui32TimerTickBefore = g_ui32Count;

    for ( j = 0; j < NUM_WRITES; j++ )    // do a series of writes
    {
        res = f_write(&TestFile, write_buf, file_size, (void *)&write_cnt);
        if ( (res != FR_OK) || (write_cnt == 0) )
        {
            am_util_stdio_printf("File Write Error!\n");
            return(res);
        }
    }

    res = f_close(&TestFile); //sync/flush did not work, have to close and then reopen for reading later
    if ( res != FR_OK )
    {
       am_util_stdio_printf("closing of file for writing failed, filename is %s \n", local_file, dirname);
       return(res);
    }

// Tried capturing timer tick before the close/flush but results did not vary much vs after the event

    g_ui32TimerTickAfter = g_ui32Count;

    volume_data_written = file_size * NUM_WRITES;
    num_of_ms = g_ui32TimerTickAfter - g_ui32TimerTickBefore;

#ifdef EMMC_DEBUG_PRINT
    am_util_stdio_printf("\n Milliseconds needed to write file %s,  %d times with total bytes written of %d at location %s is %d\n", local_file, NUM_WRITES, volume_data_written, dirname, num_of_ms);
#endif

    Bytes_per_ms = volume_data_written / num_of_ms; // Bytes per MS
    Mbytes_per_second_write = Bytes_per_ms / 1000;

#ifdef EMMC_DEBUG_PRINT
    am_util_stdio_printf("\nMegaBytes per second for disk writes of %d, total is %d bytes  is...%d\n", file_size, volume_data_written, Mbytes_per_second_write);
#endif

    Mbytes_per_second_write_list[index] = Mbytes_per_second_write;

    res = f_open(&TestFile, local_file, FA_READ );
    if ( res != FR_OK )
    {
        am_util_stdio_printf("\nfile open error when preparing to read file\n");
        return(res);
    }

#ifdef EMMC_DEBUG_PRINT
    am_util_stdio_printf("\nFile %s now being read in directory %s, reading %d bytes %d times \n", local_file, dirname, file_size, NUM_WRITES);
 #endif

    g_ui32TimerTickBefore = g_ui32Count;  // for read benchmarking

    for ( j = 0; j < NUM_WRITES; j++ )
    {
        res = f_read(&TestFile, read_buf, file_size, (UINT*)&read_cnt);
        if ( (res != FR_OK) || (read_cnt == 0) )
        {
            am_util_stdio_printf("File Read Error!\n");
            return(res);
        }
    }

    g_ui32TimerTickAfter = g_ui32Count;

#ifdef EMMC_DEBUG_PRINT
    am_util_stdio_printf("\n just finished reading file in file_create\n");
#endif

    volume_data_read = file_size * NUM_WRITES;
    num_of_ms = g_ui32TimerTickAfter - g_ui32TimerTickBefore;

#ifdef EMMC_DEBUG_PRINT
    am_util_stdio_printf("\n Milliseconds needed to read file %s,  %d times with total bytes read of %d at location %s is %d\n", local_file, NUM_WRITES, volume_data_read, dirname, num_of_ms);
#endif

    Bytes_per_ms = volume_data_read / num_of_ms;  // Bytes per MS
    Mbytes_per_second_read = Bytes_per_ms / 1000;

#ifdef EMMC_DEBUG_PRINT
    am_util_stdio_printf("\nMegaBytes per second for disk reads of %d, total is %d bytes  is...%d\n", file_size, volume_data_read, Mbytes_per_second_read);
#endif

    Mbytes_per_second_read_list[index] = Mbytes_per_second_read;  // save for later

  //
  // Check whether read data match with write data.
  //
    check_if_rwbuf_match(read_buf, write_buf, file_size);

    Mbytes_per_second_read_list[index] = Mbytes_per_second_read;

    res = f_close(&TestFile);
    if ( res != FR_OK )
    {
        am_util_stdio_printf("closing of file after reading failed, filename is %s \n", local_file);
        return(res);
    }

    memset(cwd, 0, sizeof(cwd));    // clear before using
    res = f_getcwd(cwd, sizeof(cwd));
    if ( res != FR_OK )
    {
        am_util_stdio_printf("getcwd Error!\n");
        return(res);
    }
    else
    {

#ifdef EMMC_DEBUG_PRINT
        am_util_stdio_printf("\nclosing file %s at location %s\n", local_file, dirname);
#endif
    }

    return(FR_OK);

}

//*****************************************************************************
//
//! @brief Swapping Routine, first parameter to be swapped with the 2nd parameter
//! used by bubble_sort function
//!
//! @param xp ptr to first parameter to be swapped
//!
//! @param yp ptr to second parameter to be swapped
//!
//! @return None
//
//*****************************************************************************

void
swap(uint8_t *xp, uint8_t *yp)
{
    uint32_t temp = *xp;
    *xp = *yp;
    *yp = temp;
}

//*****************************************************************************
//
//! @brief Sorting function
//!
//! @param array[] -ptr to array of numbers to be sorted
//!
//! @param n - number of elements to be sorted
//!
//! @return None
//
//*****************************************************************************

void
bubble_sort(uint8_t array[], int n)
{
   int i, j;

   for ( i = 0; i < ( n - 1); i++ )
   {
       for ( j = 0; j < ( n - i ) - 1; j++ )
       {
           if ( array[j] > array[j + 1] )
           {
              swap(&array[j], &array[j + 1]);
           }
       }
   }
}

//*****************************************************************************
//
//! @brief Calibration function for DDR50 Memory
//!
//! @param  ui8TxRxDelays - array that holds delay values
//!
//! @return false to force the calibration
//
//*****************************************************************************

bool
custom_load_ddr50_calib(uint8_t ui8TxRxDelays[2])
{
    //
    // Load ui8TxRxDelays from the somewhere - for example non-volatile memory ...
    // here simply hardcoding these values.
    //
    ui8TxRxDelays[0] = 9;
    ui8TxRxDelays[1] = 6;

    //
    // Return true not to force the calibration
    //
    return false;
}



//*****************************************************************************
//
//! @brief Init function for Timer A0.
//!
//! @param none
//!
//! @return none
//
//*****************************************************************************

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
    NVIC_SetPriority(STIMER_CMPR0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(STIMER_CMPR0_IRQn);

    //
    // Configure the STIMER and run
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ |
                         AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);

}


//*****************************************************************************
//
//! @brief ISR for System Timer
//!
//! @param none
//!
//! @return none
//
//*****************************************************************************
void
am_stimer_cmpr0_isr(void)
{
    //
    // Check the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);

    g_ui32Count++;

}



//*****************************************************************************
//
//! @brief Main entry point for Filesystem Test Example, here the following steps
//! are performed or initiated.
//! a. Initial hardware configuration, including memory configuration, SDR vs DDR
//! b. eMMC card is found and initialized
//! c. FatFS is initialize by mounting a single partition and formatting the media
//! d. a directory tree is created
//! e. within that directory tree files are created
//! f. benchmarks collected for each file created regarding reading and writing
//! g. directory is scanned and all found directories and files are reported
//! h. a summary of performance metrics along with a count of directories and files found
//!      is reported
//!
//! @param none
//!
//! @return none
//
//*****************************************************************************

int
main(void)
{
    char disk_str_num[32];
    int count;
    int i = 0;
    uint32_t k;
    bool bValid;
    uint32_t Buffer_Length;
    uint32_t Number_of_Blocks[5] = {8, 16, 32, 64}; //! from 4k to 32k byte buffers

    uint32_t SDR_Or_DDR[2] = {DDR_8bit, SDR_8bit};  //! only allowing DDR 8 bit and SDR 8 bit
    char *Memory_Type[2] = {"DDR8bit", "SDR8bit"};  //! DDR 8 bit only, SDR 8 bit only
    int buffer_index;

    TCHAR  location[32];
    char buff[32];
    am_hal_card_host_t *pSdhcCardHost = NULL;
    uint32_t MemoryTestType;
    uint32_t FileCreateIndex = 0;

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(AM_HAL_HOST_BUS_WIDTH_8);

#ifdef FMC_BOARD_EMMC_TEST
    //
    // FPGA level shift control
    //
    am_hal_gpio_pinconfig(76, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(76);
#endif

    //
    // Enable printing to the console.
    //
    am_bsp_itm_printf_enable();

    am_util_stdio_terminal_clear();

    //
    // STIMER init.
    //
    stimer_init();

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Print the banner.
    //
    am_util_stdio_printf("\nApollo4 eMMC FAT32 FileSystem test\n");

    for ( loop_index = 0; loop_index < LOOP_COUNT; loop_index++ )   // 2 different memory types
    {
        for ( k = 0, buffer_index = 0; buffer_index < 4; k++ )       // 4 different buffer sizes
        {
            am_util_stdio_printf("\nMemory type is %s\n", Memory_Type[memory_index]);
            NumberOfBlocks = Number_of_Blocks[buffer_index++];
            Buffer_Length = NumberOfBlocks * 512;
            am_util_stdio_printf("\nbuffer size is %d \n", Buffer_Length);

            MemoryTestType = SDR_Or_DDR[memory_index];

            if ( ( MemoryTestType == DDR_8bit ) && ( !( DDR8_flag )) )  // only do once
            {
                bValid = custom_load_ddr50_calib(ui8TxRxDelays);
                if ( bValid )
                {
                    am_hal_card_host_set_txrx_delay(pSdhcCardHost, ui8TxRxDelays);
                }
                else
                {
                    am_hal_card_emmc_calibrate(AM_HAL_HOST_UHS_DDR50, 48000000, AM_HAL_HOST_BUS_WIDTH_8,
                                               (uint8_t *)write_buf, START_BLK, 2, ui8TxRxDelays);

                    am_util_stdio_printf("SDIO TX delay - %d, RX Delay - %d\n", ui8TxRxDelays[0], ui8TxRxDelays[1]);
                }

                DDR8_flag++;
            }

            res = f_mount(&eMMCFatFs, (TCHAR const*)eMMCPath, 0);
            if ( res == FR_OK )
            {
                am_util_stdio_printf("\nFatFs is initialized\n");

                res = f_mkfs((TCHAR const*)eMMCPath, 0, work_buf, sizeof(work_buf));
                if ( res == FR_OK )
                {
                    am_util_stdio_printf("\nformatted the eMMC to FAT32\n");

                    // now build the directory tree

                    g_ui32TimerTickBefore = g_ui32Count;

                    for ( i = 0; i < TREE_DEPTH; i++ )
                    {
                        memset(disk_str_num, 0, sizeof(disk_str_num));   // clear it out first
                        if ( i == 0 )    // first time
                        {
                            sprintf(disk_str_num, "%s", (const char *)"/TOP");
                        }
                        else
                        {
                            sprintf(disk_str_num, "%d", i);
                        }
                        res = f_mkdir( disk_str_num );
                        if ( res != FR_OK )
                        {
                            am_util_stdio_printf("mkdir Error!\n");
                            break;
                        }

                        res = f_chdir(disk_str_num);
                        if ( res != FR_OK )
                        {
                            am_util_stdio_printf("chdir Error!\n");
                            break;
                        }
                        res = f_getcwd(cwd, sizeof(cwd));
                        if ( res != FR_OK )
                        {
                            am_util_stdio_printf("getcwd Error!\n");
                            break;
                        }
                    }    // end for loop

                    g_ui32TimerTickAfter = g_ui32Count;
#ifdef EMMC_DEBUG_PRINT
                    am_util_stdio_printf("\ntime to build directory tree %d layers deep is %d ms \n", TREE_DEPTH, (g_ui32TimerTickAfter - g_ui32TimerTickBefore));
#endif

                    am_util_stdio_printf("\n done building directory tree %d layers deep\n", TREE_DEPTH);

                    memset(location, 0, sizeof(location));   // clear before using
                    // now we revisit the directree tree and populate it with files and gather performance metrics

                    for ( i = 0; i < TREE_DEPTH; i++ )
                    {
                        memset(disk_str_num, 0, sizeof(disk_str_num));  // clear buffer
                        if ( i == 0 )  // first time, directory name is hardwired
                        {
                            sprintf(location, "%s", (const char *)"/TOP");
                        }
                        else
                        {
                            sprintf(disk_str_num, "%d", i);
                            count = strlen(location);
                            location[count] = 0x2f;    // adding trailing slash
                            strcat(location, disk_str_num);
                        }

                        res = f_chdir(location);
                        if ( res != FR_OK )
                        {
                            am_util_stdio_printf("\nchdir error before call to file_create\n");
                        }
                        memset(cwd, 0, sizeof(cwd));   // clear cwd before using
                        res = f_getcwd(cwd, sizeof(cwd));
                        if ( res != FR_OK )
                        {
                            am_util_stdio_printf("getcwd Error!\n");
                        }
                        // this printf fails at SDR8bit, buffer size of 64k, 128 blocks, at /TOP/1
                        am_util_stdio_printf("\n current location right before  file_creation is %s \n", cwd);
                        //
                        // file creation and the index is used for the table of read/write performance numbers
                        //
                        res = file_create(location, FileCreateIndex++, NumberOfBlocks);
                        if ( res != FR_OK)
                        {
                            am_util_stdio_printf("\nfailure in file creation and benchmarking\n");
                        }

                        memset(cwd, 0, sizeof(cwd));   // clear cwd before using
                        res = f_getcwd(cwd, sizeof(cwd));
                        if ( res != FR_OK )
                        {
                            am_util_stdio_printf("getcwd Error!\n");
                        }

#ifdef EMMC_DEBUG_PRINT
                        am_util_stdio_printf("\n just exited file creation and cwd is %s\n", cwd);
#endif

                    }    // loop ends here
                }
                else
                {
                    am_util_stdio_printf("FatFs Format Fail\n");
                }
            }
            else   // mount failed
            {
                am_util_stdio_printf("FatFs Initialization Fail\n");
            }

            memset(location, 0, sizeof(location));   // clear buffer
            //
            // got all buffer sizes now bump index for memory types.
            // done here to get around IAR optimization/issue
            //

            if ( buffer_index == 4 )    //
            {
                memory_index++;
            }

        }  // end for loop for 4 buffer sizes

    }  // end big loop of 2 memory types

    memset(buff, 0, sizeof(buff));  // clear before setting
    strcpy(buff, "/");              // start at the top
    res = scan_files(buff);
    if ( res != FR_OK )
    {
        am_util_stdio_printf("\n error scanning directories\n");
    }

    bubble_sort(Mbytes_per_second_read_list, FileCreateIndex);
    bubble_sort(Mbytes_per_second_write_list, FileCreateIndex);


    am_util_stdio_printf("\n\n\n*************************************************************************\n");
    am_util_stdio_printf("                   EXEC SUMMARY\n");
    am_util_stdio_printf("\n Directory depth is %d,  number of files created, one per directory, is %d\n", num_of_dirs, num_of_files);
    am_util_stdio_printf("\n File Read performance files ranged from %d Mbps to %d Mbps\n", Mbytes_per_second_read_list[0], Mbytes_per_second_read_list[FileCreateIndex - 1]);
    am_util_stdio_printf("\n File Write performance files ranged from %d Mbps to %d Mbps\n", Mbytes_per_second_write_list[0], Mbytes_per_second_write_list[FileCreateIndex - 1]);
    // End banner.
    //
    am_util_stdio_printf("\nApollo4 eMMC Filesystem Example complete\n");

    while(1);
}

