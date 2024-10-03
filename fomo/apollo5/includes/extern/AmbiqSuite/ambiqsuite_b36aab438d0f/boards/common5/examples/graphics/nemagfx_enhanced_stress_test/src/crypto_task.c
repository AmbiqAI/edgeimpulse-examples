//*****************************************************************************
//
//! @file crypto_task.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdlib.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "crc_task.h"
#include "crypto_operations.h"
#include "crypto_stress_test_config.h"
#include "crypto_task.h"
#include "task.h"

//
// The crypto task crypto_test_operation() takes ~5.5 seconds to complete.  
// During this time, the idle task will never be called and the CPU will never 
// go to sleep.  Set the maximum task delay to 2x the task duration to ensure 
// the chip spends enough time entering and exiting deepsleep
//
#define MAX_CRYPTO_TASK_DELAY    (5500 * 2)

TaskHandle_t CryptoTaskHandle;

//#define CRYPTO_TASK_DEBUG_LOG

void
crypto_task(void *pvParameters)
{
    uint32_t randomDelay;

#if defined(CRYPTO_TASK_DEBUG_LOG)
    static uint32_t u32_times;
#endif
    crypto_test_init();

    while(1)
    {
        crypto_test_operation();
#if defined(CRYPTO_TASK_DEBUG_LOG)
        am_util_stdio_printf("crypto_task runs %d times \r\n", ++u32_times);
#else
        am_util_stdio_printf("[TASK] : CRYPTO\n");
#endif
        srand(xTaskGetTickCount());
        randomDelay = rand() % MAX_CRYPTO_TASK_DELAY;
        vTaskDelay(randomDelay);
    }
}


