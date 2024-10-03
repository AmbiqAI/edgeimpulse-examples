//*****************************************************************************
//
//! @file apollo4_secbl.c
//!
//! @brief A simple secondary bootloader program example template for Apollo4
//!
//! @addtogroup crypto_examples Crypto Examples
//
//! @defgroup apollo4_secbl Secondary Bootloader Example
//! @ingroup crypto_examples
//! @{
//!
//! Purpose: This example is a template for a secondary bootloader program for Apollo4.
//! It demonstrates how to access OTP key area.<br>
//! It demonstrates how to use the Ambiq SBL OTA framework for customer specific OTAs,
//!   e.g. to support external psram, or to support more advanced auth/enc schemes.<br>
//! It demonstrates how to validate & transfer control to the real main program image
//!   (assumed to be at address specified by MAIN_PROGRAM_ADDR_IN_FLASH in mram)
//!   after locking the OTP area before exiting<br><br>
//!
//! This example only demonstrates nonsecure boot and nonsecure OTA.<br><br>
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.<br>
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.<br>
//!   #define AM_DEBUG_PRINTF<br>
//!
//! Note that when this macro is defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.<br>
//!
//! To exercise this program:<br>
//!   Flash the main program at 0x100000 (MAIN_PROGRAM_ADDR_IN_FLASH)<br>
//!   Link this program at the address suitable for SBL(0x18000) configuration<br>
//!   To test OTA - construct images using magic numbers in the range matching AM_IMAGE_MAGIC_CUST<br>
//!   To test OTP key area access - need to keep OTP->Security->PLONEXIT as 1<br><br>
//!
//! Memory Addresses Used:<br>
//! ======<br>
//!
//! |   Address   | Description                                                                                                                |<br>
//! |:-----------:|:---------------------------------------------------------------------------------------------------------------------------|<br>
//! | 0x0001,8000 | Location of Sbl (unsecure secondary bootloader) at the standard apollo4 flash address.                                     |<br>
//! | 0x000D,2000 | Location where jlink script will load the unencrypted image blob that contains header info and the hello world application |<br>
//! | 0x0010,0000 | Location where sbl will move the executable portion of the hello_world from 0x000D,2000 + headerSize.                      |<br>
//! | 0x000D,0000 | Location where ota pointers are loaded (by jlink script), this is where the the sbl will find pointers to the blob         |<br>
//! | 0x4002,0264 | Address of OTA register, this is loaded by the jlink script and will point to 0x000D,0000. Used in SBL                     |<br><br>
//!
//! To Create the Example, please follow the guidelines below.<br>
//! - Generate a hello world using a modified IAR linker script.
//! - Make sure the flash region (MCU_MRAM) starts at 0x00100000 as follows:<br>
//!   - Define regions for the various types of internal memory.
//! @code
//!       define region MCU_MRAM    = mem:[from 0x00100000 to 0x00200000];
//!       define region MCU_TCM     = mem:[from 0x10000000 to 0x10060000];
//!       define region SHARED_SRAM = mem:[from 0x10060000 to 0x10160000];
//! @endcode
//!
//! - Name the hello_world binary something descriptive, say hello_world_100000.bin<br>
//!
//! - It is easiest to just copy this over to the <em>tools/apollo4b_scripts</em> directory and work from there<br><br>
//!
//! - Use one of the example blob creators found in <em>tools/apollo4b_scripts/examples</em>.<br>
//! - The output created by examples/firmware, examples/wired_download, or examples/wired_ota should all work<br>
//!
//! For example use <em>examples/firmware</em>:<br>
//!   1. Copy the hello_world_bin into the examples/wired_download directory<br>
//!   2. Edit/check these lines in firmware.ini:<br>
//!      - app_file = hello_world_1000000<br>
//!      - load_address = 0x100000<br>
//!   3. In a bash-shell type make, look at the Makefile for detail<br>
//!   4. Now there will be a file: hello_world_100000.ota.<br>
//!      - rename this to hello_world_100000.ota.bin, the jlink download command really wants a bin file<br>
//!   5. Copy this back two levels to the tools directory<br>
//!   6. At this point you should already have the apollo4_sec_bl.bin file loaded at 0x18000<br>
//!   7. Don't have any debugger running<br>
//!   8. Open up an swo window to see what is happening<br>
//!   9. Open a cmd window and using the jlink-blob.jlink file below type jlink --CommanderScript jlink-blob.jlink<br>
//!   10. This should restart the secondary bootloader, and you should see the sbl process the blob via debug output (SWO).<br>
//!
//! Since the image is resident in the MRAM, the secondary bootloader normally will jump to the hello world image.<br>
//! To do a full test of the download capability, the resident image at 0x00100000 should be erased before testing<br>
//!
//! @note All the scripts were run using python 3.8.10. They didn't work with python 3.9.<br>
//! @note Depending on your setup, you may have to type python3 instead of python<br>
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_bsp.h"
#include "am_util.h"
#include "apollo4_secBLProc.h"


static uint32_t secondarybl_exit(uint32_t *pImage) ;
static am_secvl_ret_codes_t secondarybl_JumpToImageAddr(
    uint32_t ui32ImageBaseAddr,
    am_hal_mcuctrl_device_t *psDevice) ;
static void bl_run_main(uint32_t *vtor) ;

//*****************************************************************************
//
//! @brief  Hardcoded function - to Run supplied main program
//!
//! @param  r0 = vtor - address of the vector table
//!
//! @return Returns None
//
//*****************************************************************************
#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION <  6000000)
static __asm void
bl_run_main(uint32_t *vtor)
{
    //
    // Store the vector table pointer of the new image into VTOR.
    //
    movw    r3, #0xED08
    movt    r3, #0xE000
    str     r0, [r3, #0]

    //
    // Load the new stack pointer into R3 and the new reset vector into R2.
    //
    ldr     r3, [r0, #0]
    ldr     r2, [r0, #4]

    //
    // Set the stack pointer for the new image.
    //
    mov     sp, r3

    //
    // Jump to the new reset vector.
    //
    bx      r2
}
#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION >= 6000000)
__attribute__((naked))
static void
bl_run_main(uint32_t *vtor)
{
    __asm
    (
        "   movw    r3, #0xED08\n\t"    // Load the address of the vector table pointer.
        "   movt    r3, #0xE000\n\t"
        "   str     r0, [r3, #0]\n\t"   // Save the vector table pointer for the image (vtor)
        "   ldr     r3, [r0, #0]\n\t"   // Load the stack pointer
        "   ldr     r2, [r0, #4]\n\t"   // load the starting program counter
        "   mov     sp, r3\n\t"         // Set the stack pointer for the new image.
        "   bx      r2\n\t"             // Jump to the new reset vector.
    );
}
#elif defined(__GNUC_STDC_INLINE__)
__attribute__((naked))
static void
bl_run_main(uint32_t *vtor)
{
    __asm
    (
        "   movw    r3, #0xED08\n\t"    // Load the address of the vector table pointer.
        "   movt    r3, #0xE000\n\t"
        "   str     r0, [r3, #0]\n\t"   // Save the vector table pointer for the image (vtor)
        "   ldr     r3, [r0, #0]\n\t"   // Load the stack pointer
        "   ldr     r2, [r0, #4]\n\t"   // load the starting program counter
        "   mov     sp, r3\n\t"         // Set the stack pointer for the new image.
        "   bx      r2\n\t"             // Jump to the new reset vector.
    );
}
#elif defined(__IAR_SYSTEMS_ICC__)
__stackless static inline void
bl_run_main(uint32_t *vtor)
{
    __asm volatile (
          "    movw    r3, #0xED08\n"    // Load the address of the vector table pointer.
          "    movt    r3, #0xE000\n"
          "    str     r0, [r3, #0]\n"   // Save the vector table pointer for the image (vtor)
          "    ldr     r3, [r0, #0]\n"   // Load the stack pointer
          "    ldr     r2, [r0, #4]\n"   // load the starting program counter
          "    mov     sp, r3\n"         // Set the stack pointer for the new image.
          "    bx      r2\n"             // Jump to the new reset vector.
          );
}
#else
#error Compiler is unknown, please contact Ambiq support team
#endif

//*****************************************************************************
//
//! @brief  Helper function to Perform exit operations for a secondary bootloader
//
//! @param  pImage - The address of the image to give control to
//!
//! @details This function does the necessary security operations while exiting from a
//! a secondary bootloader program. If still open, it locks the OTP key region,
//! as well as further updates to the flash protection register.
//! It also checks if it needs to halt to honor a debugger request.
//! If an image address is specified, control is transferred to the same on exit.
//!
//! @return Returns AM_HAL_STATUS_SUCCESS on success, if no image address specified
//! If an image address is provided, a successful execution results in transfer to
//! the image - and this function does not return.
//
//*****************************************************************************
static uint32_t
secondarybl_exit(uint32_t *pImage)
{
    uint32_t status = AM_HAL_STATUS_SUCCESS;

    //
    // Lock the assets
    //
    if ( MCUCTRL->BOOTLOADER_b.PROTLOCK )
    {
        am_hal_security_128bkey_t keyVal;
        for ( uint8_t i = 0; i < 4; i++ )
        {
            am_hal_otp_read_word(AM_REG_OTP_WRAP_KEY0_O + 4 * i, &keyVal.keyword[i]);
        }
        bool bLockStatus;

        //
        // PROTLOCK Open
        // This should also mean that Customer key is accessible
        // Now lock the key by writing an incorrect value
        //
        keyVal.keyword[0] = ~keyVal.keyword[0];
        am_hal_security_set_key(AM_HAL_SECURITY_LOCKTYPE_CUSTOTP_PROG, &keyVal);

        status = am_hal_security_get_lock_status(AM_HAL_SECURITY_LOCKTYPE_CUSTOTP_PROG, &bLockStatus);

        if ((status != AM_HAL_STATUS_SUCCESS) || (bLockStatus))
        {
            return AM_HAL_STATUS_FAIL;
        }

        //
        // Lock the protection register to prevent further region locking
        // CAUTION!!! - Can not do writing on BOOTLOADER register as all writable
        //              bits in this register are Write 1 to clear
        //
        MCUCTRL->BOOTLOADER = _VAL2FLD(MCUCTRL_BOOTLOADER_PROTLOCK, 1);

        //
        // Check if we need to halt (debugger request)
        //
        if (MCUCTRL->SCRATCH0 & 0x1)
        {
            //
            // Debugger wants to halt
            //
            uint32_t dhcsr = AM_REGVAL(0xE000EDF0);

            //
            // Clear the flag in Scratch register
            //
            MCUCTRL->SCRATCH0 &= ~0x1;

            //
            // Halt the core
            //
            dhcsr = ((uint32_t)0xA05F << 16) | (dhcsr & 0xFFFF) | 0x3;
            AM_REGVAL(0xE000EDF0) = dhcsr;

            //
            // Resume from halt
            //
        }
    }

    //
    // Give control to supplied image
    //
    if (pImage)
    {
        bl_run_main(pImage);
        //
        // Does not return
        //
    }
    return status;
} // secondarybl_exit()

//****************************************************************************
//
//! @brief this will test and eventually jump to the code image.
//!
//! @param ui32ImageBaseAddr   - image base address (also vector pointer)
//! @param psDevice            - info about this ambiq device
//!
//! @return am_secvl_ret_codes_t
//
//****************************************************************************
static am_secvl_ret_codes_t
secondarybl_JumpToImageAddr(uint32_t ui32ImageBaseAddr,
                            am_hal_mcuctrl_device_t *psDevice )
{

    if (ui32ImageBaseAddr >= psDevice->ui32MRAMSize)
    {
        return eSBL_INVALID_BOOT_ADDR;
    }

    //
    // check if the reset and stack vectors make sense
    // Validate the Stack Pointer
    // Validate the reset vector
    //
    uint32_t *pVtor = (uint32_t *) ui32ImageBaseAddr;
    uint32_t sp = pVtor[0];
    uint32_t reset = pVtor[1];

    if ((sp < SRAM_BASEADDR) ||
        (sp >= (SRAM_BASEADDR + psDevice->ui32SSRAMSize)) ||
        (reset < ui32ImageBaseAddr) ||
        (reset >= psDevice->ui32MRAMSize))
    {
        am_util_stdio_printf("Found invalid valid main image - Addr:0x%x,  SP:0x%x RV:0x%x\n",
                             ui32ImageBaseAddr,
                             sp,
                             reset);
        return eSBL_INVALID_IMAGE;
    }

    //
    // image and address appear ok, jump to image
    //
    am_util_stdio_printf("Found valid main image - SP:0x%x RV:0x%x\n", sp, reset);
    am_util_stdio_printf("Will transfer control over to this image after locking things down\n\n\n\n");

    //
    // wait a little while to let all the prints finish (if using uart ISR)
    //
    am_util_delay_ms(100);

    //
    // Lock the assets (if needed) and give control to main (jump to the image)
    //
    return (am_secvl_ret_codes_t) secondarybl_exit(pVtor);

} // fcn secondarybl_JumpToImageAddr

//****************************************************************************
// Main
//*****************************************************************************
int
main(void)
{
    am_hal_mcuctrl_device_t sDevice;

#ifdef WHILE1
    uint32_t ui32Security = 0;
    am_hal_otp_read_word(AM_REG_OTP_SECURITY_O, &ui32Security);
    if ((ui32Security & AM_REG_OTP_SECURITY_CUST_SECBOOT_Msk) >> AM_REG_OTP_SECURITY_CUST_SECBOOT_Pos != 2)
    {
        while(1);
    }
#endif // WHILE1


    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Enable debug printf messages using ITM on SWO pin
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("This is Apollo4 Secondary Bootloader Template Program!\r\n\r\n");

    //
    // Get chip specific info
    //
    am_hal_mcuctrl_info_get(AM_HAL_MCUCTRL_INFO_DEVICEID, &sDevice);

    //
    // print the keybanks
    //
    secondarybl_ProcOtp();

    //
    // For testing, this (MCUCTRL->OTAPOINTER) is set by the startup jlink script
    // it will point to a location of 32bit pointers also stored in MRAM by the script
    //
    am_secbl_OTA_ptr_t otaPtr;
    otaPtr.OTAPOINTER = MCUCTRL->OTAPOINTER; //OTAPOINTER_b.OTAVALID

    //
    // extract and process OTA images that may be stored in MRAM (or SRAM)
    //
    uint32_t ui32DestAddr = 0;
    am_secvl_ret_codes_t procOtpStatus = secondarybl_ProcOTAs(otaPtr, &ui32DestAddr, &sDevice);

    if (procOtpStatus == eSBL_OK)
    {

        //
        // Users can select any image format
        // Depending on the custom image format - more elaborate validation (including signature verification) can be done
        //
        //uint32_t imageAddr = MAIN_PROGRAM_ADDR_IN_FLASH;
        if (ui32DestAddr)
        {
            //
            // a valid address was recovered from one of the OTA images
            //
            secondarybl_JumpToImageAddr(ui32DestAddr, &sDevice);
        }
    }
    else
    {
        am_util_stdio_printf("Process ota failure %d\n", procOtpStatus);

    }

    secondarybl_JumpToImageAddr(MAIN_PROGRAM_ADDR_IN_FLASH, &sDevice);

    //
    // If we made it here, that means there is not bootable image found
    // Let the User know and go to deepsleep
    //
    am_util_stdio_printf("no bootable image found at %d\n", MAIN_PROGRAM_ADDR_IN_FLASH);
    am_bsp_debug_printf_enable();

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

