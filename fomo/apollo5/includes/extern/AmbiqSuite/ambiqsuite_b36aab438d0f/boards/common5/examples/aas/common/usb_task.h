//*****************************************************************************
//
//! @file usb_task.h
//!
//! @brief Functions and variables related to the audio task.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef USB_TASK_H
#define USB_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

    void usb_task_setup(void);

    void usb_task_teardown(void);

#ifdef __cplusplus
}
#endif

#endif // USB_TASK_H
