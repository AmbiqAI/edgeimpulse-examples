//*****************************************************************************
//
//! @file swd_util.c
//!
//! @brief SWD Cooper FW Loading Lib Integration Example.
//!
//! @addtogroup peripheral_examples Peripheral Examples
//
//! @defgroup swd_util_libswd_integration SWD Cooper FW Loading Lib Integration Example
//! @ingroup peripheral_examples
//! @{
//!
//! Purpose: A utility to allow the programming of Cooper flash through Apollo4
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
#include "cooper_bin.h"
#include <string.h>

#include "am_devices_cooper.h"

//*****************************************************************************
//
// Configuration.
//
//*****************************************************************************
#define HALT_ON_ERRORS              1
#define PRINT_ERRORS                1
#define ENABLE_PRINTING             1
#define ENABLE_EXTRA_PRINTING       1
#define ENABLE_PROFILING            1
#define STANDALONE_TEST             0
#define MAILBOX_MODE                1
#define VERIFY_DOWNLOADS            1
#define VERIFY_ALGORITHM            1
#define VERIFY_DOWNLOAD_BUFFER      0

// Note: Also need to change this setting in am_util_swd.h.
#define TARGET_INTERNAL_COOPER      1

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

#if ENABLE_EXTRA_PRINTING
#define PRINT_EXTRA(...) am_util_stdio_printf(__VA_ARGS__)
#else
#define PRINT_EXTRA(...)
#endif

#if ENABLE_PROFILING
#define PRINT_TIME(...) print_time()
#else
#define PRINT_TIME(...)
#endif

#define CPU_FREQ          (96 * 1000000)
#define TIME_START                                                            \
    do                                                                        \
    {                                                                         \
        am_hal_systick_load(SYSTICK_SPEED);                                   \
        am_hal_systick_start();                                               \
    }                                                                         \
    while (0);

#define TIME_STOP

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
#define ALG_RESET_VECTOR    0x200000D4
#define ALG_STACK_POINTER   0x20007000

//*****************************************************************************
//
// Buffer comparison.
//
//*****************************************************************************
uint32_t compare_buffers(const void *p1, const void *p2, uint32_t ui32NumBytes);
static void report_errors(uint32_t ui32Line, uint32_t ui32Error);
void print_time(void);
void load_algorithm(void);

//*****************************************************************************
//
// Mailbox macros.
//
//*****************************************************************************
#define delay_us(val)                 am_util_delay_us((val)) // TODO this will change for Apollo4 to am_hal_delay_us

#define COOPER_FLASH_PAGE_SIZE        4096 // 4K
#define COOPER_FLASH_PAGE_SIZE_WORDS  (COOPER_FLASH_PAGE_SIZE/4)
#define MAILBOX_LOC                   0x10040000 // Fixed location in SRAM @ 256K

// Hardcoded memory locations for cooper
#define COOPER_SCRATCH_LOC            0x20001000
#define COOPER_LIBFUNC_RETVAL         KEY2_REGISTER

#define swd_load_sp(value)            am_util_swd_reg_write(AM_UTIL_SWD_CORE_SP, (value))

#define CHK_ERR_BREAK(val)            if ((val)) break

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
uint32_t g_pui32VerifyBuffer[COOPER_HELPERS_LENGTH];
uint32_t g_pui32PatternVerifyBuffer[COOPER_FLASH_PAGE_SIZE_WORDS];
uint32_t pui32ReadBuffer[COOPER_FLASH_PAGE_SIZE];

volatile uint32_t g_ui32ElapsedTicks = 0;

//*****************************************************************************
//
// Set the PC and run.
//
//*****************************************************************************
uint32_t
swd_setpc_go(uint32_t value)
{
    uint32_t ui32Error;
    ui32Error = 0;

    ui32Error = am_util_swd_reg_write(AM_UTIL_SWD_CORE_PC, value);
    REPORT_ERRORS(ui32Error);

    ui32Error = am_util_swd_run();
    REPORT_ERRORS(ui32Error);

    return ui32Error;
}

// Write one flash page
uint32_t
cooper_page_write(uint32_t ui32WriteAddr, void *pBuf)
{
    uint32_t temp[2];
    uint32_t ui32Error = 0;

    PRINT_INFO("Programming 0x%08X to 0x%08X.\n", ui32WriteAddr, ui32WriteAddr + 4096);

    do
    {

        //
        // Halt the core.
        //
        CHK_ERR_BREAK(am_util_swd_halt());

        PRINT_EXTRA("Sending halt...");

        while (am_util_swd_halt_check() == false)
        {
            PRINT_EXTRA(".");
        }

        PRINT_EXTRA("Core halted.\n");

        //
        // Load the buffer to SRAM in Cooper
        //
        PRINT_EXTRA("Writing binary to SRAM... ");
        CHK_ERR_BREAK(am_util_swd_mem_write_words(COOPER_SCRATCH_LOC, pBuf,
                                                  COOPER_FLASH_PAGE_SIZE_WORDS));
        PRINT_EXTRA("Complete.\n");

#if VERIFY_DOWNLOAD_BUFFER

        PRINT_EXTRA("Verifying download buffer.\n");

        //
        // Verify buffer.
        //
        CHK_ERR_BREAK(am_util_swd_mem_read_words(COOPER_SCRATCH_LOC,
                                                 (uint32_t *) g_pui32PatternVerifyBuffer,
                                                 COOPER_FLASH_PAGE_SIZE_WORDS));

        ui32Error = compare_buffers(g_pui32PatternVerifyBuffer, pBuf,
                                    COOPER_FLASH_PAGE_SIZE);
        REPORT_ERRORS(ui32Error);

        PRINT_EXTRA("Buffer verified.\n");
#endif

        //
        // Set the parameters.
        //
        uint32_t ui32NumBytes = COOPER_FLASH_PAGE_SIZE;
        uint32_t ui32Key1 = 0x42345255;
        uint32_t ui32Key2 = 0x78563412;
        uint32_t ui32VTOR = 0x20000000;

        PRINT_EXTRA("Loading parameters.\n");
        CHK_ERR_BREAK(am_util_swd_mem_write_words(ADDRESS_REGISTER, &ui32WriteAddr, 1));
        CHK_ERR_BREAK(am_util_swd_mem_write_words(LENGTH_REGISTER, &ui32NumBytes, 1));
        CHK_ERR_BREAK(am_util_swd_mem_write_words(KEY1_REGISTER, &ui32Key1, 1));
        CHK_ERR_BREAK(am_util_swd_mem_write_words(KEY2_REGISTER, &ui32Key2, 1));
        CHK_ERR_BREAK(am_util_swd_mem_write_words(0xE000ED08, &ui32VTOR, 1));

        //
        // Set stack pointer and program counter, and launch the program.
        //
        uint32_t ui32PC;
        uint32_t ui32SP;
        PRINT_EXTRA("Running algorithm.\n");
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

        // am_util_delay_ms(10000);

#if 0
        ui32Error = am_util_swd_halt();
        REPORT_ERRORS(ui32Error);
        am_util_stdio_printf("Sending halt...");

        while (am_util_swd_halt_check() == false)
        {
            am_util_stdio_printf(".");
        }

#endif

        // Poll for the function to finish
        //PRINT_EXTRA("Waiting for completion...");

        uint32_t ui32NumTries = 0;

        do
        {
            ui32Error = am_util_swd_mem_read_words(COOPER_LIBFUNC_RETVAL, temp, 1);
            //PRINT_EXTRA(".");

            am_util_delay_ms(100);
            ui32NumTries++;

            if (ui32NumTries > 100)
            {
                am_util_stdio_printf("\nTimeout, dumping error info:\n");

                //
                // Halt the core.
                //
                ui32Error = am_util_swd_halt();
                REPORT_ERRORS(ui32Error);
                am_util_stdio_printf("Sending halt...");

                while (am_util_swd_halt_check() == false)
                {
                    am_util_stdio_printf(".");
                }

                am_util_stdio_printf("Core halted.\n");

                //
                // Read a few important values.
                //
                am_util_swd_reg_read(AM_UTIL_SWD_CORE_PC, &ui32PC);
                am_util_swd_reg_read(AM_UTIL_SWD_CORE_SP, &ui32SP);

                am_util_stdio_printf("\nERROR: Program timed out\n");
                am_util_stdio_printf("KEY1 (0x%08X): 0x%08X\n", KEY1_REGISTER, ui32Key1);
                am_util_stdio_printf("KEY2 (0x%08X): 0x%08X\n", KEY2_REGISTER, ui32Key2);
                am_util_stdio_printf("PC: 0x%08X\n", ui32PC);
                am_util_stdio_printf("SP: 0x%08X\n", ui32SP);

                //
                // Dump the core registers.
                //
                uint32_t i;
                uint32_t regs[15];

                for (i = 0; i < 15; i++)
                {
                    am_util_swd_reg_read(i, &regs[i]);
                    am_util_stdio_printf("R%02d: 0x%08X\n", i, regs[i]);
                }

                //
                // Show the fault status register.
                //
                uint32_t ui32CFSR;

                am_util_swd_mem_read_words(0xE000ED28, &ui32CFSR, 1);
                am_util_stdio_printf("CFSR: 0x%08X\n", ui32CFSR);

                //
                // Dump SRAM
                //
                uint32_t j;
                uint32_t ui32MemDump[4];

                for (i = 0; i < (512 / 4); i++)
                {
                    uint32_t ui32DumpAddr = 0x20000000 + (4 * i);
                    am_util_swd_mem_read_words(ui32DumpAddr, ui32MemDump, 4);
                    am_util_stdio_printf("0x%08X: ", ui32DumpAddr);

                    for (j = 0; j < 4; j++)
                    {
                        am_util_stdio_printf("0x%08X ", ui32MemDump[j]);
                    }

                    am_util_stdio_printf("\n");
                }

                while (1);
            }

        }
        while ((ui32Error == 0) && (temp[0] != 0));

        PRINT_EXTRA("DONE. Tries: %d\n", ui32NumTries);
    }
    while (0);

#if ENABLE_EXTRA_PRINTING
    PRINT_TIME();
#endif

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
cooper_program(uint32_t ui32WriteAddr, const uint32_t *pui32ReadAddr,
               uint32_t ui32NumBytes)
{
    uint32_t status = 0;
    uint32_t ui32NumWords = ui32NumBytes / 4;
    uint32_t ui32NumWordsInPage;
    uint32_t *pStart = pui32ReadBuffer;

    // Determine the preceding data bytes at the destination page
    uint32_t ui32PrecedingWords = ((ui32WriteAddr &
                                    (COOPER_FLASH_PAGE_SIZE - 1)) / 4);

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
        status = am_util_swd_mem_read_words(ui32WriteAddr, (void *)pui32ReadBuffer,
                                            ui32PrecedingWords);

        if (status)
        {
            return status;
        }
    }

    while (ui32NumWords)
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
                       COOPER_FLASH_PAGE_SIZE - (ui32NumWordsInPage + ui32PrecedingWords) * 4);
            }
        }

        // Read the image data from source
        memcpy((void *)pStart, (void *)pui32ReadAddr, ui32NumWordsInPage * 4);
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

//*****************************************************************************
//
// Read the Cooper processor state.
//
//*****************************************************************************
uint32_t
read_cooper_state(void)
{
    uint32_t ui32Error;

    PRINT_INFO("Printing Cooper internal state:\n");

    //
    // Halt the core.
    //
    ui32Error = am_util_swd_halt();
    REPORT_ERRORS(ui32Error);
    PRINT_EXTRA("Sending halt...");

    while (am_util_swd_halt_check() == false)
    {
        am_util_stdio_printf(".");
    }

    PRINT_EXTRA("Core halted.\n");

    //
    // Read a few important values.
    //
    uint32_t ui32PC, ui32SP;
    am_util_swd_reg_read(AM_UTIL_SWD_CORE_PC, &ui32PC);
    am_util_swd_reg_read(AM_UTIL_SWD_CORE_SP, &ui32SP);

    am_util_stdio_printf("PC: 0x%08X\n", ui32PC);
    am_util_stdio_printf("SP: 0x%08X\n", ui32SP);

    //
    // Dump the core registers.
    //
    uint32_t i;
    uint32_t regs[15];

    for (i = 0; i < 15; i++)
    {
        am_util_swd_reg_read(i, &regs[i]);
        am_util_stdio_printf("R%02d: 0x%08X\n", i, regs[i]);
    }

    //
    // Show the fault status register.
    //
    uint32_t ui32CFSR;

    am_util_swd_mem_read_words(0xE000ED28, &ui32CFSR, 1);
    am_util_stdio_printf("CFSR: 0x%08X\n", ui32CFSR);

    while (1);
}

//*****************************************************************************
//
// Verify image.
//
//*****************************************************************************
static uint32_t
cooper_verify(uint32_t ui32CooperAddr, const uint32_t *pui32ReadAddr,
              uint32_t ui32NumBytes)
{

    uint32_t ui32WordsRemaining = ui32NumBytes / 4;
    uint32_t ui32WordsToRead, ui32Error;
    uint32_t ui32Offset = 0;

    if ((ui32NumBytes & 0x3) || (ui32CooperAddr & 0x3))
    {
        return -1;
    }

    while (ui32WordsRemaining)
    {
        //
        // Read up to a full page of data from Cooper.
        //
        ui32WordsToRead = (ui32WordsRemaining > COOPER_FLASH_PAGE_SIZE_WORDS ?
                           COOPER_FLASH_PAGE_SIZE_WORDS : ui32WordsRemaining);

        PRINT_INFO("Verifying 0x%08X to 0x%08X.\n",
                   ui32CooperAddr + (ui32Offset * 4),
                   ui32CooperAddr + (ui32Offset * 4) + (ui32WordsToRead * 4));

        ui32Error = am_util_swd_mem_read_words(ui32CooperAddr + (ui32Offset * 4),
                                               g_pui32PatternVerifyBuffer,
                                               ui32WordsToRead);
        REPORT_ERRORS(ui32Error);

        //
        // Check the data we read against the buffer we were given.
        //
        uint32_t i;
        uint32_t *pui32CooperData = g_pui32PatternVerifyBuffer;
        const uint32_t *pui32SourceData = pui32ReadAddr + ui32Offset;

        for (i = 0; i < ui32WordsToRead; i++)
        {
            if (pui32CooperData[i] != pui32SourceData[i])
            {
                PRINT_INFO("Mismatch found at 0x%08X.\n",
                           ui32CooperAddr + (ui32Offset * 4) + (i * 4));
                return 1;
            }
        }

        ui32Offset += ui32WordsToRead;
        ui32WordsRemaining -= ui32WordsToRead;
    }

    return 0;
}

//*****************************************************************************
//
// Mailbox interface.
//
//*****************************************************************************
#define MAILBOX_CMD_FLASH_WRITE   0xFF00FFFF
#define MAILBOX_CMD_READ          0xFF01FFFF
#define MAILBOX_CMD_MEM_WRITE     0xFF02FFFF
#define MAILBOX_CMD_READ_STATE    0xFF03FFFF
#define MAILBOX_CMD_MEM_DUMP      0xFF04FFFF
#define MAILBOX_CMD_RESET         0xFF05FFFF

typedef struct
{
    // Used for both command and status back - commands follow 0xFFxxFFFF
    volatile uint32_t cmdStatus;
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
} mailbox_cmd_t;

void
handle_mailbox_message(mailbox_cmd_t *pCmd)
{
    PRINT_INFO("Received Mailbox Command 0x%x\n", pCmd->cmdStatus);

    switch (pCmd->cmdStatus)
    {
        // program(destAddr, pSrc, length)
        case MAILBOX_CMD_FLASH_WRITE:
            load_algorithm();

            pCmd->cmdStatus = cooper_program(pCmd->param0,
                                             (uint32_t *)pCmd->param1,
                                             pCmd->param2);
            PRINT_TIME();
#if VERIFY_DOWNLOADS
            pCmd->cmdStatus = cooper_verify(pCmd->param0,
                                            (uint32_t *)pCmd->param1,
                                            pCmd->param2);
            PRINT_TIME();
#endif
            PRINT_INFO("Complete.");
            break;

        // read(srcAddr, pBuf, length)
        case MAILBOX_CMD_READ:
            pCmd->cmdStatus = am_util_swd_mem_read_words(pCmd->param0,
                                                         (void *)pCmd->param1,
                                                         pCmd->param2 / 4);
            break;

        // write(destAddr, pSrc, length)
        case MAILBOX_CMD_MEM_WRITE:
            pCmd->cmdStatus = am_util_swd_mem_write_words(pCmd->param0,
                                                          (void *)pCmd->param1,
                                                          pCmd->param2 / 4);
            break;

        case MAILBOX_CMD_READ_STATE:
            pCmd->cmdStatus = read_cooper_state();
            break;

        case MAILBOX_CMD_MEM_DUMP:
            //
            // Dump SRAM
            //

            PRINT_INFO("Memory dump:\n");
            uint32_t i, j;
            uint32_t ui32MemDump[4];

            for (i = 0; i < (pCmd->param1 / 4); i++)
            {
                uint32_t ui32DumpAddr = pCmd->param0 + (4 * i);
                am_util_swd_mem_read_words(ui32DumpAddr, ui32MemDump, 4);
                am_util_stdio_printf("0x%08X: ", ui32DumpAddr);

                for (j = 0; j < 4; j++)
                {
                    am_util_stdio_printf("0x%08X ", ui32MemDump[j]);
                }

                am_util_stdio_printf("\n");
            }

            pCmd->cmdStatus = 0;

            break;

#if TARGET_INTERNAL_COOPER
        case MAILBOX_CMD_RESET:
            //
            // Toggle the physical reset pin.
            //
            am_hal_gpio_state_write(42, AM_HAL_GPIO_OUTPUT_SET);
            am_util_delay_ms(1);
            am_hal_gpio_state_write(42, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_util_delay_ms(1);
            am_hal_gpio_state_write(42, AM_HAL_GPIO_OUTPUT_SET);
            pCmd->cmdStatus = 0;
            break;
#endif

        default:
            pCmd->cmdStatus = -1;
    }

    PRINT_EXTRA("Return Status 0x%x\n", pCmd->cmdStatus);
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
        PRINT_INFO("\nError swd_util.c:%d: 0x%08X (%d)\n",
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
compare_buffers(const void *p1, const void *p2, uint32_t ui32NumBytes)
{
    uint32_t i, ui32Errors;
    const uint8_t *ptr1 = p1;
    const uint8_t *ptr2 = p2;

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

    PRINT_TIME();

    //
    // Halt the core.
    //
    ui32Error = am_util_swd_halt();
    REPORT_ERRORS(ui32Error);
    PRINT_EXTRA("Sending halt...");

    while (am_util_swd_halt_check() == false)
    {
        PRINT_EXTRA(".");
    }

    PRINT_EXTRA("Core halted.\n");

    //
    // Load the algorithm directly to SRAM.
    //
    PRINT_INFO("Loading algorithm... ");

    ui32Error = am_util_swd_mem_write(0x20000000, g_pui8CooperHelpers,
                                      COOPER_HELPERS_LENGTH);
    REPORT_ERRORS(ui32Error);
    PRINT_INFO("DONE.\n");
    PRINT_TIME();

#if VERIFY_ALGORITHM
    PRINT_INFO("Verifying algorithm... ");
    ui32Error = am_util_swd_mem_read(0x20000000, (uint8_t *) g_pui32VerifyBuffer,
                                     COOPER_HELPERS_LENGTH);
    REPORT_ERRORS(ui32Error);

    ui32Error = compare_buffers(g_pui8CooperHelpers, g_pui32VerifyBuffer,
                                COOPER_HELPERS_LENGTH);
    REPORT_ERRORS(ui32Error);

    PRINT_INFO("DONE.\n");
    PRINT_TIME();
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

    // Clear the Mailbox
    gpMailBox->cmdStatus = 0;
    gpMailBox->param0 = 0;
    gpMailBox->param1 = 0;
    gpMailBox->param2 = 0;

    while (1)
    {
        while ((gpMailBox->cmdStatus & 0xFF00FFFF) != 0xFF00FFFF)
        {
            delay_us(100);
        }

        handle_mailbox_message(gpMailBox);
        PRINT_INFO("Command complete.\n");
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
    PRINT_INFO("\nStandalone mode.\n\n");

    PRINT_TIME();

    //
    // Send a command to write a test pattern to flash.
    //
    mailbox_cmd_t cmd;
    cmd.cmdStatus = MAILBOX_CMD_FLASH_WRITE;
    cmd.param0 = 0x00000000;
    cmd.param1 = (uint32_t) g_pui32CooperBin;
    cmd.param2 = COOPER_BIN_LENGTH_WORDS * 4;

    handle_mailbox_message(&cmd);

#if !ENABLE_EXTRA_PRINTING
    PRINT_TIME();
#endif

    // PRINT_INFO("Status: 0x%08X\n", cmd.cmdStatus);
    // PRINT_INFO("P0:     0x%08X\n", cmd.param0);
    // PRINT_INFO("P1:     0x%08X\n", cmd.param1);
    // PRINT_INFO("P2:     0x%08X\n", cmd.param2);

#if VERIFY_DOWNLOADS
    uint32_t i, ui32Error;

    PRINT_INFO("Verifying\n");
    uint32_t ui32BytesLeft = COOPER_BIN_LENGTH_WORDS * 4;
    uint32_t ui32BytesToRead;
    uint32_t ui32ReadAddress = 0x00000000;
    uint32_t ui32Offset = 0x0;

    while (ui32BytesLeft)
    {
#if ENABLE_EXTRA_PRINTING
        PRINT_TIME();
#endif
        ui32BytesToRead = ui32BytesLeft > COOPER_FLASH_PAGE_SIZE ?
                          COOPER_FLASH_PAGE_SIZE :
                          ui32BytesLeft;

        PRINT_INFO("Verifying 0x%08X to 0x%08X\n", ui32ReadAddress + ui32Offset,
                                                   ui32ReadAddress + ui32Offset + ui32BytesToRead);

        cmd.cmdStatus = MAILBOX_CMD_READ;
        cmd.param0 = ui32ReadAddress + ui32Offset;
        cmd.param1 = (uint32_t) g_pui32PatternVerifyBuffer;
        cmd.param2 = ui32BytesToRead;

        handle_mailbox_message(&cmd);

        //PRINT_INFO("Status: 0x%08X\n", cmd.cmdStatus);
        //PRINT_INFO("P0:     0x%08X\n", cmd.param0);
        //PRINT_INFO("P1:     0x%08X\n", cmd.param1);
        //PRINT_INFO("P2:     0x%08X\n", cmd.param2);

        ui32Error = compare_buffers(g_pui32PatternVerifyBuffer,
                                    &g_pui32CooperBin[ui32Offset / 4],
                                    ui32BytesToRead);
        if (ui32Error)
        {
            PRINT_INFO("Errors found:\n");
            PRINT_INFO("Binary data: \n");

            for (i = 0; i < ui32BytesToRead / 4; i += 4)
            {
                PRINT_INFO("0x%08X: 0x%08X 0x%08X 0x%08X 0x%08X\n",
                           ui32Offset + i * 4,
                           g_pui32PatternVerifyBuffer[i],
                           g_pui32PatternVerifyBuffer[i + 1],
                           g_pui32PatternVerifyBuffer[i + 2],
                           g_pui32PatternVerifyBuffer[i + 3]);
            }
        }

        ui32Offset += ui32BytesToRead;
        ui32BytesLeft -= ui32BytesToRead;

    }

#endif
    PRINT_INFO("Complete.");
    PRINT_TIME();
}

//*****************************************************************************
//
// Systick Handler
//
//*****************************************************************************
#if ENABLE_PROFILING
void
SysTick_Handler(void)
{
    g_ui32ElapsedTicks++;
}

//*****************************************************************************
//
// Function to retrieve the current time.
//
//*****************************************************************************
void
print_time(void)
{
    uint32_t ui32Ticks = g_ui32ElapsedTicks;

    PRINT_INFO("Elapsed time: %d.%d s\n",
               ui32Ticks / 10,
               ui32Ticks % 10);
}

#endif

//*****************************************************************************
//
// Main function
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Error;

    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

#if TARGET_INTERNAL_COOPER
    // Enable crystals for Cooper first
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_ENABLE, 0);

    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, (void *)&g_amHalMcuctrlArgBLEDefault);

    // Pull down CS to keep Cooper wakeup
    am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_CS,   AM_HAL_GPIO_PINCFG_OUTPUT);
    am_hal_gpio_state_write(AM_BSP_GPIO_IOM4_CS, AM_HAL_GPIO_OUTPUT_CLEAR);

    //
    // Set up cooper GPIOs.
    //
    am_hal_gpio_pincfg_t sResetPin = AM_HAL_GPIO_PINCFG_OUTPUT;

    //
    // Issue a reset to Cooper.
    //
    am_hal_gpio_state_write(42, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_pinconfig(42, sResetPin);
    am_util_delay_ms(1);
    am_hal_gpio_state_write(42, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_util_delay_ms(1);
    am_hal_gpio_state_write(42, AM_HAL_GPIO_OUTPUT_SET);

#endif

    //
    // Start Systick.
    //
#if ENABLE_PROFILING
    am_hal_systick_int_enable();
    NVIC_SetPriority(RTC_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(RTC_IRQn);
    am_hal_systick_load(CPU_FREQ / 10);
    am_hal_systick_start();
#endif

#if ENABLE_PRINTING
    //
    // Configure the UART for debug messages.
    //
#if 0
    am_hal_uart_initialize(0, &pvCOMUART);
    am_bsp_uart_printf_enable(pvCOMUART);
#else

    //am_bsp_uart_printf_enable();
    am_bsp_itm_printf_enable();
#endif
    am_util_stdio_terminal_clear();
    PRINT_INFO("SWD Util.\n\n");
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
#if STANDALONE_TEST
    load_algorithm();
    standalone_test();
#elif MAILBOX_MODE
    mailbox_mode();
#endif

#if ENABLE_PROFILING
    am_hal_systick_stop();
#endif

    while (1)
    {
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

