//*****************************************************************************
//
//! @file am_widget_ios.h
//!
//! @brief Functions for using the IOS interface 
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
#ifndef AM_WIDGET_IOS_H
#define AM_WIDGET_IOS_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    uint32_t            size;
    uint8_t             address; // address 0x7F will trigger FIFO
} am_widget_iosspi_test_t;

typedef struct
{
    uint32_t            iomModule;
    am_hal_iom_config_t iomHalCfg;
} am_widget_iosspi_stimulus_iom_t;

typedef struct
{
    uint32_t            iosModule;
    am_hal_ios_config_t iosHalCfg;
} am_widget_iosspi_stimulus_ios_t;

typedef struct
{
    am_widget_iosspi_stimulus_ios_t stimulusCfgIos;
    am_widget_iosspi_stimulus_iom_t stimulusCfgIom;
} am_widget_iosspi_config_t;

uint32_t am_widget_iosspi_setup(am_widget_iosspi_config_t *pIosCfg, 
    void **ppWidget, char *pErrStr);


uint32_t am_widget_iosspi_cleanup(void *pWidget, char *pErrStr);

uint32_t
am_widget_iosspi_test(void *pWidget, void *pTestCfg, char *pErrStr);

extern volatile bool g_bTimeOut;

#ifdef __cplusplus
}
#endif


#endif // AM_WIDGET_IOS_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
