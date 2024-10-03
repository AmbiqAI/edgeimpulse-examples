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
#include "nemagfx_enhanced_stress_test.h"

#define MAX_CRYPTO_TASK_DELAY    (700 * 2)


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
        if (eTaskGetState(GuiTaskHandle) == eDeleted)
        {
            break;
        }
    }
    am_util_stdio_printf("\nCrypto task end!\n");

    vTaskDelete(NULL);
    while (1);
}

