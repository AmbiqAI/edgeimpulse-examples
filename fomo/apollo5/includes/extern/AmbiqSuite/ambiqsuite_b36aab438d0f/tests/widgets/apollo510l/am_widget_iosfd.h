//*****************************************************************************
//
//! @file am_widget_iosfd.h
//!
//! @brief Functions for using the IOSFD interface
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
#ifndef AM_WIDGET_IOSFD_H
#define AM_WIDGET_IOSFD_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SLINT_GPIO                      1
#if (SLINT_GPIO == 1)
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_GPIO
#else
#define SLINT_FUN_SEL                   AM_HAL_PIN_4_SLFDINT
#endif

#define HANDSHAKE_IOS_PIN        4
#define HANDSHAKE_IOM_PIN        46 // GPIO used to connect the IOSINT to host side

typedef struct
{
    uint32_t            size;
    uint8_t             address; // address 0x7F will trigger FIFO
} am_widget_iosfd_test_t;

uint32_t am_widget_fullduplex_slave_setup(void);

uint32_t am_widget_fullduplex_slave_cleanup(void);

uint32_t
am_widget_fullduplex_slave_test(void *pTestCfg);

#ifdef __cplusplus
}
#endif


#endif // AM_WIDGET_IOSFD_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

