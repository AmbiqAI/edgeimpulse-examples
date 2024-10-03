//*****************************************************************************
//
//! @file mve_task.c
//!
//! @brief Task to exercise Helium/MVE matrix multiplication
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "mve_workload_test_cases.h"
#include "mve_task.h"
#include "arm_nnfunctions.h"

//*****************************************************************************
//
// MVE task handle.
//
//*****************************************************************************
TaskHandle_t mve_task_handle;

//*****************************************************************************
//
// Global variables and templates
//
//*****************************************************************************
volatile bool g_bMveTaskComplete = false;
extern void fully_connected_arm_fully_connected_s8(void);
extern void fully_connected_w_zp_arm_fully_connected_s8(void);
extern void fully_connected_mve_0_arm_fully_connected_s8(void);
extern void fully_connected_mve_1_arm_fully_connected_s8(void);
extern void fully_connected_null_bias_0_arm_fully_connected_s8(void);
extern void fully_connected_out_activation_arm_fully_connected_s8(void);
extern void fully_connected_int16_arm_fully_connected_s16(void);
extern void fully_connected_int16_big_arm_fully_connected_s16(void);
extern void fc_int16_slow_arm_fully_connected_s16(void);

// PMU_PROFILING_EN
// Enable PMU profiling to gather metrics
// on Helium/MVE usage
//
// 0: disable, 1: enable
#define PMU_PROFILING_EN 1

// BIG_MATRIX_ONLY
// Execution of only the int16_big_arm_fully_connected_s16 (max usage) test
//
// 0: execute all fully connected tests
// 1: execute int16_big_arm_fully_connected_s16 only
#define BIG_MATRIX_ONLY 1

#define NUM_LOOPS 100 // number of times to loop the MVE code before each task delay

#if PMU_PROFILING_EN == 1
// PMU Profiling variables
am_util_pmu_config_t pmu_config;
am_util_pmu_profiling_t pmu_profiling;

// Readable strings describing PMU counter functions
char pmu_counter_config[8][50] =
    { {"PMU MVE Instruction"},
      {"Chain Counter"},
      {"PMU MVE Integer Instruction"},
      {"Chain Counter"},
      {"PMU MVE Load or Store Instruction"},
      {"Chain Counter"},
      {"PMU MVE Vector Reduction Instruction"},
      {"Chain Counter"} };

void
pmu_profiling_print(void)
{
    for (uint32_t i = 0; i < __PMU_NUM_EVENTCNT; i++)
    {
        am_util_stdio_printf("Event Counter %d: %s\n", i, &pmu_counter_config[i][0]);
    }
}

void
pmu_profiling_init(void)
{
    //
    // Select event types here
    //
    pmu_config.ui32Counters = VALID_PMU_COUNTERS;               // Enable all valid event counters
    pmu_config.ui32EventType[0] = ARM_PMU_MVE_INST_RETIRED;
    pmu_config.ui32EventType[1] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[2] = ARM_PMU_MVE_INT_RETIRED;
    pmu_config.ui32EventType[3] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[4] = ARM_PMU_MVE_LDST_RETIRED;
    pmu_config.ui32EventType[5] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[6] = ARM_PMU_MVE_VREDUCE_RETIRED;
    pmu_config.ui32EventType[7] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    am_util_pmu_init(&pmu_config);
    am_util_pmu_enable();

}
#endif

//*****************************************************************************
//
// Define dummy syscalls to avoid compiler warnings when building
// with GCC 13.
//
//*****************************************************************************
void _close(void)
{
}
void _fstat(void)
{
}
void _getpid(void)
{
}
void _isatty(void)
{
}
void _kill(void)
{
}
void _lseek(void)
{
}
void _read(void)
{
}
void _write(void)
{
}
//*****************************************************************************
//
// Perform initial setup for the hello task.
//
//*****************************************************************************
void
MveTaskSetup(void)
{
    am_util_stdio_printf("MVE task: setup\r\n");
#if PMU_PROFILING_EN == 1
    pmu_profiling_print();
#endif
}


//*****************************************************************************
//
// MveTask - will delete itself after MVE_TASK_COUNT loops complete
//
//*****************************************************************************

void
MveTask(void *pvParameters)
{
    uint32_t i = 0;
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS; // 1 second delay
    am_util_stdio_printf("\r\nRunning MveTask\r\n");

#if PMU_PROFILING_EN == 1
    pmu_profiling_init();
#endif

    while (i < MVE_TASK_COUNT)
    {
        // Toggle an observation GPIO
        am_hal_gpio_output_toggle(GPIO_MVE_TASK);
        // Print out number of iterations
        am_util_stdio_printf("\r\nMveTask Iteration: %d\r\n", i++);

        for (uint32_t j = 0; j < NUM_LOOPS; j++)
        {
#if BIG_MATRIX_ONLY == 1
            // Execute big matrix function 10 times
            for (uint32_t k = 0; k < 10; k++)
            {
                fully_connected_int16_big_arm_fully_connected_s16();
            }
#else
            // Cycle through all available fully connected tests
            fully_connected_arm_fully_connected_s8();
            fully_connected_w_zp_arm_fully_connected_s8();
            fully_connected_mve_0_arm_fully_connected_s8();
            fully_connected_mve_1_arm_fully_connected_s8();
            fully_connected_null_bias_0_arm_fully_connected_s8();
            fully_connected_out_activation_arm_fully_connected_s8();
            fully_connected_int16_arm_fully_connected_s16();
            fully_connected_int16_big_arm_fully_connected_s16();
            fc_int16_slow_arm_fully_connected_s16();
#endif
        }

#if PMU_PROFILING_EN == 1
        am_util_pmu_get_profiling(&pmu_config,&pmu_profiling);
#endif

        // Delay (block) the task for a specified period
        am_hal_gpio_output_toggle(GPIO_MVE_TASK);
        vTaskDelay(xDelay);
    }
#if PMU_PROFILING_EN == 1
    am_util_pmu_disable();
    am_util_pmu_deinit();
#endif
    // Indicate the HelloTask loops are complete
    g_bMveTaskComplete = true;
    // Suspend and delete the task
    vTaskDelete(NULL);
}
