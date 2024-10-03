//*****************************************************************************
//
//! @file am_widget_ble.h
//!
//! @brief BLE test widget.
//!
//! @addtogroup
//! @ingroup
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_WIDGET_BLE_H
#define AM_WIDGET_BLE_H

typedef struct
{
    uint32_t ui32Test;
}
am_widget_ble_config_t;

#ifdef __cplusplus
extern "C"
{
#endif

extern void am_widget_ble_setup(void **ppWidget, char *pErrStr);
extern void am_widget_ble_test_config(void *pWidget, am_hal_ble_config_t *pBleConfig);
extern void am_widget_ble_cleanup(void *pWidget, char *pErrStr);
extern bool am_widget_ble_test_status_check(void *pWidget, char *pErrStr);
extern bool am_widget_ble_test_interrupt_check(void *pWidget, char *pErrStr);
extern bool am_widget_ble_test_blocking_hci(void *pWidget, char *pErrStr);
extern bool am_widget_ble_test_nonblocking_hci(void *pWidget, char *pErrStr);

#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_BLE_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

