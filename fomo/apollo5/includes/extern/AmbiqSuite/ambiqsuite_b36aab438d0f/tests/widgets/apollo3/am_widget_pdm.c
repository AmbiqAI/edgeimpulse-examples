//*****************************************************************************
//
//! @file am_widget_pdm.c
//!
//! @brief
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#define ARM_MATH_CM4
#include <arm_math.h>

#include "am_mcu_apollo.h"
#include "am_widget_pdm.h"
#include "am_bsp.h"
#include "am_util.h"

// #define PRINT_PDM_DATA

//*****************************************************************************
//
// Global PDM handle
//
//*****************************************************************************
void *PDMHandle;

//*****************************************************************************
//
// Helper structure.
//
//*****************************************************************************
typedef struct
{
    uint32_t ui32Expected;
    uint32_t ui32Actual;
}
compare_values_t;

//*****************************************************************************
//
// Helper macros.
//
//*****************************************************************************
#define ARRAY_LEN(x)                                                          \
    (sizeof(x) / sizeof(x[0]))

//*****************************************************************************
//
// PDM ISR redirection.
//
// Different tests might require different ISR behavior. This ISR definition
// and function pointer approach allows us to change the ISR behavior on the
// fly.
//
//*****************************************************************************
static void (*pfnPDMWidgetISR)(void) = 0;

void
am_pdm0_isr(void)
{
    if (pfnPDMWidgetISR)
    {
        pfnPDMWidgetISR();
    }
}

//*****************************************************************************
//
// PDM config test.
//
//*****************************************************************************
uint32_t
am_widget_pdm_config_test(am_hal_pdm_config_t *psConfig, char *pErrStr)
{
    uint32_t ui32Status;

    //
    // Turn on the PDM.
    //
    am_hal_pdm_initialize(0, &PDMHandle);
    am_hal_pdm_power_control(PDMHandle, AM_HAL_PDM_POWER_ON, false);

    //
    // Set the configuration using the normal HAL operation.
    //
    am_hal_pdm_configure(PDMHandle, psConfig);

    //
    // Make a list of comparisons to do between the structure and the actual
    // registers.
    //
    compare_values_t psComparisons[] =
    {
        { PDM->PCFG_b.ADCHPD,     psConfig->bHighPassEnable },
        { PDM->PCFG_b.CYCLES,     psConfig->ui32GainChangeDelay },
        { PDM->PCFG_b.HPCUTOFF,   psConfig->ui32HighPassCutoff },
        { PDM->PCFG_b.LRSWAP,     psConfig->bLRSwap },
        { PDM->PCFG_b.MCLKDIV,    psConfig->eClkDivider },
        { PDM->PCFG_b.PGALEFT,    psConfig->eLeftGain },
        { PDM->PCFG_b.PGARIGHT,   psConfig->eRightGain },
        { PDM->PCFG_b.SINCRATE,   psConfig->ui32DecimationRate },
        { PDM->PCFG_b.SOFTMUTE,   psConfig->bSoftMute },
        { PDM->VCFG_b.BCLKINV,    psConfig->bInvertI2SBCLK },
        { PDM->VCFG_b.CHSET,      psConfig->ePCMChannels },
        { PDM->VCFG_b.DMICKDEL,   psConfig->bPDMSampleDelay },
        { PDM->VCFG_b.I2SEN,      psConfig->bI2SEnable },
        { PDM->VCFG_b.IOCLKEN,    PDM_VCFG_IOCLKEN_DIS },
        { PDM->VCFG_b.PCMPACK,    psConfig->bDataPacking },
        { PDM->VCFG_b.PDMCLKEN,   PDM_VCFG_PDMCLKEN_DIS },
        { PDM->VCFG_b.PDMCLKSEL,  psConfig->ePDMClkSpeed },
        { PDM->VCFG_b.SELAP,      psConfig->ePDMClkSource }
    };

    //
    // Now we'll loop through the comparisons, stopping if we find one that
    // doesn't come out correctly.
    //
    ui32Status = AM_HAL_STATUS_SUCCESS;

    for (uint32_t i; i < ARRAY_LEN(psComparisons); i++)
    {
        uint32_t ui32Expected = psComparisons[i].ui32Expected;
        uint32_t ui32Actual = psComparisons[i].ui32Actual;

        if (ui32Expected != ui32Actual)
        {
            ui32Status = AM_HAL_STATUS_FAIL;
            am_util_stdio_sprintf(pErrStr, "Config test failed comparison %d\n", i);
            break;
        }
    }

    //
    // Turn off and de-initialize the PDM.
    //
    am_hal_pdm_power_control(PDMHandle, AM_HAL_PDM_POWER_OFF, false);
    am_hal_pdm_deinitialize(&PDMHandle);

    return ui32Status;
}

//*****************************************************************************
//
// PDM DMA test.
//
//*****************************************************************************
#define FFT_SIZE          4096
static uint32_t ui32PDMDataBuffer[FFT_SIZE];
static float fSamples[FFT_SIZE];
static float fMagnitudes[FFT_SIZE];
volatile uint32_t ui32DMATestDone = 0;
volatile uint32_t ui32NumDMASamples = 0;

//
// Helper functions.
//
void am_widget_pdm_dma_test_isr(void);
void am_widget_pdm_dma_pin_config(void);
uint32_t am_widget_pdm_dma_freq_check(void);
uint32_t am_widget_pdm_sample_freq_get(am_hal_pdm_config_t *psConfig);

uint32_t
am_widget_pdm_dma_test(void *psTestConfig, char *pErrStr)
{
    am_widget_pdm_dma_test_config_t *psTest = psTestConfig;

    //
    // Set our ISR function.
    //
    pfnPDMWidgetISR = am_widget_pdm_dma_test_isr;
    ui32DMATestDone = 0;

    //
    // Set up the PDM.
    //
    am_hal_pdm_initialize(0, &PDMHandle);
    am_hal_pdm_power_control(PDMHandle, AM_HAL_PDM_POWER_ON, false);
    am_hal_pdm_configure(PDMHandle, &psTest->sConfig);
    am_hal_pdm_enable(PDMHandle);

    //
    // Enable PDM pins.
    //
    am_widget_pdm_dma_pin_config();

    //
    // Configure and enable PDM interrupts (set up to trigger on DMA
    // completion).
    //
    am_hal_pdm_interrupt_enable(PDMHandle, (AM_HAL_PDM_INT_DERR
                                            | AM_HAL_PDM_INT_DCMP
                                            | AM_HAL_PDM_INT_UNDFL
                                            | AM_HAL_PDM_INT_OVF));
    NVIC_EnableIRQ(PDM_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Configure DMA and target address.
    //
    am_hal_pdm_transfer_t sTransfer;
    sTransfer.ui32TargetAddr = (uint32_t ) ui32PDMDataBuffer;
    sTransfer.ui32TotalCount = psTest->ui32NumBytes;
    ui32NumDMASamples = psTest->ui32NumBytes/2;

    //
    // Start the data transfer.
    //
    am_hal_pdm_enable(PDMHandle);
    am_util_delay_ms(100);
    am_hal_pdm_fifo_flush(PDMHandle);
    am_hal_pdm_dma_start(PDMHandle, &sTransfer);

    //
    // FIXME: Add timeout here?
    //
    while (ui32DMATestDone == false);

    //
    // Turn off and de-initialize the PDM.
    //
    NVIC_DisableIRQ(PDM_IRQn);
    am_hal_pdm_power_control(PDMHandle, AM_HAL_PDM_POWER_OFF, false);
    am_hal_pdm_deinitialize(&PDMHandle);
    pfnPDMWidgetISR = 0;

    //
    // Check to see if the DMA transaction looks good. To do this, we'll run an
    // FFT over the bits we have to check for the frequency component with the
    // highest magnitude. If it looks like our input signal, we have a good
    // transfer.
    //
    uint32_t ui32BiggestBin = am_widget_pdm_dma_freq_check();
    uint32_t ui32SampleFreq = am_widget_pdm_sample_freq_get(&psTest->sConfig);
    float fBinSize = (float) ui32SampleFreq / (float) ui32NumDMASamples;
    float fLoudestFreq = fBinSize * ui32BiggestBin;

#ifdef PRINT_PDM_DATA
    am_util_stdio_printf("Bin Size: %f\n", fBinSize);
    am_util_stdio_printf("Biggest frequency bin: %f\n", fLoudestFreq);
#endif

    //am_util_stdio_printf("FFT Length:             %12d\n", ui32NumDMASamples);
    //am_util_stdio_printf("FFT Resolution:         %12d\n", ui32SampleFreq / ui32NumDMASamples);

    return (uint32_t)(fLoudestFreq);
}

void
am_widget_pdm_dma_test_isr(void)
{
    uint32_t ui32Status;

    //
    // Read the interrupt status.
    //
    am_hal_pdm_interrupt_status_get(PDMHandle, &ui32Status, true);
    am_hal_pdm_interrupt_clear(PDMHandle, ui32Status);

    //
    // If this was a DMA complete interrupt, disable the PDM, and send a flag
    // back down to the base-level test.
    //
    if (ui32Status & AM_HAL_PDM_INT_DCMP)
    {
        am_hal_pdm_disable(PDMHandle);

#ifdef PRINT_PDM_DATA
        //
        // We can optionally print the raw PCM data to the SWO port here.
        //
        for (uint32_t i = 0; i < ui32NumDMASamples; i++)
        {
            int16_t *pi16PDMSamples = (int16_t *) ui32PDMDataBuffer;
            am_util_stdio_printf("%d\n", pi16PDMSamples[i]);
        }

        am_util_stdio_printf("END\n");
#endif
        ui32DMATestDone = 1;
    }
}

void
am_widget_pdm_dma_pin_config(void)
{
    am_hal_gpio_pincfg_t sCfg = {0,0,0,0,0,0,0,0,0,0,0,0};

    uint32_t ui32ErrorStatus;

    sCfg.uFuncSel = AM_HAL_PIN_11_PDMDATA;
    ui32ErrorStatus = am_hal_gpio_pinconfig(11, sCfg);
    while(ui32ErrorStatus);

    sCfg.uFuncSel = AM_HAL_PIN_12_PDMCLK;
    ui32ErrorStatus = am_hal_gpio_pinconfig(12, sCfg);
    while(ui32ErrorStatus);

    sCfg.uFuncSel = AM_HAL_PIN_13_I2SBCLK;
    ui32ErrorStatus = am_hal_gpio_pinconfig(13, sCfg);
    while(ui32ErrorStatus);

    sCfg.uFuncSel = AM_HAL_PIN_23_I2SWCLK;
    ui32ErrorStatus = am_hal_gpio_pinconfig(23, sCfg);
    while(ui32ErrorStatus);

    sCfg.uFuncSel = AM_HAL_PIN_30_I2SDAT;
    ui32ErrorStatus = am_hal_gpio_pinconfig(30, sCfg);
    while(ui32ErrorStatus);

    am_hal_gpio_pinconfig(14, g_AM_HAL_GPIO_OUTPUT);
    ui32ErrorStatus = am_hal_gpio_state_write(14, AM_HAL_GPIO_OUTPUT_CLEAR);
    while(ui32ErrorStatus);
}

uint32_t
am_widget_pdm_dma_freq_check(void)
{
    float fMaxValue;
    uint32_t ui32MaxIndex;
    int16_t *pui16PDMData = (int16_t *) ui32PDMDataBuffer;

    //
    // Convert the PDM samples to floats, and arrange them in the format
    // required by the FFT function.
    //
    for (uint32_t i = 0; i < ui32NumDMASamples; i++)
    {
        fSamples[2*i] = pui16PDMData[i]/1.0;
        fSamples[2*i+1] = 0.0;
    }

    //
    // Perform the FFT.
    //
    arm_cfft_radix4_instance_f32 S;
    arm_cfft_radix4_init_f32(&S, ui32NumDMASamples, 0, 1);
    arm_cfft_radix4_f32(&S, fSamples);
    arm_cmplx_mag_f32(fSamples, fMagnitudes, ui32NumDMASamples);

#ifdef PRINT_PDM_DATA
    //
    // Print the resulting array of magnitudes over SWO.
    //
    for (uint32_t i = 0; i < ui32NumDMASamples/2; i++)
    {
        am_util_stdio_printf("%f\n", fMagnitudes[i]);
    }

    am_util_stdio_printf("END\n");
#endif

    //
    // Find the frequency bin with the largest magnitude.
    //
    arm_max_f32(fMagnitudes, ui32NumDMASamples/2, &fMaxValue, &ui32MaxIndex);

    return ui32MaxIndex;
}

uint32_t
am_widget_pdm_sample_freq_get(am_hal_pdm_config_t *psConfig)
{
    uint32_t ui32PDMClk;
    uint32_t ui32MClkDiv;
    uint32_t ui32SampleFreq;

    //
    // If we're not supplying our own clock, there's no way to tell what the
    // frequency is.
    //
    if (psConfig->ePDMClkSource != AM_HAL_PDM_INTERNAL_CLK)
    {
        return 0;
    }

    //
    // Read the config structure to figure out what our internal clock is set
    // to.
    //
    switch (psConfig->eClkDivider)
    {
        case AM_HAL_PDM_MCLKDIV_4: ui32MClkDiv = 4; break;
        case AM_HAL_PDM_MCLKDIV_3: ui32MClkDiv = 3; break;
        case AM_HAL_PDM_MCLKDIV_2: ui32MClkDiv = 2; break;
        case AM_HAL_PDM_MCLKDIV_1: ui32MClkDiv = 1; break;

        default:
            return 0;
    }

    switch (psConfig->ePDMClkSpeed)
    {
        case AM_HAL_PDM_CLK_12MHZ:  ui32PDMClk = 12000000; break;
        case AM_HAL_PDM_CLK_6MHZ:   ui32PDMClk =  6000000; break;
        case AM_HAL_PDM_CLK_3MHZ:   ui32PDMClk =  3000000; break;
        case AM_HAL_PDM_CLK_1_5MHZ: ui32PDMClk =  1500000; break;
        case AM_HAL_PDM_CLK_750KHZ: ui32PDMClk =   750000; break;
        case AM_HAL_PDM_CLK_375KHZ: ui32PDMClk =   375000; break;
        case AM_HAL_PDM_CLK_187KHZ: ui32PDMClk =   187500; break;

        default:
            return 0;
    }

    ui32SampleFreq = (ui32PDMClk /
                      (ui32MClkDiv * 2 * psConfig->ui32DecimationRate));

    //am_util_stdio_printf("Settings:\n");
    //am_util_stdio_printf("PDM Clock (Hz):         %12d\n", ui32PDMClk);
    //am_util_stdio_printf("Decimation Rate:        %12d\n", psConfig->ui32DecimationRate);
    //am_util_stdio_printf("Effective Sample Freq.: %12d\n", ui32SampleFreq);

    return ui32SampleFreq;
}
