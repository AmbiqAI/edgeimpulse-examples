//*****************************************************************************
//
//! @file am_devices_display_st7735.c
//!
//! @brief Driver for handling low-level graphics and display operations for
//!        the Sitronix st7735 TFT display module.
//!
//! @addtogroup display_st7735 ST7735 Display Device Driver
//! @ingroup devices
//! @{
//
//*****************************************************************************

// #### INTERNAL BEGIN ####
//  Notes:
// 29  Lite - this is the PWM input for the backlight control.
//      Connect to 3-5VDC to turn on the backlight. Connect to ground
//      to turn it off. Or, you can PWM at any frequency.
// 8  SCLK - this is the SPI clock input pin. Mode 0, Active High
//10  MOSI - this is the SPI Master Out Slave In pin,
//12  TFT_CS - this is the TFT SPI chip select pin
//31  D/C - this is the TFT SPI data or command selector pin
//nRST  RST - this is the TFT reset pin. Software resets screen, can NC.
//      Optional connect to nRST.
//  Vcc - this is the power pin, connect to 3-5VDC
//  GND - this is the power and signal ground pin
//
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
//! @name Local definitions.
//! @{
//
//*****************************************************************************
#define ADDR_MODE_NONE  0
#define ADDR_MODE_HORZ  1
#define ADDR_MODE_VERT  2

#define ROTATE_90       0       // 1 = rotate image 90 (counter-clockwise)

//! @}

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
static uint32_t g_ui32AddressMode = ADDR_MODE_NONE;
static uint32_t g_bDCn = false;

static volatile bool bComplete = false;
am_hal_iom_callback_t display_st7735_yield;
am_hal_iom_callback_t display_st7735_callback;

static uint32_t g_ui32IOMBuf[64 / 4];
static uint32_t g_FB4bitLine[96*4 / 8 / 4];

#if ROTATE_90
static uint8_t g_ui8MaskTbl[8] =
{
    0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};
#else // not ROTATE_90
static uint32_t g_XlateNibble[] =
{
    0x00000000, //0
    0x00000F00, //1
    0x0000F000, //2
    0x0000FF00, //3
    0x0000000F, //4
    0x00000F0F, //5
    0x0000F00F, //6
    0x0000FF0F, //7
    0x000000F0, //8

    0x00000FF0, //9
    0x0000F0F0, //A
    0x0000FFF0, //B
    0x000000FF, //C
    0x00000FFF, //D
    0x0000F0FF, //E
    0x0000FFFF, //F
};
#endif // ROTATE_90

// Command Block Tables
#if 0
#define DELAY 0x80
static const uint8_t
  Bcmd[] = {                  // Initialization commands for 7735B screens
    18,                       // 18 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, no args, w/delay
      50,                     //     50 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, no args, w/delay
      255,                    //     255 = 500 ms delay
    ST7735_COLMOD , 1+DELAY,  //  3: Set color mode, 1 arg + delay:
      0x05,                   //     16-bit color
      10,                     //     10 ms delay
    ST7735_FRMCTR1, 3+DELAY,  //  4: Frame rate control, 3 args + delay:
      0x00,                   //     fastest refresh
      0x06,                   //     6 lines front porch
      0x03,                   //     3 lines back porch
      10,                     //     10 ms delay
    ST7735_MADCTL , 1      ,  //  5: Memory access ctrl (directions), 1 arg:
      0x08,                   //     Row addr/col addr, bottom to top refresh
    ST7735_DISSET5, 2      ,  //  6: Display settings #5, 2 args, no delay:
      0x15,                   //     1 clk cycle nonoverlap, 2 cycle gate
                              //     rise, 3 cycle osc equalize
      0x02,                   //     Fix on VTL
    ST7735_INVCTR , 1      ,  //  7: Display inversion control, 1 arg:
      0x0,                    //     Line inversion
    ST7735_PWCTR1 , 2+DELAY,  //  8: Power control, 2 args + delay:
      0x02,                   //     GVDD = 4.7V
      0x70,                   //     1.0uA
      10,                     //     10 ms delay
    ST7735_PWCTR2 , 1      ,  //  9: Power control, 1 arg, no delay:
      0x05,                   //     VGH = 14.7V, VGL = -7.35V
    ST7735_PWCTR3 , 2      ,  // 10: Power control, 2 args, no delay:
      0x01,                   //     Opamp current small
      0x02,                   //     Boost frequency
    ST7735_VMCTR1 , 2+DELAY,  // 11: Power control, 2 args + delay:
      0x3C,                   //     VCOMH = 4V
      0x38,                   //     VCOML = -1.1V
      10,                     //     10 ms delay
    ST7735_PWCTR6 , 2      ,  // 12: Power control, 2 args, no delay:
      0x11, 0x15,
    ST7735_GMCTRP1,16      ,  // 13: Magical unicorn dust, 16 args, no delay:
      0x09, 0x16, 0x09, 0x20, //     (seriously though, not sure what
      0x21, 0x1B, 0x13, 0x19, //      these config values represent)
      0x17, 0x15, 0x1E, 0x2B,
      0x04, 0x05, 0x02, 0x0E,
    ST7735_GMCTRN1,16+DELAY,  // 14: Sparkles and rainbows, 16 args + delay:
      0x0B, 0x14, 0x08, 0x1E, //     (ditto)
      0x22, 0x1D, 0x18, 0x1E,
      0x1B, 0x1A, 0x24, 0x2B,
      0x06, 0x06, 0x02, 0x0F,
      10,                     //     10 ms delay
    ST7735_CASET  , 4      ,  // 15: Column addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 2
      0x00, 0x81,             //     XEND = 129
    ST7735_RASET  , 4      ,  // 16: Row addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 1
      0x00, 0x81,             //     XEND = 160
    ST7735_NORON  ,   DELAY,  // 17: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,   DELAY,  // 18: Main screen turn on, no args, w/delay
      255 },                  //     255 = 500 ms delay

  Rcmd1[] = {                 // Init for 7735R, part 1 (red or green tab)
    15,                       // 15 commands in list:
    ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
      150,                    //     150 ms delay
    ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
      255,                    //     500 ms delay
    ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,       //     Dot inversion mode
      0x01, 0x2C, 0x2D,       //     Line inversion mode
    ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
      0x07,                   //     No inversion
    ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                   //     -4.6V
      0x84,                   //     AUTO mode
    ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
      0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
      0x0A,                   //     Opamp current small
      0x00,                   //     Boost frequency
    ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
      0x8A,                   //     BCLK/2, Opamp current small & Medium low
      0x2A,
    ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
    ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
      0xC8,                   //     row addr/col addr, bottom to top refresh
    ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
      0x05 },                 //     16-bit color

  Rcmd2green[] = {            // Init for 7735R, part 2 (green tab only)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x02,             //     XSTART = 0
      0x00, 0x7F+0x02,        //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x01,             //     XSTART = 0
      0x00, 0x9F+0x01 },      //     XEND = 159
  Rcmd2red[] = {              // Init for 7735R, part 2 (red tab only)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x9F },           //     XEND = 159

  Rcmd2green144[] = {              // Init for 7735R, part 2 (green 1.44 tab)
    2,                        //  2 commands in list:
    ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F,             //     XEND = 127
    ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,             //     XSTART = 0
      0x00, 0x7F },           //     XEND = 127

  Rcmd3[] = {                 // Init for 7735R, part 3 (red or green tab)
    4,                        //  4 commands in list:
    ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
      10,                     //     10 ms delay
    ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
      100 };                  //     100 ms delay

// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void display_commandList(const uint8_t *addr)
{
    uint8_t  numCommands, numArgs;
    uint16_t ms;
    int retval;

    numCommands = pgm_read_byte(addr++);        // Number of commands to follow
    while ( numCommands-- )                     // For each command...
    {
        writecommand(pgm_read_byte(addr++));    //   Read, issue command
        numArgs  = pgm_read_byte(addr++);       //   Number of args to follow
        ms       = numArgs & DELAY;             //   If hibit set, delay follows args
        numArgs &= ~DELAY;                      //   Mask out delay bit
        while ( numArgs-- )                     //   For each argument...
        {
            //writedata(pgm_read_byte(addr++));   //     Read, issue argument
            //writedata(pgm_read_byte(addr++));   //     Read, issue argument
        }

        if ( ms )
        {
            ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
            if ( ms == 255 )
            {
                ms = 500;     // If 255, delay for 500 ms
            }
            delay(ms);
        }
    }
}
#endif

//*****************************************************************************
//
// Static helper functions.
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief IOMCallBack(void).
//
//*****************************************************************************
static void
IOMCallBack(void)
{
    bComplete = true;
}

//*****************************************************************************
//
//! @brief IOMYield(void).
//
//*****************************************************************************
static void
IOMYield(void)
{
    while ( !bComplete );
    bComplete = false;
}

//*****************************************************************************
//
//! @briefThis function starts a SPI transaction using the IOM parameters
//!  specified in u32SPIparms and ui32CSpin.
//!
//! @param psDisplayContext - Pointer to the displey device context structure.
//!                      - The structure contains the IOM parameters ui32Module,
//!                      ui32ChipSelect, and ui32ChipSelectPin.
//! @param pui32Data        - Pointer to the data buffer.
//! @param ui32NumBytes     - Number of bytes to send.
//! @param ui32Options      - IOM command options, e.g. AM_HAL_IOM_RAW.
//
//*****************************************************************************
static void
start_iom(am_devices_display_st7735_t *psDisplayContext,
          uint32_t *pui32Data,
          uint32_t ui32NumBytes,
          uint32_t ui32Options)
{
    //
    // Write data to the display.
    //
    am_hal_iom_queue_spi_write(psDisplayContext->ui32Module,
                         psDisplayContext->ui32ChipSelect,
                         pui32Data,
                         ui32NumBytes,
                         ui32Options,
                         display_st7735_callback);

    display_st7735_yield();

} // start_iom()

//*****************************************************************************
//
//! @brief This function sends a display COMMAND by first setting D/C low.
//! @details The command can be up to 3 bytes (3 bytes is the largest command
//! the displaycan handle).
//!
//!
//! @param u8Cmd0       - Always contains a valid command byte.
//! @param u8Cmd1       - Used for 2 byte and 3 byte commands.  Ignored for 1 byte commands.
//! @param u8Cmd2       - Used for 3 byte commands.  Ignored for 1 or 2 byte commands.
//! @param ui32nBytes   - The number of command bytes to be sent.
//! @param psDisplayContext    - Pointer to the displey device context structure.
//
//*****************************************************************************
static void
send_cmd(uint8_t u8Cmd0, uint8_t u8Cmd1, uint8_t u8Cmd2, uint32_t ui32nBytes,
         am_devices_display_st7735_t *psDisplayContext)
{
    uint32_t ui32Buf[4 / 4];
    uint8_t *pui8Buf = (uint8_t*)ui32Buf;
    uint32_t ui32DCpin = psDisplayContext->ui32DCselect;

    if ( ui32nBytes > 3 )
    {
        //
        // Invalid command.
        //
        return;
    }

    //
    // Now send the device commands bytes.
    // The command will use up to 3 bytes, but must be terminated with a null.
    //
    pui8Buf[0] = u8Cmd0;
    pui8Buf[1] = u8Cmd1;
    pui8Buf[2] = u8Cmd2;
    pui8Buf[ui32nBytes] = 0x00;

    //
    // Take D/C low for commands
    // And set DC state
    //
#if AM_APOLLO3_GPIO
    am_hal_gpio_state_write(ui32DCpin, AM_HAL_GPIO_OUTPUT_TRISTATE_ENABLE);
    am_hal_gpio_state_write(ui32DCpin, AM_HAL_GPIO_OUTPUT_CLEAR);
#else // AM_APOLLO3_GPIO
    AM_HAL_GPIO_ENS(ui32DCpin) = AM_HAL_GPIO_ENS_M(ui32DCpin);
    AM_HAL_GPIO_WTC(ui32DCpin) = AM_HAL_GPIO_WTC_M(ui32DCpin);
#endif // AM_APOLLO3_GPIO
    g_bDCn = false;

    //
    // We have a buffer to send.  Go send the bytes to the IOM.
    //
    start_iom(psDisplayContext, ui32Buf, ui32nBytes, AM_HAL_IOM_RAW);
}

//*****************************************************************************
//
//! @brief This function sends multiple bytes of data to the display by first
//! setting D/C high.
//!
//!
//!  @param pui8Data         - Pointer to the data bytes to be sent to the display.
//!  @param ui32nBytes       - The number of bytes to send to the display.
//!  @param psDisplayContext - Pointer to the displey device context structure.
//
//*****************************************************************************
static void
send_data(uint8_t *pui8Data, uint32_t ui32nBytes,
          am_devices_display_st7735_t *psDisplayContext)
{
    uint8_t *pui8Buf = (uint8_t*)g_ui32IOMBuf;
    uint32_t ix, ui32SendBytes, ui32DCpin;
    uint32_t ui32ByteCnt = ui32nBytes;

    if ( !g_bDCn )
    {
        ui32DCpin = psDisplayContext->ui32DCselect;

        //
        // Take D/C high for data
        //
#if AM_APOLLO3_GPIO
        am_hal_gpio_state_write(ui32DCpin, AM_HAL_GPIO_OUTPUT_TRISTATE_ENABLE);
        am_hal_gpio_state_write(ui32DCpin, AM_HAL_GPIO_OUTPUT_SET);
#else // AM_APOLLO3_GPIO
        AM_HAL_GPIO_ENS(ui32DCpin) = AM_HAL_GPIO_ENS_M(ui32DCpin);
        AM_HAL_GPIO_WTS(ui32DCpin) = AM_HAL_GPIO_WTS_M(ui32DCpin);
#endif // AM_APOLLO3_GPIO

        g_bDCn = true;
    }

    //
    // Now send the device data bytes.
    // The data has to be word-aligned, so we'll need to stuff the bytes into
    // a stack array, which is 64 bytes to correspond to the IOM FIFO size.
    //
    while ( ui32ByteCnt )
    {
        ui32SendBytes = 0;
        for ( ix = 0; ui32ByteCnt > 0 && ix < 64; ix++ )
        {
            pui8Buf[ix] = *pui8Data++;
            ui32ByteCnt--;
            ui32SendBytes++;
        }

        //
        // We have a buffer to send.  Go send the bytes to the IOM.
        //
        start_iom(psDisplayContext, g_ui32IOMBuf, ui32SendBytes, AM_HAL_IOM_RAW);
    }
}

//*****************************************************************************
//
//! @brief This function sets the display for horizontal mode with autoincrement.
//! @details This is the typical mode in which pixels are read or written by rows,
//! left to right then to the next row.  No setting of address is required between
//! accesses.
//!
//! @note Calling this function sets the address to 0,0.
//!
//! @param psDisplayContext  - Pointer to the display device context structure.
//
//*****************************************************************************
static void
set_horz_mode(am_devices_display_st7735_t *psDisplayContext)
{
    //
    // Remap to horizontal mode
    //
    send_cmd(ST7735_CMD2B_SET_REMAP, 0x53, 0, 2, psDisplayContext);

    //
    // Set row address.
    // Start 0, end 95.
    //
    send_cmd(ST7735_CMD3B_SET_ROW_ADDR, 0,
             AM_DISPLAY_ST7735_HEIGHT - 1, 3, psDisplayContext);

    //
    // Set column address.
    // Start from 8th column of driver IC, which is the 0th column for OLED.
    // End at (8+(96/8)).  Each column has 8 pixels.
    //
    send_cmd(ST7735_CMD3B_SET_COL_ADDR, 8,
             8 + (AM_DISPLAY_ST7735_WIDTH / 2) - 1, 3, psDisplayContext);

    //
    // Set the addressing mode state.
    //
    g_ui32AddressMode = ADDR_MODE_HORZ;

    //
    // Make sure we're done
    //
    am_hal_iom_poll_complete(psDisplayContext->ui32Module);
}

// #### INTERNAL BEGIN ####
#if 0
//*****************************************************************************
//
// set_vert_mode() - Set display for vertical access mode.
//
//  This function sets the display for vertical mode with autoincrement.  This
//  mode reads or writes pixels by column.
//
//  @param
//  psDisplayContext    Pointer to the displey device context structure.
//
//*****************************************************************************
static void
set_vert_mode(am_devices_display_st7735_t *psDisplayContext)
{
    //
    // Remap to vertical mode.
    //
    send_cmd(ST7735_CMD2B_SET_REMAP, 0x57, 0, 2, psDisplayContext);

    //
    // Set row address.
    // Start 0, end 95.
    //
    send_cmd(ST7735_CMD3B_SET_ROW_ADDR, 0,
             AM_DISPLAY_ST7735_HEIGHT - 1, 3, psDisplayContext);

    //
    // Start from 8th column of driver IC, which is the 0th column for OLED.
    // End at (8+(96/8)).  Each column has 8 pixels.
    //
    send_cmd(ST7735_CMD3B_SET_COL_ADDR, 8,
             8 + (AM_DISPLAY_ST7735_WIDTH / 2) - 1, 3, psDisplayContext);

    //
    // Set the addressing mode state.
    //
    g_ui32AddressMode = ADDR_MODE_VERT;

    //
    // Make sure we're done
    //
    am_hal_iom_poll_complete(psDisplayContext->ui32Module);
}
#endif
// #### INTERNAL END ####
//*****************************************************************************
//
//!  @briefThis function sets the row number.  All subsequently written data will
//!  start on the specified row.
//!
//! @param psDisplayContext - Pointer to the displey device context structure.
//! @param ui32BegLineNum   - Beginning line number (0-based).
//! @param ui32EndLineNum   - Ending line number (0-based).
//
//*****************************************************************************
static void
set_row(am_devices_display_st7735_t *psDisplayContext,
        uint32_t ui32BegLineNum, uint32_t ui32EndLineNum)
{
    //
    // Set row address.
    // By offseting by 1, a blank line on the display is avoided.
    //
    send_cmd(ST7735_CMD3B_SET_ROW_ADDR, ui32BegLineNum + 1,
             ui32EndLineNum + 1, 3, psDisplayContext);

    //
    // While we're at it, reset the column address.
    //
    send_cmd(QG9696TSWHG04_CMD3B_SET_COL_ADDR, 8,
             8 + (AM_DISPLAY_QG9696TSWHG04_WIDTH / 2) - 1, 3, psDisplayContext);

    //
    // Make sure we're done
    //
    am_hal_iom_poll_complete(psDisplayContext->ui32Module);
}

//*****************************************************************************
//
//  Initialize the display device and driver.
//
//*****************************************************************************
uint32_t
am_devices_display_st7735_init(
                        am_devices_display_st7735_t *psDisplayContext)
{
    uint32_t ui32Ret = 0x01;

    //
    // Initialize the IOM callbacks.
    //
    display_st7735_yield = (am_hal_iom_callback_t) IOMYield;
    display_st7735_callback = (am_hal_iom_callback_t) IOMCallBack;

    if ( (psDisplayContext->ui32DisplayWidth  != AM_DISPLAY_ST7735_WIDTH )  ||
         (psDisplayContext->ui32DisplayHeight != AM_DISPLAY_ST7735_HEIGHT) )
    {
        psDisplayContext->ui32DisplayWidth  = AM_DISPLAY_ST7735_WIDTH;
        psDisplayContext->ui32DisplayHeight = AM_DISPLAY_ST7735_HEIGHT;
        ui32Ret |= 0x02;
    }

    //
    // Return the hardware refresh period for this device (ms).
    // For this particular display, a periodic refresh is not required,
    //  so return the period as 0.
    //
    psDisplayContext->ui32HwRefreshMs = 0;

    //
    // Display background polarity.  The frame buffer handler will use this
    //  attribute to determine the polarity used in the frame buffer.
    //  Set to 1 if the background clears with 1s (0xffs).
    //  Set to 0 if the background clears with 0s (0x00s).
    //
    // The st7735 is cleared with 0x00s, so set to 0.
    //
    psDisplayContext->ui32DisplayBkgndPolarity = ST7735_FB_CLEAR_VALUE & 1;

    //
    // Now send the device initialization commands.
    // Unlock the display to accept commands.
    //
    send_cmd(ST7735_CMD2B_SET_CMD_LOCK, 0x12, 0, 2, psDisplayContext);

    //
    // Set the display off.
    //
    send_cmd(ST7735_CMD1B_DISPLAY_OFF, 0, 0, 1, psDisplayContext);

    //
    // Set re-map.
    //
    send_cmd(ST7735_CMD2B_SET_REMAP, 0x53, 0, 2, psDisplayContext);

    //
    // Set display start line.
    //
    send_cmd(ST7735_CMD2B_SET_START_LINE, 0x00, 0, 2, psDisplayContext);

    //
    // Set display offset.
    //
    send_cmd(ST7735_CMD2B_SET_OFFSET, 0x20, 0, 2, psDisplayContext);

    //
    // Set Normal display.
    //
    send_cmd(ST7735_CMD1B_SETMODE_INVERT, 0, 0, 1, psDisplayContext);

    //
    // Set Multiplex (Mux) ratio.
    //
    send_cmd(ST7735_CMD2B_SET_MULTIPLEX,
             AM_DISPLAY_ST7735_HEIGHT, 0, 2, psDisplayContext);

    //
    // Function select A.
    // Enable internal VDD regulator.
    //
    send_cmd(ST7735_CMD2B_SELECT_FUNC, 0x01, 0, 2, psDisplayContext);

    //
    // Set Contrast.
    //
    send_cmd(ST7735_CMD2B_SET_CONTRAST_CURRENT, 0x49, 0, 2, psDisplayContext);

    //
    // Set Phase Length.
    //
    send_cmd(ST7735_CMD2B_SET_PHASE_LEN, 0x32, 0, 2, psDisplayContext);

    //
    // Set front clock divider/oscillator frequency.
    //
    send_cmd(ST7735_CMD2B_SET_CLK_DIVIDER, 0x51, 0, 2, psDisplayContext);

    //
    // For Brightness enhancement.
    //
    send_cmd(0xB4, 0xB5, 0, 2, psDisplayContext);

    //
    // Set second pre-charge period.
    //
    send_cmd(ST7735_CMD2B_SET_2ND_PRECHARGE, 0x0D, 0, 2, psDisplayContext);

    //
    // Set pre-charge voltage.
    //
    send_cmd(ST7735_CMD2B_SET_PRECHARGE_VOLT, 0x07, 0, 2, psDisplayContext);

    //
    // Set vcomh.
    //
    send_cmd(ST7735_CMD2B_SET_VCOMH_VOLT, 0x07, 0, 2, psDisplayContext);

    //
    // Function selection B.
    //
    send_cmd(ST7735_CMD2B_SELECT_FUNC_B, 0x02, 0, 2, psDisplayContext);

    //
    // Display ON.
    //
    send_cmd(ST7735_CMD1B_DISPLAY_ON, 0, 0, 1, psDisplayContext);

    //
    // Set row address, start 0, end 95.
    //
    send_cmd(ST7735_CMD3B_SET_ROW_ADDR, 0,
             AM_DISPLAY_ST7735_HEIGHT - 1, 3, psDisplayContext);

    //
    // Initialize to horizontal mode.
    //
    set_horz_mode(psDisplayContext);

    if ( g_ui32AddressMode == ADDR_MODE_NONE )
    {
        return ui32Ret;
    }

    return ui32Ret;
} // am_devices_display_st7735_init()

//*****************************************************************************
//
//  Clear the display.
//
//*****************************************************************************
void
am_devices_display_st7735_screen_clear(
                            am_devices_display_st7735_t *psDisplayContext)
{
    int32_t i32x;

    //
    // Begin by clearing the framebuffer
    //
    for (i32x = 0; i32x < (AM_DISPLAY_ST7735_WIDTH / 8) * AM_DISPLAY_ST7735_HEIGHT; i32x++)
    {
        psDisplayContext->pui8Framebuffer[i32x] = ST7735_FB_CLEAR_VALUE;
    }

    //
    // Set to horizontal mode, which also resets the coordinates to 0,0.
    //
    if ( g_ui32AddressMode != ADDR_MODE_HORZ )
    {
        set_horz_mode(psDisplayContext);
    }

    //
    // Now, send the framebuffer to the display.
    // Since this display is 4bpp, but our framebuffer is 1bpp, we need to
    // send the cleared framebuffer 4 times.
    //
    for (i32x = 0; i32x < 4; i32x++)
    {
        send_data(psDisplayContext->pui8Framebuffer,
            AM_DISPLAY_ST7735_WIDTH / 8 * AM_DISPLAY_ST7735_HEIGHT,
            psDisplayContext);
    }

} // am_devices_display_st7735_screen_clear()

//*****************************************************************************
//
//  Perform a hardware refresh of the display.
//
//*****************************************************************************
bool
am_devices_display_st7735_hw_refresh(
                            am_devices_display_st7735_t *psDisplayContext,
                            bool bFullScreenDataRefresh)
{
    if ( bFullScreenDataRefresh )
    {
        //
        // We'll do a complete refresh of the display this time.
        //
        am_devices_display_st7735_lines_copy(psDisplayContext, 0,
                            AM_DISPLAY_ST7735_HEIGHT - 1);
    }

    return true;
} // am_devices_display_st7735_hw_refresh()

//*****************************************************************************
//
//  Copy lines from the framebuffer to the display.
//
//*****************************************************************************
void
am_devices_display_st7735_lines_copy(
                            am_devices_display_st7735_t *psDisplayContext,
                            uint32_t ui32BegLineNum, uint32_t ui32EndLineNum)
{
    uint8_t *pui8FrBuf, *pu8Line4bit;
    int32_t i32row, i32col;
#if ROTATE_90
    uint8_t *pui8FB;
    uint8_t ui8Mask, ui8FBbyte0, ui8FBbyte1, ui8Pix;
#endif // ROTATE_90

    if ( (ui32BegLineNum > ui32EndLineNum)  ||
         (ui32EndLineNum > (AM_DISPLAY_QG9696TSWHG04_WIDTH-1)) )
    {
        return;
    }

    //
    // Set the row that we'll be writing the data to.
    //
    set_row(psDisplayContext, ui32BegLineNum, ui32EndLineNum);

#if ROTATE_90
    //
    // Rotate the image 90 (counter-clockwise). It should be noted that the
    //  following method only works if width=height and if each dimension is
    //  a multiple of 8.
    // To accomplish the rotatation, we'll read the pixels from the binary
    //  bitmap starting at the rightmost edge (column 95), translate the binary
    //  bit to a 4bpp pixel, and send them to the incrementally to the display
    //  device (which is in horizontal mode).  After getting all of the pixels
    //  from the rightmost edge of the binary bitmap, we'll move in 1 pixel
    //  (column 94) and read those, continuing until the entire bitmap has been
    //  read, translated, and sent to the display.
    //

    //
    // Get a byte pointer to the line buffer.
    //
    pu8Line4bit = (uint8_t*)g_FB4bitLine;

    //
    // Get ptr to the beginning of the framebuffer.
    //
    pui8FrBuf = psDisplayContext->pui8Framebuffer;

    //
    // Begin the binary bitmap to 4bpp translation loop.
    // This loop rotates the image by 90 from its normal orientation.
    //
    for ( i32row = ui32BegLineNum; i32row <= ui32EndLineNum; i32row++ )
    {
        //
        // Get a ptr to the byte in the first row of the binary bitmap
        // containing the pixel that we're interested in.
        //
        pui8FB = pui8FrBuf + ((AM_DISPLAY_QG9696TSWHG04_WIDTH / 8) - 1) -
                 (i32row / 8);

        //
        // Compute the bitmask needed for the pixels in this row.
        // The LSB of the binary bitmap is bit7 in MCU memory.
        //
        ui8Mask = g_ui8MaskTbl[i32row & 0x7];

        for ( i32col = 0; i32col < AM_DISPLAY_QG9696TSWHG04_WIDTH / 2; i32col++ )
        {
            //
            // Translate 2 pixels at a time.
            // Get the byte containing the pixel from the current row, and then
            // the byte with the 2nd pixel from the next row.
            //
            ui8FBbyte0 = *pui8FB;
            ui8FBbyte1 = *(pui8FB + (AM_DISPLAY_QG9696TSWHG04_WIDTH / 8));

            //
            // Translate the two binary-bitmask pixels to 4bpp pixels.
            //
            ui8Pix  = (ui8FBbyte0 & ui8Mask) ? 0x0F : 0x00;
            ui8Pix |= (ui8FBbyte1 & ui8Mask) ? 0xF0 : 0x00;

            //
            // Copy these two 4bpp pixels to the line buffer.
            //
            pu8Line4bit[i32col] = ui8Pix;

            //
            // Increment the framebuffer ptr to the next rows to be translated.
            //
            pui8FB += (AM_DISPLAY_QG9696TSWHG04_WIDTH / 8) * 2;
        }

        //
        // Now that we have a line of 4bpp data, write it to the display.
        //
        send_data((uint8_t*)g_FB4bitLine,
                  AM_DISPLAY_QG9696TSWHG04_WIDTH / 8 * 4, psDisplayContext);
    }
#else // Normal orientation (no rotation)
    //
    // Begin the binary bitmap to 4bpp translation loop.
    // This loop displays the image in its normal orientation.
    //
    pui8FrBuf = &psDisplayContext->pui8Framebuffer[ui32BegLineNum *
                (AM_DISPLAY_QG9696TSWHG04_WIDTH / 8)];

    for ( i32row = ui32BegLineNum; i32row <= ui32EndLineNum; i32row++ )
    {
        //
        // Translate one line worth of monochrome data to 4bpp data.
        // Translate 4 pixels at a time via the lookup table.
        //
        pu8Line4bit = (uint8_t*)g_FB4bitLine;
        for ( i32col = 0; i32col < AM_DISPLAY_QG9696TSWHG04_WIDTH; i32col += 8)
        {
            uint8_t u8FBbyte = *pui8FrBuf++;
            uint32_t u32FB4bit;
            u32FB4bit  = g_XlateNibble[u8FBbyte & 0xF];
            *pu8Line4bit++ = (u32FB4bit >> 8) & 0xFF;
            *pu8Line4bit++ = (u32FB4bit >> 0) & 0xFF;
            u8FBbyte >>= 4;
            u32FB4bit  = g_XlateNibble[u8FBbyte & 0xF];
            *pu8Line4bit++ = (u32FB4bit >> 8) & 0xFF;
            *pu8Line4bit++ = (u32FB4bit >> 0) & 0xFF;
        }

        //
        // Now that we have a line of 4bpp data, write it to the display.
        //
        send_data((uint8_t*)g_FB4bitLine,
                  AM_DISPLAY_QG9696TSWHG04_WIDTH / 8 * 4, psDisplayContext);
    }
#endif // ROTATE_90

    //
    // Set rows to default to the entire screen.
    //
    set_row(psDisplayContext, 0, AM_DISPLAY_QG9696TSWHG04_WIDTH - 1);

} // am_devices_display_st7735_lines_copy()

//*****************************************************************************
//
//  Configure the callbacks for the IOM to enable nonblocking functionality.
//
//*****************************************************************************
void
am_devices_display_st7735_callbacks_config(am_hal_iom_callback_t pfnYield,
                                           am_hal_iom_callback_t pfnCallback)
{
    display_st7735_yield = pfnYield;
    display_st7735_callback = pfnCallback;
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

