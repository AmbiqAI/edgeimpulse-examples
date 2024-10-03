//*****************************************************************************
//
//! @file FlashTest.c
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

/******************************************************************************/
/*  This file is part of the uVision/ARM development tools                    */
/* Copyright (c) 2010 KEIL - An ARM Company. All rights reserved.             */
/******************************************************************************/
/*                                                                            */
/*  TEST.C:  Test for Flash Programming Functions                             */
/*                                                                            */
/******************************************************************************/

#include "FlashOS.h"                          // FlashOS Structures


#define M8(adr) (*((volatile unsigned char  *) (adr)))

extern struct FlashDevice const FlashDevice;


volatile int  ret;                                  // Return Code
unsigned char g_buf[8*1024];                        // Programming Buffer



/*----------------------------------------------------------------------------
  Main Function
 *----------------------------------------------------------------------------*/
int main (void)
{
    unsigned long n;

#if 0
#define M32(adr) (*((volatile unsigned long  *) (adr)))
    M32(0x400401FC) = 0x61EBBE3C;
    M32(0x400400F8) = 0xAA16A508;
    M32(0x40040144) = 0x5C13A663;
    M32(0x40040278) = 0x49B43703;
    M32(0x40040018) = 0xFFFFFFFF; // SKUOVERRIDE
    M32(0x400401FC) = 0xFFFFFFFF;
#endif

    // Programming Test Pattern
    for (n = 0; n < FlashDevice.szPage; n++)
    {
        g_buf[n] = (unsigned char)n;
    }

    /* Test ProgramPage Function -----------------------------------------------*/
    ret |= Init(                                    // Initialize Flash Programming Functions
                0x00400000,                         // Device base address = 0x00000000
                48000000,                           // Clock frequency = 48MHz
                2);                                 // Function Code: 1-Erase, 2-Program, 3- Verify

    for (n = FlashDevice.DevAdr; n < (FlashDevice.DevAdr + FlashDevice.szDev); n += FlashDevice.szPage)
    {
        ret |= ProgramPage(n, FlashDevice.szPage, g_buf);   // Test Page Programming
    }

                                                    // Verify Programm Page
    for (n = 0; n < FlashDevice.szDev; n++)
    {
        if (M8(FlashDevice.DevAdr + n) != ((unsigned char)n))
        {
            ret = 1;                                // Error
            break;
        }
    }

    ret |= UnInit (2);                              // Uninitialize Flash Programming Functions


    /* Test EraseSector Function -----------------------------------------------*/
    ret  = Init(                                    // Initialize Flash Programming Functions
                0x00400000,                         // Device base address = 0x00000000
                48000000,                           // Clock frequency = 48MHz
                1);                                 // Function Code: 1-Erase, 2-Program, 3- Verify

    for (n = FlashDevice.DevAdr; n < (FlashDevice.DevAdr + FlashDevice.szDev); n += FlashDevice.szPage)
    {
        ret |= EraseSector(n);                      // Test Sector Erase
    }

    // Verify Erase
//  for (n = 0; n < 0x80000; n++)
    for (n = 0; n < FlashDevice.szDev; n++)
    {
        if (M8(FlashDevice.DevAdr + n) != FlashDevice.valEmpty)
        {
            ret = 1;                                // Error
            break;
        }
    }

    ret |= UnInit (1);                              // Uninitialize Flash Programming Functions


    /* Test ProgramPage Function -----------------------------------------------*/
    ret |= Init(                                    // Initialize Flash Programming Functions
                0x00400000,                         // Device base address = 0x00000000
                48000000,                           // Clock frequency = 48MHz
                2);                                 // Function Code: 1-Erase, 2-Program, 3- Verify

    for (n = FlashDevice.DevAdr; n < (FlashDevice.DevAdr + FlashDevice.szDev); n += FlashDevice.szPage)
    {
        ret |= ProgramPage(n, FlashDevice.szPage, g_buf);   // Test Page Programming
    }

                                                    // Verify Programm Page
    for (n = 0; n < FlashDevice.szDev; n++)
    {
        if (M8(FlashDevice.DevAdr + n) != ((unsigned char)n))
        {
            ret = 1;                                // Error
            break;
        }
    }

    ret |= UnInit (2);                              // Uninitialize Flash Programming Functions


    /* Test EraseChip Function -------------------------------------------------*/
    ret |= Init(0x08000000, 8000000, 1);            // Initialize Flash Programming Functions

    ret |= EraseChip();                             // Test Erase Chip
                                                    // Verify Erase
    for (n = 0; n < FlashDevice.szDev; n++)
    {
        if (M8(FlashDevice.DevAdr + n) != FlashDevice.valEmpty)
        {
            ret = 1;                                // Error
            break;
        }
    }

    ret |= UnInit (1);                              // Uninitialize Flash Programming Functions

    if ( ret )
    {
        while (ret);                                // FAIL: Wait forever
    }
    else
    {
        while (1);                                  // PASS: Wait forever
    }

} // main()
