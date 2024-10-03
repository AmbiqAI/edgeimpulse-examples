//*****************************************************************************
//
//! @file audio_task.c
//!
//! @brief Task to handle audio related operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>

//*****************************************************************************
//
// FreeRTOS include files.
//
//*****************************************************************************
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#include "am_util_debug.h"
#include "am_util_stdio.h"

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_hal_pwrctrl.h"

#include "message_manager.h"
#include "config_manager.h"

#include "acore_thread.h"
#include "acore_dbgio.h"
#include "acore_event.h"
#include "acore_dsp_ep_agent.h"
#include "adev.h"
#include "aust_mcps.h"

#include "pipeline.h"
#include "audio_task.h"

/*
 * MACRO DEFINITIONS
 ******************************************************************************
 */
#define MCPS_SYS_INTERVAL (10) // Seconds

/*
 * LOCAL FUNCTION DECLARATIONS
 ******************************************************************************
 */
static void audio_task(void *pvParameters);

//*****************************************************************************
//
// Local variable.
//
//*****************************************************************************
static metal_thread_t audio_task_handle;

void audio_pipeline_start(void)
{
    if (!pipeline_create())
    {
        am_util_debug_printf("audio_pipeline_start pipeline_create failed\n");
        return;
    }

    int ret = pipeline_set_state(ST_PLAYING);
    if (ret)
    {
        am_util_debug_printf("audio_pipeline_start playing failed %d\n", ret);
    }
    else
    {
        am_util_debug_printf("audio_pipeline_start playing successful\n");
    }
#ifdef AM_PART_APOLLO5A
    // Flush the log immediately.
    am_bsp_debug_itm_printf_flush();
#endif
}

void audio_pipeline_stop(void)
{
    pipeline_set_state(ST_STOP);

    pipeline_destroy();
}

static void audio_thread_notify(void)
{
    // am_util_debug_printf("audio_thread_notify\n");
    metal_thread_notify_give(&audio_task_handle);
}

#ifdef AAS_MCPS
static uint32_t audio_task_get_mcu_freq(void)
{
    uint32_t mcu_freq;
    am_hal_pwrctrl_mcu_mode_e eCurrentPowerMode;

    am_hal_pwrctrl_mcu_mode_status(&eCurrentPowerMode);
    if (eCurrentPowerMode == AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE)
    {
#if defined(AM_PART_APOLLO5A)
        mcu_freq = AM_HAL_CLKGEN_FREQ_HP192_MHZ;
#elif defined(AM_PART_APOLLO5B)
        mcu_freq = AM_HAL_CLKGEN_FREQ_HP250_MHZ;
#else
#warning "audio task: MUST define MCU type here."
        mcu_freq = AM_HAL_CLKGEN_FREQ_MAX_KHZ;
#endif
    }
    else
    {
        mcu_freq = AM_HAL_CLKGEN_FREQ_MAX_KHZ;
    }

    return mcu_freq;
}
#endif

void audio_task_setup(void)
{
    audio_event_init(audio_thread_notify);

    adev_init();

    acore_dsp_ep_agent_init();

    acore_dbgio_init(DBGIO_OFF);

    audio_message_init();

    audio_config_init();

#ifdef AAS_MCPS
    aust_mcps_init(audio_task_get_mcu_freq());

    aust_mcps_module_enable(MCPS_SYS, true, "SYS");

    aust_mcps_set_sys_update_interval(MCPS_SYS_INTERVAL);
#endif

    int ret = metal_thread_init(&audio_task_handle, audio_task, "audio task",
                                4 * 1024, 0, 4);
    if (ret)
    {
        am_util_debug_printf("metal_thread_init audio task failure %d\n", ret);
        return;
    }
}

//*****************************************************************************
//
// Task to handle audio messages.
//
//*****************************************************************************
static void audio_task(void *pvParameters)
{
    (void)pvParameters;

    am_util_debug_printf("audio task start\r\n");

    while (1)
    {
        // Process audio schedule when thread is awake
        audio_event_schedule();

        // Put audio task into sleep here
        metal_thread_notify_take();
    }
}
