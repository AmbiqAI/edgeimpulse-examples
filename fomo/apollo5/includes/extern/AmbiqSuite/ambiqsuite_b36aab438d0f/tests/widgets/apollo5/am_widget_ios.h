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

#if defined(APOLLO5_FPGA)
#define TEST_IOM_MODULE     1
#else
#define TEST_IOM_MODULE     0
#endif

#define TEST_IOS_MODULE     0

#if (TEST_IOS_MODULE == 0)
#define lram_array am_hal_ios_pui8LRAM
#define IOS_ACC_IRQ IOSLAVEACC_IRQn
#define IOS_IRQ     IOSLAVE_IRQn
#define ios_isr     am_ioslave_ios_isr
#define ios_acc_isr am_ioslave_acc_isr
#if defined(AM_PART_APOLLO5B)
#define SLINT_GPIO                      1
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_GPIO
#else
#define SLINT_GPIO                      0
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_SLINT
#endif
#elif (TEST_IOS_MODULE == 1)
#define lram_array  am_hal_iosfd0_pui8LRAM
#define IOS_ACC_IRQ IOSLAVEFDACC0_IRQn
#define IOS_IRQ     IOSLAVEFD0_IRQn
#define ios_isr     am_ioslave_fd0_isr
#define ios_acc_isr am_ioslave_fd0_acc_isr
#define SLINT_GPIO                      0
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_SLFDINT
#elif (TEST_IOS_MODULE == 2)
#define lram_array am_hal_iosfd1_pui8LRAM
#define IOS_ACC_IRQ IOSLAVEFDACC1_IRQn
#define IOS_IRQ     IOSLAVEFD1_IRQn
#define ios_isr     am_ioslave_fd1_isr
#define ios_acc_isr am_ioslave_fd1_acc_isr
#define SLINT_GPIO                      0
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_SLFD1INT
#endif

#define HANDSHAKE_IOM_PIN        46 // GPIO used to connect the IOSINT to host side
#define HANDSHAKE_IOS_PIN        4

#if defined(AM_PART_APOLLO5B)
#define TEST_IOS_DMA
#endif

#if defined(APOLLO5_FPGA)
#define DMA_PACK_SIZE  (16 * 1024)
#else
#define DMA_PACK_SIZE  (64 * 1024)
#endif

typedef struct
{
    uint32_t            size;
    uint32_t            address; // address 0x7F will trigger FIFO
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
