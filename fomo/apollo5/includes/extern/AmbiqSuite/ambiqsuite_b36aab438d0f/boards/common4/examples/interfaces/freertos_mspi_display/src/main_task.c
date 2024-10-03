//*****************************************************************************
//
//! @file main_task.c
//!
//! @brief Task to handle main operation.
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
#include "freertos_mspi_display.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//extern const unsigned char g_ambiq_logo_bmp0[];
//extern const unsigned char g_ambiq_logo_bmp2[];
//uint8_t                    *img0 = (uint8_t *)g_ambiq_logo_bmp0;
//uint8_t                    *img1 = (uint8_t *)g_ambiq_logo_bmp2;

extern const uint8_t acambiq_logo[];
extern const uint8_t watch_background[];
uint8_t                    *img0 = (uint8_t *)acambiq_logo;
uint8_t                    *img1 = (uint8_t *)watch_background;

uint32_t        fb1;
uint32_t        fb2;

//*****************************************************************************
//
// Composition task handle.
//
//*****************************************************************************
TaskHandle_t main_task_handle;

//*****************************************************************************
//
// Handle for Compose-related events.
//
//*****************************************************************************
EventGroupHandle_t xMainEventHandle;

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void am_gpio0_405f_isr(void)
{
    //
    // Read and clear the GPIO interrupt status.
    //
    am_hal_gpio_mask_t IntStatus;
    uint32_t    ui32IntStatus;

    AM_CRITICAL_BEGIN
    am_hal_gpio_interrupt_irq_status_get(GPIO0_405F_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_405F_IRQn, ui32IntStatus);
    AM_CRITICAL_END
    am_hal_gpio_interrupt_service(GPIO0_405F_IRQn, ui32IntStatus);
}

//*****************************************************************************
//
// Perform initial setup for the main task.
//
//*****************************************************************************
void
MainTaskSetup(void)
{
    int iRet;

    am_util_debug_printf("MainTask: setup\r\n");
    //
    // Create an event handle for our wake-up events.
    //
    xMainEventHandle = xEventGroupCreate();

    //
    // Make sure we actually allocated space for the events we need.
    //
    while (xMainEventHandle == NULL);

    NVIC_SetPriority(DISPLAY_MSPI_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(GPIO0_405F_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);

    // Initialize the frame buffer pointers
    fb1 = (uint32_t)img0;
    fb2 = (uint32_t)img1;

    // Initialize the MSPI Display

    iRet = display_init();
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize Display\n");
        while(1);
    }

    am_hal_interrupt_master_enable();

}

typedef enum
{
    DISPLAY_IDLE,
    DISPLAY_SCHEDULED,
    DISPLAY_STARTED
} display_state_e;

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
MainTask(void *pvParameters)
{
    uint32_t eventMask;
    display_state_e displayState = DISPLAY_IDLE;

    while (1)
    {
        //
        // Wait for an event to be posted to the Radio Event Handle.
        //
        eventMask = xEventGroupWaitBits(xMainEventHandle, 0xF, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        if (eventMask != 0)
        {
            if (eventMask & MAIN_EVENT_DISPLAY_DONE)
            {
                displayState = DISPLAY_IDLE;
            }
            if ((displayState == DISPLAY_IDLE) && (eventMask & MAIN_EVENT_TE))
            {
                displayState = DISPLAY_STARTED;
                // Initiate new frame transfer
                xEventGroupSetBits(xRenderEventHandle, RENDER_EVENT_START_NEW_FRAME);
            }
        }
    }
}
