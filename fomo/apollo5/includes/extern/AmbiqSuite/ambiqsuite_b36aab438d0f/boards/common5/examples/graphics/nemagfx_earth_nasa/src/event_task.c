//*****************************************************************************
//
//! @file Event_task.c
//!
//! @brief Task to handle timer and GPIO interrupt event
//! The task can schedule display task and render task through external interrupt 
//! event.
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
#include "nemagfx_earth_nasa.h"
#include "rtos.h"
#include "event_task.h"
#include "render_task.h"
#include "display_task.h"

// #define SUPPORT_LOG
#ifdef SUPPORT_LOG
#define APPLICATION_PRINT(...) am_util_stdio_printf(__VA_ARGS__)
#else
#define APPLICATION_PRINT(...)
#endif

//*****************************************************************************
//
// Event task handle.
//
//*****************************************************************************
TaskHandle_t EventTaskHandle;

void            *g_pCHSC5816Handle;
void            *g_pIOMCHSC5816Handle;

AM_SHARED_RW uint32_t DMATCBBuffer[DMABUFFSIZE];
am_devices_iom_chsc5816_config_t g_sI2cNBConfig =
{
    .ui32ClockFreq          = AM_HAL_IOM_400KHZ,
    .ui32NBTxnBufLength     = sizeof(DMATCBBuffer)/4,
    .pNBTxnBuf              = DMATCBBuffer
};

static bool g_bXferDone = false;
static am_devices_tc_chsc5816_info_t g_sIntTouchInfo = {0};
static am_devices_tc_chsc5816_info_t g_sPresentTouch = {0};
static am_devices_tc_chsc5816_info_t g_sPreviousTouch = {0};

//
// Take over the interrupt handler for whichever IOM we're using.
//
#define fram_iom_isr                                                          \
    am_iom_isr1(AM_BSP_TP_IOM_MODULE)
#define am_iom_isr1(n)                                                        \
    am_iom_isr(n)
#define am_iom_isr(n)                                                         \
    am_iomaster ## n ## _isr
//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
void
fram_iom_isr(void)
{
    uint32_t ui32Status;
    if (!am_hal_iom_interrupt_status_get(g_pIOMCHSC5816Handle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMCHSC5816Handle, ui32Status);
            am_hal_iom_interrupt_service(g_pIOMCHSC5816Handle, ui32Status);
        }
    }
}

void pfnCHSC5816_GETPOINT_Callback(void *pCallbackCtxt)
{
    g_bXferDone = true;
}

//*****************************************************************************
//
// Handler of touch gpio interrupt
//
//*****************************************************************************
static void touch_handler(void *x)
{
    am_devices_chsc5816_nonblocking_get_point(g_pCHSC5816Handle, &g_sIntTouchInfo, pfnCHSC5816_GETPOINT_Callback, NULL);
}

//*****************************************************************************
//
//! @brief Initialize touch feature
//!
//! This function initializes IOM and external interrupt to implement the touch
//! function of the panel.
//!
//! @return None.
//
//*****************************************************************************
static void
touch_init(void)
{
    if (g_sDispCfg.eIC == DISP_IC_CO5300)
    {
        am_devices_chsc5816_init(AM_BSP_TP_IOM_MODULE, 
                                &g_sI2cNBConfig, 
                                &g_pCHSC5816Handle, 
                                &g_pIOMCHSC5816Handle, 
                                AM_BSP_GPIO_TOUCH_INT, 
                                AM_BSP_GPIO_TOUCH_RST, 
                                touch_handler, 
                                NULL);
    }
    else
    {
        APPLICATION_PRINT("Error touch IC not supported!\n");
    }
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();
}

//*****************************************************************************
//
// Event Task.
//
//*****************************************************************************
void
EventTask(void *pvParameters)
{
    //
    // Initialize the display touch
    //
    touch_init();

    g_sIntTouchInfo.touch_released = true;

    while (1)
    {
        if (g_sIntTouchInfo.touch_released)
        {
            bDrawWire = true;
            angle_dx = 0;
            angle_dy = 3;
            //
            // Clean touch points
            //
            g_sPreviousTouch.finger_number = 0;
            g_sPresentTouch.finger_number = 0;
        }
        if (g_bXferDone)
        {
            g_bXferDone = false;
            //
            // Record present points
            //
            memcpy((void*)(&g_sPresentTouch), (const void*)&g_sIntTouchInfo, sizeof(g_sIntTouchInfo));
            if (g_sPreviousTouch.finger_number)
            {
                if (g_sPreviousTouch.finger_number == g_sPresentTouch.finger_number)
                {
                    bDrawWire = false;
                    if (g_sPreviousTouch.finger_number == 1)
                    {
                        angle_dy = -(g_sPresentTouch.x0 - g_sPreviousTouch.x0);
                        angle_dx = (g_sPresentTouch.y0 - g_sPreviousTouch.y0);
                    }
                    else if (g_sPreviousTouch.finger_number == 2)
                    {
                        trans_dz = (nema_sqrt( nema_pow(g_sPreviousTouch.x1 - g_sPreviousTouch.x0, 2) + nema_pow(g_sPreviousTouch.y1 - g_sPreviousTouch.y0, 2))
                                   - nema_sqrt( nema_pow(g_sPresentTouch.x1 - g_sPresentTouch.x0, 2) + nema_pow(g_sPresentTouch.y1 - g_sPresentTouch.y0, 2)));
                    }
                }
            }
            //
            // Record present as the previous points
            //
            memcpy((void*)(&g_sPreviousTouch), (const void*)&g_sPresentTouch, sizeof(g_sPresentTouch));
        }

    }
}
