//*****************************************************************************
//
//! @file am_hal_mram.c
//!
//! @brief BootROM Helper Function Table
//!
//! @addtogroup mram4_4b MRAM Functionality
//! @ingroup apollo4b_hal
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
#include "regs/am_mcu_apollo4b_info0.h"
#include "regs/am_mcu_apollo4b_info1.h"
// #### INTERNAL BEGIN ####
#include "regs/apollo4b_mram.h"
// #### INTERNAL END ####

// #### INTERNAL BEGIN ####
#if 0 // This is the code corresponding to SRAM function g_program_mram_tmc_tcycrd
// The opcodes from an IAR position independent build.
void program_mram_tmc_tcycrd(uint32_t N) {
  // Enable MRAM-TMC direct access registers
  // REG_MRAM_ACCESS_WR(0xc3);
  MRAM->ACCESS = MRAM_ACCESS_ACCESS_Key;
  //BFW_MRAM_TMCCTRL_TMCOVERRIDE(1);
  MRAM->TMCCTRL_b.TMCOVERRIDE = 1;
  // Open "security gate" in TMC via R_TEST (XADR = 0)
  // Write configuration
  // BFW_MRAM_TMCCMD_TMCIFREN1(0);
  // BFW_MRAM_TMCCMD_TMCSRAMDMA(0);
  // BFW_MRAM_TMCCMD_TMCINFO1(0);
  // BFW_MRAM_TMCCMD_TMCINFO0(0);
  // BFW_MRAM_TMCCMD_TMCXADR(0);  // R_TEST
  // BFW_MRAM_TMCCMD_TMCYADR(0);
  // Invoke config command
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_WRITECFG);
  MRAM->TMCCMD = (MRAM_TMCCMD_TMCCMD_WRITECFG << MRAM_TMCCMD_TMCCMD_Pos) ;
  // Security "test-key"
  // REG_MRAM_WDATA0_WR(1);
  // REG_MRAM_WDATA1_WR(0);
  // REG_MRAM_WDATA2_WR(0);
  // REG_MRAM_WDATA3_WR(0);
  MRAM->WDATA0 = 1;
  MRAM->WDATA1 = 0;
  MRAM->WDATA2 = 0;
  MRAM->WDATA3 = 0;

  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;

  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;
  // Write R_TIMER1 in TMC with Tcycrd of 2 clocks (N=1) (XADR = 9)
  // Start by reading R_TIMER1
  // BFW_MRAM_TMCCMD_TMCXADR(0x9);  // R_TIMER1
  MRAM->TMCCMD_b.TMCXADR = 9;
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_READCFG);
  MRAM->TMCCMD_b.TMCCMD = MRAM_TMCCMD_TMCCMD_READCFG;
  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;
#if 0 // Old buggy code
  // uint32_t r_timer1_val = REG_MRAM_TMCRD0_RD();
  uint32_t r_timer1_val = MRAM->TMCRD0;
#else
  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;
  uint32_t r_timer1_val = MRAM->TMCREGIFDOUT;
#endif
  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;
  // Modify copy of R_TIMER1
  r_timer1_val &= 0xFFFFFFF0;  // Clear least significant nybble (Tcycrd)
  r_timer1_val |= (N & 0x0000000F);  // Or-in least significant bit (N = 1 -> Tcycrd = 2)
  // Write R_TIMER1
  // REG_MRAM_WDATA0_WR(r_timer1_val);
  MRAM->WDATA0 = r_timer1_val;
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_WRITECFG);
  MRAM->TMCCMD_b.TMCCMD = MRAM_TMCCMD_TMCCMD_WRITECFG;
  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;

  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;

  // Close "security gate" in TMC via R_TEST (XADR = 0)
  // BFW_MRAM_TMCCMD_TMCXADR(0);  // R_TEST
  MRAM->TMCCMD_b.TMCXADR = 0;
  // Clear security "test-key"
  // REG_MRAM_WDATA0_WR(0);
  MRAM->WDATA0 = 0;
  // Invoke config command
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_WRITECFG);
  MRAM->TMCCMD_b.TMCCMD = MRAM_TMCCMD_TMCCMD_WRITECFG;
  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;

  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;

  // Disable MRAM-TMC direct access registers

  // BFW_MRAM_TMCCTRL_TMCOVERRIDE(0);
  MRAM->TMCCTRL_b.TMCOVERRIDE = 0;

  // REG_MRAM_ACCESS_WR(0);
  MRAM->ACCESS = 0;
}

// This function 'fixes' the corruption incorrect
// program_mram_tmc_tcycrd implementation in SBR/SBL
// could have caused
// This is the code corresponding to SRAM function g_recover_broken_mram_tmc_r_timer1
// The opcodes from an IAR position independent build.
void recover_broken_mram_tmc_r_timer1(void) {

  // Enable MRAM-TMC direct access registers

  // REG_MRAM_ACCESS_WR(0xc3);
  MRAM->ACCESS = MRAM_ACCESS_ACCESS_Key;
  //BFW_MRAM_TMCCTRL_TMCOVERRIDE(1);
  MRAM->TMCCTRL_b.TMCOVERRIDE = 1;
  // Open "security gate" in TMC via R_TEST (XADR = 0)
  // Write configuration
  // BFW_MRAM_TMCCMD_TMCIFREN1(0);
  // BFW_MRAM_TMCCMD_TMCSRAMDMA(0);
  // BFW_MRAM_TMCCMD_TMCINFO1(0);
  // BFW_MRAM_TMCCMD_TMCINFO0(0);
  // BFW_MRAM_TMCCMD_TMCXADR(0);  // R_TEST
  // BFW_MRAM_TMCCMD_TMCYADR(0);
  // Invoke config command
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_WRITECFG);
  MRAM->TMCCMD = (MRAM_TMCCMD_TMCCMD_WRITECFG << MRAM_TMCCMD_TMCCMD_Pos) ;
  // Security "test-key"
  // REG_MRAM_WDATA0_WR(1);
  // REG_MRAM_WDATA1_WR(0);
  // REG_MRAM_WDATA2_WR(0);
  // REG_MRAM_WDATA3_WR(0);
  MRAM->WDATA0 = 1;
  MRAM->WDATA1 = 0;
  MRAM->WDATA2 = 0;
  MRAM->WDATA3 = 0;

  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;

  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;
  // Write R_TIMER1 in TMC with Tcycrd of 2 clocks (N=1) (XADR = 9)
  // Start by reading R_TIMER1
  // BFW_MRAM_TMCCMD_TMCXADR(0x9);  // R_TIMER1
  MRAM->TMCCMD_b.TMCXADR = 9;
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_READCFG);
  MRAM->TMCCMD_b.TMCCMD = MRAM_TMCCMD_TMCCMD_READCFG;
  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;

  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;

  // uint32_t r_timer1_val = REG_MRAM_TMCREGIFDOUT_RD();
  uint32_t r_timer1_val = MRAM->TMCREGIFDOUT;

  // Modify copy of R_TIMER1

  r_timer1_val &= 0x0000000F;  // Keep least significant nybble (Tcycrd)

  r_timer1_val |= (0x001058A0 & 0xFFFFFFF0);  // Or-in most significant 28 bits

  // Write R_TIMER1

  // REG_MRAM_WDATA0_WR(r_timer1_val);
  MRAM->WDATA0 = r_timer1_val;
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_WRITECFG);
  MRAM->TMCCMD_b.TMCCMD = MRAM_TMCCMD_TMCCMD_WRITECFG;
  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;

  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;

  // Close "security gate" in TMC via R_TEST (XADR = 0)
  // BFW_MRAM_TMCCMD_TMCXADR(0);  // R_TEST
  MRAM->TMCCMD_b.TMCXADR = 0;
  // Clear security "test-key"
  // REG_MRAM_WDATA0_WR(0);
  MRAM->WDATA0 = 0;
  // Invoke config command
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_WRITECFG);
  MRAM->TMCCMD_b.TMCCMD = MRAM_TMCCMD_TMCCMD_WRITECFG;
  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;

  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;

  // Disable MRAM-TMC direct access registers

  // BFW_MRAM_TMCCTRL_TMCOVERRIDE(0);
  MRAM->TMCCTRL_b.TMCOVERRIDE = 0;

  // REG_MRAM_ACCESS_WR(0);
  MRAM->ACCESS = 0;

}

// Just a test function to confirm the recovery
// should return 0x001058A0 in the upper 28 bit positions
// This is the code corresponding to SRAM function g_read_mram_tmc_r_timer1
// The opcodes from an IAR position independent build.
uint32_t read_mram_tmc_r_timer1(void) {

  // Enable MRAM-TMC direct access registers
  // REG_MRAM_ACCESS_WR(0xc3);
  MRAM->ACCESS = MRAM_ACCESS_ACCESS_Key;
  //BFW_MRAM_TMCCTRL_TMCOVERRIDE(1);
  MRAM->TMCCTRL_b.TMCOVERRIDE = 1;
  // Open "security gate" in TMC via R_TEST (XADR = 0)
  // Write configuration
  // BFW_MRAM_TMCCMD_TMCIFREN1(0);
  // BFW_MRAM_TMCCMD_TMCSRAMDMA(0);
  // BFW_MRAM_TMCCMD_TMCINFO1(0);
  // BFW_MRAM_TMCCMD_TMCINFO0(0);
  // BFW_MRAM_TMCCMD_TMCXADR(0);  // R_TEST
  // BFW_MRAM_TMCCMD_TMCYADR(0);
  // Invoke config command
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_WRITECFG);
  MRAM->TMCCMD = (MRAM_TMCCMD_TMCCMD_WRITECFG << MRAM_TMCCMD_TMCCMD_Pos) ;
  // Security "test-key"
  // REG_MRAM_WDATA0_WR(1);
  // REG_MRAM_WDATA1_WR(0);
  // REG_MRAM_WDATA2_WR(0);
  // REG_MRAM_WDATA3_WR(0);
  MRAM->WDATA0 = 1;
  MRAM->WDATA1 = 0;
  MRAM->WDATA2 = 0;
  MRAM->WDATA3 = 0;

  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;

  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;
  // Read R_TIMER1
  // BFW_MRAM_TMCCMD_TMCXADR(0x9);  // R_TIMER1
  MRAM->TMCCMD_b.TMCXADR = 9;
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_READCFG);
  MRAM->TMCCMD_b.TMCCMD = MRAM_TMCCMD_TMCCMD_READCFG;
  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;

  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;
  uint32_t r_timer1_val = MRAM->TMCREGIFDOUT;

  // Close "security gate" in TMC via R_TEST (XADR = 0)
  // BFW_MRAM_TMCCMD_TMCXADR(0);  // R_TEST
  MRAM->TMCCMD_b.TMCXADR = 0;
  // Clear security "test-key"
  // REG_MRAM_WDATA0_WR(0);
  MRAM->WDATA0 = 0;
  // Invoke config command
  // BFW_MRAM_TMCCMD_TMCCMD(BFC_MRAM_TMCCMD_TMCCMD_WRITECFG);
  MRAM->TMCCMD_b.TMCCMD = MRAM_TMCCMD_TMCCMD_WRITECFG;
  // BFW_MRAM_CTRL_GENCMD(1);
  MRAM->CTRL_b.GENCMD = 1;

  // BFW_MRAM_CTRL_GENCMD(0);
  MRAM->CTRL_b.GENCMD = 0;

  // Disable MRAM-TMC direct access registers

  // BFW_MRAM_TMCCTRL_TMCOVERRIDE(0);
  MRAM->TMCCTRL_b.TMCOVERRIDE = 0;

  // REG_MRAM_ACCESS_WR(0);
  MRAM->ACCESS = 0;

  return r_timer1_val;

}

#endif
// #### INTERNAL END ####

static uint32_t g_program_mram_tmc_tcycrd[] =
{
    0x0104f244, 0x0101f2c4, 0x604a22c3, 0x6ccb2205,
    0x0310f043, 0x650a64cb, 0x60cb2301, 0x610a2200,
    0x618a614a, 0x000ff000, 0xf043680b, 0x600b0308,
    0xf022680a, 0x600a0208, 0x6d0b2209, 0x2395f362,
    0x2206650b, 0xf3626d0b, 0x650b0304, 0xf042680a,
    0x600a0208, 0xf023680b, 0x600b0308, 0x680b6eca,
    0x0308f023, 0x600b0912, 0x1002ea40, 0x200560c8,
    0xf3606d0a, 0x650a0204, 0xf0406808, 0x60080008,
    0xf022680a, 0x600a0208, 0x6d082200, 0x2095f36f,
    0x60ca6508, 0x6d0a2005, 0x0204f360, 0x6808650a,
    0x0008f040, 0x680a6008, 0x0208f022, 0x2200600a,
    0xf0206cc8, 0x64c80010, 0x4770604a,
};

static uint32_t g_recover_broken_mram_tmc_r_timer1[] =
{
    0x0104f244, 0x0101f2c4, 0x604820c3, 0x6cca2005,
    0x0210f042, 0x650864ca, 0x60ca2201, 0x61082000,
    0x61886148, 0xf042680a, 0x600a0208, 0xf0206808,
    0x60080008, 0x6d0a2009, 0x2295f360, 0x2006650a,
    0xf3606d0a, 0x650a0204, 0xf0406808, 0x60080008,
    0x00a0f645, 0xf022680a, 0x600a0208, 0x0010f2c0,
    0xf0026eca, 0x4302020f, 0x200560ca, 0xf3606d0a,
    0x650a0204, 0xf0406808, 0x60080008, 0xf022680a,
    0x600a0208, 0x6d082200, 0x2095f36f, 0x60ca6508,
    0x6d0a2005, 0x0204f360, 0x6808650a, 0x0008f040,
    0x680a6008, 0x0208f022, 0x2200600a, 0xf0206cc8,
    0x64c80010, 0x2000604a, 0x00004770,
};

// #### INTERNAL BEGIN ####
#if 0
uint32_t g_read_mram_tmc_r_timer1[] =
{
    0x0104f244, 0x0101f2c4, 0x604820c3, 0x6cca2005,
    0x0210f042, 0x650864ca, 0x60ca2201, 0x61082000,
    0x61886148, 0x680a2300, 0x0208f042, 0x6808600a,
    0x0008f020, 0x20096008, 0xf3606d0a, 0x650a2295,
    0x6d0a2006, 0x0204f360, 0x6808650a, 0x0008f040,
    0x680a6008, 0x0208f022, 0x6ec8600a, 0xf36f6d0a,
    0x650a2295, 0x220560cb, 0xf3626d0b, 0x650b0304,
    0xf042680a, 0x600a0208, 0xf023680b, 0x600b0308,
    0x6cca2300, 0x0210f022, 0x604b64ca, 0x00004770,
};
typedef uint32_t (*read_mram_tmc_r_timer1_t)(void);
read_mram_tmc_r_timer1_t read_mram_tmc_r_timer1 = (read_mram_tmc_r_timer1_t)((uint8_t *)g_read_mram_tmc_r_timer1 + 1);
#endif
// #### INTERNAL END ####

// SRAM functions
typedef void (*program_mram_tmc_tcycrd_t)(uint32_t N);
program_mram_tmc_tcycrd_t program_mram_tmc_tcycrd = (program_mram_tmc_tcycrd_t)((uint8_t *)g_program_mram_tmc_tcycrd + 1);
typedef void (*recover_broken_mram_tmc_r_timer1_t)(void);
recover_broken_mram_tmc_r_timer1_t recover_broken_mram_tmc_r_timer1 = (recover_broken_mram_tmc_r_timer1_t)((uint8_t *)g_recover_broken_mram_tmc_r_timer1 + 1);

//*****************************************************************************
//
// Macros
//
//*****************************************************************************
#define MRAM_OVERRIDE()     program_mram_tmc_tcycrd(1)
#define MRAM_REVERT()       program_mram_tmc_tcycrd(0)
#define MRAM_RECOVER()      recover_broken_mram_tmc_r_timer1()

//*****************************************************************************
//
//! @brief This programs up to N words of the Main MRAM
//!
//! @param ui32ProgramKey - The programming key, AM_HAL_MRAM_PROGRAM_KEY.
//! @param pui32Src - Pointer to word aligned array of data to program into
//! the MRAM.
//! @param pui32Dst - Pointer to the words (4 byte) aligned MRAM location where
//! programming of the MRAM is to begin.
//! @param ui32NumWords - The number of words to be programmed.
//!
//! This function will program multiple words (4 byte) tuples in main MRAM.
//!
//! @note This function is provided only for convenience. It is most efficient
//! to operate on MRAM in be 4 words (16 byte) aligned multiples. Doing word
//! access will be very inefficient and should be avoided.
//!
//! @return 0 for success, non-zero for failure.
//!     Failing return code indicates:
//!    -1   invalid alignment for pui32Dst or ui32NumWords not 16 bye multiple
//!     1   ui32ProgramKey is invalid.
//!     2   pui32Dst is invalid.
//!     3   Flash addressing range would be exceeded.  That is, (pui32Dst +
//!         (ui32NumWords * 4)) is greater than the last valid address.
//!     4   pui32Src is invalid.
//!     5   pui32Src is invalid.
//!     6   Flash controller hardware timeout.
//
//*****************************************************************************
int
am_hal_mram_main_words_program(uint32_t ui32ProgramKey, uint32_t *pui32Src,
                               uint32_t *pui32Dst, uint32_t ui32NumWords)
{
    //
    // Use the new helper function to efficiently program the data in MRAM.
// #### INTERNAL BEGIN ####
    // New Apollo4B helper function that efficiently performs one of two
    //  functions.  For this function we'll be using function 1.
    //  0) Writes a 32-bit pattern repeatedly.
    //     parm0: ui32ProgramKey, parm1: 0, parm2: 32-bit fill value,
    //     parm3: pui32Dst, parm4: ui32NumWords.
    //  1) Program data from a source.
    //     parm0: ui32ProgramKey, parm1: 1, parm2: pui32Src,
    //     parm3: pui32Dst, parm4: ui32NumWords.
// #### INTERNAL END ####
    //

    if ( (uint32_t)pui32Dst <= AM_HAL_MRAM_LARGEST_VALID_ADDR )
    {
        //
        // This helper function requires a word offset rather than an actual
        // address. Since MRAM addresses start at 0x0, we can convert the addr
        // into a word offset by simply dividing the destination address by 4.
        //
        pui32Dst = (uint32_t*)((uint32_t)pui32Dst / 4);
    }

    return g_am_hal_bootrom_helper.nv_program_main2(ui32ProgramKey, 1,
                                                    (uint32_t)pui32Src,
                                                    (uint32_t)pui32Dst,
                                                    ui32NumWords);
}

//*****************************************************************************
//
//! @brief This programs up to N words of the Main MRAM
//!
//! @param ui32ProgramKey - The programming key, AM_HAL_MRAM_PROGRAM_KEY.
//! @param pui32Src - Pointer to word aligned array of data to program into
//! the MRAM.
//! @param pui32Dst - Pointer to the 4 words (16 byte) aligned MRAM location where
//! programming of the MRAM is to begin.
//! @param ui32NumWords - The number of words to be programmed. This MUST be
//! a multiple of 4 (16 byte multiple)
//!
//! This function will program multiple 4 words (16 byte) tuples in main MRAM.
//!
//! @note THIS FUNCTION ONLY OPERATES ON 16 BYTE BLOCKS OF MAIN MRAM. The pDst
//! MUST be 4 words (16 byte) aligned, and ui32NumWords MUST be multiple of 4.
//!
//! @return 0 for success, non-zero for failure.
//!     Failing return code indicates:
//!    -1   invalid alignment for pui32Dst or ui32NumWords not 16 byte multiple
//!     1   ui32ProgramKey is invalid.
//!     2   pui32Dst is invalid.
//!     3   Flash addressing range would be exceeded.  That is, (pui32Dst +
//!         (ui32NumWords * 4)) is greater than the last valid address.
//!     4   pui32Src is invalid.
//!     5   pui32Src is invalid.
//!     6   Flash controller hardware timeout.
//
//*****************************************************************************
int
am_hal_mram_main_program(uint32_t ui32ProgramKey, uint32_t *pui32Src,
                         uint32_t *pui32Dst, uint32_t ui32NumWords)
{
    //
    // Check for pui32Dst & ui32NumWords
    //
    if (((uint32_t)pui32Dst & 0xf) || (ui32NumWords & 0x3))
    {
        return -1;
    }

    return am_hal_mram_main_words_program(ui32ProgramKey, pui32Src,
                                          pui32Dst, ui32NumWords);
}

//*****************************************************************************
//
//! @brief This Fills up to N words of the Main MRAM
//!
//! @param ui32ProgramKey - The programming key, AM_HAL_MRAM_PROGRAM_KEY.
//! @param ui32Value - 32-bit data value to fill into the MRAM
//! @param pui32Dst - Pointer to the 4 words (16 byte) aligned MRAM location where
//! programming of the MRAM is to begin.
//! @param ui32NumWords - The number of words to be programmed. This MUST be
//! a multiple of 4 (16 byte multiple)
//!
//! This function will fill multiple 16 byte tuples in main MRAM with specified pattern.
//!
//! @note THIS FUNCTION ONLY OPERATES ON 16 BYTE BLOCKS OF MAIN MRAM. The pDst
//! MUST be 4 words (16 byte) aligned, and ui32NumWords MUST be multiple of 4.
//!
//! @return 0 for success, non-zero for failure.
//!    -1   invalid alignment for pui32Dst or ui32NumWords not 16 byte multiple
//!     1   ui32InfoKey is invalid.
//!     2   ui32Offset is invalid.
//!     3   addressing range would be exceeded.  That is, (ui32Offset +
//!         (ui32NumWords * 4)) is greater than the last valid address.
//!     4   pui32Src is invalid.
//!     5   pui32Src is invalid.
//!     6   Flash controller hardware timeout.
//
//*****************************************************************************
int
am_hal_mram_main_fill(uint32_t ui32ProgramKey, uint32_t ui32Value,
                      uint32_t *pui32Dst, uint32_t ui32NumWords)
{
    //
    // Check for pui32Dst & ui32NumWords
    //
    if (((uint32_t)pui32Dst & 0xf) || (ui32NumWords & 0x3))
    {
        return -1;
    }

    //
    // Use the new helper function to efficiently fill MRAM with a value.
// #### INTERNAL BEGIN ####
    // New Apollo4B helper function that efficiently performs one of two
    //  functions. For this function we'll be using function 0.
    //  0) Writes a 32-bit pattern repeatedly.
    //     parm0: ui32ProgramKey, parm1: 0, parm2: 32-bit fill value,
    //     parm3: pui32Dst, parm4: ui32NumWords.
    //  1) Program data from a source.
    //     parm0: ui32ProgramKey, parm1: 1, parm2: pui32Src,
    //     parm3: pui32Dst, parm4: ui32NumWords.
// #### INTERNAL END ####
    //

    if ( (uint32_t)pui32Dst <= AM_HAL_MRAM_LARGEST_VALID_ADDR )
    {
        //
        // This helper function requires a word offset rather than an actual
        // address. Since MRAM addresses start at 0x0, we can convert the addr
        // into a word offset by simply dividing the destination address by 4.
        //
        pui32Dst = (uint32_t*)((uint32_t)pui32Dst / 4);
    }

    return g_am_hal_bootrom_helper.nv_program_main2(ui32ProgramKey, 0,
                                                    (uint32_t)ui32Value,
                                                    (uint32_t)pui32Dst,
                                                    ui32NumWords);
}

//*****************************************************************************
//
//! @brief This programs up to N words of the Main array on one MRAM.
//!
//! @param ui32InfoKey - The programming key, AM_HAL_MRAM_INFO_KEY.
//! @param pui32Src - Pointer to word aligned array of data to program into
//! INFO0
//! @param ui32Offset - Pointer to the word aligned INFO0 offset where
//! programming of the INFO0 is to begin.
//! @param ui32NumWords - The number of words to be programmed.
//!
//! This function will program multiple words in INFO0
//!
//! @return 0 for success, non-zero for failure.
//!     1   ui32InfoKey is invalid.
//!     2   ui32Offset is invalid.
//!     3   addressing range would be exceeded.  That is, (ui32Offset +
//!         (ui32NumWords * 4)) is greater than the last valid address.
//!     4   pui32Src is invalid.
//!     5   pui32Src is invalid.
//!     6   Flash controller hardware timeout.
//
//*****************************************************************************
int
am_hal_mram_info_program(uint32_t ui32InfoKey, uint32_t *pui32Src,
                         uint32_t ui32Offset, uint32_t ui32NumWords)
{
// #### INTERNAL BEGIN ####
#if 0
    //
    // For B0 we need to handle non-quad-word-aligned writes.
    // We'll use the temporary read function to handle.
    //
    uint32_t ui32QuadOffset, ui32FirstWds, ui32MiddleWds, ui32EndWds, ui32InfoSpace;
    uint32_t ui32Data[8];
    int retval;

    //
    // Initializations
    //
    retval = 0;
    ui32InfoSpace = 0;
    ui32QuadOffset = ui32Offset & 0x3;

    if ( ui32NumWords >= 4 )
    {
        ui32FirstWds   = 4 - ui32QuadOffset;
        ui32FirstWds   = ui32FirstWds == 4 ? 0 : ui32FirstWds;
        ui32EndWds     = (ui32NumWords - ui32FirstWds) & 0x3;
        ui32MiddleWds  = ui32NumWords - ui32FirstWds - ui32EndWds;
    }
    else
    {
        // This case could still cross quad boundaries.
        // We'll handle it directly.
        uint32_t *pui32Data;

        ui32FirstWds = ui32MiddleWds = ui32EndWds = 0;

        // Get 2 quads.
        am_hal_mram_info_read(ui32InfoSpace, ui32Offset & 0xFFFFFFFC, 8, &ui32Data[0]);

        // Get ptr to first data word to modify.
        switch ( ui32QuadOffset )
        {
            case 0:
                pui32Data = &ui32Data[0];
                break;
            case 1:
                pui32Data = &ui32Data[1];
                break;
            case 2:
                pui32Data = &ui32Data[2];
                break;
            case 3:
                pui32Data = &ui32Data[3];
                break;
        }

        // Program the words
        while ( ui32NumWords )
        {
            *pui32Data++ = *pui32Src++;
            ui32NumWords--;
        }

        //
        // Program the 2 quads and we're done.
        //
        retval |= g_am_hal_bootrom_helper.nv_program_info_area(
                    ui32InfoKey, &ui32Data[0], ui32Offset & 0xFFFFFFFC, 8);
        return retval;
    }

    if ( ui32FirstWds )
    {
        //
        // Use the new read function to read the misaligned data.
        // Then modify the data and program it.
        //
        am_hal_mram_info_read(ui32InfoSpace, ui32Offset & 0xFFFFFFFC, 4, &ui32Data[0]);

        switch ( ui32FirstWds )
        {
            case 1:
                ui32Data[3] = *pui32Src++;
                break;
            case 2:
                ui32Data[2] = *pui32Src++;
                ui32Data[3] = *pui32Src++;
                break;
            case 3:
                ui32Data[1] = *pui32Src++;
                ui32Data[2] = *pui32Src++;
                ui32Data[3] = *pui32Src++;
                break;
        }

        //
        // Let's program the first full quad-word and get aligned.
        //
        retval |= g_am_hal_bootrom_helper.nv_program_info_area(
                    ui32InfoKey, &ui32Data[0], ui32Offset & 0xFFFFFFFC, 4);
        if ( retval )
        {
            return retval;
        }

        ui32Offset   += ui32FirstWds;
        ui32NumWords -= ui32FirstWds;
    }

    if ( ui32MiddleWds )
    {
        //
        // We're aligned, so program the middle part of the data.
        //
        retval |= g_am_hal_bootrom_helper.nv_program_info_area(
                    ui32InfoKey, pui32Src, ui32Offset, ui32MiddleWds);
        if ( retval )
        {
            return retval;
        }

        ui32Offset   += ui32MiddleWds;
        ui32NumWords -= ui32MiddleWds;
        pui32Src     += ui32MiddleWds;
    }

    if ( ui32EndWds )
    {
        //
        // Finish up with the trailing end of the data.
        //
        am_hal_mram_info_read(ui32InfoSpace, ui32Offset & 0xFFFFFFFC, 4, &ui32Data[0]);

        switch ( ui32EndWds )
        {
            case 1:
                ui32Data[0] = *pui32Src++;
                break;
            case 2:
                ui32Data[0] = *pui32Src++;
                ui32Data[1] = *pui32Src++;
                break;
            case 3:
                ui32Data[0] = *pui32Src++;
                ui32Data[1] = *pui32Src++;
                ui32Data[2] = *pui32Src++;
                break;
        }
        //
        // Write the final quad-word.
        //
        retval |= g_am_hal_bootrom_helper.nv_program_info_area(
                    ui32InfoKey, &ui32Data[0], ui32Offset, 4);
    }

    return retval;

#else
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
    //
    // TODO - Check for pui32Dst & ui32NumWords
    //
// #### INTERNAL END ####
    int retval;

    MRAM_OVERRIDE();

    retval = g_am_hal_bootrom_helper.nv_program_info_area(ui32InfoKey,
                                pui32Src, ui32Offset, ui32NumWords);

    MRAM_REVERT();

    return retval;
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
}

//*****************************************************************************
//
//! @brief This function erases main MRAM + customer INFO space
//!
//! @param ui32BrickKey - The Brick key.
//!
//! This function erases main MRAM and customer INFOinstance
//! even if the customer INFO space is programmed to not be erasable. This
//! function completely erases the MRAM main and info instance and wipes the
//! SRAM. Upon completion of the erasure operations, it does a POI (power on
//! initialization) reset.
//!
//! @note This function enforces 128 bit customer key lock. The caller needs to assert
//! the Recovery Lock using am_hal_security_set_key() providing appropriate key.
//! Otherwise, the function will fail.  Therefore, always check for a return code.
//! If the function returns, a failure has occured.
//!
//! @return Does not return if successful.  Returns failure code otherwise.
//!     Failing return code indicates:
//!     0x00000001  ui32BrickKey is invalid.
//!     0x00000002  Recovery key lock not set.
//!     Other values indicate Internal error.
//
//*****************************************************************************
int
am_hal_mram_recovery(uint32_t ui32BrickKey)
{
    return g_am_hal_bootrom_helper.nv_recovery(ui32BrickKey);
}

// #### INTERNAL BEGIN ####
#if 0
// #### I N T E R N A L BEGIN ####
// These opcodes from an IAR position independent build.
// #### I N T E R N A L END ####
static uint8_t
g_info_read[] =
{
    0x2D, 0xE9, 0xF8, 0x47, 0x14, 0x46, 0x00, 0x28,
    0x1A, 0x46, 0x3E, 0xD0, 0x01, 0x28, 0x4B, 0xD1,
    0x40, 0xF2, 0x01, 0x63, 0x99, 0x42, 0x28, 0xBF,
    0x02, 0x20, 0x80, 0xF0, 0xAD, 0x80, 0x65, 0x18,
    0x6D, 0x1E, 0x9D, 0x42, 0x28, 0xBF, 0x03, 0x20,
    0x80, 0xF0, 0xA6, 0x80, 0x4F, 0xF4, 0x00, 0x69,
    0x8B, 0x08, 0x03, 0xF0, 0x1F, 0x07, 0x09, 0xEB,
    0x53, 0x19, 0x44, 0xF2, 0x00, 0x06, 0xC4, 0xF2,
    0x01, 0x06, 0x46, 0xF6, 0xEB, 0x73, 0xC3, 0xF6,
    0x4C, 0x73, 0x4F, 0xF6, 0x0C, 0x15, 0x33, 0x64,
    0xCF, 0xF6, 0x26, 0x35, 0x42, 0xF6, 0xC0, 0x73,
    0x75, 0x64, 0xCA, 0xF6, 0x04, 0x13, 0x44, 0xF6,
    0x6F, 0x45, 0xB3, 0x64, 0xC9, 0xF6, 0x4E, 0x45,
    0xF5, 0x64, 0xC3, 0x23, 0xB3, 0x60, 0x00, 0x23,
    0x35, 0x6D, 0x45, 0xF0, 0x10, 0x05, 0x35, 0x65,
    0x4F, 0xF4, 0x80, 0x08, 0x01, 0xF0, 0x03, 0x05,
    0x64, 0x19, 0x4F, 0xF4, 0x00, 0x0E, 0x01, 0x46,
    0x6C, 0xE0, 0x40, 0xF2, 0x01, 0x23, 0x99, 0x42,
    0x28, 0xBF, 0x04, 0x20, 0x70, 0xD2, 0x65, 0x18,
    0x6D, 0x1E, 0x9D, 0x42, 0x28, 0xBF, 0x05, 0x20,
    0x6A, 0xD2, 0x4F, 0xF4, 0x60, 0x69, 0xC3, 0xE7,
    0x08, 0x20, 0xBD, 0xE8, 0xF2, 0x87, 0x70, 0x6D,
    0x20, 0xF0, 0x00, 0x70, 0x70, 0x65, 0x01, 0x29,
    0x70, 0x6D, 0x20, 0xF0, 0x80, 0x70, 0x70, 0x65,
    0x0C, 0xBF, 0x70, 0x46, 0x00, 0x20, 0xD6, 0xF8,
    0x54, 0xC0, 0x2C, 0xF4, 0x00, 0x0C, 0x40, 0xEA,
    0x0C, 0x00, 0x70, 0x65, 0x00, 0x29, 0xD6, 0xF8,
    0x54, 0xC0, 0x14, 0xBF, 0x00, 0x20, 0x40, 0x46,
    0x2C, 0xF4, 0x80, 0x0C, 0x40, 0xEA, 0x0C, 0x00,
    0x70, 0x65, 0x70, 0x6D, 0x69, 0xF3, 0x95, 0x20,
    0x70, 0x65, 0x70, 0x6D, 0x67, 0xF3, 0x49, 0x10,
    0x70, 0x65, 0x01, 0x20, 0xD6, 0xF8, 0x54, 0xC0,
    0x60, 0xF3, 0x04, 0x0C, 0xC6, 0xF8, 0x54, 0xC0,
    0x70, 0x68, 0x40, 0xF0, 0x08, 0x00, 0x70, 0x60,
    0x64, 0x20, 0xD6, 0xF8, 0x00, 0xA0, 0xCD, 0xF8,
    0x00, 0xA0, 0x40, 0x1E, 0xF9, 0xD1, 0xAB, 0x42,
    0x24, 0xBF, 0x30, 0x6E, 0x42, 0xF8, 0x04, 0x0B,
    0x5B, 0x1C, 0xAB, 0x42, 0x24, 0xBF, 0x70, 0x6E,
    0x42, 0xF8, 0x04, 0x0B, 0x5B, 0x1C, 0xAB, 0x42,
    0x24, 0xBF, 0xB0, 0x6E, 0x42, 0xF8, 0x04, 0x0B,
    0x5B, 0x1C, 0xAB, 0x42, 0x24, 0xBF, 0xF0, 0x6E,
    0x42, 0xF8, 0x04, 0x0B, 0x5B, 0x1C, 0x7F, 0x1C,
    0xB8, 0x06, 0x70, 0x68, 0x20, 0xF0, 0x08, 0x00,
    0x44, 0xBF, 0x07, 0xF0, 0x1F, 0x07, 0x09, 0xF1,
    0x01, 0x09, 0x70, 0x60, 0xA3, 0x42, 0xA2, 0xD3,
    0x30, 0x6D, 0x20, 0xF0, 0x10, 0x00, 0x30, 0x65,
    0x00, 0x21, 0xB1, 0x60, 0x00, 0x20, 0x30, 0x64,
    0xBD, 0xE8, 0xF2, 0x87
};

#define INFO0_SIZE_WORDS    ((2 * 1024) / 4)
#define INFO1_SIZE_WORDS    ((6 * 1024) / 4)

//*****************************************************************************
//
// Read INFO function.
//
// This function is not intended to be made public!
//
// g_info_read contains the opcodes for this function implementation as obtained
// from an IAR compilation with Code/Read Only and No Data Reads options.
//
//*****************************************************************************
//static int
int
info_read_copytosram(uint32_t ui32InfoSpace,   // Must be 0
                     uint32_t ui32Offset,
                     uint32_t ui32NumWords,
                     uint32_t *pui32Dst)
{
    volatile uint32_t ui32regval;
    uint32_t ui32Xadr, ui32Yadr;

    ui32Yadr = 0;

    switch ( ui32InfoSpace )
    {
// #### I N T E R N A L BEGIN ####
        case 1: // Ambiq info 1 - 6kB
            if ( ui32Offset > INFO1_SIZE_WORDS )
            {
                return 2;
            }

            if ( (ui32Offset + ui32NumWords - 1) > INFO1_SIZE_WORDS )
            {
                return 3;
            }

            ui32Xadr = 0x800;
            break;
// #### I N T E R N A L END ####
        case 0: // Customer info 0 - 2kB
            if ( ui32Offset > INFO0_SIZE_WORDS )
            {
                return 4;
            }

            if ( (ui32Offset + ui32NumWords - 1) > INFO0_SIZE_WORDS )
            {
                return 5;
            }

            ui32Xadr = 0xE00;
            break;

        default:
            // Bad info space selection
            return 8;  // TODO restore interrupts
    }

    // YADR is 5 bits, so it can select between 32 quadwords
    // Remainder can be placed in XADR

    uint32_t which_quadword = (ui32Offset / 4);
    ui32Yadr += which_quadword & 0x1F;
    ui32Xadr += which_quadword >> 5;

    // Write info access key (may only be necessary for Ambiq info 1?)
    MRAM->KEY0 = 0x3f4c6feb;
    MRAM->KEY1 = 0xfb26f90c;
    MRAM->KEY2 = 0xa9042fc0;
    MRAM->KEY3 = 0x9c4e4c6f;

    // Enable MRAM-TMC direct access registers
    MRAM->ACCESS = 0xc3;
    MRAM->TMCCTRL_b.TMCOVERRIDE = 1;

    for ( uint32_t count = 0; count < ((ui32Offset & 3) + ui32NumWords); )
    {
        // Write configuration
        MRAM->TMCCMD_b.TMCIFREN1 = 0;
        MRAM->TMCCMD_b.TMCSRAMDMA = 0;
        MRAM->TMCCMD_b.TMCINFO1 = ui32InfoSpace == 1 ? 1 : 0;
        MRAM->TMCCMD_b.TMCINFO0 = ui32InfoSpace == 0 ? 1 : 0;
        MRAM->TMCCMD_b.TMCXADR = ui32Xadr;
        MRAM->TMCCMD_b.TMCYADR = ui32Yadr;

        // Invoke read command
        MRAM->TMCCMD_b.TMCCMD = MRAM_TMCCMD_TMCCMD_READ;
        MRAM->CTRL_b.GENCMD = 1;

        // Waste time
        for ( uint32_t i = 0; i < 100; i++ )
        {
            ui32regval = MRAM->STATUS;
        }

        // Read data out of MRAM TMC data registers
        for ( uint32_t i = 0; i < 4; i++ )
        {
            if ( count >= (ui32Offset & 3) )
            {   // Deal with non-quadword aligned word offset
                *pui32Dst = *(uint32_t *)(&MRAM->TMCRD0 + i);
                pui32Dst++;
            }
            count++;
        }

        ui32Yadr++;
        if ( ui32Yadr & (1 << 5) )
        {
            ui32Yadr &= 0x1F;
            ui32Xadr++;
        }

        MRAM->CTRL_b.GENCMD = 0;
    }

    MRAM->TMCCTRL_b.TMCOVERRIDE = 0;
    MRAM->ACCESS = 0;
    MRAM->KEY0 = 0;

    return 0;
}
#endif
// #### INTERNAL END ####
//*****************************************************************************
//
//! @brief Read INFO data.
//!
//! This function implements a workaround required for Apollo4 B0 parts in
//! order to accurately read INFO space.
//!
//! @param ui32InfoSpace - 0 = Read INFO0.
//!                        1 = Only valid to read the customer visible area
//!                            of INFO1. If INFO1, the ui32WordOffset argument
//!                            must be 0x480 or greater (byte offset 0x1200).
//! @param ui32WordOffset - Desired word offset into INFO space.
//! @param ui32NumWords  - The number of words to be retrieved.
//! @param pui32Dst      - Pointer to the location where the INFO data
//!                        is to be copied to.
//!
//! @return 0 for success, non-zero for failure.                                                6
//
//*****************************************************************************
int
am_hal_mram_info_read(uint32_t ui32InfoSpace,
                      uint32_t ui32WordOffset,
                      uint32_t ui32NumWords,
                      uint32_t *pui32Dst)
{
    int retval = 0;
    uint32_t ux;
    uint32_t *pui32Info;

    if ( ui32InfoSpace == 0 )
    {
        if ( (ui32WordOffset >= (AM_HAL_INFO0_SIZE_BYTES / 4)) || ((ui32WordOffset + ui32NumWords) > (AM_HAL_INFO0_SIZE_BYTES / 4))  )
        {
            return 2;
        }
        pui32Info = (uint32_t*)(AM_REG_INFO0_BASEADDR + (ui32WordOffset * 4));
    }
    else if ( ui32InfoSpace == 1 )
    {
// #### INTERNAL BEGIN ####
        if ( (ui32WordOffset >= (AM_HAL_INFO1_SIZE_BYTES / 4)) || ((ui32WordOffset + ui32NumWords) > (AM_HAL_INFO1_SIZE_BYTES / 4))  )
#if 0
// #### INTERNAL END ####
        if ( (ui32WordOffset < (AM_HAL_INFO1_VISIBLE_OFFSET / 4)) || (ui32WordOffset >= (AM_HAL_INFO1_SIZE_BYTES / 4)) || ((ui32WordOffset + ui32NumWords) > (AM_HAL_INFO1_SIZE_BYTES / 4))  )
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
        {
            return 2;
        }
        pui32Info = (uint32_t*)(AM_REG_INFO1_BASEADDR + (ui32WordOffset * 4));
    }
    else
    {
        return 1;
    }

    MRAM_OVERRIDE();

    //
    // Start a critical section.
    //
    AM_CRITICAL_BEGIN

    for (ux = 0; ux < ui32NumWords; ux++ )
    {
        // INFO read should use the util function - to ensure the code is not running from MRAM
        *pui32Dst++ = am_hal_load_ui32(pui32Info++);
    }

    //
    // End the critical section.
    //
    AM_CRITICAL_END

    MRAM_REVERT();
// #### INTERNAL BEGIN ####
#if 0
    uint32_t SRAMCode = (uint32_t)g_info_read | 0x1;
    int (*pFunc)(uint32_t, uint32_t, uint32_t, uint32_t*) =
        (int (*)(uint32_t, uint32_t, uint32_t, uint32_t*))SRAMCode;
    retval = (*pFunc)(ui32InfoSpace, ui32WordOffset, ui32NumWords, pui32Dst);
#endif
// #### INTERNAL END ####

    return retval;
}

//*****************************************************************************
//
//! @brief Initialize MRAM for DeepSleep.
//!
//! This function implements a workaround required for Apollo4 B0 parts in
//! order to fix the MRAM DeepSleep config params.
//!
//! @return 0 for success, non-zero for failure.                                                6
//
//*****************************************************************************
int am_hal_mram_ds_init(void)
{
    MRAM_RECOVER();
    return 0;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
