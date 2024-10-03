//*****************************************************************************
//
//! @file am_hal_mram.c
//!
//! @brief MRAM and ROM Helper Function Access
//!
//! @addtogroup mram4 MRAM Functionality
//! @ingroup apollo5a_hal
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

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_hal_bootrom_helper.h"

#define AM_HAL_MRAM_WIPE                   0
#define AM_HAL_MRAM_PROGRAM                1

static uint32_t
nv_size_bytes(void)
{
    switch (MCUCTRL->SKU_b.SKUMRAMSIZE)
    {
        case 0x3:    return (4096 * 1024);
        case 0x2:    return (3072 * 1024);
        case 0x1:    return (2048 * 1024);
        case 0x0:    return (1024 * 1024);
        default:     return (1024 * 1024);   // Out of range, return the minimum
    }
} // nv_size_bytes()


//*****************************************************************************
//
// This programs up to N words of the Main MRAM
//    (with workaround for ragged edge programming issue, only required for RevA)
//
//*****************************************************************************
uint32_t
am_hal_mram_main_words_program(uint32_t ui32ProgramKey, uint32_t *pui32Src,
                               uint32_t *pui32Dst, uint32_t ui32NumWords)
{
    // Initialize the status
    uint32_t    ui32Status = 0;

    uint32_t numStrtAdjCnt = 0;
    uint32_t numEndAdjCnt = 0;
    bool bPrgmStrtWrds = false;
    bool bPrgmEndWrds = false;

    // Destination addresses so we don't have to keep type casting
    uint32_t dstStrtAddr = (uint32_t) pui32Dst;
    uint32_t dstEndAddr = ((uint32_t) pui32Dst  + (4 * ui32NumWords));

    // The address of the last word in the source buffer
    uint32_t* srcEndAddr = (uint32_t*)((uint32_t)pui32Src + (4 * (ui32NumWords -1 )) ) ;

    // Buffers to handle the ragged edge
    uint32_t ui32StartBuffer[4];
    uint32_t ui32EndBuffer[4];

// #### INTERNAL BEGIN ####
    // Uses Apollo5 helper function that efficiently performs one of two options specified
    // by the second paraneter.  (This function uses option 1)
    //  0) Writes a 32-bit pattern repeatedly.
    //     parm0: ui32ProgramKey,  parm1: 0,  parm2: 32-bit fill value,
    //     parm3: pui32Dst,  parm4: ui32NumWords.
    //  1) Program data from a source buffer.
    //     parm0: ui32ProgramKey,  parm1: 1,  parm2: pui32Src,
    //     parm3: pui32Dst,  parm4: ui32NumWords.
// #### INTERNAL END ####
#ifndef AM_HAL_DISABLE_API_VALIDATION

    // Check for word alignment for the MRAM destination address
    if ( (uint32_t) pui32Dst & 0x3 )
    {
        return AM_HAL_MRAM_INVLD_ADDR_ALIGNMENT;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION
// #### INTERNAL BEGIN ####
    // Workaround for CAYNSWS-1947 for a ragged edge handling above 2MB
    // for RevB all this is done correctly by the Rom'ed helper function
    // only the helper function is needed and called to program the specified range
// #### INTERNAL END ####
    //
    // check start and end addresses - if invalid return same errors as helper function
    //
    if ( (uint32_t) pui32Dst < AM_HAL_MRAM_ADDR )
    {
        return AM_HAL_MRAM_INVLD_START_MRAM_ADDR;
    }

    //
    // Check if the address is within the MRAM SKU Size
    //
    if ( ((uint32_t) pui32Dst + (4 * ui32NumWords)) > (AM_HAL_MRAM_ADDR + nv_size_bytes()) )
    {
        return AM_HAL_MRAM_INVLD_END_MRAM_ADDR;
    }

    //
    // verify MRAM is powered on for each instance accessed - or return error.
    //
    if ( ((uint32_t) pui32Dst < (AM_HAL_MRAM_ADDR + AM_HAL_MRAM_INSTANCE_SIZE)) &&
          PWRCTRL->MEMPWRSTATUS_b.PWRSTNVM0 == 0 )
    {
        return AM_HAL_MRAM_PWR_OFF;
    }
    if ( (((uint32_t) pui32Dst + (4 * ui32NumWords)) > (AM_HAL_MRAM_ADDR + AM_HAL_MRAM_INSTANCE_SIZE)) &&
          PWRCTRL->MEMPWRSTATUS_b.PWRSTNVM1 == 0 )
    {
        return AM_HAL_MRAM_PWR_OFF;
    }


    //
    // Check if we want to write to the upper 2MB of the NV and
    // if we these is a ragged edge at the beginning of the programming range
    // (the destination address is not 128-bit aligned)
    //
    if ( (dstStrtAddr > (AM_HAL_MRAM_ADDR + AM_HAL_MRAM_INSTANCE_SIZE)) && (dstStrtAddr & 0xF) )
    {
        //
        // set flag for begining ragged edge
        //
        bPrgmStrtWrds = true;

        //
        // Preload the buffer starting with the existing data at the 128-bit aligned starting address
        //
        for ( uint32_t i = 0; i < 4; i++ )
        {
            ui32StartBuffer[i] = *((uint32_t*)(dstStrtAddr & ~0xF ) + i );
        }

        //
        // Read in the words we want to write at the ragged edge
        // Note: The breaks are intentionally left out for a fall through to get all the valid
        // words in the NV memory. Also we checked for word alignment so only 4 values are
        // considered in the switch case.
        //
        switch ( (dstStrtAddr >> 2) & 0x3 )
        {
            case 0x1:
                ui32StartBuffer[1] = *(pui32Src + numStrtAdjCnt++);
                //
                // write the words that are in the source buffer only
                //
                if ( ui32NumWords == numStrtAdjCnt )
                {
                    break;
                }
            case 0x2:
                ui32StartBuffer[2] = *(pui32Src + numStrtAdjCnt++);
                if ( ui32NumWords == numStrtAdjCnt )
                {
                    break;
                }
            case 0x3:
                ui32StartBuffer[3] = *(pui32Src + numStrtAdjCnt++);
            case 0:
                // No need to do anything since the Src address is already 128 bit aligned
                break;
        }

        //
        // adjust the starting address for the non-ragged (main) blob to be 128-bit alligned
        //
        pui32Dst = (uint32_t *)((dstStrtAddr & ~0xF) + 0x10);
    }

    //
    // Check for a ragged end (and above 2MB)
    //
    if ( (dstEndAddr > (AM_HAL_MRAM_ADDR + AM_HAL_MRAM_INSTANCE_SIZE)) && (dstEndAddr & 0xF) )
    {
        if ( numStrtAdjCnt != ui32NumWords )
        {

            //
            // set flag for ending ragged edge
            //
            bPrgmEndWrds = true;

            //
            // Preload the buffer with the existing data at the 128-bit aligned ending address
            //
            for ( uint32_t i = 0; i<4; i++ )
            {
                ui32EndBuffer[i] = *((uint32_t*)(dstEndAddr & ~0xF ) + i );
            }

            //
            // Read in the word we want to write at the ragged edge
            // Note: The breaks are intentionally left out for a fall through to get all the valid
            // words in the NV memory so only 4 values are considered in the switch case.
            //
            switch ( (dstEndAddr >> 2) & 0x3 )
            {
                case 0x3:
                    ui32EndBuffer[2] = *(srcEndAddr - numEndAdjCnt++ );
                case 0x2:
                    ui32EndBuffer[1] = *(srcEndAddr - numEndAdjCnt++ );
                case 0x1:
                    ui32EndBuffer[0] = *(srcEndAddr - numEndAdjCnt++ );
                case 0:
                    // No need to do anything since the Src address is already 128 bit aligned
                    break;
            }
            //
            // adjust the  address for the ragged-edge at the end be 128-bit alligned
            //
            dstEndAddr = (dstEndAddr & ~0xF);

        }
    }

    //
    // Enable the ROM for helper function.
    //
    am_hal_pwrctrl_rom_enable();

    AM_CRITICAL_BEGIN

    //
    // Now program the main block without ragged edge(s), only if the ragged edges don't fully cover it
    // or there are no ragged edges (the original destination is 128-bit aligned on both ends)
    //
    if ( (!bPrgmStrtWrds && !bPrgmEndWrds) || ( ui32NumWords > (numStrtAdjCnt + numEndAdjCnt )) )
    {
        ui32Status =  g_am_hal_bootrom_helper.nv_program_main2(ui32ProgramKey, AM_HAL_MRAM_PROGRAM,
                                                              (uint32_t)( pui32Src + numStrtAdjCnt ),
                                                              ((uint32_t)pui32Dst - AM_HAL_MRAM_ADDR) >> 2,
                                                              ui32NumWords -(numStrtAdjCnt + numEndAdjCnt) );
    }
    //
    // if the bPrgmStrtWrds is true then program the 4 words of the begining ragged edge
    //
    if ( bPrgmStrtWrds && ( ui32Status == AM_HAL_STATUS_SUCCESS ) )
    {
        ui32Status =  g_am_hal_bootrom_helper.nv_program_main2(ui32ProgramKey, AM_HAL_MRAM_PROGRAM,
                                                              (uint32_t) ui32StartBuffer,
                                                              ( ((uint32_t)pui32Dst - 0x10) - AM_HAL_MRAM_ADDR ) >> 2,
                                                              4);
    }

    //
    // if the bPrgmEndWrds is true then program the 4 words of the ending ragged edge
    // for ui32NumWords of 0-3 words, the starting ragged edge may have covered it, then no
    // need to waste time reprogramming the same words.
    //
    if ( bPrgmEndWrds && ( ui32Status == AM_HAL_STATUS_SUCCESS ) )
    {
        ui32Status =  g_am_hal_bootrom_helper.nv_program_main2(ui32ProgramKey, AM_HAL_MRAM_PROGRAM,
                                                              (uint32_t)ui32EndBuffer,
                                                              (dstEndAddr - AM_HAL_MRAM_ADDR ) >> 2,
                                                              4);
    }

    AM_CRITICAL_END

    // Disable the ROM.
    am_hal_pwrctrl_rom_disable();

    //
    // Return the status.
    //
    return ui32Status;
} // am_hal_mram_main_words_program()

//*****************************************************************************
//
// This programs up to N words of the Main MRAM
//
//*****************************************************************************
uint32_t
am_hal_mram_main_program(uint32_t ui32ProgramKey, uint32_t *pui32Src,
                         uint32_t *pui32Dst, uint32_t ui32NumWords)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check for 16 byte aligned pui32Dst & ui32NumWords
    //
    if ( ((uint32_t)pui32Dst & 0xf) || (ui32NumWords & 0x3) )
    {
        return AM_HAL_MRAM_INVLD_ADDR_ALIGNMENT;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    return am_hal_mram_main_words_program(ui32ProgramKey, pui32Src,
                                          pui32Dst, ui32NumWords);
} // am_hal_mram_main_program()

//*****************************************************************************
//
// This Fills up to N words of the Main MRAM
//
//*****************************************************************************
uint32_t
am_hal_mram_main_fill(uint32_t ui32ProgramKey, uint32_t ui32Value,
                      uint32_t *pui32Dst, uint32_t ui32NumWords)
{
    uint32_t    ui32Status;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    //
    // Check for 16 byte aligned pui32Dst & ui32NumWords
    //
    if ( ((uint32_t)pui32Dst & 0xf) || (ui32NumWords & 0x3) )
    {
        return AM_HAL_MRAM_INVLD_ADDR_ALIGNMENT;
    }

#endif // AM_HAL_DISABLE_API_VALIDATION
    // check start and end addresses - if invalid return same errors as helper function
    if ( (uint32_t) pui32Dst < AM_HAL_MRAM_ADDR )
    {
        return AM_HAL_MRAM_INVLD_START_MRAM_ADDR;
    }
    if ( ((uint32_t) pui32Dst + (4 * ui32NumWords)) > (AM_HAL_MRAM_ADDR + nv_size_bytes()) )
    {
        return AM_HAL_MRAM_INVLD_END_MRAM_ADDR;
    }

    // verify MRAM is powered on for each instance accessed - or return error.
    if ( ((uint32_t) pui32Dst < (AM_HAL_MRAM_ADDR + AM_HAL_MRAM_INSTANCE_SIZE)) &&
          PWRCTRL->MEMPWRSTATUS_b.PWRSTNVM0 == 0 )
    {
        return AM_HAL_MRAM_PWR_OFF;
    }
    if ( (((uint32_t) pui32Dst + (4 * ui32NumWords)) > (AM_HAL_MRAM_ADDR + AM_HAL_MRAM_INSTANCE_SIZE)) &&
          PWRCTRL->MEMPWRSTATUS_b.PWRSTNVM1 == 0 )
    {
        return AM_HAL_MRAM_PWR_OFF;
    }


    //
// #### INTERNAL BEGIN ####
    // Uses Apollo5 helper function that efficiently performs one of two options specified
    // by the second paraneter.  (This function uses option 0)
    //  0) Writes a 32-bit pattern repeatedly.
    //     parm0: ui32ProgramKey,  parm1: 0,  parm2: 32-bit fill value,
    //     parm3: pui32Dst,  parm4: ui32NumWords.
    //  1) Program data from a source buffer.
    //     parm0: ui32ProgramKey,  parm1: 1,  parm2: pui32Src,
    //     parm3: pui32Dst,  parm4: ui32NumWords.
// #### INTERNAL END ####
    //

    //
    // Enable the ROM for helper functions.
    //
    am_hal_pwrctrl_rom_enable();

    AM_CRITICAL_BEGIN

    ui32Status =  g_am_hal_bootrom_helper.nv_program_main2(ui32ProgramKey, AM_HAL_MRAM_WIPE,
                                                          (uint32_t)ui32Value,
                                                          ((uint32_t)pui32Dst - AM_HAL_MRAM_ADDR ) >> 2,
                                                          ui32NumWords);
    AM_CRITICAL_END

    //
    // Disable the ROM.
    //
    am_hal_pwrctrl_rom_disable();

    return ui32Status;
} // am_hal_mram_main_fill()

//*****************************************************************************
//
// Initialize MRAM for DeepSleep.
//
//*****************************************************************************
uint32_t
am_hal_mram_ds_init(void)
{

    return AM_HAL_STATUS_SUCCESS;

} // am_hal_mram_ds_init()


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
