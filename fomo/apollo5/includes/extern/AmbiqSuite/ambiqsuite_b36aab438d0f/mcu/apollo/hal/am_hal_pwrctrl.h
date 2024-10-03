//*****************************************************************************
//
//  am_hal_pwrctrl.h
//! @file
//!
//! @brief Function stubs for accessing and configuring the PWR controller.
//!
//! @addtogroup pwrctrl1 Power Control
//! @ingroup apollo1hal
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
#ifndef AM_HAL_PWRCTRL_H
#define AM_HAL_PWRCTRL_H

//*****************************************************************************
//
// Peripheral enable bits for am_hal_pwrctrl_periph_enable/disable()
//
//*****************************************************************************
#define AM_HAL_PWRCTRL_ADC      AM_REG_PWRCTRL_DEVICEEN_ADC_EN
#define AM_HAL_PWRCTRL_IOM0     AM_REG_PWRCTRL_DEVICEEN_IO_MASTER0_EN
#define AM_HAL_PWRCTRL_IOM1     AM_REG_PWRCTRL_DEVICEEN_IO_MASTER1_EN
#define AM_HAL_PWRCTRL_IOM2     AM_REG_PWRCTRL_DEVICEEN_IO_MASTER2_EN
#define AM_HAL_PWRCTRL_IOM3     AM_REG_PWRCTRL_DEVICEEN_IO_MASTER3_EN
#define AM_HAL_PWRCTRL_IOM4     AM_REG_PWRCTRL_DEVICEEN_IO_MASTER4_EN
#define AM_HAL_PWRCTRL_IOM5     AM_REG_PWRCTRL_DEVICEEN_IO_MASTER5_EN
#define AM_HAL_PWRCTRL_IOS      AM_REG_PWRCTRL_DEVICEEN_IO_SLAVE_EN
#define AM_HAL_PWRCTRL_PDM      AM_REG_PWRCTRL_DEVICEEN_PDM_EN
#define AM_HAL_PWRCTRL_UART0    AM_REG_PWRCTRL_DEVICEEN_UART0_EN
#define AM_HAL_PWRCTRL_UART1    AM_REG_PWRCTRL_DEVICEEN_UART1_EN

//*****************************************************************************
//
// Macro to set the appropriate IOM peripheral when using
//  am_hal_pwrctrl_periph_enable()/disable().
// For Apollo2, the module argument must resolve to be a value from 0-5.
//
//*****************************************************************************
#define AM_HAL_PWRCTRL_IOM(module)

//*****************************************************************************
//
// Macro to set the appropriate UART peripheral when using
//  am_hal_pwrctrl_periph_enable()/disable().
// For Apollo2, the module argument must resolve to be a value from 0-1.
//
//*****************************************************************************
#define AM_HAL_PWRCTRL_UART(module)

// ##### INTERNAL BEGIN ######
#if 0
//*****************************************************************************
//
// Memory enable values for am_hal_pwrctrl_memory_enable()
//
//*****************************************************************************
#define AM_HAL_PWRCTRL_MEMEN_SRAM8K     AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM8K
#define AM_HAL_PWRCTRL_MEMEN_SRAM16K    AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM16K
#define AM_HAL_PWRCTRL_MEMEN_SRAM24K   (AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM16K | \
                                        AM_REG_PWRCTRL_MEMEN_SRAMEN_GROUP0_SRAM2)
#define AM_HAL_PWRCTRL_MEMEN_SRAM32K    AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM32K
#define AM_HAL_PWRCTRL_MEMEN_SRAM64K    AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM64K
#define AM_HAL_PWRCTRL_MEMEN_SRAM96K                    \
            (AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM64K    |   \
             AM_REG_PWRCTRL_MEMEN_SRAMEN_GROUP2)
#define AM_HAL_PWRCTRL_MEMEN_SRAM128K   AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM128K
#define AM_HAL_PWRCTRL_MEMEN_SRAM160K                   \
            (AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM128K   |   \
             AM_REG_PWRCTRL_MEMEN_SRAMEN_GROUP4)
#define AM_HAL_PWRCTRL_MEMEN_SRAM192K                   \
            (AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM128K   |   \
             AM_REG_PWRCTRL_MEMEN_SRAMEN_GROUP4     |   \
             AM_REG_PWRCTRL_MEMEN_SRAMEN_GROUP5)
#define AM_HAL_PWRCTRL_MEMEN_SRAM224K                   \
            (AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM128K   |   \
             AM_REG_PWRCTRL_MEMEN_SRAMEN_GROUP4     |   \
             AM_REG_PWRCTRL_MEMEN_SRAMEN_GROUP5     |   \
             AM_REG_PWRCTRL_MEMEN_SRAMEN_GROUP6)
#define AM_HAL_PWRCTRL_MEMEN_SRAM256K   AM_REG_PWRCTRL_MEMEN_SRAMEN_SRAM256K

#define AM_HAL_PWRCTRL_MEMEN_FLASH512K  AM_REG_PWRCTRL_MEMEN_FLASH0_EN
#define AM_HAL_PWRCTRL_MEMEN_FLASH1M                \
            (AM_REG_PWRCTRL_MEMEN_FLASH0_EN     |   \
             AM_REG_PWRCTRL_MEMEN_FLASH1_EN)
#define AM_HAL_PWRCTRL_MEMEN_CACHE                  \
            (AM_REG_PWRCTRL_MEMEN_CACHEB0_EN    |   \
             AM_REG_PWRCTRL_MEMEN_CACHEB2_EN)
#define AM_HAL_PWRCTRL_MEMEN_CACHE_DIS              \
            ~(AM_REG_PWRCTRL_MEMEN_CACHEB0_EN   |   \
              AM_REG_PWRCTRL_MEMEN_CACHEB2_EN)

//
// Power up all available memory devices (this is the default power up state)
//
#define AM_HAL_PWRCTRL_MEMEN_ALL                    \
            (AM_REG_PWRCTRL_MEMEN_SRAMEN_ALL    |   \
             AM_REG_PWRCTRL_MEMEN_FLASH0_EN     |   \
             AM_REG_PWRCTRL_MEMEN_FLASH1_EN     |   \
             AM_REG_PWRCTRL_MEMEN_CACHEB0_EN    |   \
             AM_REG_PWRCTRL_MEMEN_CACHEB2_EN)

//*****************************************************************************
//
// Memory PWRONSTATUS enable values for am_hal_pwrctrl_memory_enable()
//
//*****************************************************************************
#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_8K                  \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_16K                 \
           (AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM1_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M)

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_24K                 \
           (AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM2_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM1_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M)

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_32K                 \
           (AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM3_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM2_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM1_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M)

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_64K                 \
           (AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP1_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM3_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM2_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM1_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M)

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_96K                 \
           (AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP2_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP1_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM3_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM2_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM1_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M)

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_128K                \
           (AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP3_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP2_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP1_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM3_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM2_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM1_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M)

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_160K                \
           (AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP4_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP3_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP2_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP1_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM3_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM2_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM1_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M)

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_192K                \
           (AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP5_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP4_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP3_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP2_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP1_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM3_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM2_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM1_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M)

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_224K                \
           (AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP6_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP5_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP4_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP3_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP2_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP1_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM3_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM2_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM1_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M)

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_256K                \
           (AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP7_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP6_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP5_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP4_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP3_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP2_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP1_SRAM_M   |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM3_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM2_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM1_M  |   \
            AM_REG_PWRCTRL_PWRONSTATUS_PD_GRP0_SRAM0_M)

#define AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_ALL                 \
        AM_HAL_PWRCTRL_PWRONSTATUS_SRAM_256K
#endif
// ##### INTERNAL END ######

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
#define am_hal_pwrctrl_periph_enable(x)
#define am_hal_pwrctrl_periph_disable(x)
#define am_hal_pwrctrl_memory_enable(x)
#define am_hal_pwrctrl_bucks_enable         am_hal_mcuctrl_bucks_enable
#define am_hal_pwrctrl_bucks_disable        am_hal_mcuctrl_bucks_disable
#define am_hal_pwrctrl_low_power_init(x)

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_PWRCTRL_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
