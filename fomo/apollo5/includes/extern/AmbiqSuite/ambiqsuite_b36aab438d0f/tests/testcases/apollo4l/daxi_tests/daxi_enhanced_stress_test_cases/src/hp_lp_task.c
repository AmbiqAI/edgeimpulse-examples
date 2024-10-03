//*****************************************************************************
//
//! @file hp_lp_task.c
//!
//! @brief Task to handle hp lp switch operations.
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
#include "nemagfx_enhanced_stress_test.h"

extern const unsigned char w0_360x196_rgba4444[];

#define MAX_STRIDE (31)

#if defined(AM_PART_APOLLO4L)
#define EXTENDED_RAM_BUFFER_LOCATION ((void*)(PSRAM_XIP_BASE_ADDRESS + PSRAM_EXTENDED_RAM_OFFSET))
#else
#define EXTENDED_RAM_BUFFER_LOCATION ((void*)(0x10160000 + 64800))
#endif

#define MRAM_BUFFER_LOCATION (w0_360x196_rgba4444)
#define BUFFER_SIZE (1024*8)

#define TOGGLE_HP_LP_MODE
#define HP_LP_TASK_DEBUG_LOG

#define MAX_HP_LP_TASK_DELAY    (400*2)

#ifndef BAREMETAL
//*****************************************************************************
//
// HP LP switch task handle.
//
//*****************************************************************************
TaskHandle_t HPLPTaskHandle;

//*****************************************************************************
//
// Task function.
//
//*****************************************************************************
uint32_t checksum_calculate(const uint8_t* buffer, uint32_t stride)
{
    uint32_t i, j;
    uint32_t sum = 0;

    for ( i = 0; i<stride; i++ )
    {
        for ( j = i; j < BUFFER_SIZE; j += stride )
        {
            sum += buffer[j];
        }
    }

    return sum;
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
HPLPTask(void *pvParameters)
{
    uint32_t stride;
    uint32_t sum_mram;
    uint32_t sum_extended_ram;
    am_hal_pwrctrl_mcu_mode_e ePowerMode = AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE;
    char* mode_string;
    uint32_t randomDelay;
    
    am_util_stdio_printf("HP LP mode switch task start!\n");
#ifdef APOLLO4_FPGA
    // Turn on SIMOBUCK for meeting the status check requirement in am_hal_pwrctrl_mcu_mode_select in HAL.
    PWRCTRL->VRCTRL |= 0x00000001;
#endif
    mode_string = (ePowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) ? "HP" : "LP";
#if !defined(TOGGLE_HP_LP_MODE)
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
      TEST_FAIL();
      am_util_stdio_printf("HP_LP:Enter %s mode failed!\n", mode_string);
    }
    else
    {
      am_util_stdio_printf("HP_LP:%s mode entered.\n", mode_string);
    }
#endif

    while(1)
    {

#if !defined(HP_LP_TASK_DEBUG_LOG)
      am_util_stdio_printf("HP_LP\n");
#endif

#if defined(TOGGLE_HP_LP_MODE)
        mode_string = (ePowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) ? "HP" : "LP";

        if ( am_hal_pwrctrl_mcu_mode_select(ePowerMode) != AM_HAL_STATUS_SUCCESS )
        {
            TEST_FAIL();
            am_util_stdio_printf("HP_LP:Enter %s mode failed!\n", mode_string);
            break;
        }
        else
        {
            //am_util_stdio_printf("HP_LP:%s mode entered.\n", mode_string);
            am_util_stdio_printf("HP_LP \n");
        }
#endif
        //copy data from MRAM to extended_ram
        memcpy(EXTENDED_RAM_BUFFER_LOCATION, MRAM_BUFFER_LOCATION, BUFFER_SIZE);

        //calculate checksum in extended ram

        stride = rand() % MAX_STRIDE + 1;

        sum_extended_ram = checksum_calculate((uint8_t*)EXTENDED_RAM_BUFFER_LOCATION, stride);

#if defined(HP_LP_TASK_DEBUG_LOG)
        //am_util_stdio_printf("buffer:0x%8X, stride:%d, checksum:0x%8x.\n", EXTENDED_RAM_BUFFER_LOCATION, stride, sum_extended_ram);
#endif
        //calculate checksum in mram

        stride = rand() % MAX_STRIDE + 1;

        sum_mram = checksum_calculate((uint8_t*)MRAM_BUFFER_LOCATION, stride);

#if defined(HP_LP_TASK_DEBUG_LOG)
        //am_util_stdio_printf("buffer:0x%8X, stride:%d, checksum:0x%8x.\n", MRAM_BUFFER_LOCATION, stride, sum_mram);

        if ( sum_extended_ram != sum_mram )
        {
            TEST_FAIL();
            am_util_stdio_printf("checksum compare failed in %s mode!\n", mode_string);
        }
        else
        {
            //am_util_stdio_printf("HP_LP:checksum compare success in %s mode.\n", mode_string);
        }

#endif

        //switch mode
        if ( ePowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE )
        {
            ePowerMode = AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER;
        }
        else
        {
            ePowerMode = AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE;
        }

        srand(xTaskGetTickCount());
         
        //wait here, to put the mcu to sleep.
        randomDelay = rand() % MAX_HP_LP_TASK_DELAY;

        vTaskDelay(randomDelay);
        if ((eTaskGetState(GuiTaskHandle) == eDeleted) && (ePowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE)) // make sure mcu is back to low power mode
        {
            break;
        }
    }
    am_util_stdio_printf("\nHPLP task end!\n");

    vTaskDelete(NULL);
    while (1);
}
#endif
