//*****************************************************************************
//
//  am_bsp.h
//! @file
//!
//! @brief Functions to aid with configuring the GPIOs.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo3_fpga_bsp BSP for the Apollo3 Hotshot FPGA
//! @ingroup BSP
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

#ifndef AM_BSP_H
#define AM_BSP_H

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp_pins.h"
#include "am_devices_led.h"
#include "am_devices_button.h"

#ifdef __cplusplus
extern "C"
{
#endif

// #### INTERNAL BEGIN ####
//
// Define MCU_VALIDATION for building tests for the validation group.
//
//#define MCU_VALIDATION  1
// #### INTERNAL END ####
//*****************************************************************************
//
// Print interface type
//
//*****************************************************************************
#define AM_BSP_UART_PRINT_INST  0
//*****************************************************************************
//
// PWM_LED peripheral assignments.
//
//*****************************************************************************
#define AM_BSP_PWM_LED_TIMER                0
#define AM_BSP_PWM_LED_TIMER_SEG            AM_HAL_CTIMER_TIMERB
#define AM_BSP_PWM_LED_TIMER_INT            AM_HAL_CTIMER_INT_TIMERB0

//*****************************************************************************
//
// Button definitions.
//
//*****************************************************************************
#define AM_BSP_NUM_BUTTONS                  0
#if AM_BSP_NUM_BUTTONS
extern am_devices_button_t am_bsp_psButtons[AM_BSP_NUM_BUTTONS];
#endif

//*****************************************************************************
//
// LED definitions.
//
//*****************************************************************************
#if 1   // FPGAs and EBs generally have no native LEDs.
#undef  AM_BSP_NUM_LEDS
#else
#define AM_BSP_NUM_LEDS                     4
extern am_devices_led_t am_bsp_psLEDs[AM_BSP_NUM_LEDS];
#endif

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// Memory for a faked printf application
//
//*****************************************************************************
typedef struct
{
    uint8_t *pui8Buffer;
    uint32_t ui32Size;
    uint32_t ui32Index;
} am_bsp_memory_printf_state_t;
// #### INTERNAL END ####

#define SDIO_BUS_WIDTH_1 1
#define SDIO_BUS_WIDTH_4 4
#define SDIO_BUS_WIDTH_8 8

//*****************************************************************************
//
// Display interface.
//
//*****************************************************************************
typedef enum
{
    DISP_IF_SPI4,
    DISP_IF_DSPI,
    DISP_IF_QSPI,
    DISP_IF_DSI,
} am_devices_disp_if_e;

//*****************************************************************************
//
// Display interface, resolution and flipping configurations.
//
//*****************************************************************************
typedef struct
{
    uint32_t ui32PanelResX;
    uint32_t ui32PanelResY;
    bool     bFlip;
    bool     bUseDPHYPLL;
    am_devices_disp_if_e        eInterface;
} am_devices_display_hw_config_t;

//*****************************************************************************
//
// Display types.
//
//*****************************************************************************
typedef enum
{
    RM67162_SPI4 = 0,
    RM67162_DSPI,
    RM67162_DSI,
    RM69330_DSPI,
    RM69330_QSPI,
    RM69330_DSI,
    BOE_DSI,
} am_bsp_display_type_e;

//*****************************************************************************
//
// Display interface, resolution and flipping configurations.
//
//*****************************************************************************
typedef struct
{
    am_hal_dsi_freq_trim_e  eDsiFreq;
    am_hal_dsi_dbi_width_e  eDbiWidth;
    uint8_t ui8NumLanes;
} am_bsp_dsi_config_t;

//*****************************************************************************
//
// External global variables.
//
//*****************************************************************************
extern am_bsp_display_type_e g_eDispType;
extern am_devices_display_hw_config_t g_sDispCfg[7];
extern am_bsp_dsi_config_t g_sDsiCfg;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void am_bsp_low_power_init(void);
extern void am_bsp_iom_pins_enable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode);
extern void am_bsp_iom_pins_disable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode);
extern void am_bsp_ios_pins_enable(uint32_t ui32Module, uint32_t ui32IOSMode);
extern void am_bsp_ios_pins_disable(uint32_t ui32Module, uint32_t ui32IOSMode);
extern void am_bsp_mspi_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);
extern void am_bsp_mspi_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice);

extern void am_bsp_sdio_pins_enable(uint8_t ui8BusWidth);
extern void am_bsp_sdio_pins_disable(uint8_t ui8BusWidth);

extern void am_bsp_debug_printf_enable(void);
extern void am_bsp_debug_printf_disable(void);

extern void am_bsp_itm_string_print(char *pcString);
extern void am_bsp_itm_printf_enable(void);
extern void am_bsp_itm_printf_disable(void);

extern void am_bsp_uart_string_print(char *pcString);

extern void am_bsp_uart_printf_enable(void);
extern void am_bsp_uart_printf_disable(void);

extern void am_bsp_buffered_uart_printf_enable(void);
extern void am_bsp_buffered_uart_printf_disable(void);
extern void am_bsp_buffered_uart_service(void);

// #### INTERNAL BEGIN ####
extern void am_bsp_memory_printf_enable(am_bsp_memory_printf_state_t *psPrintfState);
extern void am_bsp_memory_printf_disable(void);
// #### INTERNAL END ####

extern void am_bsp_disp_spi_pins_enable(void);
extern void am_bsp_disp_qspi_pins_enable(void);
extern void am_bsp_disp_dspi_pins_enable(void);
extern void am_bsp_disp_dsi_pins_enable(void);

extern void am_bsp_external_vdd18_switch(bool bEnable);

#ifdef __cplusplus
}
#endif

#endif // AM_BSP_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
