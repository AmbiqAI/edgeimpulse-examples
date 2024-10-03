//*****************************************************************************
//
//! @file swd_util_test.c
//!
//! @brief A test of the features of the SWD util for Cooper.
//!
//! This test is meant to test the features of LibSWD needed for programming
//! cooper.
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

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
uint32_t g_pui32ProgramData[512];
uint32_t g_pui32VerifyBuffer[COOPER_HELPERS_LENGTH / 4];
uint32_t g_pui32AlgBuffer[COOPER_HELPERS_LENGTH / 4];

//*****************************************************************************
//
// Macros.
//
//*****************************************************************************
#define REPORT_ERRORS(x)    report_errors(__LINE__, (x))

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
// Error routine.
//
//*****************************************************************************
static void
report_errors(uint32_t line, uint32_t ui32Error)
{
    if (ui32Error)
    {
        am_util_stdio_printf("\nError swd_util_test.c:%d: 0x%08X (%d)\n",
                             line, ui32Error, ui32Error);

        while (1);
    }
}

//*****************************************************************************
//
// Load the algorithm.
//
//*****************************************************************************
void
load_algorithm(void)
{
    uint32_t ui32Error;
    uint32_t i;

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
    // Load the algorithm directly to SRAM.
    //
    am_util_stdio_printf("Loading algorithm... ");

    for (i = 0; i < COOPER_HELPERS_LENGTH; i++)
    {
        ((uint8_t *) g_pui32AlgBuffer)[i] = g_pui8CooperHelpers[i];
    }

    ui32Error = am_util_swd_mem_write_words(0x20000000, g_pui32AlgBuffer,
                                      COOPER_HELPERS_LENGTH / 4);
    REPORT_ERRORS(ui32Error);
    am_util_stdio_printf("DONE.\n");


    am_util_stdio_printf("Verifying algorithm... ");
    ui32Error = am_util_swd_mem_read_words(0x20000000, g_pui32VerifyBuffer,
                                     COOPER_HELPERS_LENGTH / 4);

    bool bErrorsFound = false;
    for (i = 0; i < COOPER_HELPERS_LENGTH / 4; i++)
    {
        if (g_pui32AlgBuffer[i] != g_pui32VerifyBuffer[i])
        {
            am_util_stdio_printf("\nMismatch 0x%08X: 0x%08X (expected 0x%08X)",
                                 0x20000000 + i * 4, g_pui32VerifyBuffer[i],
                                 g_pui32AlgBuffer[i]);
            bErrorsFound = true;
        }
    }

    while (bErrorsFound);

    am_util_stdio_printf("DONE.\n");
}

//*****************************************************************************
//
// Halt the cooper core.
//
//*****************************************************************************
void
page_program(uint32_t ui32Address, uint32_t *pui32Data, uint32_t ui32NumBytes)
{
    uint32_t ui32Error;
    uint32_t ui32Key1 = 0x42345255;
    uint32_t ui32Key2 = 0x78563412;
    uint32_t ui32Output = 0;
    uint32_t i;
    bool bSuccess = false;
    uint32_t ui32PC;
    uint32_t ui32SP;


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
    // Write the code to SRAM
    //
    am_util_stdio_printf("Downloading code...");
    am_util_swd_mem_write_words(DATA_LOCATION, pui32Data, ui32NumBytes / 4);
    am_util_stdio_printf("DONE.\n");

    //
    // Set the keys and parameters.
    //
    uint32_t ui32VTOR = 0x20000000;
    am_util_stdio_printf("Writing parameters...");
    am_util_swd_mem_write_words(ADDRESS_REGISTER, &ui32Address, 1);
    am_util_swd_mem_write_words(LENGTH_REGISTER, &ui32NumBytes, 1);
    am_util_swd_mem_write_words(KEY1_REGISTER, &ui32Key1, 1);
    am_util_swd_mem_write_words(KEY2_REGISTER, &ui32Key2, 1);
    am_util_swd_mem_write_words(0xE000ED08, &ui32VTOR, 1);
    am_util_stdio_printf("DONE.\n");

    //
    // Set the SP and PC.
    //
    am_util_swd_reg_write(AM_UTIL_SWD_CORE_PC, ALG_RESET_VECTOR);
    am_util_swd_reg_write(AM_UTIL_SWD_CORE_SP, ALG_STACK_POINTER);

    //
    // Verifying registers.
    //
    am_util_swd_reg_read(AM_UTIL_SWD_CORE_PC, &ui32PC);
    am_util_swd_reg_read(AM_UTIL_SWD_CORE_SP, &ui32SP);

    if (ui32PC != ALG_RESET_VECTOR - 1)
    {
        am_util_stdio_printf("ERROR: PC write failed.\n");
        am_util_stdio_printf("PC: 0x%08X\n", ui32PC);
        while (1);
    }

    if (ui32SP != ALG_STACK_POINTER)
    {
        am_util_stdio_printf("ERROR: SP write failed.\n");
        am_util_stdio_printf("SP: 0x%08X\n", ui32SP);
        while (1);
    }

    //
    // Run.
    //
    am_util_stdio_printf("Running program.\n");
    am_util_swd_run();

    //
    // Wait until the program finishes.
    //
    am_util_stdio_printf("Waiting for completion...");
    am_util_delay_ms(3000);

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
    // Check to see if we're done.
    //
    for (i = 0; i < 100; i++)
    {
        am_util_swd_mem_read_words(KEY1_REGISTER, &ui32Key1, 1);
        am_util_swd_mem_read_words(KEY2_REGISTER, &ui32Output, 1);

        if (ui32Output == 0x0)
        {
            am_util_stdio_printf("DONE\n");
            bSuccess = true;
            break;
        }
        else
        {
            am_util_stdio_printf(".");
            am_util_delay_ms(100);
        }
    }

    if (!bSuccess)
    {
        am_util_stdio_printf("\nERROR: Program timed out\n");

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
        uint32_t regs[15];

        for (i = 0; i < 16; i++)
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
    }
}

//*****************************************************************************
//
// Main.
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Error, i;

    //
    // Initialize the printf interface for UART output
    //
    am_bsp_uart_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("SWD Util Test\n\n");

    //
    // Initialize the SWD interface.
    //
    ui32Error = am_util_swd_initialize();
    REPORT_ERRORS(ui32Error);
    am_util_stdio_printf("Initialized\n");

    //
    // Load the algorithm.
    //
    load_algorithm();

    //
    // Program some pages.
    //
    for (i = 0; i < 512; i++)
    {
        g_pui32ProgramData[i] = 0x1 << (i % 32);
    }

    page_program(0x0, g_pui32ProgramData, 128);

    //
    // Verify the results?
    //

    while (1);
}
