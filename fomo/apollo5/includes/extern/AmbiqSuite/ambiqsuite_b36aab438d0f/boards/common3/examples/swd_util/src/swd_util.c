//*****************************************************************************
//
//! @file swd_util.c
//!
//! @brief A utility to allow the programming of Cooper flash through Apollo4
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
#include "cooper_helpers.h"
#include "test_pattern.h"
#include <string.h>

//*****************************************************************************
//
// Configuration.
//
//*****************************************************************************
#define HALT_ON_ERRORS      1
#define PRINT_ERRORS        1
#define ENABLE_PRINTING     1
#define STANDALONE_TEST     1
#define VERIFY_DOWNLOADS    1

//*****************************************************************************
//
// Macros
//
//*****************************************************************************
#define REPORT_ERRORS(x) report_errors(__LINE__, (x))

#if ENABLE_PRINTING
#define PRINT_INFO(...) am_util_stdio_printf(__VA_ARGS__)
#else
#define PRINT_INFO(...)
#endif

//*****************************************************************************
//
// Algorithm constants.
//
//*****************************************************************************
#define DATA_LOCATION       0x20001000
#define ADDRESS_REGISTER    0x20000FC0
#define LENGTH_REGISTER     0x20000FD0
#define KEY1_REGISTER       0x20000F00
#define KEY2_REGISTER       0x20000FF0
#define ALG_RESET_VECTOR    0x200000D5
#define ALG_STACK_POINTER   0x20007000

//*****************************************************************************
//
// Buffer comparison.
//
//*****************************************************************************
uint32_t compare_buffers(void *p1, void *p2, uint32_t ui32NumBytes);
static void report_errors(uint32_t ui32Line, uint32_t ui32Error);

//*****************************************************************************
//
// Mailbox macros.
//
//*****************************************************************************
#define NUM_WORDS  1024 * 4
uint32_t pui32ReadBuffer[NUM_WORDS];
uint32_t pui32WriteBuffer[NUM_WORDS];

#define delay_us(val)                 am_util_delay_us((val)) // TODO this will change for Apollo4 to am_hal_delay_us

#define COOPER_FLASH_PAGE_SIZE        4096 // 4K
#define COOPER_FLASH_PAGE_SIZE_WORDS  (COOPER_FLASH_PAGE_SIZE/4)
#define MAILBOX_LOC                   0x10040000 // Fixed location in SRAM @ 256K

// Hardcoded memory locations for cooper
#define COOPER_SCRATCH_LOC            0x20001000    // TODO
#define COOPER_LIBFUNC_LOC            0x20000210    // TODO
#define COOPER_LIBFUNC_STACK          0x20007000    // TODO
#define COOPER_LIBFUNC_PARAM          0x20000000    // TODO
#define COOPER_LIBFUNC_RETVAL         KEY2_REGISTER // TODO
#define COOPER_LIBFUNC_CALL           0x200000D5    // TODO

#define swd_load_sp(value)            am_util_swd_reg_write(AM_UTIL_SWD_CORE_SP, (value))
#define swd_setpc_go(value)           (am_util_swd_reg_write(AM_UTIL_SWD_CORE_PC, (value)) || am_util_swd_run())

#define CHK_ERR_BREAK(val)            if ((val)) break

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
uint32_t g_pui32VerifyBuffer[COOPER_HELPERS_LENGTH];
uint32_t g_pui32PatternVerifyBuffer[COOPER_FLASH_PAGE_SIZE_WORDS];
uint32_t g_pui32PatternBuffer[COOPER_FLASH_PAGE_SIZE_WORDS];


// Cooper prebuilt library function - TODO - need to populate with actual binary
uint32_t cooper_libfunc[1024];
uint32_t cooper_libfunc_size_words = (sizeof(cooper_libfunc) + 3) / 4;

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

        //
        // Load the buffer to SRAM in Cooper
        //
        CHK_ERR_BREAK(am_util_swd_mem_write_words(COOPER_SCRATCH_LOC, pBuf, COOPER_FLASH_PAGE_SIZE_WORDS));

        //
        // Verify buffer.
        //
        CHK_ERR_BREAK(am_util_swd_mem_read_words(COOPER_SCRATCH_LOC, (uint32_t *) g_pui32PatternVerifyBuffer,
                                        COOPER_FLASH_PAGE_SIZE_WORDS));

        ui32Error = compare_buffers(g_pui32PatternVerifyBuffer, pBuf, COOPER_FLASH_PAGE_SIZE);
        REPORT_ERRORS(ui32Error);

        //
        // Set the parameters.
        //
        uint32_t ui32NumBytes = COOPER_FLASH_PAGE_SIZE_WORDS;
        uint32_t ui32Key1 = 0x42345255;
        uint32_t ui32Key2 = 0x78563412;

        CHK_ERR_BREAK(am_util_swd_mem_write_words(ADDRESS_REGISTER, &ui32WriteAddr, 1));
        CHK_ERR_BREAK(am_util_swd_mem_write_words(LENGTH_REGISTER, &ui32NumBytes, 1));
        CHK_ERR_BREAK(am_util_swd_mem_write_words(KEY1_REGISTER, &ui32Key1, 1));
        CHK_ERR_BREAK(am_util_swd_mem_write_words(KEY2_REGISTER, &ui32Key2, 1));

        //
        // Set stack pointer and program counter, and launch the program.
        //
        CHK_ERR_BREAK(swd_load_sp(ALG_STACK_POINTER));
        CHK_ERR_BREAK(swd_setpc_go(ALG_RESET_VECTOR));

        // temp[0] = 0xDEADBEEF;
        // CHK_ERR_BREAK(am_util_swd_mem_write_words(COOPER_LIBFUNC_RETVAL, temp, 1));

        // temp[0] = ui32WriteAddr;
        // temp[1] = COOPER_SCRATCH_LOC;

        // CHK_ERR_BREAK(am_util_swd_mem_write_words(COOPER_LIBFUNC_PARAM, temp, 2));

        // CHK_ERR_BREAK(am_util_swd_mem_write_words(COOPER_SCRATCH_LOC, pBuf, COOPER_FLASH_PAGE_SIZE_WORDS));

        // // Launch the SRAM helper function
        // CHK_ERR_BREAK(swd_load_sp(COOPER_LIBFUNC_STACK));
        // CHK_ERR_BREAK(swd_setpc_go(COOPER_LIBFUNC_CALL));

        // Poll for the function to finish
        do
        {
            delay_us(10);
            ui32Error = am_util_swd_mem_read_words(COOPER_LIBFUNC_RETVAL, temp, 1);
        } while ((ui32Error == 0) && (temp[0] != 0));
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

mailbox_cmd_t *gpMailBox = (mailbox_cmd_t *)MAILBOX_LOC;

//*****************************************************************************
//
// Error routine.
//
//*****************************************************************************
static void
report_errors(uint32_t ui32Line, uint32_t ui32Error)
{
    if (ui32Error)
    {
#if PRINT_ERRORS
        am_util_stdio_printf("\nError swd_util.c:%d: 0x%08X (%d)\n",
                             ui32Line, ui32Error, ui32Error);
#endif

#if HALT_ON_ERRORS
        while (1);
#endif
    }
}

//*****************************************************************************
//
// Compare buffers
//
//*****************************************************************************
uint32_t
compare_buffers(void *p1, void *p2, uint32_t ui32NumBytes)
{
    uint32_t i, ui32Errors;
    uint8_t *ptr1 = p1;
    uint8_t *ptr2 = p2;

    ui32Errors = 0;
    for (i = 0; i < ui32NumBytes; i++)
    {
        if (ptr1[i] != ptr2[i])
        {
            ui32Errors++;

            if (ui32Errors < 128)
            {
                PRINT_INFO("Mismatch (0x%08X): 0x%02X 0x%02X\n",
                           i, ptr1[i], ptr2[i]);
            }
        }
    }

    return ui32Errors;
}

//*****************************************************************************
//
// Load Cooper flash helper functions.
//
//*****************************************************************************
void
load_algorithm(void)
{
    uint32_t ui32Error;

    //
    // Halt the core.
    //
    ui32Error = am_util_swd_halt();
    REPORT_ERRORS(ui32Error);
    PRINT_INFO("Sending halt...");

    while (am_util_swd_halt_check() == false)
    {
        PRINT_INFO(".");
    }

    PRINT_INFO("Core halted.\n");

    //
    // Load the algorithm directly to SRAM.
    //
    PRINT_INFO("Loading algorithm... ");

    ui32Error = am_util_swd_mem_write(0x20000000, g_pui8CooperHelpers,
                                      COOPER_HELPERS_LENGTH);
    REPORT_ERRORS(ui32Error);
    PRINT_INFO("DONE.\n");

#if VERIFY_DOWNLOADS
    PRINT_INFO("Verifying algorithm... ");
    ui32Error = am_util_swd_mem_read(0x20000000, (uint8_t *) g_pui32VerifyBuffer,
                                     COOPER_HELPERS_LENGTH);
    REPORT_ERRORS(ui32Error);

    ui32Error = compare_buffers(g_pui8CooperHelpers, g_pui32VerifyBuffer,
                                COOPER_HELPERS_LENGTH);
    REPORT_ERRORS(ui32Error);

    PRINT_INFO("DONE.\n");
#endif
}

//*****************************************************************************
//
// Mailbox-based operation
//
//*****************************************************************************
void
mailbox_mode(void)
{
    PRINT_INFO("Polling for Mailbox messages.\n\n");

    while (1)
    {
        while ((gpMailBox->cmdStatus & 0xFF00FFFF) != 0xFF00FFFF)
        {
            delay_us(100);
        }
        handle_mailbox_message(gpMailBox);
    }
}


//*****************************************************************************
//
// Standalone test program.
//
//*****************************************************************************
void
standalone_test(void)
{
    uint32_t i, ui32Error;
    PRINT_INFO("\nStandalone mode.\n\n");

    uint8_t *pDest = (uint8_t *) g_pui32PatternBuffer;
    for (i = 0; i < TEST_PATTERN_LENGTH; i++)
    {
        //pDest[i] = g_pui8TestPattern[i];
        pDest[i] = i & 0xff;
    }

    //
    // Send a command to write a test pattern to flash.
    //
    mailbox_cmd_t cmd;
    cmd.cmdStatus = MAILBOX_CMD_FLASH_WRITE;
    cmd.param0 = 0x00000000;
    cmd.param1 = (uint32_t ) g_pui32PatternBuffer;
    cmd.param2 = TEST_PATTERN_LENGTH;

    handle_mailbox_message(&cmd);

    PRINT_INFO("Status: 0x%08X\n", cmd.cmdStatus);
    PRINT_INFO("P0:     0x%08X\n", cmd.param0);
    PRINT_INFO("P1:     0x%08X\n", cmd.param1);
    PRINT_INFO("P2:     0x%08X\n", cmd.param2);

#if VERIFY_DOWNLOADS
    cmd.cmdStatus = MAILBOX_CMD_READ;
    cmd.param0 = 0x00000000;
    cmd.param1 = (uint32_t) g_pui32PatternVerifyBuffer;
    cmd.param2 = TEST_PATTERN_LENGTH / 4;

    handle_mailbox_message(&cmd);

    PRINT_INFO("Status: 0x%08X\n", cmd.cmdStatus);
    PRINT_INFO("P0:     0x%08X\n", cmd.param0);
    PRINT_INFO("P1:     0x%08X\n", cmd.param1);
    PRINT_INFO("P2:     0x%08X\n", cmd.param2);

    ui32Error = compare_buffers(g_pui32PatternVerifyBuffer,
                                g_pui32PatternBuffer,
                                TEST_PATTERN_LENGTH);
    REPORT_ERRORS(ui32Error);

#endif
    PRINT_INFO("Complete.");
}

//*****************************************************************************
//
// Main function
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Error;

#if ENABLE_PRINTING
    //
    // Configure the UART for debug messages.
    //
    am_bsp_uart_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("SWD Util.\n\n");
#endif

    //
    // Initialize the SWD interface.
    //
    ui32Error = am_util_swd_initialize();
    REPORT_ERRORS(ui32Error);
    PRINT_INFO("Initialized\n");

    //
    // Load the Cooper algorithm into Cooper's SRAM
    //
    load_algorithm();

#if STANDALONE_TEST
    standalone_test();
#else
#endif

    while (1);
}