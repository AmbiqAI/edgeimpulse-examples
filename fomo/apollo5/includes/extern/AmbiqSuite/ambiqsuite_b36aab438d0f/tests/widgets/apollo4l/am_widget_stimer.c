
//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_widget_stimer.h"
#include "am_util.h"
#include "stdlib.h"
#include "unity.h"

volatile uint32_t g_ui32GlobalStatus = 0;

const am_hal_gpio_pincfg_t g_AM_HAL_GPIO_PIN_0 =
{
  .GP.cfg_b.uFuncSel    = AM_HAL_PIN_0_GPIO,
  .GP.cfg_b.eGPOutCfg   = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
  .GP.cfg_b.eGPInput    = AM_HAL_GPIO_PIN_INPUT_ENABLE
};

am_hal_gpio_pincfg_t g_OutPinCfg =
{
    .GP.cfg_b.uFuncSel             = 6, //CT
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
    .GP.cfg_b.uSlewRate            = 0,
    .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.uNCE                 = 0,
    .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,
    .GP.cfg_b.uRsvd_0              = 0,
    .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE,
    .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.uRsvd_1              = 0,
};

void
am_ctimer_isr(void)
{
  uint32_t ui32Status;

  ui32Status = am_hal_stimer_int_status_get(true);
  am_hal_stimer_int_clear(ui32Status);

  //
  // If we get one of the expected interrupts, pass it through to the global
  // status. Otherwise, pass an error into the global status.
  //
  switch(ui32Status)
  {
  case AM_HAL_STIMER_INT_OVERFLOW:
  case AM_HAL_STIMER_INT_CAPTUREA:
  case AM_HAL_STIMER_INT_CAPTUREB:
  case AM_HAL_STIMER_INT_CAPTUREC:
  case AM_HAL_STIMER_INT_CAPTURED:
    g_ui32GlobalStatus = ui32Status;
    break;

  default:
    g_ui32GlobalStatus = 0xFFFFFFFF;
  }
}

// STIMER snapshot when the Compare interrupt comes
uint32_t g_ui32CmpStimerVal;

void
am_stimer_cmpr0_isr(void)
{
  uint32_t ui32Status;

  g_ui32CmpStimerVal = am_hal_stimer_counter_get();

  ui32Status = am_hal_stimer_int_status_get(true);
  am_hal_stimer_int_clear(ui32Status);

  if (ui32Status == AM_HAL_STIMER_INT_COMPAREA)
  {
    g_ui32GlobalStatus = AM_HAL_STIMER_INT_COMPAREA;
  }
  else
  {
    g_ui32GlobalStatus = 0xFFFFFFFF;
    while(1);
  }
}

void
am_stimer_cmpr1_isr(void)
{
  uint32_t ui32Status;

  g_ui32CmpStimerVal = am_hal_stimer_counter_get();

  ui32Status = am_hal_stimer_int_status_get(true);
  am_hal_stimer_int_clear(ui32Status);

  if (ui32Status == AM_HAL_STIMER_INT_COMPAREB)
  {
    g_ui32GlobalStatus = AM_HAL_STIMER_INT_COMPAREB;
  }
  else
  {
    g_ui32GlobalStatus = 0xFFFFFFFF;
    while(1);
  }
}

void
am_stimer_cmpr2_isr(void)
{
  uint32_t ui32Status;

  g_ui32CmpStimerVal = am_hal_stimer_counter_get();

  ui32Status = am_hal_stimer_int_status_get(true);
  am_hal_stimer_int_clear(ui32Status);

  if (ui32Status == AM_HAL_STIMER_INT_COMPAREC)
  {
    g_ui32GlobalStatus = AM_HAL_STIMER_INT_COMPAREC;
  }
  else
  {
    g_ui32GlobalStatus = 0xFFFFFFFF;
    while(1);
  }
}

void
am_stimer_cmpr3_isr(void)
{
  uint32_t ui32Status;

  g_ui32CmpStimerVal = am_hal_stimer_counter_get();

  ui32Status = am_hal_stimer_int_status_get(true);
  am_hal_stimer_int_clear(ui32Status);

  if (ui32Status == AM_HAL_STIMER_INT_COMPARED)
  {
    g_ui32GlobalStatus = AM_HAL_STIMER_INT_COMPARED;
  }
  else
  {
    g_ui32GlobalStatus = 0xFFFFFFFF;
    while(1);
  }
}

void
am_stimer_cmpr4_isr(void)
{
  uint32_t ui32Status;

  g_ui32CmpStimerVal = am_hal_stimer_counter_get();

  ui32Status = am_hal_stimer_int_status_get(true);
  am_hal_stimer_int_clear(ui32Status);

  if (ui32Status == AM_HAL_STIMER_INT_COMPAREE)
  {
    g_ui32GlobalStatus = AM_HAL_STIMER_INT_COMPAREE;
  }
  else
  {
    g_ui32GlobalStatus = 0xFFFFFFFF;
    while(1);
  }
}

void
am_stimer_cmpr5_isr(void)
{
  uint32_t ui32Status;

  g_ui32CmpStimerVal = am_hal_stimer_counter_get();

  ui32Status = am_hal_stimer_int_status_get(true);
  am_hal_stimer_int_clear(ui32Status);

  if (ui32Status == AM_HAL_STIMER_INT_COMPAREF)
  {
    g_ui32GlobalStatus = AM_HAL_STIMER_INT_COMPAREF;
  }
  else
  {
    g_ui32GlobalStatus = 0xFFFFFFFF;
    while(1);
  }
}

void
am_stimer_cmpr6_isr(void)
{
  uint32_t ui32Status;

  g_ui32CmpStimerVal = am_hal_stimer_counter_get();

  ui32Status = am_hal_stimer_int_status_get(true);
  am_hal_stimer_int_clear(ui32Status);

  if (ui32Status == AM_HAL_STIMER_INT_COMPAREG)
  {
    g_ui32GlobalStatus = AM_HAL_STIMER_INT_COMPAREG;
  }
  else
  {
    g_ui32GlobalStatus = 0xFFFFFFFF;
    while(1);
  }
}



void
am_stimer_cmpr7_isr(void)
{
  uint32_t ui32Status;

  g_ui32CmpStimerVal = am_hal_stimer_counter_get();

  ui32Status = am_hal_stimer_int_status_get(true);
  am_hal_stimer_int_clear(ui32Status);

  if (ui32Status == AM_HAL_STIMER_INT_COMPAREH)
  {
    g_ui32GlobalStatus = AM_HAL_STIMER_INT_COMPAREH;
  }
  else
  {
    g_ui32GlobalStatus = 0xFFFFFFFF;
    while(1);
  }
}


void
am_stimerof_isr(void)
{
  uint32_t ui32Status;

  ui32Status = am_hal_stimer_int_status_get(true);
  am_hal_stimer_int_clear(ui32Status);

  g_ui32GlobalStatus = ui32Status;
}



uint32_t pui32StimerInts[] =
{
  AM_HAL_STIMER_INT_COMPAREA,
  AM_HAL_STIMER_INT_COMPAREB,
  AM_HAL_STIMER_INT_COMPAREC,
  AM_HAL_STIMER_INT_COMPARED,
  AM_HAL_STIMER_INT_COMPAREE,
  AM_HAL_STIMER_INT_COMPAREF,
  AM_HAL_STIMER_INT_COMPAREG,
  AM_HAL_STIMER_INT_COMPAREH,
  AM_HAL_STIMER_INT_OVERFLOW,
  AM_HAL_STIMER_INT_CAPTUREA,
  AM_HAL_STIMER_INT_CAPTUREB,
  AM_HAL_STIMER_INT_CAPTUREC,
  AM_HAL_STIMER_INT_CAPTURED
};

#define NUM_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

//*****************************************************************************
//
// Test all of the STIMER interrupts.
//
//*****************************************************************************
uint32_t
am_widget_stimer_int_test(void)
{
  //
  // Make sure interrupts are enabled in general.
  //
  NVIC_EnableIRQ(TIMER_IRQn);
  NVIC_EnableIRQ(STIMER_OVF_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR0_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR1_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR2_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR3_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR4_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR5_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR6_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR7_IRQn);
  am_hal_interrupt_master_enable();

  //
  // Loop over the list of interrupt conditions.
  //
  for(uint32_t i = 0; i < NUM_ELEMENTS(pui32StimerInts); i++)
  {
    //
    // Check to make sure we can enable the interupt.
    //
    am_hal_stimer_int_enable(pui32StimerInts[i]);
    if (am_hal_stimer_int_enable_get() != pui32StimerInts[i])
    {
      return 1;
    }

    //
    // Make sure the interrupt actually works.
    //
    am_hal_stimer_int_set(pui32StimerInts[i]);
    while (g_ui32GlobalStatus != pui32StimerInts[i]);

    //
    // Make sure we can disable the interrupt.
    //
    am_hal_stimer_int_disable(pui32StimerInts[i]);
    if (am_hal_stimer_int_enable_get() & pui32StimerInts[i])
    {
      return 2;
    }
  }

  return 0;
}

//*****************************************************************************
//
// Run the STIMER to see if it runs according to the settings.
//
//*****************************************************************************
uint32_t
am_widget_stimer_speed_test(uint32_t ui32Speed)
{
  uint32_t StimerVal;
  uint32_t StimerVal2;
  uint32_t StimerDelta;

  //
  // Start the timer.
  //
  StimerVal = am_hal_stimer_counter_get();
  am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_RUN);

  //
  // Delay for a pre-determined amount of time.
  //
  am_hal_delay_us(1000000);     // 1s

  //
  // Stop SysTick
  //
  am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_FREEZE);
  StimerVal2 = am_hal_stimer_counter_get();

  am_hal_gpio_state_write(24, AM_HAL_GPIO_OUTPUT_SET);
  am_hal_gpio_state_write(24, AM_HAL_GPIO_OUTPUT_CLEAR);

  //
  // Return the appoximate clock frequency.
  //
  StimerDelta = StimerVal2 - StimerVal;
  am_util_stdio_printf("\nSpeed Test at %d speed. Delta value is %d\n",ui32Speed,StimerDelta);
  return StimerDelta;
}

//*****************************************************************************
//
// Make sure the STIMER HAL functions get to the capture registers correctly.
//
//*****************************************************************************
uint32_t
am_widget_stimer_capture_test(uint32_t ui32Speed, uint32_t ui32CaptureNum)
{
    uint32_t StimerVal;
    uint32_t StimerVal2;
    uint32_t StimerDelta;

    uint32_t ui32GPIONumber = 0;
    bool bPolarity[] = {true, false};          // Capture on high to low GPIO transition
    NVIC_EnableIRQ(TIMER_IRQn);
    am_hal_interrupt_master_enable();

    for(uint8_t i = 0; i < 2; i++)
    {
        //
        // Begin by clearing the counter so that we don't overflow.
        //
        am_hal_stimer_counter_clear();

        //
        // Make sure timer is stopped.
        //
        am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR);

        //
        // Clear the capture interrupt.  Later we'll need to check the interrupt
        // status before we read the capture value.
        //
        am_hal_stimer_int_clear(0x1FFF);
        am_hal_stimer_int_enable(AM_HAL_STIMER_INT_CAPTUREA << ui32CaptureNum);

        //
        // Set up a GPIO to use as a trigger.
        //
        am_hal_gpio_pinconfig(ui32GPIONumber, g_AM_HAL_GPIO_PIN_0);
        if(bPolarity[i])
        {   //Hi2Lo
            am_hal_gpio_output_set(ui32GPIONumber);
        }
        else
        {   //Lo2Hi
            am_hal_gpio_output_clear(ui32GPIONumber);
        }

        //
        // Set up a capture.
        //
        am_hal_stimer_capture_start(ui32CaptureNum, ui32GPIONumber, bPolarity[i]);

        //
        // Start the timer.
        //
        StimerVal = am_hal_stimer_counter_get();
        am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_RUN);

        //
        // Delay for a pre-determined amount of time.
        //
        am_hal_delay_us(1000000);     // 1s

#if 0   // Used only for measuring the delay
        am_hal_gpio_out_bit_clear(ui32GPIONumber);
        am_hal_gpio_out_bit_set(ui32GPIONumber);
#endif

        //
        // High to low transition to trigger the capture.
        //
        if(bPolarity[i])
        {   //Hi2Lo
            am_hal_gpio_output_clear(ui32GPIONumber);
        }
        else
        {   //Lo2Hi
            am_hal_gpio_output_set(ui32GPIONumber);
        }

        //
        // Make sure the capture event has occurred so that the read of the
        // capture value is valid.
        //
        while ( !(g_ui32GlobalStatus &
                (AM_HAL_STIMER_INT_CAPTUREA << ui32CaptureNum)));

        //
        // Get the capture value and stop further capturing.
        //
        StimerVal2 = am_hal_stimer_capture_get(ui32CaptureNum);
        am_hal_stimer_capture_stop(ui32CaptureNum);
        am_hal_stimer_int_disable(AM_HAL_STIMER_INT_CAPTUREA << ui32CaptureNum);

        //
        // Since this is measuring a 100ms delay (0.1s), return 10x the delta
        // which should be close to the original frequency.
        //
        StimerDelta = StimerVal2 - StimerVal;
        am_util_stdio_printf("\nCapture %d Test at %d speed. Polarity: %d. Delta value is %d\n", ui32CaptureNum, ui32Speed, bPolarity[i], StimerDelta);
    }

    return StimerDelta;
}

//*****************************************************************************
//
// Make sure we can set a compare.
//
//*****************************************************************************
int32_t
am_widget_stimer_compare_test(uint32_t ui32Speed)
{
  uint32_t ui32StimerVal;
  uint32_t ui32StimerVal1;
  uint32_t ui32CompValue;
  int32_t StimerDelta, StimerDelta1;
  uint32_t ui32Ret;

  uint32_t ui32CmprInstance = 0;
  uint32_t ui32Delta = 1000;

  //
  // Start the STIMER.
  //
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_RUN | AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);

  am_hal_delay_us(3000);     // 3ms

  //
  // Save the STIMER value.
  //
  ui32StimerVal = am_hal_stimer_counter_get();

  //
  // Set a compare value for the future, and check to see what the absolute
  // value of the compare register gets set to.
  //
  ui32Ret = am_hal_stimer_compare_delta_set(ui32CmprInstance, ui32Delta);
  //
  // Save the STIMER value snapshot again.
  //
  ui32StimerVal1 = am_hal_stimer_counter_get();
  if ( ui32Ret )
  {
    return 0x40000000 | ui32Ret;
  }
  ui32CompValue = am_hal_stimer_compare_get(ui32CmprInstance);

  // 1 Cycle delay for interrupt is accounted for inside the HAL
  ui32CompValue += 1;
  //
  // If all went well, this value should be ui32StimerVal+ui32Delta (or close to it).
  //
  // Note - for Apollo3, the delta will be somewhat larger than 0
  //  due to increased read access cycle times across the APB.
  //
  StimerDelta = (ui32CompValue - (ui32StimerVal + ui32Delta));
  StimerDelta1 = (ui32CompValue - (ui32StimerVal1 + ui32Delta));
  am_util_stdio_printf("\nCompare Test at %d speed. Delta value is %d %d\n",ui32Speed,StimerDelta, StimerDelta1);
  //
  // Actual Stimer value when the delta was applied should be somewhere between
  // ui32StimerVal & ui32StimerVal1. Need to allow some tolerance
  //
  if ((StimerDelta >= 0) && (StimerDelta1 <=0))
  {
      StimerDelta = 0;
  }
  return StimerDelta;
}

//*****************************************************************************
//
// Make sure we get the Compare interrupt at correct time.
//
//*****************************************************************************
bool
am_widget_stimer_compare_int_test(uint32_t ui32Speed, uint32_t tolerance)
{
  uint32_t ui32StimerVal;
  uint32_t ui32StimerVal1;
  int32_t StimerDelta, StimerDelta1;

  uint32_t ui32Delta = 100;

  //
  // Start the STIMER.
  //
  am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
  am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_RUN);

  am_hal_delay_us(3000);     // 3ms

  am_hal_interrupt_master_enable();
  for (uint32_t i = 0; i < 8; i++)
  {
      g_ui32CmpStimerVal = g_ui32GlobalStatus = 0xFFFFFFFF;
      NVIC_ClearPendingIRQ(STIMER_CMPR0_IRQn + i);
      NVIC_EnableIRQ(STIMER_CMPR0_IRQn + i);
      am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA << i);
      am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA << i);
      am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_RUN | (AM_HAL_STIMER_CFG_COMPARE_A_ENABLE << i));
      //
      // Save the STIMER values.
      //
      // Stimer value before setting delta
      ui32StimerVal = am_hal_stimer_counter_get();
      am_hal_stimer_compare_delta_set(i, ui32Delta + i);
      // Stimer value after setting delta
      ui32StimerVal1 = am_hal_stimer_counter_get();
      // Wait for interrupt
      while(g_ui32GlobalStatus != (AM_HAL_STIMER_INT_COMPAREA << i));
      // Compute delta against expected - with both references
      StimerDelta = (g_ui32CmpStimerVal - ui32StimerVal - ui32Delta - i);
      StimerDelta1 = (g_ui32CmpStimerVal - ui32StimerVal1 - ui32Delta - i);
      am_hal_stimer_int_disable(AM_HAL_STIMER_INT_COMPAREA << i);
      NVIC_DisableIRQ(STIMER_CMPR0_IRQn + i);
      am_util_stdio_printf("\nCompare Interrupt Test at %d speed. Compare Inst %d Delta value is %d %d\n",ui32Speed, i, StimerDelta, StimerDelta1);
      if ((StimerDelta > (tolerance + ui32StimerVal1 - ui32StimerVal)) || ((StimerDelta < 0) && (-StimerDelta > (tolerance + ui32StimerVal1 - ui32StimerVal))))
      {
        am_util_stdio_printf("Start 0x%x 0x%x Delta 0x%x, End 0x%x diff %d\n", ui32StimerVal, ui32StimerVal1, ui32Delta+i, g_ui32CmpStimerVal, g_ui32CmpStimerVal - (ui32StimerVal + ui32Delta + i));
        am_util_stdio_printf("\nCompare Interrupt Test at %d speed. Compare Inst %d Delta value is %d %d\n",ui32Speed, i, StimerDelta, StimerDelta1);
        return false;
      }
  }
  am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_RUN);

  return true;
}

//*****************************************************************************
//
// Run the STIMER to see if counter value is correct (only for Apollo4P and later versions!).
//
//*****************************************************************************
bool
am_widget_stimer_counter_test(uint32_t ui32Speed)
{
    uint32_t StimerVal, ui32Status;
    uint32_t StimerValLast = 0, ui32Delta, ui32Delta1;
    bool bPass = true;
#ifdef TEST_TIMES_LIMIT
    uint32_t ui32TestCnt = 0;
#endif
    //
    // Configure overflow interrupt, didn't enable related NVIC.
    // Enable COMP_A and COMP_B interrupts for producing counter bit flip issue.
    //
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_OVERFLOW | AM_HAL_STIMER_INT_COMPAREA | AM_HAL_STIMER_INT_COMPAREB);
    //
    // Clear and start the timer.
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    //
    // Configure COMP_A and COMP_B for producing counter bit flip issue.
    //
    am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_RUN | AM_HAL_STIMER_CFG_COMPARE_A_ENABLE | AM_HAL_STIMER_CFG_COMPARE_B_ENABLE);
    //
    // Set delta to gernerate interrupt once per millisecond
    //
#ifdef APOLLO4_FPGA
    ui32Delta = 1500;
#else
    if (ui32Speed == 1) // HFRC 6MHz
    {
        ui32Delta = 6000;
    }
    else
    {
        ui32Delta = 500; //!< provisaionnal value
    }
#endif
    am_hal_stimer_compare_delta_set(0, ui32Delta);
    //
    // Make COMP_B to generate interrupt after COMP_A interrupt.
    //
    ui32Delta1 = ui32Delta + rand()%50 + 5;
    am_hal_stimer_compare_delta_set(1, ui32Delta1);
    //
    // Check counting backwards
    //
    while (1)
    {
        StimerVal = am_hal_stimer_counter_get();
        ui32Status = am_hal_stimer_int_status_get(true);
        if (ui32Status & STIMER_STMINTSTAT_OVERFLOW_Msk)
        {
            am_hal_stimer_int_clear(ui32Status);
            am_util_stdio_printf("\nSTimer counter test passed with clock source %d in one count period.\n", ui32Speed);
            break;
        }
#ifdef TEST_TIMES_LIMIT
        if (ui32TestCnt++ > 10000000)
        {
            am_util_stdio_printf("\nSTimer counter test passed with clock source %d in 10 million times of tests.\n", ui32Speed);
            break;
        }
#endif
        if (ui32Status & STIMER_STMINTSTAT_COMPAREA_Msk)
        {
            am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
            am_hal_stimer_compare_delta_set(0, ui32Delta);
        }
        if (ui32Status & STIMER_STMINTSTAT_COMPAREB_Msk)
        {
            am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREB);
            ui32Delta1 = ui32Delta + rand()%50 + 5;
            am_hal_stimer_compare_delta_set(1, ui32Delta1);
        }
        if (StimerValLast > StimerVal)
        {
            bPass = false;
            am_util_stdio_printf("\nStimer counter error - Stimer clock source %d, current counter value is 0x%08X, last counter value is 0x%08X\n", ui32Speed, StimerVal, StimerValLast);
            break;
        }
        StimerValLast = StimerVal;
    }
    return bPass;
}

//*****************************************************************************
//
// Make sure the STIMER can wakeup core correctly.
//
//*****************************************************************************
#define MANUAL_TRIGGER 0

#if defined(APOLLO4_FPGA)
#define HFRC_BASE (APOLLO4_FPGA * 1000000)
#else
#define HFRC_BASE 96000000
#endif

void
am_widget_stimer_sleep_wakeup_test(void)
{
    am_hal_timer_config_t       TimerConfig;
    uint32_t ui32CapIONum = 0;
    uint32_t ui32WakeupSignalIO = 1;

    am_hal_gpio_pinconfig(ui32WakeupSignalIO, g_AM_HAL_GPIO_PIN_0);
    am_hal_gpio_output_clear(ui32WakeupSignalIO);

    //
    // Begin by clearing the counter so that we don't overflow.
    //
    am_hal_stimer_counter_clear();

    //
    // Make sure timer is stopped.
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR);

    //
    // Clear the capture interrupt.  Later we'll need to check the interrupt
    // status before we read the capture value.
    //
    am_hal_stimer_int_clear(0x1FFF);
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_CAPTUREA);
    NVIC_EnableIRQ(STIMER_OVF_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Set up a capture.
    //
    am_hal_stimer_capture_start(0, ui32CapIONum, false);

    //
    // Start the timer.
    //
    am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);

    //
    // Set up the default configuration.
    //
    am_hal_timer_default_config_set(&TimerConfig);

#if (MANUAL_TRIGGER == 0)
    //
    // Update the clock in the config.
    //
    TimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    TimerConfig.ui32Compare0 = HFRC_BASE / 16 * 105 / 100;

    // Config output pins
    am_hal_timer_output_config(ui32CapIONum, AM_HAL_TIMER_OUTPUT_TMR0_OUT0);
    am_hal_gpio_pinconfig(ui32CapIONum, g_OutPinCfg);

    //
    // Configure the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_config(0, &TimerConfig));

    //
    // Start the TIMER.
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_timer_start(0));
#endif

    //
    // Go to Normal Sleep.
    //
    am_util_stdio_printf("\nFall into normal sleep\n");
#if (MANUAL_TRIGGER == 1)
    am_util_stdio_printf("Connect GPIO1 to VDD manually to wakeup the core\n");
#endif
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);

    am_util_stdio_printf("\nWake up done\n");
#if (MANUAL_TRIGGER == 1)
    am_util_stdio_printf("Disconnect GPIO1 now\n");
#endif
    am_hal_gpio_output_toggle(ui32WakeupSignalIO);

    //
    // Go to Deep Sleep.
    //
    am_util_stdio_printf("\nFall into deep sleep\n");
#if (MANUAL_TRIGGER == 1)
    am_util_stdio_printf("Connect GPIO1 to VDD manually to wakeup the core\n");
#endif
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

    am_util_stdio_printf("\nWake up done\n");
    am_hal_gpio_output_toggle(ui32WakeupSignalIO);
}


