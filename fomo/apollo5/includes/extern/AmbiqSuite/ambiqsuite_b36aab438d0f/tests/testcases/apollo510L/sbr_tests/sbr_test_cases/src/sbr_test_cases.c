//*****************************************************************************
//
//! @file sbr_test_cases.c
//!
//! @brief SBR test case program.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_sbr_test.h"


#if defined(PTE_TEST)
    uint32_t g_security_word;
#endif
uint32_t g_tests_run = 0;
uint32_t g_tests_passed = 0;
static char g_info_buf[400];

#define ENABLE_DEBUGGER
#define BUFFER_SIZE 3072
char g_cPrintBuff[BUFFER_SIZE];
char *g_pcBuff;
uint32_t g_ui32BuffCnt = 0;


//
// DCU controls needed for debugger
//
#if defined(AM_PART_APOLLO5A)
#define DCU_DEBUGGER (AM_HAL_DCU_SWD | AM_HAL_DCU_CPUDBG_INVASIVE | AM_HAL_DCU_CPUDBG_NON_INVASIVE)
#elif defined(AM_PART_APOLLO5B) || defined(AM_PART_APOLLO510L)
#define DCU_DEBUGGER (AM_HAL_DCU_SWD | AM_HAL_DCU_CPUDBG_INVASIVE | AM_HAL_DCU_CPUDBG_NON_INVASIVE | AM_HAL_DCU_CPUDBG_S_INVASIVE | AM_HAL_DCU_CPUDBG_S_NON_INVASIVE)
#else
#error "Undefined Part Number"
#endif


// ****************************************************************************
//
// void unity_begin_local(void)
//
// Local copy of UnityBegin() without the call to UNITY_OUTPUT_START().
// UnityBegin() is removed from test_main.c in the custom testcase.mk file,
// replaced with a call to this local function in globalSetUp().
// UNITY_OUTPUT_START() is now called at the beginning of globalTearDown(). 
// This fixes the DCU Enables test failure in Secure LCS as described 
// in https://ambiqmicro.atlassian.net/browse/CAYNSWS-2315.
//
// ****************************************************************************
extern struct UNITY_STORAGE_T Unity;

void unity_begin_local(void)
{   
    Unity.TestFile = "src/sbr_test_cases.c";
    Unity.CurrentTestName = NULL;
    Unity.CurrentTestLineNumber = 0;
    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
    Unity.TestIgnores = 0;
    Unity.CurrentTestFailed = 0;
    Unity.CurrentTestIgnored = 0;

    UNITY_CLR_DETAILS();
}

// ****************************************************************************
//
// Get the Security Mode
//
// ****************************************************************************
static uint32_t get_security_mode(void)
{
    uint32_t security_word, sw_org;

    // get security word
    security_word = (*((volatile uint32_t*)(AM_REG_OTP_INFOC_SECURITY_ADDR)));
    sw_org = security_word;

    // mask the fields we are interested in
    security_word &=  SECURITY_WORD_MASK;

    // TODO -- add Apollo4 as a security mode (will run @0x00400000)
    // TODO -- add mode type to run as an Ambiq  SBL (@0x00402000) to test SBR without SBL

    switch (security_word)
    {
        // unprogrammed (vanilla part)
        case (0x00000000):   return BOOTMODE_NONE;

        // Ambiq-SBL non secure
        case (0x022000A0):   // Ambiq-SBL (security not provisioned)
        case (0x022005A0):   return BOOTMODE_AMB_SBL_NONSECURE;

        // Ambiq-SBL secure
        case (0x022002A0):   return BOOTMODE_AMB_SBL_SECURE;

        // No-SBL nonsecure
        case (0x052000A0):   // Non-SBL (security not provisioned)
        case (0x052005A0):   return BOOTMODE_NOSBL_NONSECURE;

        // No-SBL secure
        case (0x052002A0):   return BOOTMODE_NOSBL_SECURE;

        default:
            output_buffer("  ***** Invalid Security Mode reported %x\n", CRYPTO->LCSREG_b.LCSREG);
            return BOOTMODE_ERROR;    // Bootmode Error
    }
 }

// ****************************************************************************
//
// Get the LCS from the Crypto Register
//
// ****************************************************************************
uint32_t get_lcs(void)
{
    // TODO: check for partial RMA mode and set flag (and report) parital RAM

    switch (CRYPTO->LCSREG_b.LCSREG)  {
        case 0x0:   return CM_LCS;
        case 0x1:   return DM_LCS;
        case 0x5:   return SECURE_LCS;
        case 0x7:   return RMA_LCS;
        default:
            output_buffer("  ***** Invalid LCS reported %x\n", CRYPTO->LCSREG_b.LCSREG);
            return LCS_ERROR;
    }
}

// ****************************************************************************
//
// Print Reset Status
//
// ****************************************************************************
void get_reset_status( security_t* sec_mode )
{

    uint32_t rstStat;
    output_buffer("");

    // Report the reset status register
    if ( sec_mode->mode == BOOTMODE_NONE )
    {
        // Non-SBL part
// #### INTERNAL BEGIN ####
        rstStat = AM_REGVAL(0x4000000C);
// #### INTERNAL END ####
    }
    else
    {
        // The reset status in all other modes comes from the RSTGEN->STAT register
        rstStat = RSTGEN->STAT;
    }

    output_buffer("\nReset Status:\n");

    // Print the Reset Status
    if (rstStat & AM_HAL_RESET_STATUS_EXTERNAL)
    {
        output_buffer("  Button Reset\n");
    }

    if (rstStat & AM_HAL_RESET_STATUS_SWPOI)
    {
        output_buffer("  POI Reset\n");
    }

    if (rstStat & AM_HAL_RESET_STATUS_SWPOR)
    {
        output_buffer("  POR Reset\n");
    }

    if (rstStat & AM_HAL_RESET_STATUS_DEBUGGER)
    {
        output_buffer("  Debugger\n");
    }

    if (rstStat & AM_HAL_RESET_STATUS_BOD)
    {
        output_buffer("  Brown-Out Reset\n");
    }

    if (rstStat & AM_HAL_RESET_STATUS_WDT)
    {
        output_buffer("  WatchDog Reset\n");
    }

    if (rstStat & AM_HAL_RESET_STATUS_POR)
    {
        output_buffer("  Power-On Reset\n");
    }

    //Need to clear this register since it accumulates the reset status after different resets
// #### INTERNAL BEGIN ####
        AM_REGVAL(0x40000010) = 0x01;
// #### INTERNAL END ####

}

// ****************************************************************************
//
// Get the Bootmode and LCS and check they are valid
//
// ****************************************************************************
void get_boot_mode_lcs(security_t *sec_mode)
{
    // init the security mode and current lcs for the reset of the tests to use
    sec_mode->mode = get_security_mode();
    sec_mode->lcs = get_lcs();

    // TODO -- test for mcuctl->bootloader error

    if (sec_mode->mode == BOOTMODE_ERROR || sec_mode->lcs == LCS_ERROR)
    {
        output_buffer("Invalid Configuration, can't continue - Halting)\n");
        while (1);
    }

    output_buffer("Current Security Mode: ");
    switch (sec_mode->mode)  {
        case BOOTMODE_NONE:                 output_buffer("Unprovisioned Part, 'Vanilla'\n"); break;    // 'Vanilla' unprovisioned part
        case BOOTMODE_AMB_SBL_NONSECURE:    output_buffer("Ambiq-SBL, NonSecure\n");          break;    // Ambq SBL - NonSecure
        case BOOTMODE_AMB_SBL_SECURE:       output_buffer("Ambiq-SBL, Secure\n");             break;    // Ambq SBL - Secure
        case BOOTMODE_NOSBL_NONSECURE:      output_buffer("Non-SBL, NonSecure\n");            break;    // Customer SBL - NonSecure
        case BOOTMODE_NOSBL_SECURE:         output_buffer("Non-SBL, Secure\n");               break;    // Customer SBL - Secure
    }

    //  TODO:  Handle and report Parial RMA mode -- then set to LCS to DM or Secure for rest of the testing

    // report the LCS state
    output_buffer("LCS State: ");
    switch (sec_mode->lcs)  {
        case CM_LCS:             output_buffer("CM-LCS");      break;
        case DM_LCS:             output_buffer("DM-LCS");      break;
        case SECURE_LCS:         output_buffer("Secure-LCS");  break;
        case RMA_LCS:            output_buffer("RMA-LCS");     break;
    }

    // Report the reset status
    get_reset_status(sec_mode);

    am_util_stdio_printf("\n\n");
}

// ****************************************************************************
//
// Output test result status called once per test/
//
// ****************************************************************************
void output_status(char *test_name, uint32_t pf, uint32_t *tests_run, uint32_t *tests_passed, char *info_str)
{
    output_buffer("Test: %-50s - %s\n", test_name, (pf == AM_STATUS_SUCCESS) ? "Pass" : "Fail");
    if (am_util_string_strlen(info_str))
    {
        output_buffer("  (%s)\n", info_str);
    }
    output_buffer("\n");

    (*tests_run)++;
    if (pf == AM_STATUS_SUCCESS)
        (*tests_passed)++;

}

// ****************************************************************************
//
// Init print buffer
//
// ****************************************************************************
void print_buffer_init(void)
{
    int ix;

    //
    // Initialize our printf buffer.
    //
    for ( ix = 0; ix < BUFFER_SIZE; ix++ )
    {
        g_cPrintBuff[ix] = '\0';
    }
    g_pcBuff = g_cPrintBuff;
    g_ui32BuffCnt = 0;
}

// ****************************************************************************
//
// Put all output statement into a buffer
//
// ****************************************************************************
uint32_t output_buffer(char *pcFmt, ...)
{
    uint32_t ui32RetVal;
    uint32_t ui32NumChar;
    
    va_list pArgs;
    
    if ( g_ui32BuffCnt < BUFFER_SIZE )
    {
        va_start(pArgs, pcFmt);
        ui32NumChar = am_util_stdio_vsprintf(g_pcBuff, pcFmt, pArgs);
        va_end(pArgs);
        
        if ( ( g_ui32BuffCnt + ui32NumChar >= BUFFER_SIZE ) )
        {
            // BUffer overflow
            am_util_stdio_printf("\nBUFFER OVERFLOW\n");
        }
        else
        {
            g_pcBuff += ui32NumChar;
            // Skip over the NULL terminator
            g_pcBuff++;
            g_ui32BuffCnt += ui32NumChar;
            // Add the NULL terminator
            g_ui32BuffCnt++;
            ui32RetVal = ui32NumChar;
        }
    }
    
    return ui32RetVal;
}

// ****************************************************************************
//
// Print everything 
//
// ****************************************************************************

void print_buffer(void)
{
    uint32_t ui32Cnt;
    char *pcBuff;
    
    //
    // Initialize the printf interface for ITM output.
    //
    if ( am_bsp_debug_printf_enable() )
    {
        while(1);
    }
    
    //
    // Print the Banner
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("-------------------------------------------------------\n\n");
    am_util_stdio_printf("          Ambiq SBR Test App " SBR_TEST_VERSIONSTR "\n\n");
    
    // Parse the buffer and print out 
    pcBuff = g_cPrintBuff;
    ui32Cnt = 0;
    
    //
    // Find the last NULL Character
    //
    char *pcItr = g_cPrintBuff;
    int count = 0;
    int win_size = 4;
    for (int i = 0; i< (BUFFER_SIZE/win_size); i++)    
    {
        if ( *pcItr == '\0' && *(pcItr+1) == '\0' &&  *(pcItr+2) == '\0' &&  *(pcItr+3) == '\0' )
        {
            break;
        }
        count +=4;
        pcItr += 4;
    }
    
    
    while( ( ui32Cnt < count ) )
    {
        am_util_stdio_printf(pcBuff);
        while( *pcBuff != '\0' )
        {
            pcBuff++;
            ui32Cnt++;
        }
        ui32Cnt++;
        pcBuff++;
    }

    am_util_stdio_printf("\n\nFinished the available tests\n\n");
    am_util_stdio_printf("-------------------------------------------------------\n\n");    
    
}
//*****************************************************************************
//
// Optional setup/tear-down functions.
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
    AM_UPDATE_TEST_PROGRESS(); // toggle GPIO to indicate the test is still running
}

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{

    // Re-enable power to the ROM, which was disabled in the call
    // to am_hal_pwrctrl_low_power_init (called in test_main.c)
    PWRCTRL->MEMPWREN_b.PWRENROM = 1;

    // Call local unit_begin which omits the prints (enabled in globalTearDown)
    unity_begin_local();

    // Init print buffer 
    // NOTE: Do not use successive NULL character otherwise the printing fails
    print_buffer_init();

#if defined(PTE_TEST)
    am_hal_gpio_pincfg_t x = AM_HAL_GPIO_PINCFG_OUTPUT;
    GPIO->PADKEY = 0x73;
    GPIO->PINCFG0 = GPIO->PINCFG1 = GPIO->PINCFG2 = (uint32_t) x.GP.cfg;
    GPIO->PADKEY = 0;
    am_hal_gpio_output_set(0);
    am_hal_gpio_output_clear(1);
    am_hal_gpio_output_clear(2);
#endif


    // Get and report the current Secuiry and LCS mode
    get_boot_mode_lcs(&g_secModeLCS);


}

void
globalTearDown(void)
{
    // Send the test start character sequence
    // (normally done in UnityBegin())
    UNITY_OUTPUT_START();
    
    print_buffer();

#if defined(PTE_TEST)
    // verify all test pass
    if (tests_run == g_tests_passed)
    {
        // verify that we are AmbiqSBL non-provisioned - and DM-LCS (read reg directly to verify)
        g_security_word = (*((volatile uint32_t*)(AM_REG_OTP_INFOC_SECURITY_ADDR))) & SECURITY_WORD_MASK;
        if ((g_security_word == 0x022000A0) && (CRYPTO->LCSREG_b.LCSREG == 0x1) )
        {
            am_hal_gpio_output_set(2);
        }
    }

    am_hal_gpio_output_set(1);
#endif

    am_util_stdio_printf("Compiler version: %s\n", COMPILER_VERSION);

#ifdef ENABLE_DEBUGGER
// #### INTERNAL BEGIN ####
    if (MCUCTRL->BOOTLOADER_b.SECBOOTFEATURE == MCUCTRL_BOOTLOADER_SECBOOTFEATURE_ENABLED)
// #### INTERNAL END ####
    {
        // Enable Debugger
        if (am_hal_dcu_update(true, DCU_DEBUGGER) != AM_HAL_STATUS_SUCCESS)
        {
            // Cannot enable Debugger
            am_util_stdio_printf("Could not enable debugger using DCU\n");
        }
        else
        {
            am_util_stdio_printf("Forcibly enabling debugger using DCU\n");
        }
    }
    MCUCTRL->DEBUGGER = 0;
#endif // ENABLE_DEBUGGER

}

//*****************************************************************************
//
// Begin test cases 
//
//*****************************************************************************
void
sbr_cleared_reg_test(void)
{
    // CLEARED REGISTERS TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = cleared_reg_test(&g_secModeLCS, g_info_buf);
    output_status("Cleared Register Check", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_dtcm_memory_test(void)
{
    // DTCM MEMORY TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = DTCM_Memory_Test(&g_secModeLCS, g_info_buf);
    output_status("DTCM memory", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_boot_status_code_test(void)
{
    // BOOT STATUS CHECK
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = boot_status_code(&g_secModeLCS, g_info_buf);
    output_status("Boot Status", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_debug_words_test(void)
{
    // SBR DEBUG WORDS
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = sbr_debug_words(&g_secModeLCS, g_info_buf);
    output_status("SBR Debug Words", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_sbr_lock_test(void)
{
    // SBR LOCK TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = sbr_lock_test(&g_secModeLCS, g_info_buf);
    output_status("SBR lock test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_sbl_lock_test(void)
{
    // SBL LOCK TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = sbl_lock_test(&g_secModeLCS, g_info_buf);
    output_status("SBL lock test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_dcu_lock_test(void)
{
    // DCU LOCK TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = dcu_lock_test(&g_secModeLCS, g_info_buf);
    output_status("DCU lock test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_dcu_enable_test(void)
{
    // DCU ENABLE TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = dcu_enables_test(&g_secModeLCS, g_info_buf);
    output_status("DCU Enables test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_vtor_test(void)
{
    // VTOR TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = vtor_test(&g_secModeLCS, g_info_buf);
    output_status("VTOR test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

#warning See CAYNSWS-1508 for updates to be made to sbr_bootrom_copy_test and sbr_helper_copy_test
// void
// sbr_bootrom_copy_test(void)
// {
//     // BOOTROM COPY PROTECTION TEST
//     uint32_t ui32status;
//     g_info_buf[0] = '\0';
//     ui32status = bootrom_copy_test(&g_secModeLCS, g_info_buf);
//     output_status("Bootrom Copy test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
//     TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
// }

// void
// sbr_helper_copy_test(void)
// {
//     // HELPER FUNCTION COPY PROTECTION TEST
//     uint32_t ui32status;
//     g_info_buf[0] = '\0';
//     ui32status = helper_copy_test(&g_secModeLCS, g_info_buf);
//     output_status("Helper ROM Copy Protection", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
//     TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
// }

void
sbr_helper_execute_test(void)
{
    // HELPER EXECUTION TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = helper_execute_test(&g_secModeLCS, g_info_buf);
    output_status("Helper Execution Test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_mcuctrl_protection_test(void)
{
    // MCUCTRL PROCTECTION TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = run_mcuctrl_protection_test(&g_secModeLCS, g_info_buf);
    output_status("MCUCTRL Protection Test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_publock_test(void)
{
    // PUBLOCK TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = run_publock_test(&g_secModeLCS, g_info_buf);
    output_status("PUBLOCK Test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_crypto_hostao_lock_test(void)
{
    // CRYPTO HOST AO LOCK BITS TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = run_crypto_hostao_lock_test(&g_secModeLCS, g_info_buf);
    output_status("CRYPTO HOST A0 LOCK BITS Test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_otp_oem_flag_test(void)
{
    // OTP OEM FLAG TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = run_otp_oem_flag_test(&g_secModeLCS, g_info_buf);
    output_status("OTP OEM Flag Test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_otp_nvcounters_test(void)
{
    // OTP NVCOUNTERS TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = run_otp_nvcounters_test(&g_secModeLCS, g_info_buf);
    output_status("OTP NV Counters Test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

void
sbr_ota_test(void)
{
    // OTA TEST
    uint32_t ui32status;
    g_info_buf[0] = '\0';
    ui32status = run_ota_test(&g_secModeLCS, g_info_buf);
    output_status("OTA Pointer Test", ui32status, &g_tests_run, &g_tests_passed, g_info_buf);
    TEST_ASSERT(ui32status == AM_STATUS_SUCCESS); 
}

// ****************************************************************************
//
// save_startup_regs() - save the initial registers and SPLIMs to the area above the startup SP
//      This function is called as the very first thing, it will dump all the registers
//      to the 16 words above the top of the stack (which was just loaded) so they are at known location
//      It then branches directly to the original program start function (Reset_Handler)
//
// ****************************************************************************

void __attribute__((naked)) save_startup_regs(void)
{
          __asm("  ADD   SP, SP,#0x44");    // move the stack up by 0x44 bytes
          __asm("  PUSH  {R0-R7}");         // push the registers
          __asm("  MOV   R0, R8");          // move the remaining registers
          __asm("  MOV   R1, R9");
          __asm("  MOV   R2, R10");
          __asm("  MOV   R3, R11");
          __asm("  MOV   R4, R12");
          __asm("  MRS   R5, MSPLIM");      // get the stack limit registers
          __asm("  MRS   R6, PSPLIM");
          __asm("  PUSH  {R0-R6, LR}");     // and push again
          __asm("  SUB   SP, SP,#0x44");    // set the stack back to its original location
          __asm("  B  Reset_Handler");      // branch to the original reset handler
}
