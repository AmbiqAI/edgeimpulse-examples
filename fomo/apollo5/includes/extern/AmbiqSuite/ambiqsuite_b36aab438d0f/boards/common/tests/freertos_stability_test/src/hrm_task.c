//*****************************************************************************
//
//! @file hrm_task.c
//!
//! @brief Task to simulate heart rate monitor sensor I/O operation using a
//!        SPI FRAM device.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Standard AmbiqSuite includes.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_devices.h"
#include "am_util.h"

//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

//*****************************************************************************
//
// Stability Test include files.
//
//*****************************************************************************
#include "hrm_task.h"
#include "freertos_stability_test.h"

//*****************************************************************************
//
// Defines.
//
//*****************************************************************************
//! Input buffer set to this value when empty.
#define INBUFFER_EMPTY      (0xEE)
//! Output buffer set to this value when empty.
#define OUTBUFFER_EMPTY     (0xED)

//*****************************************************************************
//
// HRM task handle.
//
//*****************************************************************************
TaskHandle_t HRMTaskHandle;

//*****************************************************************************
//
// Handle for HRM Task related events.
//
//*****************************************************************************
EventGroupHandle_t xHRMEventHandle;

//*****************************************************************************
//
// Global Variables.
//
//*****************************************************************************
//
// Transfer buffers
//
static unsigned char g_pucInBuffer[HRM_TRANSFER_SIZE];
static unsigned char g_pucOutBuffer[HRM_TRANSFER_SIZE];

//
// IOM Configuration
//
am_hal_iom_config_t HRM_IOM_Config =
{
  .ui32InterfaceMode = AM_HAL_IOM_SPIMODE,
  .ui32ClockFrequency = AM_HAL_IOM_4MHZ,
  .bSPHA = 0,
  .bSPOL = 0,
  .ui8WriteThreshold = 60,
  .ui8ReadThreshold = 100
};

//*****************************************************************************
//
// Interrupt handler for the CTIMER
//
//*****************************************************************************
void
hrm_ctimer_handler(void)
{
  BaseType_t xHigherPriorityTaskWoken, xResult;

  xHigherPriorityTaskWoken = pdFALSE;

  //
  // Send event to the HRM task.
  //
  xResult = xEventGroupSetBitsFromISR(xHRMEventHandle, 1,
                                      &xHigherPriorityTaskWoken);
  //
  // If the HRM task is higher-priority than the context we're currently
  // running from, we should yield now and run the radio task.
  //
  if (xResult != pdFAIL)
  {
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }

}

//*****************************************************************************
//
// Initialize CTIMER for periodic interrupts.
//
//*****************************************************************************
static void
init_timer(void)
{
  //
  // Register the handler for the CTIMER interrupt.
  //
  am_hal_ctimer_int_register(HRM_CTIMER_INT, hrm_ctimer_handler);

  //
  // Enable the LFRC.
  //
  am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_LFRC);

  //
  // Start a timer to trigger the SPI FRAM read (10 Hz).
  //
  am_hal_ctimer_config_single(HRM_CTIMER, AM_HAL_CTIMER_TIMERA,
                              AM_HAL_CTIMER_HFRC_12KHZ |
                                AM_HAL_CTIMER_FN_REPEAT |
                                  AM_HAL_CTIMER_INT_ENABLE);

  am_hal_ctimer_int_enable(HRM_CTIMER_INT);

  am_hal_ctimer_period_set(HRM_CTIMER, AM_HAL_CTIMER_TIMERA, 1200, 0);


  //
  // Start the timer.
  //
  am_hal_ctimer_start(HRM_CTIMER, AM_HAL_CTIMER_TIMERA);

  am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);

  am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_CTIMER, configMAX_SYSCALL_INTERRUPT_PRIORITY);
}

//*****************************************************************************
//
// Initialize the IOM
//
//*****************************************************************************
static uint32_t hrm_iom_init(uint32_t ui32IOMModule, am_hal_iom_config_t *psIOMConfig)
{
  uint32_t status = 0;
  if (ui32IOMModule < AM_REG_IOMSTR_NUM_MODULES)
  {
    //
    // Enable power to IOM.
    //
    am_hal_iom_pwrctrl_enable(ui32IOMModule);

    //
    // Set the required configuration settings for the IOM.
    //
    am_hal_iom_config(ui32IOMModule, psIOMConfig);

    //
    // Setup the pins for SPI mode (BSP).
    //
    if (AM_HAL_IOM_SPIMODE == psIOMConfig->ui32InterfaceMode)
    {
      //
      // Set up IOM SPI pins. Attributes are set in am_bsp_gpio.h.
      //
      am_bsp_iom_spi_pins_enable(ui32IOMModule);

      //
      // Enable the chip-select and data-ready pin.
      //! @note You can enable pins in the HAL or BSP.
      //
      am_hal_gpio_pin_config(apollo2_iomce0[ui32IOMModule][0], apollo2_iomce0[ui32IOMModule][1]);
    }
    else
    {
      am_bsp_iom_i2c_pins_enable(ui32IOMModule);
    }

    //
    // Enable all interrupts to check for any errors
    //
    am_hal_iom_int_enable(ui32IOMModule, 0xFF);
    am_hal_interrupt_enable(iomaster_interrupt[ui32IOMModule]);
    am_hal_interrupt_master_enable();

    //
    // Enable the IOM
    //
    am_hal_iom_enable(ui32IOMModule);

  }
  else
  {
    status = 0xFFFFFFFF;
  }

  return status;
}

//*****************************************************************************
//
// Perform initial setup for the HRM task.
//
//*****************************************************************************
void
HRMTaskSetup(void)
{
  uint8_t  manufID = 0;
  uint16_t prodID = 0;


  //
  // Print the banner
  //
  am_util_debug_printf("HRM Task: setup\r\n");

  //
  // Create an event handle for our wake-up events.
  //
  xHRMEventHandle = xEventGroupCreate();

  //
  // Make sure we actually allocated space for the events we need.
  //
  while (xHRMEventHandle == NULL);

  //
  // Set up the task LED.
  //
  //am_devices_led_init(&am_bsp_psLEDs[HRM_LED]);

  //
  // Set up IOM for the SPI FRAM
  //
  hrm_iom_init(HRM_IOM, &HRM_IOM_Config);

  //
  // Get the SPI FRAM identifiers and validate
  //
  am_devices_spifram_mt_id(HRM_IOM, 0, &manufID, &prodID);
  if ((manufID != 0x04) && (prodID != 0x0302))
  {
    am_util_debug_printf("SPI FRAM Device not found to simulate HRM sensor\r\n");
  }

  //
  // Initialize the transfer buffers.
  //
  memset(g_pucInBuffer, INBUFFER_EMPTY, sizeof(g_pucInBuffer));
  memset(g_pucOutBuffer, OUTBUFFER_EMPTY, sizeof(g_pucOutBuffer));
  for (uint32_t i = 0; i < sizeof(g_pucOutBuffer); i++)
  {
    g_pucOutBuffer[i] = i & 0xFF;
    if ((i & 0x0F) == 0)
    {
      g_pucOutBuffer[i] = 0xFF;
    }
  }

  //
  // Write the buffer out to the SPI FRAM once at startup.
  //
  am_devices_spifram_mt_write_mode(HRM_IOM, 0, g_pucOutBuffer, 0, HRM_TRANSFER_SIZE, AM_HAL_IOM_MODE_NB, 0);

  //
  // Initialize the timer.
  //
  init_timer();


}

//*****************************************************************************
//
// HRM Task main.
//
//*****************************************************************************

void
HRMTask(void *pvParameters)
{
  bool bCompareOk;

  //
  // Print the banner
  //
  am_util_debug_printf("HRM Task: running\r\n");

  //
  // Run the task forever.
  //
  while (1)
  {
    //
    // Wait for an event to be posted to the SPI FRAM Event Handle.
    //
    xEventGroupWaitBits(xHRMEventHandle, 1, pdTRUE,
                        pdFALSE, portMAX_DELAY);

    //
    // Simulate external HRM sensor write/read using SPI FRAM
    //
    am_devices_spifram_mt_read_mode(HRM_IOM, 0, g_pucInBuffer, 0, HRM_TRANSFER_SIZE, AM_HAL_IOM_MODE_NB, 0);

    //
    // Check the read info and compare to the original
    //
    bCompareOk = true;
    for (uint32_t i = 0; i < sizeof(g_pucOutBuffer); i++)
    {
      if (g_pucOutBuffer[i] != g_pucInBuffer[i])
      {
        bCompareOk = false;
        break;
      }
    }

    //
    // If the original compares, then print a progress icon.
    //
    if (bCompareOk)
    {
      am_util_debug_printf(".");
//      am_devices_led_toggle(am_bsp_psLEDs,HRM_LED);
    }

  }
}