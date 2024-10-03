//*****************************************************************************
//
//! @file c000raw.c
//!
//! @brief Run a 0xC000 binary on a raw Apollo3 device.
//!
//! IMPORTANT!
//! Since we do not ship raw Apollo3 devices, this program is not intended to
//! ever be included in any build.
//!
//! This mini-bootloader allows a .bin file that was compiled for an SBL-enabled
//! part to be run on a raw (non-SBL) device.  This is often the case when a
//! .bin file is received from the field or from a customer.
//!
//!
//! Steps:
//! - Use the bin2array.py script to convert the .bin to an array in files
//!   named binary_array.c and binary_array.h.
//!   Note - The original example binary_array included here is hello_world.bin
//!   as compiled by the Keil compiler for the Apollo3 EVB.
//! - Copy binary_array.c and binary_array.h to this src/ directory.
//!   A couple of edits to the binary_array files are then required.
//!
//! - In binary_array.h, replace the extern declaration with the following:
//!   #define ABS_LOCATION    0x0000C000
//!   #if defined(__IAR_SYSTEMS_ICC__)
//!   #define DECLARE_LOCATION    @ABS_LOCATION
//!   #define EXTERN_LOCATION
//!   #elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION < 6000000)
//!   #define DECLARE_LOCATION
//!   #define EXTERN_LOCATION      __attribute__((at(ABS_LOCATION)))
//!   #else
//!   #error c000raw.c Compiler not yet supported.
//!   #endif
//!   extern const uint8_t g_pui8BinaryArray[BINARY_ARRAY_LENGTH] EXTERN_LOCATION;
//! - In binary_array.c, replace the array declaration with the following:
//!   const uint8_t g_pui8BinaryArray[BINARY_ARRAY_LENGTH] DECLARE_LOCATION =
//!
//! - Compile c000raw normally with ROBase=0x0 using either Keil or IAR.
//!   Note that it does not matter what compiler was used for the original .bin.
//! - Load and run c000raw normally on the raw part.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "binary_array.h"

//*****************************************************************************
//
// Main
//
//*****************************************************************************
#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION < 6000000)
extern const uint8_t g_pui8BinaryArray[BINARY_ARRAY_LENGTH] __attribute__((at(0xC000)));
#elif defined(__IAR_SYSTEMS_ICC__)
extern const uint8_t g_pui8BinaryArray[BINARY_ARRAY_LENGTH] @ 0x0000C000;
#else
#error c000raw.c Compiler not supported.
#endif

#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION < 6000000)
static __asm void
run_c000(uint32_t ui32VTOR)
{
    //
    // Set the new vector table pointer.
    //
    mov     r3, #0xED08     // VTOR
    movt    r3, #0xE000     // "
    str     r0, [r3, #0]

    //
    //  R3 = New stack pointer.
    //  R2 = New reset vector.
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
#elif defined(__IAR_SYSTEMS_ICC__)
__stackless static void
run_c000( uint32_t ui32VTOR )
{
    //
    // Set the new vector table pointer.
    //
    __asm("     mov     r3, #0xED08");      // VTOR
    __asm("     movt    r3, #0xE000");      //  "
    __asm("     str     r0, [r3, #0]");

    //
    //  R3 = New stack pointer.
    //  R2 = New reset vector.
    //
    __asm("     ldr     r3, [r0, #0]");
    __asm("     ldr     r2, [r0, #4]");

    //
    // Set the stack pointer for the new image.
    //
    __asm("     mov     sp, r3");

    //
    // Jump to the new reset vector.
    //
    __asm("     bx      r2");
}
#endif


int
main(void)
{
    //
    // Read a value from the array just to make sure the linker
    // doesn't throw out the array.
    //
    volatile uint8_t ui8Value;
    ui8Value = g_pui8BinaryArray[0];
    ui8Value = g_pui8BinaryArray[BINARY_ARRAY_LENGTH - 1];

    run_c000(0x0000C000);
}
