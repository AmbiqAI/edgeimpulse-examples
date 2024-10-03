//*****************************************************************************
//
//  am_hal_vcomp.h
//! @file
//!
//! @brief Functions for operating the on-chip Voltage Comparator
//!
//! @addtogroup vcomp2 Voltage Comparator (VCOMP)
//! @ingroup apollo2hal
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
#ifndef AM_HAL_VCOMP_H
#define AM_HAL_VCOMP_H

//*****************************************************************************
//
//! @name Positive Input Selection
//! @brief Use these macros to determine the positive input to the comparator.
//! @{
//
//*****************************************************************************
#define AM_HAL_VCOMP_PSEL_VDDADJ            AM_REG_VCOMP_CFG_PSEL_VDDADJ
#define AM_HAL_VCOMP_PSEL_VTEMP             AM_REG_VCOMP_CFG_PSEL_VTEMP
#define AM_HAL_VCOMP_PSEL_VEXT1             AM_REG_VCOMP_CFG_PSEL_VEXT1
#define AM_HAL_VCOMP_PSEL_VEXT2             AM_REG_VCOMP_CFG_PSEL_VEXT2
//! @}

//*****************************************************************************
//
//! @name Negative Input Selection
//! @brief Use these macros to determine the negative input to the comparator.
//! @{
//
//*****************************************************************************
#define AM_HAL_VCOMP_NSEL_VREFEXT1          AM_REG_VCOMP_CFG_NSEL_VREFEXT1
#define AM_HAL_VCOMP_NSEL_VREFEXT2          AM_REG_VCOMP_CFG_NSEL_VREFEXT2
#define AM_HAL_VCOMP_NSEL_VREFEXT3          AM_REG_VCOMP_CFG_NSEL_VREFEXT3
#define AM_HAL_VCOMP_NSEL_DAC_LEVEL         AM_REG_VCOMP_CFG_NSEL_DAC
//! @}

//*****************************************************************************
//
//! @name Negative Input DAC Selectioin
//! @brief Use these macros to determine the NSEL DAC voltage setting
//! @{
//
//*****************************************************************************
#define AM_HAL_VCOMP_DAC_LVLSEL_0_58V        AM_REG_VCOMP_CFG_LVLSEL_0P58V
#define AM_HAL_VCOMP_DAC_LVLSEL_0_77V        AM_REG_VCOMP_CFG_LVLSEL_0P77V
#define AM_HAL_VCOMP_DAC_LVLSEL_0_97V        AM_REG_VCOMP_CFG_LVLSEL_0P97V
#define AM_HAL_VCOMP_DAC_LVLSEL_1_16V        AM_REG_VCOMP_CFG_LVLSEL_1P16V
#define AM_HAL_VCOMP_DAC_LVLSEL_1_35V        AM_REG_VCOMP_CFG_LVLSEL_1P35V
#define AM_HAL_VCOMP_DAC_LVLSEL_1_55V        AM_REG_VCOMP_CFG_LVLSEL_1P55V
#define AM_HAL_VCOMP_DAC_LVLSEL_1_74V        AM_REG_VCOMP_CFG_LVLSEL_1P74V
#define AM_HAL_VCOMP_DAC_LVLSEL_1_93V        AM_REG_VCOMP_CFG_LVLSEL_1P93V
#define AM_HAL_VCOMP_DAC_LVLSEL_2_13V        AM_REG_VCOMP_CFG_LVLSEL_2P13V
#define AM_HAL_VCOMP_DAC_LVLSEL_2_32V        AM_REG_VCOMP_CFG_LVLSEL_2P32V
#define AM_HAL_VCOMP_DAC_LVLSEL_2_51V        AM_REG_VCOMP_CFG_LVLSEL_2P51V
#define AM_HAL_VCOMP_DAC_LVLSEL_2_71V        AM_REG_VCOMP_CFG_LVLSEL_2P71V
#define AM_HAL_VCOMP_DAC_LVLSEL_2_90V        AM_REG_VCOMP_CFG_LVLSEL_2P90V
#define AM_HAL_VCOMP_DAC_LVLSEL_3_09V        AM_REG_VCOMP_CFG_LVLSEL_3P09V
#define AM_HAL_VCOMP_DAC_LVLSEL_3_29V        AM_REG_VCOMP_CFG_LVLSEL_3P29V
#define AM_HAL_VCOMP_DAC_LVLSEL_3_48V        AM_REG_VCOMP_CFG_LVLSEL_3P48V
//! @}

//*****************************************************************************
//
//! @name Interrupt Status Bits
//! @brief Interrupt Status Bits for enable/disble use
//!
//! These macros may be used to set and clear interrupt bits
//! @{
//
//*****************************************************************************
#define AM_HAL_VCOMP_INT_OUTHI            AM_REG_VCOMP_INTEN_OUTHI_M
#define AM_HAL_VCOMP_INT_OUTLO            AM_REG_VCOMP_INTEN_OUTLOW_M
//! @}

//*****************************************************************************
//
//! @brief Configuration struct
//
//*****************************************************************************
typedef struct
{
    //
    //! The DAC level setting
    //
    uint32_t ui32LevelSelect;

    //
    //! The "positive" comparator input channel
    //!
    //! This channel is usually used as the signal to be monitored.
    //
    uint32_t ui32PosInput;

    //
    //! The "negative" comparator input channel
    //!
    //! This channel is usually used as the reference signal.
    //
    uint32_t ui32NegInput;
}
am_hal_vcomp_config_t;

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_hal_vcomp_config(const am_hal_vcomp_config_t *psConfig);
extern void am_hal_vcomp_dac_level_set(uint32_t ui3Level);
extern bool am_hal_vcomp_read(void);
extern void am_hal_vcomp_enable(void);
extern void am_hal_vcomp_disable(void);
extern void am_hal_vcomp_int_enable(uint32_t ui32Interrupt);
extern uint32_t am_hal_vcomp_int_enable_get(void);
extern void am_hal_vcomp_int_disable(uint32_t ui32Interrupt);
extern void am_hal_vcomp_int_clear(uint32_t ui32Interrupt);
extern void am_hal_vcomp_int_set(uint32_t ui32Interrupt);
extern uint32_t am_hal_vcomp_int_status_get(bool bEnabledOnly);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_VCOMP_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
