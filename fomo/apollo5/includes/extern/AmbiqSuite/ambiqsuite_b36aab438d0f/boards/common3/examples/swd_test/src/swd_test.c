//*****************************************************************************
//
//! @file swd_test.c
//!
//! @brief A brief test of the SWD interface.
//!
//! This example prints a banner message over SWO at 1M baud.
//! To see the output of this program, use J-Link SWO Viewer (or
//! similar viewer appl) and configure SWOClock for 1000.
//! The example sleeps after it is done printing.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include <string.h>

#define TEST
#define delay_us(val)                 am_util_delay_us((val)) // TODO this will change for Apollo4 to am_hal_delay_us

#define COOPER_FLASH_PAGE_SIZE        4096 // 4K
#define COOPER_FLASH_PAGE_SIZE_WORDS  (COOPER_FLASH_PAGE_SIZE/4)
#define MAILBOX_LOC                   0x10040000 // Fixed location in SRAM @ 256K

// Hardcoded memory locations for cooper
#define COOPER_SCRATCH_LOC            0x20004000 // TODO
#define COOPER_LIBFUNC_LOC            0x20000210 // TODO
#define COOPER_LIBFUNC_STACK          0x20008000 // TODO
#define COOPER_LIBFUNC_PARAM          0x20000000 // TODO
#define COOPER_LIBFUNC_RETVAL         0x20000010 // TODO
#define COOPER_LIBFUNC_CALL           0x20000210 // TODO

#define swd_load_sp(value)            am_util_swd_reg_write(AM_UTIL_SWD_CORE_SP, (value))
#define swd_setpc_go(value)           (am_util_swd_reg_write(AM_UTIL_SWD_CORE_PC, (value)) || am_util_swd_run())
#define am_util_swd_mem_write_words   am_util_swd_mem_write
#define am_util_swd_mem_read_words    am_util_swd_mem_read

#define CHK_ERR_BREAK(val)            if ((val)) break

// Cooper prebuilt library function - TODO - need to populate with actual binary
uint32_t cooper_libfunc[1024];
uint32_t cooper_libfunc_size_words = (sizeof(cooper_libfunc) + 3) / 4;

//*****************************************************************************
//
// Memory to write.
//
//*****************************************************************************

#define NUM_WORDS  1024 * 4
uint32_t pui32ReadBuffer[NUM_WORDS];
uint32_t pui32WriteBuffer[NUM_WORDS];

#ifdef TEST
//*****************************************************************************
//
// Generate image.
//
//*****************************************************************************
void
generate_image(void)
{
    uint32_t i;

    for (i = 0; i < NUM_WORDS; i++)
    {
        pui32WriteBuffer[i] = 1 << (i % 32);
    }
}
#endif


//*****************************************************************************
//
// Check image.
//
//*****************************************************************************
uint32_t
verify_image(uint32_t *pui32Buf1, uint32_t *pui32Buf2, uint32_t sizeWords)
{
    uint32_t i;
    uint32_t ui32Errors = 0;

    for (i = 0; i < sizeWords; i++)
    {
        if (pui32Buf1[i] != pui32Buf2[i])
        {
            am_util_stdio_printf("Mismatch: [%d] 1: 0x%08X 2: 0x%08X\n",
                                 pui32Buf1[i], pui32Buf2[i]);

            ui32Errors++;
        }
    }

    return ui32Errors;
}

// Write one flash page
uint32_t cooper_page_write(uint32_t ui32WriteAddr, void *pBuf)
{
    uint32_t temp[2];
    uint32_t ui32Error = 0;

    do
    {
        //
        // Halt the core.
        //
        CHK_ERR_BREAK(am_util_swd_halt());

        am_util_stdio_printf("Sending halt...");

        while (am_util_swd_halt_check() == false)
        {
            am_util_stdio_printf(".");
        }
        am_util_stdio_printf("Core halted.\n");
        // Load the buffer to SRAM in Cooper
        CHK_ERR_BREAK(am_util_swd_mem_write_words(COOPER_SCRATCH_LOC, pBuf, COOPER_FLASH_PAGE_SIZE_WORDS));
        temp[0] = 0xDEADBEEF;
        CHK_ERR_BREAK(am_util_swd_mem_write_words(COOPER_LIBFUNC_RETVAL, temp, 1));
        temp[0] = ui32WriteAddr;
        temp[1] = COOPER_SCRATCH_LOC;
        CHK_ERR_BREAK(am_util_swd_mem_write_words(COOPER_LIBFUNC_PARAM, temp, 2));
        CHK_ERR_BREAK(am_util_swd_mem_write_words(COOPER_SCRATCH_LOC, pBuf, COOPER_FLASH_PAGE_SIZE_WORDS));
        // Launch the SRAM helper function
        CHK_ERR_BREAK(swd_load_sp(COOPER_LIBFUNC_STACK));
        CHK_ERR_BREAK(swd_setpc_go(COOPER_LIBFUNC_CALL));
        // Poll for the function to finish
        do
        {
            delay_us(10);
            ui32Error = am_util_swd_mem_read_words(COOPER_LIBFUNC_RETVAL, temp, 1);
        } while ((ui32Error == 0) && (temp[0] == 0xDEADBEEF));
    } while (0);
    return ui32Error;
}

//*****************************************************************************
//
//! @brief  Programs in cooper flash
//!
//! The fuction iterates across blocks of data from flash page to page, copying the
//! data to SRAM first, followed by erase and reprogram of the intended destination flash page
//! This function also takes care of the fact that the address/size may not be
//! flash page aligned. It preserves the existing flash data in such a case
//!
//! @param  ui32WriteAddr specifies the destination address in flash
//! @param  pui32ReadAddr points to the source address
//! @param  ui32NumBytes indicates size of blob
//!
//! @return Returns None
//
//*****************************************************************************
static uint32_t
cooper_program(uint32_t ui32WriteAddr, uint32_t *pui32ReadAddr, uint32_t ui32NumBytes)
{
    uint32_t status = 0;
    uint32_t ui32NumWords = ui32NumBytes / 4;
    uint32_t ui32NumWordsInPage;
    uint32_t *pStart = pui32ReadBuffer;
    // Determine the preceding data bytes at the destination page
    uint32_t ui32PrecedingWords = (ui32WriteAddr & (COOPER_FLASH_PAGE_SIZE - 1)) / 4;

    if ((ui32NumBytes & 0x3) || (ui32WriteAddr & 0x3))
    {
        return -1;
    }
    // Flash Write can only happen in terms of pages
    // So, if the image does not start on page boundary - need to take proper precautions
    // to preserve other data in the page
    if (ui32PrecedingWords)
    {
        // Page aligned
        ui32WriteAddr &= ~(COOPER_FLASH_PAGE_SIZE - 1);
        // Copy the preceding content at destination page in buffer
        status = am_util_swd_mem_read_words(ui32WriteAddr, (void *)pui32ReadBuffer, ui32PrecedingWords);
        if (status)
        {
            return status;
        }
    }
    while ( ui32NumWords )
    {
        pStart = pui32ReadBuffer + ui32PrecedingWords;
        if ((ui32PrecedingWords + ui32NumWords) > COOPER_FLASH_PAGE_SIZE_WORDS)
        {
            ui32NumWordsInPage = COOPER_FLASH_PAGE_SIZE_WORDS - ui32PrecedingWords;
        }
        else
        {
            // Last sector to be written
            ui32NumWordsInPage = ui32NumWords;
            if ((ui32NumWordsInPage + ui32PrecedingWords) != COOPER_FLASH_PAGE_SIZE_WORDS)
            {
                // Copy the trailing content at destination page in buffer
                memcpy((void *)(pStart + ui32NumWordsInPage),
                        (void *)(pui32ReadAddr + ui32NumWordsInPage),
                        COOPER_FLASH_PAGE_SIZE - (ui32NumWordsInPage + ui32PrecedingWords)*4);
            }
        }
        // Read the image data from source
        memcpy((void *)pStart, (void *)pui32ReadAddr, ui32NumWordsInPage*4);
        status = cooper_page_write(ui32WriteAddr, pui32ReadBuffer);
        if (status)
        {
            return status;
        }
        ui32WriteAddr += COOPER_FLASH_PAGE_SIZE;
        pui32ReadAddr += ui32NumWordsInPage;
        ui32NumWords -= ui32NumWordsInPage;
        ui32PrecedingWords = 0;
    }
    return status;
}

#define MAILBOX_CMD_FLASH_WRITE   0xFF00FFFF
#define MAILBOX_CMD_READ          0xFF01FFFF
#define MAILBOX_CMD_MEM_WRITE     0xFF02FFFF

typedef struct
{
    volatile uint32_t cmdStatus; // Used for both command and status back - commands follow 0xFFxxFFFF
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
} mailbox_cmd_t;

void handle_mailbox_message(mailbox_cmd_t *pCmd)
{
    am_util_stdio_printf("Received Mailbox Command 0x%x\n", pCmd->cmdStatus);
    switch (pCmd->cmdStatus)
    {
        case MAILBOX_CMD_FLASH_WRITE: // program(destAddr, pSrc, length)
            pCmd->cmdStatus = cooper_program(pCmd->param0, (uint32_t *)pCmd->param1, pCmd->param2);
            break;
        case MAILBOX_CMD_READ: // read(srcAddr, pBuf, length)
            pCmd->cmdStatus = am_util_swd_mem_read_words(pCmd->param0, (void *)pCmd->param1, pCmd->param2);
            break;
        case MAILBOX_CMD_MEM_WRITE: // write(destAddr, pSrc, length)
            pCmd->cmdStatus = am_util_swd_mem_write_words(pCmd->param0, (void *)pCmd->param1, pCmd->param2);
            break;
        default:
            pCmd->cmdStatus = -1;
    }
    am_util_stdio_printf("Return Status 0x%x\n", pCmd->cmdStatus);
}


int32_t cooper_prog_util_init(void)
{
    uint32_t ui32Error;

    //
    // Initialize the SWD interface.
    //
    ui32Error = am_util_swd_initialize();
    if (ui32Error)
    {
        return -1;
    }
    am_util_stdio_printf("Initialized\n");

    //
    // Halt the core.
    //
    ui32Error = am_util_swd_halt();
    if (ui32Error)
    {
        return -1;
    }
    am_util_stdio_printf("Sending halt...");

    while (am_util_swd_halt_check() == false)
    {
        am_util_stdio_printf(".");
    }
    am_util_stdio_printf("Core halted.\n");

    //
    // Load the image.
    //
    am_util_stdio_printf("Writing SRAM.\n");

    // load the SRAM library to Cooper SRAM at designated location
    am_util_swd_mem_write_words(COOPER_LIBFUNC_LOC, cooper_libfunc, cooper_libfunc_size_words);

    //
    // Read back and verify.
    //
    am_util_stdio_printf("Reading SRAM and verifying.\n");
    am_util_swd_mem_read_words(COOPER_LIBFUNC_LOC, cooper_libfunc, cooper_libfunc_size_words);
    if (verify_image(cooper_libfunc, pui32ReadBuffer, cooper_libfunc_size_words))
    {
        return -1;
    }
    am_util_stdio_printf("Verification complete.\n");
    return 0;
}

mailbox_cmd_t *gpMailBox = (mailbox_cmd_t *)MAILBOX_LOC;

//*****************************************************************************
//
// Main
//
//*****************************************************************************
#ifndef TEST
int
main(void)
{
    //
    // Initialize the printf interface for UART output
    //
    am_bsp_uart_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("SWD Util.\n\n");

#ifdef TEST
    generate_image();
#endif

    if (cooper_prog_util_init())
    {
        while(1);
    }
    am_util_stdio_printf("Polling for Mailbox messages.\n\n");
    while (1)
    {
        while ((gpMailBox->cmdStatus & 0xFF00FFFF) != 0xFF00FFFF)
        {
            delay_us(100);
        }
        handle_mailbox_message(gpMailBox);
    }
}
#else
int
main(void)
{
    uint32_t ui32Error;

    //
    // Initialize the printf interface for UART output
    //
    am_bsp_uart_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("SWD Util Test.\n\n");

    //
    // Initialize the SWD interface.
    //
    ui32Error = am_util_swd_initialize();
    if (ui32Error)
    {
        while (1);
    }
    am_util_stdio_printf("Initialized\n");

    //
    // Halt the core.
    //
    ui32Error = am_util_swd_halt();
    if (ui32Error)
    {
        while (1);
    }
    am_util_stdio_printf("Sending halt...");

    while (am_util_swd_halt_check() == false)
    {
        am_util_stdio_printf(".");
    }
    am_util_stdio_printf("Core halted.\n");

    //
    // Load the image.
    //
    am_util_stdio_printf("Writing SRAM.\n");
    generate_image();
    am_util_swd_mem_write_words(0x10000000, pui32WriteBuffer, NUM_WORDS);

    //
    // Read back and verify.
    //
    am_util_stdio_printf("Reading SRAM and verifying.\n");
    am_util_swd_mem_read_words(0x10000000, pui32ReadBuffer, NUM_WORDS);
    if (verify_image(pui32WriteBuffer, pui32ReadBuffer, NUM_WORDS))
    {
        am_util_stdio_printf("Verification failed.\n");
        while(1);
    }
    else
    {
        am_util_stdio_printf("Verification complete.\n");
    }

    //
    // Verify register read and write operations.
    //
    uint32_t ui32RegWriteValue = 0x10000000;
    uint32_t ui32RegReadValue = 0;

    am_util_stdio_printf("Checking PC read/write.\n");
    am_util_swd_reg_write(AM_UTIL_SWD_CORE_PC, ui32RegWriteValue);
    am_util_swd_reg_read(AM_UTIL_SWD_CORE_PC, &ui32RegReadValue);

    if (ui32RegReadValue != ui32RegWriteValue)
    {
        while (1);
    }

    ui32RegWriteValue = 0x10004000;
    ui32RegReadValue = 0;

    am_util_stdio_printf("Checking SP read/write.\n");
    am_util_swd_reg_write(AM_UTIL_SWD_CORE_SP, ui32RegWriteValue);
    am_util_swd_reg_read(AM_UTIL_SWD_CORE_SP, &ui32RegReadValue);

    if (ui32RegReadValue != ui32RegWriteValue)
    {
        while (1);
    }

    am_util_stdio_printf("Testing complete.\n\n");

    am_util_swd_print_malloc_stats();

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}
#endif
