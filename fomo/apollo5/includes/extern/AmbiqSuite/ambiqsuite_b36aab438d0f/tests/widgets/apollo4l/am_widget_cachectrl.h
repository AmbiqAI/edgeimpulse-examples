//*****************************************************************************
//
//! @file am_widget_cachectrl.h
//!
//! @brief CACHECTRL test cases.
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

#ifndef AM_WIDGET_CACHECTRL_H
#define AM_WIDGET_CACHECTRL_H

typedef enum
{
    AM_WIDGET_SUCCESS,
    AM_WIDGET_ERROR
} am_widget_cachectrl_status_t;

typedef struct
{
    uint32_t              ui32Module;
    uint32_t              ui32DeviceType;
} am_widget_cachectrl_config_t;


#endif // AM_WIDGET_CACHECTRL_H

