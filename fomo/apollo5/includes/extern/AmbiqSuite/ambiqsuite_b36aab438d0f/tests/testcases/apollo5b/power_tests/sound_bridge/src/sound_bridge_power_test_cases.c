#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include <math.h>
#include "predefined_value.h"

//*****************************************************************************
//
// Test parameters.
//
//*****************************************************************************
#define ENUM_TEST_MODE          (AUDADC_TO_I2S)
//                              I2S_IN_ONLY
//                              PDM_IN_ONLY
//                              AUDADC_IN_ONLY
//                              I2S_OUT_ONLY
//                              I2S_TO_I2S
//                              PDM_TO_I2S
//                              AUDADC_TO_I2S

#define ENUM_CLOCK_SOURCE       (CLKSRC_HFRC)
//                              CLKSRC_HFRC
//                              CLKSRC_HF2ADJ
//                              CLKSRC_PLL

#define ENUM_I2S_IN_MODULE      (I2S0_MODULE)
//                              I2S0_MODULE
//                              I2S1_MODULE

#define ENUM_I2S_IN_ROLE        (AM_HAL_I2S_IO_MODE_MASTER)
#define ENUM_I2S_OUT_ROLE       (AM_HAL_I2S_IO_MODE_MASTER)
//                              AM_HAL_I2S_IO_MODE_MASTER
//                              AM_HAL_I2S_IO_MODE_SLAVE

#define UINT_SYS_SAMPLE_RATES   (16000)
//                              {8000, 16000, 24000, 32000}
//                              When selecting HFRC as the clock source, the actual sampleing rates of PDM and I2S are {7812.5, 15625, 23437.5, 31250}.

#define UINT_I2S_IN_CHANNELS    (2)
//                              [1, 8]

#define UINT_I2S_IN_BITS        (24)
//                              {8, 16, 24, 32}

#define UINT_I2S_OUT_CHANNELS   (2)
//                              [1, 8]

#define UINT_I2S_OUT_BITS       (24)
//                              {8, 16, 24, 32}

#define UINT_PDM_CHANNELS       (2)
//                              [1, 2]

#define UINT_PDM_SINCRATE       (48)
//                              {16, 24, 32, 48, 64}

#define UINT_DMA_FRAME_MS       (5)
//                              [1, inf]

#define BOOL_ENABLE_IO_TOGGLE   (1)

#include "check_macro.h"

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
    am_hal_itm_disable();

    am_hal_pwrctrl_sram_memcfg_t SRAMMemCfg =
    {
      .eSRAMCfg         = AM_HAL_PWRCTRL_SRAM_1M,
      .eActiveWithMCU   = AM_HAL_PWRCTRL_SRAM_NONE,
      .eActiveWithGFX   = AM_HAL_PWRCTRL_SRAM_NONE,
      .eActiveWithDISP  = AM_HAL_PWRCTRL_SRAM_NONE,
      .eSRAMRetain      = AM_HAL_PWRCTRL_SRAM_1M
    };

    am_hal_pwrctrl_mcu_memory_config_t McuMemCfg =
    {
        .eROMMode              = AM_HAL_PWRCTRL_ROM_AUTO,
        .eDTCMCfg              = AM_HAL_PWRCTRL_ITCM256K_DTCM512K,
        .eRetainDTCM           = AM_HAL_PWRCTRL_MEMRETCFG_TCMPWDSLP_RETAIN,
        .eNVMCfg               = AM_HAL_PWRCTRL_NVM0_ONLY,
        .bKeepNVMOnInDeepSleep = false
    };

    am_hal_pwrctrl_sram_config(&SRAMMemCfg);
    am_hal_pwrctrl_mcu_memory_config(&McuMemCfg);
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_DIS_PERIPHS_ALL, 0);

    MCUCTRL->APBDMACTRL_b.HYSTERESIS = 0x0;
    MCUCTRL->DBGCTRL = 0;

    // MCUCTRL->AUDADCPWRDLY_b.AUDADCPWR1 = 2;

    #if BOOL_ENABLE_IO_TOGGLE
    am_hal_gpio_pinconfig(PDM_ISR_IO, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(I2S_IN_ISR_IO, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(I2S_OUT_ISR_IO, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(AUDADC_ISR_IO, am_hal_gpio_pincfg_output);
    #endif

    #if ((ENUM_TEST_MODE & I2S_IN) | (ENUM_TEST_MODE & I2S_OUT))
    if (APOLLO5_B0)
    {
        if ((MCUCTRL->PLLMUXCTL_b.I2S0PLLCLKSEL != MCUCTRL_PLLMUXCTL_I2S0PLLCLKSEL_CLKGEN) ||
            (MCUCTRL->PLLMUXCTL_b.I2S1PLLCLKSEL != MCUCTRL_PLLMUXCTL_I2S1PLLCLKSEL_CLKGEN))
        {
            am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
            am_hal_delay_us(1500);
            am_hal_pwrctrl_syspll_enable();
            SYSPLLn(0)->PLLCTL0_b.FOUTPOSTDIVPD  = MCUCTRL_PLLCTL0_FOUTPOSTDIVPD_ACTIVE;
            SYSPLLn(0)->PLLCTL0_b.FOUT4PHASEPD   = MCUCTRL_PLLCTL0_FOUT4PHASEPD_ACTIVE;
            SYSPLLn(0)->PLLCTL0_b.FREFSEL        = MCUCTRL_PLLCTL0_FREFSEL_XTAL32MHz;
            SYSPLLn(0)->PLLCTL0_b.BYPASS         = 1;
            SYSPLLn(0)->PLLCTL0_b.SYSPLLPDB      = MCUCTRL_PLLCTL0_SYSPLLPDB_ENABLE;
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_I2S0);
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_I2S1);
            I2Sn(0)->CLKCFG_b.MCLKEN = 1;
            I2Sn(1)->CLKCFG_b.MCLKEN = 1;
            MCUCTRL->PLLMUXCTL_b.I2S0PLLCLKSEL = MCUCTRL_PLLMUXCTL_I2S0PLLCLKSEL_CLKGEN;
            MCUCTRL->PLLMUXCTL_b.I2S1PLLCLKSEL = MCUCTRL_PLLMUXCTL_I2S1PLLCLKSEL_CLKGEN;
            am_hal_delay_us(20);
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_I2S0);
            am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_I2S1);
            SYSPLLn(0)->PLLCTL0_b.SYSPLLPDB      = MCUCTRL_PLLCTL0_SYSPLLPDB_DISABLE;
            SYSPLLn(0)->PLLCTL0_b.BYPASS         = 0;
            SYSPLLn(0)->PLLCTL0_b.FREFSEL        = MCUCTRL_PLLCTL0_FREFSEL_XTAL32MHz;
            SYSPLLn(0)->PLLCTL0_b.FOUT4PHASEPD   = MCUCTRL_PLLCTL0_FOUT4PHASEPD_POWERDOWN;
            SYSPLLn(0)->PLLCTL0_b.FOUTPOSTDIVPD  = MCUCTRL_PLLCTL0_FOUTPOSTDIVPD_POWERDOWN;
            am_hal_pwrctrl_syspll_disable();
            am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_DISABLE, false);
        }
    }
    #endif
}

void
globalTearDown(void)
{
}

#if (ENUM_TEST_MODE & I2S_IN) || (ENUM_TEST_MODE & I2S_OUT)
const IRQn_Type i2sInterrupts[] =
{
    I2S0_IRQn,
    I2S1_IRQn
};

void i2s_init(uint32_t ui32Module, void** ppHandle, am_hal_i2s_config_t *pConfig)
{
    //
    // Configure the necessary pins.
    //
    am_bsp_i2s_pins_enable(ui32Module, false);

    //
    // Configure the I2S.
    //
    am_hal_i2s_initialize(ui32Module, ppHandle);
    am_hal_i2s_power_control(*ppHandle, AM_HAL_I2S_POWER_ON, false);
    am_hal_i2s_configure(*ppHandle, pConfig);
    am_hal_i2s_enable(*ppHandle);

    NVIC_SetPriority(i2sInterrupts[ui32Module], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(i2sInterrupts[ui32Module]);
}

void i2s_deinit(uint32_t ui32Module, void** ppHandle)
{
    am_hal_i2s_dma_transfer_complete(*ppHandle);

    am_hal_i2s_interrupt_clear(*ppHandle, (AM_HAL_I2S_INT_RXDMACPL | AM_HAL_I2S_INT_TXDMACPL));
    am_hal_i2s_interrupt_disable(*ppHandle, (AM_HAL_I2S_INT_RXDMACPL | AM_HAL_I2S_INT_TXDMACPL));

    NVIC_DisableIRQ(i2sInterrupts[ui32Module]);

    am_bsp_i2s_pins_disable(ui32Module, false);

    am_hal_i2s_disable(*ppHandle);
    am_hal_i2s_power_control(*ppHandle, AM_HAL_I2S_POWER_OFF, false);
    am_hal_i2s_deinitialize(*ppHandle);
}

#if ENUM_TEST_MODE & I2S_IN
void *i2sInHandle;
bool i2sInDmaCpl = false;

am_hal_i2s_io_signal_t i2sInIoFmt =
{
    .sFsyncPulseCfg = {
        .eFsyncPulseType = AM_HAL_I2S_FSYNC_PULSE_ONE_SUBFRAME,
    },
    .eFyncCpol = AM_HAL_I2S_IO_FSYNC_CPOL_LOW,
    .eTxCpol   = AM_HAL_I2S_IO_TX_CPOL_FALLING,
    .eRxCpol   = AM_HAL_I2S_IO_RX_CPOL_RISING,
};

am_hal_i2s_data_format_t i2sDataInFmt =
{
    .ePhase                   = AM_HAL_I2S_DATA_PHASE_SINGLE,
    .eChannelLenPhase1        = I2S_IN_WORD_WIDTH,
    .ui32ChannelNumbersPhase1 = UINT_I2S_IN_CHANNELS,
    .eDataDelay               = 0x1,
    .eSampleLenPhase1         = I2S_IN_BIT_DEPTH,
    .eDataJust                = AM_HAL_I2S_DATA_JUSTIFIED_LEFT,
};

am_hal_i2s_config_t i2sInConfig =
{
    .eMode  = ENUM_I2S_IN_ROLE,
    .eXfer  = AM_HAL_I2S_XFER_RX,
    .eClock = I2S_IN_CLK,
    .eDiv3  = I2S_IN_CLK_DIV3,
    .eASRC  = 0,
    .eData  = &i2sDataInFmt,
    .eIO    = &i2sInIoFmt,
};

AM_SHARED_RW uint32_t i2sInDmaBuffer[2 * I2S_IN_DMA_BUF_SIZE / sizeof(uint32_t)] __attribute__((aligned(32)));

am_hal_i2s_transfer_t i2sInDmaCfg =
{
    .ui32RxTargetAddr = (uint32_t)(&i2sInDmaBuffer[0]),
    .ui32RxTargetAddrReverse = (uint32_t)(&i2sInDmaBuffer[I2S_IN_DMA_BUF_SIZE / sizeof(uint32_t)]),
    .ui32RxTotalCount = I2S_IN_DMA_BUF_SIZE / sizeof(uint32_t),
};

void i2s_in_isr()
{
    uint32_t ui32Status;

    #if BOOL_ENABLE_IO_TOGGLE
    am_hal_gpio_output_set(I2S_IN_ISR_IO);
    #endif

    am_hal_i2s_interrupt_status_get(i2sInHandle, &ui32Status, true);
    am_hal_i2s_interrupt_clear(i2sInHandle, ui32Status);

    //
    // Swich ping pong buffer.
    //
    am_hal_i2s_interrupt_service(i2sInHandle, ui32Status, &i2sInConfig);

    if (ui32Status & AM_HAL_I2S_INT_RXDMACPL)
    {
        i2sInDmaCpl = 1;
    }

    #if BOOL_ENABLE_IO_TOGGLE
    am_hal_gpio_output_clear(I2S_IN_ISR_IO);
    #endif
}
#endif

#if ENUM_TEST_MODE & I2S_OUT
void *i2sOutHandle;
bool i2sOutDmaCpl = false;

am_hal_i2s_io_signal_t i2sOutIoFmt =
{
    .sFsyncPulseCfg = {
        .eFsyncPulseType = AM_HAL_I2S_FSYNC_PULSE_ONE_SUBFRAME,
    },
    .eFyncCpol = AM_HAL_I2S_IO_FSYNC_CPOL_LOW,
    .eTxCpol   = AM_HAL_I2S_IO_TX_CPOL_FALLING,
    .eRxCpol   = AM_HAL_I2S_IO_RX_CPOL_RISING,
};

am_hal_i2s_data_format_t i2sDataOutFmt =
{
    .ePhase                   = AM_HAL_I2S_DATA_PHASE_SINGLE,
    .eChannelLenPhase1        = I2S_OUT_WORD_WIDTH,
    .ui32ChannelNumbersPhase1 = UINT_I2S_OUT_CHANNELS,
    .eDataDelay               = 0x1,
    .eSampleLenPhase1         = I2S_OUT_BIT_DEPTH,
    .eDataJust                = AM_HAL_I2S_DATA_JUSTIFIED_LEFT,
};

am_hal_i2s_config_t i2sOutConfig =
{
    .eMode  = ENUM_I2S_OUT_ROLE,
    .eXfer  = AM_HAL_I2S_XFER_TX,
    .eClock = I2S_OUT_CLK,
    .eDiv3  = I2S_OUT_CLK_DIV3,
    .eASRC  = 0,
    .eData  = &i2sDataOutFmt,
    .eIO    = &i2sOutIoFmt,
};

AM_SHARED_RW uint32_t i2sOutDmaBuffer[2 * I2S_OUT_DMA_BUF_SIZE / sizeof(uint32_t)] __attribute__((aligned(32)));

am_hal_i2s_transfer_t i2sOutDmaCfg =
{
    .ui32TxTargetAddr = (uint32_t)(&i2sOutDmaBuffer[0]),
    .ui32TxTargetAddrReverse = (uint32_t)(&i2sOutDmaBuffer[I2S_OUT_DMA_BUF_SIZE / sizeof(uint32_t)]),
    .ui32TxTotalCount = I2S_OUT_DMA_BUF_SIZE / sizeof(uint32_t),
};

void i2s_out_isr()
{
    uint32_t ui32Status;

    #if BOOL_ENABLE_IO_TOGGLE
    am_hal_gpio_output_set(I2S_OUT_ISR_IO);
    #endif

    am_hal_i2s_interrupt_status_get(i2sOutHandle, &ui32Status, true);
    am_hal_i2s_interrupt_clear(i2sOutHandle, ui32Status);

    //
    // Swich ping pong buffer.
    //
    am_hal_i2s_interrupt_service(i2sOutHandle, ui32Status, &i2sOutConfig);

    if (ui32Status & AM_HAL_I2S_INT_TXDMACPL)
    {
        i2sOutDmaCpl = 1;
    }

    #if BOOL_ENABLE_IO_TOGGLE
    am_hal_gpio_output_clear(I2S_OUT_ISR_IO);
    #endif
}
#endif
#endif

#if ENUM_TEST_MODE & PDM_IN
void *pdm0Handle;
bool pdmDmaCpl = false;

const IRQn_Type pdmInterrupts[] =
{
    PDM0_IRQn,
};

am_hal_pdm_config_t pdmConfig =
{
    .eClkDivider         = PDM_MCLK_DIV,
    .ePDMAClkOutDivder   = PDM_CLKO_DIV,
    .ui32DecimationRate  = UINT_PDM_SINCRATE,
    .ePDMClkSpeed        = PDM_CLK,
    .eLeftGain           = AM_HAL_PDM_GAIN_0DB,
    .eRightGain          = AM_HAL_PDM_GAIN_0DB,
    .eStepSize           = AM_HAL_PDM_GAIN_STEP_0_13DB,
    .bHighPassEnable     = AM_HAL_PDM_HIGH_PASS_ENABLE,
    .ui32HighPassCutoff  = 0x3,
    .bDataPacking        = 0,
    .ePCMChannels        = PDM_CHANNEL,
    .bPDMSampleDelay     = AM_HAL_PDM_CLKOUT_PHSDLY_NONE,
    .ui32GainChangeDelay = AM_HAL_PDM_CLKOUT_DELAY_NONE,
    .bSoftMute           = 0,
    .bLRSwap             = 0,
};

AM_SHARED_RW uint32_t pdmDmaBuffer[2 * PDM_DMA_BUF_SIZE / sizeof(uint32_t)] __attribute__((aligned(32)));

am_hal_pdm_transfer_t pdmDmaConfig =
{
    .ui32TargetAddr        = (uint32_t)(&pdmDmaBuffer[0]),
    .ui32TargetAddrReverse = (uint32_t)(&pdmDmaBuffer[PDM_DMA_BUF_SIZE / sizeof(uint32_t)]),
    .ui32TotalCount        = PDM_DMA_BUF_SIZE,
};

void am_pdm0_isr(void)
{
    uint32_t ui32Status;

    #if BOOL_ENABLE_IO_TOGGLE
    am_hal_gpio_output_set(PDM_ISR_IO);
    #endif

    //
    // Read the interrupt status.
    //
    am_hal_pdm_interrupt_status_get(pdm0Handle, &ui32Status, true);
    am_hal_pdm_interrupt_clear(pdm0Handle, ui32Status);

    //
    // Swich ping pong buffer.
    //
    am_hal_pdm_interrupt_service(pdm0Handle, ui32Status, &pdmDmaConfig);

    if (ui32Status & AM_HAL_PDM_INT_DCMP)
    {
        pdmDmaCpl = true;
    }

    #if BOOL_ENABLE_IO_TOGGLE
    am_hal_gpio_output_clear(PDM_ISR_IO);
    #endif
}

void pdm_init(uint32_t ui32Module, void** ppHandle, am_hal_pdm_config_t *pConfig)
{
    //
    // Configure the necessary pins.
    //
    am_bsp_pdm_pins_enable(ui32Module);

    //
    // Initialize, power-up, and configure the PDM.
    //
    am_hal_pdm_initialize(ui32Module, ppHandle);
    am_hal_pdm_power_control(*ppHandle, AM_HAL_PDM_POWER_ON, false);
    am_hal_pdm_configure(*ppHandle, pConfig);

    //
    // Setup the FIFO threshold.
    //
    am_hal_pdm_fifo_threshold_setup(*ppHandle, 16);

    //
    // Configure and enable PDM interrupts (set up to trigger on DMA completion).
    //
    am_hal_pdm_interrupt_enable(*ppHandle, (AM_HAL_PDM_INT_DERR  |\
                                            AM_HAL_PDM_INT_DCMP  |\
                                            AM_HAL_PDM_INT_UNDFL |\
                                            AM_HAL_PDM_INT_OVF));

    am_hal_pdm_enable(*ppHandle);

    NVIC_SetPriority(pdmInterrupts[ui32Module], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(pdmInterrupts[ui32Module]);
}

//*****************************************************************************
//
// PDM deinitialization.
//
//*****************************************************************************
void pdm_deinit(uint32_t ui32Module, void** ppHandle)
{
    am_hal_pdm_interrupt_clear(*ppHandle, (AM_HAL_PDM_INT_DERR  |\
                                         AM_HAL_PDM_INT_DCMP  |\
                                         AM_HAL_PDM_INT_UNDFL |\
                                         AM_HAL_PDM_INT_OVF));

    am_hal_pdm_interrupt_disable(*ppHandle, (AM_HAL_PDM_INT_DERR  |\
                                           AM_HAL_PDM_INT_DCMP  |\
                                           AM_HAL_PDM_INT_UNDFL |\
                                           AM_HAL_PDM_INT_OVF));

    NVIC_DisableIRQ(PDM0_IRQn);

    am_bsp_pdm_pins_disable(ui32Module);

    am_hal_pdm_disable(*ppHandle);
    am_hal_pdm_power_control(*ppHandle, AM_HAL_PDM_POWER_OFF, false);
    am_hal_pdm_deinitialize(*ppHandle);
}
#endif

#if ENUM_TEST_MODE & AUDADC_IN
void *audadc0Handle;
bool audadcDmaCpl = false;

const IRQn_Type audadcInterrupts[] =
{
    AUDADC0_IRQn,
};

AM_SHARED_RW uint32_t audadcDmaBuffer[2 * AUDADC_DMA_BUF_SIZE / sizeof(uint32_t)] __attribute__((aligned(32)));

am_hal_audadc_dma_config_t audadcDmaConfig =
{
    .bDynamicPriority  = true,
    .ePriority         = AM_HAL_AUDADC_PRIOR_SERVICE_IMMED,
    .bDMAEnable        = true,
    .ui32SampleCount   = AUDADC_DMA_BUF_SIZE / sizeof(uint32_t),
    .ui32TargetAddress = (uint32_t)(&audadcDmaBuffer[0]),
    .ui32TargetAddressReverse = (uint32_t)(&audadcDmaBuffer[AUDADC_DMA_BUF_SIZE / sizeof(uint32_t)]),
};

void am_audadc0_isr(void)
{
    uint32_t ui32Status;

    #if BOOL_ENABLE_IO_TOGGLE
    am_hal_gpio_output_set(AUDADC_ISR_IO);
    #endif

    //
    // Read and clear interruput status.
    //
    am_hal_audadc_interrupt_status(audadc0Handle, &ui32Status, false);
    am_hal_audadc_interrupt_clear(audadc0Handle, ui32Status);

    //
    // Swich ping pong buffer.
    //
    am_hal_audadc_interrupt_service(audadc0Handle, ui32Status);

    if (ui32Status & AM_HAL_AUDADC_INT_DCMP)
    {
        audadcDmaCpl = true;
    }

    #if BOOL_ENABLE_IO_TOGGLE
    am_hal_gpio_output_clear(AUDADC_ISR_IO);
    #endif
}

void audadc_init(uint32_t ui32Module, void** ppHandle)
{
    // Initialize the AUDADC handler.
    am_hal_audadc_initialize(ui32Module, ppHandle);

    // Power AUDADC on.
    am_hal_audadc_power_control(*ppHandle, AM_HAL_SYSCTRL_WAKE, false);

    // Power up the reference voltage.
    am_hal_audadc_refgen_powerup();

    // Power up the bias voltage.
    am_hal_audadc_micbias_powerup(24);

    // Power up PGA A0 and PGA A1
    am_hal_audadc_pga_powerup(0);
    am_hal_audadc_pga_powerup(1);

    // Set gain.
    am_hal_audadc_gain_set(0, 2*PREAMP_FULL_GAIN);
    am_hal_audadc_gain_set(1, 2*PREAMP_FULL_GAIN);

    // Set PGA.
    am_hal_audadc_gain_config_t audadcGainConfig;
    audadcGainConfig.ui32LGA = (uint32_t)((float)CH_A0_GAIN_DB*2 + 12);
    audadcGainConfig.ui32HGADELTA = ((uint32_t)((float)CH_A1_GAIN_DB*2 + 12)) - audadcGainConfig.ui32LGA;
    audadcGainConfig.eUpdateMode = AM_HAL_AUDADC_GAIN_UPDATE_IMME;
    am_hal_audadc_internal_pga_config(*ppHandle, &audadcGainConfig);

    // Configure AUDADC using a predefined configuration.
    am_hal_audadc_config_t audadcConfig =
    {
        .eClock         = AUDADC_CLK,
        .ePolarity      = AM_HAL_AUDADC_TRIGPOL_RISING,
        .eTrigger       = AM_HAL_AUDADC_TRIGSEL_SOFTWARE,
        .eClockMode     = AM_HAL_AUDADC_CLKMODE_LOW_POWER,
        .ePowerMode     = AM_HAL_AUDADC_LPMODE1,
        .eRepeat        = AM_HAL_AUDADC_REPEATING_SCAN,
        .eRepeatTrigger = AM_HAL_AUDADC_RPTTRIGSEL_INT,
        .eSampMode      = AM_HAL_AUDADC_SAMPMODE_MED,
    };
    am_hal_audadc_configure(*ppHandle, &audadcConfig);

    // Config slot 0.
    am_hal_audadc_slot_config_t audadcSlot0Config =
    {
        .eMeasToAvg      = AM_HAL_AUDADC_SLOT_AVG_1,
        .ePrecisionMode  = AM_HAL_AUDADC_SLOT_12BIT,
        .ui32TrkCyc      = 34,
        .eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE0,
        .bWindowCompare  = false,
        .bEnabled        = true,
    };
    am_hal_audadc_configure_slot(*ppHandle, 0, &audadcSlot0Config);

    // Config slot 1.
    am_hal_audadc_slot_config_t audadcSlot1Config =
    {
        .eMeasToAvg      = AM_HAL_AUDADC_SLOT_AVG_1,
        .ePrecisionMode  = AM_HAL_AUDADC_SLOT_12BIT,
        .ui32TrkCyc      = 34,
        .eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE1,
        .bWindowCompare  = false,
        .bEnabled        = true,
    };
    am_hal_audadc_configure_slot(*ppHandle, 1, &audadcSlot1Config);

    // Set up internal repeat trigger timer.
    am_hal_audadc_irtt_config_t audadcIrttConfig =
    {
        .bIrttEnable      = false,
        .eClkDiv          = AUDADC_RPTT_CLK_DIV,
        .ui32IrttCountMax = AUDADC_COUNT_MAX,
    };
    am_hal_audadc_configure_irtt(*ppHandle, &audadcIrttConfig);

    // Enable internal repeat trigger timer.
    am_hal_audadc_irtt_enable(*ppHandle);

    // Configure and enable AUDADC interrupts.
    am_hal_audadc_interrupt_enable(*ppHandle, AM_HAL_AUDADC_INT_DCMP | AM_HAL_AUDADC_INT_DERR);

    // Enable AUDADC.
    am_hal_audadc_enable(*ppHandle);

    NVIC_SetPriority(audadcInterrupts[ui32Module], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(audadcInterrupts[ui32Module]);
}
#endif

uint32_t pll_config_calculate(am_hal_syspll_config_t *pllConfig, uint32_t freq)
{
    bool postDivFound = false;
    bool fbDivFound = false;
    uint32_t divTable[][3] =
    {
        { 1, 1, 1}, { 2, 2, 1}, { 3, 3, 1}, { 4, 4, 1}, { 5, 5, 1},
        { 6, 6, 1}, { 7, 7, 1}, { 8, 4, 2}, { 9, 3, 3}, {10, 5, 2},
        {12, 6, 2}, {14, 7, 2}, {15, 5, 3}, {16, 4, 4}, {18, 6, 3},
        {20, 5, 4}, {21, 7, 3}, {24, 6, 4}, {25, 5, 5}, {28, 7, 4},
        {30, 6, 5}, {35, 7, 5}, {36, 6, 6}, {42, 7, 6}, {49, 7, 7},
    };

    pllConfig->eFref = AM_HAL_SYSPLL_FREFSEL_XTAL32MHz;
    pllConfig->eVCOSel = AM_HAL_SYSPLL_VCOSEL_VCOLO;

    uint32_t div = (((AM_HAL_SYSPLL_VCO_LO_MODE_FREQ_MIN * 1000000) % freq) == 0) ?
                    ((AM_HAL_SYSPLL_VCO_LO_MODE_FREQ_MIN * 1000000) / freq) :
                    ((AM_HAL_SYSPLL_VCO_LO_MODE_FREQ_MIN * 1000000) / freq) + 1;

    for (uint32_t i = 0; i < (sizeof(divTable) / sizeof(uint32_t[3])) - 1; i++)
    {
        if (divTable[i][0] == div)
        {
            pllConfig->ui8PostDiv1 = divTable[i][1];
            pllConfig->ui8PostDiv2 = divTable[i][2];
            postDivFound = true;
            break;
        }
        else if ((divTable[i][0] < div) && (divTable[i+1][0] >= div))
        {
            pllConfig->ui8PostDiv1 = divTable[i+1][1];
            pllConfig->ui8PostDiv2 = divTable[i+1][2];
            postDivFound = true;
            break;
        }
    }

    if (postDivFound != true)
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    for (uint32_t i = 1; i < 64; i++)
    {
        uint64_t preDivFreq = (uint64_t)freq * pllConfig->ui8PostDiv1 * pllConfig->ui8PostDiv2 * i;
        uint32_t fbDiv = preDivFreq / (32000000ULL);

        if (preDivFreq % (32000000ULL) == 0)
        {
            if ((fbDiv >= AM_HAL_SYSPLL_MIN_FBDIV_INT_MODE) && (fbDiv <= AM_HAL_SYSPLL_MAX_FBDIV_INT_MODE))
            {
                pllConfig->eFractionMode = AM_HAL_SYSPLL_FMODE_INTEGER;
                pllConfig->ui8RefDiv = i;
                pllConfig->ui16FBDivInt = fbDiv;
                pllConfig->ui32FBDivFrac = 0;
                fbDivFound = true;
                break;
            }
        }
        else
        {
            if ((fbDiv >= AM_HAL_SYSPLL_MIN_FBDIV_FRAC_MODE) && (fbDiv <= AM_HAL_SYSPLL_MAX_FBDIV_FRAC_MODE))
            {
                pllConfig->eFractionMode = AM_HAL_SYSPLL_FMODE_FRACTION;
                pllConfig->ui8RefDiv = i;
                pllConfig->ui16FBDivInt = fbDiv;
                uint64_t rem = preDivFreq - ((32000000ULL) * fbDiv);
                uint64_t fracPart = ((rem << 24) + ((32000000ULL) / 2)) / (32000000ULL);
                pllConfig->ui32FBDivFrac = (uint32_t)fracPart;
                fbDivFound = true;
                break;
            }
        }
    }

    if (fbDivFound == true)
    {
        return AM_HAL_STATUS_SUCCESS;
    }
    else
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
}

int
sound_bridge_power_test(void)
{
    #if (ENUM_CLOCK_SOURCE == CLKSRC_PLL)
    // Start XTHS for use by PLL.
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
    am_util_delay_us(1500);

    void *pllHandle;
    am_hal_syspll_config_t pllConfig;
    if (AM_HAL_STATUS_SUCCESS != pll_config_calculate(&pllConfig, PLL_FREQ))
    {
        return AM_HAL_STATUS_FAIL;
    }

    am_hal_syspll_initialize(0, &pllHandle);
    am_hal_syspll_configure(pllHandle, &pllConfig);
    am_hal_syspll_enable(pllHandle);

    // Wait for PLL Lock.
    am_hal_syspll_lock_wait(pllHandle);
    #elif (ENUM_CLOCK_SOURCE == CLKSRC_HF2ADJ)
    // Start the 32MHz crystal rapidly used to adjust HFRC2.
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
    am_util_delay_us(1500);

    // Enable HFRC2 adjustment.
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HF2ADJ_ENABLE, false);
    am_util_delay_us(500);
    #endif

    //
    // Initialzie perephreals.
    //
    #if (ENUM_TEST_MODE & I2S_IN)
    i2s_init(ENUM_I2S_IN_MODULE, &i2sInHandle, &i2sInConfig);
    #endif

    #if (ENUM_TEST_MODE & PDM_IN)
    pdm_init(PDM0_MODULE, &pdm0Handle, &pdmConfig);
    #endif

    #if (ENUM_TEST_MODE & AUDADC_IN)
    audadc_init(AUDADC0_MODULE, &audadc0Handle);
    #endif

    #if (ENUM_TEST_MODE & I2S_OUT)
    i2s_init(I2S_OUT_MODULE, &i2sOutHandle, &i2sOutConfig);
    #endif

    //
    // Configure DMA.
    //
    #if (ENUM_TEST_MODE & I2S_IN)
    am_hal_i2s_dma_configure(i2sInHandle, &i2sInConfig, &i2sInDmaCfg);
    am_hal_i2s_dma_transfer_start(i2sInHandle, &i2sInConfig);
    #endif

    #if (ENUM_TEST_MODE & PDM_IN)
    am_hal_pdm_dma_start(pdm0Handle, &pdmDmaConfig);
    #endif

    #if (ENUM_TEST_MODE & AUDADC_IN)
    am_hal_audadc_configure_dma(audadc0Handle, &audadcDmaConfig);
    am_hal_audadc_dma_transfer_start(audadc0Handle);
    #endif

    #if (ENUM_TEST_MODE & I2S_OUT)
    am_hal_i2s_dma_configure(i2sOutHandle, &i2sOutConfig, &i2sOutDmaCfg);
    am_hal_i2s_dma_transfer_start(i2sOutHandle, &i2sOutConfig);
    #endif

    am_hal_interrupt_master_enable();

    while (1)
    {
        #if ENUM_TEST_MODE == AUDADC_TO_I2S
        if (audadcDmaCpl == true)
        {
            audadcDmaCpl = false;
            uint32_t* srcBuf = (uint32_t*)am_hal_audadc_dma_get_buffer(audadc0Handle);
            i2sOutSample* dstBuf = (i2sOutSample*)am_hal_i2s_dma_get_buffer(i2sOutHandle, AM_HAL_I2S_XFER_TX);
            am_hal_cachectrl_dcache_invalidate(&(am_hal_cachectrl_range_t){(uint32_t)srcBuf, AUDADC_DMA_BUF_SIZE}, false);

            for (uint32_t i = 0; i < (UINT_SYS_SAMPLE_RATES * UINT_DMA_FRAME_MS / 1000); i++)
            {
                dstBuf[UINT_I2S_OUT_CHANNELS * i] = AUDADC_SAMPLE_SHIFT(srcBuf[i] & 0x0000FFF0);
                #if (UINT_I2S_OUT_CHANNELS > 1)
                dstBuf[UINT_I2S_OUT_CHANNELS * i + 1] = dstBuf[UINT_I2S_OUT_CHANNELS * i];
                #endif
            }

            am_hal_cachectrl_dcache_clean(&(am_hal_cachectrl_range_t){(uint32_t)(&dstBuf[0]), I2S_OUT_DMA_BUF_SIZE});
        }
        #elif ENUM_TEST_MODE == PDM_TO_I2S
        if (pdmDmaCpl == true)
        {
            pdmDmaCpl = false;
            uint32_t* srcBuf = (uint32_t*)am_hal_pdm_dma_get_buffer(pdm0Handle);
            i2sOutSample* dstBuf = (i2sOutSample*)am_hal_i2s_dma_get_buffer(i2sOutHandle, AM_HAL_I2S_XFER_TX);
            am_hal_cachectrl_dcache_invalidate(&(am_hal_cachectrl_range_t){(uint32_t)srcBuf, PDM_DMA_BUF_SIZE}, false);

            for (uint32_t i = 0; i < (UINT_SYS_SAMPLE_RATES * UINT_DMA_FRAME_MS / 1000); i++)
            {
                dstBuf[UINT_I2S_OUT_CHANNELS * i] = PDM_SAMPLE_SHIFT(srcBuf[UINT_PDM_CHANNELS * i]);
                #if (UINT_I2S_OUT_CHANNELS > 1) && (UINT_PDM_CHANNELS == 2)
                dstBuf[UINT_I2S_OUT_CHANNELS * i + 1] = PDM_SAMPLE_SHIFT(srcBuf[UINT_PDM_CHANNELS * i + 1]);
                #endif
            }

            am_hal_cachectrl_dcache_clean(&(am_hal_cachectrl_range_t){(uint32_t)(&dstBuf[0]), I2S_OUT_DMA_BUF_SIZE});
        }
        #endif

        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}
