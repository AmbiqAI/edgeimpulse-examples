//*****************************************************************************
//
//! @file am_hal_clkgen.h
//!
//! @brief Functions for Interfacing with the CLKGEN.
//!
//! @addtogroup Clkgen3p Clkgen - Clock Generator
//! @ingroup apollo3p_hal
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
//! Designate this peripheral.
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
#if APOLLO3_FPGA
#define AM_HAL_CLKGEN_FREQ_MAX_HZ       24000000
#else
// #### INTERNAL END ####
#define AM_HAL_CLKGEN_FREQ_MAX_HZ       48000000
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
#define AM_HAL_CLKGEN_FREQ_MAX_KHZ      (AM_HAL_CLKGEN_FREQ_MAX_HZ / 1000)
#define AM_HAL_CLKGEN_FREQ_MAX_MHZ      (AM_HAL_CLKGEN_FREQ_MAX_HZ / 1000000)
#define AM_HAL_CLKGEN_CORESEL_MAXDIV    1
//! @}

//
//! Control operations.
//
typedef enum
{
    AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX,
    AM_HAL_CLKGEN_CONTROL_XTAL_START,
    AM_HAL_CLKGEN_CONTROL_LFRC_START,
    AM_HAL_CLKGEN_CONTROL_XTAL_STOP,
    AM_HAL_CLKGEN_CONTROL_LFRC_STOP,
    AM_HAL_CLKGEN_CONTROL_SYSCLK_DIV2,
    AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL,
    AM_HAL_CLKGEN_CONTROL_RTC_SEL_LFRC,
    AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE,
    AM_HAL_CLKGEN_CONTROL_HFADJ_DISABLE,
// #### INTERNAL BEGIN ####
    AM_HAL_CLKGEN_CONTROL_SYSCLK_MIN = 0x1001,
// #### INTERNAL END ####
} am_hal_clkgen_control_e;

//
//! Current RTC oscillator.
//
typedef enum
{
    AM_HAL_CLKGEN_STATUS_RTCOSC_XTAL,
    AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC,
} am_hal_clkgen_status_rtcosc_e;

//
//! CLKOUT
//
typedef enum
{
    AM_HAL_CLKGEN_CLKOUT_LFRC_1024  = 0x0,      // LFRC
    AM_HAL_CLKGEN_CLKOUT_XTAL_16384,            // XTAL / 2
    AM_HAL_CLKGEN_CLKOUT_XTAL_8192,             // XTAL / 4
    AM_HAL_CLKGEN_CLKOUT_XTAL_4096,             // XTAL / 8
    AM_HAL_CLKGEN_CLKOUT_XTAL_2048,             // XTAL / 16
    AM_HAL_CLKGEN_CLKOUT_XTAL_1024,             // XTAL / 32
    AM_HAL_CLKGEN_CLKOUT_RTC_1HZ    = 0x10,     // RTC
    AM_HAL_CLKGEN_CLKOUT_XTAL_0_015 = 0x16,     // XTAL / 2097152 = 0.015625 Hz
    AM_HAL_CLKGEN_CLKOUT_XTAL_32768,            // XTAL
    AM_HAL_CLKGEN_CLKOUT_CG_100,                // ClkGen 100Hz
// #### INTERNAL BEGIN ####
    // A2SD-613 and AS-228: HFRC CLKOUT is defeatured for Apollo3x due to poor signal integrity.
    AM_HAL_CLKGEN_CLKOUT_HFRC_48M,              // HFRC         = 48000000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_12M,              // HFRC / 4     = 12000000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_6M,               // HFRC / 8     =  6000000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_3M,               // HFRC / 16    =  3000000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_750K,             // HFRC / 64    =   750000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_375K,             // HFRC / 128   =   375000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_187K,             // HFRC / 256   =   187500 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_93750,            // HFRC / 512   =   375000 Hz
    AM_HAL_CLKGEN_CLKOUT_FLASHCLK   = 0x22,     // Flash Clock
// #### INTERNAL END ####
    AM_HAL_CLKGEN_CLKOUT_LFRC_512 = 0x23,       // LFRC / 2     = 512 Hz
    AM_HAL_CLKGEN_CLKOUT_LFRC_32,               // LFRC / 32    =  32 Hz
    AM_HAL_CLKGEN_CLKOUT_LFRC_2,                // LFRC / 512   =   2 Hz
    AM_HAL_CLKGEN_CLKOUT_LFRC_0_03,             // LFRC / 32768 = 0.03125 Hz
    AM_HAL_CLKGEN_CLKOUT_XTAL_128,              // XTAL / 256   = 128 Hz
    AM_HAL_CLKGEN_CLKOUT_XTAL_4,                // XTAL / 8192  =  4 Hz
    AM_HAL_CLKGEN_CLKOUT_XTAL_0_5,              // XTAL / 65536 =  0.5 Hz
    // The next 5 are Uncalibrated LFRC
    AM_HAL_CLKGEN_CLKOUT_ULFRC_64,              // ULFRC / 16   = 64 Hz (uncal LFRC)
    AM_HAL_CLKGEN_CLKOUT_ULFRC_8,               // ULFRC / 128  =  8 Hz (uncal LFRC)
    AM_HAL_CLKGEN_CLKOUT_ULFRC_1,               // ULFRC / 1024 =  1 Hz (uncal LFRC)
    AM_HAL_CLKGEN_CLKOUT_ULFRC_0_25,            // ULFRC / 4096 = 0.25 Hz (uncal LFRC)
    AM_HAL_CLKGEN_CLKOUT_ULFRC_0_0009,          // ULFRC / 1M   = 0.000976 Hz (uncal LFRC)
    //
// #### INTERNAL BEGIN ####
    AM_HAL_CLKGEN_CLKOUT_HFRC_732,              // HFRC / 65536 = 732.421 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRC_2,                // HFRC / 16M   = 2.861 Hz
// #### INTERNAL END ####
    AM_HAL_CLKGEN_CLKOUT_LFRC_0_0004 = 0x31,    // LFRC / 2M    = 0.00048828125 Hz
    // Following are Not Autoenabled ("NE")
// #### INTERNAL BEGIN ####
    AM_HAL_CLKGEN_CLKOUT_HFRCNE_48M,            // HFRCNE / 1   = 48000000 Hz
    AM_HAL_CLKGEN_CLKOUT_HFRCNE_6M,             // HFRCNE / 8   =  6000000 Hz
// #### INTERNAL END ####
    AM_HAL_CLKGEN_CLKOUT_XTALNE_32768 = 0x35,   // XTALNE / 1   = 32768 Hz
    AM_HAL_CLKGEN_CLKOUT_XTALNE_2048,           // XTALNE / 16  =  2048 Hz
    AM_HAL_CLKGEN_CLKOUT_LFRCNE_32,             // LFRCNE / 32  =    32 Hz
    AM_HAL_CLKGEN_CLKOUT_LFRCNE_1024 = 0x39     // LFRCNE / 1   =  1024 Hz
} am_hal_clkgen_clkout_e;

//
//! ClkGen Interrupts
//
typedef enum
{
    AM_HAL_CLKGEN_INTERRUPT_OF          = CLKGEN_INTRPTEN_OF_Msk,
    AM_HAL_CLKGEN_INTERRUPT_ACC         = CLKGEN_INTRPTEN_ACC_Msk,
    AM_HAL_CLKGEN_INTERRUPT_ACF         = CLKGEN_INTRPTEN_ACF_Msk
} am_hal_clkgen_interrupt_e;

//
//! Status structure.
//
typedef struct
{
    //
    //!  Returns the current system clock frequency, in hertz.
    //
    uint32_t    ui32SysclkFreq;

    //
    //!  Returns the current RTC oscillator as one of:
    //!      - AM_HAL_CLKGEN_STATUS_RTCOSC_LFRC
    //!      - AM_HAL_CLKGEN_STATUS_RTCOSC_XTAL
    //
    uint32_t    eRTCOSC;

    //
    //!  true = XTAL has failed (is enabled but not oscillating).\n
    //!         Also if the LFRC is selected as the oscillator in OCTRL.OSEL.
    //
    bool        bXtalFailure;
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
//!      - AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX
//!      - AM_HAL_CLKGEN_CONTROL_XTAL_START
//!      - AM_HAL_CLKGEN_CONTROL_LFRC_START
//!      - AM_HAL_CLKGEN_CONTROL_XTAL_STOP
//!      - AM_HAL_CLKGEN_CONTROL_LFRC_STOP
//!      - AM_HAL_CLKGEN_CONTROL_RTC_SEL_XTAL
//!      - AM_HAL_CLKGEN_CONTROL_RTC_SEL_LFRC
//!      - AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE
//!      - AM_HAL_CLKGEN_CONTROL_HFADJ_DISABLE
//! @param pArgs - pointer to args struct
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
//!      - AM_HAL_CLKGEN_CLKOUT_LFRC_1024
//!      - AM_HAL_CLKGEN_CLKOUT_XTAL_16384
//!      - AM_HAL_CLKGEN_CLKOUT_XTAL_8192
//!      - AM_HAL_CLKGEN_CLKOUT_XTAL_4096
//!      - AM_HAL_CLKGEN_CLKOUT_XTAL_2048
//!      - AM_HAL_CLKGEN_CLKOUT_XTAL_1024
//!      - AM_HAL_CLKGEN_CLKOUT_RTC_1HZ
//!      - AM_HAL_CLKGEN_CLKOUT_XTAL_0_015
//!      - AM_HAL_CLKGEN_CLKOUT_XTAL_32768
//!      - AM_HAL_CLKGEN_CLKOUT_CG_100
// #### INTERNAL BEGIN ####
//!      - AM_HAL_CLKGEN_CLKOUT_HFRC_48M
//!      - AM_HAL_CLKGEN_CLKOUT_HFRC_12M
//!      - AM_HAL_CLKGEN_CLKOUT_HFRC_6M
//!      - AM_HAL_CLKGEN_CLKOUT_HFRC_3M
//!      - AM_HAL_CLKGEN_CLKOUT_HFRC_750K
//!      - AM_HAL_CLKGEN_CLKOUT_HFRC_375K
//!      - AM_HAL_CLKGEN_CLKOUT_HFRC_187K
//!      - AM_HAL_CLKGEN_CLKOUT_HFRC_93750
//!      - AM_HAL_CLKGEN_CLKOUT_FLASHCLK
// #### INTERNAL END ####
//!      - AM_HAL_CLKGEN_CLKOUT_LFRC_512
//!      - AM_HAL_CLKGEN_CLKOUT_LFRC_32
//!      - AM_HAL_CLKGEN_CLKOUT_LFRC_2
//!      - AM_HAL_CLKGEN_CLKOUT_LFRC_03
//!      - AM_HAL_CLKGEN_CLKOUT_XTAL_128
//!      - AM_HAL_CLKGEN_CLKOUT_XTAL_4
//!      - AM_HAL_CLKGEN_CLKOUT_XTAL_0_5
//!      - ** The next 5 are Uncalibrated LFRC
//!      - AM_HAL_CLKGEN_CLKOUT_ULFRC_64
//!      - AM_HAL_CLKGEN_CLKOUT_ULFRC_8
//!      - AM_HAL_CLKGEN_CLKOUT_ULFRC_1
//!      - AM_HAL_CLKGEN_CLKOUT_ULFRC_0_25
//!      - AM_HAL_CLKGEN_CLKOUT_ULFRC_0_0009
// #### INTERNAL BEGIN ####
//!      - AM_HAL_CLKGEN_CLKOUT_HFRC_732
//!      - AM_HAL_CLKGEN_CLKOUT_HFRC_2
// #### INTERNAL END ####
//!      -AM_HAL_CLKGEN_CLKOUT_LFRC_0_0004
//!      - ** The Following are Not Autoenabled ("NE")
// #### INTERNAL BEGIN ####
//!      - AM_HAL_CLKGEN_CLKOUT_HFRCNE_48M
//!      - AM_HAL_CLKGEN_CLKOUT_HFRCNE_6M
// #### INTERNAL END ####
//!      - AM_HAL_CLKGEN_CLKOUT_XTALNE_32768
//!      - AM_HAL_CLKGEN_CLKOUT_XTALNE_2048
//!      - AM_HAL_CLKGEN_CLKOUT_LFRCNE_32
//!      - AM_HAL_CLKGEN_CLKOUT_LFRCNE_1024
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_clkout_enable(bool bEnable,
                                            am_hal_clkgen_clkout_e eClkSelect);

// ****************************************************************************
//
//! @brief Enable selected CLKGEN Interrupts.
//!
//! This function enables the interrupts.
//!
//! @param ui32IntMask - One or more of the following bitmasks.
//!      - AM_HAL_CLKGEN_INTERRUPT_OF
//!      - AM_HAL_CLKGEN_INTERRUPT_ACC
//!      - AM_HAL_CLKGEN_INTERRUPT_ACF
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_interrupt_enable(am_hal_clkgen_interrupt_e ui32IntMask);

// ****************************************************************************
//
//! @brief Disable selected CLKGEN Interrupts.
//!
//! This function disables the CLKGEN interrupts.
//!
//! @param ui32IntMask - One or more of the following bitmasks.
//!      - AM_HAL_CLKGEN_INTERRUPT_OF
//!      - AM_HAL_CLKGEN_INTERRUPT_ACC
//!      - AM_HAL_CLKGEN_INTERRUPT_ACF
//!
//! @return status      - generic or interface specific status.
//
// ****************************************************************************
extern uint32_t am_hal_clkgen_interrupt_disable(am_hal_clkgen_interrupt_e ui32IntMask);

//*****************************************************************************
//
//! @brief IOM interrupt clear
//!
//! This function clears the interrupts for the given peripheral.
//!
//! @param ui32IntMask  - Interface specific interrupt mask.
//!      The following are valid clear bits, any of which can be ORed together.
//!      - AM_HAL_CLKGEN_INTERRUPT_OF
//!      - AM_HAL_CLKGEN_INTERRUPT_ACC
//!      - AM_HAL_CLKGEN_INTERRUPT_ACF
//!
//! @return status      - generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_clkgen_interrupt_clear(am_hal_clkgen_interrupt_e ui32IntMask);

// ****************************************************************************
//
//! @brief Return CLKGEN interrupts.
//!
//! @note Use this function to get all CLKGEN interrupts, or only the interrupts
//! that are enabled.
//!
//! @param bEnabledOnly   - return only interrupts that are enabled
//! @param pui32IntStatus - returns interrupt status here
//!
//! @return status      - generic or interface specific status.
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
//!      - AM_HAL_CLKGEN_INTERRUPT_OF
//!      - AM_HAL_CLKGEN_INTERRUPT_ACC
//!      - AM_HAL_CLKGEN_INTERRUPT_ACF
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

