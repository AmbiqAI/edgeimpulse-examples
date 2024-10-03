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

#define TEST_IOS_MODULE     0

// #define lram_array(instance)   am_hal_iosfd ##instance## _pui8LRAM
#if (TEST_IOS_MODULE == 0)
#define lram_array am_hal_iosfd0_pui8LRAM

#define SLINT_GPIO                      1
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_SLFDINT
#elif (TEST_IOS_MODULE == 1)
// #define lram_array am_hal_iosfd1_pui8LRAM
#define SLINT_GPIO                      0
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_SLFD1INT
#endif

#define HANDSHAKE_IOM_PIN        46 // GPIO used to connect the IOSINT to host side
#define HANDSHAKE_IOS_PIN        4

#if defined(APOLLO5_FPGA)
#define DMA_PACK_SIZE  (16 * 1024)
#else
#define DMA_PACK_SIZE  (64 * 1024)
#endif

#define TEST_IOS_DMA


typedef struct
{
    uint32_t            address; // address 0x7F will trigger FIFO
    uint32_t            size;

    bool                slint_enable;   // Configurable Slave Interrupt
    bool                dma
} am_widget_iosspi_test_t;

typedef struct
{
    uint8_t             directSize;
    uint8_t             roBase;
    uint8_t             roSize;
    uint8_t             fifoSize;
    uint8_t             ahbRamBase;
    uint8_t             ahbRamSize;
}larm_info_t;

typedef struct
{
    bool                bSpi;
    uint32_t            i2cAddr;

    uint32_t            iomModule;
    void                *pIOMHandle;
    am_hal_iom_config_t iomHalCfg;

    uint32_t            iosModule;
    void                *pIOSHandle;
    am_hal_ios_config_t iosHalCfg;

    larm_info_t         larm_info;

} am_widget_iosspi_config_t;

bool am_widget_iosspi_setup(am_widget_iosspi_config_t *pIosCfg, void **ppWidget, char *pErrStr);
bool am_widget_iosspi_cleanup(am_widget_iosspi_config_t *pIosCfg);


uint32_t
am_widget_iosspi_test(void *pWidget, void *pTestCfg, char *pErrStr);

uint32_t
am_widget_iosspi_dma_test(void *pWidget, void *pTestCfg, char *pErrStr);

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
