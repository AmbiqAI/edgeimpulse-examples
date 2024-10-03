//*****************************************************************************
//
//! @file emmc_bm_fatfs_benchmark_test_cases.c
//!
//! @brief eMMC FatFS benchmark test case.
//!
//! Purpose: This example does the file system benchmark test on the eMMC device
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
#include "unity.h"
#include "sdio_test_common.h"


#define LOOP_COUNT        2 //! SDR_8bit,DDR_8bit

#define OneK                    1024
#define FourK                   4096
#define EightK                  8192
#define SixteenK                16384
#define Thirty32K               32768
#define SixtyFourK              65536
#define One28K                  131072
#define TwoFiftySixK            262144
#define OneMeg                  1048576
#define TREE_DEPTH              5  //! above 5 is an issue, exhaust available memory

#define NUM_WRITES      100  //! reduced due to memory constraints
#define START_BLK       3000

// #### INTERNAL BEGIN ####
//
// only supporting 8 bit modes for this test case
// could easily expand to 4 bit
// #define DDR_4bit          1  // only DDR 8 bit
#define DDR_8bit          0
#define SDR_8bit          1   //! only SDR 8 bit
// #define SDR_8bit          2
//

//
// DaveC changed in ffconf.h #define FF_FS_RPATH   2
// DaveC changed in ffconf.h #define FF_USE_LFN	   1
//
// #### INTERNAL END ####

#define FMC_BOARD_EMMC_TEST

// only enable for really verbose console output
// #define EMMC_DEBUG_PRINT

bool bTestPass = true;

uint32_t g_ui32TimerTickBefore;
uint32_t g_ui32TimerTickAfter;


uint32_t NumberOfBlocks = 0; 
int memory_index = 0; //! moved here due to compiler optimizations local variable stays stack based
int loop_index;  //! also moved here due to IAR optimization

uint32_t MemoryTestType;

FRESULT res;                             //! Filesystem  function return code
FATFS eMMCFatFs;                         //! Filesystem object
FIL TestFile;                            //! File object
char eMMCPath[4];                        //! eMMC logical drive path
uint8_t Mbytes_per_second_write_list[60];
uint8_t Mbytes_per_second_read_list[60];
int num_of_dirs = 0,num_of_files = 0;
uint32_t Total_time;
FILINFO fno;
DIR     dir;
TCHAR  cwd[32];

uint8_t work_buf[FF_MAX_SS];          //! working buffer for format process
uint8_t read_buf[One28K];             //! buffer for file read
uint8_t write_buf[One28K];            //! buffer for file write *
uint32_t write_cnt, read_cnt;         //! numbers of bytes to write and read


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
            bTestPass = false;
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
    int32_t i=0;
    char* path_final;

    res = f_opendir(&dir, path);
    if ( res == FR_OK )
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);                   //! Read a directory item
            if ( res != FR_OK || fno.fname[0] == 0 ) break;  //! Break on error or end of dir
            if ( fno.fattrib & AM_DIR ) 
            {                                             //! it is a directory
                num_of_dirs++;
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                path_final = path +1;  //! dropping first back slash

                res = scan_files(path);                    //! Enter the directory
                if ( res != FR_OK ) break;
                path[i] = 0;
            } 
            else 
            {                                            //! It is a file
                path_final = path + 1;
                am_util_stdio_printf("\n scanning directory tree and file is found, directory is  %s\n", path_final);
                am_util_stdio_printf(" filename is %s \n",fno.fname);
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

void
file_create(char* filename, int index, uint32_t NumBlks)
{
  int i,j;
  char local_file[256];
  uint32_t file_size,volume_data_written;
  uint32_t Bytes_per_ms;
  uint32_t num_of_ms;
  uint32_t volume_data_read;
  uint32_t Mbytes_per_second_write;
  uint32_t Mbytes_per_second_read;

    
  strcpy(local_file,filename);

  for ( i=0, j=0; local_file[i] != '\0'; ++i ) 
  {
    if (local_file[i] != 0x2F)                    //! Ascii value for '/'
    {  
       local_file[j] = local_file[i];
       local_file[j] = toupper(local_file[j]);   //! want upper case
       ++j;
    }
  }
  
  local_file[j] = '\0'; //! need terminator 

#ifdef EMMC_DEBUG_PRINT
  am_util_stdio_printf("\n now creating/writing & then reading back to verify contents of files \n");
#endif

  res = f_open(&TestFile, local_file, FA_CREATE_ALWAYS | FA_WRITE );

  if ( res != FR_OK )
  {   
      am_util_stdio_printf("\nProblem opening file, returning\n");
      bTestPass = false;
      return;
  }

  // prep the write and read buffers
   
  for (i = 0; i < (NumBlks*512); i++)
  {
      write_buf[i]=rand() % 256;
      read_buf[i] = 0;
  }
    
  
  file_size = NumBlks*512;

#ifdef EMMC_DEBUG_PRINT
      am_util_stdio_printf("\nFile %s is opened for write in directory %s, writing %d bytes %d times\n",local_file,filename,file_size,NUM_WRITES);
#endif
     
   // Calculating total MS
      g_ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);

      for (j = 0; j < NUM_WRITES; j++)    // do a series of writes
      {
          res = f_write(&TestFile, write_buf, file_size, (void *)&write_cnt);
          if ( (res != FR_OK) || (write_cnt == 0) )
          {
              am_util_stdio_printf("File Write Error!\n");
              bTestPass = false;
          break;
          }
      }
      
      res = f_close(&TestFile); //sync/flush did not work, have to close and then reopen for reading later
      if ( res != FR_OK )
      {
         am_util_stdio_printf("closing of file for writing failed, filename is %s \n",local_file,filename);
         bTestPass = false;
      }

      g_ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);  //! tried before the close/flush, did not really matter much
      
      volume_data_written = file_size * NUM_WRITES;
      num_of_ms = am_widget_sdio_cal_time(g_ui32TimerTickBefore, g_ui32TimerTickAfter);

#ifdef EMMC_DEBUG_PRINT     
      am_util_stdio_printf("\n Milliseconds needed to write file %s,  %d times with total bytes written of %d at location %s is %d\n",local_file,NUM_WRITES,volume_data_written,filename,num_of_ms);
#endif
     
      Bytes_per_ms = volume_data_written/num_of_ms/TIME_DIVIDER;  // Bytes per MS
      Mbytes_per_second_write = Bytes_per_ms;
  
#ifdef EMMC_DEBUG_PRINT
      am_util_stdio_printf("\nMegaBytes per second for disk writes of %d, total is %d bytes  is...%d\n",file_size,volume_data_written,Mbytes_per_second_write);		
#endif 
      Mbytes_per_second_write_list[index] = Mbytes_per_second_write;
             
      res = f_open(&TestFile, local_file, FA_READ );
      if ( res != FR_OK )
      {
 
#ifdef EMMC_DEBUG_PRINT
          am_util_stdio_printf("\nfile open error when preparing to read file\n");
#endif

          bTestPass = false;
          return;
      }

#ifdef EMMC_DEBUG_PRINT      
      am_util_stdio_printf("\nFile %s now being read in directory %s, reading %d bytes %d times \n",local_file,filename,file_size,NUM_WRITES);
#endif
     
      g_ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);  // for read benchmarking
       
      for( j = 0; j < NUM_WRITES; j++ )
      {       
          res = f_read(&TestFile, read_buf, file_size, (UINT*)&read_cnt);
          if ( (res != FR_OK) || (read_cnt == 0) )
          {
              am_util_stdio_printf("File Read Error!\n");
              bTestPass = false;
              return;
          }
      }	
      
      g_ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);

#ifdef EMMC_DEBUG_PRINT
      am_util_stdio_printf("\n just finished reading file in file_create\n");
#endif

      volume_data_read = file_size * NUM_WRITES;
      num_of_ms = am_widget_sdio_cal_time(g_ui32TimerTickBefore, g_ui32TimerTickAfter);

#ifdef EMMC_DEBUG_PRINT     
      am_util_stdio_printf("\n Milliseconds needed to read file %s,  %d times with total bytes read of %d at location %s is %d\n",local_file,NUM_WRITES,volume_data_read,filename,num_of_ms);
#endif
     
      Bytes_per_ms = volume_data_read/num_of_ms/TIME_DIVIDER;  // Bytes per MS
      Mbytes_per_second_read = Bytes_per_ms;

#ifdef EMMC_DEBUG_PRINT	  
      am_util_stdio_printf("\nMegaBytes per second for disk reads of %d, total is %d bytes  is...%d\n",file_size,volume_data_read,Mbytes_per_second_read);
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
          am_util_stdio_printf("closing of file after reading failed, filename is %s \n",local_file);
          bTestPass = false;
          return;
      }

      memset(cwd,0,sizeof(cwd));   // clear before using
      res = f_getcwd(cwd, 256);
      if ( res != FR_OK )
      {
          am_util_stdio_printf("getcwd Error!\n");
          bTestPass = false;
      }
      else
      {  

#ifdef EMMC_DEBUG_PRINT
          am_util_stdio_printf("\nclosing file %s at location %s\n",local_file,filename);
#endif

      } 

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
bubble_sort(uint8_t array[],int n)
{
   int i,j;
   
   for ( i=0; i < (n-1); i++)
   {
       for ( j = 0; j < (n-i)-1; j++)
       {
           if (array[j] > array[j+1])
           {
              swap(&array[j],&array[j+1]);
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
// Optional setup/tear-down functions
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
  
}

void
tearDown(void)
{
 
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

void
fat_fs_emmc_test(void)
{
    char disk_str_num[16];
    int count;
    int i = 0;
    int k;
    int buffer_index = 0;
    bool bValid;
    uint32_t Buffer_Length;
    uint32_t Number_of_Blocks[6] = {8,16,32,64,128,256};  // from 4k to 128k bytes
    uint32_t SDR_Or_DDR[2] = {AM_HAL_HOST_UHS_DDR50, AM_HAL_HOST_UHS_SDR50};  // only allowing DDR 8 bit and SDR 8 bit
    char *Memory_Type[2] = {"DDR8bit","SDR8bit"};
    uint8_t DDR8_Flag = 0;

    TCHAR  location[256];
    char buff[256];
    uint8_t ui8TxRxDelays[2];                           //! for DDR50
    am_hal_card_host_t *pSdhcCardHost = NULL;
    uint32_t FileCreateIndex = 0; 
    
    //
    // Print the banner.
    //
    am_util_stdio_printf("\nApollo4 eMMC FAT32 FileSystem test with multiple Memory types and Buffer Sizes \n");
    

    for ( loop_index = 0; loop_index < LOOP_COUNT; loop_index++ )  // 2 different memory types
    {   
       for ( k=0, buffer_index = 0; buffer_index < 6; k++ )  // 6 different buffer sizes
       {
           am_util_stdio_printf("\nMemory type is %s\n",Memory_Type[memory_index]);
           NumberOfBlocks = Number_of_Blocks[buffer_index++];;
           am_util_stdio_printf("\n number of blocks is %d\n",NumberOfBlocks);
           Buffer_Length = NumberOfBlocks * 512;
           am_util_stdio_printf("\nbuffer size is %d \n",Buffer_Length);
           
           MemoryTestType = SDR_Or_DDR[memory_index];
       
           if ( (MemoryTestType == AM_HAL_HOST_UHS_DDR50) && (!(DDR8_Flag)) )  // only do once
           {
               bValid = custom_load_ddr50_calib(ui8TxRxDelays);
               if (bValid)
               {
                   am_hal_card_host_set_txrx_delay(pSdhcCardHost, ui8TxRxDelays);
               }
               else
               {
                   am_hal_card_emmc_calibrate(AM_HAL_HOST_UHS_DDR50, 48000000, AM_HAL_HOST_BUS_WIDTH_8,   
                    (uint8_t *)write_buf, START_BLK, 2, ui8TxRxDelays);                               

                   am_util_stdio_printf("SDIO TX delay - %d, RX Delay - %d\n", ui8TxRxDelays[0], ui8TxRxDelays[1]);
               }
               
               DDR8_Flag++;
           }

    
           res = f_mount(&eMMCFatFs, (TCHAR const*)eMMCPath, 0);
           if ( res == FR_OK )
           {
   
#ifdef EMMC_DEBUG_PRINT
               am_util_stdio_printf("\nFatFs is initialized\n");
#endif

               res = f_mkfs((TCHAR const*)eMMCPath, 0, work_buf, sizeof(work_buf));
               if ( res == FR_OK )
               {

#ifdef EMMC_DEBUG_PRINT
                   am_util_stdio_printf("\nformatted the eMMC to FAT32\n");
#endif

                   // now build the directory tree below
                   g_ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);
                   for( i = 0; i < TREE_DEPTH; i++ )
                   {
                       if ( i == 0 )    // first time
                       {  
                           sprintf(disk_str_num, "%s", (const char *)"/TOP");  // cause
                       }
                       else
                       {
                           sprintf(disk_str_num, "%d", i);
                       }
                       res = f_mkdir( disk_str_num );
                       if ( res != FR_OK ) 
                       {
                           am_util_stdio_printf("mkdir Error!\n");
                           bTestPass = false;
                       }
             
                       res = f_chdir( disk_str_num );
                       if (res != FR_OK ) 
                       {
                           am_util_stdio_printf("chdir Error!\n");
                           bTestPass = false;
                       }
              
                   }    // end for loop

                   g_ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);
   
#ifdef EMMC_DEBUG_PRINT
                   am_util_stdio_printf("\ntime to build directory tree %d layers deep is %d ms \n",TREE_DEPTH,(g_ui32TimerTickAfter - g_ui32TimerTickBefore));
                   am_util_stdio_printf("\n done building directory tree %d layers deep\n",TREE_DEPTH);
#endif

                   
 // now we revisit the directree tree and populate it with files and gather performance metrics
                  memset(location,0,sizeof(location));   // clear buffer

                  for (i = 0; i < TREE_DEPTH; i++ )
                 {
                     memset(disk_str_num,0,sizeof(disk_str_num));  // clear buffer
                     if ( i == 0 )  // first time, directory name is hardwired
                     {
                         sprintf(location, "%s", (const char *)"/TOP");
                     }
                     else
                     {
                         sprintf(disk_str_num, "%d", i);
                         count = strlen(location);
                         location[count] = 0x2f;    // adding trailing slash
                         strcat(location,disk_str_num);
                     }

                     res = f_chdir(location);
                     if ( res != FR_OK )
                     {
                         am_util_stdio_printf("\nchdir error before call to file_create\n");
                     }
                     memset(cwd,0,sizeof(cwd));   // clear cwd before using
                     res = f_getcwd(cwd, sizeof(cwd));
                     if( res != FR_OK )
                     {
                        am_util_stdio_printf("getcwd Error!\n");
                     }
                    
                     am_util_stdio_printf("\n current location right before  file_creation is %s \n",cwd);
                  //
                  // file creation and the index is used for the table of read/write performance numbers
                  //
                     file_create(location,FileCreateIndex++,NumberOfBlocks);  
                  
                     memset(cwd,0,sizeof(cwd));   // clear cwd before using
                     res = f_getcwd(cwd, sizeof(cwd));
                     if( res != FR_OK )
                     {
                         am_util_stdio_printf("getcwd Error!\n");
                     }
                     
#ifdef EMMC_DEBUG_PRINT
                     am_util_stdio_printf("\n just exited file creation and cwd is %s\n",cwd);
#endif
                     
                 }    // loop ends here
     
            }
            else
            {
                am_util_stdio_printf("FatFs Format Fail\n");
                bTestPass = false;
            }
        
      }
      else   // mount failed
      {
          am_util_stdio_printf("FatFs Initialization Fail\n");
          bTestPass = false;
      }
      
      //
      // got all buffer sizes now bump index for memory types.
      // done here to get around IAR optimization/issue
      //
      if ( k ==5 )
      {
          memory_index++;
      }

    }  // end for loop for 6 buffer sizes
    
 }  // end big loop of 2 memory types
 
 // at this point we want to traverse the tree and verify what was created  

    memset(buff,0,sizeof(buff));  // clear before setting
    strcpy(buff, "/");  // start at the top
    res = scan_files(buff);       
 
    bubble_sort(Mbytes_per_second_read_list,FileCreateIndex);
    bubble_sort(Mbytes_per_second_write_list,FileCreateIndex);
    
   
       
    am_util_stdio_printf("\n\n\n*************************************************************************\n");
    am_util_stdio_printf("                   EXEC SUMMARY\n");
    am_util_stdio_printf("\n Directory depth is %d,  number of files created, one per directory, is %d\n",num_of_dirs,num_of_files);
    am_util_stdio_printf("\n File Read performance files ranged from %d Mbps to %d Mbps\n",Mbytes_per_second_read_list[0],Mbytes_per_second_read_list[FileCreateIndex - 1]);
    am_util_stdio_printf("\n File Write performance files ranged from %d Mbps to %d Mbps\n",Mbytes_per_second_write_list[0],Mbytes_per_second_write_list[FileCreateIndex-1]);
    // End banner.
    //
    am_util_stdio_printf("\nApollo4 eMMC Filesystem Test complete\n");
    TEST_ASSERT_TRUE(bTestPass);


}

