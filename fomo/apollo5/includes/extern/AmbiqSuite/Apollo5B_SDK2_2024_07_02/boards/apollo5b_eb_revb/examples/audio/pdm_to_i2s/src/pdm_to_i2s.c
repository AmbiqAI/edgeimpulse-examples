//*****************************************************************************
//
//! @file pdm_to_i2s.c
//!
//! @brief An example to demostrate the I2S interface forwards data coverted by
//! PDM from microphone.
//!
//! Purpose:
//! This example enables the PDM interface to receive audio signals in 16 kHz
//! sample rate (or 1.536 MHz PDM clock) from an external digital microphone and
//! forwards converted PCM data by the I2S interface to an external I2S device.
//!
//! General pin connections:
//! GPIO_50 PDM0_CLK      to CLK_IN of digital microphone
//! GPIO_51 PDM0_DATA     to DATA_OUT of digital microphone
//! I2S_CLK_GPIO_PIN      to SCLK or BIT_CLK of external I2S device
//! I2S_WS_GPIO_PIN       to WS or FRAM_CLK of external I2S device
//! I2S_DATA_OUT_GPIO_PIN to DATA_IN of external I2S device
//!
//! Pin connections on Apollo5 EB:
//! GPIO_50 PDM0_CLK      to BIT_CLOCK of APx525 PDM module
//! GPIO_51 PDM0_DATA     to OUTPUT of APx525 PDM module
//! I2S_CLK_GPIO_PIN      to BIT_CLK of APx525 digital serial receiver
//! I2S_WS_GPIO_PIN       to FRAME of APx525 digital serial receiver
//! I2S_DATA_OUT_GPIO_PIN to DATA1 of APx525 digital serial receiver
//!
//! Printing takes place over the ITM at 1M Baud.
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
// Example parameters.
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
#define PDM_MODULE              0
#define I2S_MODULE              0
#define FIFO_THRESHOLD_CNT      16
#define NUM_OF_PCM_SAMPLES      320
#define DATA_VERIFY             0
#define PDM_ISR_TEST_PAD        2
#define I2S_ISR_TEST_PAD        3

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
uint32_t g_ui32FifoOVFCount = 0;
volatile bool g_bPDMDataReady = false;

//
// PDM and I2S handlers.
//
void *PDMHandle;
void *I2SHandle;

// PLL handler.
void *pllHandle;

//
// PDM and I2S interrupt number.
//
static const IRQn_Type pdm_interrupts[] =
{
    PDM0_IRQn,
};
static const IRQn_Type i2s_interrupts[] =
{
    I2S0_IRQn,
    I2S1_IRQn
};

//
// Used as the ping-pong buffer of PDM DMA.
// Aligned to 32 bytes to meet data cache requirements.
//
AM_SHARED_RW uint32_t g_ui32PingPongBuffer[2*NUM_OF_PCM_SAMPLES] __attribute__((aligned(32)));

//*****************************************************************************
//
// PDM configuration information.
//
//  1.536 MHz PDM CLK OUT:
//      PDM_CLK_OUT = ePDMClkSpeed / (eClkDivider + 1) / (ePDMAClkOutDivder + 1)
//  16 kHz 24bit Sampling:
//      DecimationRate = 48
//      SAMPLEING_FREQ = PDM_CLK_OUT / (ui32DecimationRate * 2)
//
//*****************************************************************************
am_hal_pdm_config_t g_sPdmConfig =
{
    #if (CLOCK_SOURCE == HFRC)
    .eClkDivider         = AM_HAL_PDM_MCLKDIV_1,
    .ePDMAClkOutDivder   = AM_HAL_PDM_PDMA_CLKO_DIV7,
    .ePDMClkSpeed        = AM_HAL_PDM_CLK_HFRC_24MHZ,
    #elif (CLOCK_SOURCE == PLL)
    .eClkDivider         = AM_HAL_PDM_MCLKDIV_1,
    .ePDMAClkOutDivder   = AM_HAL_PDM_PDMA_CLKO_DIV1,
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

//
// Ping pong buffer settings.
//
am_hal_pdm_transfer_t g_sTransferPDM =
{
    .ui32TargetAddr        = (uint32_t)(&g_ui32PingPongBuffer[0]),
    .ui32TargetAddrReverse = (uint32_t)(&g_ui32PingPongBuffer[NUM_OF_PCM_SAMPLES]),
    .ui32TotalCount        = NUM_OF_PCM_SAMPLES * sizeof(uint32_t),
};

//*****************************************************************************
//
// I2S configurations:
//  - 2 channels
//  - 16 kHz sample rate
//  - Standard I2S format
//  - 32 bits word width
//  - 24 bits bit-depth
//
// SCLK = 16000 * 2 * 32 = 1.024 MHz
//
//*****************************************************************************
static am_hal_i2s_io_signal_t g_sI2SIOConfig =
{
    .sFsyncPulseCfg = {
        .eFsyncPulseType = AM_HAL_I2S_FSYNC_PULSE_ONE_SUBFRAME,
    },
    .eFyncCpol = AM_HAL_I2S_IO_FSYNC_CPOL_LOW,
    .eTxCpol   = AM_HAL_I2S_IO_TX_CPOL_FALLING,
    .eRxCpol   = AM_HAL_I2S_IO_RX_CPOL_RISING,
};

static am_hal_i2s_data_format_t g_sI2SDataConfig =
{
    .ePhase                   = AM_HAL_I2S_DATA_PHASE_SINGLE,
    .eChannelLenPhase1        = AM_HAL_I2S_FRAME_WDLEN_32BITS,
    .eChannelLenPhase2        = AM_HAL_I2S_FRAME_WDLEN_32BITS,
    .ui32ChannelNumbersPhase1 = 2,
    .ui32ChannelNumbersPhase2 = 0,
    .eDataDelay               = 0x1,
    .eSampleLenPhase1         = AM_HAL_I2S_SAMPLE_LENGTH_24BITS,
    .eSampleLenPhase2         = AM_HAL_I2S_SAMPLE_LENGTH_24BITS,
    .eDataJust                = AM_HAL_I2S_DATA_JUSTIFIED_LEFT,
};

static am_hal_i2s_config_t g_sI2SConfig =
{
    .eMode  = AM_HAL_I2S_IO_MODE_MASTER,
    .eXfer  = AM_HAL_I2S_XFER_TX,
    #if (USE_I2S_MASTER == I2S_MODULE_0)
    .eMode  = AM_HAL_I2S_IO_MODE_MASTER,
    #else
    .eMode  = AM_HAL_I2S_IO_MODE_SLAVE,
    #endif
    .eXfer  = AM_HAL_I2S_XFER_RXTX,
    #if (CLOCK_SOURCE == HFRC)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC_3MHz,
    .eDiv3  = 1,
    #elif (CLOCK_SOURCE == PLL)
    .eClock = eAM_HAL_I2S_CLKSEL_PLL_FOUT3,
    .eDiv3  = 0,
    #elif (CLOCK_SOURCE == HF2ADJ)
    #if defined(AM_PART_APOLLO5A)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_2MHz,
    #elif defined(AM_PART_APOLLO5B)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_4MHz,
    #endif
    .eDiv3  = 1,
    #endif
    .eASRC  = 0,
    .eData  = &g_sI2SDataConfig,
    .eIO    = &g_sI2SIOConfig,
};

//
// Ping pong buffer settings.
//
static am_hal_i2s_transfer_t g_sTransferI2S =
{
    .ui32TxTotalCount        = NUM_OF_PCM_SAMPLES,
    .ui32TxTargetAddr        = 0x0,
    .ui32TxTargetAddrReverse = 0x0,
};

//*****************************************************************************
//
// PDM initialization.
//
//*****************************************************************************
void
pdm_init(void)
{
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
    // Configure and enable PDM interrupts (set up to trigger on DMA completion).
    //
    am_hal_pdm_interrupt_enable(PDMHandle, (AM_HAL_PDM_INT_DERR  |\
                                            AM_HAL_PDM_INT_DCMP  |\
                                            AM_HAL_PDM_INT_UNDFL |\
                                            AM_HAL_PDM_INT_OVF));

    NVIC_SetPriority(pdm_interrupts[PDM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(pdm_interrupts[PDM_MODULE]);
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

#if DATA_VERIFY
    am_hal_gpio_output_set(PDM_ISR_TEST_PAD);
#endif

    //
    // Read the interrupt status.
    //
    am_hal_pdm_interrupt_status_get(PDMHandle, &ui32Status, true);
    am_hal_pdm_interrupt_clear(PDMHandle, ui32Status);

#if DATA_VERIFY
    am_hal_pdm_state_t *pState = (am_hal_pdm_state_t *) PDMHandle;
    static uint32_t ui32Switch = 0;
    if ( ui32Switch )
    {
        ui32Switch = 0;
        for (int i = 0; i < DMA_COUNT; i++)
        {
            ((uint32_t*)pState->ui32BufferPtr)[i] = (i & 0xFF) | 0xAB0000;
        }
    }
    else
    {
        ui32Switch = 1;
        for (int i = 0; i < DMA_COUNT; i++)
        {
            ((uint32_t*)pState->ui32BufferPtr)[i] = (i & 0xFF) | 0xCD0000;
        }
    }

    //am_util_stdio_printf("pdm isr addr = %x.\n", pState->ui32BufferPtr);
#endif

    //
    // Swich ping pong buffer.
    //
    am_hal_pdm_interrupt_service(PDMHandle, ui32Status, &g_sTransferPDM);

    if (ui32Status & AM_HAL_PDM_INT_DCMP)
    {
        g_bPDMDataReady = true;
    }

     if (ui32Status & AM_HAL_PDM_INT_OVF)
     {
         am_hal_pdm_fifo_count_get(PDMHandle);
         am_hal_pdm_fifo_flush(PDMHandle);
         g_ui32FifoOVFCount++;
     }

#if DATA_VERIFY
    am_hal_gpio_output_clear(PDM_ISR_TEST_PAD);
#endif
}

//*****************************************************************************
//
// I2S initialization.
//
//*****************************************************************************
void
i2s_init(void)
{
    //
    // Configure the necessary pins.
    //
    am_bsp_i2s_pins_enable(I2S_MODULE, false);

    //
    // Configure the I2S.
    //
    am_hal_i2s_initialize(I2S_MODULE, &I2SHandle);
    am_hal_i2s_power_control(I2SHandle, AM_HAL_I2S_POWER_ON, false);

    if (AM_HAL_STATUS_SUCCESS != am_hal_i2s_configure(I2SHandle, &g_sI2SConfig))
    {
        am_util_stdio_printf("ERROR: Invalid I2S configuration.\nNote: For Apollo5 Rev.B0, I2S can only use PLL as the clock source.\n");
    }

    am_hal_i2s_enable(I2SHandle);
}

//*****************************************************************************
//
// I2S interrupt handler.
//
//*****************************************************************************
#define i2s_isr                 am_dspi2s_isrx(I2S_MODULE)
#define am_dspi2s_isrx(n)       am_dspi2s_isr(n)
#define am_dspi2s_isr(n)        am_dspi2s ## n ## _isr

void
i2s_isr()
{
    uint32_t ui32Status;

#if DATA_VERIFY
    am_hal_gpio_output_set(I2S_ISR_TEST_PAD);
#endif

    am_hal_i2s_interrupt_status_get(I2SHandle, &ui32Status, true);
    am_hal_i2s_interrupt_clear(I2SHandle, ui32Status);

    //
    // Swich ping pong buffer.
    //
    am_hal_i2s_interrupt_service(I2SHandle, ui32Status, &g_sI2SConfig);

#if DATA_VERIFY
    am_hal_gpio_output_clear(I2S_ISR_TEST_PAD);
#endif
}

//*****************************************************************************
//
// PDM deinitialization.
//
//*****************************************************************************
void pdm_deinit(void *pHandle)
{
    am_hal_pdm_interrupt_clear(pHandle, (AM_HAL_PDM_INT_DERR  |\
                                         AM_HAL_PDM_INT_DCMP  |\
                                         AM_HAL_PDM_INT_UNDFL |\
                                         AM_HAL_PDM_INT_OVF));

    am_hal_pdm_interrupt_disable(pHandle, (AM_HAL_PDM_INT_DERR  |\
                                           AM_HAL_PDM_INT_DCMP  |\
                                           AM_HAL_PDM_INT_UNDFL |\
                                           AM_HAL_PDM_INT_OVF));

    NVIC_DisableIRQ(pdm_interrupts[PDM_MODULE]);

    am_bsp_pdm_pins_disable(PDM_MODULE);

    am_hal_pdm_disable(pHandle);
    am_hal_pdm_power_control(pHandle, AM_HAL_PDM_POWER_OFF, false);
    am_hal_pdm_deinitialize(pHandle);
}

//*****************************************************************************
//
// I2S deinitialization.
//
//*****************************************************************************
void i2s_deinit(void *pHandle)
{
    am_hal_i2s_dma_transfer_complete(pHandle);

    am_hal_i2s_interrupt_clear(pHandle, (AM_HAL_I2S_INT_RXDMACPL | AM_HAL_I2S_INT_TXDMACPL));
    am_hal_i2s_interrupt_disable(pHandle, (AM_HAL_I2S_INT_RXDMACPL | AM_HAL_I2S_INT_TXDMACPL));

    NVIC_DisableIRQ(i2s_interrupts[I2S_MODULE]);

    am_bsp_i2s_pins_disable(I2S_MODULE, false);

    am_hal_i2s_disable(pHandle);
    am_hal_i2s_power_control(pHandle, AM_HAL_I2S_POWER_OFF, false);
    am_hal_i2s_deinitialize(pHandle);
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

    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("==============================================\n");
    am_util_stdio_printf("PDM_I2S streaming example.\n\n");

#if DATA_VERIFY
    am_hal_gpio_pinconfig(PDM_ISR_TEST_PAD, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(I2S_ISR_TEST_PAD, am_hal_gpio_pincfg_output);
    am_hal_gpio_output_clear(PDM_ISR_TEST_PAD);
    am_hal_gpio_output_clear(I2S_ISR_TEST_PAD);
#endif

    #if (CLOCK_SOURCE == PLL)
    {
        // Start XTHS for use by PLL.
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
        am_util_delay_us(1500);

        // Configure and enable PLL to output a 1.024 MHz clock for I2S and a 6.144 MHz clock for PDM.
        am_hal_syspll_config_t pllConfig =
        {
            .eFref         = AM_HAL_SYSPLL_FREFSEL_XTAL32MHz,
            .eVCOSel       = AM_HAL_SYSPLL_VCOSEL_VCOLO,
            .eFractionMode = AM_HAL_SYSPLL_FMODE_FRACTION,
            .ui8RefDiv     = 3,
            .ui8PostDiv1   = 6,
            .ui8PostDiv2   = 6,
            .ui16FBDivInt  = 20,
            .ui32FBDivFrac = 12348031,
        };
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
    // Initialize PDM-to-PCM module
    //
    pdm_init();
    am_hal_pdm_enable(PDMHandle);

    //
    // Initialize I2S.
    //
    i2s_init();
    NVIC_SetPriority(i2s_interrupts[I2S_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(i2s_interrupts[I2S_MODULE]);
    am_hal_interrupt_master_enable();

    //
    // Start PDM streaming.
    //
    am_hal_pdm_dma_start(PDMHandle, &g_sTransferPDM);

    //Avoid interrupt coming simultaneously.
    am_util_delay_ms(5);

    // use the reverser buffer of PDM
    g_sTransferI2S.ui32TxTargetAddr = am_hal_pdm_dma_get_buffer(PDMHandle);
    g_sTransferI2S.ui32TxTargetAddrReverse = (g_sTransferI2S.ui32TxTargetAddr == g_sTransferPDM.ui32TargetAddr)? g_sTransferPDM.ui32TargetAddrReverse: g_sTransferPDM.ui32TargetAddr;
    //Start I2S data transaction.
    am_hal_i2s_dma_configure(I2SHandle, &g_sI2SConfig, &g_sTransferI2S);
    am_hal_i2s_dma_transfer_start(I2SHandle, &g_sI2SConfig);

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
