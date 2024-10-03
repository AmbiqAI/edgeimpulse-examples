//*****************************************************************************
//
//! @file nemagfx_rw_psram_test_cases.c
//!
//! @brief NemaGFX reads/writes PSRAM test cases.
//! Need to connect APS25616 PSRAM card to MSPI0.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemagfx_test_common.h"
#include "windmill_200x200_rgba8888.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FB_RESX 200
#define FB_RESY 200

typedef enum
{
    GPU_MSPI_BURST_LENGTH_16 = 4,
    GPU_MSPI_BURST_LENGTH_32 = 5,
    GPU_MSPI_BURST_LENGTH_64 = 6,
    GPU_MSPI_BURST_LENGTH_128 = 7,
} nemagfx_mspi_burst_length_t;

//*****************************************************************************
//
// Buffer location
//
//*****************************************************************************
//Test process
//1. MRAM--->buffer_A_SSRAM: CPU memcpy
//2. buffer_A_SSRAM ---> buffer_B_PSRAM: GPU blit
//3. buffer_B_PSRAM ---> buffer_C_SSRAM: GPU blit
//4. compare buffer_A_SSRAM and buffer_C_SSRAM

img_obj_t buffer_A_SSRAM = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_RGBA8888, 0};
img_obj_t buffer_B_PSRAM = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_RGBA8888, 0};
img_obj_t buffer_C_SSRAM = {{0}, FB_RESX, FB_RESY, -1, 0, NEMA_RGBA8888, 0};


//*****************************************************************************
//
// Load objects.
//
//*****************************************************************************
void
load_objects(void)
{
    buffer_A_SSRAM.bo = nema_buffer_create_pool(NEMA_MEM_POOL_ASSETS, buffer_A_SSRAM.w * buffer_A_SSRAM.h * 4);
    nema_memcpy(buffer_A_SSRAM.bo.base_virt, windmill_200x200_rgba8888, buffer_A_SSRAM.bo.size);

    buffer_B_PSRAM.bo.base_phys = (uintptr_t)MSPI_XIP_BASE_ADDRESS;
    buffer_B_PSRAM.bo.base_virt = (void*)buffer_B_PSRAM.bo.base_phys;
    buffer_B_PSRAM.bo.size = buffer_B_PSRAM.w * buffer_B_PSRAM.h * 4;

    buffer_C_SSRAM.bo = nema_buffer_create_pool(NEMA_MEM_POOL_FB, buffer_C_SSRAM.w * buffer_C_SSRAM.h * 4);
}

void
destroy_objects(void)
{
    nema_buffer_destroy(&buffer_A_SSRAM.bo);
    nema_buffer_destroy(&buffer_C_SSRAM.bo);
}

//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
    load_objects();
}

void
tearDown(void)
{
    destroy_objects();
}

//*****************************************************************************
//
//! @brief GPU read/write PSRAM with continuous address sequence.
//!
//!
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemagfx_continuous_rw_psram_test(void)
{
    bool bTestPass = true;
    am_util_stdio_printf("\n -----------------------------------------------");
    am_util_stdio_printf("\n ----nemagfx read/write PSRAM: continuous----");
    am_util_stdio_printf("\n -----------------------------------------------");

    nemagfx_mspi_burst_length_t fb_burst_length = GPU_MSPI_BURST_LENGTH_16;
    nemagfx_mspi_burst_length_t tex_burst_length = GPU_MSPI_BURST_LENGTH_16;

    nema_cmdlist_t sCL = nema_cl_create();
    nema_cl_bind(&sCL);

    while(fb_burst_length <= GPU_MSPI_BURST_LENGTH_128)
    {
        while(tex_burst_length <= GPU_MSPI_BURST_LENGTH_128)
        {
            uint32_t burst_size_reg_fb = 0x1UL<<fb_burst_length;
            uint32_t burst_size_reg_tex = 0x1UL<<tex_burst_length;
            uint32_t burst_size_reg_val = 0x0UL|(fb_burst_length<<4)|(tex_burst_length);

            am_util_stdio_printf("\n\n\n");
            am_util_stdio_printf("FB burst length: %d \n", burst_size_reg_fb);
            am_util_stdio_printf("TEX burst length: %d \n", burst_size_reg_tex);
            am_util_stdio_printf("Burst size register value: %08X \n", burst_size_reg_val);
            //
            // GPU blit from ssram to PSRAM
            //
            nema_cl_rewind(&sCL);
            nema_cl_bind(&sCL);
            nema_bind_dst_tex(buffer_B_PSRAM.bo.base_phys,
                              buffer_B_PSRAM.w,
                              buffer_B_PSRAM.h,
                              buffer_B_PSRAM.format,
                              buffer_B_PSRAM.stride);
            nema_bind_src_tex(buffer_A_SSRAM.bo.base_phys,
                              buffer_A_SSRAM.w,
                              buffer_A_SSRAM.h,
                              buffer_A_SSRAM.format,
                              buffer_A_SSRAM.stride,
                              NEMA_FILTER_PS);
            nema_set_clip(0, 0, FB_RESX, FB_RESY);
            nema_cl_add_cmd(NEMA_BURST_SIZE, burst_size_reg_val);
            nema_clear(0);
            nema_set_blend_blit(NEMA_BL_SRC);
            nema_blit(0, 0);
            nema_cl_submit(&sCL);
            nema_cl_wait(&sCL);

            //
            // GPU blit from psram to ssram
            //
            nema_cl_rewind(&sCL);
            nema_cl_bind(&sCL);
            nema_bind_dst_tex(buffer_C_SSRAM.bo.base_phys,
                              buffer_C_SSRAM.w,
                              buffer_C_SSRAM.h,
                              buffer_C_SSRAM.format,
                              buffer_C_SSRAM.stride);
            nema_bind_src_tex(buffer_B_PSRAM.bo.base_phys,
                              buffer_B_PSRAM.w,
                              buffer_B_PSRAM.h,
                              buffer_B_PSRAM.format,
                              buffer_B_PSRAM.stride,
                              NEMA_FILTER_PS);
            nema_set_clip(0, 0, FB_RESX, FB_RESY);
            nema_cl_add_cmd(NEMA_BURST_SIZE, burst_size_reg_val);
            nema_clear(0);
            nema_set_blend_blit(NEMA_BL_SRC);
            nema_blit(0, 0);
            nema_cl_submit(&sCL);
            nema_cl_wait(&sCL);

            //
            // Run the memcmp for Buffer_A and Buffer_C
            //
            int cmp_ret = memcmp(buffer_C_SSRAM.bo.base_virt, 
                                 buffer_A_SSRAM.bo.base_virt, 
                                 buffer_A_SSRAM.w * buffer_A_SSRAM.h * 4);

            if(cmp_ret)
            {
                am_util_stdio_printf("Frame buffer compare FAILED! \n");
                bTestPass = false;
            }
            else
            {
                am_util_stdio_printf("Frame buffer compare PASS \n");
            }

            TEST_ASSERT_TRUE(bTestPass);

            tex_burst_length ++;
        }

        tex_burst_length = GPU_MSPI_BURST_LENGTH_16;
        fb_burst_length ++;
    }

    nema_cl_destroy(&sCL);

    return bTestPass;
} // nemagfx_rw_psram_test

//*****************************************************************************
//
//! @brief GPU read/write PSRAM with discontinuous address sequence.
//!
//!
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemagfx_discontinuous_rw_psram_test(void)
{
    bool bTestPass = true;
    am_util_stdio_printf("\n -----------------------------------------------");
    am_util_stdio_printf("\n ----nemagfx read/write PSRAM: discontinuous----");
    am_util_stdio_printf("\n -----------------------------------------------");

    nemagfx_mspi_burst_length_t fb_burst_length = GPU_MSPI_BURST_LENGTH_16;
    nemagfx_mspi_burst_length_t tex_burst_length = GPU_MSPI_BURST_LENGTH_16;

    nema_cmdlist_t sCL = nema_cl_create();
    nema_cl_bind(&sCL);

    while(fb_burst_length <= GPU_MSPI_BURST_LENGTH_128)
    {
        while(tex_burst_length <= GPU_MSPI_BURST_LENGTH_128)
        {
            uint32_t burst_size_reg_fb = 0x1UL<<fb_burst_length;
            uint32_t burst_size_reg_tex = 0x1UL<<tex_burst_length;
            uint32_t burst_size_reg_val = 0x0UL|(fb_burst_length<<4)|(tex_burst_length);

            am_util_stdio_printf("\n\n\n");
            am_util_stdio_printf("FB burst length: %d \n", burst_size_reg_fb);
            am_util_stdio_printf("TEX burst length: %d \n", burst_size_reg_tex);
            am_util_stdio_printf("Burst size register value: %08X \n", burst_size_reg_val);
            //
            // GPU blit from ssram to PSRAM
            //
            nema_cl_rewind(&sCL);
            nema_cl_bind(&sCL);
            nema_bind_dst_tex(buffer_B_PSRAM.bo.base_phys,
                              buffer_B_PSRAM.w,
                              buffer_B_PSRAM.h,
                              buffer_B_PSRAM.format,
                              buffer_B_PSRAM.stride);
            nema_bind_src_tex(buffer_A_SSRAM.bo.base_phys,
                              buffer_A_SSRAM.w,
                              buffer_A_SSRAM.h,
                              buffer_A_SSRAM.format,
                              buffer_A_SSRAM.stride,
                              NEMA_FILTER_PS);
            nema_set_clip(0, 0, FB_RESX, FB_RESY);
            nema_cl_add_cmd(NEMA_BURST_SIZE, burst_size_reg_val);
            nema_clear(0);
            nema_set_blend_blit(NEMA_BL_SRC);
            nema_blit_rotate(0, 0, 90);
            nema_cl_submit(&sCL);
            nema_cl_wait(&sCL);

            //
            // GPU blit from psram to ssram
            //
            nema_cl_rewind(&sCL);
            nema_cl_bind(&sCL);
            nema_bind_dst_tex(buffer_C_SSRAM.bo.base_phys,
                              buffer_C_SSRAM.w,
                              buffer_C_SSRAM.h,
                              buffer_C_SSRAM.format,
                              buffer_C_SSRAM.stride);
            nema_bind_src_tex(buffer_B_PSRAM.bo.base_phys,
                              buffer_B_PSRAM.w,
                              buffer_B_PSRAM.h,
                              buffer_B_PSRAM.format,
                              buffer_B_PSRAM.stride,
                              NEMA_FILTER_PS);
            nema_set_clip(0, 0, FB_RESX, FB_RESY);
            nema_cl_add_cmd(NEMA_BURST_SIZE, burst_size_reg_val);
            nema_clear(0);
            nema_set_blend_blit(NEMA_BL_SRC);
            nema_blit_rotate(0, 0, 270);
            nema_cl_submit(&sCL);
            nema_cl_wait(&sCL);

            //
            // Run the memcmp for Buffer_A and Buffer_C
            //
            int cmp_ret = memcmp(buffer_C_SSRAM.bo.base_virt, 
                                 buffer_A_SSRAM.bo.base_virt, 
                                 buffer_A_SSRAM.w * buffer_A_SSRAM.h * 4);

            if(cmp_ret)
            {
                am_util_stdio_printf("Frame buffer compare FAILED! \n");
                bTestPass = false;
            }
            else
            {
                am_util_stdio_printf("Frame buffer compare PASS \n");
            }

            TEST_ASSERT_TRUE(bTestPass);

            tex_burst_length ++;
        }
        tex_burst_length = GPU_MSPI_BURST_LENGTH_16;
        fb_burst_length ++;
    }

    nema_cl_destroy(&sCL);

    return bTestPass;
} // nemagfx_rw_psram_test

//*****************************************************************************
//
//! @brief GPU read/write PSRAM with blend effect, the frame buffer read burst 
//!        length will be tested in this test case.
//!
//!
//!
//! @return bTestPass.
//
//*****************************************************************************
bool
nemagfx_blend_rw_psram_test(void)
{
    bool bTestPass = true;
    am_util_stdio_printf("\n -----------------------------------------------");
    am_util_stdio_printf("\n ----nemagfx read/write PSRAM: blend----");
    am_util_stdio_printf("\n -----------------------------------------------");

    nemagfx_mspi_burst_length_t fb_burst_length = GPU_MSPI_BURST_LENGTH_16;
    nemagfx_mspi_burst_length_t tex_burst_length = GPU_MSPI_BURST_LENGTH_16;

    nema_cmdlist_t sCL = nema_cl_create();
    nema_cl_bind(&sCL);

    while(fb_burst_length <= GPU_MSPI_BURST_LENGTH_128)
    {
        uint32_t burst_size_reg_fb = 0x1UL<<fb_burst_length;
        uint32_t burst_size_reg_tex = 0x1UL<<tex_burst_length;
        uint32_t burst_size_reg_val = 0x0UL|(fb_burst_length<<4)|(tex_burst_length);

        am_util_stdio_printf("\n\n\n");
        am_util_stdio_printf("FB burst length: %d \n", burst_size_reg_fb);
        am_util_stdio_printf("TEX burst length: %d \n", burst_size_reg_tex);
        am_util_stdio_printf("Burst size register value: %08X \n", burst_size_reg_val);
        //
        // GPU blit from ssram to PSRAM
        //
        nema_cl_rewind(&sCL);
        nema_cl_bind(&sCL);
        nema_bind_dst_tex(buffer_B_PSRAM.bo.base_phys,
                          buffer_B_PSRAM.w,
                          buffer_B_PSRAM.h,
                          buffer_B_PSRAM.format,
                          buffer_B_PSRAM.stride);

        nema_set_clip(0, 0, FB_RESX, FB_RESY);
        nema_cl_add_cmd(NEMA_BURST_SIZE, burst_size_reg_val);                  

        nema_clear(0x7fffff00);

        nema_set_blend_fill(NEMA_BL_SIMPLE);
        nema_fill_rect(0, 0, buffer_B_PSRAM.w, buffer_B_PSRAM.h, 0x7f0000ff);
        nema_cl_submit(&sCL);
        nema_cl_wait(&sCL);

        uint32_t* ptr_fb = (uint32_t* )buffer_B_PSRAM.bo.base_virt;
        while((uint32_t)ptr_fb < (buffer_B_PSRAM.bo.base_phys + buffer_B_PSRAM.bo.size))
        {
                if(*ptr_fb != 0x7e80807f)
                {
                    bTestPass = false;
                    break;
                }

                ptr_fb ++;
        }

        TEST_ASSERT_TRUE(bTestPass);

        fb_burst_length ++;
    }

    nema_cl_destroy(&sCL);

    return bTestPass;
} // nemagfx_rw_psram_test