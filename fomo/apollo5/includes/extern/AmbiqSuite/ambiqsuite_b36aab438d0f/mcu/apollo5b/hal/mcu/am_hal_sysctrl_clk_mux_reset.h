//*****************************************************************************
//
//! @file am_hal_sysctrl_clk_mux_reset.h
//!
//! @brief Internal api definition for Clock Mux Reset Operation
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
//! @cond SYSCTRL_CLK_MUX_RESET
#ifndef AM_HAL_SYSCTRL_CLK_MUX_RESET_H
#define AM_HAL_SYSCTRL_CLK_MUX_RESET_H

#ifdef __cplusplus
extern "C"
{
#endif

// union for SNVR2 register access
typedef union
{
    __IOM uint32_t SNVR2;                   /*!< (@ 0x40008858) SNVR2 is used to store mux status to be use during bootup     */
    struct
    {
      __IOM uint32_t AUDADC_CLKGEN_OFF  :  1; /*!< [ 0.. 0] AUDADC ClockGen mux was at OFF position */
      __IOM uint32_t HFRC_DED_NEEDED    :  1; /*!< [ 1.. 1] HFRC_DED clock needed for mux reset     */
      __IOM uint32_t HFRC2_NEEDED       :  1; /*!< [ 2.. 2] HFRC2 clock needed for mux reset        */
      __IOM uint32_t XTAL_NEEDED        :  1; /*!< [ 3.. 3] XTAL clock needed for mux reset         */
      __IOM uint32_t EXTCLK_NEEDED      :  1; /*!< [ 4.. 4] EXTREF clock needed for mux reset       */
      __IOM uint32_t PLL_NEEDED         :  1; /*!< [ 5.. 5] PLL clock needed for mux reset          */
      __IOM uint32_t PLL_FREFSEL        :  1; /*!< [ 6.. 6] REFCLK selection for PLL                */
            uint32_t                    :  9; /*!< [15.. 7] Reserved bits                           */
      __IOM uint32_t SIGNATURE          : 16; /*!< [31..16] Signature for this information          */
    } SNVR2_b;
} SNVR2_type;
#define SNVR2REG ((SNVR2_type*)0x40008858)
#define SYSCTRL_CLKMUXRST_SIGNATURE 0x5AF0



//*****************************************************************************
//
//! CLKMUXRST Clock Source Bitmap
//
//*****************************************************************************
#define SYSCTRL_CLKMUXRST_CLK_CLKGEN        0x00
#define SYSCTRL_CLKMUXRST_CLK_HFRC_DED      0x01
#define SYSCTRL_CLKMUXRST_CLK_HFRC2         0x02
#define SYSCTRL_CLKMUXRST_CLK_XTAL          0x04
#define SYSCTRL_CLKMUXRST_CLK_EXTCLK        0x08
#define SYSCTRL_CLKMUXRST_CLK_PLL           0x10

//*****************************************************************************
//
//! CLKMUXRST clock mux enum
//
//*****************************************************************************
typedef enum
{
    SYSCTRL_CLKMUXRST_MUX_CG_AUDADC,
    SYSCTRL_CLKMUXRST_MUX_LL_AUDADC,
    SYSCTRL_CLKMUXRST_MUX_LL_PDM,
    SYSCTRL_CLKMUXRST_MUX_LL_I2S0,
    SYSCTRL_CLKMUXRST_MUX_LL_I2S1,
    SYSCTRL_CLKMUXRST_MUX_LL_USB,

    // SYSCTRL_CLKMUXRST_CLK_MAX Should always be the last entry
    SYSCTRL_CLKMUXRST_CLK_MAX,
} am_hal_sysctrl_clkmuxrst_clk_e;

//*****************************************************************************
//
//! @brief Update clocks needed for clock muxes reset operation
//!
//! @param bClkgenOff - AUDADC ClockGen clock selection is OFF
//
//*****************************************************************************
extern void am_hal_sysctrl_clkmuxrst_audadc_clkgen_off_update(bool bClkgenOff);

//*****************************************************************************
//
//! @brief Update clocks needed for clock muxes reset operation
//!
//! @param eFref - Reference Clock select for PLL
//
//*****************************************************************************
extern void am_hal_sysctrl_clkmuxrst_pll_fref_update(MCUCTRL_PLLCTL0_FREFSEL_Enum eFref);

//*****************************************************************************
//
//! @brief Update PLL FREF selection
//!
//! @param eClk - am_hal_sysctrl_clkmuxrst_clk_e value for mux to update.
//! @param ui8ClkSrcBm - bitmap representing clocks needed by the mux
//
//*****************************************************************************
extern void am_hal_sysctrl_clkmuxrst_clkneeded_update(am_hal_sysctrl_clkmuxrst_clk_e eClk, uint8_t ui8ClkSrcBm);


//*****************************************************************************
//
//! @brief Handle clock muxes reset during low_power_init
//
//*****************************************************************************
extern void am_hal_sysctrl_clkmuxrst_low_power_init();


#ifdef __cplusplus
}
#endif

#endif //AM_HAL_SYSCTRL_CLK_MUX_RESET_H

//! @endcond SYSCTRL_CLK_MUX_RESET