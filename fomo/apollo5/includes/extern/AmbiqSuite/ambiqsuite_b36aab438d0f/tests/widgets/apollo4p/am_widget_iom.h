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
#ifndef AM_WIDGET_IOM_H
#define AM_WIDGET_IOM_H

#include "am_devices_spipsram.h"
#include "am_devices_mb85rc256v.h"
#if MB85RS1MT
#include "am_devices_mb85rs1mt.h"
#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_SPI_DEVICE
#define IOM_WIDGET_DEVICE_ID          AM_DEVICES_MB85RS1MT_ID
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_MB85RS1MT_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_devices_mb85rs1mt_config_t
#elif MB85RQ4ML
#include "am_devices_mb85rq4ml.h"
#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_SPI_DEVICE
#define IOM_WIDGET_DEVICE_ID          AM_DEVICES_MB85RQ4ML_ID
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_MB85RQ4ML_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_devices_mb85rq4ml_config_t
#elif MB85RS64V
#include "am_devices_mb85rs64v.h"
#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_SPI_DEVICE
#define IOM_WIDGET_DEVICE_ID          AM_DEVICES_MB85RS64V_ID
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_MB85RS64V_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_devices_mb85rs64v_config_t
#elif MB85RC256V
#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_I2C_DEVICE
#define IOM_WIDGET_DEVICE_ID          AM_DEVICES_MB85RC256V_ID
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_MB85RC256V_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_devices_mb85rc256v_config_t
#elif MB85RC64TA
#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_I2C_DEVICE
#if defined(apollo4p_eb)
#define IOM_WIDGET_DEVICE_ID          AM_DEVICES_MB85RC1MT_ID
#else
#define IOM_WIDGET_DEVICE_ID          AM_DEVICES_MB85RC64TA_ID
#endif
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_MB85RC256V_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_devices_mb85rc256v_config_t
#elif APS6404L
#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_SPI_DEVICE
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_SPIPSRAM_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_devices_spipsram_config_t
#elif EM9304
#include "am_devices_em9304.h"
#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_SPI_DEVICE
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_EM9304_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_devices_em9304_config_t
#elif LOOP
#include "am_devices_em9304.h"
#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_SPI_DEVICE
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_LOOP_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_devices_loop_config_t
#elif AMBT53_SCPM
#include "am_devices_ambt53.h"
#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_I2C_DEVICE
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_SCPM_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_devices_scpm_config_t
#elif TSL2540
#include "am_devices_tsl2540.h"
#include "am_devices_tma525.h"
#define IOM_WIDGET_DEVICE_MODE        AM_WIDGET_IOM_I2C_DEVICE
#define IOM_WIDGET_DEVICE_ID          AM_DEVICES_TSL2540_ID
#define IOM_WIDGET_DEVICE_SUCCESS     AM_DEVICES_TSL2540_STATUS_SUCCESS
#define am_widget_iom_devices_t       am_hal_iom_config_t
#else
// No devices
#define am_widget_iom_devices_t       am_devices_mb85rc256v_config_t
#endif

// Whether to put IOM in low power mode between the tests
#ifndef IOM_TEST_POWER_SAVE_RESTORE
#define IOM_TEST_POWER_SAVE_RESTORE
#endif

// Whether to put IOM in low power mode between the tests
#ifndef IOM_TEST_POWER_IOM_ENABLE_DISABLE
//#define IOM_TEST_POWER_IOM_ENABLE_DISABLE
#endif

#ifndef IOM_TEST_POWER_SAVE_RESTORE_SLEEP
//#define IOM_TEST_POWER_SAVE_RESTORE_SLEEP
#endif

#ifndef IOM_TEST_POWERSAVE_MODE
// Low power mode for IOM
#define IOM_TEST_POWERSAVE_MODE AM_HAL_SYSCTRL_DEEPSLEEP
#endif

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
#define AM_DEVICES_LOOP_MAX_DEVICE_NUM    1
typedef enum
{
    AM_DEVICES_LOOP_STATUS_SUCCESS,
    AM_DEVICES_LOOP_STATUS_ERROR
} am_devices_loop_status_t;

typedef struct
{
    uint32_t                    ui32Module;
    uint32_t                    ui32CS;
    void                        *pIomHandle;
    bool                        bOccupied;
} am_devices_iom_loop_t;

typedef struct
{
    uint32_t ui32ClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
} am_devices_loop_config_t;

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
    AM_WIDGET_DEVICE_EM9304,        // EM9304 (for duplex testing)
    AM_WIDGET_DEVICE_PSRAM,         // PSRAM
    AM_WIDGET_DEVICE_SCPM,          // SCPM
    AM_WIDGET_DEVICE_TSL2540,       // I2C ALS Device
    AM_WIDGET_DEVICE_TMA525,        // I2C Touch Device
} am_widget_iom_device_t;

typedef struct
{
    uint32_t              ui32Module;
    uint32_t              ui32DeviceType;
    am_widget_iom_devices_t   IOMConfig;
} am_widget_iom_config_t;

typedef struct
{
    uint32_t                      ui32BufferSize;
    uint32_t                      ui32ByteOffset;
    uint32_t                      ui32QueueSize; // Relevant only for am_widget_iom_test_queue_write_read
    am_hal_sysctrl_power_state_e  sleepMode; // Relevant only for nonblocking tests
} am_widget_iom_test_t;

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
extern uint32_t am_widget_iom_test_blocking_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_nonblocking_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
extern uint32_t am_widget_iom_test_queue_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
#if defined(IOM_SUPPORTS_FULLDUPLEX)
extern uint32_t am_widget_iom_test_duplex(void *pWidget, void *pTestCfg, char *pErrStr);
#endif
extern uint32_t am_widget_iom_power_saverestore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode);
extern uint32_t am_widget_iom_test_blocking_write_read_error(void *pWidget, void *pCfg, char *pErrStr, uint32_t errorMask);
extern uint32_t am_widget_iom_test_queue_write_read_error(void *pWidget, void *pCfg, char *pErrStr, uint32_t errorMask);

extern void            *g_IomDevHdl;
extern void            *g_pIOMHandle;

#endif // AM_WIDGET_IOM_H
