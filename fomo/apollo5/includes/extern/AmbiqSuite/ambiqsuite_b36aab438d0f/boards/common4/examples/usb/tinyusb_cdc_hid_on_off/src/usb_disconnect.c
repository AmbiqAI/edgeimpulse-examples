//*****************************************************************************
//
//! @file usb_cdc_disconnect.c
//!
//! @brief tinyusb diconnect button
//!
//! This function programs the buttons to  disconnect and reconnect
//! (USB V bus voltage lost and restore) to shutdown / restart the usb module
//! This specifically contains code for helper functions that
//! will poll the buttons on an evb. These are used to simulate programmable actions
//! where the firmware wants to disable on USB profile and start another.
//! the example here is switching between three profiles
//!   CDC
//!   composite CDC
//!   composite CDC - HID
//!
//! when the back button (closet to edge) is pressed
//! usb will be disconnected and powered down
//! when the forward button is pressed
//! the next profile is selected and the usb is re-started causing a USB enumeration
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
#include "usb_descriptors.h"
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
    eNO_BP = 0,
    eBP1Conn = 0x01,        ///< used for Connection, sw1
    eBP2Disconn = 0x02,     ///< used for disconnect, sw2
    eBOTH = (eBP1Conn | eBP2Disconn),
    eBPX32 = 0x7FFFFFF,  ///< make the enum 32 bit for all compilers
}
usb_disconn_button_press_e;

static usb_disconn_button_press_e scan_button(void);
static void usb_disconn_power_up_callback(void *);
static void usb_disconn_power_down_cb(void *);
static void usb_disconnect_call_power_up(bool bIsIsr);
static void usb_disconnect_call_power_down(bool bIsIsr);

#if defined(AM_BSP_NUM_BUTTONS) && AM_BSP_NUM_BUTTONS >= 1
static am_devices_button_pin_cfg_t am_bsp_psButtonsInfoLocal[AM_BSP_NUM_BUTTONS];
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
//!
//! this is called periodically to scan buttons
//!
//! @return button press status
//
//*****************************************************************************
static usb_disconn_button_press_e
scan_button(void)
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
// @brief called to set up code in this file
// this gets the buttons set up
//
//*****************************************************************************
void
usb_disconnect_init(void)
{
    //
    // this button array is defined in some of the newer BSPs, but not in many, so
    // define the array here in ram
    //
#if defined(AM_BSP_NUM_BUTTONS) && AM_BSP_NUM_BUTTONS >= 1

    am_bsp_psButtonsInfoLocal[0].sDevButton.ui32GPIONumber = am_bsp_psButtons[0].ui32GPIONumber;
    am_bsp_psButtonsInfoLocal[0].sDevButton.ui32Polarity = am_bsp_psButtons[0].ui32Polarity;
    am_bsp_psButtonsInfoLocal[0].tPinCfg = &g_AM_BSP_GPIO_BUTTON0;

#if (AM_BSP_NUM_BUTTONS >= 2)
    am_bsp_psButtonsInfoLocal[1].sDevButton.ui32GPIONumber = am_bsp_psButtons[1].ui32GPIONumber;
    am_bsp_psButtonsInfoLocal[1].sDevButton.ui32Polarity = am_bsp_psButtons[1].ui32Polarity;
    am_bsp_psButtonsInfoLocal[1].tPinCfg = &g_AM_BSP_GPIO_BUTTON1;
#endif

#endif
    //
    // setup button scan
    // done in BSP, not needed here
    //
}

//*****************************************************************************
//
// periodic button scan mgr, this is used to simulate a usb disconnect
//
// this is called periodically to scan the buttons and
// initiate action when pressed
//
//*****************************************************************************
void
usb_disconnect_periodic(bool calledFromIsrOrThread)
{
    usb_disconn_button_press_e ebp = scan_button();

    if ((ebp & eBP1Conn) && (g_eUsbEnableState == eUSBDisconnected))
    {
        //
        // cause a USB restart
        //

        //
        // switch the USB profile
        //
        usb_desc_set_next_profile();

        g_eUsbEnableState = eUSBReconnected;
        usb_disconnect_call_power_up(calledFromIsrOrThread);
    }
    else if ((ebp & eBP2Disconn) && (g_eUsbEnableState != eUSBDisconnected))
    {
        //
        // cause USB dis-connection
        //
        g_eUsbEnableState = eUSBDisconnected;
        am_util_stdio_printf("bp2 power down\n");
        tud_disconnect();
        usb_disconnect_call_power_down(calledFromIsrOrThread);
    }
}

//*****************************************************************************
//
//! @brief intended to be called on powerUp/reconnect event
//!
//! @details This function could be a background or isr level call.
//! The callback function queued here is called in background (not isr level)
//!
//! @param isIsr
//
//*****************************************************************************
static void
usb_disconnect_call_power_up(bool bIsIsr)
{
    //
    // issue a powerup event callback
    //
    usbd_defer_func(usb_disconn_power_up_callback, 0, bIsIsr);
}

//*****************************************************************************
//
//! @brief intended to be called on power up down
//!
//! @details This function could be backtground or isr level call.
//! The subsequent callback function is called in background (not isr level)
//!
//! @note this is used to demonstrate a method to use the tinyusb code to preform
//! a background mode callback.
//!
//! @param isIsr
//
//*****************************************************************************
static void
usb_disconnect_call_power_down(bool bIsIsr)
{
    //
    // issue an unplugged event
    //
    dcd_event_bus_signal(0, DCD_EVENT_UNPLUGGED, bIsIsr);
    //
    // create a power down event callback
    //
    usbd_defer_func(usb_disconn_power_down_cb, 0, bIsIsr);
}

//*****************************************************************************
//
//! @brief callback called from tinyusb tud_task code on powerUp (reconnect)
//!
//! @param x unused
//
//*****************************************************************************
static void
usb_disconn_power_up_callback(void *x)
{
    (void) x;

    am_util_stdio_printf("call dcd PU\n");
    dcd_powerup(0, false);
    tud_connect();
}

//*****************************************************************************
//
//! @brief callback called from tinyusb code on powerdown (disconnect)
//!
//! @details this is called from the USB task in response to a powerDown event
//! the powerDown event was issued above in the usb_disconnect_call_power_down call
//! this will perform a graceful usb shutdown
//!
//! @note This function should only be called from the tusb_task. It is called in the
//! background or in the tud_task(if rtos).
//!
//! @param x unused
//
//*****************************************************************************
static void
usb_disconn_power_down_cb(void *x)
{
    (void) x;

    am_util_stdio_printf("call dcd PD\n");
    dcd_powerdown(0, false);
}

