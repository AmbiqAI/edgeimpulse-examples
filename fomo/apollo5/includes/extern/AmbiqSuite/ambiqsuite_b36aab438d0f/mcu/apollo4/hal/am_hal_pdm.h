//*****************************************************************************
//
//! @file am_hal_pdm.h
//!
//! @brief API for the PDM module
//!
//! @addtogroup
//! @ingroup
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

#ifndef AM_HAL_PDM_H
#define AM_HAL_PDM_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// CMSIS-style macro for handling a variable IOS module number.
//
//*****************************************************************************
#define AM_REG_PDM_NUM_MODULES                       4
#define PDMn(n) ((PDM0_Type*)(PDM0_BASE + (n * (PDM1_BASE - PDM0_BASE))))

//*****************************************************************************
//
// DMA threshold minimum.
//
// The PDM DMA works best if its threshold value is set to a multiple of 4
// between 16 and 24, but it will technically allow threshold settings between
// 4 and 24. This macro sets the minimum threshold value that the HAL layer
// will allow.
//
//*****************************************************************************
#define AM_HAL_PDM_DMA_THRESHOLD_MIN        16

//*****************************************************************************
//
// PDM-specific error conditions.
//
//*****************************************************************************
typedef enum
{
    //
    // The PDM HAL will throw this error if it can't find a threshold value to
    // match the total-count value passed in by a caller requesting a DMA
    // transfer. The PDM hardware requires all DMA transactions to be evenly
    // divisible in chunks of one FIFO size or smaller. Try changing your
    // ui32TotalCount value to a more evenly divisible number.
    //
    AM_HAL_PDM_STATUS_BAD_TOTALCOUNT = AM_HAL_STATUS_MODULE_SPECIFIC_START,
}
am_hal_pdm_status_e;

//*****************************************************************************
//
// Gain settings.
//
//*****************************************************************************
typedef enum
{
    AM_HAL_PDM_GAIN_P345DB = PDM0_CORECFG0_PGAL_P34_5DB,
    AM_HAL_PDM_GAIN_P330DB = PDM0_CORECFG0_PGAL_P33_0DB,
    AM_HAL_PDM_GAIN_P315DB = PDM0_CORECFG0_PGAL_P31_5DB,
    AM_HAL_PDM_GAIN_P300DB = PDM0_CORECFG0_PGAL_P30_0DB,
    AM_HAL_PDM_GAIN_P285DB = PDM0_CORECFG0_PGAL_P28_5DB,
    AM_HAL_PDM_GAIN_P270DB = PDM0_CORECFG0_PGAL_P27_0DB,
    AM_HAL_PDM_GAIN_P255DB = PDM0_CORECFG0_PGAL_P25_5DB,
    AM_HAL_PDM_GAIN_P240DB = PDM0_CORECFG0_PGAL_P24_0DB,
    AM_HAL_PDM_GAIN_P225DB = PDM0_CORECFG0_PGAL_P22_5DB,
    AM_HAL_PDM_GAIN_P210DB = PDM0_CORECFG0_PGAL_P21_0DB,
    AM_HAL_PDM_GAIN_P195DB = PDM0_CORECFG0_PGAL_P19_5DB,
    AM_HAL_PDM_GAIN_P180DB = PDM0_CORECFG0_PGAL_P18_0DB,
    AM_HAL_PDM_GAIN_P165DB = PDM0_CORECFG0_PGAL_P16_5DB,
    AM_HAL_PDM_GAIN_P150DB = PDM0_CORECFG0_PGAL_P15_0DB,
    AM_HAL_PDM_GAIN_P135DB = PDM0_CORECFG0_PGAL_P13_5DB,
    AM_HAL_PDM_GAIN_P120DB = PDM0_CORECFG0_PGAL_P12_0DB,
    AM_HAL_PDM_GAIN_P105DB = PDM0_CORECFG0_PGAL_P10_5DB,
    AM_HAL_PDM_GAIN_P90DB  = PDM0_CORECFG0_PGAL_P9_0DB,
    AM_HAL_PDM_GAIN_P75DB  = PDM0_CORECFG0_PGAL_P7_5DB,
    AM_HAL_PDM_GAIN_P60DB  = PDM0_CORECFG0_PGAL_P6_0DB,
    AM_HAL_PDM_GAIN_P45DB  = PDM0_CORECFG0_PGAL_P4_5DB,
    AM_HAL_PDM_GAIN_P30DB  = PDM0_CORECFG0_PGAL_P3_0DB,
    AM_HAL_PDM_GAIN_P15DB  = PDM0_CORECFG0_PGAL_P1_5DB,
    AM_HAL_PDM_GAIN_0DB    = PDM0_CORECFG0_PGAL_0DB,
    AM_HAL_PDM_GAIN_M15DB  = PDM0_CORECFG0_PGAL_M1_5DB,
    AM_HAL_PDM_GAIN_M30DB  = PDM0_CORECFG0_PGAL_M3_0DB,
    AM_HAL_PDM_GAIN_M45DB  = PDM0_CORECFG0_PGAL_M4_5DB,
    AM_HAL_PDM_GAIN_M60DB  = PDM0_CORECFG0_PGAL_M6_0DB,
    AM_HAL_PDM_GAIN_M75DB  = PDM0_CORECFG0_PGAL_M7_5DB,
    AM_HAL_PDM_GAIN_M90DB  = PDM0_CORECFG0_PGAL_M9_0DB,
    AM_HAL_PDM_GAIN_M105DB = PDM0_CORECFG0_PGAL_M10_5DB,
    AM_HAL_PDM_GAIN_M120DB = PDM0_CORECFG0_PGAL_M12_0DB
}
am_hal_pdm_gain_e;

//*****************************************************************************
//
// gain step size.
//
//*****************************************************************************
typedef enum
{
  AM_HAL_PDM_GAIN_STEP_0_13DB = PDM0_CORECFG1_SELSTEP_0_13DB,
  AM_HAL_PDM_GAIN_STEP_0_26DB = PDM0_CORECFG1_SELSTEP_0_26DB
}
am_hal_pdm_gain_stepsize_e;


//*****************************************************************************
//
// high pass filter enable/disable.
//
//*****************************************************************************
typedef enum
{
  AM_HAL_PDM_HIGH_PASS_ENABLE = 0,
  AM_HAL_PDM_HIGH_PASS_DISABLE = 1
}
am_hal_pdm_highpass_filter_onoff_e;

//*****************************************************************************
//
// PDM internal clock speed selection.
//
//*****************************************************************************
typedef enum
{
    AM_HAL_PDM_CLK_48MHZ        = PDM0_CTRL_CLKSEL_48MHZ,
    AM_HAL_PDM_CLK_24MHZ        = PDM0_CTRL_CLKSEL_24MHZ,
    AM_HAL_PDM_CLK_12MHZ        = PDM0_CTRL_CLKSEL_12MHZ,
    AM_HAL_PDM_CLK_6MHZ         = PDM0_CTRL_CLKSEL_6MHZ,
    AM_HAL_PDM_CLK_3MHZ         = PDM0_CTRL_CLKSEL_3MHZ,
    AM_HAL_PDM_CLK_1_5MHZ       = PDM0_CTRL_CLKSEL_1_5MHZ,
    AM_HAL_PDM_CLK_750KHZ       = PDM0_CTRL_CLKSEL_750KHZ,
    AM_HAL_PDM_CLK_HS_CRYSTAL   = PDM0_CTRL_CLKSEL_HSXTAL
}
am_hal_pdm_clkspd_e;

//*****************************************************************************
//
// PDM clock divider setting.
// DIVMCLKQ REG_PDM_CORECFG1(3:2)
// Divide down ratio for generating internal master MCLKQ.
// Fmclkq = Fpdmclk/(DIVMCLKQ) if DIVMCLKQ > 0;
//
//*****************************************************************************
typedef enum
{
    AM_HAL_PDM_MCLKDIV_3 = PDM0_CORECFG1_DIVMCLKQ_DIV3,
    AM_HAL_PDM_MCLKDIV_2 = PDM0_CORECFG1_DIVMCLKQ_DIV2,
    AM_HAL_PDM_MCLKDIV_1 = PDM0_CORECFG1_DIVMCLKQ_DIV1,
}
am_hal_pdm_mclkdiv_e;

//*****************************************************************************
//
// PDMA_CKO frequency divisor. Fpdma_cko = Fmclk_l/(MCLKDIV+1)
//
//*****************************************************************************
// #warning TODO FIXME - These enums should match, e.g. DIV15 should equal DIV15 (not DIV16).
typedef enum
{
    AM_HAL_PDM_PDMA_CLKO_DIV15 = PDM0_CORECFG0_MCLKDIV_DIV16,
    AM_HAL_PDM_PDMA_CLKO_DIV14 = PDM0_CORECFG0_MCLKDIV_DIV15,
    AM_HAL_PDM_PDMA_CLKO_DIV13 = PDM0_CORECFG0_MCLKDIV_DIV14,
    AM_HAL_PDM_PDMA_CLKO_DIV12 = PDM0_CORECFG0_MCLKDIV_DIV13,
    AM_HAL_PDM_PDMA_CLKO_DIV11 = PDM0_CORECFG0_MCLKDIV_DIV12,
    AM_HAL_PDM_PDMA_CLKO_DIV10 = PDM0_CORECFG0_MCLKDIV_DIV11,
    AM_HAL_PDM_PDMA_CLKO_DIV9  = PDM0_CORECFG0_MCLKDIV_DIV10,
    AM_HAL_PDM_PDMA_CLKO_DIV8  = PDM0_CORECFG0_MCLKDIV_DIV9,
    AM_HAL_PDM_PDMA_CLKO_DIV7  = PDM0_CORECFG0_MCLKDIV_DIV8,
    AM_HAL_PDM_PDMA_CLKO_DIV6  = PDM0_CORECFG0_MCLKDIV_DIV7,
    AM_HAL_PDM_PDMA_CLKO_DIV5  = PDM0_CORECFG0_MCLKDIV_DIV6,
    AM_HAL_PDM_PDMA_CLKO_DIV4  = PDM0_CORECFG0_MCLKDIV_DIV5,
    AM_HAL_PDM_PDMA_CLKO_DIV3  = PDM0_CORECFG0_MCLKDIV_DIV4,
    AM_HAL_PDM_PDMA_CLKO_DIV2  = PDM0_CORECFG0_MCLKDIV_DIV3,
    AM_HAL_PDM_PDMA_CLKO_DIV1  = PDM0_CORECFG0_MCLKDIV_DIV2,
    AM_HAL_PDM_PDMA_CLKO_DIV0  = PDM0_CORECFG0_MCLKDIV_DIV1
}
am_hal_pdm_pdma_clkodiv_e;

//*****************************************************************************
//
// PDMA_CKO clock phase delay in terms of PDMCLK period to internal sampler
//
//*****************************************************************************
typedef enum
{
    AM_HAL_PDM_CLKOUT_PHSDLY_CYCLE7 = PDM0_CORECFG1_CKODLY_7CYCLES,
    AM_HAL_PDM_CLKOUT_PHSDLY_CYCLE6 = PDM0_CORECFG1_CKODLY_6CYCLES,
    AM_HAL_PDM_CLKOUT_PHSDLY_CYCLE5 = PDM0_CORECFG1_CKODLY_5CYCLES,
    AM_HAL_PDM_CLKOUT_PHSDLY_CYCLE4 = PDM0_CORECFG1_CKODLY_4CYCLES,
    AM_HAL_PDM_CLKOUT_PHSDLY_CYCLE3 = PDM0_CORECFG1_CKODLY_3CYCLES,
    AM_HAL_PDM_CLKOUT_PHSDLY_CYCLE2 = PDM0_CORECFG1_CKODLY_2CYCLES,
    AM_HAL_PDM_CLKOUT_PHSDLY_CYCLE1 = PDM0_CORECFG1_CKODLY_1CYCLES,
    AM_HAL_PDM_CLKOUT_PHSDLY_NONE   = PDM0_CORECFG1_CKODLY_0CYCLES
}
am_hal_pdm_clkout_phsdly_e;

//*****************************************************************************
//
// Set number of PDMA_CKO cycles during gain setting changes or soft mute
//
//*****************************************************************************
typedef enum
{
    AM_HAL_PDM_CLKOUT_DELAY_CYCLE7 = PDM0_CORECFG0_SCYCLES_7CYCLES,
    AM_HAL_PDM_CLKOUT_DELAY_CYCLE6 = PDM0_CORECFG0_SCYCLES_6CYCLES,
    AM_HAL_PDM_CLKOUT_DELAY_CYCLE5 = PDM0_CORECFG0_SCYCLES_5CYCLES,
    AM_HAL_PDM_CLKOUT_DELAY_CYCLE4 = PDM0_CORECFG0_SCYCLES_4CYCLES,
    AM_HAL_PDM_CLKOUT_DELAY_CYCLE3 = PDM0_CORECFG0_SCYCLES_3CYCLES,
    AM_HAL_PDM_CLKOUT_DELAY_CYCLE2 = PDM0_CORECFG0_SCYCLES_2CYCLES,
    AM_HAL_PDM_CLKOUT_DELAY_CYCLE1 = PDM0_CORECFG0_SCYCLES_1CYCLES,
    AM_HAL_PDM_CLKOUT_DELAY_NONE   = PDM0_CORECFG0_SCYCLES_0CYCLES
}
am_hal_pdm_clkout_delay_e;

//*****************************************************************************
//
// PCM Channel Select.
//
//*****************************************************************************
typedef enum
{
    AM_HAL_PDM_CHANNEL_LEFT   = PDM0_CORECFG1_PCMCHSET_MONOL,
    AM_HAL_PDM_CHANNEL_RIGHT  = PDM0_CORECFG1_PCMCHSET_MONOR,
    AM_HAL_PDM_CHANNEL_STEREO = PDM0_CORECFG1_PCMCHSET_STEREO,
}
am_hal_pdm_chset_e;

//*****************************************************************************
//
// PDM power state settings.
//
//*****************************************************************************
#define AM_HAL_PDM_POWER_ON           AM_HAL_SYSCTRL_WAKE
#define AM_HAL_PDM_POWER_OFF          AM_HAL_SYSCTRL_NORMALSLEEP

//*****************************************************************************
//
// PDM interrupts.
//
//*****************************************************************************
#define AM_HAL_PDM_INT_DERR           PDM0_INTSTAT_DERR_Msk
#define AM_HAL_PDM_INT_DCMP           PDM0_INTSTAT_DCMP_Msk
#define AM_HAL_PDM_INT_UNDFL          PDM0_INTSTAT_UNDFL_Msk
#define AM_HAL_PDM_INT_OVF            PDM0_INTSTAT_OVF_Msk
#define AM_HAL_PDM_INT_THR            PDM0_INTSTAT_THR_Msk

//*****************************************************************************
//
// PDM DMA STATE.
//
//*****************************************************************************
#define AM_HAL_PDM_DMASTAT_DMATIP     PDM0_DMASTAT_DMATIP_Msk
#define AM_HAL_PDM_DMASTAT_DCMP       PDM0_DMASTAT_DMACPL_Msk
#define AM_HAL_PDM_DMASTAT_DMAERR     PDM0_DMASTAT_DMAERR_Msk

//*****************************************************************************
//
// Configuration structure for the PDM
//
//*****************************************************************************
typedef struct
{
    //
    // Clock
    //
    //Divide down ratio for generating internal master MCLKQ.
    am_hal_pdm_mclkdiv_e eClkDivider;
    // PDMA_CKO frequency divisor.Fpdma_cko = Fmclk_l/(MCLKDIV+1)
    am_hal_pdm_pdma_clkodiv_e ePDMAClkOutDivder ;

    // Gain
    am_hal_pdm_gain_e eLeftGain;
    am_hal_pdm_gain_e eRightGain;
    // Fine grain step size for smooth PGA or Softmute attenuation
    // transition0: 0.13dB1: 0.26dB
    am_hal_pdm_gain_stepsize_e eStepSize;

    // Decimation Rate
    uint32_t ui32DecimationRate;

    // Filters
    bool bHighPassEnable;
    // HPGAIN: Adjust High Pass Coefficients
    uint32_t ui32HighPassCutoff;

    // PDM Clock select.
    am_hal_pdm_clkspd_e ePDMClkSpeed;

    // PCMPACK
    bool bDataPacking;

    // CHSET
    am_hal_pdm_chset_e ePCMChannels;

    // CKODLY
    am_hal_pdm_clkout_phsdly_e bPDMSampleDelay;

    // SCYCLES
    am_hal_pdm_clkout_delay_e ui32GainChangeDelay;

    bool bSoftMute;

    bool bLRSwap;
}
am_hal_pdm_config_t;

//*****************************************************************************
//
// DMA transfer structure
//
//*****************************************************************************
typedef struct
{
    uint32_t ui32TargetAddr;
    uint32_t ui32TotalCount;
}
am_hal_pdm_transfer_t;

// Init/De-init.
extern uint32_t am_hal_pdm_initialize(uint32_t ui32Module, void **ppHandle);
extern uint32_t am_hal_pdm_deinitialize(void *pHandle);

// Power
extern uint32_t am_hal_pdm_power_control(void *pHandle, am_hal_sysctrl_power_state_e ePowerState, bool bRetainState);

// Config
extern uint32_t am_hal_pdm_configure(void *pHandle, am_hal_pdm_config_t *psConfig);

// Enable/Disable
extern uint32_t am_hal_pdm_enable(void *pHandle);
extern uint32_t am_hal_pdm_disable(void *pHandle);
extern uint32_t am_hal_pdm_reset(void *pHandle);

// Gather PDM data.
extern uint32_t am_hal_pdm_dma_start(void *pHandle, am_hal_pdm_transfer_t *pDmaCfg);
extern uint32_t am_hal_pdm_dma_state(void *pHandle);
extern void am_hal_pdm_dma_reset_count(uint32_t count);

// Flush the PDM FIFO.
extern uint32_t am_hal_pdm_fifo_flush(void *pHandle);

// Read FIFO data.
uint32_t am_hal_pdm_fifo_data_read(void *pHandle);

uint32_t am_hal_pdm_fifo_data_reads(void *pHandle, uint8_t* buffer, uint32_t size);

uint32_t am_hal_pdm_fifo_count_get(void *pHandle);
uint32_t am_hal_pdm_fifo_threshold_setup(void *pHandle, uint32_t value);


// I2S Passthrough
extern uint32_t am_hal_pdm_i2s_enable(void *pHandle);
extern uint32_t am_hal_pdm_i2s_disable(void *pHandle);

// Interrupts.
extern uint32_t am_hal_pdm_interrupt_enable(void *pHandle, uint32_t ui32IntMask);
extern uint32_t am_hal_pdm_interrupt_disable(void *pHandle, uint32_t ui32IntMask);
extern uint32_t am_hal_pdm_interrupt_clear(void *pHandle, uint32_t ui32IntMask);
extern uint32_t am_hal_pdm_interrupt_status_get(void *pHandle, uint32_t *pui32Status, bool bEnabledOnly);
// #### INTERNAL BEGIN ####
//
// Test use only!
//
extern uint32_t am_hal_pdm_find_dma_threshold(uint32_t ui32TotalCount);
// #### INTERNAL END ####

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_PDM_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

