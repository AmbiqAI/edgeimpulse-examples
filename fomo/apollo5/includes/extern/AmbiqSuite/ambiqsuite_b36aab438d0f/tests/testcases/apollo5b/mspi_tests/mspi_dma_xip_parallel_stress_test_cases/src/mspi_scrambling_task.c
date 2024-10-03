//*****************************************************************************
//
//! @file mspi_scrambling_task.c
//!
//! @brief Task to handle mspi scrambling task operations with gui task.
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

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "mspi_dma_xip_parallel_stress_test_cases.h"

#if (MSPI_SCRAMBLING_TASK_ENABLE == 1)

#include "am_widget_mspi.h"
#include "mspi_test_common.h"

#define MSPI_SCARMBLING_TEST_SIZE 1024*64
#define MAX_MSPI_TASK_SCRAMBLING_TASK_DELAY    (100*2)
#define MSPI_SCAMBLING_LOOP_CNT 500


//*****************************************************************************
//
// mspi scrambling task handle.
//
//*****************************************************************************
TaskHandle_t MspiScramblingTaskHandle;

uint8_t ui8ScramblingRXBuffer[MSPI_SCARMBLING_TEST_SIZE];

static bool mspi_check_buffer(uint32_t ui32NumBytes, uint8_t *pRxBuffer, uint8_t *pTxBuffer)
{
  for (uint32_t i = 0; i < ui32NumBytes; i++)
  {
    if (pRxBuffer[i] != pTxBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, pRxBuffer[i], pTxBuffer[i]);
      return false;
    }
  }
  return true;
}

int
mspi_scrambling_test(void)
{
    uint32_t ui32Status;
    uint32_t ix;
    uint32_t *pu32Ptr1;

    if( ui8ScramblingRXBuffer == NULL )
    {
      return false;
    }

    pu32Ptr1 = (uint32_t *)(MSPI0_APERTURE_START_ADDR + PSRAM_XIPMM_ADDR_OFFSET);

    for (uint32_t ui32LoopNum = 0; ui32LoopNum < MSPI_SCAMBLING_LOOP_CNT; ui32LoopNum++)
    {
      memcpy((uint8_t *)(&ui8ScramblingRXBuffer), pu32Ptr1, MSPI_SCARMBLING_TEST_SIZE);
    }

    if(!mspi_check_buffer(MSPI_SCARMBLING_TEST_SIZE, (uint8_t *)(&ui8ScramblingRXBuffer), (uint8_t *)pu32Ptr1))
    {
      return false;
    }

    return true;
}

void
MspiScramblingTask(void *pvParameters)
{
    uint32_t randomDelay, ui32Status;

    uint8_t *pu8TxPtr = (uint8_t *)(MSPI0_APERTURE_START_ADDR + PSRAM_XIPMM_ADDR_OFFSET);
    for(uint32_t ix = 0; ix < MSPI_SCARMBLING_TEST_SIZE; ix++)
    {
      *pu8TxPtr++ = ix;
    }

    while (1)
    {
        //
        // Run MSPI scrambling memcopy test.
        //
        if ( mspi_scrambling_test() == false )
        {
            am_util_stdio_printf("mspi task scrambling test fail!");
        }
       
        srand(xTaskGetTickCount());
        randomDelay = rand() % MAX_MSPI_TASK_SCRAMBLING_TASK_DELAY;
        vTaskDelay(randomDelay);
    }
}

#endif
