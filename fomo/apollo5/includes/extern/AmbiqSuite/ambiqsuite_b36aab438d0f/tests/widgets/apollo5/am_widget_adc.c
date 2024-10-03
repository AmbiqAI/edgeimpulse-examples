//*****************************************************************************
//
//! @file am_widget_adc.c
//!
//! @brief This widget allows test cases to exercise the Apollo5 ADC using an
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
//!     GPIO[19]- ADC EXT Input #0 (SE0) from pin 6 (VOUT) of the MCP4725
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
#define BITSCALE        (0x2C0)
#define BITOFFSET       (0x2C0)
#define TIMER_FREQ      (1024)
#define EDGE_SCALE      (3900)

//*****************************************************************************
//
// Memory allocation.
//
//*****************************************************************************
//
// ADC Device Handle.
//
static void *g_ADCHandle;

//
// IOM Device Handle.
//
static void *g_pIOMHandle;

//
// ADC test configuration (defines the widget state).
//
am_widget_adc_config_t          g_ADCTestConfig;

//
// ADC sample counter.
//
volatile uint32_t               g_ADCSampleIndex;

//
// ADC DMA complete flag.
//
volatile bool                   g_bADCDMAComplete;

//
// ADC DMA error flag.
//
volatile bool                   g_bADCDMAError;

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

//
// Variables for FFT calculations by CMSIS-DSP library.
//
arm_cfft_radix4_instance_f32    S;
arm_status                      status;
float                           fSamples[SAMPLES];
float                           fMagnitudes[FFT_SIZE];

//
// IOM Configuration.
//
static am_hal_iom_config_t g_sIOMSpiConfig =
{
    .eInterfaceMode     = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq      = AM_HAL_IOM_400KHZ,
    .pNBTxnBuf          = NULL,
    .ui32NBTxnBufLength = 0
};

#if ADC_TRIGGER_SW_TIMER
uint8_t g_ui8AdcTriggerCount = 0;
#endif

//*****************************************************************************
//
// Send DAC Command
//
//*****************************************************************************
void
sendDACCommand(uint32_t ui32Address, uint16_t ui16Value)
{
  am_hal_iom_transfer_t       Transaction;

  //
  // Scale the value into a 2-byte 12-bit format.
  //
  g_sRamBuffer.bytes[0] = (ui16Value & 0x0F00) >> 8;
  g_sRamBuffer.bytes[1] = (ui16Value & 0xFF);


  //
  // Create the transaction and send the data.
  Transaction.uPeerInfo.ui32I2CDevAddr  = ui32Address;
  Transaction.ui32InstrLen              = 0;
  Transaction.ui64Instr                 = 0;
  Transaction.eDirection                = AM_HAL_IOM_TX;
  Transaction.ui32NumBytes              = 2;
  Transaction.pui32TxBuffer             = g_sRamBuffer.words;
  Transaction.bContinue                 = false;
  Transaction.ui8RepeatCount            = 0;
  Transaction.ui32PauseCondition        = 0;
  Transaction.ui32StatusSetClr          = 0;
  am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);

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

  am_hal_timer_interrupt_status_get(true, &ui32Status);
  
  am_hal_timer_interrupt_clear(ui32Status);

  //am_hal_timer_interrupt_clear(AM_HAL_TIMER_INT_TMR0_CMP0_WG);

  am_waveform_handler();

#if ADC_TRIGGER_SW_TIMER
  g_ui8AdcTriggerCount++;
  if(g_ui8AdcTriggerCount == 4) {
      am_hal_adc_sw_trigger(g_ADCHandle);
      g_ui8AdcTriggerCount = 0;
  }
#endif
}

void
am_timer07_isr(void)
{
//  uint32_t ui32Status;

//  am_hal_timer_interrupt_status_get(true, &ui32Status);
//  am_hal_timer_interrupt_clear(ui32Status);
//  am_hal_timer_interrupt_clear(AM_HAL_TIMER_INT_TMR7_CMP0_WG);
}
//*****************************************************************************
//
// Interrupt handler for IOM0
//
//*****************************************************************************
void
am_iomaster0_isr(void)
{
    uint32_t ui32IntMask;

    //
    // Read and clear the interrupt status.
    //
    am_hal_iom_interrupt_status_get(g_pIOMHandle, false, &ui32IntMask);
    am_hal_iom_interrupt_clear(g_pIOMHandle, ui32IntMask);

    //
    // Service FIFO interrupts as necessary, and call IOM callbacks as
    // transfers are completed.
    //
    am_hal_iom_interrupt_service(g_pIOMHandle, ui32IntMask);
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
  uint32_t ui32IntMask;
  uint32_t ui32NumSamples;

  //
  // Read the interrupt status.
  //
  ui32Status = am_hal_adc_interrupt_status(g_ADCHandle, &ui32IntMask, true);

  //
  // Clear the ADC interrupt.
  //
  am_hal_adc_interrupt_clear(g_ADCHandle,ui32IntMask);

  //
  // If we got a conversion completion interrupt (which should be our only
  // ADC interrupt), go ahead and read the data.
  //
  if (ui32IntMask & AM_HAL_ADC_INT_CNVCMP)//AM_HAL_ADC_INT_SCNCMP)
  {
    //
    // Read the value from the FIFO.
    //
    ui32NumSamples = 1;
    ui32Status = am_hal_adc_samples_read(g_ADCHandle, false,
                                         NULL,
                                         &ui32NumSamples,
                                         &g_ADCTestConfig.pSampleBuf[g_ADCSampleIndex]);

    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Error - failed to read samples from ADC.\n");
    }

    //
    // Update the global sample count.
    //
    g_ADCSampleIndex += ui32NumSamples;

  }

  //
  // Check if we got the FIFO 100% full interrupt.  This should not happen.
  //
  // TODO-Fixme: waiting for am_hal_debug finished.
//  am_hal_debug_assert_msg(0 == (ui32Status & AM_HAL_ADC_INT_FIFOOVR2),"ADC Test Failed.  Received a FIFO 100% full interrupt!");

  //
  // If we got a FIFO 75% full, go ahead and read the data.
  //
  if ((ui32IntMask & AM_HAL_ADC_INT_FIFOOVR1) && (g_ADCTestConfig.useDMA == 0))
  {
    //
    // Read the value from the FIFO.
    //
    ui32NumSamples = 16;
    ui32Status = am_hal_adc_samples_read(g_ADCHandle, false,
                                         NULL,
                                         &ui32NumSamples,
                                         &g_ADCTestConfig.pSampleBuf[g_ADCSampleIndex]);

    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      am_util_stdio_printf("Error - failed to read samples from ADC.\n");
    }

    //
    // Update the global sample count.
    //
    g_ADCSampleIndex += ui32NumSamples;

  }

  //
  // If we got an Window Comparator Inclusion interrupt, count it.
  //
  if (ui32IntMask & AM_HAL_ADC_INT_WCINC)
  {
    g_ADCWCInclusionCount++;
  }

  //
  // If we got an Window Comparator Exclusion interrupt, count it.
  //
  if (ui32IntMask & AM_HAL_ADC_INT_WCEXC)
  {
    g_ADCWCExclusionCount++;
  }

  //
  // If we got a DMA complete, set the flag. (Workaround)
  //
  if (ui32IntMask & AM_HAL_ADC_INT_FIFOOVR1)
  {
      if ( ADCn(0)->DMASTAT_b.DMACPL )
      {
          g_bADCDMAComplete = true;
      }
  }

  //
  // If we got a DMA complete, set the flag.
  //
  if (ui32IntMask & AM_HAL_ADC_INT_DCMP)
  {
    g_bADCDMAComplete = true;
  }

  //
  // If we got a DMA error, set the flag.
  //
  if (ui32IntMask & AM_HAL_ADC_INT_DERR)
  {
    g_bADCDMAError = true;
  }

}

//*****************************************************************************
//
// Init function for Timer A0.
//
//*****************************************************************************
void
timerA0_init(void)
{
  am_hal_timer_config_t sTimer0Config;
  
  am_hal_timer_default_config_set(&sTimer0Config);

  // Set up Timer0A.
  sTimer0Config.eInputClock = AM_HAL_TIMER_CLOCK_XT;
  sTimer0Config.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
  
  //
  // Calculate and store the period.
  //
  g_ui32WaveformPeriod = TIMER_FREQ/g_ADCTestConfig.ui32Frequency;

  //
  // Reset the waveform index.
  //
  g_ui32WaveformIndex = 0;

  //
  // Set up timer A0 to count 3MHz clocks but don't start it yet
  //
  am_hal_timer_clear(0);
  am_hal_timer_config(0, &sTimer0Config);

//  am_hal_ctimer_int_register(CTIMER_INTEN_CTMRA0C0INT_Msk, am_waveform_handler);

  //
  // Set up timerA0 to 1024Hz.
  //
  am_hal_timer_compare0_set(0, 31);

  //
  // Clear and enable the timer Interrupt
  //
  am_hal_timer_interrupt_enable(AM_HAL_TIMER_INT_TMR0_CMP0_WG);
  am_hal_timer_interrupt_clear(AM_HAL_TIMER_INT_TMR0_CMP0_WG);

  //
  // Enable the timer interrupt in the NVIC.
  //
  NVIC_EnableIRQ(TIMER_IRQn);
}

//*****************************************************************************
//
// Init function for Timer A0.
//
//*****************************************************************************
#if ADC_TRIGGER_TIMER_7_ADCEN
void
timer7_init(void)
{
  am_hal_timer_config_t sTimer7Config;
  
  am_hal_timer_default_config_set(&sTimer7Config);
  
  // Set up Timer7.
  sTimer7Config.eInputClock = AM_HAL_TIMER_CLOCK_XT;
  sTimer7Config.eFunction = AM_HAL_TIMER_FN_UPCOUNT;

  am_hal_timer_clear(7);
  am_hal_timer_config(7, &sTimer7Config);

  //
  // Set up timer 7 to ADC trigger enable.
  //
  TIMER->GLOBEN_b.ADCEN = TIMER_GLOBEN_ADCEN_EN;
  

  //
  // Set up timer 7 to 256Hz // 1024Hz.
  //
  am_hal_timer_compare0_set(7, (32 * 4) - 1);

  //
  // Clear and enable the timer Interrupt
  //
  am_hal_timer_interrupt_enable(AM_HAL_TIMER_INT_TMR7_CMP0_WG);
  am_hal_timer_interrupt_clear(AM_HAL_TIMER_INT_TMR7_CMP0_WG);

  //
  // Enable the timer interrupt in the NVIC.
  //
  NVIC_EnableIRQ(TIMER7_IRQn);
}
#endif

//*****************************************************************************
//
// IOM Initialize
//
//*****************************************************************************
void
initialize_iom(uint32_t ui32Module)
{
#if 0
  const am_hal_gpio_pincfg_t g_AM_PIN_5_M0SCL =
  {
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_5_M0SCL,
    .GP.cfg_b.ePullup        = AM_HAL_GPIO_PIN_PULLUP_1_5K,
  };
  const am_hal_gpio_pincfg_t g_AM_PIN_6_M0SDAWIR3 =
  {
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_6_M0SDAWIR3,
    .GP.cfg_b.ePullup        = AM_HAL_GPIO_PIN_PULLUP_1_5K,
  };

  //
  // Set up the IOM #0 SCL and SDA pins for I2C
  //
  am_hal_gpio_pinconfig(5, g_AM_PIN_5_M0SCL);
  am_hal_gpio_pinconfig(6, g_AM_PIN_6_M0SDAWIR3);
#endif
  
  //
  // Initialize the IOM.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_iom_initialize(ui32Module,&g_pIOMHandle))
  {
    am_util_stdio_printf("Error - failed to initialize the IOM instance\n");
  }

  //
  // Turn on the IOM.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_iom_power_ctrl(g_pIOMHandle, AM_HAL_SYSCTRL_WAKE, false))
  {
    am_util_stdio_printf("Error - failed to power on the IOM\n");
  }

  //
  // Configure IOM #0.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_iom_configure(g_pIOMHandle, &g_sIOMSpiConfig))
  {
    am_util_stdio_printf("Error - failed to configure the IOM\n");
  }

  //
  // Set up IOM #0 interrupts.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_iom_interrupt_enable(g_pIOMHandle, AM_HAL_IOM_INT_CMDCMP))
  {
    am_util_stdio_printf("Error - failed to enable the IOM interrupts\n");
  }

  //
  // Enable IOM #0.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_iom_enable(g_pIOMHandle))
  {
    am_util_stdio_printf("Error - failed to enable the IOM\n");
  }

  am_bsp_iom_pins_enable(0, AM_HAL_IOM_I2C_MODE);
  
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
  NVIC_EnableIRQ(IOMSTR0_IRQn);

  //
  // Enable the timer Interrupt.
  //
  am_hal_timer_interrupt_clear(AM_HAL_TIMER_INT_TMR0_CMP0_WG);
#if ADC_TRIGGER_TIMER_7_ADCEN
  am_hal_timer_interrupt_clear(AM_HAL_TIMER_INT_TMR7_CMP0_WG);
#endif

  //
  // Start timer A0
  //
  am_hal_timer_start(0);
#if ADC_TRIGGER_TIMER_7_ADCEN
  am_hal_timer_start(7);
#endif
  
  //
  // Start the timer.
  //
//  am_hal_ctimer_start(3, AM_HAL_CTIMER_TIMERA);

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
  am_hal_adc_config_t           ADCConfig;
  am_hal_adc_slot_config_t      ADCSlotConfig;
  am_hal_adc_window_config_t    ADCWindowConfig;
  am_hal_adc_dma_config_t       ADCDMAConfig;
  uint32_t                      ui32IntMask;
  const am_hal_gpio_pincfg_t g_AM_PIN_19_ADCSE0 =
  {
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_19_ADCSE0,
  };

  //
  // Initialize the ADC and get the handle.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_initialize(ADC_TEST_MODULE,&g_ADCHandle))
  {
    am_util_stdio_printf("Error - failed to initialize the ADC instance.\n");
  }

  //
  // Turn on the ADC.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle, AM_HAL_SYSCTRL_WAKE, false))
  {
    am_util_stdio_printf("Error - failed to power on the ADC\n");
  }

  //
  // Set up the ADC configuration parameters.
  //
  ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC_24MHZ;
  ADCConfig.ePolarity          = AM_HAL_ADC_TRIGPOL_RISING;
  ADCConfig.eTrigger           = g_ADCTestConfig.ADCConfig.eTrigger;
  ADCConfig.eClockMode         = AM_HAL_ADC_CLKMODE_LOW_LATENCY;
  ADCConfig.ePowerMode         = g_ADCTestConfig.ADCConfig.ePowerMode;
  ADCConfig.eRepeat            = g_ADCTestConfig.ADCConfig.eRepeat;
  ADCConfig.eRepeatTrigger     = g_ADCTestConfig.ADCConfig.eRepeatTrigger;
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure(g_ADCHandle, &ADCConfig))
  {
    am_util_stdio_printf("Error - failed to configure the ADC\n");
  }

#if ADC_TRIGGER_RPTTRIG
   //
   // Set up internal repeat trigger timer
   //
   am_hal_adc_irtt_config_t      ADCIrttConfig =
   {
       .bIrttEnable        = true,
       .eClkDiv            = 5, // divided by 32 (48MHz / 32)
       //.eClkDiv            = AM_HAL_ADC_RPTT_CLK_DIV16, // 48MHz
       .ui32IrttCountMax   = 1023,
   };

    am_hal_adc_configure_irtt(g_ADCHandle, &ADCIrttConfig);
#endif

  //
  // For this example, the samples will be coming in slowly. This means we
  // can afford to wake up for every conversion.
  //
  if (g_ADCTestConfig.useFifo)
  {
    ui32IntMask = AM_HAL_ADC_INT_WCINC | AM_HAL_ADC_INT_WCEXC | AM_HAL_ADC_INT_FIFOOVR2 | AM_HAL_ADC_INT_FIFOOVR1;
  }
  else if (g_ADCTestConfig.useDMA)
  {
    // Workaround for ADC DMA interrup handling.
    ui32IntMask = AM_HAL_ADC_INT_DERR | AM_HAL_ADC_INT_DCMP | AM_HAL_ADC_INT_FIFOOVR2 | AM_HAL_ADC_INT_FIFOOVR1;
    //ui32IntMask = AM_HAL_ADC_INT_DERR | AM_HAL_ADC_INT_DCMP;
  }
  else
  {
    ui32IntMask = AM_HAL_ADC_INT_WCINC | AM_HAL_ADC_INT_WCEXC | AM_HAL_ADC_INT_CNVCMP | AM_HAL_ADC_INT_SCNCMP;
  }
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_enable(g_ADCHandle, ui32IntMask))
  {
    am_util_stdio_printf("Error - failed to enable the ADC interrupts\n");
  }

  //
  // Set up an ADC slot
  //
  ADCSlotConfig.eMeasToAvg      = g_ADCTestConfig.SlotConfig.eMeasToAvg;
  ADCSlotConfig.ePrecisionMode  = g_ADCTestConfig.SlotConfig.ePrecisionMode;
  ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE0;
  ADCSlotConfig.bWindowCompare  = true;
  ADCSlotConfig.bEnabled        = true;
  ADCSlotConfig.ui32TrkCyc      = g_ADCTestConfig.SlotConfig.ui32TrkCyc;
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 0, &ADCSlotConfig))
  {
    am_util_stdio_printf("Error - failed to configure the ADC slots\n");
  }

  //
  // Set up the ADC Window comparison.
  //
  ADCWindowConfig.bScaleLimits = false;
  ADCWindowConfig.ui32Upper    = (g_ADCTestConfig.WindowConfig.ui32Upper<<6);
  ADCWindowConfig.ui32Lower    = (g_ADCTestConfig.WindowConfig.ui32Lower<<6);
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_control(g_ADCHandle, AM_HAL_ADC_REQ_WINDOW_CONFIG, (void *)&ADCWindowConfig))
  {
    am_util_stdio_printf("Error - failed to configure the ADC window limits\n");
  }

  //
  // Set up DMA if requested.
  //
  if (g_ADCTestConfig.useDMA)
  {
    ADCDMAConfig.bDynamicPriority = true;
    ADCDMAConfig.ePriority = AM_HAL_ADC_PRIOR_SERVICE_IMMED;
    ADCDMAConfig.bDMAEnable = true;
    ADCDMAConfig.ui32SampleCount = g_ADCTestConfig.DMABufSize;
    ADCDMAConfig.ui32TargetAddress = (uint32_t)g_ADCTestConfig.pDMABuffer;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_dma(g_ADCHandle, &ADCDMAConfig))
    {
      am_util_stdio_printf("Error - failed to configure the ADC DMA\n");
    }
  }

  //
  // Set up the SE0 input pin.
  //
  am_hal_gpio_pinconfig(19, g_AM_PIN_19_ADCSE0);

  //
  // Reset the sample index.
  //
  g_ADCSampleIndex = 0;

  //
  // Clear the DMA Complete flag.
  //
  g_bADCDMAComplete = false;

  //
  // Clear the DMA Error flag.
  //
  g_bADCDMAError = false;

  //
  // Reset the Window Comparator counters.
  //
  g_ADCWCInclusionCount = 0;
  g_ADCWCExclusionCount = 0;

  //
  // Enable the ADC.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_enable(g_ADCHandle))
  {
    am_util_stdio_printf("Error - failed to enable the ADC\n");
  }

#if ADC_TRIGGER_RPTTRIG
  //
  // Enable internal repeat trigger timer
  //
  am_hal_adc_irtt_enable(g_ADCHandle);
#endif

  //
  // Enable the ADC interrupt.
  //
  NVIC_EnableIRQ(ADC_IRQn);
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
  // TimerA0 init.
  //
  timerA0_init();

  //
  // Initialize the IOM
  //
  initialize_iom(g_ADCTestConfig.ui32IOM);

  //
  // Configure the ADC
  //
  adc_config();

#if ADC_TRIGGER_TIMER_7_ADCEN
  timer7_init();
#endif
  
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
  // Disable the ADC.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_disable(g_ADCHandle))
  {
    am_util_stdio_printf("Error - failed to disable the ADC\n");
  }

  ui32Status = am_hal_adc_interrupt_disable(g_ADCHandle,
                                            AM_HAL_ADC_INT_WCINC |
                                              AM_HAL_ADC_INT_WCEXC |
                                                AM_HAL_ADC_INT_FIFOOVR2 |
                                                  AM_HAL_ADC_INT_FIFOOVR1 |
                                                    AM_HAL_ADC_INT_SCNCMP |
                                                      AM_HAL_ADC_INT_CNVCMP );
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
    am_util_stdio_printf("Error - failed to disable the ADC interrupts\n");
  }

  //
  // Turn off the ADC.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false))
  {
    am_util_stdio_printf("Error - failed to power off the ADC\n");
  }

  if (AM_HAL_STATUS_SUCCESS != am_hal_adc_deinitialize(g_ADCHandle))
  {
    am_util_stdio_printf("Error - failed to deinitialize the ADC\n");
  }

  //
  // Clear the timer Interrupt
  //
  am_hal_timer_interrupt_clear(AM_HAL_TIMER_INT_TMR0_CMP0_WG);
#if ADC_TRIGGER_TIMER_7_ADCEN
  am_hal_timer_interrupt_clear(AM_HAL_TIMER_INT_TMR7_CMP0_WG);
#endif

  //
  // Disable the timer interrupt in the NVIC.
  //
  NVIC_DisableIRQ(TIMER_IRQn);

  //
  // Disable the timer Interrupts.
  //
  am_hal_timer_interrupt_disable(AM_HAL_TIMER_INT_TMR0_CMP0_WG);
#if ADC_TRIGGER_TIMER_7_ADCEN
  am_hal_timer_interrupt_disable(AM_HAL_TIMER_INT_TMR7_CMP0_WG);
#endif


  //
  // Stop the Ctimers
  //
//  am_hal_ctimer_stop(3, AM_HAL_CTIMER_TIMERA);
  am_hal_timer_stop(0);
#if ADC_TRIGGER_TIMER_7_ADCEN
  am_hal_timer_stop(7);
#endif

  //
  // Disable the IOM #0.
  //
  if (AM_HAL_STATUS_SUCCESS != am_hal_iom_disable(g_pIOMHandle))
  {
    am_util_stdio_printf("Error - failed to disable the IOM\n");
  }

  if (AM_HAL_STATUS_SUCCESS != am_hal_iom_interrupt_disable(g_pIOMHandle, AM_HAL_IOM_INT_CMDCMP))
  {
    am_util_stdio_printf("Error - failed to disable the IOM interrupts\n");
  }

  if (AM_HAL_STATUS_SUCCESS != am_hal_iom_uninitialize(g_pIOMHandle))
  {
    am_util_stdio_printf("Error - failed to uninitialize the IOM\n");
  }

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
  am_hal_adc_sw_trigger(g_ADCHandle);

  //
  // Wait until the proper number of samples has been read by the ISR.
  //
  while(g_ADCSampleIndex < pADCConfig->SampleBufSize);

  return ui32Status;
}

//*****************************************************************************
//
// Widget DMA Test Execution Function.
//
//*****************************************************************************
uint32_t am_widget_adc_dma_test(void *pWidget, char *pErrStr)
{
  uint32_t ui32Status;
  uint32_t ui32NumSamples;

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
  am_hal_adc_sw_trigger(g_ADCHandle);

  //
  // Wait until we get a DMA Complete or DMA Error interrupt.
  //
  while( (!g_bADCDMAComplete) && (!g_bADCDMAError) );

  //
  // Process the DMA buffer.
  //
  ui32NumSamples = g_ADCTestConfig.DMABufSize;
  ui32Status = am_hal_adc_samples_read(g_ADCHandle, false,
                                       g_ADCTestConfig.pDMABuffer,
                                       &ui32NumSamples,
                                       g_ADCTestConfig.pSampleBuf);

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
    fSamples[2*i] = pADCConfig->pSampleBuf[i].ui32Sample/64.0;
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
  bool                          result;

  //
  // Create a pointer to the ADC widget state configuration.
  //
  am_widget_adc_config_t        *pADCConfig = (am_widget_adc_config_t *)pWidget;

  //
  // Scan the sample buffer looking for edges and count them.
  //
  for (uint32_t i=0; i<FFT_SIZE-1; i++)
  {
    delta = (int32_t)(pADCConfig->pSampleBuf[i+1].ui32Sample) -
      (int32_t)(pADCConfig->pSampleBuf[i].ui32Sample);
    if ((delta > EDGE_SCALE) || (delta < (-EDGE_SCALE)))
    {
      numEdges++;
    }
  }

  //
  // The number of edges should be either (2*Frequency) or (2*Frequency - 1).  A picket fence phenomenon.
  //
  result = ((numEdges >= (pADCConfig->ui32Frequency*2 - 2)) && (numEdges <= (pADCConfig->ui32Frequency*2 + 2)));

  if (result)
  {
    return true;
  }
  else
  {
    return false;
  }

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
// Widget FIFO Analysis
//
//*****************************************************************************
void am_widget_adc_analyze_fifo(void *pWidget, char *pErrStr, uint32_t *FifoCount)
{
  //
  // Return the Fifo Sample Index.
  //
  *FifoCount =   g_ADCSampleIndex;

}


//*****************************************************************************
//
// Widget Window DMA Analysis
//
//*****************************************************************************
void am_widget_adc_analyze_dma(void *pWidget, char *pErrStr, uint32_t *DMACount)
{
  //
  // Return the DMA Total Count.
  //
  *DMACount =   ADC->DMATOTCOUNT_b.TOTCOUNT;

}





