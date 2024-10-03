//*****************************************************************************
//
//! @file am_devices_display_types.h
//!
//! @brief Display related types declarations that are common across all of the
//!        Ambiq Micro supported display devices.
//!
//! @addtogroup disp_type Display Devices Type Defines
//! @ingroup devices
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
#ifndef AM_DEVICES_DISPLAY_TYPES_H
#define AM_DEVICES_DISPLAY_TYPES_H

//
// NEMA_PLATFORM is a macro inherited from NemaGFX_SDK
// it is used here to distinguish graphics and non-graphics projects
//
#ifdef NEMA_PLATFORM

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
//! @name Display controller IP type.
//! @brief The display controller IPs in Apollo SoC to drive the panel.
//!
//! @note DC stands for the dedicated Display Controller IP in Apollo4B/Apollo4P/Apollo5 SoC,
//! MSPI stands for the Multi-SPI master interface IP in all Apollo series SoC.
//! More details please refer to the corresponding datasheet.
//! @{
//
//*****************************************************************************
#define  DISP_CTRL_IP_DC            0x01
#define  DISP_CTRL_IP_MSPI          0x02

//*****************************************************************************
//
//! @brief Default display controller IP
//
//*****************************************************************************
#if !defined (DISP_CTRL_IP)
#  if defined (AM_PART_APOLLO4L)
#    define DISP_CTRL_IP            DISP_CTRL_IP_MSPI
#  else
#    define DISP_CTRL_IP            DISP_CTRL_IP_DC
#  endif
#endif

//*****************************************************************************
//
//! @brief Default MSPI instance number when MSPI is used as the controller
//
//*****************************************************************************
#if (DISP_CTRL_IP == DISP_CTRL_IP_MSPI)
#  if !defined (DISPLAY_MSPI_INST)
      //
      // use MSPI2 to drive the display by default, set to 1 if MSPI1 is used
      //
#    define DISPLAY_MSPI_INST       2
#  endif
#endif

//*****************************************************************************
//
//! @brief Default TE pin number
//! @} Display controller IP type.
//
//*****************************************************************************
#if !defined (DISPLAY_TE_PIN)
#  if (DISP_CTRL_IP == DISP_CTRL_IP_MSPI)
#    define DISPLAY_TE_PIN          AM_BSP_GPIO_MSPI2_TE
#  else
     //
     // change to other TE pin according to your BSP usage
     // default is the one for DSI interface
     //
#    define DISPLAY_TE_PIN          AM_BSP_GPIO_DISP_DSI_TE
#  endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @brief The display IC in the display panels which Apollo SoC talks to.
//!
//! @note These macros stand for different display panel IC types. For more details,
//! please refer to the IC datasheets.
//
//*****************************************************************************
typedef enum
{
    DISP_IC_RM69330                 = 0x01,
    DISP_IC_RM67162                 = 0x02,
    DISP_IC_CO5300                  = 0x03,
    DISP_IC_NT38350                 = 0x04
} am_devices_disp_ic_e;

//*****************************************************************************
//
//! @brief The interface type between Apollo SoC and the display panel.
//!
//! @note Apollo DC IP supports SPI4, DSPI, QSPI , DBI, JDI and MIPI-DSI together
//! with DSI IP. Apollo MSPI IP supports QSPI for now. For more details, please
//! refer to the apollo SoCs datasheet.
//
//*****************************************************************************
typedef enum
{
    DISP_IF_DSI                     = 0x00,
    DISP_IF_SPI4                    = 0x01,
    DISP_IF_DSPI                    = 0x02,
    DISP_IF_QSPI                    = 0x04,
    DISP_IF_JDI                     = 0x08,
    DISP_IF_DPI                     = 0x10,
    DISP_IF_DBI                     = 0x20
} am_devices_disp_if_e;

//*****************************************************************************
//
//! @brief The color format supported.
//!
//! @note RGB565 and RGB888 are the coding color formats this driver supports now,
//! Please contact us if more format is needed.
//
//*****************************************************************************
typedef enum
{
// #### INTERNAL BEGIN ####
    COLOR_FORMAT_3BIT,
    COLOR_FORMAT_8BIT,
// #### INTERNAL END ####
    COLOR_FORMAT_RGB565,
// #### INTERNAL BEGIN ####
    COLOR_FORMAT_RGB666,
// #### INTERNAL END ####
    COLOR_FORMAT_RGB888
} am_devices_disp_color_e;

//*****************************************************************************
//
//! @brief The handling method by Apollo SoC of TE signal from the panel.
//!
//! @note Apollo could disable TE handling, or treat TE signal as an GPIO interrupt,
//! or handle the TE signal by its DC IP.
//
//*****************************************************************************
typedef enum
{
    DISP_TE_DISABLE,
    DISP_TE_GPIO,
    DISP_TE_DC
}am_devices_disp_te_type_e;

//*****************************************************************************
//
//! @brief DC used structure
//
//*****************************************************************************
typedef struct
{
    uint32_t ui32PixelFormat;
    uint16_t ui16ResX;
    uint16_t ui16ResY;
    uint16_t ui16MinX;
    uint16_t ui16MinY;
    bool bTEEnable;
    bool bFlip;
    uint8_t ui8Lanes;
} am_devices_dc_config_t;

//*****************************************************************************
//
//! @brief DSI used structure
//
//*****************************************************************************
typedef enum
{
    FMT_RGB565 = 0,
    FMT_RGB888,
    FMT_NUM
} pixel_format_t;

//*****************************************************************************
//
//! @brief Detailed result after display operations
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_DISPLAY_STATUS_SUCCESS,
    AM_DEVICES_DISPLAY_STATUS_ERROR,
    AM_DEVICES_DISPLAY_STATUS_INVALID_HANDLE,
    AM_DEVICES_DISPLAY_STATUS_IN_USE,
    AM_DEVICES_DISPLAY_STATUS_TIMEOUT,
    AM_DEVICES_DISPLAY_STATUS_OUT_OF_RANGE,
    AM_DEVICES_DISPLAY_STATUS_INVALID_ARG,
    AM_DEVICES_DISPLAY_STATUS_INVALID_OPERATION,
    AM_DEVICES_DISPLAY_STATUS_PANEL_ERR,
    AM_DEVICES_DISPLAY_STATUS_DPCTR_ERR,
    AM_DEVICES_DISPLAY_STATUS_TRY_AGAIN,
} am_devices_display_status_t;

//*****************************************************************************
//
//! @brief The display configurations at the BSP (board) level
//
//*****************************************************************************
typedef struct
{
    //! Display Panel IC
    am_devices_disp_ic_e    eIC;

    //! Display interface type
    am_devices_disp_if_e    eInterface;

    //! Display panel offset
    uint16_t                ui16Offset;

    //! Display flip or not
    bool                    bFlip;

    //! Display panel resolution
    uint16_t                ui16ResX;
    uint16_t                ui16ResY;

#if (DISP_CTRL_IP == DISP_CTRL_IP_MSPI)
    //
    //! MSPI as display controller IP
    //
    uint32_t                ui32Module;        // MSPI IP instance number
    am_hal_mspi_device_e    eDeviceConfig;     // MSPI interface mode & CE
    am_hal_mspi_clock_e     eClockFreq;        // MSPI clock frequency
    bool                    bClockonD4;        // MSPI clock on Data4
#endif

#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
    //
    //! DC as display controller IP
    //
    am_hal_dsi_freq_trim_e  eDsiFreq;          // DSI IF frequency
    am_hal_dsi_dbi_width_e  eDbiWidth;         // DSI IP source (DBI) width
    uint8_t ui8NumLanes;                       // DSI IF lane number
#endif

    uint8_t                 ui8DispMspiSelect; // DISP QSPI interface number

    //
    //! TE (Tearing-Effect) signal configurations
    //
    am_devices_disp_te_type_e   eTEType;           // TE type
    uint16_t                ui16TEpin;         // GPIO No (TE GPIO interrupt)
// #### INTERNAL BEGIN ####
    bool                    bTEAdj;            // Adjust TE frequency of panel
// #### INTERNAL END #####
} am_devices_display_hw_config_t;

#ifdef __cplusplus
}
#endif

#endif // NEMA_PLATFORM

#endif // AM_DEVICES_DISPLAY_TYPES_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
