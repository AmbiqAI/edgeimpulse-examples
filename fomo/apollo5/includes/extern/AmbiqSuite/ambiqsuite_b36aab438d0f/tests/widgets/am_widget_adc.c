//*****************************************************************************
//
//! @file am_widget_adc.c
//!
//! @brief This widget allows test cases to exercise the Apollo2 ADC using an
//! adafruit MCP4725 12-bit DAC, IOM #0, and CTIMERS A0 and A3.
//!
//! This program uses the CTIMER A0 and IOM #0 in I2C mode to create digital
//! samples on the MCP4725 DAC.  The DAC is driven at 1024Hz.  The CTIMER ISR
//! callback calculates either Sine, Square, or Triangle waveforms of the proper
//! 12-bit amplitude.  The program also uses the CTIMER A3 to trigger the ADC to
//! sample at 256Hz.  There are three required functions: setup, cleanup, and
//! test.  There are also two analysis routines which use an FFT (CMSIS-DSP) to
//! analyze the sine frequency and an edge detector to analyze square wave
//! samples.
//!
//! @verbatim
//! The Pin configuration is as follows:
//!     GPIO[5] - IOM #0 SCL to pin 3 (SCL) of the MCP4725
//!     GPIO[6] - IOM #0 SDA to pin 4 (SDA) of the MCP4725
//!     GPIO[16]- ADC EXT Input #0 (SE0) from pin 6 (VOUT) of the MCP4725
//!     GPIO[28]- ADC ISR Timing GPIO
//!     MCP4725 pin 1 (VDD) to any VDD EXP
//!     MCP4725 pin 2 (GND) to any GND
//! @endverbatim
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
#include "am_widget_adc.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define BITSCALE        (0x3FF)
#define BITOFFSET       (0x400)
#define TIMER_FREQ      (1024)
#define EDGE_SCALE      (12288)

//*****************************************************************************
//
// Memory allocation.
//
//*****************************************************************************
//
// ADC test configuration (defines the widget state).
//
am_widget_adc_config_t          g_ADCTestConfig;

//
// ADC sample counter.
//
volatile uint32_t               g_ADCSampleIndex;

//
// Waveform generation variables.
//
uint32_t                        g_ui32WaveformIndex;
uint32_t                        g_ui32WaveformPeriod;

//
// Window Comparator counters.
//
uint32_t                        g_ADCWCInclusionCount;
uint32_t                        g_ADCWCExclusionCount;

//
// IOM buffer and queue.
//
am_hal_iom_buffer(16)           g_sRamBuffer;
am_hal_iom_queue_entry_t        g_psQueueMemory[32];

//
// Variables for FFT calculations by CMSIS-DSP library.
//
arm_cfft_radix4_instance_f32    S;
arm_status                      status;
float                           fSamples[SAMPLES];
float                           fMagnitudes[FFT_SIZE];


//*****************************************************************************
//
// Send DAC Command
//
//*****************************************************************************
void
sendDACCommand(uint32_t ui32Address, uint16_t ui16Value)
{
  //
  // Scale the value into a 2-byte 12-bit format.
  //
  g_sRamBuffer.bytes[0] = (ui16Value & 0x0F00) >> 8;
  g_sRamBuffer.bytes[1] = (ui16Value & 0xFF);

  //
  // Send the value to the MCP4725 via the IOM #0
  //
  am_hal_iom_i2c_write(0, ui32Address, g_sRamBuffer.words, 2, AM_HAL_IOM_RAW);

}

//*****************************************************************************
//
// CTimer A0 Waveform Generation Handler
//
//*****************************************************************************
void
am_waveform_handler(void)
{
  uint32_t      ui32Value;

  //
  // Clear TimerA0 Interrupt (write to clear).
  //
  am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);

  //
  // Compute the next value based on the phase and waveform type.
  //
  switch(g_ADCTestConfig.eWaveform)
  {
  case   AM_WIDGET_WAVEFORM_SINE:
    ui32Value = (uint32_t)(BITOFFSET + BITSCALE * sin((double)(g_ui32WaveformIndex*2*PI/TIMER_FREQ)));
    break;

  case AM_WIDGET_WAVEFORM_TRIANGLE:
    ui32Value = (g_ui32WaveformIndex < TIMER_FREQ/2) ? (g_ui32WaveformIndex) : (TIMER_FREQ - g_ui32WaveformIndex);
    ui32Value *= (BITOFFSET + BITSCALE);
    ui32Value /= TIMER_FREQ/2;
    break;

  case AM_WIDGET_WAVEFORM_SQUARE:
    ui32Value = (g_ui32WaveformIndex < TIMER_FREQ/2) ? (0) : (BITOFFSET + BITSCALE);
    break;
  }

  //
  // Update the waveform index (phase).
  //
  g_ui32WaveformIndex = (g_ui32WaveformIndex + g_ADCTestConfig.ui32Frequency) % TIMER_FREQ;

  //
  // Send the digital value via I2C to the 12-bit DAC.
  //
  sendDACCommand(g_ADCTestConfig.ui32I2CAddress,ui32Value);

}

//*****************************************************************************
//
// Interrupt handler for CTimer
//
//*****************************************************************************
void
am_ctimer_isr(void)
{

  uint32_t ui32Status;

  ui32Status = am_hal_ctimer_int_status_get(true);

  am_hal_ctimer_int_service(ui32Status);

}
//*****************************************************************************
//
// Interrupt handler for IOM0
//
//*****************************************************************************
void
am_iomaster0_isr(void)
{
    uint32_t ui32IntStatus;

    //
    // Read and clear the interrupt status.
    //
    ui32IntStatus = am_hal_iom_int_status_get(0, false);
    am_hal_iom_int_clear(0, ui32IntStatus);

    //
    // Service FIFO interrupts as necessary, and call IOM callbacks as
    // transfers are completed.
    //
    am_hal_iom_int_service(0, ui32IntStatus);
}

//*****************************************************************************
//
// Interrupt handler for the ADC.
//
//*****************************************************************************
void
am_adc_isr(void)
{
  uint32_t ui32Status;
  uint32_t ui32PeekValue, ui32PopValue;

  //
  // Read the interrupt status.
  //
  ui32Status = am_hal_adc_int_status_get(true);

  //
  // If we got a conversion completion interrupt (which should be our only
  // ADC interrupt), go ahead and read the data.
  //
  if (ui32Status & AM_HAL_ADC_INT_CNVCMP)
  {
    //
    // Set the GPIO to record ADC ISR timing.
    //
    am_hal_gpio_out_bit_set(28);
    //
    // Peek into the FIFO to record the value.
    //
    ui32PeekValue = am_hal_adc_fifo_peek();
    //
    // Read the value from the FIFO.
    //
    ui32PopValue = am_hal_adc_fifo_pop();

    //
    // Check to make sure Peek and Pop API calls result in the same value always.
    //
    am_hal_debug_assert_msg(ui32PeekValue == ui32PopValue,"ADC Test Failed.  Peek and Pop Values do not match!");

    //
    // Store just the sample in the sample buffer.
    //
    g_ADCTestConfig.pSampleBuf[g_ADCSampleIndex++] = AM_HAL_ADC_FIFO_SAMPLE(ui32PopValue);

    //
    // Clear the GPIO to record the ADC ISR timing.
    //
    am_hal_gpio_out_bit_clear(28);
  }

  //
  // Check if we got the FIFO 100% full interrupt.  This should not happen.
  //
  am_hal_debug_assert_msg(0 == (ui32Status & AM_HAL_ADC_INT_FIFOOVR2),"ADC Test Failed.  Received a FIFO 100% full interrupt!");

  //
  // If we got a FIFO 75% full, go ahead and read the data.
  //
  if (ui32Status & AM_HAL_ADC_INT_FIFOOVR1)
  {
    do
    {
      //
      // Peek into the FIFO to record the value.
      //
      ui32PeekValue = am_hal_adc_fifo_peek();

      //
      // Read the value from the FIFO.
      //
      ui32PopValue = am_hal_adc_fifo_pop();

      //
      // Check to make sure Peek and Pop API calls result in the same value always.
      //
      am_hal_debug_assert_msg(ui32PeekValue == ui32PopValue,"ADC Test Failed.  Peek and Pop Values do not match!");

      //
      // Store just the sample in the sample buffer.
      //
      g_ADCTestConfig.pSampleBuf[g_ADCSampleIndex++] = AM_HAL_ADC_FIFO_SAMPLE(ui32PopValue);

    } while (AM_HAL_ADC_FIFO_COUNT(ui32PopValue) > 0);
  }

  //
  // If we got an Window Comparator Inclusion interrupt, count it.
  //
  if (ui32Status & AM_HAL_ADC_INT_WCINC)
  {
    g_ADCWCInclusionCount++;
  }

  //
  // If we got an Window Comparator Exclusion interrupt, count it.
  //
  if (ui32Status & AM_HAL_ADC_INT_WCEXC)
  {
    g_ADCWCExclusionCount++;
  }

  //
  // Clear the ADC interrupt.
  //
  am_hal_adc_int_clear(ui32Status);

}

//*****************************************************************************
//
// Init function for Timer A0.
//
//*****************************************************************************
void
timerA0_init(void)
{
  am_hal_ctimer_config_t sTimer0Config;

  // Don't link timers.
  sTimer0Config.ui32Link = 0;

  // Set up Timer0A.
  sTimer0Config.ui32TimerAConfig =
    (AM_HAL_CTIMER_FN_REPEAT |
     AM_HAL_CTIMER_INT_ENABLE    |
       AM_HAL_CTIMER_XT_32_768KHZ);

  // No configuration for Timer0B.
  sTimer0Config.ui32TimerBConfig = 0;


  //
  // Calculate and store the period.
  //
  g_ui32WaveformPeriod = TIMER_FREQ/g_ADCTestConfig.ui32Frequency;

  //
  // Reset the waveform index.
  //
  g_ui32WaveformIndex = 0;

  //
  // Enable the LFRC.
  //
  am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_LFRC);

  //
  // Set up timer A0 to count 3MHz clocks but don't start it yet
  //
  am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);
  am_hal_ctimer_config(0, &sTimer0Config);

  am_hal_ctimer_int_register(AM_REG_CTIMER_INTEN_CTMRA0C0INT_M,am_waveform_handler);

  //
  // Set up timerA0 to 1024Hz.
  //
  am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, 31, 0);

  //
  // Clear the timer Interrupt
  //
  am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);

  //
  // Enable the timer interrupt in the NVIC.
  //
#if AM_CMSIS_REGS
  NVIC_EnableIRQ(CTIMER_IRQn);
#else // AM_CMSIS_REGS
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);
#endif // AM_CMSIS_REGS

}

//*****************************************************************************
//
// IOM Initialize
//
//*****************************************************************************
void
initialize_iom(uint32_t ui32Module)
{

  //
  // IOM #0 Configuration.
  //
  am_hal_iom_config_t sIOMConfig = {
    .ui32InterfaceMode = AM_HAL_IOM_I2CMODE,
    .ui32ClockFrequency = AM_HAL_IOM_400KHZ,
    .ui8WriteThreshold = 12,
    .ui8ReadThreshold = 120,
  };

  //
  // Set up the IOM #0 SCL and SDA pins for I2C
  //
  am_hal_gpio_out_bit_set(5);
  am_hal_gpio_pin_config(5, AM_HAL_PIN_5_M0SCL | AM_HAL_GPIO_PULL1_5K);
  am_hal_gpio_out_bit_set(6);
  am_hal_gpio_pin_config(6, AM_HAL_PIN_6_M0SDA | AM_HAL_GPIO_PULL1_5K);

  //
  // Enable the IOM #0 power domain.
  //
  am_hal_iom_pwrctrl_enable(ui32Module);

  //
  // Configure IOM #0.
  //
  am_hal_iom_config(ui32Module, &sIOMConfig);

  //
  // Set up IOM #0 interrupts.
  //
  am_hal_iom_int_enable(ui32Module, AM_HAL_IOM_INT_THR | AM_HAL_IOM_INT_CMDCMP);

  //
  // Enable IOM #0.
  //
  am_hal_iom_enable(ui32Module);

}

//*****************************************************************************
//
// Start the waveform generation.
//
//*****************************************************************************
void
startWaveform(void)
{
  //
  // Enable the IOM interrupt.
  //
#if AM_CMSIS_REGS
  NVIC_EnableIRQ(IOMSTR0_IRQn);
#else // AM_CMSIS_REGS
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_IOMASTER0);
#endif // AM_CMSIS_REGS

  //
  // Enable the timer Interrupt.
  //
  am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);

  //
  // Start timer A0
  //
  am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);

  //
  // Start the timer.
  //
  am_hal_ctimer_start(3, AM_HAL_CTIMER_TIMERA);

  //
  // Enable the timer interrupt in the NVIC.
  //
  am_hal_interrupt_master_enable();

}


//*****************************************************************************
//
// Configure the ADC.
//
//*****************************************************************************
void
adc_config(void)
{
  am_hal_adc_config_t sADCConfig;

  //
  // Enable the ADC power domain.
  //
  am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_ADC);

  //
  // Set up the ADC configuration parameters.
  //
  sADCConfig.ui32Clock = AM_HAL_ADC_CLOCK_HFRC;
  sADCConfig.ui32TriggerConfig = g_ADCTestConfig.Trigger;
  sADCConfig.ui32Reference = AM_HAL_ADC_REF_INT_2P0;
  sADCConfig.ui32ClockMode = AM_HAL_ADC_CK_LOW_LATENCY;
  sADCConfig.ui32PowerMode = g_ADCTestConfig.Power;
  sADCConfig.ui32Repeat = g_ADCTestConfig.Sampling;
  am_hal_adc_config(&sADCConfig);

  //
  // For this example, the samples will be coming in slowly. This means we
  // can afford to wake up for every conversion.
  //
  if (g_ADCTestConfig.useFifo)
  {
    am_hal_adc_int_enable(AM_HAL_ADC_INT_WCINC | AM_HAL_ADC_INT_WCEXC | AM_HAL_ADC_INT_FIFOOVR2 | AM_HAL_ADC_INT_FIFOOVR1 );
  }
  else
  {
    am_hal_adc_int_enable(AM_HAL_ADC_INT_WCINC | AM_HAL_ADC_INT_WCEXC | AM_HAL_ADC_INT_CNVCMP);
  }

  //
  // Set up an ADC slot
  //
  am_hal_adc_slot_config(0, g_ADCTestConfig.Averaging |
                         g_ADCTestConfig.Precision |
                           AM_HAL_ADC_SLOT_CHSEL_SE0 |
                             AM_HAL_ADC_SLOT_ENABLE | AM_HAL_ADC_SLOT_WINDOW_EN);

  //
  // Set up the Window Comparator Limits
  //
  am_hal_adc_window_set(g_ADCTestConfig.WindowUpperLimit<<6, g_ADCTestConfig.WindowLowerLimit<<6);

  //
  // Set up the SE0 input pin.
  //
  am_hal_gpio_pin_config(16, AM_HAL_PIN_16_ADCSE0);

  //
  // Reset the sample index.
  //
  g_ADCSampleIndex = 0;

  //
  // Reset the Window Comparator counters.
  //
  g_ADCWCInclusionCount = 0;
  g_ADCWCExclusionCount = 0;

  //
  // Enable the ADC.
  //
  am_hal_adc_enable();

  //
  // Enable the ADC interrupt.
  //
#if AM_CMSIS_REGS
  NVIC_EnableIRQ(ADC_IRQn);
#else // AM_CMSIS_REGS
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_ADC);
#endif // AM_CMSIS_REGS

}

//*****************************************************************************
//
// Initialize Ctimer A3 for ADC repetitive sampling.
//
//*****************************************************************************
void
timerA3_init(void)
{
  //
  // Start a timer to trigger the ADC periodically (1 second).
  //
  am_hal_ctimer_config_single(3, AM_HAL_CTIMER_TIMERA,
                              AM_HAL_CTIMER_XT_32_768KHZ |
                                AM_HAL_CTIMER_FN_REPEAT |
                                  AM_HAL_CTIMER_INT_ENABLE |
                                    AM_HAL_CTIMER_PIN_ENABLE);

  //
  // Set the CTIMER A3 Period to be 256Hz (32K/128).
  //
  am_hal_ctimer_period_set(3, AM_HAL_CTIMER_TIMERA, 127, 0);

  //
  // Enable the timer A3 to trigger the ADC directly
  //
  am_hal_ctimer_adc_trigger_enable();

}

//*****************************************************************************
//
// Widget Setup Function.
//
//*****************************************************************************
uint32_t am_widget_adc_setup(am_widget_adc_config_t *pADCTestConfig, void **ppWidget, char *pErrStr)
{
  uint32_t      ui32Status = 0;

  g_ADCTestConfig = *pADCTestConfig;

  //
  // Initialize the widget state configuration.
  //
  *ppWidget = &g_ADCTestConfig;

  //
  // Set the system clock to maximum frequency, and set the default low-power
  // settings for this board.
  //
  am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);
  am_hal_pwrctrl_bucks_enable();
  am_hal_vcomp_disable();

  //
  // Set up GPIO on Pin 28 for timing debug.
  //
  am_hal_gpio_pin_config(28, AM_HAL_GPIO_OUTPUT);

  //
  // TimerA0 init.
  //
  timerA0_init();

  //
  // Initialize the IOM
  //
  initialize_iom(g_ADCTestConfig.ui32IOM);

  //
  // Start the CTIMER A3 for timer-based ADC measurements.
  //
  timerA3_init();

  //
  // Configure the ADC
  //
  adc_config();

  //
  // Enable interrupts.
  //
  am_hal_interrupt_master_enable();

  return ui32Status;
}

//*****************************************************************************
//
// Widget Cleanup Function.
//
//*****************************************************************************
uint32_t am_widget_adc_cleanup(void *pWidget, char *pErrStr)
{
  uint32_t      ui32Status = 0;

  //
  // Create a pointer to the ADC widget state configuration.
  //
  am_widget_adc_config_t        *pADCConfig = (am_widget_adc_config_t *)pWidget;

  //
  // Disable the ADC.
  //
  am_hal_adc_disable();
  am_hal_adc_int_disable(AM_HAL_ADC_INT_WCINC |
                         AM_HAL_ADC_INT_WCEXC |
                           AM_HAL_ADC_INT_FIFOOVR2 |
                             AM_HAL_ADC_INT_FIFOOVR1 |
                               AM_HAL_ADC_INT_SCNCMP |
                                 AM_HAL_ADC_INT_CNVCMP );

  //
  // Clear the timer Interrupt
  //
  am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);

  //
  // Disable the timer interrupt in the NVIC.
  //
#if AM_CMSIS_REGS
  NVIC_DisableIRQ(CTIMER_IRQn);
#else // AM_CMSIS_REGS
  am_hal_interrupt_disable(AM_HAL_INTERRUPT_CTIMER);
#endif // AM_CMSIS_REGS

  //
  // Disable the timer Interrupts.
  //
  am_hal_ctimer_int_disable(AM_HAL_CTIMER_INT_TIMERA0);
  am_hal_ctimer_int_disable(AM_HAL_CTIMER_INT_TIMERA3);


  //
  // Stop the Ctimers
  //
  am_hal_ctimer_stop(3, AM_HAL_CTIMER_TIMERA);
  am_hal_ctimer_stop(0, AM_HAL_CTIMER_TIMERA);

  //
  // Disable the IOM #0.
  //
  am_hal_iom_disable(pADCConfig->ui32IOM);
  am_hal_iom_pwrctrl_disable(pADCConfig->ui32IOM);
  am_hal_iom_int_disable(0, AM_HAL_IOM_INT_THR | AM_HAL_IOM_INT_CMDCMP);

  return ui32Status;
}

//*****************************************************************************
//
// Widget Test Execution Function.
//
//*****************************************************************************
uint32_t am_widget_adc_test(void *pWidget, char *pErrStr)
{
  uint32_t                      ui32Status = 0;

  //
  // Create a pointer to the ADC widget state configuration.
  //
  am_widget_adc_config_t        *pADCConfig = (am_widget_adc_config_t *)pWidget;

  //
  // Enable interrupts.
  //
  am_hal_interrupt_master_enable();

  //
  // Start the waveform generation.
  //
  startWaveform();

  //
  // Trigger the ADC sampling for the first time manually.
  //
  am_hal_adc_trigger();

  //
  // Wait until the proper number of samples has been read by the ISR.
  //
  while(g_ADCSampleIndex < pADCConfig->SampleBufSize);

  return ui32Status;
}

//*****************************************************************************
//
// Widget FFT Analysis
//
//*****************************************************************************
uint32_t am_widget_adc_analyze_fft(void *pWidget, char *pErrStr)
{
  float                         fMaxValue;
  uint32_t                      ui32MaxIndex;

  //
  // Create a pointer to the ADC widget state configuration.
  //
  am_widget_adc_config_t        *pADCConfig = (am_widget_adc_config_t *)pWidget;

  //
  // The CMSIS-DSP functions expect to work on a complex input.
  // Put the real samples into the array and zero out the complex inputs.
  //
  for (uint32_t i=0; i<FFT_SIZE; i++)
  {
    fSamples[2*i] = pADCConfig->pSampleBuf[i]/64.0;
    fSamples[2*i+1] = 0.0;
  }

  //
  //Initialize the CFFT/CIFFT module, intFlag = 0, doBitReverse = 1
  //
  arm_cfft_radix4_init_f32(&S, FFT_SIZE, 0, 1);

  //
  // Process the data through the CFFT/CIFFT module
  //
  arm_cfft_radix4_f32(&S, fSamples);

  //
  // Process the data through the Complex Magnitude Module for calculating the magnitude at each bin */
  //
  arm_cmplx_mag_f32(fSamples, fMagnitudes, FFT_SIZE);

  //
  // Zero out DC.  There is a strong DC component in the input signal.
  //
  fMagnitudes[0] = 0.0;

  //
  //Calculates maxValue and returns corresponding value.
  //
  arm_max_f32(fMagnitudes, FFT_SIZE/2, &fMaxValue, &ui32MaxIndex);

  return ui32MaxIndex;
}

//*****************************************************************************
//
// Widget Edge Analysis
//
//*****************************************************************************
bool am_widget_adc_analyze_edges(void *pWidget, char *pErrStr)
{
  uint32_t                      numEdges = 0;
  int32_t                       delta;

  //
  // Create a pointer to the ADC widget state configuration.
  //
  am_widget_adc_config_t        *pADCConfig = (am_widget_adc_config_t *)pWidget;

  //
  // Scan the sample buffer looking for edges and count them.
  //
  for (uint32_t i=0; i<FFT_SIZE-1; i++)
  {
    delta = (int32_t)(pADCConfig->pSampleBuf[i+1]) - (int32_t)(pADCConfig->pSampleBuf[i]);
    if ((delta > EDGE_SCALE) || (delta < (-EDGE_SCALE)))
    {
      numEdges++;
    }
  }

  //
  // The number of edges should be either (2*Frequency) or (2*Frequency - 1).  A picket fence phenomenon.
  //
  return ((numEdges == (pADCConfig->ui32Frequency*2)) || (numEdges == (pADCConfig->ui32Frequency*2 -1)));

}

//*****************************************************************************
//
// Widget Window Comparator Analysis
//
//*****************************************************************************
void am_widget_adc_analyze_comparator(void *pWidget, char *pErrStr, uint32_t *ExclusionCount, uint32_t *InclusionCount)
{
  //
  // Return the Window Comparator Counts.
  //
  *ExclusionCount = g_ADCWCExclusionCount;
  *InclusionCount = g_ADCWCInclusionCount;

}

//*****************************************************************************
//
// Widget Window Comparator Analysis
//
//*****************************************************************************
void am_widget_adc_analyze_fifo(void *pWidget, char *pErrStr, uint32_t *FifoCount)
{
  //
  // Return the Fifo Sample Index.
  //
  *FifoCount = g_ADCSampleIndex;

}





