//*****************************************************************************
//
//! @file hello_world.c
//!
//! @brief A simple "Hello World" example.
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

//*****************************************************************************
//
// Globals.
//
//*****************************************************************************
#define NUM_TRIALS             100         // Number of crypto power cycles before the test fails
#define LOW_POWER_INIT         0          // Call the low_power_init function
#define ENABLE_PRINT_EARLY     1          // Enable printf as the *first* thing in the program (otherwise happens after cachectrl)
#define ENABLE_CACHE           1          // Enable the cachectrl (don't use with ASM sequence)
#define ENABLE_POI             1          // POI at the end of N trials.

#define MANUAL_POWER_SEQUENCE  1          // Use one of the following sequences instead of the standard pwrctrl function.
#define BRENDAN_SEQUENCE       0          // Use Brendan's sequence to control the crypto power sequence (can't use with ASM)
#define ASM_POWER_SEQUENCE     1          // Use the ASM sequence to control the crypto power. (ASM uses pwrctrl register)
#define NULL_SEQUENCE          0          // Don't actually turn the crypto on or off.
#define USE_WFI                1          // Use WFI as part of asm power sequence

#define WAKE_INTERVAL          100        // How often STIMER ticks (1000 = 166us)
#define POWER_CYCLE_DELAY_MS   0          // Time to delay after crypto power-up/down

#define NO_INIT_COUNTER        0x400201C4 // Used to track how many times the test has done a POI. Must be reset manually (power cycle should do it).
                                          // 0x400201C4 is SCRATCH1.

#define ENABLE_MARKERS         1          // Write approximate code location markers to 0x10060000 (Beginning of SSRAM)

#define RUN_CRYPTO_OPERATIONS  1          // Run crypto operations between power-cycles.
#define RUN_SECONDARY_TASK     0          // Do something else to stress the system during crypto operations.
#define SECONDARY_TASK_DELAY   10         // How often to run the secondary task (in us)
#define POI_DELAY              0          // Milliseconds to wait before doing POI (slows the test down to help debuggers connect)
#define GPIO_PAUSE             1          // If set, stop the test when GPIO1 is HIGH.
#define CHECK_STIMER_DELTA     1          // Record STIMER value and delta during powerup/powerdown

// Note: If you use the ASM sequence and WFI, you need to check the WFI
// instructions to make sure they're within a 16-byte cache line of the STR
// instruction that controls crypto power. You can do this by either reading
// the disassembly file, or by running with a debugger, setting breakpoints on
// each wfi, and making sure the preceding str instruction has the same digit in
// the second-to-last position in its address. For example:
//
// 0x8080: STR R1 [R0]  <- Good
// 0x8082: WFI
//
// 0x802E: STR R1 [R0]  <- Bad
// 0x8030: WFI
//
// You can fix this by inserting NOPs and recompiling. We hope to eliminate this
// problem in future test iterations.

//*****************************************************************************
//
// Error macro
//
//*****************************************************************************
#define HALT_ON_ERROR(x, message) \
    if ((x) != AM_HAL_STATUS_SUCCESS) { am_util_stdio_printf(message); while (1); }

//******************************************************************************
//
// Helper macros.
//
//******************************************************************************

//
// Globals for STIMER tracking
//
#if CHECK_STIMER_DELTA
volatile uint32_t g_ui32STIMERValue = 0;
volatile uint32_t g_ui32STIMERCompare = 0;
volatile uint32_t g_ui32STIMERInterruptCount = 0;
volatile uint32_t g_ui32STIMERLastIntCount = 0;

#define RECORD_STIMER_VALUES()            \
{                                         \
    AM_CRITICAL_BEGIN;                    \
    g_ui32STIMERValue = STIMER->STTMR;    \
    g_ui32STIMERCompare = STIMER->SCMPR0; \
    g_ui32STIMERLastIntCount = g_ui32STIMERInterruptCount;    \
    AM_CRITICAL_END;                      \
}

#endif // CHECK_STIMER_DELTA

//
// Manual power sequence
//
#if MANUAL_POWER_SEQUENCE
#define CRYPTO_UP()                                                              \
    {                                                                            \
        am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);                       \
        RECORD_STIMER_VALUES();                                                  \
        crypto_powerup();                                                        \
    }
#define CRYPTO_DOWN()                                                            \
    {                                                                            \
        am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);                       \
        RECORD_STIMER_VALUES();                                                  \
        crypto_powerdown();                                                      \
    }
#else
#define CRYPTO_UP()                                                              \
    {                                                                            \
        uint32_t _ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_CRYPTO); \
        HALT_ON_ERROR(_ui32Status, "Enable failed\n");                            \
    }
#define CRYPTO_DOWN()                                                            \
    {                                                                            \
        uint32_t _ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);\
        HALT_ON_ERROR(_ui32Status, "Disable failed\n");                           \
    }
#endif

//
// Crypto operations
//
#if RUN_CRYPTO_OPERATIONS
#define RUN_CRYPTO_UP()                                                         \
    {                                                                           \
        CRYPTO_UP();                                                            \
        mbedtls_mng_resume (0, 0);                                              \
    }
#define RUN_CRYPTO_DOWN()                                                       \
    {                                                                           \
        mbedtls_mng_suspend(0, 0);                                              \
        CRYPTO_DOWN();                                                          \
    }
#else
#define RUN_CRYPTO_UP()        CRYPTO_UP()
#define RUN_CRYPTO_DOWN()      CRYPTO_DOWN()
#endif

//
// Power cycle delays (between individual power-up/power-down calls.
//
#if POWER_CYCLE_DELAY_MS
#define POWER_CYCLE_DELAY()     am_util_delay_ms(POWER_CYCLE_DELAY_MS);
#else
#define POWER_CYCLE_DELAY()
#endif

//
// Printing settings.
//
#define PRINT_TEST_SETTING(x) \
    am_util_stdio_printf(#x": %d\n", x);

#define PRINT_TEST_SETTINGX(x) \
    am_util_stdio_printf(#x": 0x%08X\n", x);

#define PRINT_ALL_SETTINGS()                   \
{                                              \
    PRINT_TEST_SETTING(NUM_TRIALS);            \
    PRINT_TEST_SETTING(LOW_POWER_INIT);        \
    PRINT_TEST_SETTING(ENABLE_PRINT_EARLY);    \
    PRINT_TEST_SETTING(ENABLE_CACHE);          \
    PRINT_TEST_SETTING(ENABLE_POI);            \
    PRINT_TEST_SETTING(MANUAL_POWER_SEQUENCE); \
    PRINT_TEST_SETTING(BRENDAN_SEQUENCE);      \
    PRINT_TEST_SETTING(ASM_POWER_SEQUENCE);    \
    PRINT_TEST_SETTING(USE_WFI);               \
    PRINT_TEST_SETTING(WAKE_INTERVAL);         \
    PRINT_TEST_SETTING(POWER_CYCLE_DELAY_MS);  \
    PRINT_TEST_SETTINGX(NO_INIT_COUNTER);      \
    PRINT_TEST_SETTING(ENABLE_MARKERS);        \
    PRINT_TEST_SETTING(RUN_CRYPTO_OPERATIONS); \
    PRINT_TEST_SETTING(POI_DELAY);             \
    PRINT_TEST_SETTING(RUN_SECONDARY_TASK);    \
    PRINT_TEST_SETTING(GPIO_PAUSE);            \
    am_util_stdio_printf("\n\n");              \
}

//
// Secondary task
//
#if RUN_SECONDARY_TASK
//******************************************************************************
//
// Global for secondary task.
//
//******************************************************************************
uint32_t g_ui32TimerCount = 0;

//******************************************************************************
//
// Prepare a secondary task for the core to do periodically.
//
//******************************************************************************
void
secondary_task_init(void)
{
    am_hal_timer_config_t sTimerConfig;

    am_hal_timer_default_config_set(&sTimerConfig);
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    sTimerConfig.ui32PatternLimit = 0;
    sTimerConfig.ui32Compare1 = (6 * SECONDARY_TASK_DELAY);  // Default clock is 6MHz

    if (AM_HAL_STATUS_SUCCESS != am_hal_timer_config(0, &sTimerConfig))
    {
        am_util_stdio_printf("Failed to configure TIMER0\n");
    }

    am_hal_timer_clear(0);
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));

    g_ui32TimerCount = 0;
}

//******************************************************************************
//
// Enable the secondary task
//
//******************************************************************************
void
secondary_task_enable(void)
{
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
    NVIC_SetPriority(TIMER_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(TIMER_IRQn);
    am_hal_timer_start(0);
}

//******************************************************************************
//
// Disable the secondary task
//
//******************************************************************************
void
secondary_task_disable(void)
{
    am_hal_timer_stop(0);
    NVIC_DisableIRQ(TIMER_IRQn);
    am_hal_timer_interrupt_disable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
}

//******************************************************************************
//
// Final tasks for the secondary task before reset.
//
//******************************************************************************
void
secondary_task_complete(void)
{
    am_util_stdio_printf("Secondary task count: %d\n", g_ui32TimerCount);
}

//******************************************************************************
//
// TIMER ISR
//
//******************************************************************************
void
am_ctimer_isr(void)
{
    //
    // Clear TimerA0 Interrupt (write to clear).
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
    am_hal_timer_clear(0);

    g_ui32TimerCount++;

} // am_ctimer_isr()

#define SECONDARY_TASK_INIT() secondary_task_init()
#define SECONDARY_TASK_ENABLE() secondary_task_enable()
#define SECONDARY_TASK_DISABLE() secondary_task_disable()
#define SECONDARY_TASK_COMPLETE() secondary_task_complete()

#else // RUN_SECONDARY_TASK

#define SECONDARY_TASK_INIT()
#define SECONDARY_TASK_ENABLE()
#define SECONDARY_TASK_DISABLE()
#define SECONDARY_TASK_COMPLETE()

#endif // RUN_SECONDARY_TASK

//******************************************************************************
//
// Write markers in SRAM to show where the code is.
//
//******************************************************************************
#if ENABLE_MARKERS
#define MARK_LOCATION(x) \
    AM_REGVAL(0x10060000) = x
#else
#define MARK_LOCATION(x)
#endif

#if NULL_SEQUENCE
static void
crypto_powerup(void)
{
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);
#if USE_WFI
    __asm("wfi");
#endif

    //((pfn)((uint8_t *)crypto_powerup_asm + 1))();
}

static void
crypto_powerdown(void)
{
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);
#if USE_WFI
    __asm("wfi");
#endif

    //((pfn)((uint8_t *)crypto_powerdown_asm + 1))();
}
#endif

#if BRENDAN_SEQUENCE
static void crypto_powerup(void)
{
    setAIKey(true);
    AM_REGVAL(0x400403DC) &= ~0x00000010; // AM_BFW(MCUCTRL, DEVICEPGCTRL1, CRYPTOISOOR,   0);  // Assert isolation
    AM_REGVAL(0x400403DC) &= ~0x00000020; // AM_BFW(MCUCTRL, DEVICEPGCTRL1, CRYPTORESETOR, 0);  // Assert reset
    AM_REGVAL(0x400403DC) |= 0x00000008; // AM_BFW(MCUCTRL, DEVICEPGCTRL1, CRYPTOPGOR,    1);  // Power down (PG)
    AM_REGVAL(0x400403DC) &= ~0x80000000; // AM_BFW(MCUCTRL, DEVICEPGCTRL1, DEVICEPGOVER1, 0);  // Enable override
    setAIKey(false);
}

static void crypto_powerdown(void)
{
    setAIKey(true);
    AM_REGVAL(0x400403DC) = 0x09;  //     AM_REGWR(MCUCTRL, DEVICEPGCTRL1, 0x09);  // Match register defaults - device PG (power-gated)
    AM_REGVAL(0x400403DC) |= 0x80000000; // AM_BFW(MCUCTRL, DEVICEPGCTRL1, DEVICEPGOVER1, 1);  // Enable override
    AM_REGVAL(0x400403DC) &= ~0x00000008; // AM_BFW(MCUCTRL, DEVICEPGCTRL1, CRYPTOPGOR,    0);  // Power up (!PG)
    AM_REGVAL(0x400403DC) |= 0x00000020; // AM_BFW(MCUCTRL, DEVICEPGCTRL1, CRYPTORESETOR, 1);  // Deassert reset
    AM_REGVAL(0x400403DC) |= 0x00000010; // AM_BFW(MCUCTRL, DEVICEPGCTRL1, CRYPTOISOOR,   1);  // Deassert isolation
    setAIKey(false);
}
#endif

#if MANUAL_POWER_SEQUENCE
#if ASM_POWER_SEQUENCE
#if 1
// This typedef is defined to ensure 8B alignment
typedef union
{
    uint64_t dummy[3];
    uint16_t code[12];
} EightBAlignedCode_t;

const EightBAlignedCode_t crypto_powerup_asm =
{
    .code =
    {
        0xf241, 0x0004,
        0xf2c4, 0x0002,
        0x6801,
        0xf451, 0x1180,
        0xbf00, // NOP inserted to ensure alignment
        0x6001,
        0xbf30,
        0x4770,
    }
};

const EightBAlignedCode_t crypto_powerdown_asm =
{
    .code =
    {
        0xf241, 0x0004,
        0xf2c4, 0x0002,
        0x6801,
        0xf431, 0x1180,
        0xbf00, // NOP inserted to ensure alignment
        0x6001,
        0xbf30,
        0x4770,
    }
};

typedef void (*voidfn_t)(void);
voidfn_t crypto_powerup = (voidfn_t)((uint8_t *)&crypto_powerup_asm + 1);
voidfn_t crypto_powerdown = (voidfn_t)((uint8_t *)&crypto_powerdown_asm + 1);

#else

#define DEVPWREN_REG 0x40021004
#define CRYPTO_PWR_BIT (1 << 20)

static void
crypto_powerup(void)
{
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);

    //AM_REGVAL(0x40021004) |= CRYPTO_PWR_BIT;

    __asm("movw r0, %L0 \n"
          "movt r0, %H0 \n" : :"i"(0x40021004));
    __asm("ldr r1, [r0]");
    __asm("orrs.w r1, r1, #0x100000");
    __asm("nop");
    __asm("nop");
    __asm("str r1, [r0]");
#if USE_WFI
    __asm("wfi");
#endif

    //((pfn)((uint8_t *)crypto_powerup_asm + 1))();
}

static void
crypto_powerdown(void)
{
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);

    //AM_REGVAL(0x40021004) &= ~(CRYPTO_PWR_BIT);

    __asm("movw r0, %L0 \n"
          "movt r0, %H0 \n" : :"i"(0x40021004));
    __asm("ldr r1, [r0]");
    __asm("bics.w r1, r1, #0x100000");
    __asm("nop");
    __asm("nop");
    __asm("str r1, [r0]");
#if USE_WFI
    __asm("wfi");
#endif

    //((pfn)((uint8_t *)crypto_powerdown_asm + 1))();
}
#endif
#endif

//*****************************************************************************
//
// Timer Interrupt Service Routine (ISR)
//
//*****************************************************************************
void
am_stimer_cmpr0_isr(void)
{
    //
    // Check the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
//    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);

#if CHECK_STIMER_DELTA
    g_ui32STIMERInterruptCount++;
#endif

}

//*****************************************************************************
//
// Init function for the STimer.
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
    //am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);
    am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ |
                         AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);

}

#endif

#if RUN_CRYPTO_OPERATIONS
//******************************************************************************
//
// Runtime crypto functions.
//
//******************************************************************************
#include "config-cc312-apollo4-no-os.h"

#include "cc_lib.h"

#include "mbedtls/rsa.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/memory_buffer_alloc.h"
#include "mbedtls_cc_mng.h"
#include "crypto_test_data.h"
#include "rsa_keys.h"

//
// Dynamic memory for mbedTLS stack.
//
#define AM_MBEDTLS_HEAP_SIZE (128*1024/4)
uint32_t g_ui32MbedTLSHeap[AM_MBEDTLS_HEAP_SIZE];

//
// Context variables for mbedTLS operations.
//
mbedtls_rsa_context rsa;
CCRndContext_t RndContex;
CCRndWorkBuff_t RndWorkBuff;
mbedtls_ctr_drbg_context RndState;
mbedtls_entropy_context MbedtlsEntropy;

//******************************************************************************
//
// Initialize crypto test
//
//******************************************************************************
void
crypto_test_init(void)
{
    /* init Rnd context's inner member */
    RndContex.rndState = &RndState;
    RndContex.entropyCtx = &MbedtlsEntropy;

    MARK_LOCATION(0x2001);
    CRYPTO_UP();

    MARK_LOCATION(0x2002);
    mbedtls_memory_buffer_alloc_init((uint8_t*)g_ui32MbedTLSHeap, AM_MBEDTLS_HEAP_SIZE);
    MARK_LOCATION(0x2003);
    CC_LibInit(&RndContex, &RndWorkBuff);

    MARK_LOCATION(0x2004);
    RUN_CRYPTO_DOWN();
}

void
crypto_test_operation(void)
{
    MARK_LOCATION(0x1001);

    //
    // Initialize the RSA module, and import our keys.
    //
    mbedtls_rsa_init(&rsa, MBEDTLS_RSA_PKCS_V15, 0);
    mbedtls_mpi_read_string(&rsa.N, 16, PUBLIC_RSA_N);
    mbedtls_mpi_read_string(&rsa.E, 16, PUBLIC_RSA_E);

    MARK_LOCATION(0x1002);
    rsa.len = ( mbedtls_mpi_bitlen( &rsa.N ) + 7 ) >> 3;

    if (mbedtls_rsa_check_pubkey(&rsa))
    {
        MARK_LOCATION(0x1003);
        while (1);
    }

    //
    // Run SHA256 on our binary, and then compare against the previously calculated hash.
    //
    uint8_t digest[TEST_HASH_SIZE];
    MARK_LOCATION(0x1004);
    mbedtls_sha256(pui8TestBinary, TEST_BIN_SIZE, digest, false);
    MARK_LOCATION(0x1005);

    uint32_t i;
    uint32_t ui32HashErrors = 0;
    for (i = 0; i < TEST_HASH_SIZE; i++)
    {
        MARK_LOCATION(0x1006);
        if (digest[i] != pui8TestHash[i])
        {
            ui32HashErrors++;
        }
    }

    //
    // Print our SHA256 results.
    //
    if (ui32HashErrors == 0)
    {
        MARK_LOCATION(0x1007);
        am_util_stdio_printf("SHA256 values match.\n");
    }
    else
    {
        MARK_LOCATION(0x1008);
        am_util_stdio_printf("Error, %d mismatches found in SHA256.\n", ui32HashErrors);
        while (1);
    }

    //
    // Attempt to verify the provided signature using our public key.
    //
    MARK_LOCATION(0x1009);
    uint32_t result = mbedtls_rsa_pkcs1_verify(&rsa,
                                               NULL,
                                               NULL,
                                               MBEDTLS_RSA_PUBLIC,
                                               MBEDTLS_MD_SHA256,
                                               32,
                                               digest,
                                               pui8TestSignature);

    //
    // Release the RSA context. We're done with RSA operations.
    //
    MARK_LOCATION(0x100A);
    mbedtls_rsa_free(&rsa);

    //
    // Print our RSA results.
    //
    if (result == 0)
    {
        MARK_LOCATION(0x100B);
        am_util_stdio_printf("RSA signature verified.\n", result);
    }
    else
    {
        MARK_LOCATION(0x100C);
        am_util_stdio_printf("RSA Error: 0x%08X\n", result);
        while (1);
    }

}
#else
#define crypto_test_init()
#define crypto_test_operation()
#endif

//*****************************************************************************
//
// Main function
//
//*****************************************************************************
int
main(void)
{
    AM_REGVAL(NO_INIT_COUNTER) += 1;

    MARK_LOCATION(0);

    //
    // Enable printing
    //
#if ENABLE_PRINT_EARLY
    am_bsp_debug_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Crypto power test POI# %d running from 0x%x\n\n", AM_REGVAL(NO_INIT_COUNTER), AM_REGVAL(0xE000ED08));
#endif

    MARK_LOCATION(1);

#if MANUAL_POWER_SEQUENCE
    //
    // stimer init.
    //
    stimer_init();
#endif

    MARK_LOCATION(2);

#if LOW_POWER_INIT
    am_bsp_low_power_init();
#else
    CRYPTO_DOWN();
#endif

    MARK_LOCATION(3);

    //
    // Set the default cache configuration
    //
#if ENABLE_CACHE
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();
#endif

    MARK_LOCATION(4);

#if !ENABLE_PRINT_EARLY
    am_bsp_debug_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Crypto power test POI# %d\n\n", AM_REGVAL(NO_INIT_COUNTER));
#endif

    PRINT_ALL_SETTINGS();

    MARK_LOCATION(5);

    //
    // Enable the timer interrupt in the NVIC.
    //
    am_hal_interrupt_master_enable();

    MARK_LOCATION(6);

    crypto_test_init();

    SECONDARY_TASK_INIT();

    MARK_LOCATION(7);

    for (uint32_t i = 0; i < NUM_TRIALS; i++)
    {
        MARK_LOCATION(8);

        am_util_stdio_printf("******** Trial %d ********\n", i);

        MARK_LOCATION(9);
        RUN_CRYPTO_UP();

        SECONDARY_TASK_ENABLE();

        MARK_LOCATION(10);
        POWER_CYCLE_DELAY();

        MARK_LOCATION(11);
        crypto_test_operation();

        SECONDARY_TASK_DISABLE();

        MARK_LOCATION(12);
        RUN_CRYPTO_DOWN();

        MARK_LOCATION(13);
        POWER_CYCLE_DELAY();

        MARK_LOCATION(14);
        am_util_stdio_printf("Complete\n");
    }

    MARK_LOCATION(15);
    am_util_stdio_printf("Test complete.\n");

    MARK_LOCATION(16);
    SECONDARY_TASK_COMPLETE();

#if NO_INIT_COUNTER
    am_util_stdio_printf("POI COUNT: %d\n", AM_REGVAL(NO_INIT_COUNTER));
#endif

#if GPIO_PAUSE
    am_hal_gpio_pincfg_t sInputConfig = AM_HAL_GPIO_PINCFG_INPUT;
    am_hal_gpio_pinconfig(1, sInputConfig);

    uint32_t ui32PausePin = 0;
    am_hal_gpio_state_read(1, AM_HAL_GPIO_INPUT_READ, &ui32PausePin);

    if (ui32PausePin)
    {
        am_util_stdio_printf("Pausing (GPIO high).\n");
        while (1);
    }
    else
    {
        am_util_stdio_printf("Resetting (POI)\n");
    }
#endif

#if ENABLE_POI
#if POI_DELAY
    am_util_delay_ms(POI_DELAY);
#endif
    AM_REGVAL(0x40000004) = 0x1B;
#endif

    while (1);
}
