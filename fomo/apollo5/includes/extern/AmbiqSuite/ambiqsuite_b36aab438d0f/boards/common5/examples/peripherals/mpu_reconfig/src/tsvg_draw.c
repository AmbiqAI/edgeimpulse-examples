//*****************************************************************************
//
//! @file mpu_reconfig.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "mpu_reconfig.h"
#include "tiger.tsvg.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define RESX 464
#define RESY 464

#define FRAME_BUFFER_FORMAT NEMA_RGB24

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
nema_cmdlist_t cl_clear;
nema_cmdlist_t cl_svg;
unsigned char *tsvg_bin;
nema_buffer_t fb_bo;

//*****************************************************************************
//
// MPU region setting 1
//
//*****************************************************************************
am_hal_mpu_region_config_t sMPUCfgTest_1[7] = {

                                                 {.ui32RegionNumber = 0,
                                                  .ui32BaseAddress = 0x00000000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RO_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x0003FFFFU,
                                                  .ui32AttrIndex = 0,
                                                  .bEnable = true
                                                 }, // ITCM

                                                 {.ui32RegionNumber = 1,
                                                  .ui32BaseAddress = 0x00400000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RO_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x007FFFFFU,
                                                  .ui32AttrIndex = 1,
                                                  .bEnable = true
                                                 }, // MRAM_I1

                                                 {.ui32RegionNumber = 2,
                                                  .ui32BaseAddress = 0x20000000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RW_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x2007FFFFU,
                                                  .ui32AttrIndex = 0,
                                                  .bEnable = true
                                                 }, // DTCM

                                                 {.ui32RegionNumber = 3,
                                                  .ui32BaseAddress = 0x20080000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RO_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x2017FFFFU,
                                                  .ui32AttrIndex = 1,
                                                  .bEnable = true
                                                 }, // SSRAM_I

                                                 {.ui32RegionNumber = 4,
                                                  .ui32BaseAddress = 0x20180000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RW_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x2037FFFFU,
                                                  .ui32AttrIndex = 2,
                                                  .bEnable = true
                                                 }, // SSRAM_D

                                                 {.ui32RegionNumber = 5,
                                                  .ui32BaseAddress = 0x60000000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RO_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x600FFFFFU,
                                                  .ui32AttrIndex = 1,
                                                  .bEnable = true
                                                 }, // XIPMM0_I

                                                 {.ui32RegionNumber = 6,
                                                  .ui32BaseAddress = 0x60100000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RW_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x602FFFFFU,
                                                  .ui32AttrIndex = 2,
                                                  .bEnable = true
                                                 } // XIPMM0_D
                                             };

//*****************************************************************************
//
// MPU region setting 2
//
//*****************************************************************************
am_hal_mpu_region_config_t sMPUCfgTest_2[8] = {

                                                 {.ui32RegionNumber = 0,
                                                  .ui32BaseAddress = 0x00000000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RO_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x0003FFFFU,
                                                  .ui32AttrIndex = 0,
                                                  .bEnable = true
                                                 }, // ITCM

                                                 {.ui32RegionNumber = 1,
                                                  .ui32BaseAddress = 0x00400000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RO_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x004FFFFFU,
                                                  .ui32AttrIndex = 1,
                                                  .bEnable = true
                                                 }, // MRAM_I1

                                                 {.ui32RegionNumber = 2,
                                                  .ui32BaseAddress = 0x00500000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RO_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x005FFFFFU,
                                                  .ui32AttrIndex = 0,
                                                  .bEnable = true
                                                 }, // MRAM_D

                                                 {.ui32RegionNumber = 3,
                                                  .ui32BaseAddress = 0x00600000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RO_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x007FFFFFU,
                                                  .ui32AttrIndex = 1,
                                                  .bEnable = true
                                                 }, // MRAM_I2

                                                 {.ui32RegionNumber = 4,
                                                  .ui32BaseAddress = 0x20000000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RW_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x2007FFFFU,
                                                  .ui32AttrIndex = 0,
                                                  .bEnable = true
                                                 }, // DTCM

                                                 {.ui32RegionNumber = 5,
                                                  .ui32BaseAddress = 0x20080000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RO_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x2017FFFFU,
                                                  .ui32AttrIndex = 1,
                                                  .bEnable = true
                                                 }, // SSRAM_I

                                                 {.ui32RegionNumber = 6,
                                                  .ui32BaseAddress = 0x20180000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RW_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x2037FFFFU,
                                                  .ui32AttrIndex = 2,
                                                  .bEnable = true
                                                 }, // SSRAM_D

                                                 {.ui32RegionNumber = 7,
                                                  .ui32BaseAddress = 0x60000000U,
                                                  .eShareable = NON_SHARE,
                                                  .eAccessPermission = RW_NONPRIV,
                                                  .bExecuteNever = false,
                                                  .ui32LimitAddress = 0x602FFFFFU,
                                                  .ui32AttrIndex = 2,
                                                  .bEnable = true
                                                 }, // XIPMM0

                                             };

//*****************************************************************************
//
// MPU Attribute
//
//*****************************************************************************
am_hal_mpu_attr_t sMPUAttrNew[3] = {
                                    {.ui8AttrIndex = 0, // Index 0 - Non-Cacheable region.
                                     .bNormalMem = true,
                                     .sOuterAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = false, .bWriteAllocate = false},
                                     .sInnerAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = false, .bWriteAllocate = false},
                                     .eDeviceAttr = 0
                                    },
                                    {.ui8AttrIndex = 1, // Index 1 - Read-Allocation.  Used for code segments.
                                     .bNormalMem = true,
                                     .sOuterAttr = {.bNonTransient = false, .bWriteBack = false, .bReadAllocate = true, .bWriteAllocate = false},
                                     .sInnerAttr = {.bNonTransient = false, .bWriteBack = false, .bReadAllocate = true, .bWriteAllocate = false},
                                     .eDeviceAttr = 0
                                    },
                                    {.ui8AttrIndex = 2, // Index 2 - Read-Allocation,  Write-Allocation and Write back.  Used for data segments.
                                     .bNormalMem = true,
                                     .sOuterAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = true, .bWriteAllocate = true},
                                     .sInnerAttr = {.bNonTransient = false, .bWriteBack = true, .bReadAllocate = true, .bWriteAllocate = true},
                                     .eDeviceAttr = 0
                                    },
                                };

//*****************************************************************************
//
//! @brief Config MPU
//!
//! @param config_select, 1: select sMPUCfgTest_1. 2: select region sMPUCfgTest_2
//!
//! @return None.
//
//*****************************************************************************
void mpu_config(uint32_t config_select)
{
    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(sMPUAttrNew, sizeof(sMPUAttrNew) / sizeof(am_hal_mpu_attr_t));

    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();

    //
    // Set up the regions.
    //
    if ( config_select == 0 )
    {
        am_hal_mpu_region_configure(sMPUCfgTest_1, sizeof(sMPUCfgTest_1) / sizeof(am_hal_mpu_region_config_t));
    }
    else
    {
        am_hal_mpu_region_configure(sMPUCfgTest_2, sizeof(sMPUCfgTest_2) / sizeof(am_hal_mpu_region_config_t));
    }

    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);

}

//*****************************************************************************
//
//! @brief Disable MPU
//!
//! @param None
//!
//! @return None.
//
//*****************************************************************************
void mpu_disable(void)
{
    //
    // MPU disable
    //
    am_hal_mpu_disable();
}

//*****************************************************************************
//
//! @brief Prepare to draw tiger
//!
//! @param None
//!
//! @return None.
//
//*****************************************************************************
void draw_tiger_prepare(void)
{
    //
    // Initialize NemaGFX
    //
    int ret = nema_init();
    if (ret)
    {
        am_util_stdio_printf("Nema init failed.\n");
        return;
    }

    //
    // Initialize NemaVG
    //
    nema_vg_init(RESX, RESY);

    //Create buffer for TSVG in SSRAM and load them from MRAM to ssram
    // nema_buffer_t tsvg_buffer = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, tiger_tsvg_length);
    // tsvg_bin = tsvg_buffer.base_virt;

    //
    // Create buffer for TSVG in PSRAM and load them from MRAM to ssram
    //
    tsvg_bin = (unsigned char *)MSPI_XIP_BASE_ADDRESS + 0x100000;
    memcpy(tsvg_bin, tiger_tsvg, tiger_tsvg_length);

    //
    // Allocate framebuffer
    //
    fb_bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, RESX * RESY *  nema_format_size(FRAME_BUFFER_FORMAT));
    if (fb_bo.base_virt == NULL)
    {
        am_util_stdio_printf("Could not alloc image buffer.\n");
        return;
    }

    //
    // Create cl_clear Command List that will clear the framebuffer on each frame
    //
    cl_clear = nema_cl_create();

    nema_cl_bind(&cl_clear);
    nema_cl_rewind(&cl_clear);

    nema_set_clip(0, 0, RESX, RESY);
    nema_bind_dst_tex(fb_bo.base_phys, RESX, RESY, FRAME_BUFFER_FORMAT, -1);
    nema_clear(0xffffffff);

    nema_cl_submit(&cl_clear);
    nema_cl_wait(&cl_clear);

    //
    // Create CL for tiger
    //
    cl_svg = nema_cl_create_sized(16 * 1024);
}

//*****************************************************************************
//
//! @brief Draw tiger
//!
//! @param None
//!
//! @return None.
//
//*****************************************************************************
void draw_tiger(void)
{
    //
    // Clear the framebuffer
    //
    nema_cl_submit(&cl_clear);
    nema_cl_wait(&cl_clear);

    //
    // Draw the SVG
    //
    nema_cl_bind_circular(&cl_svg);
    nema_cl_rewind(&cl_svg);

    nema_vg_draw_tsvg(tsvg_bin);

    nema_cl_submit(&cl_svg);
    nema_cl_wait(&cl_svg);

    //
    // Get error code.
    //
    uint32_t nema_error_code = nema_get_error();
    if ( nema_error_code != NEMA_ERR_NO_ERROR )
    {
        am_util_stdio_printf("nemagfx error: %8X\n", nema_error_code);
    }
}

//*****************************************************************************
//
//! @brief Clear assets
//!
//! @param None
//!
//! @return None.
//
//*****************************************************************************
void draw_tiger_clean(void)
{
    //
    // Destroy allocated assets
    //
    nema_vg_deinit();
    nema_buffer_destroy(&fb_bo);
}


int mpu_reconfig_demo(void)
{
    uint32_t count = 0;

    //
    // Prepare to draw.
    //
    draw_tiger_prepare();

    //
    // Disable MPU.
    //
    mpu_disable();

    while (1)
    {
        //
        //  Config and enable MPU.
        //
        mpu_config(count % 2);
        count ++;

        //
        // Enable cache
        //
        am_hal_cachectrl_icache_enable();
        am_hal_cachectrl_dcache_enable(true);

        //
        // Draw tiger image.
        //
        draw_tiger();

        //
        // Disable cache
        //
        am_hal_cachectrl_icache_disable();
        am_hal_cachectrl_dcache_disable();

        //
        // Disable MPU.
        //
        mpu_disable();

        //
        // Output log to indicate status.
        //
        am_util_stdio_printf("Running....: %8X\n", count);
    }

    draw_tiger_clean();

    return 0;
}
