//*****************************************************************************
//
//! @file mspi_loader_example.c
//!
//! @brief Example demonstrating how to load and run binary image to external MSPI devices.
//!
//! Purpose: The binary must be linked to run from MSPI XIP Aperture range
//! (as specified by BIN_INSTALL_ADDR). The location and size of the binary
//! in internal flash are specified using BIN_ADDR_FLASH & BIN_SIZE
//!
//! This example has been enhanced to use the new 'binary patching' feature
//! This example will not build if proper startup/linker files are not used.
//!
//! Additional Information:
//! Prepare the example as follows:
//!     1. Generate hello_world example to load and execute at MSPI XIP location.
//!         i. In the /examples/hello_world/gcc directory modify the MRAM region to MSPIx_APERTURE_START_ADDR and MSPIx_APERTURE_END_ADDR
//!         ii. Execute "make" in the /examples/hello_world/iar directory to rebuild the project.
//!     2. Copy /examples/hello_world/iar/bin/hello_world.bin into /boards/common4/examples/interfaces/mspi_loader_example/
//!     3. Create the binary with mspi_loader_example + external executable from Step #1.
//!         ./mspi_loader_binary_combiner.py --loaderbin iar/bin/mspi_loader_example.bin --appbin hello_world.bin --install-address MSPIx_APERTURE_START_ADDR --flags 0x2 --outbin loader_hello_world --loader-address 0x410000
//!     4. Open the J-Link SWO Viewer to the target board.
//!     5. Open the J-Flash Lite program.  Select the /examples/interfaces/mspi_loader_example/loader_hello_world.bin file and program at 0x410000 offset.
//!
//!
//! Starting from apollo5b, the MSPI XIP read bandwidth is boosted by the
//! ARM CM55 cache and the MSPI CPU read combine feature. By default,
//! the CPU read queue is on(CPURQEN). Cache prefetch(RID3) and cache miss(RID2)
//! requests deemed appropriate by MSPI hardware are combined and processed
//! with a 2:1 ratio(GQARB) between general queue and CPU read queue.
//!
//! am_hal_mspi_cpu_read_burst_t default =
//! {
//!     .eGQArbBais                         = AM_HAL_MSPI_GQARB_2_1_BAIS,
//!     .bCombineRID2CacheMissAccess        = true,
//!     .bCombineRID3CachePrefetchAccess    = true,
//!     .bCombineRID4ICacheAccess           = false,
//!     .bCPUReadQueueEnable                = true,
//! }
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"
#include "mspi_loader_example.h"

#include <string.h>

#define MSPI_TIMING_SCAN
#define MSPI_MODULE             0
#define MSPI_CLK_FREQ_INDEX     3
// #### INTERNAL BEGIN ####
#if defined (APOLLO5_FPGA)
#undef MSPI_TIMING_SCAN
#undef MSPI_MODULE
#undef MSPI_CLK_FREQ_INDEX

#define MSPI_CLK_FREQ_INDEX     0

#if defined(APS25616N) || defined(APS25616BA) || defined(W958D6NW)
#define MSPI_MODULE             0
#elif defined(ATXP032) || defined(IS25WX064)
#define MSPI_MODULE             1
#endif

#endif
// #### INTERNAL END ####

#define READ_BUFFER_SIZE        2048

// Patchable section of binary
extern uint32_t * __pPatchable;

void            *g_pDevHandle;
void            *g_pHandle;
AM_SHARED_RW uint32_t        g_ReadBuf[READ_BUFFER_SIZE / 4] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t        DMATCBBuffer[256];

am_hal_mpu_region_config_t sMPUCfg =
{
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)DMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)DMATCBBuffer + sizeof(DMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
};
am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .sInnerAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .eDeviceAttr = 0,
};

mspi_device_mode_str_t mspi_device_modes[] =
{
  { AM_HAL_MSPI_FLASH_SERIAL_CE0,       " SERIAL CE0" },
  { AM_HAL_MSPI_FLASH_SERIAL_CE1,       " SERIAL CE1" },
  { AM_HAL_MSPI_FLASH_DUAL_CE0,         " DUAL CE0" },
  { AM_HAL_MSPI_FLASH_DUAL_CE1,         " DUAL CE1" },
  { AM_HAL_MSPI_FLASH_QUAD_CE0,         " QUAD CE0" },
  { AM_HAL_MSPI_FLASH_QUAD_CE1,         " QUAD CE1" },
  { AM_HAL_MSPI_FLASH_OCTAL_CE0,        " OCTAL CE0" },
  { AM_HAL_MSPI_FLASH_OCTAL_CE1,        " OCTAL CE1" },
  { AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,    " OCTAL DDR CE0" },
  { AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,    " OCTAL DDR CE1" },
  { AM_HAL_MSPI_FLASH_HEX_DDR_CE0,      " HEX DDR CE0" },
  { AM_HAL_MSPI_FLASH_HEX_DDR_CE1,      " HEX DDR CE1" },
} ;

#if defined(APS25616N) || defined(APS25616BA) || defined(W958D6NW)

mspi_speed_t mspi_speeds[] =
{
  { AM_HAL_MSPI_CLK_250MHZ,     " 125MHz" },
  { AM_HAL_MSPI_CLK_192MHZ,     " 96MHz" },
  { AM_HAL_MSPI_CLK_125MHZ,     "62.5MHz" },
  { AM_HAL_MSPI_CLK_96MHZ,      " 48MHz" },

  { AM_HAL_MSPI_CLK_62P5MHZ,    "31.3MHz"},
  { AM_HAL_MSPI_CLK_48MHZ,      " 24MHz" },
} ;

am_abstract_mspi_devices_config_t sMSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_250MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

#elif defined(ATXP032)

mspi_speed_t mspi_speeds[] =
{
  { AM_HAL_MSPI_CLK_250MHZ,     " 250MHz" },
  { AM_HAL_MSPI_CLK_192MHZ,     " 192MHz" },
  { AM_HAL_MSPI_CLK_125MHZ,     " 125MHz" },
  { AM_HAL_MSPI_CLK_96MHZ,      " 96MHz" },

  { AM_HAL_MSPI_CLK_62P5MHZ,    " 62.5MHz"},
  { AM_HAL_MSPI_CLK_48MHZ,      " 48MHz" },
} ;

am_abstract_mspi_devices_config_t sMSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

#elif defined(IS25WX064)

mspi_speed_t mspi_speeds[] =
{
  { AM_HAL_MSPI_CLK_250MHZ,     " 250MHz" },
  { AM_HAL_MSPI_CLK_192MHZ,     " 192MHz" },
  { AM_HAL_MSPI_CLK_125MHZ,     " 125MHz" },
  { AM_HAL_MSPI_CLK_96MHZ,      " 96MHz" },

  { AM_HAL_MSPI_CLK_62P5MHZ,    " 62.5MHz"},
  { AM_HAL_MSPI_CLK_48MHZ,      " 48MHz" },
} ;

am_abstract_mspi_devices_config_t sMSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

#endif

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
    MSPI3_IRQn,
};

#if defined (AM_PART_APOLLO5A)
const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES] =
{
  {MSPI0_XIP_BASEADDR, MSPI0_APERTURE_START_ADDR},
  {MSPI1_XIP_BASEADDR, MSPI1_APERTURE_START_ADDR},
  {MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_START_ADDR},
  {MSPI3_XIP_BASEADDR, MSPI3_APERTURE_START_ADDR},
};
#elif defined (AM_PART_BRONCO) || defined (AM_PART_APOLLO5B)
const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES] =
{
  {MSPI0_APERTURE_START_ADDR, MSPI0_APERTURE_START_ADDR},
  {MSPI1_APERTURE_START_ADDR, MSPI1_APERTURE_START_ADDR},
  {MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_START_ADDR},
  {MSPI3_APERTURE_START_ADDR, MSPI3_APERTURE_START_ADDR},
};
#endif

mspi_device_func_t mspi_device_func =
{
#if defined(APS25616N)
    .devName = "APS25616N",
    .mspi_init                  = am_devices_mspi_psram_aps25616n_ddr_init,
    .mspi_term                  = am_devices_mspi_psram_aps25616n_ddr_deinit,
    .mspi_read_id               = am_devices_mspi_psram_aps25616n_ddr_id,
    .mspi_read                  = am_devices_mspi_psram_aps25616n_ddr_read,
    .mspi_write                 = am_devices_mspi_psram_aps25616n_ddr_write,
    .mspi_xip_enable            = am_devices_mspi_psram_aps25616n_ddr_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_psram_aps25616n_ddr_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_psram_aps25616n_ddr_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_psram_aps25616n_ddr_disable_scrambling,
    .mspi_init_timing_check     = am_devices_mspi_psram_aps25616n_ddr_init_timing_check,
    .mspi_init_timing_apply     = am_devices_mspi_psram_aps25616n_apply_ddr_timing,
#elif defined(APS25616BA)
    .devName = "APS25616BA",
    .mspi_init                  = am_devices_mspi_psram_aps25616ba_ddr_init,
    .mspi_term                  = am_devices_mspi_psram_aps25616ba_ddr_deinit,
    .mspi_read_id               = am_devices_mspi_psram_aps25616ba_ddr_id,
    .mspi_read                  = am_devices_mspi_psram_aps25616ba_ddr_read,
    .mspi_write                 = am_devices_mspi_psram_aps25616ba_ddr_write,
    .mspi_xip_enable            = am_devices_mspi_psram_aps25616ba_ddr_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_psram_aps25616ba_ddr_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_psram_aps25616ba_ddr_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_psram_aps25616ba_ddr_disable_scrambling,
    .mspi_init_timing_check     = am_devices_mspi_psram_aps25616ba_ddr_init_timing_check,
    .mspi_init_timing_apply     = am_devices_mspi_psram_aps25616ba_apply_ddr_timing,
#elif defined(W958D6NW)
    .devName = "W958D6NW",
    .mspi_init                  = am_devices_mspi_psram_w958d6nw_ddr_init,
    .mspi_term                  = am_devices_mspi_psram_w958d6nw_ddr_deinit,
    .mspi_read_id               = am_devices_mspi_psram_w958d6nw_ddr_id,
    .mspi_read                  = am_devices_mspi_psram_w958d6nw_ddr_read,
    .mspi_write                 = am_devices_mspi_psram_w958d6nw_ddr_write,
    .mspi_xip_enable            = am_devices_mspi_psram_w958d6nw_ddr_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_psram_w958d6nw_ddr_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_psram_w958d6nw_ddr_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_psram_w958d6nw_ddr_disable_scrambling,
    .mspi_init_timing_check     = am_devices_mspi_psram_hex_ddr_init_timing_check,
    .mspi_init_timing_apply     = am_devices_mspi_psram_apply_hex_ddr_timing,
#elif defined(ATXP032)
    .devName = "ATXP032",
    .mspi_init                  = am_devices_mspi_atxp032_init,
    .mspi_init_timing_check     = am_devices_mspi_atxp032_sdr_init_timing_check,
    .mspi_init_timing_apply     = am_devices_mspi_atxp032_apply_sdr_timing,
    .mspi_term                  = am_devices_mspi_atxp032_deinit,
    .mspi_read_id               = am_devices_mspi_atxp032_id,
    .mspi_read                  = am_devices_mspi_atxp032_read,
    .mspi_read_adv              = am_devices_mspi_atxp032_read_adv,
    .mspi_read_callback         = am_devices_mspi_atxp032_read_callback,
    .mspi_write                 = am_devices_mspi_atxp032_write,
    .mspi_mass_erase            = am_devices_mspi_atxp032_mass_erase,
    .mspi_sector_erase          = am_devices_mspi_atxp032_sector_erase,
    .mspi_xip_enable            = am_devices_mspi_atxp032_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_atxp032_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_atxp032_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_atxp032_disable_scrambling,
#elif defined(IS25WX064)
    .devName = "IS25WX064",
    .mspi_init                  = am_devices_mspi_is25wx064_init,
    .mspi_init_timing_check     = am_devices_mspi_is25wx064_init_timing_check,
    .mspi_init_timing_apply     = am_devices_mspi_is25wx064_apply_ddr_timing,
    .mspi_term                  = am_devices_mspi_is25wx064_deinit,
    .mspi_read_id               = am_devices_mspi_is25wx064_id,
    .mspi_read                  = am_devices_mspi_is25wx064_read,
    .mspi_read_adv              = am_devices_mspi_is25wx064_read_adv,
    .mspi_read_callback         = am_devices_mspi_is25wx064_read_callback,
    .mspi_write                 = am_devices_mspi_is25wx064_write,
    .mspi_mass_erase            = am_devices_mspi_is25wx064_mass_erase,
    .mspi_sector_erase          = am_devices_mspi_is25wx064_sector_erase,
    .mspi_xip_enable            = am_devices_mspi_is25wx064_enable_xip,
    .mspi_xip_disable           = am_devices_mspi_is25wx064_disable_xip,
    .mspi_scrambling_enable     = am_devices_mspi_is25wx064_enable_scrambling,
    .mspi_scrambling_disable    = am_devices_mspi_is25wx064_disable_scrambling,
#endif
};
//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define mspi_isr                                                          \
    am_mspi_isr1(MSPI_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pHandle, ui32Status);
}

// This function intializes the VTOR, SP and jumps the the Reset Vector of the image provided
#if defined(__GNUC_STDC_INLINE__)
__attribute__((naked))
static void
run_new_image(uint32_t *vtor)
{
    __asm
    (
        "   movw    r3, #0xED08\n\t"    // Store the vector table pointer of the new image into VTOR.
        "   movt    r3, #0xE000\n\t"
        "   str     r0, [r3, #0]\n\t"
        "   ldr     r3, [r0, #0]\n\t"   // Load the new stack pointer into R3 and the new reset vector into R2.
        "   ldr     r2, [r0, #4]\n\t"
        "   mov     sp, r3\n\t"         // Set the stack pointer for the new image.
        "   bx      r2\n\t"            // Jump to the new reset vector.
    );
}

#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION > 6000000)
__attribute__((naked))
static void
run_new_image(uint32_t *vtor)
{
    //
    // Store the vector table pointer of the new image into VTOR.
    //
    __asm("movw    r3, #0xED08\n");
    __asm("movt    r3, #0xE000\n");
    __asm("str     r0, [r3, #0]\n");

    //
    // Load the new stack pointer into R1 and the new reset vector into R2.
    //
    __asm("ldr     r3, [r0, #0]\n");
    __asm("ldr     r2, [r0, #4]\n");

    //
    // Set the stack pointer for the new image.
    //
    __asm("mov     sp, r3\n");


    //
    // Jump to the new reset vector.
    //
    __asm("bx      r2\n");

}

#elif (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION <  6000000)
static __asm void
run_new_image(uint32_t *vtor)
{
    //
    // Store the vector table pointer of the new image into VTOR.
    //
    movw    r3, #0xED08
    movt    r3, #0xE000
    str     r0, [r3, #0]

    //
    // Load the new stack pointer into R1 and the new reset vector into R2.
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
__stackless static inline void
run_new_image(uint32_t *vtor)
{
    __asm volatile (
          "    movw    r3, #0xED08\n"    // Store the vector table pointer of the new image into VTOR.
          "    movt    r3, #0xE000\n"
          "    str     r0, [r3, #0]\n"
          "    ldr     r3, [r0, #0]\n"   // Load the new stack pointer into R1 and the new reset vector into R2.
          "    ldr     r2, [r0, #4]\n"
          "    mov     sp, r3\n"         // Set the stack pointer for the new image.
          "    bx      r2\n"            // Jump to the new reset vector.
          );
}
#else
#error "IDE not supported"
#endif

// Default Details of binary in internal flash
#define BIN_ADDR_FLASH          0x500000
#define BIN_SIZE                0x100000
#define BIN_INSTALL_ADDR        g_MSPIAddresses[MSPI_MODULE].XIPMMBase

//*****************************************************************************
//
// MSPI Example Main.
//
//*****************************************************************************
int
main(void)
{
    uint32_t      ui32Status;

    am_abstract_mspi_devices_timing_config_t sMSPITimingConfig;

    bool          bScramble = false;
    bool          bRun = false;

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();
    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttr, 1);
    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();
    //
    // Set up the regions.
    //
    am_hal_mpu_region_configure(&sMPUCfg, 1);
    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);
    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_debug_printf("Apollo5 MSPI Device Loader Example\n\n");

    uint32_t ui32BinSourceAddr, ui32BinExecuteAddr, ui32BinSize, ui32InstallOffset;

    // some of the parameters are controllable through binary patching
    if (__pPatchable[0])
    {
        am_util_debug_printf("Valid Patched information found\n");
        ui32BinSourceAddr = __pPatchable[0];
        ui32BinExecuteAddr = __pPatchable[1];
        ui32BinSize = __pPatchable[2];
        ui32InstallOffset = (ui32BinExecuteAddr - g_MSPIAddresses[MSPI_MODULE].XIPMMBase);
        if (__pPatchable[3] & 0x1)
        {
            // Enable scrambling
            bScramble = true;
        }
        if (__pPatchable[3] & 0x2)
        {
            // Jump to program after installing
            bRun = true;
        }
// #### INTERNAL BEGIN ####
        if (__pPatchable[3] & 0x4)
        {
            // Disable the cache
            //bCacheDisable = true;
        }
// #### INTERNAL END ####
    }
    else
    {
        ui32BinSourceAddr = BIN_ADDR_FLASH;
        ui32BinExecuteAddr = BIN_INSTALL_ADDR;
        ui32InstallOffset = (BIN_INSTALL_ADDR - g_MSPIAddresses[MSPI_MODULE].XIPMMBase);
        ui32BinSize = BIN_SIZE;
    }

    am_util_debug_printf("Bin address in internal flash = 0x%x\n", ui32BinSourceAddr);
    am_util_debug_printf("Bin address in external Device = 0x%x\n", ui32InstallOffset);
    am_util_debug_printf("Bin Size = 0x%x\n", ui32BinSize);
    am_util_debug_printf("Bin install address = 0x%x\n", ui32BinExecuteAddr);
    am_util_debug_printf("Scrambling is %s\n", bScramble ? "Enabled" : "Disabled");

    am_util_debug_printf("\nDevice %s interface with MSPI%d, Configured%s @%s\n", mspi_device_func.devName,
                                                                                   MSPI_MODULE,
                                                                                   mspi_device_modes[sMSPIConfig.eDeviceConfig].string,
                                                                                   mspi_speeds[MSPI_CLK_FREQ_INDEX].string);

    if ( bScramble )
    {
        sMSPIConfig.ui32ScramblingStartAddr = ui32InstallOffset;
        sMSPIConfig.ui32ScramblingEndAddr = ui32InstallOffset + ui32BinSize - 1;
    }
    sMSPIConfig.eClockFreq = mspi_speeds[MSPI_CLK_FREQ_INDEX].eFreq;

#if defined(MSPI_TIMING_SCAN)
    am_util_debug_printf("Starting MSPI Timing Scan: \n");
    if ( AM_ABSTRACT_MSPI_SUCCESS == mspi_device_func.mspi_init_timing_check(MSPI_MODULE, (void*)&sMSPIConfig, &sMSPITimingConfig) )
    {
        am_util_debug_printf("Scan Result: OK\n");
    }
    else
    {
        am_util_stdio_printf("Scan Result: Failed, no valid setting.  \n");
        return -1;
    }
#endif
    //
    // Configure the MSPI and Flash Device.
    //
    ui32Status = mspi_device_func.mspi_init(MSPI_MODULE, (void*)&sMSPIConfig, &g_pDevHandle, &g_pHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_MODULE]);
    am_hal_interrupt_master_enable();

#if defined(MSPI_TIMING_SCAN)
    //
    //  Set the timing from previous scan.
    //
    mspi_device_func.mspi_init_timing_apply(g_pDevHandle, &sMSPITimingConfig);
#endif

    if (bScramble)
    {
        //
        // Turn on scrambling operation.
        //
        am_util_debug_printf("Putting the MSPI into Scrambling mode\n");
        ui32Status = mspi_device_func.mspi_scrambling_enable(g_pDevHandle);
        if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
        {
            am_util_debug_printf("Failed to enable MSPI scrambling!\n");
            return -1;
        }
    }

    //
    // ui32InstallOffset shall be mutiple of sector size
    //
#if defined(REQUIRE_ERASE)
    uint32_t ui32EraseStart = ui32InstallOffset / AM_DEVICES_MSPI_FLASH_SECTOR_SIZE;
    uint32_t ui32EraseCnt = ui32BinSize / AM_DEVICES_MSPI_FLASH_SECTOR_SIZE;
    if ( ui32BinSize % AM_DEVICES_MSPI_FLASH_SECTOR_SIZE )
    {
        ui32EraseCnt += 1;
    }
    am_util_debug_printf("Erasing External MSPI Device.");
    for ( uint8_t i = 0; i < ui32EraseCnt; i++ )
    {
        ui32Status = mspi_device_func.mspi_sector_erase(g_pDevHandle, (ui32EraseStart + i) * AM_DEVICES_MSPI_FLASH_SECTOR_SIZE);
        if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("\nFailed to erase Flash Device sector!\n");
            return -1;
        }
        am_util_debug_printf(".");
    }
#endif

    //
    // Write the executable binary into MSPI device
    //
    am_util_debug_printf("\nWriting image to External MSPI Device!\n");
    ui32Status = mspi_device_func.mspi_write(g_pDevHandle, (uint8_t *)ui32BinSourceAddr, ui32InstallOffset, ui32BinSize, true);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_debug_printf("Failed to write buffer to MSPI Device!\n");
        return -1;
    }

    //
    // Confirm that the device now has the correct data
    //
    am_util_debug_printf("Verifying image in External MSPI Device.");
#if defined(VERIFY_WITH_DMA)
    uint32_t ui32DeviceAddress = ui32InstallOffset;
    uint32_t ui32ReadSize = READ_BUFFER_SIZE;
    uint32_t ui32NumbytesLeft = ui32BinSize;
    do
    {
        if ( ui32NumbytesLeft > READ_BUFFER_SIZE )
        {
            ui32NumbytesLeft -= READ_BUFFER_SIZE;
        }
        else
        {
            ui32ReadSize = ui32NumbytesLeft;
            ui32NumbytesLeft = 0;
        }
        //
        // Read the data back into the RX buffer.
        //
        ui32Status = mspi_device_func.mspi_read(g_pDevHandle, (uint8_t *)g_ReadBuf, ui32DeviceAddress, ui32ReadSize, true);
        if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
        {
            am_util_debug_printf("Failed to read image from Device!\n");
            return -1;
        }

        //
        // Compare the buffers
        //
        uint32_t *pui32Source = (uint32_t *)(ui32BinSourceAddr + ui32DeviceAddress - ui32InstallOffset);
        for (uint32_t i = 0; i < ui32ReadSize / 4; i++)
        {
            uint32_t ui32Source = *pui32Source++;
            if ( ui32Source != g_ReadBuf[i] )
            {
                am_util_debug_printf("Failed to verify image at offset 0x%x - Expected 0x%x Actual 0x%x!\n",
                                    ui32DeviceAddress - ui32InstallOffset + i*4, ui32Source, g_ReadBuf[i]);
                return -1;
            }
        }
        ui32DeviceAddress += ui32ReadSize;
        am_util_debug_printf(".");
    } while (ui32NumbytesLeft > 0);

#elif defined(VERIFY_WITH_XIP)
    am_util_debug_printf("\nPutting the MSPI and External Device into XIP mode\n");
    ui32Status = mspi_device_func.mspi_xip_enable(g_pDevHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_debug_printf("Failed to put MSPI into XIP mode!\n");
        return -1;
    }

    ui32Status = memcmp((void *)ui32BinSourceAddr, (uint32_t *)ui32BinExecuteAddr, ui32BinSize);
    if ( ui32Status != 0 )
    {
        am_util_debug_printf("Failed to verify image\n");
    }
#else
#warning "No image verification will be conducted!!!"
#endif
    am_util_debug_printf("\nImage verified successfully!\n");

    if (bRun)
    {
        //
        // Set up for XIP operation.
        //
        am_util_debug_printf("Putting the MSPI and External Device into XIP mode\n");
        ui32Status = mspi_device_func.mspi_xip_enable(g_pDevHandle);
        if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
        {
            am_util_debug_printf("Failed to put MSPI into XIP mode!\n");
            return -1;
        }

        uint32_t vtor = ui32BinExecuteAddr;

        am_util_debug_printf("Jumping to relocated image in MSPI Device\n");
        // Add delay
        am_util_delay_ms(100);
        // Run binary from MSPI
        run_new_image((uint32_t *)vtor);
        // Will not return!!
    }
    else
    {
        am_util_debug_printf("MSPI Device Loaded\n");
        while(1);
    }
}
