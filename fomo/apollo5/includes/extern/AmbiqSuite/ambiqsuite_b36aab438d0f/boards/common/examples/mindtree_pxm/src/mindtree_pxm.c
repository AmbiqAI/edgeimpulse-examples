//*****************************************************************************
//
//! @file mindtree_test.c
//!
//! @brief Test application to see if Mindtree will compile.
//!
//! Detailed description.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "stdio.h"
#include "stdarg.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "FreeRTOS.h"
#include "task.h"

#define UART_DEBUG

extern int appl_init(void);

#ifdef UART_DEBUG
#define AM_FTOA_ERR_VAL_TOO_SMALL   -1
#define AM_FTOA_ERR_VAL_TOO_LARGE   -2
static char g_uartprfbuf[256];
// Flag to do conversion of '\n' to '\n\r' in sprintf()
static bool g_buartTxtXlate = false;
unsigned char r_byte;
char rbuf[128];

int32_t i32Module = AM_BSP_UART_PRINT_INST;

//*****************************************************************************
//
// UART configuration settings.
//
//*****************************************************************************
am_hal_uart_config_t g_sUartConfig =
{
    .ui32BaudRate = 115200,
    .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
    .bTwoStopBits = false,
    .ui32Parity   = AM_HAL_UART_PARITY_NONE,
    .ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE,
};

//*****************************************************************************
//
// Initialize the UART
//
//*****************************************************************************
void
uart_init(int32_t i32Module)
{
    //
    // Make sure the UART RX and TX pins are enabled.
    //
    am_bsp_pin_enable(COM_UART_TX);
    am_bsp_pin_enable(COM_UART_RX);

    //
    // Power on the selected UART
    //
    am_hal_uart_pwrctrl_enable(i32Module);

    //
    // Start the UART interface, apply the desired configuration settings, and
    // enable the FIFOs.
    //
    am_hal_uart_clock_enable(i32Module);

    //
    // Disable the UART before configuring it.
    //
    am_hal_uart_disable(i32Module);

    //
    // Configure the UART.
    //
    am_hal_uart_config(i32Module, &g_sUartConfig);

    //
    // Enable the UART FIFO.
    //
    am_hal_uart_fifo_config(i32Module, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);

    //
    // Enable the UART.
    //
    am_hal_uart_enable(i32Module);
}

//*****************************************************************************
//
// Enable the UART
//
//*****************************************************************************
void
uart_enable(int32_t i32Module)
{
    //
    // Enable the UART clock.
    //
    am_hal_uart_clock_enable(i32Module);

    //
    // Enable the UART.
    //
    am_hal_uart_enable(i32Module);

    //
    // Enable the UART pins.
    //
    am_bsp_pin_enable(COM_UART_TX);
    am_bsp_pin_enable(COM_UART_RX);
}

//*****************************************************************************
//
// Disable the UART
//
//*****************************************************************************
void
uart_disable(int32_t i32Module)
{
      //
      // Clear all interrupts before sleeping as having a pending UART interrupt
      // burns power.
      //
      am_hal_uart_int_clear(i32Module, 0xFFFFFFFF);

      //
      // Disable the UART.
      //
      am_hal_uart_disable(i32Module);

      //
      // Disable the UART pins.
      //
      am_bsp_pin_disable(COM_UART_TX);
      am_bsp_pin_disable(COM_UART_RX);

      //
      // Disable the UART clock.
      //
      am_hal_uart_clock_disable(i32Module);
}

//*****************************************************************************
//
// Transmit delay waits for busy bit to clear to allow
// for a transmission to fully complete before proceeding.
//
//*****************************************************************************
void
uart_transmit_delay(int32_t i32Module)
{
    //
    // Wait until busy bit clears to make sure UART fully transmitted last byte
    //
    while ( am_hal_uart_flags_get(i32Module) & AM_HAL_UART_FR_BUSY );
}

//*****************************************************************************
//
//  Divide an unsigned 32-bit value by 10.
//
//  Note: Adapted from Ch10 of Hackers Delight (hackersdelight.org).
//
//*****************************************************************************
static uint64_t
divu64_10(uint64_t ui64Val)
{
    uint64_t q64, r64;
    uint32_t q32, r32, ui32Val;

    //
    // If a 32-bit value, use the more optimal 32-bit routine.
    //
    if ( ui64Val >> 32 )
    {
        q64 = (ui64Val>>1) + (ui64Val>>2);
        q64 += (q64 >> 4);
        q64 += (q64 >> 8);
        q64 += (q64 >> 16);
        q64 += (q64 >> 32);
        q64 >>= 3;
        r64 = ui64Val - q64*10;
        return q64 + ((r64 + 6) >> 4);
    }
    else
    {
        ui32Val = (uint32_t)(ui64Val & 0xffffffff);
        q32 = (ui32Val>>1) + (ui32Val>>2);
        q32 += (q32 >> 4);
        q32 += (q32 >> 8);
        q32 += (q32 >> 16);
        q32 >>= 3;
        r32 = ui32Val - q32*10;
        return (uint64_t)(q32 + ((r32 + 6) >> 4));
    }
}

//*****************************************************************************
//
// Return the number of decimal digits in an uint64_t.
//
// example: 10000 return 5, 123 returns 3.
//
//*****************************************************************************
static int
ndigits_in_u64(uint64_t ui64Val)
{
    int iNDigits = ui64Val ? 0 : 1;

    while ( ui64Val )
    {
        //
        // ui32Val /= 10;
        //
        ui64Val = divu64_10(ui64Val);
        ++iNDigits;
    }

    return iNDigits;
}

//*****************************************************************************
//
// Return the number of decimal digits in a 64-bit integer.
//
// Note: Does not include the '-' sign.
//
// example: -3 returns 1, 3 returns 1, 15 returns 2, -15 returns 2, ...
//
//*****************************************************************************
static int
ndigits_in_i64(int64_t i64Val)
{
    if ( i64Val < 0 )
    {
        //
        // Get absolute value
        //
        i64Val = -i64Val;
    }

    return ndigits_in_u64((uint64_t) i64Val);
}

//*****************************************************************************
//
// Return the number of hex digits in an uint64_t.
//
//*****************************************************************************
static int
ndigits_in_hex(uint64_t ui64Val)
{
    int iDigits = ui64Val ? 0 : 1;

    while ( ui64Val )
    {
        ui64Val >>= 4;
        ++iDigits;
    }

    return iDigits;
}

//*****************************************************************************
//
// Converts a string representing a decimal value to an int32_t.
//
// Returns the int32_t integer value.
//
// Note: If a count of the number of chars is desired, then provide
// pui32CharCnt.
//
//*****************************************************************************
static uint32_t
decstr_to_int(const char *pcStr, uint32_t *pui32CharCnt)
{
    bool bNeg = false;
    uint32_t ui32Val = 0, uCnt = 0;

    if ( *pcStr == '-')
    {
        bNeg = true;
        pcStr++;
        uCnt++;
    }

    while ( *pcStr >= '0'  &&  *pcStr <= '9' )
    {
        ++uCnt;

        //
        // Multiply accumulated value by 10.
        //
        ui32Val *= 10;

        //
        // Add in the new low digit.
        //
        ui32Val += (*pcStr - '0');
        pcStr++;
    }

    if ( pui32CharCnt )
    {
        *pui32CharCnt = uCnt;
    }

    return bNeg ? -ui32Val : ui32Val;
}

//*****************************************************************************
//
// Converts ui64Val to a string.
// Note: pcBuf[] must be sized for a minimum of 21 characters.
//
// Returns the number of decimal digits in the string.
//
// NOTE: If pcBuf is NULL, will compute a return ui64Val only (no chars
// written).
//
//*****************************************************************************
static int
uint64_to_str(uint64_t ui64Val, char *pcBuf)
{
    char tbuf[25];
    int ix = 0, iNumDig = 0;
    unsigned uMod;
    uint64_t u64Tmp;

    do
    {
        //
        // Divide by 10
        //
        u64Tmp = divu64_10(ui64Val);

        //
        // Get modulus
        //
        uMod = ui64Val - (u64Tmp * 10);

        tbuf[ix++] = uMod + '0';
        ui64Val = u64Tmp;
    } while ( ui64Val );

    //
    // Save the total number of digits
    //
    iNumDig = ix;

    //
    // Now, reverse the buffer when saving to the caller's buffer.
    //
    if ( pcBuf )
    {
        while ( ix-- )
        {
            *pcBuf++ = tbuf[ix];
        }

        //
        // Terminate the caller's buffer
        //
        *pcBuf = 0x00;
    }

    return iNumDig;
}

//*****************************************************************************
//
// Converts ui64Val to a hex string.  Alpha chars are lower case.
// Input:
//  ui64Val = Value to be converted.
//  pcBuf[] must be sized for a minimum of 17 characters.
//
// Returns the number of hex digits required for ui64Val (does not
//  include the terminating NULL char in the string).
//
// NOTE: If pcBuf is NULL, will compute a return value only (no chars
// written).
//
//*****************************************************************************
static int
uint64_to_hexstr(uint64_t ui64Val, char *pcBuf, bool bLower)
{
    int iNumDig, ix = 0;
    char cCh, tbuf[20];

    if ( ui64Val == 0 )
    {
        tbuf[ix++] = '0';   // Print a '0'
    }

    while ( ui64Val )
    {
        cCh = ui64Val & 0xf;

        //
        // Alpha character
        //
        if ( cCh > 9 )
        {
            cCh += bLower ? 0x27 : 0x7;
        }

        tbuf[ix++] = cCh + '0';
        ui64Val >>= 4;
    }

    //
    // Save the total number of digits
    //
    iNumDig = ix;

    //
    // Now, reverse the buffer when saving to the callers buffer.
    //
    if (pcBuf)
    {
        while (ix--)
        {
            *pcBuf++ = tbuf[ix];
        }

        //
        // Terminate the caller's buffer
        //
        *pcBuf = 0;
    }

    return iNumDig;
}

//*****************************************************************************
//
// Return length of the given string.
//
//*****************************************************************************
static uint32_t
simple_strlen(char *pcBuf)
{
    uint32_t ui32RetVal = 0;
    if ( !pcBuf )
    {
        return ui32RetVal;
    }

    while ( *pcBuf++ )
    {
        ui32RetVal++;
    }
    return ui32RetVal;
}

//*****************************************************************************
//
// Pad a string buffer with pad characters.
//
//*****************************************************************************
static int32_t
padbuffer(char *pcBuf, uint8_t cPadChar, int32_t i32NumChars)
{
    int32_t i32Cnt = 0;

    if ( i32NumChars <= 0 )
    {
        return i32Cnt;
    }

    while ( i32NumChars-- )
    {
        if ( pcBuf )
        {
            *pcBuf++ = cPadChar;
        }
        i32Cnt++;
    }

    return i32Cnt;
}

//*****************************************************************************
//
//  Float to ASCII text. A basic implementation for providing support for
//  single-precision %f.
//
//  param
//      fValue     = Float value to be converted.
//      pcBuf      = Buffer to place string AND input of buffer size.
//      iPrecision = Desired number of decimal places.
//      IMPORTANT: On entry, the first 32-bit word of pcBuf must
//                 contain the size (in bytes) of the buffer!
//                 The recommended size is at least 16 bytes.
//
//  This function performs a basic translation of a floating point single
//  precision value to a string.
//
//  return Number of chars printed to the buffer.
//
//*****************************************************************************
#define AM_FTOA_ERR_VAL_TOO_SMALL   -1
#define AM_FTOA_ERR_VAL_TOO_LARGE   -2
#define AM_FTOA_ERR_BUFSIZE         -3

typedef union
{
    int32_t I32;
    float F;
} i32fl_t;

static int ftoa(float fValue, char *pcBuf, int iPrecision)
{
    i32fl_t unFloatValue;
    int iExp2, iBufSize;
    int32_t i32Significand, i32IntPart, i32FracPart;
    char *pcBufInitial, *pcBuftmp;

    iBufSize = *(uint32_t*)pcBuf;
    if (iBufSize < 4)
    {
        return AM_FTOA_ERR_BUFSIZE;
    }

    if (fValue == 0.0f)
    {
        // "0.0"
        *(uint32_t*)pcBuf = 0x00 << 24 | ('0' << 16) | ('.' << 8) | ('0' << 0);
        return 3;
    }

    pcBufInitial = pcBuf;

    unFloatValue.F = fValue;

    iExp2 = ((unFloatValue.I32 >> 23) & 0x000000FF) - 127;
    i32Significand = (unFloatValue.I32 & 0x00FFFFFF) | 0x00800000;
    i32FracPart = 0;
    i32IntPart = 0;

    if (iExp2 >= 31)
    {
        return AM_FTOA_ERR_VAL_TOO_LARGE;
    }
    else if (iExp2 < -23)
    {
        return AM_FTOA_ERR_VAL_TOO_SMALL;
    }
    else if (iExp2 >= 23)
    {
        i32IntPart = i32Significand << (iExp2 - 23);
    }
    else if (iExp2 >= 0)
    {
        i32IntPart = i32Significand >> (23 - iExp2);
        i32FracPart = (i32Significand << (iExp2 + 1)) & 0x00FFFFFF;
    }
    else // if (iExp2 < 0)
    {
        i32FracPart = (i32Significand & 0x00FFFFFF) >> -(iExp2 + 1);
    }

    if (unFloatValue.I32 < 0)
    {
        *pcBuf++ = '-';
    }

    if (i32IntPart == 0)
    {
        *pcBuf++ = '0';
    }
    else
    {
        if (i32IntPart > 0)
        {
            uint64_to_str(i32IntPart, pcBuf);
        }
        else
        {
            *pcBuf++ = '-';
            uint64_to_str(-i32IntPart, pcBuf);
        }
        while (*pcBuf)    // Get to end of new string
        {
            pcBuf++;
        }
    }

    //
    // Now, begin the fractional part
    //
    *pcBuf++ = '.';

    if (i32FracPart == 0)
    {
        *pcBuf++ = '0';
    }
    else
    {
        int jx, iMax;

        iMax = iBufSize - (pcBuf - pcBufInitial) - 1;
        iMax = (iMax > iPrecision) ? iPrecision : iMax;

        for (jx = 0; jx < iMax; jx++)
        {
            i32FracPart *= 10;
            *pcBuf++ = (i32FracPart >> 24) + '0';
            i32FracPart &= 0x00FFFFFF;
        }

        //
        // Per the printf spec, the number of digits printed to the right of the
        // decimal point (i.e. iPrecision) should be rounded.
        // Some examples:
        // Value        iPrecision          Formatted value
        // 1.36399      Unspecified (6)     1.363990
        // 1.36399      3                   1.364
        // 1.36399      4                   1.3640
        // 1.36399      5                   1.36399
        // 1.363994     Unspecified (6)     1.363994
        // 1.363994     3                   1.364
        // 1.363994     4                   1.3640
        // 1.363994     5                   1.36399
        // 1.363995     Unspecified (6)     1.363995
        // 1.363995     3                   1.364
        // 1.363995     4                   1.3640
        // 1.363995     5                   1.36400
        // 1.996        Unspecified (6)     1.996000
        // 1.996        2                   2.00
        // 1.996        3                   1.996
        // 1.996        4                   1.9960
        //
        // To determine whether to round up, we'll look at what the next
        // decimal value would have been.
        //
        if ( ((i32FracPart * 10) >> 24) >= 5 )
        {
            //
            // Yes, we need to round up.
            // Go back through the string and make adjustments as necessary.
            //
            pcBuftmp = pcBuf - 1;
            while ( pcBuftmp >= pcBufInitial )
            {
                if ( *pcBuftmp == '.' )
                {
                }
                else if ( *pcBuftmp == '9' )
                {
                    *pcBuftmp = '0';
                }
                else
                {
                    *pcBuftmp += 1;
                    break;
                }
                pcBuftmp--;
            }
        }
    }

    //
    // Terminate the string and we're done
    //
    *pcBuf = 0x00;

    return (pcBuf - pcBufInitial);
} // ftoa()

uint32_t
am_uart_vsprintf(char *pcBuf, const char *pcFmt, va_list pArgs)
{
    char *pcStr;
    uint64_t ui64Val;
    int64_t i64Val;
    uint32_t ui32NumChars, ui32CharCnt = 0;
    int iWidth, iVal, iPrecision;
    uint8_t ui8CharSpecifier, ui8PadChar;
    bool bLower, bLongLong, bNeg;
    uint32_t ui32strlen = 0;

    while ( *pcFmt != 0x0 )
    {
        iPrecision = 6;             // printf() default precision for %f is 6

        if ( *pcFmt != '%' )
        {
            //
            // Accumulate the string portion of the format specification.
            //
            if ( pcBuf )
            {
                // If '\n', convert to '\r\n'
                if ( *pcFmt == '\n'  &&  g_buartTxtXlate )
                {
                    *pcBuf++ = '\r';
                    ++ui32CharCnt;
                }
                *pcBuf++ = *pcFmt;
            }

            ++pcFmt;
            ++ui32CharCnt;
            continue;
        }

        //
        // Handle the specifier.
        //
        ++pcFmt;
        bLower = bLongLong = false;

        //
        // Default to space as ui8PadChar
        //
        ui8PadChar = ' ';

        if ( *pcFmt == '0' )
        {
            ui8PadChar = '0';
            ++pcFmt;
        }

        //
        // Width specifier
        //
        iWidth = decstr_to_int(pcFmt, &ui32NumChars);
        pcFmt += ui32NumChars;

        //
        // For now, only support a negative width specifier for %s
        //
        if ( ( *pcFmt != 's' )  &&  ( iWidth < 0 ) )
        {
            iWidth = -iWidth;
        }

        //
        // Check for precision specifier
        //
        if (*pcFmt == '.')
        {
            ++pcFmt;
            iPrecision = decstr_to_int(pcFmt, &ui32NumChars);
            pcFmt += ui32NumChars;
        }

        //
        // Check for the long or long long length field sub-specifiers, 'l' or
        // 'll', which must be a modifier for either 'd', 'i', 'u', 'x', or 'X'
        // (or even 'o', which is not currently supported). Other sub-specifiers
        // like 'hh','h', etc. are not currently handled.
        // Note - 'l' is used in Coremark, a primary reason it's supported here.
        //
        if ( *pcFmt == 'l' )
        {
            pcFmt++;
            if ( *pcFmt == 'l' )    // "ll" (long long)
            {
                pcFmt++;
                bLongLong = true;
            }
        }

        switch ( *pcFmt )
        {
            case 'c':
                ui8CharSpecifier = va_arg(pArgs, uint32_t);

                if ( pcBuf )
                {
                    *pcBuf++ = ui8CharSpecifier;
                }

                ++ui32CharCnt;
                break;

            case 's':
                pcStr = va_arg(pArgs, char *);

                //
                // For %s, we support the width specifier. If iWidth is negative
                // the string is left-aligned (padding on the right).  Otherwise
                // the string is padded at the beginning with spaces.
                //
                ui32strlen = simple_strlen(pcStr);
                if ( iWidth > 0 )
                {
                    // Pad the beginning of the string (right-aligned).
                    if ( ui32strlen < iWidth )
                    {
                        // String needs some padding.
                        iWidth -= ui32strlen;
                        iWidth = padbuffer(pcBuf, ui8PadChar, iWidth);
                        pcBuf += pcBuf ? iWidth : 0;
                        ui32CharCnt += iWidth;
                        iWidth = 0;
                    }
                }

                while (*pcStr != 0x0)
                {
                    if ( pcBuf )
                    {
                        *pcBuf++ = *pcStr;
                    }

                    ++pcStr;
                    ++ui32CharCnt;
                }

                if ( iWidth )
                {
                    iWidth = -iWidth;

                    // Pad the end of the string (left-aligned).
                    if ( ui32strlen < iWidth )
                    {
                        // String needs some padding.
                        iWidth -= ui32strlen;
                        iWidth = padbuffer(pcBuf, ui8PadChar, iWidth);
                        pcBuf += pcBuf ? iWidth : 0;
                        ui32CharCnt += iWidth;
                        iWidth = 0;
                    }
                }
                break;

            case 'x':
                bLower = true;
            case 'X':
                ui64Val = bLongLong ? va_arg(pArgs, uint64_t) :
                                      va_arg(pArgs, uint32_t);

                if ( iWidth )
                {
                    //
                    // Compute # of leading chars
                    //
                    iWidth -= ndigits_in_hex(ui64Val);

                    iWidth = padbuffer(pcBuf, ui8PadChar, iWidth);
                    pcBuf += pcBuf ? iWidth : 0;
                    ui32CharCnt += iWidth;
                    iWidth = 0;
                }

                iVal = uint64_to_hexstr(ui64Val, pcBuf, bLower);

                if ( pcBuf )
                {
                    pcBuf += iVal;
                }

                ui32CharCnt += iVal;
                break;

            case 'u':
                ui64Val = bLongLong ? va_arg(pArgs, uint64_t) :
                                      va_arg(pArgs, uint32_t);

                if ( iWidth )
                {
                    //
                    // We need to pad the beginning of the value.
                    // Compute # of leading chars
                    //
                    iWidth -= ndigits_in_u64(ui64Val);

                    iWidth = padbuffer(pcBuf, ui8PadChar, iWidth);
                    pcBuf += pcBuf ? iWidth : 0;
                    ui32CharCnt += iWidth;
                    iWidth = 0;
                }

                iVal = uint64_to_str(ui64Val, pcBuf);

                if ( pcBuf )
                {
                    pcBuf += iVal;
                }

                ui32CharCnt += iVal;
                break;

            case 'd':
            case 'i':
                //
                // Output for a negative number, for example, -5:
                //   %d:-5
                //  %5d:   -5
                // %05d:-0005
                //
                i64Val = bLongLong ? va_arg(pArgs, int64_t) :
                                     va_arg(pArgs, int32_t);

                //
                // Get absolute value
                //
                if ( i64Val < 0 )
                {
                    ui64Val = -i64Val;          // Get absolute value
                    bNeg = true;
                }
                else
                {
                    ui64Val = i64Val;
                    bNeg = false;
                }

                if ( iWidth )
                {
                    //
                    // We need to pad the beginning of the value.
                    // Compute # of leading chars
                    //
                    iWidth -= ndigits_in_i64(ui64Val);

                    if ( bNeg )
                    {
                        --iWidth;

                        //
                        // Allow for the negative sign
                        //
                        if ( ui8PadChar == '0' )
                        {
                            //
                            // Print the neg sign BEFORE the leading zeros
                            //
                            if ( pcBuf )
                            {
                                *pcBuf++ = '-';
                            }

                            ++ui32CharCnt;
                        }
                    }

                    iWidth = padbuffer(pcBuf, ui8PadChar, iWidth);
                    pcBuf += pcBuf ? iWidth : 0;
                    ui32CharCnt += iWidth;
                    iWidth = 0;

                    if ( bNeg  &&  (ui8PadChar == ' ') )
                    {
                        //
                        // Print the neg sign AFTER the leading blanks
                        //
                        if ( pcBuf )
                        {
                            *pcBuf++ = '-';
                        }

                        ++ui32CharCnt;
                    }
                }
                else
                {
                    if ( bNeg )
                    {
                        if ( pcBuf )
                        {
                            *pcBuf++ = '-';
                        }
                        ++ui32CharCnt;
                    }
                }

                iVal = uint64_to_str(ui64Val, pcBuf);

                if ( pcBuf )
                {
                    pcBuf += iVal;
                }

                ui32CharCnt += iVal;
                break;


            case 'f':
            case 'F':
                if ( pcBuf )
                {
                    float fValue = va_arg(pArgs, double);

                    //
                    // pcBuf is an input (size of buffer) and also an output of ftoa()
                    //
                    *(uint32_t*)pcBuf = 20;

                    iVal = ftoa(fValue, pcBuf, iPrecision);
                    if ( iVal < 0 )
                    {
                        uint32_t u32PrntErrVal;
                        if ( iVal == AM_FTOA_ERR_VAL_TOO_SMALL )
                        {
                            u32PrntErrVal = (0x00 << 24) | ('0' << 16) |
                                            ('.' << 8)   | ('0' << 0);  // "0.0"
                        }
                        else if ( iVal == AM_FTOA_ERR_VAL_TOO_LARGE )
                        {
                            u32PrntErrVal = (0x00 << 24) | ('#' << 16) |
                                            ('.' << 8)   | ('#' << 0);  // "#.#"
                        }
                        else
                        {
                            u32PrntErrVal = (0x00 << 24) | ('?' << 16) |
                                            ('.' << 8)   | ('?' << 0);  // "?.?"
                        }
                        *(uint32_t*)pcBuf = u32PrntErrVal;
                        iVal = 3;
                    }
                    ui32CharCnt += iVal;
                    pcBuf += iVal;
                }
                break;

            //
            // Invalid specifier character
            // For non-handled specifiers, we'll just print the character.
            // e.g. this will allow the normal printing of a '%' using
            // "%%".
            //
            default:
                if ( pcBuf )
                {
                    *pcBuf++ = *pcFmt;
                }

                ++ui32CharCnt;
                break;

        } // switch()

        //
        // Bump the format specification to the next character
        //
        ++pcFmt;

    } // while ()

    //
    // Terminate the string
    //
    if ( pcBuf )
    {
        *pcBuf = 0x0;
    }

    return (ui32CharCnt);
}


uint32_t
am_uart_printf(const char *pcFmt, ...)
{
    uint32_t ui32NumChars;

    //
    // Convert to the desired string.
    //
    va_list pArgs;
    va_start(pArgs, pcFmt);
    ui32NumChars = am_uart_vsprintf(g_uartprfbuf, pcFmt, pArgs);
    va_end(pArgs);

    //
    // This is where we print the buffer to the configured interface.
    //
    //g_pfnCharPrint(g_uartprfbuf);
    am_bsp_uart_string_print(g_uartprfbuf);

    //
    // return the number of characters printed.
    //
    return ui32NumChars;
}
uint32_t am_uart_scanf(const char *pcFmt, ...)
{

  char r_byte;
  int  count;
  unsigned int c_count = 0;
  va_list pArgs;
  rbuf[c_count] = 0;
  while(1)
  {

    am_hal_uart_char_receive_polled(i32Module, &r_byte);

    //am_hal_uart_char_transmit_polled(i32Module, r_byte);

    if ( r_byte == 0x0d || r_byte == 0x0a )
    {

      rbuf[c_count] = 0; //EOF
      break;
    }
    rbuf[c_count++] = r_byte;


  }
  va_start(pArgs, pcFmt);
  count = vsscanf(rbuf, pcFmt, pArgs);
  va_end(pArgs);
  return (count);
}
#endif
//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void
am_gpio_isr(void)
{
    uint64_t ui64Status;

    //
    // Read and clear the GPIO interrupt status.
    //
    ui64Status = am_hal_gpio_int_status_get(false);
    am_hal_gpio_int_clear(ui64Status);

    //
    // Call the individual pin interrupt handlers for any pin that triggered an
    // interrupt.
    //
    am_hal_gpio_int_service(ui64Status);
}

/* Task to be created. */
void vPXMTask( void * pvParameters )
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. */
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    appl_init();
}

/* Function that creates a task. */
void vCreatePXMTask( void )
{
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    vPXMTask,           /* Function that implements the task. */
                    "PXMTask",          /* Text name for the task. */
                    512,                /* Stack size in words, not bytes. */
                    ( void * ) 1,       /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,   /* Priority at which the task is created. */
                    &xHandle );         /* Used to pass out the created task's handle. */

    if ( xReturned == pdPASS )
    {
        /* The task was created.  Use the task's handle to delete the task. */
        //vTaskDelete( xHandle );
    }
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    //
    // Default setup.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);
    am_bsp_low_power_init();

    //
    // Enable ITM for debug.
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);
    am_bsp_pin_enable(ITM_SWO);
    am_hal_itm_enable();
    am_bsp_debug_printf_enable();

    //
    // Initialize and Enable the UART.
    //
    uart_init(i32Module);

    //
    // Print the banner.
    //
    am_util_stdio_printf("mindtree proximity monitor example\r\n");
    uart_transmit_delay(i32Module);

    vCreatePXMTask();
    vTaskStartScheduler();

    //
    // Never reach here.
    //
    while (1)
    {
    }
}
