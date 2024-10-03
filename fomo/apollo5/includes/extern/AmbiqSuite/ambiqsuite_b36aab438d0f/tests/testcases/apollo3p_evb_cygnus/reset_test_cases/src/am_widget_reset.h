//*****************************************************************************
//
//! @file am_widget_reset.h
//!
//! @brief Test widget for testing RESET.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_WIDGET_RESET_H
#define AM_WIDGET_RESET_H

typedef enum
{
    AM_WIDGET_SUCCESS,
    AM_WIDGET_ERROR
} am_widget_reset_status_t;

typedef struct
{
    uint32_t    ui32Module;
    uint32_t    ui32DeviceType;
} am_widget_reset_config_t;

typedef struct
{
    uint32_t    ui32Something;
} am_widget_reset_test_t;


//
// Widget function prototypes
//
//extern uint32_t am_widget_reset_first(void *pWidget, char *pErrStr);
extern uint32_t am_widget_reset_enable_int_test(void *pWidget, char *pErrStr);

#endif // AM_WIDGET_RESET_H
