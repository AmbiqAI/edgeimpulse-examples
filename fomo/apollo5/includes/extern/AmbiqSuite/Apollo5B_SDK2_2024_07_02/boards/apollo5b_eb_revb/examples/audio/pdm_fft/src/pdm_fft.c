//*****************************************************************************
//
//! @file pdm_fft.c
//!
//! @brief An example to show basic PDM operation.
//!
//! Purpose:
//! This example enables the PDM interface to receive audio signals in 16 kHz sample
//! rate from an external digital microphone and prints the dominant frequency
//! calculated by FFT.
//!
//! General pin connections:
//! GPIO 50 to CLK_IN of digital microphone
//! GPIO 51 to DATA_OUT of digital microphone
//!
//! Pin connections on Apollo5 EB:
//! GPIO_50 PDM0_CLK  to BIT_CLOCK of APx525 PDM module
//! GPIO_51 PDM0_DATA to OUTPUT of APx525 PDM module
//!
//! Printing takes place over the SWO at 1M Baud.
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
#include <arm_math.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Example parameters.
//
//*****************************************************************************
#if defined(AM_PART_APOLLO5A)
#define CLOCK_SOURCE        (HFRC)
#elif defined(AM_PART_APOLLO5B)
#define CLOCK_SOURCE        (PLL)
#endif
    #define HFRC            (0)
    #define PLL             (1)
    #define HF2ADJ          (2)
#define PDM_MODULE          0
#define PDM_FFT_SIZE        4096
#define PDM_FFT_BYTES       (PDM_FFT_SIZE * 4)
#define PRINT_PDM_DATA      0
#define PRINT_FFT_DATA      0
#define FIFO_THRESHOLD_CNT  16
#define DMA_BYTES           PDM_FFT_BYTES

//*****************************************************************************
//
// Insert compiler version at compile time.
//
//*****************************************************************************
#define STRINGIZE_VAL(n)                    STRINGIZE_VAL2(n)
#define STRINGIZE_VAL2(n)                   #n

#ifdef __GNUC__
#define COMPILER_VERSION                    ("GCC " __VERSION__)
#elif defined(__ARMCC_VERSION)
#define COMPILER_VERSION                    ("ARMCC " STRINGIZE_VAL(__ARMCC_VERSION))
#elif defined(__KEIL__)
#define COMPILER_VERSION                    "KEIL_CARM " STRINGIZE_VAL(__CA__)
#elif defined(__IAR_SYSTEMS_ICC__)
#define COMPILER_VERSION                    __VERSION__
#else
#define COMPILER_VERSION                    "Compiler unknown"
#endif

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
volatile bool g_bPDMDataReady = false;
uint32_t g_ui32SampleFreq;
uint32_t g_ui32FifoOVFCount = 0;

//
// Buffers used for FFT.
//
float g_fPDMTimeDomain[PDM_FFT_SIZE * 2];
float g_fPDMFrequencyDomain[PDM_FFT_SIZE * 2];
float g_fPDMMagnitudes[PDM_FFT_SIZE * 2];

//
// Used as the ping-pong buffer of DMA.
// Aligned to 32 bytes to meet data cache requirements.
//
AM_SHARED_RW uint32_t g_ui32PDMDataBuffer[PDM_FFT_SIZE*2] __attribute__((aligned(32)));

//
// PDM handler.
//
void *PDMHandle;

// PLL handler.
void *pllHandle;

//
// PDM interrupt number.
//
static const IRQn_Type pdm_interrupts[] =
{
    PDM0_IRQn,
};

//*****************************************************************************
//
// PDM configuration information.
//
//*****************************************************************************
am_hal_pdm_config_t g_sPdmConfig =
{
    //
    // Example setting:
    //  1.536 MHz PDM CLK OUT:
    //      PDM_CLK_OUT = ePDMClkSpeed / (eClkDivider + 1) / (ePDMAClkOutDivder + 1)
    //  16 kHz 24bit Sampling:
    //      DecimationRate = 48
    //      SAMPLEING_FREQ = PDM_CLK_OUT / (ui32DecimationRate * 2)
    //
    #if (CLOCK_SOURCE == HFRC)
    .eClkDivider         = AM_HAL_PDM_MCLKDIV_1,
    .ePDMAClkOutDivder   = AM_HAL_PDM_PDMA_CLKO_DIV7,
    .ePDMClkSpeed        = AM_HAL_PDM_CLK_HFRC_24MHZ,
    #elif (CLOCK_SOURCE == PLL)
    .eClkDivider         = AM_HAL_PDM_MCLKDIV_1,
    .ePDMAClkOutDivder   = AM_HAL_PDM_PDMA_CLKO_DIV3,
    .ePDMClkSpeed        = AM_HAL_PDM_CLK_PLL,
    #elif (CLOCK_SOURCE == HF2ADJ)
    .eClkDivider         = AM_HAL_PDM_MCLKDIV_1,
    #if defined(AM_PART_APOLLO5A)
    .ePDMAClkOutDivder   = AM_HAL_PDM_PDMA_CLKO_DIV15,
    #elif defined(AM_PART_APOLLO5B)
    .ePDMAClkOutDivder   = AM_HAL_PDM_PDMA_CLKO_DIV7,
    #endif
    .ePDMClkSpeed        = AM_HAL_PDM_CLK_HFRC2_31MHZ,
    #endif
    .ui32DecimationRate  = 48,
    .eLeftGain           = AM_HAL_PDM_GAIN_0DB,
    .eRightGain          = AM_HAL_PDM_GAIN_0DB,
    .eStepSize           = AM_HAL_PDM_GAIN_STEP_0_13DB,
    .bHighPassEnable     = AM_HAL_PDM_HIGH_PASS_ENABLE,
    .ui32HighPassCutoff  = 0x3,
    .bDataPacking        = 1,
    .ePCMChannels        = AM_HAL_PDM_CHANNEL_LEFT,
    .bPDMSampleDelay     = AM_HAL_PDM_CLKOUT_PHSDLY_NONE,
    .ui32GainChangeDelay = AM_HAL_PDM_CLKOUT_DELAY_NONE,
    .bSoftMute           = 0,
    .bLRSwap             = 0,
};

am_hal_pdm_transfer_t sTransfer =
{
    //
    // DMA ping-pong buffer.
    //
    .ui32TargetAddr        = (uint32_t)(&g_ui32PDMDataBuffer[0]),
    .ui32TargetAddrReverse = (uint32_t)(&g_ui32PDMDataBuffer[PDM_FFT_SIZE]),
    .ui32TotalCount        = DMA_BYTES,
};

//*****************************************************************************
//
// PDM initialization.
//
//*****************************************************************************
void
pdm_init(void)
{
    #if (CLOCK_SOURCE == PLL)
    {
        // Start XTHS for use by PLL.
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
        am_util_delay_us(1500);

        // Configure and enable PLL to output 12.288 MHz clock to PDM.
        am_hal_syspll_config_t pllConfig = AM_HAL_SYSPLL_DEFAULT_CFG_TELCO_48K_16BIT_2CH;
        am_hal_syspll_initialize(0, &pllHandle);
        am_hal_syspll_configure(pllHandle, &pllConfig);
        am_hal_syspll_enable(pllHandle);

        // Wait for PLL Lock.
        am_hal_syspll_lock_wait(pllHandle);
    }
    #elif (CLOCK_SOURCE == HF2ADJ)
    {
        // Start the 32MHz crystal rapidly used to adjust HFRC2.
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
        am_util_delay_us(1500);

        // Enable HFRC2 adjustment.
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HF2ADJ_ENABLE, false);
        am_util_delay_us(500);
    }
    #endif

    //
    // Configure the necessary pins.
    //
    am_bsp_pdm_pins_enable(PDM_MODULE);

    //
    // Initialize, power-up, and configure the PDM.
    //
    am_hal_pdm_initialize(PDM_MODULE, &PDMHandle);
    am_hal_pdm_power_control(PDMHandle, AM_HAL_PDM_POWER_ON, false);
    am_hal_pdm_configure(PDMHandle, &g_sPdmConfig);

    //
    // Setup the FIFO threshold.
    //
    am_hal_pdm_fifo_threshold_setup(PDMHandle, FIFO_THRESHOLD_CNT);

    //
    // Configure and enable PDM interrupts.
    //
    am_hal_pdm_interrupt_enable(PDMHandle, (AM_HAL_PDM_INT_DCMP | AM_HAL_PDM_INT_DERR));
    NVIC_SetPriority(pdm_interrupts[PDM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(pdm_interrupts[PDM_MODULE]);

    //
    // Enable PDM, CLK_OUT pad starts outputting clock.
    //
    am_hal_pdm_enable(PDMHandle);
}

//*****************************************************************************
//
// Print PDM configuration data.
//
//*****************************************************************************
void
pdm_config_print(void)
{
    uint32_t ui32PDMClk, ui32PDMClkOut;
    uint32_t ui32MClkDiv;
    uint32_t ui32DivClkQ;
    float fFrequencyResolution;

    //
    // Read the config structure to figure out what our internal clock is set
    // to.
    //
    switch (g_sPdmConfig.eClkDivider)
    {
        case AM_HAL_PDM_MCLKDIV_3: ui32MClkDiv = 3; break;
        case AM_HAL_PDM_MCLKDIV_2: ui32MClkDiv = 2; break;
        case AM_HAL_PDM_MCLKDIV_1: ui32MClkDiv = 1; break;
        default:                   ui32MClkDiv = 1; break;
    }

    switch (g_sPdmConfig.ePDMAClkOutDivder)
    {
        case AM_HAL_PDM_PDMA_CLKO_DIV1:  ui32DivClkQ =  1; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV2:  ui32DivClkQ =  2; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV3:  ui32DivClkQ =  3; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV4:  ui32DivClkQ =  4; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV5:  ui32DivClkQ =  5; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV6:  ui32DivClkQ =  6; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV7:  ui32DivClkQ =  7; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV8:  ui32DivClkQ =  8; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV9:  ui32DivClkQ =  9; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV10: ui32DivClkQ = 10; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV11: ui32DivClkQ = 11; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV12: ui32DivClkQ = 12; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV13: ui32DivClkQ = 13; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV14: ui32DivClkQ = 14; break;
        case AM_HAL_PDM_PDMA_CLKO_DIV15: ui32DivClkQ = 15; break;
        default:                         ui32DivClkQ =  1; break;
    }

    switch (g_sPdmConfig.ePDMClkSpeed)
    {
        case AM_HAL_PDM_CLK_HFXTAL:      ui32PDMClk = 32000000; break;
        case AM_HAL_PDM_CLK_HFRC_24MHZ:  ui32PDMClk = 24000000; break;
        // Since HF2ADJ was enabled, HFRC2_31MHZ frequency changed.
        #if defined(AM_PART_APOLLO5A)
        case AM_HAL_PDM_CLK_HFRC2_31MHZ: ui32PDMClk = 49152000; break;
        #elif defined(AM_PART_APOLLO5B)
        case AM_HAL_PDM_CLK_HFRC2_31MHZ: ui32PDMClk = 24576000; break;
        case AM_HAL_PDM_CLK_PLL:         ui32PDMClk = 12288000; break;
        #endif
        default:                         ui32PDMClk = 24000000; break;
    }

    //
    // Record the effective sample frequency. We'll need it later to print the
    // loudest frequency from the sample.
    //
    g_ui32SampleFreq = ui32PDMClk / ((ui32MClkDiv + 1) * (ui32DivClkQ + 1) * 2 * g_sPdmConfig.ui32DecimationRate);
    ui32PDMClkOut    = ui32PDMClk / ((ui32MClkDiv + 1) * (ui32DivClkQ + 1));

    fFrequencyResolution = (float) g_ui32SampleFreq / (float) PDM_FFT_SIZE;

    am_util_stdio_printf("PDM Settings:\n");
    am_util_stdio_printf("PDM Clock Out(Hz):      %12d\n", ui32PDMClkOut);
    am_util_stdio_printf("Decimation Rate:        %12d\n", g_sPdmConfig.ui32DecimationRate);
    am_util_stdio_printf("Effective Sample Freq.: %12d\n", g_ui32SampleFreq);
    am_util_stdio_printf("FFT Length:             %12d\n\n", PDM_FFT_SIZE);
    am_util_stdio_printf("FFT Resolution: %15.3f Hz\n", fFrequencyResolution);
}


//*****************************************************************************
//
// PDM interrupt handler.
//
//*****************************************************************************
void
am_pdm0_isr(void)
{
    uint32_t ui32Status;

    //
    // Read and clear the interrupt status.
    //
    am_hal_pdm_interrupt_status_get(PDMHandle, &ui32Status, true);
    am_hal_pdm_interrupt_clear(PDMHandle, ui32Status);

    //
    // Swich ping pong buffer.
    //
    am_hal_pdm_interrupt_service(PDMHandle, ui32Status, &sTransfer);

    if (ui32Status & AM_HAL_PDM_INT_DCMP)
    {
        g_bPDMDataReady = true;

    }

     if (ui32Status & AM_HAL_PDM_INT_OVF)
     {
        uint32_t count = am_hal_pdm_fifo_count_get(PDMHandle);
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
        volatile uint32_t ui32FifoDummy;
        for ( uint8_t i = 0; i < count; i++ )
        {
            ui32FifoDummy = PDMn(PDM_MODULE)->FIFOREAD;
        }
#pragma GCC diagnostic pop
#endif
        am_hal_pdm_fifo_flush(PDMHandle);

        g_ui32FifoOVFCount++;
     }
}


//*****************************************************************************
//
// Analyze and print frequency data.
//
//*****************************************************************************
void
pcm_fft_print(int32_t *pi32PDMData)
{
    float fMaxValue;
    uint32_t ui32MaxIndex;
    uint32_t ui32LoudestFrequency;

    //
    // Convert the PDM samples to floats, and arrange them in the format
    // required by the FFT function.
    //
    for (uint32_t i = 0; i < PDM_FFT_SIZE; i++)
    {
        if (PRINT_PDM_DATA)
        {
            am_util_stdio_printf("%d\n", pi32PDMData[i]);
        }

        g_fPDMTimeDomain[2 * i] = (pi32PDMData[i] << 8) / 65536.0;
        g_fPDMTimeDomain[2 * i + 1] = 0.0;
    }

    if (PRINT_PDM_DATA)
    {
        am_util_stdio_printf("END\n");
    }

    //
    // Perform the FFT.
    //
    arm_cfft_radix4_instance_f32 S;
    arm_cfft_radix4_init_f32(&S, PDM_FFT_SIZE, 0, 1);
    arm_cfft_radix4_f32(&S, g_fPDMTimeDomain);
    arm_cmplx_mag_f32(g_fPDMTimeDomain, g_fPDMMagnitudes, PDM_FFT_SIZE);

    if (PRINT_FFT_DATA)
    {
        for (uint32_t i = 0; i < PDM_FFT_SIZE / 2; i++)
        {
            am_util_stdio_printf("%f\n", g_fPDMMagnitudes[i]);
        }

        am_util_stdio_printf("END\n");
    }

    //
    // Find the frequency bin with the largest magnitude.
    //
    arm_max_f32(g_fPDMMagnitudes, PDM_FFT_SIZE / 2, &fMaxValue, &ui32MaxIndex);

    ui32LoudestFrequency = (g_ui32SampleFreq * ui32MaxIndex) / PDM_FFT_SIZE;

    if (PRINT_FFT_DATA)
    {
        am_util_stdio_printf("Loudest frequency bin: %d\n", ui32MaxIndex);
    }
    am_util_stdio_printf("Loudest frequency: %d\n", ui32LoudestFrequency);
}


//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("==============================================\n");
    am_util_stdio_printf("PDM FFT example.\n\n");

    //
    // Initialize PDM-to-PCM module
    //
    pdm_init();

    //
    // Print compiler version and PDM configuration.
    //
    am_util_stdio_printf("App Compiler:    %s\n", COMPILER_VERSION);
    pdm_config_print();

    //
    // Start DMA transfer.
    //
    am_hal_pdm_dma_start(PDMHandle, &sTransfer);

    //
    // Enable interrupts.
    //
    am_hal_interrupt_master_enable();

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
        if (g_bPDMDataReady)
        {
            g_bPDMDataReady = false;
            int32_t *pPDMData = (int32_t *) am_hal_pdm_dma_get_buffer(PDMHandle);
            pcm_fft_print(pPDMData);
        }

        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}
