//*****************************************************************************
//
//! @file am_hal_info.c
//!
//! @brief INFO helper functions
//!
//! @addtogroup info5 INFO Functionality
//! @ingroup apollo510L_hal
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

//*****************************************************************************
//
// Globals
//
//*****************************************************************************

//*****************************************************************************
//
// Helper functions to translate OTP offsets to MRAM offsets.
//
//*****************************************************************************
static uint32_t
INFO0_xlateOTPoffsetToMRAM(uint32_t ui32wordOffset)
{
    //
    // Translate the INFO0 OTP offset to an MRAM offset.
    // For Apollo5, INFO0 OTP and MRAM offsets are the same.
    //
    return ui32wordOffset;
} // INFO0_xlateOTPoffsetToMRAM()

static uint32_t
INFO1_xlateOTPoffsetToMRAM(uint32_t ui32wordOffset)
{
    uint32_t ui32MRAMwordOffset;

    //
    // Translate the INFO1 OTP offset to an MRAM offset.
    // Apollo5 INFO1 offsets:
    // From 0x000-0x7FC, MRAM offsets are identical to OTP offsets.
    // From 0x800,       Add 0xA00 to OTP offset to get MRAM offset.
    //
    //
    ui32MRAMwordOffset = ui32wordOffset < (0x800 / 4)   ?
                         ui32wordOffset                 :
                         ui32wordOffset + (0xA00 / 4);

    return ui32MRAMwordOffset;
} // INFO1_xlateOTPoffsetToMRAM()

//*****************************************************************************
//
// Read INFO data.
//
// eInfoSpace - Specifies which info space to be read.
//      AM_HAL_INFO_INFOSPACE_CURRENT_INFO0 // Currently active INFO0
//      AM_HAL_INFO_INFOSPACE_CURRENT_INFO1 // Currently active INFO1
//      AM_HAL_INFO_INFOSPACE_OTP_INFO0     // INFO0 from OTP  (regardless of current)
//      AM_HAL_INFO_INFOSPACE_OTP_INFO1     // INFO1 from OTP  (regardless of current)
//      AM_HAL_INFO_INFOSPACE_MRAM_INFO0    // INFO0 from MRAM (regardless of current)
//      AM_HAL_INFO_INFOSPACE_MRAM_INFO1    // INFO1 from MRAM (regardless of current)
//
//*****************************************************************************
static uint32_t
infoX_read(am_hal_info_infospace_e eInfoSpace,
           uint32_t ui32WordOffset,
           uint32_t ui32NumWords,
           uint32_t *pui32Dst)
{
    bool bINFO0SELOTP, bINFO1SELOTP, bOTPpowered;
    uint32_t ui32retval = AM_HAL_STATUS_SUCCESS;
    uint32_t ux;
    uint32_t *pui32Info;

    ux = MCUCTRL->SHADOWVALID;
    bINFO0SELOTP = ux & MCUCTRL_SHADOWVALID_INFO0SELOTP_Msk ? true : false;
    bINFO1SELOTP = ux & MCUCTRL_SHADOWVALID_INFO1SELOTP_Msk ? true : false;
    bOTPpowered  = PWRCTRL->DEVPWRSTATUS_b.PWRSTOTP ? true : false;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    uint32_t ui32InfoWds;

    if ( pui32Dst == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }

    switch ( eInfoSpace )
    {
        case AM_HAL_INFO_INFOSPACE_CURRENT_INFO0:
            ui32InfoWds = ( bINFO0SELOTP == true ) ? (AM_REG_OTP_INFO0_SIZE / 4) : (AM_REG_INFO0_SIZE / 4);
            break;
        case AM_HAL_INFO_INFOSPACE_OTP_INFO0:
            ui32InfoWds = (AM_REG_OTP_INFO0_SIZE / 4);
            break;
        case AM_HAL_INFO_INFOSPACE_MRAM_INFO0:
            ui32InfoWds = (AM_REG_INFO0_SIZE / 4);
            break;
        case AM_HAL_INFO_INFOSPACE_CURRENT_INFO1:
            ui32InfoWds = ( bINFO1SELOTP == true ) ? (AM_REG_OTP_INFO1_SIZE / 4) : (AM_REG_INFO1_SIZE / 4);
            break;
        case AM_HAL_INFO_INFOSPACE_OTP_INFO1:
            ui32InfoWds = (AM_REG_OTP_INFO1_SIZE / 4);
            break;
        case AM_HAL_INFO_INFOSPACE_MRAM_INFO1:
            ui32InfoWds = (AM_REG_INFO1_SIZE / 4);
            break;
        default:
            return AM_HAL_STATUS_INVALID_ARG;
    } // switch (eInfoSpace)

    if ( (ui32WordOffset + ui32NumWords) > ui32InfoWds )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    switch ( eInfoSpace )
    {
        case AM_HAL_INFO_INFOSPACE_CURRENT_INFO0:
            //
            // INFO0 MRAM offsets are the same as OTP offsets.
            //
            if ( bINFO0SELOTP == true )
            {
                if ( bOTPpowered == false )
                {
                    return AM_HAL_STATUS_HW_ERR;
                }
                pui32Info = (uint32_t*)(AM_REG_OTP_INFO0_BASEADDR + (ui32WordOffset * 4));
            }
            else
            {
                pui32Info = (uint32_t*)(AM_REG_INFO0_BASEADDR + (INFO0_xlateOTPoffsetToMRAM(ui32WordOffset) * 4));
            }
            break;

        case AM_HAL_INFO_INFOSPACE_CURRENT_INFO1:

            if ( bINFO1SELOTP == true )
            {
                if ( bOTPpowered == false )
                {
                    return AM_HAL_STATUS_HW_ERR;
                }
                pui32Info = (uint32_t*)(AM_REG_OTP_INFO1_BASEADDR + (ui32WordOffset * 4));
            }
            else
            {
                pui32Info = (uint32_t*)(AM_REG_INFO1_BASEADDR + (INFO1_xlateOTPoffsetToMRAM(ui32WordOffset) * 4));
            }
            break;

        case AM_HAL_INFO_INFOSPACE_OTP_INFO0:
            if ( bOTPpowered == false )
            {
                return AM_HAL_STATUS_HW_ERR;
            }
            pui32Info = (uint32_t*)(AM_REG_OTP_INFO0_BASEADDR + (ui32WordOffset * 4));
            break;

        case AM_HAL_INFO_INFOSPACE_OTP_INFO1:
            if ( bOTPpowered == false )
            {
                return AM_HAL_STATUS_HW_ERR;
            }
            pui32Info = (uint32_t*)(AM_REG_OTP_INFO1_BASEADDR + (ui32WordOffset * 4));
            break;

        case AM_HAL_INFO_INFOSPACE_MRAM_INFO0:
            pui32Info = (uint32_t*)(AM_REG_INFO0_BASEADDR + (ui32WordOffset * 4));
            break;

        case AM_HAL_INFO_INFOSPACE_MRAM_INFO1:
            pui32Info = (uint32_t*)(AM_REG_INFO1_BASEADDR + (ui32WordOffset * 4));
            break;

        default:
            ui32retval = AM_HAL_STATUS_INVALID_ARG;
            break;

    } // switch (eInfoSpace)

    if ( ui32retval == AM_HAL_STATUS_SUCCESS )
    {
        for (ux = 0; ux < ui32NumWords; ux++ )
        {
            *pui32Dst++ = *pui32Info++;
        }
    }

    return ui32retval;

} // infoX_read()

//*****************************************************************************
//
// Determine if INFO0 is valid.
//
//*****************************************************************************
bool
am_hal_info0_valid(void)
{
    return MCUCTRL->SHADOWVALID_b.INFO0VALID ? true : false;

} // am_hal_info0_valid()

//*****************************************************************************
//
// Read INFO0 data.
//
// eInfoSpace - Specifies which info space to be read.
//      AM_HAL_INFO_INFOSPACE_CURRENT_INFO0 // Currently active INFO0
//      AM_HAL_INFO_INFOSPACE_OTP_INFO0     // INFO0 from OTP  (regardless of current)
//      AM_HAL_INFO_INFOSPACE_MRAM_INFO0    // INFO0 from MRAM (regardless of current)
//
// Note: If OTP, it must be powered on entry to this function.
//       OTP can then be safely disabled after exit from this function.
//       am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_OTP);
//       am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
//
//*****************************************************************************
uint32_t
am_hal_info0_read(am_hal_info_infospace_e eInfoSpace,
                  uint32_t ui32wordOffset,
                  uint32_t ui32NumWords,
                  uint32_t *pui32Dst)
{
    uint32_t ui32retval;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( (eInfoSpace != AM_HAL_INFO_INFOSPACE_CURRENT_INFO0)    &&
         (eInfoSpace != AM_HAL_INFO_INFOSPACE_OTP_INFO0)        &&
         (eInfoSpace != AM_HAL_INFO_INFOSPACE_MRAM_INFO0) )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    ui32retval = infoX_read(eInfoSpace, ui32wordOffset, ui32NumWords, pui32Dst);

    return ui32retval;

} // am_hal_info0_read()

//*****************************************************************************
//
// Read INFO1 data.
//
// eInfoSpace - Specifies which info space to be read.
//      AM_HAL_INFO_INFOSPACE_CURRENT_INFO1 // Currently active INFO1
//      AM_HAL_INFO_INFOSPACE_OTP_INFO1     // INFO1 from OTP  (regardless of current)
//      AM_HAL_INFO_INFOSPACE_MRAM_INFO1    // INFO1 from MRAM (regardless of current)
//
// Note: If INFO1 is OTP, caller is responsible for making sure OTP is powered
//       up before calling this function. OTP can then be safely disabled after
//       exit from this function. See full documentation for more information.
//
//*****************************************************************************
uint32_t
am_hal_info1_read(am_hal_info_infospace_e eInfoSpace,
                  uint32_t ui32wordOffset,
                  uint32_t ui32NumWords,
                  uint32_t *pui32Dst)
{
#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( (eInfoSpace != AM_HAL_INFO_INFOSPACE_CURRENT_INFO1)    &&
         (eInfoSpace != AM_HAL_INFO_INFOSPACE_OTP_INFO1)        &&
         (eInfoSpace != AM_HAL_INFO_INFOSPACE_MRAM_INFO1) )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    return infoX_read(eInfoSpace, ui32wordOffset, ui32NumWords, pui32Dst);

} // am_hal_info1_read()

//*****************************************************************************
//
// This programs up to N words of the Main array on one MRAM.
//
// Note: If OTP, it must be powered on entry to this function.
//       OTP can then be safely disabled after exit from this function.
//       am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_OTP);
//       am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
//
//*****************************************************************************
uint32_t
am_hal_info0_program(am_hal_info_infospace_e eInfoSpace,
                     uint32_t ui32InfoKey,    uint32_t *pui32Src,
                     uint32_t ui32WordOffset, uint32_t ui32NumWords)
{
    int ui32retval;
    bool bINFO0SELOTP = MCUCTRL->SHADOWVALID_b.INFO0SELOTP ? true : false;
    bool bOTPpowered  = PWRCTRL->DEVPWRSTATUS_b.PWRSTOTP ? true : false;
    bool bDoOTP = false;

#ifndef AM_HAL_DISABLE_API_VALIDATION
    if ( pui32Src == NULL )
    {
        return AM_HAL_STATUS_INVALID_ARG;
    }
#endif // AM_HAL_DISABLE_API_VALIDATION

    switch ( eInfoSpace )
    {
        case AM_HAL_INFO_INFOSPACE_CURRENT_INFO0:
            if ( bINFO0SELOTP )
            {
                bDoOTP = true;
#ifndef AM_HAL_DISABLE_API_VALIDATION
                if ( (ui32WordOffset + ui32NumWords) > (AM_REG_OTP_INFO0_SIZE / 4) )
                {
                    return AM_HAL_STATUS_OUT_OF_RANGE;
                }
#endif // AM_HAL_DISABLE_API_VALIDATION
            }
            else
            {
#ifndef AM_HAL_DISABLE_API_VALIDATION
                if ( (ui32WordOffset + ui32NumWords) > (AM_REG_INFO0_SIZE / 4) )
                {
                    return AM_HAL_STATUS_OUT_OF_RANGE;
                }
#endif // AM_HAL_DISABLE_API_VALIDATION
            }
            break;

        case AM_HAL_INFO_INFOSPACE_OTP_INFO0:
                bDoOTP = true;
#ifndef AM_HAL_DISABLE_API_VALIDATION
                if ( (ui32WordOffset + ui32NumWords) > (AM_REG_OTP_INFO0_SIZE / 4) )
                {
                    return AM_HAL_STATUS_OUT_OF_RANGE;
                }
#endif // AM_HAL_DISABLE_API_VALIDATION
            break;

        case AM_HAL_INFO_INFOSPACE_MRAM_INFO0:
#ifndef AM_HAL_DISABLE_API_VALIDATION
                if ( (ui32WordOffset + ui32NumWords) > (AM_REG_INFO0_SIZE / 4) )
                {
                    return AM_HAL_STATUS_OUT_OF_RANGE;
                }
#endif // AM_HAL_DISABLE_API_VALIDATION
            break;

        default:
            return AM_HAL_STATUS_INVALID_ARG;
    } // switch (eInfoSpace)

    if ( bDoOTP && (bOTPpowered == false) )
    {
        ui32retval = AM_HAL_STATUS_HW_ERR;
        return ui32retval;
    }

    AM_CRITICAL_BEGIN

    //
    // Enable the ROM for helper functions.
    //
    am_hal_pwrctrl_rom_enable();

    if ( bDoOTP )
    {
        ui32retval = g_am_hal_bootrom_helper.otp_program_info_area(ui32InfoKey,
                                                                   pui32Src,
                                                                   ui32WordOffset,
                                                                   ui32NumWords);
    }
    else
    {
        ui32retval = g_am_hal_bootrom_helper.nv_program_info_area(ui32InfoKey,
                                                                  pui32Src,
                                                                  ui32WordOffset,
                                                                  ui32NumWords);
    }

    //
    // Disable the ROM.
    //
    am_hal_pwrctrl_rom_disable();

    AM_CRITICAL_END

    return ui32retval;

} // am_hal_info0_program()

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
