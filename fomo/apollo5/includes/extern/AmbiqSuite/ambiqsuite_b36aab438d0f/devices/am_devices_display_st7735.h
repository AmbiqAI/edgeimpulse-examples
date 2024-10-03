//*****************************************************************************
//
//! @file am_devices_display_st7735.h
//!
//! @brief Driver for handling low-level graphics and display operations for
//!        the Sitronix st7735 TFT display module.
//!
//! @addtogroup display_st7735 ST7735 Display Device Driver
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

#ifndef AM_DEVICES_DISPLAY_ST7735_H
#define AM_DEVICES_DISPLAY_ST7735_H

//*****************************************************************************
//
// C99
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
// Specific display macro definitions
//
//*****************************************************************************
//
//! Define the physical size of the screen (inches)
//
#define AM_DISPLAY_ST7735_SCREEN_SIZE 1.2F

//
//! Define the screen dimensions for the ST7735 memory display.
//
#define AM_DISPLAY_ST7735_WIDTH              128
#define AM_DISPLAY_ST7735_HEIGHT             160
#define AM_DISPLAY_ST7735_POLARITY           0

#if 1

//
//! @name Display Commands
//! @{
//

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

//! @} Display Commands

//! @name  Color definitions
//! @{
#define ST7735_BLACK   0x0000
#define ST7735_BLUE    0x001F
#define ST7735_RED     0xF800
#define ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF
//! @} Color definitions
#endif

#if 1
//
//! Define the value written to the framebuffer to clear the screen.
//
#define ST7735_FB_CLEAR_VALUE                0x00

//
//! @name Define the display commands
//! @{
//
#define ST7735_CMD3B_SET_COL_ADDR            0x15
#define ST7735_CMDxB_HORZ_SCROLL_SETUPA      0x26
#define ST7735_CMDxB_HORZ_SCROLL_SETUPB      0x27
#define ST7735_CMDxB_HORZ_SCROLL_DEACTIVATE  0x2E
#define ST7735_CMDxB_HORZ_SCROLL_ACTIVATE    0x2F
#define ST7735_CMD3B_SET_ROW_ADDR            0x75
#define ST7735_CMD2B_SET_CONTRAST_CURRENT    0x81
#define ST7735_CMD1B_NOP                     ST7735_CMD1B_NOP0
#define ST7735_CMD1B_NOP0                    0x84
#define ST7735_CMD1B_NOP1                    0x85
#define ST7735_CMD1B_NOP2                    0x86
#define ST7735_CMD2B_SET_REMAP               0xA0
#define ST7735_CMD2B_SET_START_LINE          0xA1
#define ST7735_CMD2B_SET_OFFSET              0xA2
#define ST7735_CMD1B_SETMODE_NORMAL          0xA4
#define ST7735_CMD1B_SETMODE_ON              0xA5
#define ST7735_CMD1B_SETMODE_OFF             0xA6
#define ST7735_CMD1B_SETMODE_INVERT          0xA7
#define ST7735_CMD2B_SET_MULTIPLEX           0xA8
#define ST7735_CMD2B_SELECT_FUNC             0xAB
#define ST7735_CMD1B_DISPLAY_OFF             0xAE
#define ST7735_CMD1B_DISPLAY_ON              0xAF
#define ST7735_CMD2B_SET_PHASE_LEN           0xB1
#define ST7735_CMD1B_NOP3                    0xB2
#define ST7735_CMD2B_SET_CLK_DIVIDER         0xB3
#define ST7735_CMD2B_SET_GPIO                0xB5
#define ST7735_CMD2B_SET_2ND_PRECHARGE       0xB6
#define SSD1327_CMDxB_SET_GRAYSCALE_TBL             0xB8
#define SSD1327_CMDxB_SET_DEF_GRAYSCALE_TBL         0xB9
#define ST7735_CMD1B_NOP4                    0xBB
#define ST7735_CMD2B_SET_PRECHARGE_VOLT      0xBC
#define ST7735_CMD2B_SET_VCOMH_VOLT          0xBE
#define ST7735_CMD2B_SELECT_FUNC_B           0xD5
#define ST7735_CMD2B_SET_CMD_LOCK            0xFD

//! @} // Define the display commands

//
//! @name Define the display commands (reversed)
//! @details These are the commands we'll use with the Ambiq IOM in bit-reversed mode.
//! @{
//
#define ST7735_RCMD3B_SET_COL_ADDR           0xA8    // 0x15
#define ST7735_RCMDxB_HORZ_SCROLL_SETUPA     0x64    // 0x26
#define ST7735_RCMDxB_HORZ_SCROLL_SETUPB     0xE4    // 0x27
#define ST7735_RCMDxB_HORZ_SCROLL_DEACTIVATE 0x74    // 0x2E
#define ST7735_RCMDxB_HORZ_SCROLL_ACTIVATE   0xF4    // 0x2F
#define ST7735_RCMD3B_SET_ROW_ADDR           0xAE    // 0x75
#define ST7735_RCMD2B_SET_CONTRAST_CURRENT   0x81    // 0x81
#define ST7735_RCMD1B_NOP                    ST7735_RCMD1B_NOP0
#define ST7735_RCMD1B_NOP0                   0x21    // 0x84
#define ST7735_RCMD1B_NOP1                   0xA1    // 0x85
#define ST7735_RCMD1B_NOP2                   0x61    // 0x86
#define ST7735_RCMD2B_SET_REMAP              0x05    // 0xA0
#define ST7735_RCMD2B_SET_START_LINE         0x85    // 0xA1
#define ST7735_RCMD2B_SET_OFFSET             0x45    // 0xA2
#define ST7735_RCMD1B_SETMODE_NORMAL         0x25    // 0xA4
#define ST7735_RCMD1B_SETMODE_ON             0xA5    // 0xA5
#define ST7735_RCMD1B_SETMODE_OFF            0x65    // 0xA6
#define ST7735_RCMD1B_SETMODE_INVERT         0xE5    // 0xA7
#define ST7735_RCMD2B_SET_MULTIPLEX          0x15    // 0xA8
#define ST7735_RCMD2B_SELECT_FUNC            0xD5    // 0xAB
#define ST7735_RCMD1B_DISPLAY_OFF            0x75    // 0xAE
#define ST7735_RCMD1B_DISPLAY_ON             0xF5    // 0xAF
#define ST7735_RCMD2B_SET_PHASE_LEN          0x8D    // 0xB1
#define ST7735_RCMD1B_NOP3                   0x4D    // 0xB2
#define ST7735_RCMD2B_SET_CLK_DIVIDER        0xCD    // 0xB3
#define ST7735_RCMD2B_SET_GPIO               0xAD    // 0xB5
#define ST7735_RCMD2B_SET_2ND_PRECHARGE      0x6D    // 0xB6
#define SSD1327_RCMDxB_SET_GRAYSCALE_TBL            0x1D    // 0xB8
#define SSD1327_RCMDxB_SET_DEF_GRAYSCALE_TBL        0x9D    // 0xB9
#define ST7735_RCMD1B_NOP4                   0xDD    // 0xBB
#define ST7735_RCMD2B_SET_PRECHARGE_VOLT     0x3D    // 0xBC
#define ST7735_RCMD2B_SET_VCOMH_VOLT         0x7D    // 0xBE
#define ST7735_RCMD2B_SELECT_FUNC_B          0xAB    // 0xD5
#define ST7735_RCMD2B_SET_CMD_LOCK           0xBF    // 0xFD
//! @} // Define the display commands (reversed)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! Data structure used for display devices.
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
    uint32_t    ui32ChipSelectPin;  // AM_BSP_GPIO_IOM1_DISPLAY_CS (app)
    uint32_t    ui32Module;         // AM_BSP_IOM_DISPLAY_INST (app supplied)
    uint32_t    ui32ChipSelect;     // AM_BSP_IOM_DISPLAY_NCE  (app supplied)
    uint32_t    ui32DCselect;       // AM_BSP_GPIO_DISPLAY_D_C (app supplied)

    //! @} IOM SPI information

    //! @} Parameters supplied by application

    //*************************************************************************
    //! @name Parameters supplied by the display driver
    //! @details Parameters supplied by the display driver after call to
    //! am_devices_display_st7735_init().
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
    //      - Determine the polarity used in the frame buffer.
    //      - Set to 1 if the background clears with 1s (0xffs).
    //      - Set to 0 if the background clears with 0s (0x00s).
    //
    uint32_t ui32DisplayBkgndPolarity;

    //
    //! Hardware refresh period (in ms).
    //!     - This is the required number of milliseconds between calls
    //!       to am_devices_display_st7735_hw_refresh().
    //
    uint32_t ui32HwRefreshMs;
    //! @} Parameters supplied by display driver
}
am_devices_display_st7735_t;

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
//! @param psDisplayContext     - Pointer to the display device context structure.
//!
//! @note This function should be called before any other am_devices_display
//! functions. It optionally initializes the IO SPI interface and then the
//! display device.
//!
//! @return Non-zero if successful, 0 if fail.
//!     - Non-zero indicates that the am_util_draw_context_t structure
//!       was successfully updated with device information as follows:
//!         - bit0 if ui32HwFreshMs was successfully updated.
//!         - bit1 if ui32DrawWidth and/or ui32DrawHeight were updated.
//
//*****************************************************************************
extern uint32_t am_devices_display_st7735_init(
                        am_devices_display_st7735_t *psDisplayContext);

//*****************************************************************************
//
//! @brief Configure the callbacks for the IOM to enable nonblocking functionality.
//!
//! @param pfnYield    - Pointer to function used while waiting for the IOM to complete.
//! @param pfnCallback - Pointer to function used when transaction are complete.
//!
//! @note This function configures the function pointers used during IOM transactions
//! to allow for nonblocking transfers. <b>If not configured the transfers will be
//! blocking.</b>
//
//*****************************************************************************
extern void am_devices_display_st7735_callbacks_config(
                        am_hal_iom_callback_t pfnYield,
                        am_hal_iom_callback_t pfnCallback);

//*****************************************************************************
//
//! @brief Clear the display.
//!
//! @param  psDisplayContext  - Pointer to the display device context structure.
//!
//! Clears the LCD panel and sets the framebuffer to 0xFF.
//
//*****************************************************************************
extern void am_devices_display_st7735_screen_clear(
                        am_devices_display_st7735_t *psDisplayContext);

//*****************************************************************************
//
//! @brief Perform a hardware refresh of the display.
//!
//! @param psDisplayContext     - Pointer to the display device context structure.
//! @param bFullScreenRefresh   - Refresh all pixels in the display.
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
extern bool am_devices_display_st7735_hw_refresh(
                        am_devices_display_st7735_t *psDisplayContext,
                        bool bFullScreenRefresh);

//*****************************************************************************
//
//! @brief Copy lines from the framebuffer to the display.
//!
//! @param  psDisplayContext - Pointer to the display device context structure.
//! @param  u32BegLineNum    - First line number (0-based) to be updated.
//! @param  u32EndLineNum    - Last line number (0-based) to be updated.
//!
//! Copy the given lines from the given framebuffer to the display.
//
//*****************************************************************************
extern void  am_devices_display_st7735_lines_copy(
                        am_devices_display_st7735_t *psDisplayContext,
                        uint32_t u32BegLineNum,
                        uint32_t u32EndLineNum);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_DISPLAY_ST7735_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

