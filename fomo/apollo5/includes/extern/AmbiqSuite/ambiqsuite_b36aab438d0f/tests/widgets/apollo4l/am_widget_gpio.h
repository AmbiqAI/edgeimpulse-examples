//*****************************************************************************
//
//! @file am_widget_gpio.h
//!
//! @brief
//!
//! @addtogroup hal Hardware Abstraction Layer (HAL)
//! @addtogroup ctimer Counter/Timer (CTIMER)
//! @ingroup hal
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
#ifndef AM_WIDGET_GPIO_H
#define AM_WIDGET_GPIO_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Macros to check return values.
//
//*****************************************************************************
//
// Macros to check for pinconfig errors.
//
#define CHKPINCFG(pin,ret)                                                          \
        if ( ret )                                                                  \
        {                                                                           \
            am_util_stdio_printf("GPIO %d: pinconfig error 0x%08x encountered.",    \
                                 pin, ret);                                         \
            /* TODO: Handle this error condition. */                                \
            while(1);                                                               \
        }                                                                           \
        else                                                                        \
        {                                                                           \
         /* am_util_stdio_printf("GPIO %d pinconfig success.", pin); */             \
        }                                                                           \
     /* am_util_stdio_printf("  CFG reg=0x%08X\n", *((uint32_t*)0x40010000 + pin)); */


#define CHKGPIOINTCTRL(chl,pin,ret)                                                 \
        {                                                                           \
        if ( ret )                                                                  \
        {                                                                           \
            am_util_stdio_printf("GPIO %d interrupt control failed.",               \
                                 pin, ret);                                         \
            /* TODO: Handle this error condition. */                                \
            while(1);                                                               \
        }                                                                           \
        else                                                                        \
        {                                                                           \
         /* am_util_stdio_printf("GPIO %d interrupt control success.", pin); */     \
        }                                                                           \
     /* uint32_t ugrp, *puaddr;                                                     \
        ugrp = pin / 32;                                                            \
        puaddr = (uint32_t*)0x400102C0 + ((chl & 1) * 16)+ (ugrp * 4);              \
        am_util_stdio_printf("  INTEN reg 0x%08X=0x%08X.\n",(uint32_t)puaddr,*puaddr); */ \
        }

#define TINT_LO2HI              0x00000001
#define TINT_LO2HI_OTHER_INT    0x00000002
#define TINT_HI2LO              0x00000004
#define TINT_HI2LO_OTHER_INT    0x00000008

extern uint32_t am_widget_gpio_int_check(uint32_t ui32PinTestIntrpt,
                                         uint32_t ui32PinDrive,
                                         am_hal_gpio_intdir_e eIntType,
                                         am_hal_gpio_int_channel_e eChl,
                                         uint8_t ui8LoopCount);



#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_GPIO_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
