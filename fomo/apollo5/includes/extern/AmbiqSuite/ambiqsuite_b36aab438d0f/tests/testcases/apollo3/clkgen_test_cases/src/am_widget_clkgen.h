//*****************************************************************************
//
//! @file am_widget_clkgen.h
//!
//! @brief CLKGEN test cases.
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

#ifndef AM_WIDGET_CLKGEN_H
#define AM_WIDGET_CLKGEN_H

typedef enum
{
    AM_WIDGET_SUCCESS,
    AM_WIDGET_ERROR
} am_widget_clkgen_status_t;



typedef struct
{
    uint32_t              ui32Module;
    uint32_t              ui32DeviceType;
//    am_hal_clkgen_config_t   IOMConfig;
} am_widget_clkgen_config_t;




#if 0
typedef enum
{
    AM_WIDGET_SUCCESS,
    AM_WIDGET_ERROR
} am_widget_iom_status_t;

typedef enum
{
    AM_WIDGET_IOM_BLOCKING,
    AM_WIDGET_IOM_NONBLOCKING
} am_widget_iom_mode_t;

typedef enum
{
    AM_WIDGET_DEVICE_MB85RS64V,     // SPI FRAM
    AM_WIDGET_DEVICE_MB85RC256V,    // I2C FRAM
    AM_WIDGET_DEVICE_EM9304         // EM9304 (for duplex testing)
} am_widget_iom_device_t;

typedef struct
{
    uint32_t              ui32Module;
    uint32_t              ui32DeviceType;
    am_hal_iom_config_t   IOMConfig;
} am_widget_iom_config_t;

typedef struct
{
    uint32_t                      ui32BufferSize;
    uint32_t                      ui32ByteOffset;
    uint32_t                      ui32QueueSize; // Relevant only for am_widget_iom_test_queue_write_read
    am_hal_sysctrl_power_state_e  sleepMode; // Relevant only for nonblocking tests
} am_widget_iom_test_t;

extern volatile uint32_t g_IOMInterruptStatus;
extern uint32_t am_widget_iom_test_setup(am_widget_iom_config_t *pTestCfg,
                             void **ppWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_cleanup(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_read_id(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_status_get(void *pWidget, char *pErrStr);
extern uint32_t am_widget_iom_test_blocking_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_nonblocking_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_queue_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_duplex(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_power_saverestore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode);
#endif


#endif // AM_WIDGET_CLKGEN_H

