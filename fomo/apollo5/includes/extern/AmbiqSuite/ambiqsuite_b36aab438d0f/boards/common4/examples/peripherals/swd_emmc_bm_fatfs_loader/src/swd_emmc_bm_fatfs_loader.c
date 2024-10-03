//*****************************************************************************
//
//! @file swd_emmc_bm_fatfs_loader.c
//!
//! @brief load file to eMMC bare-metal FatFs.
//!
//! @addtogroup peripheral_examples Peripheral Examples
//
//! @defgroup swd_emmc_bm_fatfs_loader SWD eMMC FATFS Loader Example
//! @ingroup peripheral_examples
//! @{
//!
//! Purpose: Example demonstrating how to load file to EMMC via SWD
//!
//! Steps are as follows:
//!  1. (Compile this project) Download and run swd_emmc_bm_fatfs_loader.bin to Apollo4,
//!     check the SWO log, file downloading is allowed after "Ready to load file" printed
//!  2. Then there are two ways to download files
//!   i. Download single file
//!      python .\swd_emmc_bm_fatfs_loader.py -f C:\load_file_to_emmc_bm_fatfs\TH_BG_454.argb8888
//!   ii. Download the whole folder of files
//!      python .\swd_emmc_bm_fatfs_loader.py -d C:\load_file_to_emmc_bm_fatfs\
//!   the filelist of emmc will be printed out after all downloadings complete
//!  3. Download other application binary like lvgl_watch_demo or lvgl_widget_examples
//!
//! Note:
//!  The files will be loaded to address FILE_LOADER_BASE, and control bitfields are placed at
//!  LOADER_CTRL_BASEADDR, so be careful not to overwrite these memory in this program
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
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "mmc_apollo4.h"

#define CHECK_LOAD_FILE     0
//
// deletes a name from the filesystem.
//
#define UNLINK_FILE         0
//
// deletes file name
//
#define UNLINK_FILE_NAME        "hello_world.bin"

//*****************************************************************************
//
// Register address.
//
//*****************************************************************************
#define LOADER_CTRL_BASEADDR            (0x100DF800UL)
#define LOADER_CTRL_SIZE                (0x200)

#define FILE_INFO_BASEADDR              (LOADER_CTRL_BASEADDR + LOADER_CTRL_SIZE)   //0x100DFA00UL
#define FILE_INFO_SIZE                  (0x600)

#define FILE_LOADER_BASE                (FILE_INFO_BASEADDR + FILE_INFO_SIZE)       //0x100E0000UL
#define FILE_LOADER_SIZE                (0x80000)

/* =========================================================================== */
/* ================                 LOAD CTRL                 ================ */
/* =========================================================================== */
typedef struct
{
    volatile uint32_t   mode;
    volatile uint32_t   status;
    volatile uint32_t   number_bytes;   //! Number of bytes to transfer
    volatile uint32_t   address;        //! Transfer Address

}loader_ctrl_type;

#define loader_ctrl             ((loader_ctrl_type*)           LOADER_CTRL_BASEADDR)
/* =================================  MODE  ================================= */
#define FILE_LOADER_MODE_IDLE           0x00L
#define FILE_LOADER_MODE_READY          0x01L

#define FILE_LOADER_MODE_LOAD           0x02L
#define FILE_LOADER_MODE_COMPLETE       0x04L
/* ================================  STATUS  ================================ */
#define FILE_LOADER_STATUS_IDLE         0x00L
#define FILE_LOADER_STATUS_BUSY         0x01L
#define FILE_LOADER_STATUS_ERROR        0x02L

/* =========================================================================== */
/* ================                 FILE INFO                 ================ */
/* =========================================================================== */
#define FILE_NAME_LENGTH        128
typedef struct
{
    volatile char       name[FILE_NAME_LENGTH];
    volatile uint32_t   size;
    volatile uint32_t   checksum;
}load_file_info_type;
#define load_file_info          ((load_file_info_type*)           FILE_INFO_BASEADDR)

FRESULT res;                            /* File function return code */
FATFS eMMCFatFs;                        /* Filesystem object */
FIL FatFile;                            /* File object */
char eMMCPath[4];                       /* eMMC logical drive path */
static uint8_t work_buf[FF_MAX_SS];     /* working buffer for format process */
uint8_t read_buf[512];                  /* buffer for file read */

const char * FR_Table[] =
{
    "FR_OK",                            /* (0) Succeeded */
    "FR_DISK_ERR",                      /* (1) A hard error occurred in the low level disk I/O layer */
    "FR_INT_ERR",                       /* (2) Assertion failed */
    "FR_NOT_READY",                     /* (3) The physical drive cannot work */
    "FR_NO_FILE",                       /* (4) Could not find the file */
    "FR_NO_PATH",                       /* (5) Could not find the path */
    "FR_INVALID_NAME",                  /* (6) The path name format is invalid */
    "FR_DENIED",                        /* (7) Access denied due to prohibited access or directory full */
    "FR_EXIST",                         /* (8) Access denied due to prohibited access */
    "FR_INVALID_OBJECT",                /* (9) The file/directory object is invalid */
    "FR_WRITE_PROTECTED",               /* (10) The physical drive is write protected */
    "FR_INVALID_DRIVE",                 /* (11) The logical drive number is invalid */
    "FR_NOT_ENABLED",                   /* (12) The volume has no work area */
    "FR_NO_FILESYSTEM",                 /* (13) There is no valid FAT volume */
    "FR_MKFS_ABORTED",                  /* (14) The f_mkfs() aborted due to any parameter error */
    "FR_TIMEOUT",                       /* (15) Could not get a grant to access the volume within defined period */
    "FR_LOCKED",                        /* (16) The operation is rejected according to the file sharing policy */
    "FR_NOT_ENOUGH_CORE",               /* (17) LFN working buffer could not be allocated */
    "FR_TOO_MANY_OPEN_FILES",           /* (18) Number of open files > _FS_SHARE */
    "FR_INVALID_PARAMETER"              /* (19) Given parameter is invalid */
};

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

    res = f_opendir(&dir, path);
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
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                am_util_stdio_printf("%s\t\t<DIR>\n", fno.fname);
                path_final = path + 1;  //! dropping first back slash

                res = scan_files(path);                    //! Enter the directory
                if ( res != FR_OK )
                {
                    break;
                }
                path[i] = 0;
            }
            else
            {                                            //! It is a file
                path_final = path + 1;
//                am_util_stdio_printf("\n scanning directory tree and file is found, directory is  %s\n", path_final);
                am_util_stdio_printf("%-32s\t\t%lu\n", fno.fname, fno.fsize);
            }
        }
        f_closedir(&dir);
    }
    return res;
}

//*****************************************************************************
//
//! @brief scans a directory tree and reports all directories and subdirectories
//! along with all files found in the directory tree
//!
//! @param path     - the path of the file
//! @param nbytes   - write byte count
//!
//! @return result of filesystem access
//
//*****************************************************************************
#define LOADER_PAGE_SIZE        4096
FRESULT ff_write(FIL * fd, void *buf, uint32_t nbytes)
{
    FRESULT res;
    UINT cnt;
    uint32_t ui32NumBytes;
    uint32_t size;
    uint8_t *pBuffer;
    uint32_t i = 1;

    if (nbytes > FILE_LOADER_SIZE)
    {
        am_util_stdio_printf("Given parameter is invalid!\n");
        res = FR_DISK_ERR;
        return res;
    }
    ui32NumBytes = nbytes;
    pBuffer = (uint8_t*)buf;

    while(ui32NumBytes)
    {
        if (ui32NumBytes > LOADER_PAGE_SIZE)
        {
          size = LOADER_PAGE_SIZE;
        }
        else
        {
          size = ui32NumBytes;
        }
        am_util_debug_printf("ui32NumBytes = %d, size = %d\n", ui32NumBytes,  size);

        res = f_write((FIL *)fd, (const void*) pBuffer, size, (UINT *)&cnt);
        if ( (res != FR_OK) || (cnt == 0) )
        {
            break;
        }
        ui32NumBytes -= size;
        pBuffer += size;

        am_util_stdio_printf(".");
        if (i++ % 32 == 0)
        {
          am_util_stdio_printf("\n");
        }

    }
    am_util_stdio_printf("\n");

    return res;
}

//*****************************************************************************
//
//! @brief Calculate the load file.
//!
//! @param path         - the path of the file
//! @param size         - file size
//! @param ref_checksum - file checksum
//!
//! @return true if checksum is correct, false if checksum is incorrect.
//
//*****************************************************************************
bool check_load_file(char *path, uint32_t size, uint32_t ref_checksum)
{
    FRESULT res;
    bool result = false;
    uint32_t read_cnt;  /* numbers of bytes to write and read */
    uint32_t total_read = 0;

    uint32_t i, checksum = 0;
    am_util_stdio_printf("Please wait for several minutes...\n");

    res = f_open(&FatFile, path, FA_OPEN_EXISTING | FA_READ);
    if (res !=  FR_OK)
    {
        am_util_stdio_printf("Fail to open file for read, %s\n", FR_Table[res]);
        result = false;
        goto _release;
    }

    do
    {
        res = f_read(&FatFile, read_buf, sizeof(read_buf), (UINT*)&read_cnt);
        if (res != FR_OK)
        {
            am_util_stdio_printf("File Read Error! %s\n", FR_Table[res]);
            result = false;
            break;
        }
        for ( i = 0; i < read_cnt; i++ )
        {
            checksum += read_buf[i];
        }
        total_read += read_cnt;
    }while(read_cnt);

    f_close(&FatFile);

    if ((total_read == size) && (ref_checksum == checksum))
    {
        result = true;
    }
    else
    {
        result = false;
    }
    am_util_debug_printf("File checksum is %d, file size is %d\n", checksum, total_read);

_release:
    return result;
}

//*****************************************************************************
//
// MSPI Example Main.
//
//*****************************************************************************
int
main(void)
{
    char path[128];
    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

//    g_ePrintDefault = AM_BSP_PRINT_IF_UART;

    am_bsp_debug_printf_enable();

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
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();
    //
    // Print the banner.
    //
    am_util_stdio_printf("\nApollo4 load binary to eMMC FatFs \n");

    res = f_mount(&eMMCFatFs, (TCHAR const*)eMMCPath, 1);
    if ( res != FR_OK )
    {
        am_util_stdio_printf("\nformatted the eMMC to FAT32\n");
        res = f_mkfs((TCHAR const*)eMMCPath, 0, work_buf, sizeof(work_buf));
        if ( res == FR_OK )
        {
            am_util_stdio_printf("\nVolume is created on the logical drive\n");
            if (f_mount(&eMMCFatFs, (TCHAR const*)eMMCPath, 1) != FR_OK)
            {
                am_util_stdio_printf("FatFs mount Fail!.\n");
                while(1);
            }
        }
        else
        {
            am_util_stdio_printf("FatFs Format Fail!.\n");
            while(1);
        }
    }
    am_util_stdio_printf("\nFatFs is initialized\n");

#if UNLINK_FILE
    char * unlink_file = UNLINK_FILE_NAME;
    sprintf(path, "%s%s", eMMCPath, unlink_file);
    res = f_unlink(path);
    if (res != FR_OK )
    {
        am_util_stdio_printf("removes file %s faile, %s\n", unlink_file, FR_Table[res]);
    }
#endif
    //
    // list directory files
    //
    memset(path, 0, sizeof(path));         // clear before setting
    strcpy(path, "/");                     // start at the top
    am_util_stdio_printf("=> List files in %s\n", path);
    res = scan_files(path);
    if (res != FR_OK)
    {
        am_util_stdio_printf("open directory failed ,%s", FR_Table[res]);
    }

    am_util_stdio_printf("=> Ready to load file\n");
    while(1)
    {
        switch (loader_ctrl->mode)
        {
            case FILE_LOADER_MODE_IDLE:

            break;

            case FILE_LOADER_MODE_READY:
                loader_ctrl->mode = FILE_LOADER_MODE_IDLE;
                loader_ctrl->status = FILE_LOADER_STATUS_BUSY;
                am_util_stdio_printf("FILE_LOADER_MODE_READY start\n");
                am_util_stdio_printf("=> load file info \n");
                am_util_stdio_printf("   load file name is %s:\n", load_file_info->name);
                am_util_stdio_printf("   load file size is %d:\n", load_file_info->size);
                am_util_stdio_printf("   load file checksum is %d:\n", load_file_info->checksum);
                //
                //Creates a new file. If the file is existing, it will be truncated and overwritten.
                //
                res = f_open(&FatFile, (const TCHAR*)load_file_info->name, FA_CREATE_ALWAYS | FA_WRITE);
                if ( res != FR_OK )
                {
                    am_util_stdio_printf("Fail to open file for write, %s\n", FR_Table[res]);
                    loader_ctrl->status = FILE_LOADER_STATUS_ERROR;
                }
                else
                {
                    loader_ctrl->status = FILE_LOADER_STATUS_IDLE;
                }

                am_util_stdio_printf("FILE_LOADER_MODE_READY end\n");

            break;
            case FILE_LOADER_MODE_LOAD:
                loader_ctrl->mode = FILE_LOADER_MODE_IDLE;
                loader_ctrl->status = FILE_LOADER_STATUS_BUSY;
                am_util_stdio_printf("FILE_LOADER_MODE_LOAD start\n");
                am_util_stdio_printf("=> Loading, load bytes size :0x%x \n", loader_ctrl->number_bytes);
                res = ff_write(&FatFile, (void*)(FILE_LOADER_BASE), loader_ctrl->number_bytes);
                if ( res != FR_OK )
                {
                    am_util_stdio_printf("File Write Error! %s\n, FR_Table[res]");
                    loader_ctrl->status = FILE_LOADER_STATUS_ERROR;
                    //
                    // delete file
                    //
                    sprintf(path, "%s%s", eMMCPath, load_file_info->name);
                    res = f_unlink(path);
                    if (res != FR_OK )
                    {
                        am_util_stdio_printf("removes file faile, %s\n", FR_Table[res]);
                    }

                }
                else
                {
                    loader_ctrl->status = FILE_LOADER_STATUS_IDLE;
                }

                am_util_stdio_printf("FILE_LOADER_MODE_LOAD end\n");
            break;
            case FILE_LOADER_MODE_COMPLETE:
                loader_ctrl->mode = FILE_LOADER_MODE_IDLE;
                loader_ctrl->status = FILE_LOADER_STATUS_BUSY;
                am_util_stdio_printf("=> Load file complete \n");
                res = f_close(&FatFile);          //close the file to flush data into device
                if ( res != FR_OK )
                {
                    am_util_stdio_printf("File close Error!,%s\n", FR_Table[res]);
                    loader_ctrl->status = FILE_LOADER_STATUS_ERROR;
                    break;
                }

#if CHECK_LOAD_FILE
                if (check_load_file((char*)load_file_info->name, load_file_info->size, load_file_info->checksum) != true)
                {
                    am_util_stdio_printf("File verification failed, delete the file \n");
                    //Delete the file
                    sprintf(path, "%s%s", eMMCPath, load_file_info->name);
                    res = f_unlink(path);
                    if (res != FR_OK )
                    {
                        am_util_stdio_printf("Delete file %s faile, %s\n", load_file_info->name, FR_Table[res]);
                    }
                    break;
                }
#endif
                loader_ctrl->status = FILE_LOADER_STATUS_IDLE;
                am_util_stdio_printf("FILE_LOADER_MODE_COMPLETE\n");

                //
                // list directory files
                //
                memset(path, 0, sizeof(path));         // clear before setting
                strcpy(path, "/");                     // start at the top
                am_util_stdio_printf("=> List files in %s\n", path);
                res = scan_files(path);
                if (res != FR_OK)
                {
                    am_util_stdio_printf("open directory failed ,%s", FR_Table[res]);
                }
            break;
            default:
            break;
        }
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

