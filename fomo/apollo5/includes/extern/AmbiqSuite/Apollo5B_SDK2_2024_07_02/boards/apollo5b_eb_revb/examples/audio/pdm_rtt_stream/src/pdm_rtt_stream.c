//*****************************************************************************
//
//! @file pdm_rtt_stream.c
//!
//! @brief An example to show PDM audio streaming to PC over RTT data logger.
//!
//! Purpose:
//! This example enables the PDM interface to receive audio signals in 16 kHz
//! sample rate from an external digital microphone and uploads those data to
//! PC via RTT.
//!
//! Usage:
//! Build and download the program into the target device.
//! Reset the target and check the SWO print for PDM settings.
//! Exit SWO.exe and run the helper script 'rtt_logger.py' in the project folder
//!
//! -    python3 rtt_logger.py
//!
//! The audio captured is stored into the same folder as the rtt_logger.py,
//! with a file name of yyyymmdd-hh-mm-ss.pcm.
//!
//! To check the audio, load the *.pcm file into audio analysis tools and check.
//! E.g. in Audacity, https://www.audacityteam.org/:
//! -    File -> Import -> Raw data...
//! -    Default import format is:
//! -        Signed 24-bit PCM
//! -        Little-endian
//! -        2 Channels (Stereo)
//! -        Start offset: 0 bytes
//! -        Amount to import: 100%
//! -        Sample rate: 16000 Hz
//!
//! General pin connections:
//! GPIO_50 PDM0_CLK  to CLK_IN of digital microphone
//! GPIO_51 PDM0_DATA to DATA_OUT of digital microphone
//!
//! Pin connections on Apollo5 EB:
//! GPIO_50 PDM0_CLK  to BIT_CLOCK of APx525 PDM module
//! GPIO_51 PDM0_DATA to OUTPUT of APx525 PDM module
//!
//! NOTE:
//! In this example, RTT control block is mapped to a fixed address to facilitate
//! the searching process. If the address is changed, make sure to modify the
//! rtt_logger.py script to match the address, which can be get from SWO prints.
//!
//! Printing takes place over the SWO at 1M Baud.
//! RTT logger takes place over the SWD at 4M Baud.
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
#include "SEGGER_RTT.h"

//*****************************************************************************
//
// Example option parameters.
//
//*****************************************************************************
#if defined(AM_PART_APOLLO5A)
#define CLOCK_SOURCE            (HFRC)
#elif defined(AM_PART_APOLLO5B)
#define CLOCK_SOURCE            (PLL)
#endif
    #define HFRC                (0)
    #define PLL                 (1)
    #define HF2ADJ              (2)
#define PDM_MODULE              (0)
#define FIFO_THRESHOLD_CNT      (16)
#define NUM_OF_SAMPLES          (256)
#define SIZE_OF_SAMPLES         (NUM_OF_SAMPLES * sizeof(uint32_t))

//
// Exercise DMA start and DMA stop APIs.
//
#define EXERCISE_DMA_ONOFF_API  (0)

#if (EXERCISE_DMA_ONOFF_API == 1)
#define DMA_ONOFF_INTERVAL_MS   (3000)
#define XT_PERIOD               (32768)
#define WAKE_INTERVAL           (XT_PERIOD * DMA_ONOFF_INTERVAL_MS / 1000)
#endif

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
volatile bool g_bPDMDataReady = false;

//
// PDM buffer with or without DMA
// Aligned to 32 bytes to meet data cache requirements.
//
AM_SHARED_RW uint32_t g_ui32PDMDataBuffer[2*NUM_OF_SAMPLES] __attribute__((aligned(32)));

//
// RTT buffer
//
#define RTT_BUFFER_LENGTH       (256*3*400)
uint8_t g_rttRecorderBuffer[RTT_BUFFER_LENGTH];

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
    .ePCMChannels        = AM_HAL_PDM_CHANNEL_STEREO,
    .bPDMSampleDelay     = AM_HAL_PDM_CLKOUT_PHSDLY_NONE,
    .ui32GainChangeDelay = AM_HAL_PDM_CLKOUT_DELAY_NONE,
    .bSoftMute           = 0,
    .bLRSwap             = 0,
};

am_hal_pdm_transfer_t sTransfer =
{
    .ui32TargetAddr        = (uint32_t)(&g_ui32PDMDataBuffer[0]),
    .ui32TargetAddrReverse = (uint32_t)(&g_ui32PDMDataBuffer[NUM_OF_SAMPLES]),
    .ui32TotalCount        = SIZE_OF_SAMPLES,
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
    uint32_t ui32SampleFreq;

    //
    // Read the config structure to figure out what our internal clock is set
    // to.
    //
    switch (g_sPdmConfig.eClkDivider)
    {
        case AM_HAL_PDM_MCLKDIV_3: ui32MClkDiv = 3; break;
        case AM_HAL_PDM_MCLKDIV_2: ui32MClkDiv = 2; break;
        case AM_HAL_PDM_MCLKDIV_1: ui32MClkDiv = 1; break;
        default:
            ui32MClkDiv = 1;
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
        default:
            ui32DivClkQ = 1;
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

    ui32SampleFreq = (ui32PDMClk / ((ui32MClkDiv + 1) * (ui32DivClkQ + 1) * 2 * g_sPdmConfig.ui32DecimationRate));
    ui32PDMClkOut  = (ui32PDMClk / ((ui32MClkDiv + 1) * (ui32DivClkQ + 1)));

    am_util_stdio_printf("PDM Settings:\n");
    am_util_stdio_printf("PDM Clock Out(Hz):      %12d\n", ui32PDMClkOut);
    am_util_stdio_printf("Decimation Rate:        %12d\n", g_sPdmConfig.ui32DecimationRate);
    am_util_stdio_printf("Effective Sample Freq.: %12d\n", ui32SampleFreq);
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
    // DMA ping-pong buffer switch.
    //
    am_hal_pdm_interrupt_service(PDMHandle, ui32Status, &sTransfer);

    //
    // Check if DMA buffer is ready.
    //
    if (ui32Status & AM_HAL_PDM_INT_DCMP)
    {
        g_bPDMDataReady = true;
    }
}

#if (EXERCISE_DMA_ONOFF_API == 1)
//*****************************************************************************
//
// Init a stimer, ISR will trigger every {WAKE_INTERVAL}.
//
//*****************************************************************************
void stimer_init(void)
{
    //
    // Enable compare A interrupt in STIMER
    //
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_EnableIRQ(STIMER_CMPR0_IRQn);

    //
    // Configure the STIMER and run
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ | AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);
}

//*****************************************************************************
//
// Stimer ISR, start or stop DMA transfer in this ISR.
//
//*****************************************************************************
void am_stimer_cmpr0_isr(void)
{
    static bool dmaRunning = true;

    //
    // Check the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);

    //
    // Stop DMA if it's running, vice versa.
    //
    if (dmaRunning)
    {
        am_util_stdio_printf("Stop DMA transfer.\n");
        am_hal_pdm_dma_stop(PDMHandle);
        dmaRunning = false;
    }
    else
    {
        am_util_stdio_printf("Start DMA transfer.\n");
        am_hal_pdm_dma_start(PDMHandle, &sTransfer);
        dmaRunning = true;
    }
}
#endif

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
    // Initialize the printf interface for ITM output.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("==============================================\n");
    am_util_stdio_printf("PDM RTT streaming example.\n\n");

    //
    // Disable interrupt and clear the possible pending interrupt.
    //
    am_hal_interrupt_master_disable();
    NVIC_ClearPendingIRQ(pdm_interrupts[PDM_MODULE]);

    //
    // Initialize RTT.
    //
    SEGGER_RTT_Init();
    SEGGER_RTT_ConfigUpBuffer(1, "DataLogger", g_rttRecorderBuffer, sizeof(g_rttRecorderBuffer), SEGGER_RTT_MODE_NO_BLOCK_SKIP);

    //
    // Initialize PDM-to-PCM module.
    //
    pdm_init();

    //
    // Print compiler version, RTT control block address, and PDM configuration.
    //
    am_util_stdio_printf("App Compiler:    %s\n", COMPILER_VERSION);
    am_util_stdio_printf("RTT Control Block Address:  0x%08X\n", (uint32_t)&_SEGGER_RTT);
    pdm_config_print();
    am_util_stdio_printf("==============================================\n");
    am_util_stdio_printf("Run 'python3 rtt_logger.py' command to dump audio data.\n");

    //
    // Start DMA transfer.
    //
    am_hal_pdm_dma_start(PDMHandle, &sTransfer);

    //
    // Enable interrupts.
    //
    am_hal_interrupt_master_enable();

    #if (EXERCISE_DMA_ONOFF_API == 1)
    stimer_init();
    #endif

    //
    // Loop forever.
    //
    while (1)
    {
        //
        // Record the DMA buffer if it's ready.
        //
        if (g_bPDMDataReady)
        {
            g_bPDMDataReady = false;

            //
            // Re-arrange data.
            //
            uint32_t* ui32PDMDatabuffer = (uint32_t*)am_hal_pdm_dma_get_buffer(PDMHandle);

            uint8_t pByte[NUM_OF_SAMPLES * 3];
            for (uint32_t i = 0; i < NUM_OF_SAMPLES; i++ )
            {
                pByte[3 * i + 0] = (ui32PDMDatabuffer[i] & 0x0000FF);
                pByte[3 * i + 1] = (ui32PDMDatabuffer[i] & 0x00FF00) >>  8U;
                pByte[3 * i + 2] = (ui32PDMDatabuffer[i] & 0xFF0000) >> 16U;
            }

            //
            // Recording 24-bit data via RTT.
            //
            SEGGER_RTT_Write(1, pByte, NUM_OF_SAMPLES * 3);
        }
    }
}
