//*****************************************************************************
//
//! @file usb_disconnect.c
//!
//! @brief tinyusb cdc- disconnect example.
//!
//! This example demonstrates how to use disconnect and reconnect
//! (USB V bus voltage lost and restore) to shutdown / restart the usb module
//! This specifically contains code for helper functions that
//! simulate power loss/restore on boards that don't have detection capability,
//! it is intended to be used as an example and template.
//!
//! This also contains code to place events or callback functions on the usb event queue.
//! In this way events detected in an Isr or another thread are queued and subsequently called
//! from the tinyusb thread (or in the background when no RTOS is used).
//!
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "usb_disconnect.h"
#include "am_devices_button.h"
#include "am_bsp.h"
#include "dcd_apollo4.h"
#include "device/usbd_pvt.h"
#include "tusb.h"

//!
//! button press definitions
//!
typedef enum
{
    eNO_BP          = 0,
    eBP1Conn        = 0x01,        ///< used for Connection
    eBP2Disconn     = 0x02,
    eBOTH           = (eBP1Conn | eBP2Disconn),
    eBPX32          = 0x7FFFFFF,  ///< make the enum 32 bit for all compilers

}
usb_disconn_button_press_e;

static usb_disconn_button_press_e scanButton(void);
static void usb_disconn_pwrUpCB(void *);
static void usb_disconn_pwrDownCB(void *);
static void usb_disconnect_callPowerUp(bool bIsIsr);
static void usb_disconnect_callPowerDown(bool bIsIsr);

#if defined(AM_BSP_NUM_BUTTONS) && AM_BSP_NUM_BUTTONS >= 1
static am_devices_button_pin_cfg_t am_bsp_psButtonsInfoLocal[AM_BSP_NUM_BUTTONS] ;
#endif

typedef enum
{
    eUSBIntialized,
    eUSBDisconnected,
    eUSBReconnected
} usb_enable_state;
static usb_enable_state g_eUsbEnableState = eUSBIntialized;

//*****************************************************************************
//
//! @brief scan buttons for button presses
//! this is called peridoically to scan buttons
//! @return button press status
//
//*****************************************************************************
static usb_disconn_button_press_e scanButton(void)
{
    //
    // init the button array, read the buttons, then de-init
    //
    usb_disconn_button_press_e ebp = eNO_BP;

#if defined(AM_BSP_NUM_BUTTONS) && (AM_BSP_NUM_BUTTONS >= 1)
    int numButtons = AM_BSP_NUM_BUTTONS >= 2 ? 2 : AM_BSP_NUM_BUTTONS;

    am_devices_button_array_pin_config(am_bsp_psButtonsInfoLocal, numButtons);

    for (int i = 0; i < numButtons; i++)
    {
        if (am_bsp_psButtonsInfoLocal[i].sDevButton.bChanged &&
            am_bsp_psButtonsInfoLocal[i].sDevButton.bPressed)
        {
            ebp |= (1 << i);
        }
    }
#endif
    return ebp;
}

//*****************************************************************************
//
// @brief called to setup code in this file
//
//*****************************************************************************
void usb_disconnect_init(void)
{

    //
    // this button array is defined in some of the newer BSPs, but not in many, so
    // define the array here in ram
    //
#if defined(AM_BSP_NUM_BUTTONS) && AM_BSP_NUM_BUTTONS >= 1

    am_bsp_psButtonsInfoLocal[0].sDevButton.ui32GPIONumber = am_bsp_psButtons[0].ui32GPIONumber ;
    am_bsp_psButtonsInfoLocal[0].sDevButton.ui32Polarity   = am_bsp_psButtons[0].ui32Polarity ;
    am_bsp_psButtonsInfoLocal[0].tPinCfg                   = &g_AM_BSP_GPIO_BUTTON0 ;

#if ( AM_BSP_NUM_BUTTONS >= 2 )
    am_bsp_psButtonsInfoLocal[1].sDevButton.ui32GPIONumber = am_bsp_psButtons[1].ui32GPIONumber ;
    am_bsp_psButtonsInfoLocal[1].sDevButton.ui32Polarity   = am_bsp_psButtons[1].ui32Polarity ;
    am_bsp_psButtonsInfoLocal[1].tPinCfg                   = &g_AM_BSP_GPIO_BUTTON1 ;
#endif
#endif
}

//*****************************************************************************
//
// @brief periodic button scan mgr, this is used to simulate a usb disconnect
// @details this is called periodically to scan the buttons and
// initiate action when pressed
// @note, one way to use, disconnect the usb connector, then push the button assigned to
// the disconnect simulation
//
//*****************************************************************************
void usb_disconnect_periodic(bool calledFromIsrOrThread)
{
    usb_disconn_button_press_e ebp = scanButton();
    if ((ebp & eBP1Conn) && (g_eUsbEnableState == eUSBDisconnected))
    {
        //
        // simulate connection detection
        //
        g_eUsbEnableState = eUSBReconnected;
        am_util_stdio_printf("bp1 power up\n");
        usb_disconnect_callPowerUp(calledFromIsrOrThread);
    }
    else if ((ebp & eBP2Disconn) && (g_eUsbEnableState != eUSBDisconnected))
    {
        //
        // simulate dis-connection detection
        //
        g_eUsbEnableState = eUSBDisconnected;
        am_util_stdio_printf("bp2 power down\n");
        tud_disconnect();
        usb_disconnect_callPowerDown(calledFromIsrOrThread);
    }
}

//*****************************************************************************
//
//! @brief intended to be called on power-up/reconnect event
//!
//! @details This function could be backtground or isr level call.
//! The subsequent callback function is called in background (not isr level)
//!
//! @note this is used to demonstrate a method to use the tinyusb code to preform
//! a background mode callback from an ISR call
//!
//! @param isIsr
//
//*****************************************************************************
void
usb_disconnect_callPowerUp(bool bIsIsr)
{
    //
    // issue a powerup event callback
    //
    usbd_defer_func(usb_disconn_pwrUpCB, 0, bIsIsr);
}
//*****************************************************************************
//
//! @brief intended to be called on power up down
//!
//! @details This function could be backtground or isr level call.
//! The subsequent callback function is called in background (not isr level)
//!
//! @note this is used to demonstrate a method to use the tinyusb code to preform
//! a background mode callback from an ISR call
//!
//! @param isIsr
//
//*****************************************************************************
void
usb_disconnect_callPowerDown(bool bIsIsr)
{
    //
    // issue an unplugged event
    //
    dcd_event_bus_signal(0, DCD_EVENT_UNPLUGGED, bIsIsr);

    //
    // create a power down event callback
    //
    usbd_defer_func(usb_disconn_pwrDownCB, 0, bIsIsr);
}

//*****************************************************************************
//
//! @brief callback called from tinyusb tud_task code on powerup (reconnect)
//!
//! @param x unused
//
//*****************************************************************************
static void
usb_disconn_pwrUpCB(void *x)
{
    (void) x;

    am_util_stdio_printf("call dcd PU\n");
    dcd_powerup(0, false);
    tud_connect();
}
//*****************************************************************************
//
//! @brief callback called from tinyusb code on powerloss (shutdown)
//! @note this is called from the USB task in response to an Unplugged event,
//! the Unplugged event was issues above in the usb_disconnect_callPowerDown call
//! This function should only be called from the tusb_task. It is called in the
//! background or in the tud_task(if rtos).
//!
//! @param x unused
//
//*****************************************************************************
static void
usb_disconn_pwrDownCB(void *x)
{
    (void) x;

    am_util_stdio_printf("call dcd PD\n");
    dcd_powerdown(0, false);
}

