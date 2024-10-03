//*****************************************************************************
//
//! @file am_devices_display_ls013b4dn04.c
//!
//! @brief Driver for handling low-level graphics and display operations for
//!        the Adafruit Sharp LS013B4DN04 Memory LCD.
//!
//! @addtogroup display_ls013b4dn04 LS013B4DN04 Display Device Driver
//! @ingroup devices
//! @{
//
//*****************************************************************************

// #### INTERNAL BEGIN ####
//!  Notes:
//!      - A quick guide to connect a Sharp LCD to an Apollo EVK via IOM1:
//!            - Sharp_display       EVK
//!            - Pin 1 VIN           3.3v
//!            - Pin 2 V3_3          n/c
//!            - Pin 3 GND           gnd
//!            - Pin 4 SCK           M1SCK   pad8
//!            - Pin 5 MOSI          M1MOSI  pad10
//!            - Pin 6 SCS           GPIO12
//!            - Pin 7 EXTMODE       n/c
//!            - Pin 8 DISP          n/c
//!            - Pin 9 EXTCOMIN      n/c
//!      - The Sharp LCD displays use an asserted high CS, the Apollo IOM
//!        provides asserted LOW CEs. Therefore, the CS must be bit-banged
//!        via a standard GPIO (GPIO 12 in the above case).
// #### INTERNAL END ####

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_devices.h"
#include "am_bsp.h"

//*****************************************************************************
//
//! @name Global variables.
//! @{
//*****************************************************************************
static volatile uint8_t g_u8vcom = 0x00;
static volatile bool bComplete = false;

am_hal_iom_callback_t display_ls013b4dn04_yield;
am_hal_iom_callback_t display_ls013b4dn04_callback;
//! @}

//*****************************************************************************
//
// Static helper functions.
//
//*****************************************************************************
//*****************************************************************************
//! @brief
//*****************************************************************************
static void
IOMCallBack(void)
{
    bComplete = true;
}

//*****************************************************************************
//! @brief
//*****************************************************************************
static void
IOMYield(void)
{
    while ( !bComplete );
    bComplete = false;
}

//*****************************************************************************
//
//! @breif This function starts a SPI transaction using the IOM parameters specified
//! in u32SPIparms and ui32CSpin.
//!
//! @param psDisplayContext    - Pointer to the displey device context structure.
//! @param u32CSskipflags
//!      - 0: Assert CS, do the SPI transaction, wait till complete, deassert CS.
//!      - bit0: Assume CS is already asserted, so do not reassert.  Also do
//!                not remove CS after sending the command.
//!      - bit1: Deassert CS and return.
//
//*****************************************************************************
static void
start_iom(am_devices_display_ls013b4dn04_t *psDisplayContext, uint32_t u32CSskipflags)
{
    uint32_t ui32CSpin = psDisplayContext->ui32ChipSelectPin;

    //
    // Before asserting or removing CS, wait until any SPI transactions have
    // completed.
    //
    am_hal_iom_poll_complete(psDisplayContext->ui32Module);

    if ( u32CSskipflags & 0x02 )
    {
        //
        // Take CS low
        //
#if AM_APOLLO3_GPIO
        am_hal_gpio_state_write(ui32CSpin, AM_HAL_GPIO_OUTPUT_CLEAR);
#else // AM_APOLLO3_GPIO
        AM_HAL_GPIO_WTC(ui32CSpin) = AM_HAL_GPIO_WTC_M(ui32CSpin);
#endif // AM_APOLLO3_GPIO

        //
        // And return
        //
        return;
    }

    //
    // Take CS high
    //
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(ui32CSpin, AM_HAL_GPIO_OUTPUT_TRISTATE_ENABLE);
    am_hal_gpio_state_write(ui32CSpin, AM_HAL_GPIO_OUTPUT_SET);
#else // AM_APOLLO3_GPIO
    AM_HAL_GPIO_ENS(ui32CSpin) = AM_HAL_GPIO_ENS_M(ui32CSpin);
    AM_HAL_GPIO_WTS(ui32CSpin) = AM_HAL_GPIO_WTS_M(ui32CSpin);
#endif // AM_APOLLO3_GPIO

    //
    // Now write to the display.
    //
    am_hal_iom_queue_spi_write(psDisplayContext->ui32Module,
                         psDisplayContext->ui32ChipSelect,
                         psDisplayContext->pui32Data,
                         psDisplayContext->ui32NumBytes,
                         psDisplayContext->ui32Options,
                         display_ls013b4dn04_callback);

    display_ls013b4dn04_yield();

    if ( !(u32CSskipflags & 0x01) )
    {
        //
        // Before removing CS, wait until the SPI transaction has completed.
        //
        am_hal_iom_poll_complete(psDisplayContext->ui32Module);

        //
        // Take CS low
        //
#if AM_APOLLO3_GPIO
        am_hal_gpio_state_write(ui32CSpin, AM_HAL_GPIO_OUTPUT_CLEAR);
#else // AM_APOLLO3_GPIO
        AM_HAL_GPIO_WTC(ui32CSpin) = AM_HAL_GPIO_WTC_M(ui32CSpin);
#endif // AM_APOLLO3_GPIO
    }

} // start_iom()

//*****************************************************************************
//!
//! @brief refresh Sharp LS013B4DN04
//!
//! @details The Sharp LS013B4DN04 memory LCD panel requires a periodic refresh cycle.
//! This refresh is not to necessarily refresh the pixels, but to provide a
//! command to the display to avoid a DC bias buildup within the panel.  Sharp
//! recommends this refresh occur about once per second (for more details see
//! p.12 of Sharp LS013B4DN04 Application Note, Reference Code SMA11008,
//! LS013B4DN04_Application_Info.pdf).
//!
//! The hardware refresh consists of any valid command, including the NOP cmd,
//! with bit6 toggled from the previous time.  This function tracks the state
//! of the VCOM bit globally such that any time it is called it will toggle the
//! state of the bit from the last time it was called.
//!
//! @return Returns the VCOM command bit toggled from the last time this function was
//! called (see above discussion for more details).  The return value can be
//! ORed directly into the command byte and sent to the display.
//
//*****************************************************************************
static uint8_t
get_vcom(void)
{
    uint8_t u8ret = g_u8vcom;

    //
    // Toggle the VCOM variable
    //
    g_u8vcom ^= SHARP_LS013B4DN04_VCOM_TOGGLE;

    //
    // Return the new value to use for VCOM
    //
    return u8ret;
} // get_vcom()

//*****************************************************************************
//
//  Initialize the display device and driver.
//
//*****************************************************************************
uint32_t
am_devices_display_ls013b4dn04_init(
                            am_devices_display_ls013b4dn04_t *psDisplayContext)
{
    uint32_t ui32Ret = 0x01;

    //
    // Initialize the IOM callbacks.
    //
    display_ls013b4dn04_yield = (am_hal_iom_callback_t) IOMYield;
    display_ls013b4dn04_callback = (am_hal_iom_callback_t) IOMCallBack;

    if ( (psDisplayContext->ui32DisplayWidth  != AM_DISPLAY_LS013B4DN04_WIDTH )  ||
         (psDisplayContext->ui32DisplayHeight != AM_DISPLAY_LS013B4DN04_HEIGHT) )
    {
        psDisplayContext->ui32DisplayWidth  = AM_DISPLAY_LS013B4DN04_WIDTH;
        psDisplayContext->ui32DisplayHeight = AM_DISPLAY_LS013B4DN04_HEIGHT;
        ui32Ret |= 0x02;
    }

    //
    // Return the hardware refresh period for this device (ms)
    //
    psDisplayContext->ui32HwRefreshMs = 1000;

    //
    // Display background polarity.  The frame buffer handler will use this
    //  attribute to determine the polarity used in the frame buffer.
    //  Set to 1 if the background clears with 1s (0xffs).
    //  Set to 0 if the background clears with 0s (0x00s).
    //
    // The LS013b4dn04 is cleared with 0xffs, so set to 1.
    //
    psDisplayContext->ui32DisplayBkgndPolarity = 1;

    //
    // Initialize the framebuffer and clear the screen.
    //
    am_devices_display_ls013b4dn04_screen_clear(psDisplayContext);

    return ui32Ret;
} // am_devices_display_ls013b4dn04_init()

//*****************************************************************************
//
//  Clear the display.
//
//*****************************************************************************
void
am_devices_display_ls013b4dn04_screen_clear(
                            am_devices_display_ls013b4dn04_t *psDisplayContext)
{
    int32_t ix;
    uint32_t ui32Buf[4 / 4];
    uint8_t *pui8Buf = (uint8_t*)ui32Buf;

    //
    // Begin by clearing the framebuffer
    //
    for (ix = 0; ix < (AM_DISPLAY_LS013B4DN04_WIDTH / 8) * AM_DISPLAY_LS013B4DN04_HEIGHT; ix++)
    {
        psDisplayContext->pui8Framebuffer[ix] = SHARP_LS013B4DN04_FB_CLEAR_VALUE;
    }

    //
    // Now send the command to clear the screen.
    //
    pui8Buf[0] = SHARP_LS013B4DN04_CMD_CLRMEM;
    pui8Buf[1] = 0x00;

    //
    // The device structure contains the first 3 parameters:
    //  ui32ChipSelectPin, ui32Module and ui32ChipSelect.  Fill
    //  in the remaining 3 needed for this operation.
    //
    psDisplayContext->pui32Data    = (uint32_t*)pui8Buf;
    psDisplayContext->ui32NumBytes = 2;
    psDisplayContext->ui32Options  = AM_HAL_IOM_RAW;
    start_iom(psDisplayContext, 0);

} // am_devices_display_ls013b4dn04_screen_clear()

//*****************************************************************************
//
//  Perform a hardware refresh of the display.
//
//*****************************************************************************
bool
am_devices_display_ls013b4dn04_hw_refresh(
                            am_devices_display_ls013b4dn04_t *psDisplayContext,
                            bool bFullScreenDataRefresh)
{
    uint32_t ui32CmdBuf[4 / 4];
    uint8_t *pui8CmdBuf = (uint8_t*)ui32CmdBuf;

    if ( !(am_hal_iom_status_get(psDisplayContext->ui32Module) &
           AM_REG_IOMSTR_STATUS_IDLEST_M) )
    {
        //
        // If the IOM is not idle, it means it's probably busy writing data to
        // the display.  Therefore, we'll just put off the refresh until the
        // next interrupt and hopefully it won't be so busy then.
        //
        return false;
    }

    //
    // The Sharp LCD requires a VCOM refresh at least once per second.
    // Further, it requires a pixel refresh at least once every 2 hours.
    //
    // This function assumes it will be called once every second.
    //
    if ( bFullScreenDataRefresh )
    {
        //
        // We'll do a complete refresh of the display this time.
        //
        am_devices_display_ls013b4dn04_lines_copy(psDisplayContext, 0,
                            AM_DISPLAY_LS013B4DN04_HEIGHT - 1);
    }

    pui8CmdBuf[0] = SHARP_LS013B4DN04_CMD_NOP + get_vcom();
    pui8CmdBuf[1] = 0x00;

    //
    // Now toggle VCOM.
    //
    // The device structure contains the first 3 parameters:
    //  ui32ChipSelectPin, ui32Module and ui32ChipSelect.  Fill
    //  in the remaining 3 needed for this operation.
    //
    psDisplayContext->pui32Data    = ui32CmdBuf;
    psDisplayContext->ui32NumBytes = 2;
    psDisplayContext->ui32Options  = AM_HAL_IOM_RAW;
    start_iom(psDisplayContext, 0);

    return true;
} // am_devices_display_ls013b4dn04_hw_refresh()

//*****************************************************************************
//
//  Copy lines from the framebuffer to the display.
//
//*****************************************************************************
void
am_devices_display_ls013b4dn04_lines_copy(
                            am_devices_display_ls013b4dn04_t *psDisplayContext,
                            uint32_t u32BegLineNum,
                            uint32_t u32EndLineNum)
{
    uint32_t ui32Buf[(AM_DISPLAY_LS013B4DN04_WIDTH / 8 + 4) / 4];
    uint8_t *pui8FB, *pui8Buf;
    int32_t ix, jx, i32nBytes;
    uint32_t ui32Options;

    if ( u32BegLineNum > u32EndLineNum )
    {
        return;
    }

    pui8Buf = (uint8_t*)ui32Buf;
    pui8FB = &psDisplayContext->pui8Framebuffer[u32BegLineNum * (AM_DISPLAY_LS013B4DN04_WIDTH / 8)];

    ui32Options = ( u32BegLineNum < u32EndLineNum )                         ?
                  AM_HAL_IOM_RAW | AM_HAL_IOM_LSB_FIRST | AM_HAL_IOM_CS_LOW :
                  AM_HAL_IOM_RAW | AM_HAL_IOM_LSB_FIRST;

    //
    // Write the first line normally.
    //
    pui8Buf[0] = SHARP_LS013B4DN04_CMD_WRLN_LSB;
    pui8Buf[1] = u32BegLineNum + 1;

    //
    // Copy the data of the first line into the command buffer
    //
    for (jx = 2; jx < 2 + (AM_DISPLAY_LS013B4DN04_WIDTH / 8); jx++)
    {
        pui8Buf[jx] = *pui8FB++;
    }

    //
    // Write trailer
    //
    pui8Buf[jx] = 0x00;

    //
    // Write the IOM, but keep CS asserted.
    //
    // The device structure contains the first 3 parameters:
    //  ui32ChipSelectPin, ui32Module and ui32ChipSelect.  Fill
    //  in the remaining 3 needed for this operation.
    //
    psDisplayContext->pui32Data    = (uint32_t*)pui8Buf;
    psDisplayContext->ui32NumBytes = (AM_DISPLAY_LS013B4DN04_WIDTH / 8) + 3;
    psDisplayContext->ui32Options  = ui32Options;

    start_iom(psDisplayContext, 1);

    //
    // The first line has been written.  Now write intermediate lines.
    // The cmds for these lines only need the line number, data, 1 byte trailer.
    //
    i32nBytes = (AM_DISPLAY_LS013B4DN04_WIDTH / 8) + 2;
    ui32Options = AM_HAL_IOM_RAW | AM_HAL_IOM_CS_LOW | AM_HAL_IOM_LSB_FIRST;
    for ( ix = u32BegLineNum + 1; ix <= u32EndLineNum; ix++ )
    {
        pui8Buf[0] = ix + 1;

        for (jx = 1; jx < 1 + (AM_DISPLAY_LS013B4DN04_WIDTH / 8); jx++)
        {
            pui8Buf[jx] = *pui8FB++;
        }
        pui8Buf[jx + 0] = 0x00;
        pui8Buf[jx + 1] = 0x00;

        if ( ix == u32EndLineNum )
        {
            ui32Options = AM_HAL_IOM_RAW | AM_HAL_IOM_LSB_FIRST;
            i32nBytes++;
        }

        //
        // The device structure contains the first 3 parameters:
        //  ui32ChipSelectPin, ui32Module and ui32ChipSelect.  Fill
        //  in the remaining 3 needed for this operation.
        //
        psDisplayContext->pui32Data    = (uint32_t*)pui8Buf;
        psDisplayContext->ui32NumBytes = i32nBytes;
        psDisplayContext->ui32Options  = ui32Options;

        start_iom(psDisplayContext, 1);
    }
    start_iom(psDisplayContext, 2);
} // am_devices_display_ls013b4dn04_lines_copy()

//*****************************************************************************
//
//  Configure the callbacks for the IOM to enable nonblocking functionality.
//
//*****************************************************************************
void
am_devices_display_ls013b4dn04_callbacks_config(am_hal_iom_callback_t pfnYield,
                                                am_hal_iom_callback_t pfnCallback)
{
    display_ls013b4dn04_yield = pfnYield;
    display_ls013b4dn04_callback = pfnCallback;
}

// #### INTERNAL BEGIN ####
#if 0
//*****************************************************************************
//
// swizzle() - Static helper function.
//
//*****************************************************************************
static uint8_t
swizzle(uint8_t value)
{
    int8_t i8tmp, tmpbyte;
    tmpbyte = 0;
    for (i8tmp=0; i8tmp<8; i8tmp++)
    {
        tmpbyte |= ((value & 0x01) << (7-i8tmp));
        value >>= 1;
    }
    return tmpbyte;
} // swizzle()
#endif
// #### INTERNAL END ####

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

