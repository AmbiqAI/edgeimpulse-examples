//*****************************************************************************
//
//! @file am_widget_iom.h
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

#ifndef AM_WIDGET_IOMSPI_H
#define AM_WIDGET_IOMSPI_H

#if 0
typedef int (*am_widget_setup_func_t)(uint32_t id, void *pIomCfg, void **ppWidget, char *pErrStr);

typedef int (*am_widget_cleanup_func_t)(void *pWidget, char *pErrStr);

typedef int (*am_widget_test_func_t)(void *pWidget, void *pTestCfg, char *pErrStr);
#endif

typedef struct
{
    am_hal_iom_config_t     halCfg;
    AM_HAL_IOM_MODE_E       mode;
    
} am_widget_iom_config_t;

typedef struct
{
    uint32_t                size;
} am_widget_iom_test_t;

uint32_t am_widget_iom_setup(uint32_t iomModule, 
    am_widget_iom_config_t *pIomCfg, 
    void **ppWidget, char *pErrStr);


uint32_t am_widget_iom_cleanup(void *pWidget, char *pErrStr);

uint32_t
am_widget_iom_test(void *pWidget, void *pTestCfg, char *pErrStr);


#endif // AM_WIDGET_IOMSPI_H
