//*****************************************************************************
//
//! @file am_hal_mram.c
//!
//! @brief BootROM Helper Function Table
//!
//! @addtogroup mram4 MRAM Functionality
//! @ingroup bronco_hal
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
//#include "regs/am_mcu_apollo4b_info0.h"
//#include "regs/am_mcu_apollo4b_info1.h"
// #### INTERNAL BEGIN ####
#include "regs/apollo4b_mram.h"
// #### INTERNAL END ####

//#define INFO_ACCESS_WA

// #### INTERNAL BEGIN ####
#ifdef INFO_ACCESS_WA
#if 0 // This is the code corresponding to SRAM function g_program_mram_tmc_tcycrd
// The opcodes from an IAR position independent build.
void program_mram_tmc_tcycrd(uint32_t N)
{
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

//
// This function 'fixes' the corruption incorrect
// program_mram_tmc_tcycrd implementation in SBR/SBL
// could have caused
// This is the code corresponding to SRAM function g_recover_broken_mram_tmc_r_timer1
// The opcodes from an IAR position independent build.
//
void
recover_broken_mram_tmc_r_timer1(void)
{
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

//
// Just a test function to confirm the recovery
// should return 0x001058A0 in the upper 28 bit positions
// This is the code corresponding to SRAM function g_read_mram_tmc_r_timer1
// The opcodes from an IAR position independent build.
//
uint32_t
read_mram_tmc_r_timer1(void)
{

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
#else
// #### INTERNAL END ####

#define MRAM_OVERRIDE()
#define MRAM_REVERT()
#define MRAM_RECOVER()

// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END #####

//*****************************************************************************
//
// This programs up to N words of the Main MRAM
//
//*****************************************************************************
int
am_hal_mram_main_words_program(uint32_t ui32ProgramKey, uint32_t *pui32Src,
                               uint32_t *pui32Dst, uint32_t ui32NumWords)
{
    //
    // Use the new helper function to efficiently program the data in MRAM.
// #### INTERNAL BEGIN ####
    // New Apollo4 helper function that efficiently performs one of two
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
// This programs up to N words of the Main MRAM
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
// This Fills up to N words of the Main MRAM
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
    // New Apollo4 helper function that efficiently performs one of two
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
// This programs up to N words of the Main array on one MRAM.
//
//*****************************************************************************
int
am_hal_mram_info_program(uint32_t ui32InfoKey, uint32_t *pui32Src,
                         uint32_t ui32Offset, uint32_t ui32NumWords)
{
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
}

#ifdef INFO_ACCESS_WA

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
#endif

//*****************************************************************************
//
// Read INFO data.
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

#ifdef INFO_ACCESS_WA
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
#else
    for (ux = 0; ux < ui32NumWords; ux++ )
    {
        *pui32Dst++ = AM_REGVAL(pui32Info++);
    }
#endif

    return retval;
}

//*****************************************************************************
//
// Initialize MRAM for DeepSleep.
//
//*****************************************************************************
int
am_hal_mram_ds_init(void)
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
