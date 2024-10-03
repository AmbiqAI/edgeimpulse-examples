//*****************************************************************************
//
//! @file am_devices_display_ls013b4dn04.h
//!
//! @brief Driver for handling low-level graphics and display operations for
//!        the Adafruit Sharp LS013B4DN04 Memory LCD.
//!
//! @addtogroup display_ls013b4dn04 LS013B4DN04 Display Device Driver
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

#ifndef AM_DEVICES_DISPLAY_LS013B4DN04_H
#define AM_DEVICES_DISPLAY_LS013B4DN04_H

#include "am_mcu_apollo.h"

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// C99
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>

//*****************************************************************************
//
// Specific display macro definitions
//
//*****************************************************************************
//
//! Define the physical size of the screen (inches)
//
#define AM_DISPLAY_SHARP_LS013B4DN04_SCREEN_SIZE    1.35F

//
//! @name Define the screen dimensions for the Sharp LS013B4DN04 memory display.
//! @{
//
#define AM_DISPLAY_LS013B4DN04_WIDTH        96
#define AM_DISPLAY_LS013B4DN04_HEIGHT       96
#define AM_DISPLAY_LS013B4DN04_POLARITY     1

//! @}

//
//! @name Define SHARP Memory LCD commands
//! @{
//
#define SHARP_LS013B4DN04_CMD_WRLN          0x80    //!<  Write line command
#define SHARP_LS013B4DN04_CMD_CLRMEM        0x20    //!<  Clear memory command
#define SHARP_LS013B4DN04_CMD_NOP           0x00    //!<  NOP command (VCOM)
//! @}

//
// The above commands with LSB reversed, which work well with the Apollo IOM.
//! @name Define SHARP Memory LCD commands with LSB reversed
//! @{
//
#define SHARP_LS013B4DN04_CMD_WRLN_LSB      0x01    //!<  Write line command
#define SHARP_LS013B4DN04_CMD_CLRMEM_LSB    0x04    //!<  Clear memory command
#define SHARP_LS013B4DN04_CMD_NOP_LSB       0x00    //!<  NOP command (VCOM)
//! @}

//
//! @name Define the VCOM bit in the command word
//! @{
//
#define SHARP_LS013B4DN04_VCOM_HI           0x40
#define SHARP_LS013B4DN04_VCOM_LO           0x00
#define SHARP_LS013B4DN04_VCOM_TOGGLE       0x40

#define SHARP_LS013B4DN04_VCOM_HI_LSB       0x02
#define SHARP_LS013B4DN04_VCOM_LO_LSB       0x00
//! @}

//
//! Define the value written to the framebuffer to clear the screen.
//
#define SHARP_LS013B4DN04_FB_CLEAR_VALUE    0xFF

//*****************************************************************************
//
//! @brief Device data structure used for display devices.
//
//*****************************************************************************
typedef struct
{
    //*************************************************************************
    //! @name Parameters supplied by application.
    //! @{
    //*************************************************************************

    //
    //! - Framebuffer pointer.
    //! @note Framebuffer must be guaranteed to be 32-bit aligned.
    //
    uint8_t *pui8Framebuffer;

    //
    //! @name   IOM SPI information.
    //!     - The order of the following 6 members are critical.
    //!        Do Not Change!
    //!     - The first is the SPI chip select pin number.
    //!     - The next 5 members will also be used for calls to
    //!         am_hal_iom_spi_write() and are named exactly the same as the
    //!         HAl function parameters.
    //!     - The order is critical and must be
    //!         maintained as is, and all must be 32-bit aligned.
    //!     - ui32Module and ui32ChipSelect must be filled in by the application.
    //! @{
    uint32_t    ui32ChipSelectPin;  //!< AM_BSP_GPIO_IOM1_DISPLAY_CS (app)
    uint32_t    ui32Module;         //!< AM_BSP_IOM_DISPLAY_INST (app supplied)
    uint32_t    ui32ChipSelect;     //!< AM_BSP_IOM_DISPLAY_NCE  (app supplied)
    uint32_t    *pui32Data;         //!< Pointer to the data     (driver supplied)
    uint32_t    ui32NumBytes;       //!< Number of bytes to xfer (driver supplied)
    uint32_t    ui32Options;        //!< e.g. AM_HAL_IOM_RAW     (driver supplied)

    //! @} IOM SPI information

    //! @} Parameters supplied by application

    //*************************************************************************
    //! @name Parameters supplied by the display driver
    //! @details Parameters supplied by the display driver after call to
    //! am_devices_display_ls013b4dn04_init().
    //! @{
    //*************************************************************************

    //
    //! Display width (X dimension).
    //
    uint32_t ui32DisplayWidth;

    //
    //! Display height (Y dimension).
    //
    uint32_t ui32DisplayHeight;

    //
    //! Display background polarity.  The frame buffer handler will use this to
    //!     - Determine the polarity used in the frame buffer.
    //!     - Set to 1 if the background clears with 1s (0xffs).
    //!     - Set to 0 if the background clears with 0s (0x00s).
    //
    uint32_t ui32DisplayBkgndPolarity;

    //
    //! Hardware refresh period (in ms).
    //!      - This is the required number of milliseconds between calls
    //!      to am_devices_display_ls013b4dn04_hw_refresh().
    //
    uint32_t ui32HwRefreshMs;
    //! @}
}
am_devices_display_ls013b4dn04_t;

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the display device and driver.
//!
//! @param psDisplayContext     Pointer to the displey device context structure.
//!    - NULL if interface is already configured,
//!    - or pointer to am_hal_iom_config_t structure.
//!
//! @note This function should be called before any other am_devices_display
//! functions. It optionally initializes the IO SPI interface and then the
//! display device.
//!
//! @return Non-zero if successful,
//!     - Non-zero if successful,
//!         - Non-zero indicates that the am_util_draw_context_t structure
//!         was successfully updated with device information as follows:
//!             - bit0 if ui32HwFreshMs was successfully updated.
//!             - bit1 if ui32DrawWidth and/or ui32DrawHeight were updated.
//!     - 0 if fail.
//
//*****************************************************************************
extern uint32_t am_devices_display_ls013b4dn04_init(
                        am_devices_display_ls013b4dn04_t *psDisplayContext);

//*****************************************************************************
//
//! @brief Configure the callbacks for the IOM to enable nonblocking functionality.
//!
//! @param pfnYield     - Pointer to function used while waiting for the IOM to complete.
//! @param pfnCallback  - Pointer to function used when transaction are complete.
//!
//! @note This function configures the function pointers used during IOM transactions
//! to allow for nonblocking transfers. <b>If not configured the transfers will be
//! blocking.</b>
//
//*****************************************************************************
extern void am_devices_display_ls013b4dn04_callbacks_config(
                                              am_hal_iom_callback_t pfnYield,
                                              am_hal_iom_callback_t pfnCallback);
//*****************************************************************************
//
//! @brief Clear the display.
//!
//! @param  psDisplayContext    - Pointer to the display device context structure.
//!
//! @details Clears the LCD panel and sets the framebuffer to 0xFF.
//
//*****************************************************************************
extern void am_devices_display_ls013b4dn04_screen_clear(
                            am_devices_display_ls013b4dn04_t *psDisplayContext);
//*****************************************************************************
//
//! @brief Perform a hardware refresh of the display.
//!
//! @param  psDisplayContext       - Pointer to the displey device context structure.
//! @param  bFullScreenRefresh     - Refresh all pixels in the display.
//!
//! Many memory LCD panels require a periodic refresh cycle.  This refresh is
//! not to necessarily refresh the pixels, but to provide a command to the
//! display to avoid a DC bias buildup within the LCD panel.
//!
//! Also, the entire screen needs to be refreshed about every 2 hours in order
//! to guarantee proper pixel retaining.  To do so, the application simply
//! need set the bFullScreenFresh parameter to true.
//!
//! This function is called in order to send a periodic refresh command to the
//! display.  The function am_devices_display_refresh_ms() will tell the caller
//! how often this function should be called.
//!
//! @return true if successful, false otherwise.
//
//*****************************************************************************
extern bool am_devices_display_ls013b4dn04_hw_refresh(
                            am_devices_display_ls013b4dn04_t *psDisplayContext,
                            bool bFullScreenRefresh);

//*****************************************************************************
//
//! @brief Copy lines from the framebuffer to the display.
//!
//! @param  psDisplayContext    - Pointer to the displey device context structure.
//! @param  u32BegLineNum       - Beginning line number (0-based)
//! @param  u32EndLineNum       - Last line number (0-based) to be updated.
//!
//! Copy the given lines from the given framebuffer to the display.
//
//*****************************************************************************
extern void am_devices_display_ls013b4dn04_lines_copy(
               am_devices_display_ls013b4dn04_t *psDisplayContext,
               uint32_t u32BegLineNum,
               uint32_t u32EndLineNum) ;

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_DISPLAY_LS013B4DN04_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

