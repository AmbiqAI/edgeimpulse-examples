//*****************************************************************************
//
//! @file lv_ambiq_touch.c
//!
//! @brief APIs for touch feature on LVGL.
//!
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
#ifdef LV_AMBIQ_TOUCH_USED

#include "lv_ambiq_touch.h"

#include "am_bsp.h"
#include "am_mcu_apollo.h"
#include "am_devices_tma525.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define TP_IOM_MODULE              AM_BSP_TP_IOM_MODULE
#define TP_IOM_MODE                AM_HAL_IOM_I2C_MODE

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static void lv_ambiq_touch_handler(void *x);

static am_devices_tc_tma525_info_t g_sTouchInfo = {0,0,0,0,0,0,0,0,0,0,0,0,0,true,0,0,0};

volatile lv_point_t g_touch_x_y =
{
    .x = 0,
    .y = 0
};

//
// 2023/12/19
// Comment out for would cause multi define in am_devices_display_generic.c
//

//#define TOUCH_GPIO_IDX  GPIO_NUM2IDX(AM_BSP_GPIO_TOUCH_INT)

//If touch int shares the same GPIO group with TE, isr will be defined in generic device driver
//#if (GPIO_NUM2IDX(DISPLAY_TE_PIN) != GPIO_NUM2IDX(AM_BSP_GPIO_TOUCH_INT))

//#if TOUCH_GPIO_IDX == 0
//#define disp_touch_isr        am_gpio0_001f_isr
//#elif TOUCH_GPIO_IDX == 1
//#define disp_touch_isr        am_gpio0_203f_isr
//#elif TOUCH_GPIO_IDX == 2
//#define disp_touch_isr        am_gpio0_405f_isr
//#elif TOUCH_GPIO_IDX == 3
//#define disp_touch_isr        am_gpio0_607f_isr
//#endif

////
//// GPIO TE interrupts.
////
//static const IRQn_Type touch_interrupts[] =
//{
//    GPIO0_001F_IRQn,
//    GPIO0_203F_IRQn,
//    GPIO0_405F_IRQn,
//    GPIO0_607F_IRQn
//};
////*****************************************************************************
////
//// Interrupt handler for the GPIO pins.
////
////*****************************************************************************
//void
//disp_touch_isr(void)
//{
//    uint32_t ui32Status;
//    am_hal_gpio_interrupt_irq_status_get(touch_interrupts[TOUCH_GPIO_IDX], false, &ui32Status);
//    am_hal_gpio_interrupt_irq_clear(touch_interrupts[TOUCH_GPIO_IDX], ui32Status);
//    am_hal_gpio_interrupt_service(touch_interrupts[TOUCH_GPIO_IDX], ui32Status);
//}
//#endif
// comment out end

//*****************************************************************************
//
// Touch functions
//
//*****************************************************************************
static void lv_ambiq_touch_handler(void *x)
{
    am_devices_tma525_nonblocking_get_point((am_devices_tc_tma525_info_t *)&g_sTouchInfo);                       
}

void lv_ambiq_touch_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    /*Save the pressed coordinates and the state*/
    if(g_sTouchInfo.touch_released == true)
    {
        data->point.x = g_sTouchInfo.x0;
        data->point.y = g_sTouchInfo.y0;
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->point.x = g_sTouchInfo.x0;
        data->point.y = g_sTouchInfo.y0;
        data->state = LV_INDEV_STATE_PR;
    }
}

void lv_ambiq_touch_init(void)
{
    am_devices_tma525_init(AM_BSP_TP_IOM_MODULE, lv_ambiq_touch_handler, NULL);
}

#endif