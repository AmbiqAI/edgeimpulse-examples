//*****************************************************************************
//
//! @file FlashPrg.c
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

/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for Ambiq Micro Apollo Chip                           */
/*                                                                     */
/***********************************************************************/

#include "stdint.h"
#include "stdbool.h"

#include "FlashOS.h"

#include "am_hal_flash.h"

#define AM_REGVAL(x)               (*((volatile uint32_t *)(x)))
#define AM_REG_CLKGEN_CLKKEY_KEYVAL                  0x00000047
#define AM_REG_CLKGEN_CCTRL_ADDR                     0x40004014
#define AM_REG_CLKGEN_CLKKEY_ADDR                    0x40004018

#define AM_HAL_FLASH_PROGRAM_KEY            0x12344321

#define AM_SBL_MAINPTR                      0x42003210

//
// Look-up table (taken from am_hal_bootrom_helper.c)
//
#if (AM_DEVICE == AM_DEV_APOLLO4) || (AM_DEVICE == AM_DEV_APOLLO5)
g_am_hal_flash_t g_am_hal_flash =
{
         ((int  (*)(uint32_t, uint32_t))                                    0x0200ff00 + 1),    // nv_mass_erase
         ((int  (*)(uint32_t, uint32_t, uint32_t))                          0x0200ff04 + 1),    // nv_page_erase
         ((int  (*)(uint32_t, uint32_t *, uint32_t *, uint32_t))            0x0200ff08 + 1),    // nv_program_main
         ((int  (*)(uint32_t, uint32_t *, uint32_t, uint32_t))              0x0200ff0c + 1),    // nv_program_info_area
         ((void (*)(void))                                                  0xDEADBEEF),        // nv_program_main_from_sram
         ((void (*)(void))                                                  0xDEADBEEF),        // nv_program_info_area_from_sram
         ((void (*)(void))                                                  0xDEADBEEF),        // nv_erase_main_pages_from_sram
         ((void (*)(void))                                                  0xDEADBEEF),        // nv_mass_erase_from_sram
         ((int  (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))      0x0200ff20 + 1),    // nv_program_main2
         ((uint32_t (*)(uint32_t *))                                        0x0200ff28 + 1),    // br_util_read_word
         ((void (*)( uint32_t *, uint32_t))                                 0x0200ff2c + 1),    // br_util_write_word
         ((int  (*)( uint32_t))                                             0x0200ff34 + 1),    // nv_info_erase
         ((void (*)(void))                                                  0xDEADBEEF),        // nv_info_erase_from_sram
         ((int  (*)( uint32_t ))                                            0x0200ff4c + 1),    // nv_recovery
         ((void (*)(uint32_t ))                                             0x0200ff50 + 1),    // br_util_delay_cycles
         ((void (*)(void))                                                  0xDEADBEEF)         // nv_recovery_from_sram
};
#else

#endif // AM_DEVICE == AM_DEV_APOLLO4 || AM_DEVICE == AM_DEV_APOLLO5


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc)
{
#if 0
    //
    // Set the clock for maximum frequency.
    // Notes:
    //  - if the debugger already does this, this step can be skipped.
    //  - This procedure works for Apollo, Apollo2 and Apollo3.
    //

    //
    // Unlock the clock control register.
    //
    AM_REGVAL(AM_REG_CLKGEN_CLKKEY_ADDR) = AM_REG_CLKGEN_CLKKEY_KEYVAL;

    //
    // Set the system clock for maximum frequency (set divider to 0).
    //
    AM_REGVAL(AM_REG_CLKGEN_CCTRL_ADDR) = 0x0;

    //
    // Lock the clock control register.
    //
    AM_REGVAL(AM_REG_CLKGEN_CLKKEY_ADDR) = 0;
#endif

    return 0;
} // Init()


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc)
{
    return 0;
} // UnInit()


static bool
getNVparms(uint32_t *pui32MainPtr, uint32_t *pui32NVsizeuser, uint32_t *pui32NVsizetotal)
{
    uint32_t ui32MainPtr, ui32NVsizetotal, ui32NVsizeuser;

    if ( (pui32MainPtr == 0x0) || (pui32NVsizeuser == 0x0) || (pui32NVsizetotal == 0x0))
    {
        return false;
    }

#if 1
    ui32MainPtr     = AM_STARTADDR;
    ui32NVsizetotal = AM_NVSIZE;
    ui32NVsizeuser  = AM_NVSIZE;
#else
    ui32MainPtr = AM_REGVAL(AM_SBL_MAINPTR);
    if (ui32MainPtr == 0xFFFFFFFF)
    {
        // Assume a "raw" part with no SBL.
        ui32MainPtr = 0x00000000;
    }
    else
    {
        // Compute the actual size.
        ui32NVsizeuser = ui32NVsizetotal - ui32MainPtr;
    }
#endif

    *pui32MainPtr     = ui32MainPtr;
    *pui32NVsizeuser  = ui32NVsizeuser;
    *pui32NVsizetotal = ui32NVsizetotal;

    return true;

} // getNVparms()





/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip(void)
{
    int retval = 0;

#if (AM_DEVICE == AM_DEV_APOLLO4) || (AM_DEVICE == AM_DEV_APOLLO5)
    uint32_t ui32MainPtr, ui32NVsizeuser, ui32NVsizetotal;

    getNVparms(&ui32MainPtr, &ui32NVsizeuser, &ui32NVsizetotal);

    //
    // At this point ui32MainPtr = ptr to the main program vector table.
    // Convert the parameters to word offsets.
    //
    ui32MainPtr     >>= 2;
    ui32NVsizeuser  >>= 2;

    //
    // Mass erase all of MRAM.
    //
    retval = g_am_hal_flash.nv_program_main2(
                AM_HAL_FLASH_PROGRAM_KEY,
                NV_WIPE,
                0xFFFFFFFF,                 // Value to store (wipe)
                ui32MainPtr,                // Word offset
                ui32NVsizeuser);            // Number of words
//    if ( retval )
//    {
//        return 1;
//    }

#elif AM_DEVICE == AM_DEV_APOLLO3P    // Apollo3p
    //
    // Erase first instance page-by-page skipping the SBL space, which resides
    // in the first 6 pages.
    //
#if (AM_DEV_SBL == 0)   // Non-SBL (RAW) version
    //
    // Mass erase the instances 0-3.
    //
    for ( ux = 0; ux < 4; ux++ )
    {
        retval |= g_am_hal_flash.am_hal_flash_mass_erase(AM_HAL_FLASH_PROGRAM_KEY, ux);
    }
#else   // SBL-enabled
    for ( ux = 6; ux < (512 * 1024) / (8 * 1024); ux++ )
    {
        retval = g_am_hal_flash.am_hal_flash_page_erase(
                 AM_HAL_FLASH_PROGRAM_KEY,
                 0,           // Flash instance
                 ux);         // Page number
        if ( retval )
        {
            return 1;
        }
    }

    //
    // Mass erase the instances 1-3.
    //
    for ( ux = 1; ux < 4; ux++ )
    {
        retval = g_am_hal_flash.am_hal_flash_mass_erase(AM_HAL_FLASH_PROGRAM_KEY, ux);

        if ( retval )
        {
            return 1;
        }
    }
#endif

#elif AM_DEVICE == AM_DEV_APOLLO3   // Apollo3
    //
    // Erase first instance page-by-page skipping the SBL space, which resides
    // in the first 6 pages.
    //
    for ( ux = 6; ux < 512 / 8; ux++ )
    {
        retval |= g_am_hal_flash.am_hal_flash_page_erase(
                  AM_HAL_FLASH_PROGRAM_KEY,
                  0,           // Flash instance
                  ux);         // Page number
    }

    //
    // Mass erase the second instance.
    //
    ux = 1;
    retval |= g_am_hal_flash.am_hal_flash_mass_erase(AM_HAL_FLASH_PROGRAM_KEY, ux);
#else   // Apollo2 or Apollo
    //
    // Mass erase the two instances.
    //
    for ( ux = 0; ux < 2; ux++ )
    {
        retval |= g_am_hal_flash.am_hal_flash_mass_erase(AM_HAL_FLASH_PROGRAM_KEY, ux);
    }
#endif

    return retval ? 1 : 0;
} // EraseChip()


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr)
{
  //
  // call the flash helper function for erasing a page
  //
#if (AM_DEVICE == AM_DEV_APOLLO4) || (AM_DEVICE == AM_DEV_APOLLO5)
    unsigned uRetVal, ui32inst, ui32pg, ui32lastPg;
    volatile unsigned ui32firstPg;
    uint32_t ui32MainPtr, ui32NVsizeuser, ui32NVsizetotal;

    getNVparms(&ui32MainPtr, &ui32NVsizeuser, &ui32NVsizetotal);

    //
    // At this point ui32MainPtr = ptr to the main program vector table.
    //
    // The Apollo4 page erase call ignores the instance completely and requires
    //  only an absolute page number.  We'll compute that page number.
    //
//  ui32lastPg   = ((AM_NVSIZE + AM_STARTADDR) / (8 * 1024)) - 1;
    ui32lastPg   = ((ui32MainPtr + ui32NVsizetotal) / (8 * 1024)) - 1;
    ui32firstPg = ui32MainPtr / (8 * 1024);
    ui32pg = adr / (8 * 1024);

#if 0
    if ( ui32pg < ui32firstPg )
    {
        // We told the caller that addressing began at 0, so this should have
        // been a valid page. Return success.
        return 0;
    }
#endif

    if ( ui32pg > ui32lastPg )
    {
        return 1;
    }

    ui32inst = 0;                               // Instance parameter is ignored
    uRetVal = g_am_hal_flash.nv_page_erase(
                AM_HAL_FLASH_PROGRAM_KEY,
                ui32inst,                       // Ignored for Apollo4
                ui32pg );                       // Page number

    return uRetVal ? 1 : 0;

#elif AM_DEV_APOLLO3P  ||  AM_DEVICE == AM_DEV_APOLLO3  ||  AM_DEVICE == AM_DEV_APOLLO2  // Corvette or Shelby (Apollo3p, Apollo3, Apollo2)
    unsigned uRetVal, ui32inst, ui32pg, ui32maxInst, ui32maxPg;

    ui32maxPg   = ((512 * 1024) / (8 * 1024)) - 1;  // 63 is the max page per instance in all 3 devices (Apollo2/3/3c)
#if AM_DEVICE == AM_DEV_APOLLO3P
    ui32maxInst = 3;
#else
    ui32maxInst = 1;
#endif

    ui32inst = adr / (512 * 1024);
    ui32pg   = (adr >> 13) & ((512 / 8) - 1);

    if ( (ui32inst > ui32maxInst) || (ui32pg > ui32maxPg) )
    {
        return 1;
    }

    uRetVal = g_am_hal_flash.am_hal_flash_page_erase(
                AM_HAL_FLASH_PROGRAM_KEY,
                ui32inst,                       // Flash instance
                ui32pg );                       // Page number

    return uRetVal ? 1 : 0;

#elif AM_DEVICE == AM_DEV_APOLLO    // Apollo
    //
    // This is the original routine used for Apollo.
    //

    return g_am_hal_flash.am_hal_flash_page_erase(
                AM_HAL_FLASH_PROGRAM_KEY,
                (adr & ~(0x00040000 - 1)) ? 1 : 0,  // 0 or 1
                (adr >> 11) & (256 / 2-1) );        // page number with bank
//              ((adr >> 11) & 0x0000007F) );       // page number with bank
#else
#error AM_DEVICE invalid!
#endif
} // EraseSector()


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

#if (AM_DEVICE != AM_DEV_APOLLO4) && (AM_DEVICE != AM_DEV_APOLLO5)
//
// Global dword-aligned data array.
//
//
// To minimize binary size, we're going to limit the static buffer to 2KB.
// For Shelby with 8KB pages, this means we'll have to break the programming
//  into multiple blocks. However if the incoming buffer is already
//  dword-aligned (the common case), we won't need to break it up.
//
uint32_t g_aligned_buffer[512];     // 2KB, global so we don't trash the stack with a 8KB buffer
#endif


int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf)
{
#if (AM_DEVICE == AM_DEV_APOLLO4) || (AM_DEVICE == AM_DEV_APOLLO5)
    int iRetVal = 0;
    uint32_t ui32MainPtr, ui32NVsizeuser, ui32NVsizetotal;

    getNVparms(&ui32MainPtr, &ui32NVsizeuser, &ui32NVsizetotal);

// if ( (adr & 0x3) || (sz & 0x3) || (adr < ui32MainPtr) )
   if ( (adr & 0x3) || (sz & 0x3) )
    {
        return 1;
    }

    iRetVal = g_am_hal_flash.nv_program_main2(
                    AM_HAL_FLASH_PROGRAM_KEY,
                    NV_PROGRAM,
                    (uint32_t)buf,              // Data to program
                    (adr >> 2),                 // Word address
                    sz >> 2);                   // Number of words
    return iRetVal ? 1 : 0;
#elif AM_DEV_APOLLO3P  ||  AM_DEVICE == AM_DEV_APOLLO3   // Apollo3p, Apollo3 (Stingray, Corvette)
    //
    // For Corvette, we're going to presume that we can program up to an
    // entire page in 1 burst.  But if we need to align stuff, we'll continue
    // to use the same 2KB buffer that was originally provided for Apollo.
    // For what it's worth, this is almost certainly the case for Apollo2
    // as well, although the below algorithm breaks it up.
    //
    uint32_t ui32Wds;
    int iRetVal = 0;

    ui32Wds = (sz / 4) + ( (sz & 0x3) ? 1 : 0 );
    if ( (adr & 0x3) || ((uint32_t)buf & 0x3) )
    {
        unsigned char *ptr;
        uint32_t ux, nbytes;

        while ( ui32Wds )
        {
            nbytes = (ui32Wds < (2048 / 4)) ? (ui32Wds * 4) : 2048;

            //
            // Copy all the bytes into a uint32_t aligned array
            //
            ptr = (unsigned char *)g_aligned_buffer;
            for (ux = 0; ux < nbytes; ux++)
            {
                *ptr++ = *buf++;
            }

            //
            // call the flash helper function for programming a page
            //
            iRetVal = g_am_hal_flash.am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY,
                                                               g_aligned_buffer,
                                                               (uint32_t *)(adr & 0xfffffffc),
                                                               nbytes / 4);
            if ( iRetVal )
            {
                return 1;
            }
            ui32Wds -= nbytes / 4;
            adr += nbytes;
        }
        return iRetVal ? 1 : 0;
    }
    else
    {
        iRetVal = g_am_hal_flash.am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY,
                                                           (uint32_t *)buf,
                                                           (uint32_t *)adr,
                                                           ui32Wds);
        return iRetVal ? 1 : 0;
    }
#elif AM_DEVICE == AM_DEV_APOLLO2                                   // Apollo2 (Shelby)
    //
    // Un-shadowed Shelby parts may have an issue where only somewhere just under
    //  128 words can be programmed in a burst (this is likely not an issue with
    //  shadow-enabled parts).  Therefore to be completely safe, we will limit
    //  programming cycles to 64 words at a time.
    //
#define XFER_BYTES      (64*4)      // 64 words max burst flash programming

    int iRetVal;
    uint8_t *pDst, *pSrc;
    int iXfer;

    if ( adr & 3 )
    {
        //
        // A highly unlikely case is that the flash location (adr) to be programmed
        //  is not on a dword boundary.  But there's no guarantee in the Keil
        //  spec that it will be on a dword boundary.  So to handle it, we'll
        //  first program a single dword of the beginning odd bytes.
        //
        // Begin by reading the current word at adr so we don't corrupt any
        //  existing data in the flash.
        //
        g_aligned_buffer[1] = *(uint32_t*)(adr & ~3);

        switch ( adr & 0x3 )
        {
            case 3:
                g_aligned_buffer[0]  = *buf++ << 0;
                g_aligned_buffer[0] |= g_aligned_buffer[1] & 0xFFFFFF00;
                sz  -= 1;
                break;
            case 2:
                g_aligned_buffer[0]  = *buf++ << 8;
                g_aligned_buffer[0] |= *buf++ << 0;
                g_aligned_buffer[0] |= g_aligned_buffer[1] & 0xFFFF0000;
                sz  -= 2;
                break;
            case 1:
                g_aligned_buffer[0]  = *buf++ << 16;
                g_aligned_buffer[0] |= *buf++ << 8;
                g_aligned_buffer[0] |= *buf++ << 0;
                g_aligned_buffer[0] |= g_aligned_buffer[1] & 0xFF000000;
                sz  -= 3;
                break;
        }

        //
        // Align the target addr and program the word containing the odd bytes.
        //
        adr &= ~0x3;

        iRetVal = g_am_hal_flash.am_hal_flash_program_main(
                    AM_HAL_FLASH_PROGRAM_KEY,           // ui32Value
                    g_aligned_buffer,                   // pui32Src
                    (uint32_t *)adr,                    // pui32Dst
                    1 );                                // ui32NumWords

        if ( iRetVal )
        {
            return 1;
        }

        //
        // Now that we've written the odd bytes, increment the target address.
        //
        adr += 4;
    }

    //
    // At this point, our destination flash address (adr) is guaranteed to be
    //  dword aligned.  However, we can't guarantee that the source buffer (buf)
    //  is dword aligned.  Therefore, we'll copy the data to an aligned buffer.
    //
    pSrc = buf;

    while ( sz > 0 )
    {
        iXfer = sz >= XFER_BYTES ? XFER_BYTES : sz;
        sz -= iXfer;
        pDst = (uint8_t*)g_aligned_buffer;
        while ( iXfer-- )
        {
            *pDst++ = *pSrc++;
        }

        //
        // Fill the remainder of the buffer, if any, to 0xFF. This also
        // handles the (also unlikely) case of sz not being a multiple of 4.
        //
        while ( pDst < ((uint8_t*)g_aligned_buffer + XFER_BYTES) )
        {
            *pDst++ = 0xFF;
        }

        iRetVal = g_am_hal_flash.am_hal_flash_program_main(
                    AM_HAL_FLASH_PROGRAM_KEY,           // ui32Value
                    g_aligned_buffer,                   // pui32Src
                    (uint32_t *)adr,                    // pui32Dst
                    XFER_BYTES / 4);                    // ui32NumWords

        if ( iRetVal )
        {
            return 1;
        }

        adr += XFER_BYTES;
    }

    return 0;

#elif AM_DEVICE == AM_DEV_APOLLO                                    // Apollo
    //
    // This is the original routine used for Apollo. However, the Shelby version
    // presented above should work for Apollo as well and be more general.
    //

    int i;
    unsigned char *ptr = (unsigned char *)g_aligned_buffer;

    //
    // copy bytes into a uint32_t aligned array
    //
    for (i = 0; i < sz; i++)
    {
        *ptr++ = *buf++;
    }

    //
    // call the flash helper function for programming a page
    //
    return g_am_hal_flash.am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY,
                                                    g_aligned_buffer,
                                                    (uint32_t *)(adr & 0xfffffffc),
                                                    512);
#else
#error AM_DEVICE invalid.
#endif

} // ProgramPage()

