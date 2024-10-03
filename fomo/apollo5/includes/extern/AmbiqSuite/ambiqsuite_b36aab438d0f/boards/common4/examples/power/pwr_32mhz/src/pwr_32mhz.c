//*****************************************************************************
//
//! @file pwr_32mhz.c
//!
//! @brief This demonstrates using the Apollo4 32Mhz XTHS and HFRC2
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup pwr_32mhz 32Mhz XTHS and HFRC2 Power Example
//! @ingroup power_examples
//! @{
//!
//! Purpose: This example shows how to:
//!  1. Enable 32Mhz XTHS.
//!  2. Enable 32Mhz clockout.
//!  2. Enable HFRC2 clockout.
//!  4. Enable HFRC2:
//!     a. Enable normal HFRC2 with FLL (HFRC2-adjust)
//!     b. Use HFRC2 tune where the FLL is disabled, and HFRC2 adjust is run periodically.
//!     c. Disable the XTHS once the FLL is disabled
//!
//! Default HFRC2 will output a default frequency based on a calibrated tuned value.
//! HFRC2 adjust will use the 32Mhz XTHS and an FLL to generate the
//! user requested Frequency. However the FLL can add some frequency jitter.
//!
//! HFRC2 tune, will use HFRC2 adjust to periodically compute a new tune value
//! This process takes about 1msec. Once the tune value is computed,HFRC2 adjust
//! (the FLL) is disabled and the tune value is used to generate the HFRC2 freq.
//! Using HFRC2-tune enables the HFRC2 to run with reduced jitter and reduced power use.
//!
//! When the HFRC2 FLL is off, the XTHS may optionally be disabled (if it isn't used elsewhere)
//! Disabling the 32Mhz XTHS will use less power. A delay is necessary and built into the
//! HAL to allow time for the 32Mhz XTHS to start before starting HFRC2 FLL.
//!
//! A logic analyzer or oscilloscope can be attached to the CLOCKOUT pins
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
#include "pwr_control_utils.h"
#include "pwr_gpio_utils.h"
#include "pwr_timer_utils.h"

#define ENABLE_32MHZ_XTHS   1

//
//! Enable 32Mhz clockout pin. For the Apollo4 Family, this is always on GPIO 46.
//! @note This GPIO is not available on Apollo4 Blue, Apollo4 Blue Plus,
//! or Apollo4 Blue Lite packages, where it is used for the BLE sub-system high
//! frequency clock
//
#define ENABLE_32MHZ_CLK_OUT 0
#if (ENABLE_32MHZ_CLK_OUT != 0) && (defined(AM_BSP_BLE) && (AM_BSP_BLE != 0))
#error "GPIO 46 not available for output on BLE devices"
#endif

#define ENABLE_PRINT_DBG     0 //!< print debug messages

#define ENABLE_HFRC2         1 //!< enable HFRC2
//
//! enable HFRC2 tune (where the FLL is usually disabled,
//! but the tune algorithm is called periodically to adjust the
//! HFRC2 output frequency.
//! The XTHS must be running to "tune" HFRC2
//! @note ENABLE_HFRC2_TUNE is not supported with Apollo4b.
//
#define ENABLE_HFRC2_TUNE    0

#define ENABLE_HFRC2_CLKOUT  0 //!< Enable a clock-out pin for HFRC2

#define ENABLE_32MHZ_CLOCKOFF 0 //!< Disable XTHS when the HFRC2 FLL is disabled

#define HFRC2_RECOMPUTE_RATE 10 //!< number of seconds HFRC2 re-comptue will be called


#if defined(AM_PART_APOLLO4B)
#if ENABLE_HFRC2_TUNE == 1
#warning  "ENABLE_HFRC2_TUNE" not supported with Apoll4b
#undef ENABLE_HFRC2_TUNE
#define ENABLE_HFRC2_TUNE 0
#endif // if ENABLE_HFRC2_TUNE == 1
#define HFRC2_RECOMPUTE_ALLOWED 0
#else
#define HFRC2_RECOMPUTE_ALLOWED 1
#endif

#if ENABLE_PRINT_DBG == 0
#define DEBUG_PRINT(args...)
#else
#define DEBUG_PRINT(args...) am_util_stdio_printf(args)
#endif


extern const am_hal_gpio_pincfg_t gpio_pincfg_clkout33;
extern const am_hal_gpio_pincfg_t g_gpio_default_output_cfg;
extern am_hal_gpio_pincfg_t g_devices_32M_clk;
extern const am_hal_gpio_pincfg_t gpio_pincfg_xtalHsOut46;
static void timerCallback(void);
static uint32_t hfrc2_init(void);

//
//! struct for local ram variables used in this file
//
typedef struct
{
    volatile uint32_t timerCount;  //!< Incremented in timer callback.
    volatile bool bTimerFlag;      //!< Set in timer callback.
    uint32_t ui32RecomputeCount;   //!< Increment at 1Hz until matches HFRC2_RECOMPUTE_RATE
}
local_globals_t;

//
//! allocate ram variables used in this file
//
static local_globals_t g_tGlb;


//*****************************************************************************
//
//! @brief     Timer callback
//!
//! @note This is called from the timer ISR, when the timer expires
//!
//! @param x   Unused
//
//*****************************************************************************
static void
timerCallback(void)
{
    g_tGlb.timerCount++;
    g_tGlb.bTimerFlag = true;
}


//*****************************************************************************
//
//! @brief Start the HFRC2 clock (if enabled)
//!
//! @return standard hal status
//
//*****************************************************************************
static uint32_t
hfrc2_init(void)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;

#if ENABLE_HFRC2 == 1
    do
    {
        //
        // enable HFRC2
        //
        ui32Status = am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFRC2_START, false);
        am_util_delay_us(200);      // wait for FLL to lock
        if (ui32Status)
        {
            break;
        }

        //
        // set HF2ADJ for 24.576MHz output
        //
        ui32Status = am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HF2ADJ_ENABLE, false);
        am_util_delay_us(500);      // wait for adj to apply
        if (ui32Status)
        {
            break;
        }


#if ENABLE_HFRC2_CLKOUT == 1
        //
        // turn on HFRC2 output on pin33
        //
        gpio_enable_hfrc2_clk_output(AM_HAL_CLKGEN_CLKOUT_HFRC2_24M, 33);
#endif


    } while ( false );
#endif

    return ui32Status;

}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Status;

#if HFRC2_RECOMPUTE_ALLOWED == 1
#if ENABLE_32MHZ_CLOCKOFF == 1
    am_hal_clockgen_hf2adj_recompute_t hfrcControl = tReComputeCtrlNo32MhzDefault;
#else
    am_hal_clockgen_hf2adj_recompute_t hfrcControl = tReComputeCtrlDefault;
#endif // ENABLE_32MHZ_CLOCKOFF
    am_hal_mcuctrl_control_arg_t *p_mcuctrl = &hfrcControl.mcu_ctrl_xths_controlArg;
#else
    am_hal_mcuctrl_control_arg_t tMcuCtrl;
    am_hal_mcuctrl_control_arg_t *p_mcuctrl = &tMcuCtrl;
#endif // HFRC2_RECOMPUTE_ALLOWED



#if ENABLE_PRINT_DBG == 1
    //
    // Initialize the printf interface for ITM output
    //
    if ( am_bsp_debug_printf_enable() )
    {
        //
        // Cannot print - Something wrong.
        //
        while(1);
    }
#endif

    ui32Status = pwrctrl_set_low_power();
    if (ui32Status)
    {
        while(1);
    }

    timer_init(TMR_TICKS_PER_SECOND, timerCallback);

    am_hal_interrupt_master_enable();

#if ENABLE_PRINT_DBG == 1

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("CLKOUT 32Mhz example...\n");
#endif

    *p_mcuctrl = g_amHalMcuctrlArgDefault; // struct copy

#if ENABLE_32MHZ_CLK_OUT == 1
    //
    // configure pin 46 for clockout
    // this is always pin 46, it can't be reassigned
    //
    am_hal_gpio_pinconfig(46, g_devices_32M_clk);

    //
    // This will enable HFXTAL clockout in the hal
    //
    p_mcuctrl->b_arg_enable_HfXtalClockout = true;
#endif

    // get 32Mhz running with 32Mhz clock out enabled
#if ENABLE_32MHZ_XTHS != 0
    ui32Status = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, (void *) p_mcuctrl);
    if (ui32Status)
    {
        DEBUG_PRINT( "am_hal_mcuctrl_control returned error %d\nThis program will now hang", ui32Status);
        while(1);
    }
#endif

    ui32Status = hfrc2_init();
    if (ui32Status)
    {
        DEBUG_PRINT( "hfrc2_init returned error %d\nThis program will now hang", ui32Status);
        while(1);
    }

#if ENABLE_HFRC2_TUNE == 1
    am_util_delay_ms(500); // allow full startup

    //
    // do a tune cycle before starting the loop
    //
    ui32Status = am_hal_clkgen_HFRC2_adj_recompute(&hfrcControl);
    if (ui32Status)
    {
        DEBUG_PRINT("hfrc2 adjust returned error %ld\nThis program will hang", ui32Status);
        while(1);
    }
#endif // ENABLE_HFRC2_TUNE

    while (true)
    {
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

        if (g_tGlb.bTimerFlag)
        {
            g_tGlb.bTimerFlag = false;

#if ENABLE_HFRC2_TUNE == 1
            if (++g_tGlb.ui32RecomputeCount >= HFRC2_RECOMPUTE_RATE)
            {
                g_tGlb.ui32RecomputeCount = 0;
                ui32Status = am_hal_clkgen_HFRC2_adj_recompute(&hfrcControl);
                if (ui32Status)
                {
                    DEBUG_PRINT("hfrc2 adjust returned error %ld\nThis program will hang", ui32Status);
                    while (1);
                }
                DEBUG_PRINT("hfrc2 adjust complete\n");
            }
#endif // ENABLE_HFRC2_TUNE == 1

        } // if (g_tGlb.bTimerFlag)
    } // while (true)

    // return 0; // statement unreachable
}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

