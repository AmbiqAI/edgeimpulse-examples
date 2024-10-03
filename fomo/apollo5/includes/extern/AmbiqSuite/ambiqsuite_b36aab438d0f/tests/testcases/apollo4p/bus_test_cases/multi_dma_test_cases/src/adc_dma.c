//*****************************************************************************
//
//! @file adc_dma.c
//!
//! @brief ADC&AUDADC dma opeariton.
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
#define AUDADC_SAMPLE_BUF_SIZE      (4 * 1024UL)
#define AUDADC_DATA_BUFFER_SIZE     (2 * AUDADC_SAMPLE_BUF_SIZE)

#define ADC_WAIT_TIMEOUT_MS         (1000UL)
#define AUDADC_WAIT_TIMEOUT_MS      (1000UL)
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


//
// AUDADC Sample buffer.
//
uint32_t g_ui32AUDADCSampleBuffer[AUDADC_SAMPLE_BUF_SIZE];
int16_t g_in16AudioDataBuffer[AUDADC_DATA_BUFFER_SIZE];

//
// AUDADC Global variables.
//
static void                    *g_AUDADCHandle;
volatile bool                   g_bAUDADCDMAComplete;
volatile bool                   g_bAUDADCDMAError;

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
        am_util_stdio_printf("AUDADC DMA error!\n");
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
#if 1 // As of Apr 2022, only 24MHz clock is available for GP ADC on Apollo4
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

//*****************************************************************************
//
// Configure the AUDADC.
//
//*****************************************************************************
void
audadc_config_dma(void)
{
    am_hal_audadc_dma_config_t       AUDADCDMAConfig;

    //
    // Configure the AUDADC to use DMA for the sample transfer.
    //
    AUDADCDMAConfig.bDynamicPriority = true;
    AUDADCDMAConfig.ePriority = AM_HAL_AUDADC_PRIOR_SERVICE_IMMED;
    AUDADCDMAConfig.bDMAEnable = true;
    AUDADCDMAConfig.ui32SampleCount = AUDADC_SAMPLE_BUF_SIZE;
    AUDADCDMAConfig.ui32TargetAddress = (uint32_t)g_ui32AUDADCSampleBuffer;
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_dma(g_AUDADCHandle, &AUDADCDMAConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC DMA failed.\n");
    }

    //
    // Reset the AUDADC DMA flags.
    //
    g_bAUDADCDMAComplete = false;
    g_bAUDADCDMAError = false;
}

//*****************************************************************************
//
// Configure the AUDADC SLOT.
//
//*****************************************************************************
void
audadc_slot_config(void)
{
    am_hal_audadc_slot_config_t      AUDADCSlotConfig;

    //
    // Set up an AUDADC slot
    //
    AUDADCSlotConfig.eMeasToAvg      = AM_HAL_AUDADC_SLOT_AVG_1;
    AUDADCSlotConfig.ePrecisionMode  = AM_HAL_AUDADC_SLOT_12BIT;
    AUDADCSlotConfig.ui32TrkCyc      = 40;  // ToDo: Should this be AM_HAL_AUDADC_MIN_TRKCYC?
    AUDADCSlotConfig.eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE0;
    AUDADCSlotConfig.bWindowCompare  = true;
    AUDADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_slot(g_AUDADCHandle, 0, &AUDADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC Slot 0 failed.\n");
    }

    AUDADCSlotConfig.eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE1;
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_slot(g_AUDADCHandle, 1, &AUDADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC Slot 1 failed.\n");
    }

}

//*****************************************************************************
//
// Configure the AUDADC.
//
//*****************************************************************************
void
audadc_config(void)
{
    //
    // Set up the AUDADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    //
    am_hal_audadc_config_t           AUDADCConfig =
    {
        .eClock             = AM_HAL_AUDADC_CLKSEL_HFRC2_48MHz, //AM_HAL_AUDADC_CLKSEL_XTHS_24MHz;
        .ePolarity          = AM_HAL_AUDADC_TRIGPOL_RISING,
        .eTrigger           = AM_HAL_AUDADC_TRIGSEL_SOFTWARE,
        .eClockMode         = AM_HAL_AUDADC_CLKMODE_LOW_LATENCY,
        .ePowerMode         = AM_HAL_AUDADC_LPMODE1,
        .eRepeat            = AM_HAL_AUDADC_REPEATING_SCAN,
        .eRepeatTrigger     = AM_HAL_AUDADC_RPTTRIGSEL_INT,     //AM_HAL_AUDADC_RPTTRIGSEL_INT;
        .eSampMode          = AM_HAL_AUDADC_SAMPMODE_MED,       //AM_HAL_AUDADC_SAMPMODE_LP,
    };

    //
    // Set up internal repeat trigger timer
    //
    am_hal_audadc_irtt_config_t      AUDADCIrttConfig =
    {
        .bIrttEnable        = true,
        .eClkDiv            =  AM_HAL_AUDADC_RPTT_CLK_DIV32,
        .ui32IrttCountMax   = 61,
    };

    //
    // Initialize the AUDADC and get the handle.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_audadc_initialize(0, &g_AUDADCHandle) )
    {
        am_util_stdio_printf("Error - reservation of the AUDADC instance failed.\n");
    }
    //
    // Power on the AUDADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_power_control(g_AUDADCHandle,
                                                          AM_HAL_SYSCTRL_WAKE,
                                                          false) )
    {
        am_util_stdio_printf("Error - AUDADC power on failed.\n");
    }

    //Enable hfrc2.
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFRC2_START, false);
    am_util_delay_us(200);

    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure(g_AUDADCHandle, &AUDADCConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC failed.\n");
    }

    //
    // Set up internal repeat trigger timer
    //
    am_hal_audadc_configure_irtt(g_AUDADCHandle, &AUDADCIrttConfig);

    //
    // Enable the AUDADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_enable(g_AUDADCHandle))
    {
        am_util_stdio_printf("Error - enabling AUDADC failed.\n");
    }

    //
    // Enable internal repeat trigger timer
    //
    am_hal_audadc_irtt_enable(g_AUDADCHandle);


    //
    // Configure the AUDADC to use DMA for the sample transfer.
    //
    audadc_config_dma();

    //
    // For this example, the samples will be coming in slowly. This means we
    // can afford to wake up for every conversion.
    //
    am_hal_audadc_interrupt_enable(g_AUDADCHandle, AM_HAL_AUDADC_INT_FIFOOVR1 | AM_HAL_AUDADC_INT_FIFOOVR2 | AM_HAL_AUDADC_INT_DERR | AM_HAL_AUDADC_INT_DCMP ); //| AM_HAL_AUDADC_INT_CNVCMP | AM_HAL_AUDADC_INT_SCNCMP);
}

//*****************************************************************************
//
// Interrupt handler for the AUDADC.
//
//*****************************************************************************
void
am_audadc0_isr(void)
{
    uint32_t ui32IntMask;
    //
    // Read the interrupt status.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_interrupt_status(g_AUDADCHandle, &ui32IntMask, false))
    {
        am_util_stdio_printf("Error reading AUDADC interrupt status\n");
    }

    //
    // Clear the AUDADC interrupt.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_interrupt_clear(g_AUDADCHandle, ui32IntMask))
    {
        am_util_stdio_printf("Error clearing AUDADC interrupt status\n");
    }


    //
    // If we got a DMA complete, set the flag.
    //
    if (ui32IntMask & AM_HAL_AUDADC_INT_FIFOOVR1)
    {
        if ( AUDADCn(0)->DMASTAT_b.DMACPL )
        {
            g_bAUDADCDMAComplete = true;
        }
    }

    //
    // If we got a DMA error, set the flag.
    //
    if ( ui32IntMask & AM_HAL_AUDADC_INT_DERR )
    {
        g_bAUDADCDMAError = true;
        am_util_stdio_printf("AUDADC DMA error!\n");
    }

#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_clear(DEBUG_PIN_12);
#endif
}


//*****************************************************************************
//
// Init adc&audadc.
//
//*****************************************************************************
bool
audadc_init(void)
{
    //
    // power up PGA
    //
    am_hal_audadc_refgen_powerup();

    am_hal_audadc_pga_powerup(0);
    am_hal_audadc_pga_powerup(1);
    am_hal_audadc_pga_powerup(2);
    am_hal_audadc_pga_powerup(3);

    am_hal_audadc_gain_set(0, 2*6);
    am_hal_audadc_gain_set(1, 2*6);
    am_hal_audadc_gain_set(2, 2*6);
    am_hal_audadc_gain_set(3, 2*6);

    //
    //  turn on mic bias
    //
    am_hal_audadc_micbias_powerup(24);

    //
    // Configure the AUDADC
    //
    audadc_config();
    audadc_slot_config();

    //
    // Enable interrupts.
    //
    NVIC_SetPriority(AUDADC0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(AUDADC0_IRQn);

    //
    // Trigger the AUDADC sampling for the first time manually.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_sw_trigger(g_AUDADCHandle))
    {
        am_util_stdio_printf("Error - triggering the AUDADC failed.\n");
        return false;
    }

    return true;
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
// Start AUDADC.
//
//*****************************************************************************
bool
audadc_start(void)
{
//    uint32_t  ui32SampleCount;
//
//    //DMA read.
//    ui32SampleCount = AUDADC_SAMPLE_BUF_SIZE;
//    for (int i = 0; i < ui32SampleCount; i++)
//    {
//       g_in16AudioDataBuffer[2 * i]     = (int16_t)(g_ui32AUDADCSampleBuffer[i] & 0x0000FFF0);          // Low-gain PGA sample data.
//       g_in16AudioDataBuffer[2 * i + 1] = (int16_t)((g_ui32AUDADCSampleBuffer[i] >> 16) & 0x0000FFF0);  // High-gain PGA sample data.
//    }

    //
    // Reset the DMA completion and error flags.
    //
    g_bAUDADCDMAComplete = false;
    g_bAUDADCDMAError = false;

    //
    // Re-configure the AUDADC DMA.
    //
    audadc_config_dma();

    //
    // Clear the AUDADC interrupts.
    //
    am_hal_audadc_interrupt_clear(g_AUDADCHandle, 0xFFFFFFFF);

#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_set(DEBUG_PIN_12);
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

//*****************************************************************************
//
// Wait AUDADC complete.
//
//*****************************************************************************
bool
audadc_wait(void)
{
    uint32_t count = 0;

    while((!g_bAUDADCDMAComplete) && (count < AUDADC_WAIT_TIMEOUT_MS))
    {
        am_util_delay_ms(1);
        count ++;
    }

    if(count >= AUDADC_WAIT_TIMEOUT_MS)
    {
        return false;
    }
    else
    {
        return true;
    }
}


