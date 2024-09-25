//*****************************************************************************
//
//! @file hello_world.c
//!
//! @brief A simple "Hello World" example.
//!
//! This example prints a "Hello World" message with some device info
//! over SWO at 1M baud. To see the output of this program, use J-Link
//! SWO Viewer (or similar viewer appl) and configure SWOClock for 1000.
//! The example sleeps after it is done printing.
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Insert compiler version at compile time.
//
// Note - COMPILER_VERSION is defined in am_hal_mcu.h.
//
//*****************************************************************************
#define ENABLE_DEBUGGER

//
// DCU controls needed for debugger
//
#if defined(AM_PART_APOLLO5A)
#define DCU_DEBUGGER (AM_HAL_DCU_SWD | AM_HAL_DCU_CPUDBG_INVASIVE | AM_HAL_DCU_CPUDBG_NON_INVASIVE)
#elif defined(AM_PART_APOLLO5B)
#define DCU_DEBUGGER (AM_HAL_DCU_SWD | AM_HAL_DCU_CPUDBG_INVASIVE | AM_HAL_DCU_CPUDBG_NON_INVASIVE | AM_HAL_DCU_CPUDBG_S_INVASIVE | AM_HAL_DCU_CPUDBG_S_NON_INVASIVE)
#else
#error "Undefined Part Number"
#endif

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    am_util_id_t sIdDevice;
    uint32_t ui32StrBuf;
    uint32_t ui32Ret, ui32TrimVer;
    am_hal_reset_status_t sResetStatus;
    uint8_t ui8OTPstatus;
    bool bOTPenabled;
    bool bOTPEnabled;
    uint32_t ui32Status;


    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Check and Power on OTP if it is not already on.
    //
    ui32Status = am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_OTP, &bOTPEnabled);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Error during read of OTP power status\n");
    }
    if (!bOTPEnabled)
    {
        ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_OTP);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Error in power up of OTP module\n");
        }
    }

    //
    // Enable the Crypto module 
    //
    ui32Status = am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Error in power up of Crypto module\n");
    }

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM output
    //
    if (am_bsp_debug_printf_enable())
    {
        // Cannot print - so no point proceeding
        while(1);
    }

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Hello World!\n\n");

    //
    // Print the device info.
    //
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
    am_util_stdio_printf("Device type: %s\n", sIdDevice.pui8DeviceName);
    am_util_stdio_printf("Device Info:\n"
                         "\tPart number: 0x%08X\n"
                         "\tChip ID0:    0x%08X\n"
                         "\tChip ID1:    0x%08X\n"
                         "\tRevision:    0x%08X (Rev%c%c)\n",
                         sIdDevice.sMcuCtrlDevice.ui32ChipPN,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID0,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID1,
                         sIdDevice.sMcuCtrlDevice.ui32ChipRev,
                         sIdDevice.ui8ChipRevMaj, sIdDevice.ui8ChipRevMin );

    //
    // If not a multiple of 1024 bytes, append a plus sign to the KB.
    //
    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32MRAMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tMRAM size:   %7d (%d KB%s)\n",
                         sIdDevice.sMcuCtrlDevice.ui32MRAMSize,
                         sIdDevice.sMcuCtrlDevice.ui32MRAMSize / 1024,
                         &ui32StrBuf);

    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32DTCMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tDTCM size:   %7d (%d KB%s)\n",
                         sIdDevice.sMcuCtrlDevice.ui32DTCMSize,
                         sIdDevice.sMcuCtrlDevice.ui32DTCMSize / 1024,
                         &ui32StrBuf);

    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32SSRAMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tSSRAM size:  %7d (%d KB%s)\n",
                         sIdDevice.sMcuCtrlDevice.ui32SSRAMSize,
                         sIdDevice.sMcuCtrlDevice.ui32SSRAMSize / 1024,
                         &ui32StrBuf);

    //
    // If INFO1 is OTP, OTP must first be enabled.
    //
    ui8OTPstatus = 0;
    if ( MCUCTRL->SHADOWVALID_b.INFO1SELOTP )
    {
        // bit0=INFO1 OTP, bit1=OTP enabled. Enable/disable if == 0x1.
        am_hal_pwrctrl_periph_enabled(AM_HAL_PWRCTRL_PERIPH_OTP, &bOTPenabled);
        ui8OTPstatus = bOTPenabled ? 0x3 : 0x1;
        if ( ui8OTPstatus == 0x1 )
        {
            am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_OTP);
        }
    }

    ui32Ret = am_hal_info1_read(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1, AM_REG_OTP_INFO1_TRIM_REV_O / 4, 1, &ui32TrimVer);
    if ( (ui32TrimVer != 0x00000000)    &&
#ifdef AM_PART_APOLLO5A
         (ui32TrimVer != 0xFFFFFFFF)    &&
#endif // AM_PART_APOLLO5A
         (ui32Ret == AM_HAL_STATUS_SUCCESS) )
    {
        am_util_stdio_printf("\tTrim Rev     %ld\n", ui32TrimVer);
    }
    else
    {
        am_util_stdio_printf("\tTrim Rev Unknown (0x%08X); am_hal_info1_read()=0x%X.\n", ui32TrimVer, ui32Ret);
    }

    //
    // Print the compiler version.
    //
    am_util_stdio_printf("App Compiler:    %s\n", COMPILER_VERSION);
    am_util_stdio_printf("HAL Compiler:    %s\n", g_ui8HALcompiler);
    am_util_stdio_printf("HAL SDK version: %d.%d.%d\n",
                         g_ui32HALversion.s.Major,
                         g_ui32HALversion.s.Minor,
                         g_ui32HALversion.s.Revision);
    am_util_stdio_printf("HAL compiled with %s-style registers\n",
                         g_ui32HALversion.s.bAMREGS ? "AM_REG" : "CMSIS");
    am_util_stdio_printf("\n");

    am_util_stdio_printf("SECURITY INFO\n");
    am_util_stdio_printf("=============\n");

    am_hal_reset_status_get(&sResetStatus);
    am_util_stdio_printf("Reset Status:    0x%X\n", sResetStatus.eStatus);

    bool bInfo0Valid = am_hal_info0_valid();
    am_hal_security_socid_t socId;
    uint32_t ui32Var;
    uint32_t ui32dcuVal;
    uint32_t lcs = CRYPTO->LCSREG_b.LCSREG;
    am_util_stdio_printf("Device LCS: %s\n",
                         ((lcs == 0) ? "CM" :       \
                         ((lcs == 1) ? "DM" :       \
                         ((lcs == 5) ? "Secure" :   \
                         ((lcs == 7) ? "RMA" : "Undefined")))));
    am_hal_dcu_get(&ui32dcuVal);
    am_hal_security_get_socid(&socId);
    am_util_stdio_printf("\tSOC Id:\n\t0x%08X : 0x%08X : 0x%08X : 0x%08X\n\t0x%08X : 0x%08X : 0x%08X : 0x%08X\n",
                             socId.socid[0], socId.socid[1], socId.socid[2], socId.socid[3],
                             socId.socid[4], socId.socid[5], socId.socid[6], socId.socid[7] );
    if ( bInfo0Valid )
    {
        am_hal_info0_read(AM_HAL_INFO_INFOSPACE_CURRENT_INFO0, AM_REG_OTP_INFO0_SBR_SDCERT_ADDR_O / 4, 1, &ui32Var);
    }
    else
    {
        ui32Ret = am_hal_info1_read(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1, AM_REG_OTP_INFO1_SBR_SDCERT_ADDR_O / 4, 1, &ui32Var);
        if ( (ui32Var != 0x00000000)        &&
#ifdef AM_PART_APOLLO5A
             (ui32TrimVer != 0xFFFFFFFF)    &&
#endif // AM_PART_APOLLO5A
             (ui32Ret == AM_HAL_STATUS_SUCCESS) )
        {
            am_util_stdio_printf("Secure Debug Certificate Location: 0x%08X\n", ui32Var);
        }
        else
        {
            am_util_stdio_printf("Error - could not retrieve Secure Debug Certificate Location from INFO1.\n");
        }
    }

    ui32Ret = am_hal_info1_read(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1, AM_REG_OTP_INFO1_SBR_OPT_ADDR_O / 4, 1, &ui32Var);
    if ( (ui32Var != 0x00000000)        &&
#ifdef AM_PART_APOLLO5A
         (ui32TrimVer != 0xFFFFFFFF)    &&
#endif // AM_PART_APOLLO5A
         (ui32Ret == AM_HAL_STATUS_SUCCESS) )
    {
        am_util_stdio_printf("SBR OPT Address: 0x%08X\n", ui32Var);
    }
    else
    {
        am_util_stdio_printf("Error - could not retrieve OTP Address from INFO1.\n");
    }
    am_util_stdio_printf("\n");

    //
    // Restore OTP to its original state.
    //
    if ( ui8OTPstatus == 0x1 )
    {
        am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
    }

#ifdef ENABLE_DEBUGGER
    {
        // Enable Debugger
        if (am_hal_dcu_update(true, DCU_DEBUGGER) != AM_HAL_STATUS_SUCCESS)
        {
            // Cannot enable Debugger
            am_util_stdio_printf("Could not enable debugger using DCU\n");
        }
        else
        {
            am_util_stdio_printf("Forcibly enabling debugger using DCU\n");
        }
    }
    MCUCTRL->DEBUGGER = 0;
#endif // ENABLE_DEBUGGER
    if ((PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO == 1) && (CRYPTO->HOSTCCISIDLE_b.HOSTCCISIDLE == 1))
    {
        am_util_stdio_printf("DEBUG INFO\n");
        am_util_stdio_printf("==========\n");
        am_util_stdio_printf("Original Qualified DCU Val   0x%x\n", ui32dcuVal);
        am_hal_dcu_get(&ui32dcuVal);
        am_util_stdio_printf("Current  Qualified DCU Val   0x%x\n", ui32dcuVal);
        am_hal_dcu_lock_status_get(&ui32dcuVal);
        am_util_stdio_printf("Qualified DCU Lock Val       0x%x\n", ui32dcuVal);
        DIAG_SUPPRESS_VOLATILE_ORDER()
        am_util_stdio_printf("\tRaw DCU Enable: 0x%08X : 0x%08X : 0x%08X : 0x%08X\n",
                             CRYPTO->HOSTDCUEN0, CRYPTO->HOSTDCUEN1, CRYPTO->HOSTDCUEN2, CRYPTO->HOSTDCUEN3);
        am_util_stdio_printf("\tRaw DCU Lock  : 0x%08X : 0x%08X : 0x%08X : 0x%08X\n",
                             CRYPTO->HOSTDCULOCK0, CRYPTO->HOSTDCULOCK1, CRYPTO->HOSTDCULOCK2, CRYPTO->HOSTDCULOCK3);
        DIAG_DEFAULT_VOLATILE_ORDER()
        am_util_stdio_printf("MSPLIM:  0x%08X\n", __get_MSPLIM());
        am_util_stdio_printf("PSPLIM:  0x%08X\n", __get_PSPLIM());
        am_util_stdio_printf("\n");
    }

    //
    // We can power off crypto now
    //
    ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_CRYPTO);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Error in power down of Crypto module\n");
    }
    //
    // Power down OTP as well.
    //
    ui32Status = am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Error in power down of OTP module\n");
    }
    
    //
    // We are done printing.
    // Disable debug printf messages on ITM.
    //
    am_util_stdio_printf("Done with prints. Entering While loop\n");
    am_bsp_debug_printf_disable();

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
#if AM_PART_APOLLO5A
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);
#else
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif
    }
}
