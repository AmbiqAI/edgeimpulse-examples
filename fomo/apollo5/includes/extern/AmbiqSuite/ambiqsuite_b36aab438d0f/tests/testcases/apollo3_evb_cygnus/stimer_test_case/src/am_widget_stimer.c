
//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_widget_stimer.h"
#include "am_util.h"

volatile uint32_t g_ui32GlobalStatus = 0;

#if (defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P))
const am_hal_gpio_pincfg_t g_AM_HAL_GPIO_PIN_24 =
{
  .uFuncSel             = AM_HAL_PIN_24_GPIO,
};

const am_hal_gpio_pincfg_t g_AM_HAL_GPIO_PIN_0 =
{
  .uFuncSel             = AM_HAL_PIN_0_GPIO,
  .eGPOutcfg            = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
  .eGPInput             = AM_HAL_GPIO_PIN_INPUT_ENABLE,
};
#elif (defined(AM_PART_APOLLO4))
const am_hal_gpio_pincfg_t g_AM_HAL_GPIO_PIN_0 =
{
  .GP.cfg_b.uFuncSel    = AM_HAL_PIN_0_GPIO,
  .GP.cfg_b.eGPOutCfg   = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
  .GP.cfg_b.eGPInput    = AM_HAL_GPIO_PIN_INPUT_ENABLE
};
#endif

#if (defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P))
void
am_stimer_isr(void)
#elif (defined AM_PART_APOLLO4)
void
am_ctimer_isr(void)
#endif
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

void
am_stimer_cmpr0_isr(void)
{
  uint32_t ui32Status;

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

  if (ui32Status == AM_HAL_STIMER_INT_OVERFLOW)
  {
    g_ui32GlobalStatus = AM_HAL_STIMER_INT_OVERFLOW;
  }
  else
  {
    g_ui32GlobalStatus = 0xFFFFFFFF;
    while(1);
  }
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
  AM_HAL_STIMER_INT_OVERFLOW
#if 0 // FIXME - Where are the capture ISRs???
  AM_HAL_STIMER_INT_CAPTUREA,
  AM_HAL_STIMER_INT_CAPTUREB,
  AM_HAL_STIMER_INT_CAPTUREC,
  AM_HAL_STIMER_INT_CAPTURED
#endif
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
#if AM_CMSIS_REGS
#if (defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P))
  NVIC_EnableIRQ(STIMER_IRQn);
#elif defined(AM_PART_APOLLO4)
  NVIC_EnableIRQ(TIMER_IRQn);
  NVIC_EnableIRQ(STIMER_OVF_IRQn);
#endif
  NVIC_EnableIRQ(STIMER_CMPR0_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR1_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR2_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR3_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR4_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR5_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR6_IRQn);
  NVIC_EnableIRQ(STIMER_CMPR7_IRQn);
#else // AM_CMSIS_REGS
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_STIMER);
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_STIMER_CMPR0);
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_STIMER_CMPR1);
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_STIMER_CMPR2);
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_STIMER_CMPR3);
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_STIMER_CMPR4);
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_STIMER_CMPR5);
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_STIMER_CMPR6);
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_STIMER_CMPR7);
#endif // AM_CMSIS_REGS
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
#if (defined(AM_PART_APOLLO4))
  am_hal_delay_us(1000000);     // 1s
#else
  am_hal_flash_delay(FLASH_CYCLES_US(1000000));     // 1s
#endif
  
  //
  // Stop SysTick
  //
  am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_FREEZE);
  StimerVal2 = am_hal_stimer_counter_get();

#if (defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P) || defined(AM_PART_APOLLO4))
  am_hal_gpio_state_write(24, AM_HAL_GPIO_OUTPUT_SET);
  am_hal_gpio_state_write(24, AM_HAL_GPIO_OUTPUT_CLEAR);
#else
  am_hal_gpio_out_bit_set(24);
  am_hal_gpio_out_bit_clear(24);
#endif

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
am_widget_stimer_capture_test(uint32_t ui32Speed)
{
  uint32_t StimerVal;
  uint32_t StimerVal2;
  uint32_t StimerDelta;

  uint32_t ui32GPIONumber = 0;
  uint32_t ui32CaptureNum = 1;
  bool bPolarity = true;          // Capture on high to low GPIO transition

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
  am_hal_stimer_int_clear(AM_HAL_STIMER_INT_CAPTUREA << ui32CaptureNum);

  //
  // Set up a GPIO to use as a trigger.
  //
#if (defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P) || defined(AM_PART_APOLLO4))
  am_hal_gpio_pinconfig(ui32GPIONumber, g_AM_HAL_GPIO_PIN_0);
  am_hal_gpio_state_write(ui32GPIONumber, AM_HAL_GPIO_OUTPUT_SET);
#else
  am_hal_gpio_pin_config(ui32GPIONumber, AM_HAL_GPIO_OUTPUT | AM_HAL_GPIO_INPUT);
  am_hal_gpio_out_bit_set(ui32GPIONumber);
#endif
  
  //
  // Set up a capture.
  //
  am_hal_stimer_capture_start(ui32CaptureNum, ui32GPIONumber, bPolarity);

  //
  // Start the timer.
  //
  StimerVal = am_hal_stimer_counter_get();
  am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_RUN);

  //
  // Delay for a pre-determined amount of time.
  //
#if (defined(AM_PART_APOLLO4))
  am_hal_delay_us(1000000);     // 1s
#else
  am_hal_flash_delay(FLASH_CYCLES_US(1000000));     // 1s
#endif
  
#if 0   // Used only for measuring the delay
  am_hal_gpio_out_bit_clear(ui32GPIONumber);
  am_hal_gpio_out_bit_set(ui32GPIONumber);
#endif

  //
  // High to low transition to trigger the capture.
  //
  am_hal_gpio_state_write(ui32GPIONumber, AM_HAL_GPIO_OUTPUT_CLEAR);
  am_hal_gpio_pinconfig(ui32GPIONumber, g_AM_HAL_GPIO_PIN_0);

  //
  // Make sure the capture event has occurred so that the read of the
  // capture value is valid.
  //
  while ( !(am_hal_stimer_int_status_get(false) &
            (AM_HAL_STIMER_INT_CAPTUREA << ui32CaptureNum)) );

  //
  // Get the capture value and stop further capturing.
  //
  StimerVal2 = am_hal_stimer_capture_get(ui32CaptureNum);
  am_hal_stimer_capture_stop(ui32CaptureNum);

  //
  // Since this is measuring a 100ms delay (0.1s), return 10x the delta
  // which should be close to the original frequency.
  //
  StimerDelta = StimerVal2 - StimerVal;
  am_util_stdio_printf("\nCapture Test at %d speed. Delta value is %d\n",ui32Speed,StimerDelta);
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
  uint32_t ui32CompValue;
  uint32_t StimerDelta;

  uint32_t ui32CmprInstance = 2;
  uint32_t ui32Delta = 170;

  //
  // Start the STIMER.
  //
  am_hal_stimer_config(ui32Speed | AM_HAL_STIMER_CFG_RUN);

#if (defined(AM_PART_APOLLO4))
  am_hal_delay_us(3000);     // 3ms
#else
  am_hal_flash_delay(FLASH_CYCLES_US(3000));     // 3ms
#endif

  //
  // Save the STIMER value.
  //
  ui32StimerVal = am_hal_stimer_counter_get();

  //
  // Set a compare value for the future, and check to see what the absolute
  // value of the compare register gets set to.
  //
  am_hal_stimer_compare_delta_set(ui32CmprInstance, ui32Delta);
  ui32CompValue = am_hal_stimer_compare_get(ui32CmprInstance);

  //
  // If all went well, this value should be zero (or close to it).
  //
  // Note - for Apollo3, the delta will be somewhat larger than 0
  //  due to increased read access cycle times across the APB.
  //
  StimerDelta = (ui32CompValue - ui32StimerVal - ui32Delta);
  am_util_stdio_printf("\nCompare Test at %d speed. Delta value is %d\n",ui32Speed,StimerDelta);
  return StimerDelta;
}
