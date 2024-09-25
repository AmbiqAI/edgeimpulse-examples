//*****************************************************************************
//
//! @file audadc_power_cycling.c
//!
//! @brief This example shows AUADC power cycling sequence.
//!
//! Purpose: This example shows the power cycling sequence of AUDADC, it will
//! loop and print forever if no issue occurs.
//!
//! Hardware requirements on Apollo5 EB:
//! - Apollo5 EB only.
//! - IOX is selected mode 1.
//!
//! Printing takes place over the SWO at 1M Baud.
//!
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
#include "stdlib.h"

//*****************************************************************************
//
// MACRO
//
//*****************************************************************************

//#define POWER_CYCLE_MICBIAS

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

//
// AUDADC Device Handle.
//
static void *g_AUDADCHandle;
//
// AUDADC DMA complete flag.
//
volatile bool g_bAUDADCDMAComplete;
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
#define CH_B0_EN  0
#define CH_B1_EN  0
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
// Set up the core for sleeping, and then go to sleep.
//
//*****************************************************************************
void
sleep(void)
{
    //
    // Go to Deep Sleep.
    //
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

}

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
    AUDADCSlotConfig.ui32TrkCyc      = 24;
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
        .eClock             = AM_HAL_AUDADC_CLKSEL_XTHS_32MHz,
        .ePolarity          = AM_HAL_AUDADC_TRIGPOL_RISING,
        .eTrigger           = AM_HAL_AUDADC_TRIGSEL_SOFTWARE,
        .eClockMode         = AM_HAL_AUDADC_CLKMODE_LOW_LATENCY,
        .ePowerMode         = AM_HAL_AUDADC_LPMODE1,
        .eRepeat            = AM_HAL_AUDADC_REPEATING_SCAN,
        .eRepeatTrigger     = AM_HAL_AUDADC_RPTTRIGSEL_INT,
        .eSampMode          = AM_HAL_AUDADC_SAMPMODE_MED,       //AM_HAL_AUDADC_SAMPMODE_LP,
    };
    //
    // Set up internal repeat trigger timer
    //
    am_hal_audadc_irtt_config_t      AUDADCIrttConfig =
    {
        .bIrttEnable        = true,
        .eClkDiv            = AM_HAL_AUDADC_RPTT_CLK_DIV16,
        .ui32IrttCountMax   = 125,  //sample rate = eClock / eClkDiv / ui32IrttCountMax
    };
    //
    // Initialize the AUDADC and get the handle.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_audadc_initialize(0, &g_AUDADCHandle) )
    {
        am_util_stdio_printf("Error - reservation of the AUDADC instance failed.\n");
    }
    //
    // Power on the AUDADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_power_control(g_AUDADCHandle,
                                                          AM_HAL_SYSCTRL_WAKE,
                                                          false) )
    {
        am_util_stdio_printf("Error - AUDADC power on failed.\n");
    }

    if ( AUDADCConfig.eClock == AM_HAL_AUDADC_CLKSEL_XTHS_32MHz )
    {
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_NORMAL, NULL);
        am_util_delay_us(200);
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
    if ( ui32IntMask & AM_HAL_AUDADC_INT_DERR )
    {
        g_bAUDADCDMAError = true;
    }

    #if defined(AM_PART_APOLLO5A)
    am_hal_audadc_interrupt_service(g_AUDADCHandle, &g_sAUDADCDMAConfig);
    #elif defined(AM_PART_APOLLO5B)
    #endif
}

void audadc_restart(void)
{
    //
    // power off audadc
    //
    am_hal_audadc_interrupt_disable(g_AUDADCHandle, 0xFFFFFFFF);
    while(AUDADC->DMATOTCOUNT_b.TOTCOUNT != 0); // Ensure DMATOTCOUNT is set to 0 as part of de-initialization
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_control(g_AUDADCHandle, AM_HAL_AUDADC_REQ_DMA_DISABLE, NULL))
    {
        am_util_stdio_printf("Error - AUDADC control failed.\n");
    }
    am_hal_audadc_irtt_disable(g_AUDADCHandle);
    am_hal_audadc_disable(g_AUDADCHandle);
    am_hal_audadc_deinitialize(g_AUDADCHandle);
#ifdef POWER_CYCLE_MICBIAS
    am_hal_audadc_micbias_powerdown();
#endif
    #if CH_A0_EN
    am_hal_audadc_pga_powerdown(0);
    #endif
    #if CH_A1_EN
    am_hal_audadc_pga_powerdown(1);
    #endif
    #if CH_B0_EN
    am_hal_audadc_pga_powerdown(2);
    #endif
    #if CH_B1_EN
    am_hal_audadc_pga_powerdown(3);
    #endif
    am_hal_audadc_refgen_powerdown();
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_AUDADC);

    am_util_delay_ms(rand() % 20);
    //
    // power up audadc
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_AUDADC);
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
#ifdef POWER_CYCLE_MICBIAS
    //
    //  Turn on mic bias
    //
    am_hal_audadc_micbias_powerup(24);
    am_util_delay_ms(400);
#endif
    //
    // Configure the AUDADC
    //
    audadc_config();
    //
    // Gain setting
    //
    am_hal_audadc_internal_pga_config(g_AUDADCHandle, &g_sAudadcGainConfig);
    //
    // Configure the slot
    //
    audadc_slot_config();
    //
    // Trigger the AUDADC sampling for the first time manually.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_sw_trigger(g_AUDADCHandle))
    {
        am_util_stdio_printf("Error - triggering the AUDADC failed.\n");
    }
}
void
run_dma_for_random_duration(void)
{
    uint32_t ui32DelayMs = 0;
    uint32_t ui32RandomDuration;
    ui32RandomDuration = rand() % 500;
    while(1)
    {
        //
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
            //
            // Reset the DMA completion and error flags.
            //
            g_bAUDADCDMAComplete = false;
            //
            // Clear the AUDADC interrupts.
            //
            if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_interrupt_clear(g_AUDADCHandle, 0xFFFFFFFF))
            {
                am_util_stdio_printf("Error - clearing the AUDADC interrupts failed.\n");
            }
        }
        am_util_delay_ms(1);
        if (ui32DelayMs++ > ui32RandomDuration)
        {
            break;
        }
    }
}
//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32TestLoop = 0;

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
    // Start the ITM interface.
    //
    am_bsp_itm_printf_enable();
    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("==============================================\n");
    am_util_stdio_printf("AUDADC power cycling example.\n\n");

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
    //
    // Gain setting
    //
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
    //
    // Configure the slot
    //
    audadc_slot_config();

    NVIC_SetPriority(AUDADC0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(AUDADC0_IRQn);
    am_hal_interrupt_master_enable();
    //
    // Trigger the AUDADC sampling for the first time manually.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_sw_trigger(g_AUDADCHandle))
    {
        am_util_stdio_printf("Error - triggering the AUDADC failed.\n");
    }
    //
    // Loop forever.
    //
    while(1)
    {
        am_util_stdio_printf("Loop %d.\n", ui32TestLoop++);
        run_dma_for_random_duration();
        audadc_restart();
    }
}
