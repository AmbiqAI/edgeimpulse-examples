//*****************************************************************************
//
//  am_hal_clkgen.h
//! @file
//!
//! @brief Functions for accessing and configuring the CLKGEN.
//!
//! @addtogroup clkgen4 Clock Generator (CLKGEN)
//! @ingroup apollo4hal
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
#ifndef AM_HAL_CLKGEN_H
#define AM_HAL_CLKGEN_H

#ifdef __cplusplus
extern "C"
{
#endif


//
// Designate this peripheral.
//
#define AM_APOLLO3_CLKGEN   1

//*****************************************************************************
//
//! @name System Clock max frequency
//! @brief Defines the maximum clock frequency for this device.
//!
//! These macros provide a definition of the maximum clock frequency.
//!
//! @{
//
//*****************************************************************************
// #### INTERNAL BEGIN ####
#if 0//APOLLO4_LOWPOWER
#define AM_HAL_CLKGEN_FREQ_MAX_HZ           24000000
#elif  0 //APOLLO4_BURST
#define AM_HAL_CLKGEN_FREQ_MAX_HZ           192000000
#else
// #### INTERNAL END ####
#ifdef APOLLO4_FPGA
#define AM_HAL_CLKGEN_FREQ_MAX_HZ           (APOLLO4_FPGA * 1000000)
#else // APOLLO4_FPGA
#define AM_HAL_CLKGEN_FREQ_MAX_HZ           96000000
#endif // APOLLO4_FPGA
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
#define AM_HAL_CLKGEN_FREQ_MAX_KHZ      (AM_HAL_CLKGEN_FREQ_MAX_HZ / 1000)
#define AM_HAL_CLKGEN_FREQ_MAX_MHZ      (AM_HAL_CLKGEN_FREQ_MAX_HZ / 1000000)
#define AM_HAL_CLKGEN_CORESEL_MAXDIV    1
//! @}

//
// Control operations.
//
typedef enum
{
    AM_HAL_CLKGEN_CONTROL_XTAL_START,
    AM_HAL_CLKGEN_CONTROL_LFRC_START,
    AM_HAL_CLKGEN_CONTROL_XTAL_STOP,
    AM_HAL_CLKGEN_CONTROL_LFRC_STOP,
    AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL,
    AM_HAL_CLKGEN_CONTROL_RTC_SEL_LFRC,
    AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE,
    AM_HAL_CLKGEN_CONTROL_HFADJ_DISABLE,
    AM_HAL_CLKGEN_CONTROL_XTAL24M_ENABLE,
    AM_HAL_CLKGEN_CONTROL_XTAL24M_DISABLE,
    AM_HAL_CLKGEN_CONTROL_XTAL24MDS_0,      // Drive strength
    AM_HAL_CLKGEN_CONTROL_XTAL24MDS_1,
    AM_HAL_CLKGEN_CONTROL_XTAL24MDS_2,
    AM_HAL_CLKGEN_CONTROL_XTAL24MDS_3,
    AM_HAL_CLKGEN_CONTROL_XTAL24MDS_4,
    AM_HAL_CLKGEN_CONTROL_XTAL24MDS_5,
    AM_HAL_CLKGEN_CONTROL_XTAL24MDS_6,
    AM_HAL_CLKGEN_CONTROL_XTAL24MDS_7,
    AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE,
    AM_HAL_CLKGEN_CONTROL_DCCLK_DISABLE,
    AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_OFF,
    AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC48,
    AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96,
    AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_DPHYPLL,
    AM_HAL_CLKGEN_CONTROL_PLLCLK_ENABLE,
    AM_HAL_CLKGEN_CONTROL_PLLCLK_DISABLE,
    AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_OFF,
    AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFRC12,
    AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFRC24,
    AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFXT
} am_hal_clkgen_control_e;

//
// Current RTC oscillator.
//
typedef enum
{
    AM_HAL_CLKGEN_STATUS_RTCOSC_XTAL,
    AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC,
} am_hal_clkgen_status_rtcosc_e;

//
// CLKOUT
//
typedef enum
{
    AM_HAL_CLKGEN_CLKOUT_XTAL_32768     = CLKGEN_CLKOUT_CKSEL_XT,           // XTAL
    AM_HAL_CLKGEN_CLKOUT_XTAL_16384     = CLKGEN_CLKOUT_CKSEL_XT_DIV2,      // XTAL / 2
    AM_HAL_CLKGEN_CLKOUT_XTAL_8192      = CLKGEN_CLKOUT_CKSEL_XT_DIV4,      // XTAL / 4
    AM_HAL_CLKGEN_CLKOUT_XTAL_4096      = CLKGEN_CLKOUT_CKSEL_XT_DIV8,      // XTAL / 8
    AM_HAL_CLKGEN_CLKOUT_XTAL_2048      = CLKGEN_CLKOUT_CKSEL_XT_DIV16,     // XTAL / 16
    AM_HAL_CLKGEN_CLKOUT_XTAL_1024      = CLKGEN_CLKOUT_CKSEL_XT_DIV32,     // XTAL / 32
    AM_HAL_CLKGEN_CLKOUT_XTAL_128       = CLKGEN_CLKOUT_CKSEL_XT_DIV256,    // XTAL / 256   = 128 Hz
    AM_HAL_CLKGEN_CLKOUT_XTAL_4         = CLKGEN_CLKOUT_CKSEL_XT_DIV8K,     // XTAL / 8192  =  4 Hz
    AM_HAL_CLKGEN_CLKOUT_XTAL_0_5       = CLKGEN_CLKOUT_CKSEL_XT_DIV64K,    // XTAL / 65536 =  0.5 Hz
    AM_HAL_CLKGEN_CLKOUT_XTAL_0_015     = CLKGEN_CLKOUT_CKSEL_XT_DIV2M,     // XTAL / 2097152 = 0.015625 Hz

    AM_HAL_CLKGEN_CLKOUT_LFRC           = CLKGEN_CLKOUT_CKSEL_LFRC,         // LFRC
    AM_HAL_CLKGEN_CLKOUT_LFRC_512       = CLKGEN_CLKOUT_CKSEL_LFRC_DIV2,    // LFRC / 2     = 512 Hz
    AM_HAL_CLKGEN_CLKOUT_LFRC_32        = CLKGEN_CLKOUT_CKSEL_LFRC_DIV32,   // LFRC / 32    =  32 Hz
    AM_HAL_CLKGEN_CLKOUT_LFRC_2         = CLKGEN_CLKOUT_CKSEL_LFRC_DIV512,  // LFRC / 512   =   2 Hz
    AM_HAL_CLKGEN_CLKOUT_LFRC_0_03      = CLKGEN_CLKOUT_CKSEL_LFRC_DIV32K,  // LFRC / 32768 = 0.03125 Hz
    AM_HAL_CLKGEN_CLKOUT_LFRC_0_0010    = CLKGEN_CLKOUT_CKSEL_LFRC_DIV1M,   // LFRC / 1M    = 0.0009765625 Hz

// #### INTERNAL BEGIN ####
    AM_HAL_CLKGEN_CLKOUT_HFRC_48M       = CLKGEN_CLKOUT_CKSEL_HFRC,         // HFRC         = 48000000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_24M       = CLKGEN_CLKOUT_CKSEL_HFRC_DIV2,    // HFRC / 2 = 24MHz
    AM_HAL_CLKGEN_CLKOUT_HFRC_12M       = CLKGEN_CLKOUT_CKSEL_HFRC_DIV4,    // HFRC / 4     = 12000000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_6M        = CLKGEN_CLKOUT_CKSEL_HFRC_DIV8,    // HFRC / 8     =  6000000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_3M        = CLKGEN_CLKOUT_CKSEL_HFRC_DIV16,   // HFRC / 16    =  3000000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_750K      = CLKGEN_CLKOUT_CKSEL_HFRC_DIV64,   // HFRC / 64    =   750000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_375K      = CLKGEN_CLKOUT_CKSEL_HFRC_DIV128,  // HFRC / 128   =   375000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_187K      = CLKGEN_CLKOUT_CKSEL_HFRC_DIV256,  // HFRC / 256   =   187500 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_93750     = CLKGEN_CLKOUT_CKSEL_HFRC_DIV512,  // HFRC / 512   =   375000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_732       = CLKGEN_CLKOUT_CKSEL_HFRC_DIV64K,  // HFRC / 65536 = 732.421 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_2         = CLKGEN_CLKOUT_CKSEL_HFRC_DIV16M,  // HFRC / 16M   = 2.861 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_CORE      = CLKGEN_CLKOUT_CKSEL_HFRC_CORE,    // HFRC_CORE : No Clk connected
// #### INTERNAL END ####

    AM_HAL_CLKGEN_CLKOUT_RTC_1HZ        = CLKGEN_CLKOUT_CKSEL_RTC_1Hz,      // RTC
    AM_HAL_CLKGEN_CLKOUT_CG_100         = CLKGEN_CLKOUT_CKSEL_CG_100Hz,     // ClkGen 100Hz

    // The next 5 are Uncalibrated LFRC
    AM_HAL_CLKGEN_CLKOUT_ULFRC_64       = CLKGEN_CLKOUT_CKSEL_ULFRC_DIV16,  // ULFRC / 16   = 64 Hz (uncal LFRC)
    AM_HAL_CLKGEN_CLKOUT_ULFRC_8        = CLKGEN_CLKOUT_CKSEL_ULFRC_DIV128, // ULFRC / 128  =  8 Hz (uncal LFRC)
    AM_HAL_CLKGEN_CLKOUT_ULFRC_1        = CLKGEN_CLKOUT_CKSEL_ULFRC_1Hz,    // ULFRC / 1024 =  1 Hz (uncal LFRC)
    AM_HAL_CLKGEN_CLKOUT_ULFRC_0_25     = CLKGEN_CLKOUT_CKSEL_ULFRC_DIV4K,  // ULFRC / 4096 = 0.25 Hz (uncal LFRC)
    AM_HAL_CLKGEN_CLKOUT_ULFRC_0_0009   = CLKGEN_CLKOUT_CKSEL_ULFRC_DIV1M,  // ULFRC / 1M   = 0.000976 Hz (uncal LFRC)

    // Following are Not Autoenabled ("NE")
    AM_HAL_CLKGEN_CLKOUT_XTALNE_32768   = CLKGEN_CLKOUT_CKSEL_XTNE,         // XTALNE / 1   = 32768 Hz
    AM_HAL_CLKGEN_CLKOUT_XTALNE_2048    = CLKGEN_CLKOUT_CKSEL_XTNE_DIV16,   // XTALNE / 16  =  2048 Hz
    AM_HAL_CLKGEN_CLKOUT_LFRCNE_32      = CLKGEN_CLKOUT_CKSEL_LFRCNE_DIV32, // LFRCNE / 32  =    32 Hz
// #### INTERNAL BEGIN ####
    AM_HAL_CLKGEN_CLKOUT_HFRCNE_48M     = CLKGEN_CLKOUT_CKSEL_HFRCNE,       // HFRCNE / 1   = 48MHz
    AM_HAL_CLKGEN_CLKOUT_HFRCNE_6M      = CLKGEN_CLKOUT_CKSEL_HFRCNE_DIV8,  // HFRCNE / 8   =  6MHz
// #### INTERNAL END ####
} am_hal_clkgen_clkout_e;

#define AM_HAL_CLKGEN_CLKOUT_MAX        CLKGEN_CLKOUT_CKSEL_HFRC_DIV2   // Highest valid CKSEL value

// #### INTERNAL BEGIN ####
//
// XTAL 24MHz control
//
/*typedef enum
{
    AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_0 = 0x0,      // Drive strength
    AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_1,
    AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_2,
    AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_3,
    AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_4,
    AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_5,
    AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_6,
    AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_7,
} am_hal_clkgen_xtal24mctrl_e;*/

//
// Display clock control
//
/*typedef enum
{
    AM_HAL_CLKGEN_DISPCLKCTRL_DCCLKEN,
    AM_HAL_CLKGEN_DISPCLKCTRL_DISPCLKSEL_OFF,
    AM_HAL_CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC48,
    AM_HAL_CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC96,
    AM_HAL_CLKGEN_DISPCLKCTRL_DISPCLKSEL_DPHYPLL,
    AM_HAL_CLKGEN_DISPCLKCTRL_PLLCLKEN,
    AM_HAL_CLKGEN_DISPCLKCTRL_PLLCLKSEL_OFF,
    AM_HAL_CLKGEN_DISPCLKCTRL_PLLCLKSEL_HFRC12,
    AM_HAL_CLKGEN_DISPCLKCTRL_PLLCLKSEL_HFRC24,
    AM_HAL_CLKGEN_DISPCLKCTRL_PLLCLKSEL_HFXT,
} am_hal_clkgen_dispclkctrl_e*/;
// #### INTERNAL END ####

//
// ClkGen Interrupts
//
typedef enum
{
    AM_HAL_CLKGEN_INTERRUPT_OF          = CLKGEN_INTRPTEN_OF_Msk,
    AM_HAL_CLKGEN_INTERRUPT_ACC         = CLKGEN_INTRPTEN_ACC_Msk,
    AM_HAL_CLKGEN_INTERRUPT_ACF         = CLKGEN_INTRPTEN_ACF_Msk
} am_hal_clkgen_interrupt_e;

//
// Status structure.
//
typedef struct
{
    //
    // ui32SysclkFreq
    //  Returns the current system clock frequency, in hertz.
    //
    uint32_t    ui32SysclkFreq;

    //
    // ui32RTCoscillator
    //
    //  Returns the current RTC oscillator as one of:
    //  AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC
    //  AM_HAL_CLKGEN_STATUS_RTCOSC_XTAL
    //
    uint32_t    eRTCOSC;

    //
    // bXtalFailure
    //  true = XTAL has failed (is enabled but not oscillating).  Also if the
    //         LFRC is selected as the oscillator in OCTRL.OSEL.
    //
    bool        bXtalFailure;

     //
    // enable status for all the peripheral clocks.
    //  1: enable
    //  0: disable
    //
    //uint32_t    ui32Clockenstat;
    //uint32_t    ui32Clocken2stat;
    //uint32_t    ui32Clocken3stat;
} am_hal_clkgen_status_t;


// #### INTERNAL BEGIN ####
#if 0
//
// Clkgen status and returns.
//
typedef enum
{
    AM_HAL_CLKGEN_STATUS_SYSCLK,
    AM_HAL_CLKGEN_STATUS_OSC_RTC,
    AM_HAL_CLKGEN_STATUS_XTAL_FAILURE
} am_hal_clkgen_status_e;
#endif
// #### INTERNAL END ####
// ****************************************************************************
//
//! @brief Apply various specific commands/controls on the CLKGEN module.
//!
//! This function is used to apply various controls on CLKGEN.
//!
//! @note IMPORTANT! This function MUST be called very early in execution of
//!       an application with the parameter AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX
//!       in order to set Apollo3 to its required operating frequency.
//!
//! @param eControl - One of the following:
//!     AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX
//!     AM_HAL_CLKGEN_CONTROL_XTAL_START
//!     AM_HAL_CLKGEN_CONTROL_LFRC_START
//!     AM_HAL_CLKGEN_CONTROL_XTAL_STOP
//!     AM_HAL_CLKGEN_CONTROL_LFRC_STOP
//!     AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL
//!     AM_HAL_CLKGEN_CONTROL_RTC_SEL_LFRC
//!     AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE
//!     AM_HAL_CLKGEN_CONTROL_HFADJ_DISABLE
//!
//! @return status      - generic or interface specific status.
//!
//! @note After starting the XTAL, a 2 second warm-up delay is required.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_control(am_hal_clkgen_control_e eControl,
                                      void *pArgs);

// ****************************************************************************
//
//! @brief Get CLKGEN status.
//!
//! This function returns the current value of various CLKGEN statuses.
//!
//! @param psStatus - ptr to a status structure to receive the current statuses.
//!
//! @return status      - generic or interface specific status.
//!
//! @note After selection of the RTC Oscillator, a 2 second delay is required
//! before the new oscillator takes effect.  Therefore the CLKGEN.STATUS.OMODE
//! bit will not reflect the new status until after the 2s wait period.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_status_get(am_hal_clkgen_status_t *psStatus);

// ****************************************************************************
//
//! @brief Enable CLKOUT.
//!
//! This function is used to enable and select a CLKOUT frequency.
//!
//! @param bEnable: true to enable, false to disable.
//! @param eClkSelect - One of the following:
//!     AM_HAL_CLKGEN_CLKOUT_LFRC_1024
//!     AM_HAL_CLKGEN_CLKOUT_XTAL_16384
//!     AM_HAL_CLKGEN_CLKOUT_XTAL_8192
//!     AM_HAL_CLKGEN_CLKOUT_XTAL_4096
//!     AM_HAL_CLKGEN_CLKOUT_XTAL_2048
//!     AM_HAL_CLKGEN_CLKOUT_XTAL_1024
//!     AM_HAL_CLKGEN_CLKOUT_RTC_1HZ
//!     AM_HAL_CLKGEN_CLKOUT_XTAL_0_015
//!     AM_HAL_CLKGEN_CLKOUT_XTAL_32768
//!     AM_HAL_CLKGEN_CLKOUT_CG_100
// #### INTERNAL BEGIN ####
//!     AM_HAL_CLKGEN_CLKOUT_HFRC_48M
//!     AM_HAL_CLKGEN_CLKOUT_HFRC_12M
//!     AM_HAL_CLKGEN_CLKOUT_HFRC_6M
//!     AM_HAL_CLKGEN_CLKOUT_HFRC_3M
//!     AM_HAL_CLKGEN_CLKOUT_HFRC_750K
//!     AM_HAL_CLKGEN_CLKOUT_HFRC_375K
//!     AM_HAL_CLKGEN_CLKOUT_HFRC_187K
//!     AM_HAL_CLKGEN_CLKOUT_HFRC_93750
//!     AM_HAL_CLKGEN_CLKOUT_FLASHCLK
// #### INTERNAL END ####
//!     AM_HAL_CLKGEN_CLKOUT_LFRC_512
//!     AM_HAL_CLKGEN_CLKOUT_LFRC_32
//!     AM_HAL_CLKGEN_CLKOUT_LFRC_2
//!     AM_HAL_CLKGEN_CLKOUT_LFRC_0_03
//!     AM_HAL_CLKGEN_CLKOUT_XTAL_128
//!     AM_HAL_CLKGEN_CLKOUT_XTAL_4
//!     AM_HAL_CLKGEN_CLKOUT_XTAL_0_5
//!
//!     The next 5 are Uncalibrated LFRC
//!     AM_HAL_CLKGEN_CLKOUT_ULFRC_64
//!     AM_HAL_CLKGEN_CLKOUT_ULFRC_8
//!     AM_HAL_CLKGEN_CLKOUT_ULFRC_1
//!     AM_HAL_CLKGEN_CLKOUT_ULFRC_0_25
//!     AM_HAL_CLKGEN_CLKOUT_ULFRC_0_0009
//!
// #### INTERNAL BEGIN ####
//!     AM_HAL_CLKGEN_CLKOUT_HFRC_732
//!     AM_HAL_CLKGEN_CLKOUT_HFRC_2
// #### INTERNAL END ####
//!     AM_HAL_CLKGEN_CLKOUT_LFRC_0_0004
//!
//!     Following are Not Autoenabled ("NE")
// #### INTERNAL BEGIN ####
//!     AM_HAL_CLKGEN_CLKOUT_HFRCNE_48M
//!     AM_HAL_CLKGEN_CLKOUT_HFRCNE_6M
// #### INTERNAL END ####
//!     AM_HAL_CLKGEN_CLKOUT_XTALNE_32768
//!     AM_HAL_CLKGEN_CLKOUT_XTALNE_2048
//!     AM_HAL_CLKGEN_CLKOUT_LFRCNE_32
//!     AM_HAL_CLKGEN_CLKOUT_LFRCNE_1024
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_clkout_enable(bool bEnable,
                                            am_hal_clkgen_clkout_e eClkSelect);

// ****************************************************************************
//
//! @brief Enable XTAL 24MHz.
//!
//! This function is used to enable XTAL 24MHz and select drive strength.
//!
//! @param bEnable: true to enable, false to disable.
//! @param eDSSelect - One of the following:
//!     AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_0
//!     AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_1
//!     AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_2
//!     AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_3
//!     AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_4
//!     AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_5
//!     AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_6
//!     AM_HAL_CLKGEN_XTAL24MCTRL_XTAL24MDS_7
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
//extern uint32_t am_hal_clkgen_xtal24mctrl_enable(bool bEnable,
                                               //am_hal_clkgen_xtal24mctrl_e eDSSelect);

// ****************************************************************************
//
//! @brief Enable DCCLK.
//!
//! This function is used to enable DCCLK and select clock source.
//!
//! @param bEnable: true to enable, false to disable.
//! @param eDSSelect - One of the following:
//!     CLKGEN_DISPCLKCTRL_DISPCLKSEL_OFF
//!     CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC48
//!     CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC96
//!     CLKGEN_DISPCLKCTRL_DISPCLKSEL_DPHYPLL
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
//extern uint32_t am_hal_clkgen_dcclk_enable(bool bEnable,
                                           //CLKGEN_DISPCLKCTRL_DISPCLKSEL_Enum eDispclkSelect);

// ****************************************************************************
//
//! @brief Enable PLLCLK.
//!
//! This function is used to enable PLLCLK and select pll reference clock.
//!
//! @param bEnable: true to enable, false to disable.
//! @param eDSSelect - One of the following:
//!     CLKGEN_DISPCLKCTRL_PLLCLKSEL_OFF
//!     CLKGEN_DISPCLKCTRL_PLLCLKSEL_HFRC12
//!     CLKGEN_DISPCLKCTRL_PLLCLKSEL_HFRC24
//!     CLKGEN_DISPCLKCTRL_PLLCLKSEL_HFXT
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
//extern uint32_t am_hal_clkgen_pllclk_enable(bool bEnable,
                                          //CLKGEN_DISPCLKCTRL_PLLCLKSEL_Enum ePllclkSelect);

// ****************************************************************************
//
//! @brief Enable selected CLKGEN Interrupts.
//!
//! Use this function to enable the interrupts.
//!
//! @param ui32IntMask - One or more of the following bitmasks.
//!     AM_HAL_CLKGEN_INTERRUPT_OF
//!     AM_HAL_CLKGEN_INTERRUPT_ACC
//!     AM_HAL_CLKGEN_INTERRUPT_ACF
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_interrupt_enable(am_hal_clkgen_interrupt_e ui32IntMask);

// ****************************************************************************
//
//! @brief Disable selected CLKGEN Interrupts.
//!
//! Use this function to disable the CLKGEN interrupts.
//!
//! @param ui32IntMask - One or more of the following bitmasks.
//!     AM_HAL_CLKGEN_INTERRUPT_OF
//!     AM_HAL_CLKGEN_INTERRUPT_ACC
//!     AM_HAL_CLKGEN_INTERRUPT_ACF
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_interrupt_disable(am_hal_clkgen_interrupt_e ui32IntMask);

//*****************************************************************************
//
//! @brief IOM interrupt clear
//!
//! @param ui32IntMask  - interface specific interrupt mask.
//!
//! This function clears the interrupts for the given peripheral.
//!
//! The following are valid clear bits, any of which can be ORed together.
//!     AM_HAL_CLKGEN_INTERRUPT_OF
//!     AM_HAL_CLKGEN_INTERRUPT_ACC
//!     AM_HAL_CLKGEN_INTERRUPT_ACF
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_clkgen_interrupt_clear(am_hal_clkgen_interrupt_e ui32IntMask);

// ****************************************************************************
//
//! @brief Return CLKGEN interrupts.
//!
//! Use this function to get all CLKGEN interrupts, or only the interrupts
//! that are enabled.
//!
//! @return All or only enabled CLKGEN interrupts.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_interrupt_status_get(bool bEnabledOnly,
                                                   uint32_t *pui32IntStatus);

// ****************************************************************************
//
//! @brief Sets the interrupt status.
//!
//! This function sets the CLKGEN interrupts.
//!
//! @param ui32IntMask - One or more of the following bitmasks.
//!     AM_HAL_CLKGEN_INTERRUPT_OF
//!     AM_HAL_CLKGEN_INTERRUPT_ACC
//!     AM_HAL_CLKGEN_INTERRUPT_ACF
//!
//! @return None.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_interrupt_set(am_hal_clkgen_interrupt_e ui32IntMask);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_CLKGEN_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
