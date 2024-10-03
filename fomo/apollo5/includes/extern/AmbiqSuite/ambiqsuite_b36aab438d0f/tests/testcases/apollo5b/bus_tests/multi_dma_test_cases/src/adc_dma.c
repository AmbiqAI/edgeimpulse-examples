//*****************************************************************************
//
//! @file adc_dma.c
//!
//! @brief ADC dma opeariton.
//!
//! AM_DEBUG_PRINTF
//! If enabled, debug messages will be sent over ITM.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "multi_dma.h"

//*****************************************************************************
//
// Test defines
//
//*****************************************************************************
#define ADC_SAMPLE_BUF_SIZE         128UL

#define ADC_WAIT_TIMEOUT_MS         (1000UL)
//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//
// ADC Sample buffer.
//
uint32_t g_ui32ADCSampleBuffer[ADC_SAMPLE_BUF_SIZE];
am_hal_adc_sample_t SampleBuffer[ADC_SAMPLE_BUF_SIZE];

//
// ADC Global variables.
//
static void                    *g_ADCHandle;
volatile bool                   g_bADCDMAComplete;
volatile bool                   g_bADCDMAError;

//
// Define the ADC SE0 pin to be used.
//
const am_hal_gpio_pincfg_t g_AM_PIN_19_ADCSE0 =
{
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_19_ADCSE0,
};

//*****************************************************************************
//
// Interrupt handler for the ADC.
//
//*****************************************************************************
void
am_adc_isr(void)
{
    uint32_t ui32IntMask;
    //
    // Read the interrupt status.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_status(g_ADCHandle, &ui32IntMask, false))
    {
        am_util_stdio_printf("Error reading ADC interrupt status\n");
    }

    //
    // Clear the ADC interrupt.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_clear(g_ADCHandle, ui32IntMask))
    {
        am_util_stdio_printf("Error clearing ADC interrupt status\n");
    }


    //
    // If we got a DMA complete, set the flag.
    //
    if (ui32IntMask & AM_HAL_ADC_INT_FIFOOVR1)
//        (ui32IntMask & AM_HAL_ADC_INT_CNVCMP)   || (ui32IntMask & AM_HAL_ADC_INT_DCMP) ||
//        (ui32IntMask & AM_HAL_ADC_INT_SCNCMP))
    {
        if ( ADCn(0)->DMASTAT_b.DMACPL )
        {
            g_bADCDMAComplete = true;
        }
    }

    //
    // If we got a DMA error, set the flag.
    //
    if ( ui32IntMask & AM_HAL_ADC_INT_DERR )
    {
        g_bADCDMAError = true;
        am_util_stdio_printf("ADC DMA error!\n");
    }

#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_clear(DEBUG_PIN_11);
#endif
}


//*****************************************************************************
//
// Configure the ADC.
//
//*****************************************************************************
void
adc_config_dma(void)
{
    am_hal_adc_dma_config_t       ADCDMAConfig;

    //
    // Configure the ADC to use DMA for the sample transfer.
    //
    ADCDMAConfig.bDynamicPriority = true;
    ADCDMAConfig.ePriority = AM_HAL_ADC_PRIOR_SERVICE_IMMED;
    ADCDMAConfig.bDMAEnable = true;
    ADCDMAConfig.ui32SampleCount = ADC_SAMPLE_BUF_SIZE;
    ADCDMAConfig.ui32TargetAddress = (uint32_t)g_ui32ADCSampleBuffer;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_dma(g_ADCHandle, &ADCDMAConfig))
    {
        am_util_stdio_printf("Error - configuring ADC DMA failed.\n");
    }

    //
    // Reset the ADC DMA flags.
    //
    g_bADCDMAComplete = false;
    g_bADCDMAError = false;
}

//*****************************************************************************
//
// Configure the ADC.
//
//*****************************************************************************
void
adc_config(void)
{
    am_hal_adc_config_t           ADCConfig;
    am_hal_adc_slot_config_t      ADCSlotConfig;

    //
    // Initialize the ADC and get the handle.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_adc_initialize(0, &g_ADCHandle) )
    {
        am_util_stdio_printf("Error - reservation of the ADC instance failed.\n");
    }
    //
    // Power on the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
                                                          AM_HAL_SYSCTRL_WAKE,
                                                          false) )
    {
        am_util_stdio_printf("Error - ADC power on failed.\n");
    }

    //
    // Set up internal repeat trigger timer
    //
    am_hal_adc_irtt_config_t      ADCIrttConfig =
    {
        .bIrttEnable        = true,
        .eClkDiv            = AM_HAL_ADC_RPTT_CLK_DIV2, // 48MHz
        .ui32IrttCountMax   = 20,   // 48MHz / 20 = 2.4MHz
    };

    am_hal_adc_configure_irtt(g_ADCHandle, &ADCIrttConfig);

    //
    // Set up the ADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    //
#if 1 // As of Apr 2022, only 24MHz clock is available for GP ADC on Apollo5
    ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC_24MHZ;
#else
    ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC;
#endif
    ADCConfig.ePolarity          = AM_HAL_ADC_TRIGPOL_RISING;
    ADCConfig.eTrigger           = AM_HAL_ADC_TRIGSEL_SOFTWARE;
    ADCConfig.eClockMode         = AM_HAL_ADC_CLKMODE_LOW_LATENCY;
    ADCConfig.ePowerMode         = AM_HAL_ADC_LPMODE0;
    ADCConfig.eRepeat            = AM_HAL_ADC_REPEATING_SCAN;

    ADCConfig.eRepeatTrigger     = AM_HAL_ADC_RPTTRIGSEL_INT;

    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure(g_ADCHandle, &ADCConfig))
    {
        am_util_stdio_printf("Error - configuring ADC failed.\n");
    }

    //
    // Set up an ADC slot
    //
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_128;
    ADCSlotConfig.ui32TrkCyc      = AM_HAL_ADC_MIN_TRKCYC;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_12BIT;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE0;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 0, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 0 failed.\n");
    }

    //
    // Configure the ADC to use DMA for the sample transfer.
    //
    adc_config_dma();

    //
    // For this example, the samples will be coming in slowly. This means we
    // can afford to wake up for every conversion.
    //
    am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_FIFOOVR1 | AM_HAL_ADC_INT_DERR | AM_HAL_ADC_INT_DCMP); // AM_HAL_ADC_INT_CNVCMP | AM_HAL_ADC_INT_SCNCMP |

    //
    // Enable the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_enable(g_ADCHandle))
    {
        am_util_stdio_printf("Error - enabling ADC failed.\n");
    }

    //
    // Enable internal repeat trigger timer
    //
    am_hal_adc_irtt_enable(g_ADCHandle);
}

bool
adc_init(void)
{
    //
    // Set a pin to act as our ADC input
    //
    am_hal_gpio_pinconfig(19, g_AM_PIN_19_ADCSE0);

    //
    // Configure the ADC
    //
    adc_config();

    //
    // Enable interrupts.
    //
    NVIC_SetPriority(ADC_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(ADC_IRQn);

    //
    // Trigger the ADC sampling for the first time manually.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_sw_trigger(g_ADCHandle))
    {
        am_util_stdio_printf("Error - triggering the ADC failed.\n");
        return false;
    }

    return true;
}

//*****************************************************************************
//
// Start ADC.
//
//*****************************************************************************
bool
adc_start(void)
{
//    uint32_t        ui32SampleCount;
//
//    ui32SampleCount = ADC_SAMPLE_BUF_SIZE;
//    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_samples_read(g_ADCHandle, false,
//                                                         g_ui32ADCSampleBuffer,
//                                                         &ui32SampleCount,
//                                                         SampleBuffer))
//    {
//        am_util_stdio_printf("Error - failed to process samples.\n");
//        return false;
//    }

    //
    // Reset the DMA completion and error flags.
    //
    g_bADCDMAComplete = false;
    g_bADCDMAError = false;
    //
    // Re-configure the ADC DMA.
    //
    adc_config_dma();
    //
    // Clear the ADC interrupts.
    //
    am_hal_adc_interrupt_clear(g_ADCHandle, 0xFFFFFFFF);

#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_set(DEBUG_PIN_11);
#endif

    return true;
}

//*****************************************************************************
//
// Wait ADC complete.
//
//*****************************************************************************
bool
adc_wait(void)
{
    uint32_t count = 0;

    while((!g_bADCDMAComplete) && (count < ADC_WAIT_TIMEOUT_MS))
    {
        am_util_delay_ms(1);
        count ++;
    }

    if(count >= ADC_WAIT_TIMEOUT_MS)
    {
        return false;
    }
    else
    {
        return true;
    }
}

