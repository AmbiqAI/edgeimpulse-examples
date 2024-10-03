//*****************************************************************************
//
//! @file am_devices_dc_jdi_sharp.c
//!
//! @brief Generic SHARP display driver with JDI interface. This
//! driver supports the display panel with model type LS014B7DD01.
//!
//! @addtogroup dc_xspi SHARP DC JDI Driver
//! @ingroup devices
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_devices_dc_jdi_sharp.h"
#include "am_util_delay.h"
#include "am_bsp.h"

#ifndef SIMULATION
#define DELAY am_util_delay_ms
#else
#define DELAY(...)
#endif

//
// the arguments of model type LS014B7DD01
//
#define LS014B7DD01_RESX                        280
#define LS014B7DD01_RESY                        280

#define LS014B7DD01_VCK_GCK_FF_MAX_FREQ         (0.5)       //500KHz
#define LS014B7DD01_HCK_BCK_MAX_FREQ            (0.758)     //0.758MHz

#ifdef AM_PART_APOLLO5A

#define LS014B7DD01_FRONTPORCH_X                1
#define LS014B7DD01_BLANKING_X                  6
#define LS014B7DD01_BACKPORCH_X                 1

#define LS014B7DD01_FRONTPORCH_Y                6
#define LS014B7DD01_BLANKING_Y                  2
#define LS014B7DD01_BACKPORCH_Y                 0

#define LS014B7DD01_BSP_OFFSET                  2
#define LS014B7DD01_BSP_WIDTH                   4

#define LS014B7DD01_GSP_ASSERTION               150
#define LS014B7DD01_GSP_WIDTH                   576

#define LS014B7DD01_GEN_ASSERTION               101
#define LS014B7DD01_GEN_HIGH_PLUSE              100

#define LS014B7DD01_INTB_ASSERTION              78
#define LS014B7DD01_INTB_HIGH_PLUSE             566

#else

#define LS014B7DD01_XRST_INTB_DELAY             1
#define LS014B7DD01_XRST_INTB_WIDTH             566

#define LS014B7DD01_VST_GSP_DELAY               (LS014B7DD01_XRST_INTB_DELAY + 72)
#define LS014B7DD01_VST_GSP_WIDTH               576

#define LS014B7DD01_VCK_GCK_DELAY               (LS014B7DD01_XRST_INTB_DELAY + 216)
#define LS014B7DD01_VCK_GCK_WIDTH               288
#define LS014B7DD01_VCK_GCK_CLOSING_PULSES      6

#define LS014B7DD01_HST_BSP_DELAY               2
#define LS014B7DD01_HST_BSP_WIDTH               4

#define LS014B7DD01_HCK_BCK_DATA_START          1

#define LS014B7DD01_ENB_GEN_WIDTH               90
#define LS014B7DD01_ENB_GEN_DELAY               (LS014B7DD01_VCK_GCK_WIDTH - LS014B7DD01_ENB_GEN_WIDTH) / 2

#endif

#define SHARP_VA_VCOM_DUTYCYCLE     50

static uint32_t sharp_timer_number = SHARP_VA_VCOM_TIMER;
//*****************************************************************************
//
// Initialize timer that generate VA,VCOM(VB) signals
//
//*****************************************************************************
static uint32_t
dc_jdi_pwm_init(uint32_t ui32TimerNum, am_hal_timer_clock_e eTimerClock, uint32_t ui32Frequency)
{
    uint64_t ui64BaseFreq, ui64EndCounts;
    am_hal_timer_config_t TimerConfig ;
    am_hal_timer_default_config_set( &TimerConfig ) ;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_PWM;
    TimerConfig.eInputClock = eTimerClock;
    //
    // Configure the TIMER.
    //
    am_hal_timer_config(ui32TimerNum, &TimerConfig);
    //
    // Config output VA pins
    //
    am_hal_timer_output_config(am_bsp_disp_jdi_timer_pins(0), AM_HAL_TIMER_OUTPUT_TMR0_OUT1 + ui32TimerNum * 2);
    //
    // Config output VCOM pins
    //
    am_hal_timer_output_config(am_bsp_disp_jdi_timer_pins(1), AM_HAL_TIMER_OUTPUT_TMR0_OUT0 + ui32TimerNum * 2);
// #### INTERNAL BEGIN ####
#ifdef APOLLO5_FPGA
    ui64BaseFreq = (APOLLO5_FPGA * 1000000ull) >> (2 * eTimerClock + 2);
#else
// #### INTERNAL END ####

    ui64BaseFreq = 96000000ull >> (2 * eTimerClock + 2);

// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
    ui64EndCounts = (uint64_t)(ui64BaseFreq / ui32Frequency + 0.5f);
    TimerConfig.ui32Compare0    = (uint32_t) ui64EndCounts;
    am_hal_timer_compare0_set(ui32TimerNum, TimerConfig.ui32Compare0);

    //am_util_stdio_printf("\nDuty cycle %d,\t Frequency %d\n", SHARP_VA_VCOM_DUTYCYCLE, ui32Frequency);
    //
    // this is the duty cycle computation for PWM mode
    //
    TimerConfig.ui32Compare1 = (uint32_t)(ui64EndCounts * (1 - SHARP_VA_VCOM_DUTYCYCLE / 100.0f) + 0.5f);
    am_hal_timer_compare1_set(ui32TimerNum, TimerConfig.ui32Compare1);

    sharp_timer_number = ui32TimerNum;
    return 0;
}

//*****************************************************************************
//
// Start PWM timer for JDI interface
//
//*****************************************************************************
void
am_devices_dc_jdi_sharp_timer_start(void)
{
    //
    // Enable the TIMER.
    //
    am_hal_timer_enable(sharp_timer_number);
    //
    // Start the TIMER.
    //
    am_hal_timer_start(sharp_timer_number);
}

//*****************************************************************************
//
// Stop PWM timer for JDI interface
//
//*****************************************************************************
void
am_devices_dc_jdi_sharp_timer_stop(void)
{
    //
    // Stop the TIMER.
    //
    am_hal_timer_clear_stop(sharp_timer_number);
    //
    // Disable the TIMER.
    //
    am_hal_timer_disable(sharp_timer_number);
}
//*****************************************************************************
//
// Initialize sharp JDI interface
//
//*****************************************************************************
uint32_t
am_devices_dc_jdi_sharp_init(am_devices_dc_jdi_timer_config_t *pTimerConfig,
                             nemadc_initial_config_t *pDCConfig)
{
    dc_jdi_pwm_init(pTimerConfig->ui32TimerNum, pTimerConfig->eTimerClock, pTimerConfig->ui32Frequency);

    pDCConfig->eInterface               = DISP_INTERFACE_JDI;
    pDCConfig->ui16ResX                 = LS014B7DD01_RESX;
    pDCConfig->ui16ResY                 = LS014B7DD01_RESY;

    pDCConfig->fHCKBCKMaxFreq           = LS014B7DD01_HCK_BCK_MAX_FREQ;         //BCK maximum frequency(MHz)

#ifdef AM_PART_APOLLO5A

    pDCConfig->ui32FrontPorchX          = LS014B7DD01_FRONTPORCH_X;
    pDCConfig->ui32BlankingX            = LS014B7DD01_BLANKING_X;
    pDCConfig->ui32BackPorchX           = LS014B7DD01_BACKPORCH_X;

    pDCConfig->ui32FrontPorchY          = LS014B7DD01_FRONTPORCH_Y;
    pDCConfig->ui32BlankingY            = LS014B7DD01_BLANKING_Y;
    pDCConfig->ui32BackPorchY           = LS014B7DD01_BACKPORCH_Y;

    pDCConfig->ui32HSTOffset            = LS014B7DD01_BSP_OFFSET;               //BSP offset
    pDCConfig->ui32HSTWidth             = LS014B7DD01_BSP_WIDTH;                //BSP width

    pDCConfig->ui32VSTAssertion         = LS014B7DD01_GSP_ASSERTION;            //GSP assertion
    pDCConfig->ui32VSTWidth             = LS014B7DD01_GSP_WIDTH;                //GSP width

    pDCConfig->ui32ENBAssertion         = LS014B7DD01_GEN_ASSERTION;            //GEN assertion
    pDCConfig->ui32ENBHighPulse         = LS014B7DD01_GEN_HIGH_PLUSE;           //GEN high pulse

    pDCConfig->ui32XRSTAssertion        = LS014B7DD01_INTB_ASSERTION;           //INTB assertion
    pDCConfig->ui32XRSTHighPulse        = LS014B7DD01_INTB_HIGH_PLUSE;          //INTB high pulse

#else

    pDCConfig->ui32XRSTINTBDelay        = LS014B7DD01_XRST_INTB_DELAY;          // Delay inserted prior of XRST or INTB in multiples of format_clk
    pDCConfig->ui32XRSTINTBWidth        = LS014B7DD01_XRST_INTB_WIDTH;          // Width of High state of XRST or INTB in multiples of format_clk

    pDCConfig->ui32VSTGSPDelay          = LS014B7DD01_VST_GSP_DELAY;            // Delay inserted prior of VST or GSP in multiples of format_clk
    pDCConfig->ui32VSTGSPWidth          = LS014B7DD01_VST_GSP_WIDTH;            // Width of High state of VST or GSP in multiples of format_clk

    pDCConfig->ui32VCKGCKDelay          = LS014B7DD01_VCK_GCK_DELAY;            // Delay inserted prior of VCK or GCK in multiples of format_clk
    pDCConfig->ui32VCKGCKWidth          = LS014B7DD01_VCK_GCK_WIDTH;            // Width of High state of VCK or GCK in multiples of format_clk
    pDCConfig->ui32VCKGCKClosingPulses  = LS014B7DD01_VCK_GCK_CLOSING_PULSES;   // Number of VCK or GCK pulses without ENB or GEN signal at the end of frame

    pDCConfig->ui32HSTBSPDelay          = LS014B7DD01_HST_BSP_DELAY;            // Delay inserted prior of HST or BSP in multiples of format_clk
    pDCConfig->ui32HSTBSPWidth          = LS014B7DD01_HST_BSP_WIDTH;            // Width of High state of HST or BSP in multiples of format_clk

    pDCConfig->ui32HCKBCKDataStart      = LS014B7DD01_HCK_BCK_DATA_START;       // The HCK or BCK cycle the pixel data should start at

    pDCConfig->ui32ENBGENDelay          = LS014B7DD01_ENB_GEN_DELAY;            // Delay inserted prior of ENB or GEN in multiples of format_clk
    pDCConfig->ui32ENBGENWidth          = LS014B7DD01_ENB_GEN_WIDTH;            // Width of High state of ENB or GEN in multiples of format_clk

    pDCConfig->fVCKGCKFFMaxFreq         = LS014B7DD01_VCK_GCK_FF_MAX_FREQ;      //GCK maximum frequency(MHz)

#endif
    return 0;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
