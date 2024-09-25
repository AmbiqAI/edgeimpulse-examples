//*****************************************************************************
//
//! @file audadc_rtt_stream.c
//!
//! @brief This example uses AUDADC to capture and send audio data to PC via SEGGER RTT.
//!
//! Purpose: This example uses AUDADC INTTRIGTIMER to capture audio samples at 16 kHz
//! sample rate from external analog microphones and uploads those data to PC via RTT.
//!
//! Run 'python3 rtt_logger.py' to capture PCM raw data via SEGGER RTT, you can check
//! the PCM file in Audacity https://www.audacityteam.org/:
//! -    File -> Import -> Raw data...
//! -    Default import format is:
//! -        Signed 16-bit PCM
//! -        Little-endian
//! -        2 Channels
//! -        Start offset: 0 bytes
//! -        Amount to import: 100%
//! -        Sample rate: 16000 Hz
//! These two channels are data of mic1 and data of mic2.
//! You can convert this PCM file to wav file by running 'python3 pcm_to_wav.py', but
//! only the first channel will be kept.
//!
//! NOTE:
//! In this example, RTT control block is mapped to a fixed address to facilitate
//! the searching process. If the address is changed, make sure to modify the
//! rtt_logger.py script to match the address, which can be get from SWO prints.
//!
//! Hardware requirements on Apollo5 EB:
//! - The 8080 peripheral card is needed.
//! - Two analog microphones are needed.
//! - IOX is selected mode 1.
//!
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

#define XTHS_32M                0
#define HFRC_48M                1
#define HFRC2ADJ                2
#define EXTCLK_12P288M          3
#define PLL                     4

//
// If use EXTCLK_12P288M as the clock source, please connect
// a 12.288MHz clock (the MCLK of J10 on apollo5 EB) to GPIO 15.
//
#if defined(AM_PART_APOLLO5A)
#define CLK_SRC                 HFRC_48M
#elif defined(AM_PART_APOLLO5B)
#define CLK_SRC                 PLL
#endif

#define RTT_LOGGING             1
#define ENABLE_DC_OFFSET_CAL    0

//*****************************************************************************
//
// AUDADC Sample buffer.
//
//*****************************************************************************
#define AUDADC_SAMPLE_BUF_SIZE      (2400)    // Should be padded to 12 samples follow current DMA/FIFO mechanism:
                                              // DMA trigger on FIFO 75% full
#define AUDADC_DATA_BUFFER_SIZE     (2*AUDADC_SAMPLE_BUF_SIZE)

AM_SHARED_RW uint32_t g_ui32AUDADCSampleBuffer[2*AUDADC_SAMPLE_BUF_SIZE];

AM_SHARED_RW int16_t g_in16AudioDataBuffer[AUDADC_DATA_BUFFER_SIZE];

am_hal_audadc_sample_t sLGSampleBuffer[AUDADC_SAMPLE_BUF_SIZE];
am_hal_audadc_sample_t sHGSampleBuffer[AUDADC_SAMPLE_BUF_SIZE];
//
// RTT streaming buffer
//
#if RTT_LOGGING
#define TIMEOUT                     400000      // RTT streaming timeout loop count
#define RTT_BUFFER_LENGTH           (256*1024)
#define AUDIO_SAMPLE_TO_RTT         (256*1024)
uint8_t g_rttRecorderBuffer[RTT_BUFFER_LENGTH];
uint32_t g_ui32SampleToRTT = 0;
#endif

//
// AUDADC Device Handle.
//
static void *g_AUDADCHandle;

// PLL handler.
void *pllHandle;

//
// AUDADC DMA complete flag.
//
volatile bool g_bAUDADCDMAComplete = false;

//
// AUDADC DMA error flag.
//
volatile bool g_bAUDADCDMAError;

//*****************************************************************************
//
// AUDADC gain configuration information.
//
//*****************************************************************************
#if defined(AM_PART_APOLLO5A)
#define CH_A0_EN  1
#define CH_A1_EN  1
#define CH_B0_EN  1
#define CH_B1_EN  1
#elif defined(AM_PART_APOLLO5B)
#define CH_A0_EN  1
#define CH_A1_EN  1
#endif

#define PREAMP_FULL_GAIN    12    // Enable op amps for full gain range
#if defined(AM_PART_APOLLO5A)
#define CH_A0_GAIN_DB       12
#define CH_A1_GAIN_DB       12
#define CH_B0_GAIN_DB       12
#define CH_B1_GAIN_DB       12
#elif defined(AM_PART_APOLLO5B)
#define CH_A0_GAIN_DB       12
#define CH_A1_GAIN_DB       12
#endif

am_hal_audadc_gain_config_t g_sAudadcGainConfig =
{
    #if defined(AM_PART_APOLLO5A)
    .ui32LGA        = 0,        // 0 code
    .ui32HGADELTA   = 0,        // delta from the LGA field
    .ui32LGB        = 0,        // 0 code
    .ui32HGBDELTA   = 0,        // delta from the LGB field
    #elif defined(AM_PART_APOLLO5B)
    .ui32LGA        = 0,
    .ui32HGADELTA   = 0,
    #endif
    .eUpdateMode    = AM_HAL_AUDADC_GAIN_UPDATE_IMME,
};

//
// Configure the AUDADC to use DMA for the sample transfer.
//
am_hal_audadc_dma_config_t g_sAUDADCDMAConfig =
{
    .bDynamicPriority  = true,
    .ePriority         = AM_HAL_AUDADC_PRIOR_SERVICE_IMMED,
    .bDMAEnable        = true,
    .ui32SampleCount   = AUDADC_SAMPLE_BUF_SIZE,
    .ui32TargetAddress = 0x0,
    .ui32TargetAddressReverse = 0x0,
};

//*****************************************************************************
//
// Configure the AUDADC SLOT.
//
//*****************************************************************************
void
audadc_slot_config(void)
{
    am_hal_audadc_slot_config_t      AUDADCSlotConfig;

    //
    // Set up an AUDADC slot
    //
    AUDADCSlotConfig.eMeasToAvg      = AM_HAL_AUDADC_SLOT_AVG_1;
    AUDADCSlotConfig.ePrecisionMode  = AM_HAL_AUDADC_SLOT_12BIT;
#if (CLK_SRC == XTHS)
    AUDADCSlotConfig.ui32TrkCyc      = 24;
#else
    AUDADCSlotConfig.ui32TrkCyc      = 34;
#endif
    AUDADCSlotConfig.eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE0;
    AUDADCSlotConfig.bWindowCompare  = false;
    AUDADCSlotConfig.bEnabled        = true;

#if CH_A0_EN
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_slot(g_AUDADCHandle, 0, &AUDADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC Slot 0 failed.\n");
    }
#endif
#if CH_A1_EN
    AUDADCSlotConfig.eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE1;
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_slot(g_AUDADCHandle, 1, &AUDADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC Slot 1 failed.\n");
    }
#endif
#if CH_B0_EN
    AUDADCSlotConfig.eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE2;
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_slot(g_AUDADCHandle, 2, &AUDADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC Slot 2 failed.\n");
    }
#endif
#if CH_B1_EN
    AUDADCSlotConfig.eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE3;
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_slot(g_AUDADCHandle, 3, &AUDADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC Slot 3 failed.\n");
    }
#endif
}

//*****************************************************************************
//
// Configure the AUDADC.
//
//*****************************************************************************
void
audadc_config(void)
{
    //
    // Set up the AUDADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    //
    am_hal_audadc_config_t           AUDADCConfig =
    {
#if (CLK_SRC == XTHS_32M)
        .eClock         = AM_HAL_AUDADC_CLKSEL_XTHS_32MHz,
#elif (CLK_SRC == HFRC_48M)
        .eClock         = AM_HAL_AUDADC_CLKSEL_HFRC_48MHz,
#elif (CLK_SRC == HFRC2ADJ)
        .eClock         = AM_HAL_AUDADC_CLKSEL_HFRC2_APPROX_31MHz,
#elif (CLK_SRC == EXTCLK_12P288M)
        .eClock         = AM_HAL_AUDADC_CLKSEL_EXTREF_CLK,
#elif (CLK_SRC == PLL)
        .eClock         = AM_HAL_AUDADC_CLKSEL_PLL,
#else
        #error "Invalid clock source."
#endif
        .ePolarity      = AM_HAL_AUDADC_TRIGPOL_RISING,
        .eTrigger       = AM_HAL_AUDADC_TRIGSEL_SOFTWARE,
        .eClockMode     = AM_HAL_AUDADC_CLKMODE_LOW_LATENCY,
        .ePowerMode     = AM_HAL_AUDADC_LPMODE1,
        .eRepeat        = AM_HAL_AUDADC_REPEATING_SCAN,
        .eRepeatTrigger = AM_HAL_AUDADC_RPTTRIGSEL_INT,
        .eSampMode      = AM_HAL_AUDADC_SAMPMODE_MED,       //AM_HAL_AUDADC_SAMPMODE_LP,
    };

    //
    // Set up internal repeat trigger timer
    //
    // Adjust sample rate to around 16kHz.
    // sample rate = eClock / eClkDiv / ui32IrttCountMax
    //
    am_hal_audadc_irtt_config_t AUDADCIrttConfig =
    {
        .bIrttEnable      = true,
#if (CLK_SRC == XTHS_32M)
        .eClkDiv          = AM_HAL_AUDADC_RPTT_CLK_DIV16,
        .ui32IrttCountMax = 125,
#elif (CLK_SRC == HFRC_48M)
        .eClkDiv          = AM_HAL_AUDADC_RPTT_CLK_DIV8,
        .ui32IrttCountMax = 375,
#elif (CLK_SRC == HFRC2ADJ)
        .eClkDiv          = AM_HAL_AUDADC_RPTT_CLK_DIV32,
        #if defined(AM_PART_APOLLO5A)
        .ui32IrttCountMax = 96,
        #elif defined(AM_PART_APOLLO5B)
        .ui32IrttCountMax = 48,
        #endif
#elif (CLK_SRC == EXTCLK_12P288M)
        .eClkDiv          = AM_HAL_AUDADC_RPTT_CLK_DIV32,
        .ui32IrttCountMax = 24,
#elif (CLK_SRC == PLL)
        .eClkDiv          = AM_HAL_AUDADC_RPTT_CLK_DIV32,
        .ui32IrttCountMax = 48,
#endif
    };

    #if (CLK_SRC == XTHS_32M)
    {
        // Start the 32MHz crystal.
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
        am_util_delay_us(1500);
    }
    #elif (CLK_SRC == HFRC2ADJ)
    {
        // Start the 32MHz crystal rapidly used to adjust HFRC2.
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
        am_util_delay_us(1500);

        // Enable HFRC2 adjustment, adjust HFRC2 clock to 49.152MHz for 5a part, 24.576MHz for 5b part.
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HF2ADJ_ENABLE, false);
        am_util_delay_us(500);
    }
    #elif (CLK_SRC == EXTCLK_12P288M)
    {
        // Connect the external clock (12.288MHz) to GPIO_15.
        am_hal_gpio_pincfg_t extClkPincfg =
        {
            .GP.cfg_b.uFuncSel       = AM_HAL_PIN_15_REFCLK_EXT,
            .GP.cfg_b.eGPInput       = AM_HAL_GPIO_PIN_INPUT_NONE,
            .GP.cfg_b.eGPRdZero      = AM_HAL_GPIO_PIN_RDZERO_READPIN,
            .GP.cfg_b.eIntDir        = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
            .GP.cfg_b.eGPOutCfg      = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
            .GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
            .GP.cfg_b.ePullup        = AM_HAL_GPIO_PIN_PULLUP_NONE,
            .GP.cfg_b.uNCE           = 0,
            .GP.cfg_b.eCEpol         = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,
            .GP.cfg_b.uRsvd_0        = 0,
            .GP.cfg_b.ePowerSw       = AM_HAL_GPIO_PIN_POWERSW_NONE,
            .GP.cfg_b.eForceInputEn  = AM_HAL_GPIO_PIN_FORCEEN_NONE,
            .GP.cfg_b.eForceOutputEn = AM_HAL_GPIO_PIN_FORCEEN_NONE,
            .GP.cfg_b.uRsvd_1        = 0,
        };
        am_hal_gpio_pinconfig(15, extClkPincfg);
    }
    #elif (CLK_SRC == PLL)
    {
        // Start the 32MHz crystal for use by PLL.
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
        am_hal_delay_us(1500);

        // Configure and enable PLL to output 24.576 MHz clock to AUDADC.
        am_hal_syspll_config_t pllConfig = AM_HAL_SYSPLL_DEFAULT_CFG_AUDADC;
        am_hal_syspll_initialize(0, &pllHandle);
        am_hal_syspll_configure(pllHandle, &pllConfig);
        am_hal_syspll_enable(pllHandle);

        // Wait for PLL Lock.
        am_hal_syspll_lock_wait(pllHandle);
    }
    #endif

    //
    // Initialize the AUDADC and get the handle.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_initialize(0, &g_AUDADCHandle))
    {
        am_util_stdio_printf("Error - reservation of the AUDADC instance failed.\n");
    }
    //
    // Power on the AUDADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_power_control(g_AUDADCHandle, AM_HAL_SYSCTRL_WAKE, false))
    {
        am_util_stdio_printf("Error - AUDADC power on failed.\n");
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure(g_AUDADCHandle, &AUDADCConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC failed.\n");
    }

    //
    // Set up internal repeat trigger timer
    //
    am_hal_audadc_configure_irtt(g_AUDADCHandle, &AUDADCIrttConfig);

    //
    // Enable the AUDADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_enable(g_AUDADCHandle))
    {
        am_util_stdio_printf("Error - enabling AUDADC failed.\n");
    }

    //
    // Enable internal repeat trigger timer
    //
    am_hal_audadc_irtt_enable(g_AUDADCHandle);

    //
    // Configure the AUDADC to use DMA for the sample transfer.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_dma(g_AUDADCHandle, &g_sAUDADCDMAConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC DMA failed.\n");
    }

    //
    // For this example, the samples will be coming in slowly. This means we
    // can afford to wake up for every conversion.
    //
    am_hal_audadc_interrupt_enable(g_AUDADCHandle, AM_HAL_AUDADC_INT_DCMP | AM_HAL_AUDADC_INT_DERR);
}

//*****************************************************************************
//
// Interrupt handler for the AUDADC.
//
//*****************************************************************************
void
am_audadc0_isr(void)
{
    uint32_t ui32IntMask;
    //
    // Read the interrupt status.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_interrupt_status(g_AUDADCHandle, &ui32IntMask, false))
    {
        am_util_stdio_printf("Error reading AUDADC interrupt status\n");
    }

    //
    // Clear the AUDADC interrupt.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_interrupt_clear(g_AUDADCHandle, ui32IntMask))
    {
        am_util_stdio_printf("Error clearing AUDADC interrupt status\n");
    }

    //
    // If we got a DMA complete, set the flag.
    //
    if (ui32IntMask & AM_HAL_AUDADC_INT_DCMP)
    {
        g_bAUDADCDMAError = false;
        g_bAUDADCDMAComplete = true;
    }

    //
    // If we got a DMA error, set the flag.
    //
    if (ui32IntMask & AM_HAL_AUDADC_INT_DERR)
    {
        g_bAUDADCDMAError = true;
    }

    #if defined(AM_PART_APOLLO5A)
    am_hal_audadc_interrupt_service(g_AUDADCHandle, &g_sAUDADCDMAConfig);
    #elif defined(AM_PART_APOLLO5B)
    am_hal_audadc_interrupt_service(g_AUDADCHandle, ui32IntMask);
    #endif
}

//*****************************************************************************
//
// Main function.
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

#if ENABLE_DC_OFFSET_CAL
    am_hal_offset_cal_coeffs_array_t sOffsetCalib;
#if RTT_LOGGING
    int32_t i32OffsetAdj = 0;
    uint16_t ui16AudChannel = 0;
#endif
#endif

    //
    // Start the ITM interface.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("==============================================\n");
    am_util_stdio_printf("AUDADC RTT streaming example.\n\n");

    //
    // Initialize RTT
    //
#if RTT_LOGGING
    SEGGER_RTT_Init();
    SEGGER_RTT_ConfigUpBuffer(1, "DataLogger", g_rttRecorderBuffer, RTT_BUFFER_LENGTH, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
#endif

    //
    // Print compiler version and rtt control block address
    //
    am_util_stdio_printf("App Compiler:    %s\n", COMPILER_VERSION);
#if RTT_LOGGING
    am_util_stdio_printf("RTT Control Block Address:  0x%08X\n", (uint32_t)&_SEGGER_RTT);
    am_util_stdio_printf("Starting RTT streaming...\n");
#endif
    am_util_stdio_printf("==============================================\n");

    //
    // AUDADC DMA data config:
    //
    g_sAUDADCDMAConfig.ui32TargetAddress = (uint32_t)(&g_ui32AUDADCSampleBuffer[0]);
    g_sAUDADCDMAConfig.ui32TargetAddressReverse = (uint32_t)(&g_ui32AUDADCSampleBuffer[g_sAUDADCDMAConfig.ui32SampleCount]);

    //
    // Power up PrePGA
    //
    am_hal_audadc_refgen_powerup();

    #if CH_A0_EN
    am_hal_audadc_pga_powerup(0);
    #endif
    #if CH_A1_EN
    am_hal_audadc_pga_powerup(1);
    #endif
    #if CH_B0_EN
    am_hal_audadc_pga_powerup(2);
    #endif
    #if CH_B1_EN
    am_hal_audadc_pga_powerup(3);
    #endif

    #if CH_A0_EN
    am_hal_audadc_gain_set(0, 2*PREAMP_FULL_GAIN);
    #endif
    #if CH_A1_EN
    am_hal_audadc_gain_set(1, 2*PREAMP_FULL_GAIN);
    #endif
    #if CH_B0_EN
    am_hal_audadc_gain_set(2, 2*PREAMP_FULL_GAIN);
    #endif
    #if CH_B1_EN
    am_hal_audadc_gain_set(3, 2*PREAMP_FULL_GAIN);
    #endif

    //
    //  Turn on mic bias
    //
    am_hal_audadc_micbias_powerup(24);
    am_util_delay_ms(400);

    //
    // Configure the AUDADC
    //
    audadc_config();

    // Gain setting
    #if CH_A0_EN && CH_A1_EN
    g_sAudadcGainConfig.ui32LGA = (uint32_t)((float)CH_A0_GAIN_DB*2 + 12);
    g_sAudadcGainConfig.ui32HGADELTA = ((uint32_t)((float)CH_A1_GAIN_DB*2 + 12)) - g_sAudadcGainConfig.ui32LGA;
    #endif
    #if CH_B0_EN && CH_B1_EN
    g_sAudadcGainConfig.ui32LGB = (uint32_t)((float)CH_B0_GAIN_DB*2 + 12);
    g_sAudadcGainConfig.ui32HGBDELTA = ((uint32_t)((float)CH_B1_GAIN_DB*2 + 12)) - g_sAudadcGainConfig.ui32LGB;
    #endif
    g_sAudadcGainConfig.eUpdateMode = AM_HAL_AUDADC_GAIN_UPDATE_IMME;
    am_hal_audadc_internal_pga_config(g_AUDADCHandle, &g_sAudadcGainConfig);

    // Configure the slot
    audadc_slot_config();
#if ENABLE_DC_OFFSET_CAL
    //
    // Calculate DC offset calibartion parameter.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_slot_dc_offset_calculate(g_AUDADCHandle, 4, &sOffsetCalib))
    {
        am_util_stdio_printf("Error - failed to calculate offset calibartion parameter.\n");
    }
#endif
    NVIC_SetPriority(AUDADC0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(AUDADC0_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Trigger the AUDADC sampling for the first time manually.
    //
    #if defined(AM_PART_APOLLO5A)
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_sw_trigger(g_AUDADCHandle))
    {
        am_util_stdio_printf("Error - triggering the AUDADC failed.\n");
    }
    #elif defined(AM_PART_APOLLO5B)
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_dma_transfer_start(g_AUDADCHandle))
    {
        am_util_stdio_printf("Error - triggering the AUDADC failed.\n");
    }
    #endif

    //
    // Loop forever.
    //
    while (1)
    {
        // Check for DMA errors.
        //
        if (g_bAUDADCDMAError)
        {
            am_util_stdio_printf("DMA Error occured\n");
            while (1);
        }

        //
        // Check if the AUDADC DMA completion interrupt occurred.
        //
        if (g_bAUDADCDMAComplete)
        {
            uint32_t* pui32_buffer = (uint32_t* )am_hal_audadc_dma_get_buffer(g_AUDADCHandle);
#if RTT_LOGGING
            //
            //DMA read.
            //
            uint32_t ui32SampleCount = AUDADC_SAMPLE_BUF_SIZE / 2;
            for (int i = 0; i < ui32SampleCount; i++)
            {
                g_in16AudioDataBuffer[2 * i + 0] = (int16_t)(pui32_buffer[2 * i + 0] & 0x0000FFF0); // Mic1
                g_in16AudioDataBuffer[2 * i + 1] = (int16_t)(pui32_buffer[2 * i + 1] & 0x0000FFF0); // Mic2
    #if ENABLE_DC_OFFSET_CAL
                ui16AudChannel = (uint16_t)((pui32_buffer[i] & 0x00080000) >> 19);
                //
                // Apply dc offset calibration to debug mode.
                //
                if (sOffsetCalib.sCalibCoeff[ui16AudChannel * 2].bValid)
                {
                    i32OffsetAdj = sOffsetCalib.sCalibCoeff[ui16AudChannel * 2].i32DCOffsetAdj << 4;
                    if ((g_in16AudioDataBuffer[2 * i] >= 0) && (i32OffsetAdj > (32767 - g_in16AudioDataBuffer[2 * i])))
                    {
                        g_in16AudioDataBuffer[2 * i] = 32767; // Saturation
                    }
                    else if ((g_in16AudioDataBuffer[2 * i] < 0) && (i32OffsetAdj < (-32768 - g_in16AudioDataBuffer[2 * i])))
                    {
                        g_in16AudioDataBuffer[2 * i] = -32768; // Saturation
                    }
                    else
                    {
                        g_in16AudioDataBuffer[2 * i] += i32OffsetAdj;
                    }
                }
                if (sOffsetCalib.sCalibCoeff[ui16AudChannel * 2 + 1].bValid)
                {
                    i32OffsetAdj = sOffsetCalib.sCalibCoeff[ui16AudChannel * 2 + 1].i32DCOffsetAdj << 4;
                    if ((g_in16AudioDataBuffer[2 * i + 1] >= 0) && (i32OffsetAdj > (32767 - g_in16AudioDataBuffer[2 * i + 1])))
                    {
                        g_in16AudioDataBuffer[2 * i + 1] = 32767; // Saturation
                    }
                    else if ((g_in16AudioDataBuffer[2 * i + 1] < 0) && (i32OffsetAdj < (-32768 - g_in16AudioDataBuffer[2 * i + 1])))
                    {
                        g_in16AudioDataBuffer[2 * i + 1] = -32768; // Saturation
                    }
                    else
                    {
                        g_in16AudioDataBuffer[2 * i + 1] += i32OffsetAdj;
                    }
                }
    #endif
            }

            SEGGER_RTT_Write(1, g_in16AudioDataBuffer, AUDADC_SAMPLE_BUF_SIZE*sizeof(int16_t));
#endif
            //
            // Reset the DMA completion and error flags.
            //
            g_bAUDADCDMAComplete = false;
        }
    }
}
