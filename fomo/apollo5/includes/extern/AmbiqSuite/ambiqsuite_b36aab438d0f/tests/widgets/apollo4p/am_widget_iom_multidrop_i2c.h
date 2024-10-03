//*****************************************************************************
//
//! @file am_widget_iom_multidrop_i2c.h
//!
//! @brief Test widget for testing IOM channels by data transfer using spifram.
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
#ifndef AM_WIDGET_IOM_MULTIDROP_I2C_H
#define AM_WIDGET_IOM_MULTIDROP_I2C_H


#include "am_widget_iom.h"

#define AM_WIDGET_IOM_ERRMASK_NAK_DEVADDR       0x1
#define AM_WIDGET_IOM_ERRMASK_NAK_OFFSETSTART   0x2
#define AM_WIDGET_IOM_ERRMASK_NAK_OFFSETMID     0x4
#define AM_WIDGET_IOM_ERRMASK_NAK_CONT          0x8
#define AM_WIDGET_IOM_ERRMASK_DMA_START         0x10
#define AM_WIDGET_IOM_ERRMASK_DMA_MID           0x20

//*****************************************************************************
//
// Loop Device - dummy driver
//
//*****************************************************************************

//*****************************************************************************
//
// Loop Device - dummy driver
//
//*****************************************************************************
extern uint32_t am_devices_loop_init(uint32_t ui32Module, am_widget_iom_devices_t *pDevConfig,
                        void **ppHandle, void **ppIomHandle);
extern uint32_t am_devices_loop_term(void *pHandle);

extern volatile uint32_t g_IOMInterruptStatus;
extern uint32_t am_widget_iom_test_setup(am_widget_iom_config_t *pTestCfg,
                             void **ppWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_cleanup(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_read_id(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_status_get(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_nonblocking_read(void *pWidget, void *pCfg, char *pErrStr);

extern void            *g_IomDevHdl;
extern void            *g_pIOMHandle;

#endif // AM_WIDGET_IOM_H
