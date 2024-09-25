//*****************************************************************************
//
//! @file tdm_loopback.c
//!
//! @brief An example to show basic TDM operation.
//!
//! Purpose:
//! This example enables the I2S interfaces to loop back data with different
//! configurations from each other and compares data from the I2S TX and data
//! from the another I2S RX. Either I2S0 or I2S1 can be selected as the master.
//!
//! In TDM mode, I2S supports two kinds of data format in one frame which were
//! called phase 1 and phase 2. The number of channels can be different in both
//! phases but the total number shouldn't be greater than 8.
//!
//! NOTE: Usage of software-implemented ping pong machine
//! step 1: Prepare 2 blocks of buffer.
//!  - sTransfer0.ui32RxTargetAddr = addr1;
//!  - sTransfer0.ui32RxTargetAddrReverse = addr2;
//!  - am_hal_i2s_dma_configure(pI2S0Handle, &g_sI2S0Config, &sTransfer0);
//! step 2: Call am_hal_i2s_interrupt_service() in the ISR to restart DMA operation,
//! the ISR helps automatically switch to the reverse buffer.
//! step 3: Fetch the valid data by calling am_hal_i2s_dma_get_buffer().
//!
//! The required pin connections are as follows:
//! a. for Apollo5 EB:
//! IOX is selected mode 1.
//! No peripheral card is connected.
//! GPIO_5 I2S0CLK is connected to GPIO_16 I2S1CLK.
//! GPIO_7 I2S0WS is connected to GPIO_18 I2S1WS.
//! GPIO_6 I2S0DOUT is connected to GPIO_19 I2S1DIN.
//! GPIO_14 I2S0DIN is connected to GPIO_17 I2S1DOUT.
//! b. for Apollo5 EB with 8080 peripheral card:
//! IOX is selected mode 1.
//! Make sure U13 on the 8080 card has been removed.
//! Short I2S_LBS_SEL and I2S_LB_ENn on the 8080 card to the ground.
//!
//! Printing takes place over the ITM at 1M Baud.
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
#include <stdlib.h>

//*****************************************************************************
//
// Example parameters.
//
//*****************************************************************************
#if defined(AM_PART_APOLLO5A)
#define CLOCK_SOURCE                (HFRC)
#elif defined(AM_PART_APOLLO5B)
#define CLOCK_SOURCE                (PLL)
#endif
    #define HFRC                    (0)
    #define PLL                     (1)
    #define NCO                     (2)
    #define HF2ADJ                  (3)
#define I2S_MODULE_0                (0)
#define I2S_MODULE_1                (1)
#define USE_I2S_MASTER              I2S_MODULE_0
#define SIZE_SAMPLES                (4096 - 32)     // padded to 32 samples, align with DMA Threshold.
#define DMACPL_TIMEOUT_MS           (2000)

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
//
// Record the number of test loops.
//
static uint32_t g_ui32TestLoop = 0;

//
// I2S handlers.
//
void *pI2S0Handle;
void *pI2S1Handle;

// PLL handler.
void *pllHandle;

//
// I2S interrupts number.
//
static const IRQn_Type i2s_interrupts[] =
{
    I2S0_IRQn,
    I2S1_IRQn,
};

//
// Flags of DMA complete.
//
volatile uint32_t g_ui32I2SDmaCpl[] =
{
    0, //I2S0 TX.
    0, //I2S0 RX.
    0, //I2S1 Tx.
    0, //I2S1 Rx.
};

//
// Used as the ping-pong buffer of DMA.
// Aligned to 32 bytes to meet data cache requirements.
//
AM_SHARED_RW uint32_t g_ui32I2s0TxDmaBufPing[SIZE_SAMPLES] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t g_ui32I2s0TxDmaBufPong[SIZE_SAMPLES] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t g_ui32I2s1TxDmaBufPing[SIZE_SAMPLES] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t g_ui32I2s1TxDmaBufPong[SIZE_SAMPLES] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t g_ui32I2s0RxDmaBufPing[SIZE_SAMPLES] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t g_ui32I2s0RxDmaBufPong[SIZE_SAMPLES] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t g_ui32I2s1RxDmaBufPing[SIZE_SAMPLES] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t g_ui32I2s1RxDmaBufPong[SIZE_SAMPLES] __attribute__((aligned(32)));

//*****************************************************************************
//
// I2S configurations:
//  - 16 kHz sample rate in common
//  - Total 8 channles
//  - Variable word width
//  - Variable bit-depth
//
//*****************************************************************************
static am_hal_i2s_io_signal_t g_sI2SIOConfig =
{
    .sFsyncPulseCfg = {
        .eFsyncPulseType = AM_HAL_I2S_FSYNC_PULSE_ONE_BIT_CLOCK,
    },
    .eFyncCpol = AM_HAL_I2S_IO_FSYNC_CPOL_HIGH,
    .eTxCpol = AM_HAL_I2S_IO_TX_CPOL_FALLING,
    .eRxCpol = AM_HAL_I2S_IO_RX_CPOL_RISING
};

static am_hal_i2s_data_format_t g_sI2SDataConfig[] =
{
    {
        .ePhase = AM_HAL_I2S_DATA_PHASE_SINGLE,
        .eDataDelay = 0x1,
        .ui32ChannelNumbersPhase1 = 8,
        .ui32ChannelNumbersPhase2 = 0,
        .eDataJust = AM_HAL_I2S_DATA_JUSTIFIED_LEFT,
        .eChannelLenPhase1 = AM_HAL_I2S_FRAME_WDLEN_32BITS,
        .eChannelLenPhase2 = AM_HAL_I2S_FRAME_WDLEN_32BITS,
        .eSampleLenPhase1 = AM_HAL_I2S_SAMPLE_LENGTH_24BITS,
        .eSampleLenPhase2 = AM_HAL_I2S_SAMPLE_LENGTH_24BITS
    },
    {
        .ePhase = AM_HAL_I2S_DATA_PHASE_DUAL,
        .eDataDelay = 0x1,
        .ui32ChannelNumbersPhase1 = 4,
        .ui32ChannelNumbersPhase2 = 4,
        .eDataJust = AM_HAL_I2S_DATA_JUSTIFIED_LEFT,
        .eChannelLenPhase1 = AM_HAL_I2S_FRAME_WDLEN_32BITS,
        .eChannelLenPhase2 = AM_HAL_I2S_FRAME_WDLEN_32BITS,
        .eSampleLenPhase1 = AM_HAL_I2S_SAMPLE_LENGTH_24BITS,
        .eSampleLenPhase2 = AM_HAL_I2S_SAMPLE_LENGTH_16BITS
    },
    {
        .ePhase = AM_HAL_I2S_DATA_PHASE_DUAL,
        .eDataDelay = 0x1,
        .ui32ChannelNumbersPhase1 = 4,
        .ui32ChannelNumbersPhase2 = 4,
        .eDataJust = AM_HAL_I2S_DATA_JUSTIFIED_LEFT,
        .eChannelLenPhase1 = AM_HAL_I2S_FRAME_WDLEN_32BITS,
        .eChannelLenPhase2 = AM_HAL_I2S_FRAME_WDLEN_32BITS,
        .eSampleLenPhase1 = AM_HAL_I2S_SAMPLE_LENGTH_32BITS,
        .eSampleLenPhase2 = AM_HAL_I2S_SAMPLE_LENGTH_24BITS
    },
    {
        .ePhase = AM_HAL_I2S_DATA_PHASE_DUAL,
        .eDataDelay = 0x1,
        .ui32ChannelNumbersPhase1 = 4,
        .ui32ChannelNumbersPhase2 = 4,
        .eDataJust = AM_HAL_I2S_DATA_JUSTIFIED_LEFT,
        .eChannelLenPhase1 = AM_HAL_I2S_FRAME_WDLEN_32BITS,
        .eChannelLenPhase2 = AM_HAL_I2S_FRAME_WDLEN_32BITS,
        .eSampleLenPhase1 = AM_HAL_I2S_SAMPLE_LENGTH_32BITS,
        .eSampleLenPhase2 = AM_HAL_I2S_SAMPLE_LENGTH_16BITS
    },
};

static am_hal_i2s_config_t g_sI2S0Config =
{
    #if (USE_I2S_MASTER == I2S_MODULE_0)
    .eMode  = AM_HAL_I2S_IO_MODE_MASTER,
    #else
    .eMode  = AM_HAL_I2S_IO_MODE_SLAVE,
    #endif
    .eXfer  = AM_HAL_I2S_XFER_RXTX,
    #if (CLOCK_SOURCE == HFRC)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC_12MHz,
    .eDiv3  = 1,
    #elif (CLOCK_SOURCE == PLL)
    .eClock = eAM_HAL_I2S_CLKSEL_PLL_FOUT4,
    .eDiv3  = 1,
    #elif (CLOCK_SOURCE == NCO)
    .eClock    = eAM_HAL_I2S_CLKSEL_NCO_HFRC_48MHz,
    .f64NcoDiv = 48.0 / 4.096,
    #elif (CLOCK_SOURCE == HF2ADJ)
    #if defined(AM_PART_APOLLO5A)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_8MHz,
    #elif defined(AM_PART_APOLLO5B)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_16MHz,
    #endif
    .eDiv3  = 1,
    #endif
    .eASRC  = 0,
    .eData  = &g_sI2SDataConfig[0],
    .eIO    = &g_sI2SIOConfig
};

static am_hal_i2s_config_t g_sI2S1Config =
{
    #if (USE_I2S_MASTER == I2S_MODULE_0)
    .eMode  = AM_HAL_I2S_IO_MODE_SLAVE,
    #else
    .eMode  = AM_HAL_I2S_IO_MODE_MASTER,
    #endif
    .eXfer  = AM_HAL_I2S_XFER_RXTX,
    #if (CLOCK_SOURCE == HFRC)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC_12MHz,
    .eDiv3  = 1,
    #elif (CLOCK_SOURCE == PLL)
    .eClock = eAM_HAL_I2S_CLKSEL_PLL_FOUT4,
    .eDiv3  = 1,
    #elif (CLOCK_SOURCE == NCO)
    .eClock    = eAM_HAL_I2S_CLKSEL_NCO_HFRC_48MHz,
    .f64NcoDiv = 48.0 / 4.096,
    #elif (CLOCK_SOURCE == HF2ADJ)
    #if defined(AM_PART_APOLLO5A)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_8MHz,
    #elif defined(AM_PART_APOLLO5B)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_16MHz,
    #endif
    .eDiv3  = 1,
    #endif
    .eASRC  = 0,
    .eData  = &g_sI2SDataConfig[0],
    .eIO    = &g_sI2SIOConfig,
};

//
// Ping pong buffer settings.
//
static am_hal_i2s_transfer_t sTransfer0 =
{
    .ui32RxTotalCount         = SIZE_SAMPLES,
    .ui32RxTargetAddr         = (uint32_t)(&g_ui32I2s0RxDmaBufPing[0]),
    .ui32RxTargetAddrReverse  = (uint32_t)(&g_ui32I2s0RxDmaBufPong[0]),
    .ui32TxTotalCount         = SIZE_SAMPLES,
    .ui32TxTargetAddr         = (uint32_t)(&g_ui32I2s0TxDmaBufPing[0]),
    .ui32TxTargetAddrReverse  = (uint32_t)(&g_ui32I2s0TxDmaBufPong[0]),
};

static am_hal_i2s_transfer_t sTransfer1 =
{
    .ui32RxTotalCount         = SIZE_SAMPLES,
    .ui32RxTargetAddr         = (uint32_t)(&g_ui32I2s1RxDmaBufPing[0]),
    .ui32RxTargetAddrReverse  = (uint32_t)(&g_ui32I2s1RxDmaBufPong[0]),
    .ui32TxTotalCount         = SIZE_SAMPLES,
    .ui32TxTargetAddr         = (uint32_t)(&g_ui32I2s1TxDmaBufPing[0]),
    .ui32TxTargetAddrReverse  = (uint32_t)(&g_ui32I2s1TxDmaBufPong[0]),
};

//*****************************************************************************
//
// I2S data checking function.
//
// Compare data in the RX buffer and data in the TX buffer, return true if data
// match, otherwise return false.
//
//*****************************************************************************
static bool
check_i2s_data(uint32_t ui32BufSize, uint32_t* pui32RxBuf, uint32_t* pui32TxBuf, am_hal_i2s_data_format_t* pI2SDataConfig, bool bRxIsMaster)
{
    uint32_t i, ui32Mask, ui32Mask1, ui32Mask2, ui32WordBytes;

    uint32_t ui32ChNumPh1 = pI2SDataConfig->ui32ChannelNumbersPhase1;
    uint32_t ui32ChNumPh2 = pI2SDataConfig->ui32ChannelNumbersPhase2;
    uint32_t ui32SampleLenPh1 = ui32I2sWordLength[pI2SDataConfig->eSampleLenPhase1];
    uint32_t ui32SampleLenPh2 = ui32I2sWordLength[pI2SDataConfig->eSampleLenPhase2];
    uint32_t ui32FrameChNum = ui32ChNumPh1 + ui32ChNumPh2;

    if (ui32SampleLenPh1 < ui32SampleLenPh2)
    {
        ui32SampleLenPh2 = ui32SampleLenPh1;
    }

    if (ui32SampleLenPh1 == 16)
    {
        ui32BufSize -= 2;
    }

    if (ui32SampleLenPh1 == 8)
    {
        ui32BufSize -= 3;
    }

    if (ui32SampleLenPh1 == 32)
    {
        ui32WordBytes = 4;
        ui32Mask1 = 0xFFFFFFFF;
        switch (ui32SampleLenPh2)
        {
            case 32: ui32Mask2 = 0xFFFFFFFF; break;
            case 24: ui32Mask2 = 0xFFFFFF00; break;
            case 16: ui32Mask2 = 0xFFFF0000; break;
            case 8:  ui32Mask2 = 0xFF000000; break;
            default: am_util_stdio_printf("ERROR! Invalidate sample length!"); return false;
        }
    }
    else if (ui32SampleLenPh1 == 24)
    {
        ui32WordBytes = 4;
        ui32Mask1 = 0x00FFFFFF;
        switch (ui32SampleLenPh2)
        {
            case 24: ui32Mask2 = 0x00FFFFFF; break;
            case 16: ui32Mask2 = 0x00FFFF00; break;
            case 8:  ui32Mask2 = 0x00FF0000; break;
            default: am_util_stdio_printf("ERROR! Invalidate sample length!"); return false;
        }
    }
    else if (ui32SampleLenPh1 == 16)
    {
        ui32WordBytes = 2;
        ui32Mask1 = 0xFFFF;
        switch (ui32SampleLenPh2)
        {
            case 16: ui32Mask2 = 0xFFFF; break;
            case 8:  ui32Mask2 = 0xFF00; break;
            default: am_util_stdio_printf("ERROR! Invalidate sample length!"); return false;
        }
    }
    else if (ui32SampleLenPh1 == 8)
    {
        ui32WordBytes = 1;
        ui32Mask1 = 0xFF;
        switch (ui32SampleLenPh2)
        {
            case 8:  ui32Mask2 = 0xFF; break;
            default: am_util_stdio_printf("ERROR! Invalidate sample length!"); return false;
        }
    }
    else
    {
        am_util_stdio_printf("ERROR! Invalidate sample length!");
        return false;
    }

    //
    // Find the beginning sample.
    //
    uint32_t ui32StartPoint = 0xFFFFFFFF;
    uint32_t ui32MatchCount = 0;
    uint32_t ui32RxIndex, ui32TxIndex;
    bool bMatchTrue = false;

    //
    // The maximum allowed lag between master TX and slave TX is 500 us.
    // At 12.288 MHz SCLK, there will be 12.288 MHz * 0.0005 s / 8 bits = 768 bytes of data which
    // won't be received by the slave RX, or 768 bytes of zero will be received by the master RX.
    //
    uint32_t ui32StartTolerance = (ui32BufSize > 768) ? 768 : ui32BufSize;

    for (i = 0; i < ui32StartTolerance / ui32WordBytes; i++)
    {
        ui32TxIndex = (bRxIsMaster) ? ui32MatchCount : i;
        ui32RxIndex = (bRxIsMaster) ? i : ui32MatchCount;

        ui32Mask = ((ui32RxIndex % ui32FrameChNum) < ui32ChNumPh1) ? ui32Mask1 : ui32Mask2;

        switch (ui32WordBytes)
        {
            case 1: bMatchTrue = ( ((uint8_t*)pui32RxBuf)[ui32RxIndex] ==   ((uint8_t*)pui32TxBuf)[ui32TxIndex]); break;
            case 2: bMatchTrue = (((uint16_t*)pui32RxBuf)[ui32RxIndex] == (((uint16_t*)pui32TxBuf)[ui32TxIndex] & (uint16_t)ui32Mask)); break;
            case 4: bMatchTrue = (((uint32_t*)pui32RxBuf)[ui32RxIndex] == (((uint32_t*)pui32TxBuf)[ui32TxIndex] & ui32Mask)); break;
        }

        //
        // Until we match 8 bytes of data, we find the start point.
        //
        if (bMatchTrue)
        {
            ui32MatchCount++;
            if ((ui32MatchCount * ui32WordBytes) >= 8)
            {
                ui32StartPoint = i - ui32MatchCount + 1;
                break;
            }
        }
        else
        {
            ui32MatchCount = 0;
        }
    }

    if (ui32StartPoint == 0xFFFFFFFF)
    {
        am_util_stdio_printf("Did not find the beginning word in the buffer!\n");
        return false;
    }

    //
    // Compare the remaining data.
    //
    for (i = ui32StartPoint; i < ui32BufSize / ui32WordBytes; i++)
    {
        ui32TxIndex = (bRxIsMaster) ? (i - ui32StartPoint) : i;
        ui32RxIndex = (bRxIsMaster) ? i : (i - ui32StartPoint);

        ui32Mask = ((ui32RxIndex % ui32FrameChNum) < ui32ChNumPh1) ? ui32Mask1 : ui32Mask2;

        //
        // These two values should be equal, otherwise the transmitter mask is different from the receiver mask.
        //
        if ((ui32RxIndex % ui32FrameChNum) != (ui32TxIndex % ui32FrameChNum))
        {
            am_util_stdio_printf("ERROR: Channel is out of order.\n");
            return false;
        }

        switch (ui32WordBytes)
        {
            case 1: bMatchTrue = ( ((uint8_t*)pui32RxBuf)[ui32RxIndex] ==   ((uint8_t*)pui32TxBuf)[ui32TxIndex]); break;
            case 2: bMatchTrue = (((uint16_t*)pui32RxBuf)[ui32RxIndex] == (((uint16_t*)pui32TxBuf)[ui32TxIndex] & (uint16_t)ui32Mask)); break;
            case 4: bMatchTrue = (((uint32_t*)pui32RxBuf)[ui32RxIndex] == (((uint32_t*)pui32TxBuf)[ui32TxIndex] & ui32Mask)); break;
        }

        if (bMatchTrue == false)
        {
            am_util_stdio_printf("Data checking faild: Started from: %d, failed at %d, Word bytes %d\n", ui32StartPoint, i, ui32WordBytes);
            return false;
        }
    }

    return true;
}

//*****************************************************************************
//
// I2S0 interrupt handler.
//
//*****************************************************************************
void
am_dspi2s0_isr()
{
    uint32_t ui32Status;

    am_hal_i2s_interrupt_status_get(pI2S0Handle, &ui32Status, true);
    am_hal_i2s_interrupt_clear(pI2S0Handle, ui32Status);

    //
    // Switch ping pong buffer.
    //
    am_hal_i2s_interrupt_service(pI2S0Handle, ui32Status, &g_sI2S0Config);

    if (ui32Status & AM_HAL_I2S_INT_TXDMACPL)
    {
        g_ui32I2SDmaCpl[0] = 1;
    }

    if (ui32Status & AM_HAL_I2S_INT_RXDMACPL)
    {
        g_ui32I2SDmaCpl[1] = 1;
    }
}

//*****************************************************************************
//
// I2S1 interrupt handler.
//
//*****************************************************************************
void
am_dspi2s1_isr()
{
    uint32_t ui32Status;

    am_hal_i2s_interrupt_status_get(pI2S1Handle, &ui32Status, true);
    am_hal_i2s_interrupt_clear(pI2S1Handle, ui32Status);

    //
    // Switch ping pong buffer.
    //
    am_hal_i2s_interrupt_service(pI2S1Handle, ui32Status, &g_sI2S1Config);

    if (ui32Status & AM_HAL_I2S_INT_TXDMACPL)
    {
        g_ui32I2SDmaCpl[2] = 1;
    }

    if (ui32Status & AM_HAL_I2S_INT_RXDMACPL)
    {
        g_ui32I2SDmaCpl[3] = 1;
    }
}

//*****************************************************************************
//
// I2S initialization.
//
//*****************************************************************************
void
i2s_init(void)
{
    #if (CLOCK_SOURCE == PLL)
    {
        // Start XTHS for use by PLL.
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, false);
        am_util_delay_us(1500);

        // Configure and enable PLL to output 12.288 MHz clock to I2S.
        am_hal_syspll_config_t pllConfig =
        {
            .eFref         = AM_HAL_SYSPLL_FREFSEL_XTAL32MHz,
            .eVCOSel       = AM_HAL_SYSPLL_VCOSEL_VCOLO,
            .eFractionMode = AM_HAL_SYSPLL_FMODE_FRACTION,
            .ui8RefDiv     = 5,
            .ui8PostDiv1   = 1,
            .ui8PostDiv2   = 1,
            .ui16FBDivInt  = 15,
            .ui32FBDivFrac = 6039798,
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
    // Configure the necessary pins.
    //
    am_bsp_i2s_pins_enable(I2S_MODULE_0, false);
    am_bsp_i2s_pins_enable(I2S_MODULE_1, false);

    //
    // Configure I2S0 and I2S1.
    //
    am_hal_i2s_initialize(I2S_MODULE_0, &pI2S0Handle);
    am_hal_i2s_initialize(I2S_MODULE_1, &pI2S1Handle);
    am_hal_i2s_power_control(pI2S0Handle, AM_HAL_I2S_POWER_ON, false);
    am_hal_i2s_power_control(pI2S1Handle, AM_HAL_I2S_POWER_ON, false);

    if (AM_HAL_STATUS_SUCCESS != am_hal_i2s_configure(pI2S0Handle, &g_sI2S0Config))
    {
        am_util_stdio_printf("ERROR: Invalid I2S0 configuration.\nNote: For Apollo5 Rev.B0, I2S can only use PLL as the clock source.\n");
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_i2s_configure(pI2S1Handle, &g_sI2S1Config))
    {
        am_util_stdio_printf("ERROR: Invalid I2S1 configuration.\nNote: For Apollo5 Rev.B0, I2S can only use PLL as the clock source.\n");
    }

    am_hal_i2s_enable(pI2S0Handle);
    am_hal_i2s_enable(pI2S1Handle);
}

//*****************************************************************************
//
// I2S deinitialization.
//
//*****************************************************************************
void
i2s_deinit(void)
{
    //
    // I2S0
    //
    am_hal_i2s_disable(pI2S0Handle);
    am_hal_i2s_power_control(pI2S0Handle, AM_HAL_I2S_POWER_OFF, false);
    am_hal_i2s_deinitialize(pI2S0Handle);
    am_bsp_i2s_pins_disable(I2S_MODULE_0, false);

    //
    // I2S1
    //
    am_hal_i2s_disable(pI2S1Handle);
    am_hal_i2s_power_control(pI2S1Handle, AM_HAL_I2S_POWER_OFF, false);
    am_hal_i2s_deinitialize(pI2S1Handle);
    am_bsp_i2s_pins_disable(I2S_MODULE_1, false);

    #if (CLOCK_SOURCE == PLL)
    {
        // Disable PLL.
        am_hal_syspll_disable(pllHandle);
        am_hal_syspll_deinitialize(pllHandle);

        // Disable XTHS.
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_DISABLE, false);
    }
    #elif (CLOCK_SOURCE == HF2ADJ)
    {
        // Disable HF2ADJ.
        am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HF2ADJ_DISABLE, false);

        // Disable XTHS.
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_DISABLE, false);
    }
    #endif
}

//*****************************************************************************
//
// Pin independency checking function.
//
// This pin shouldn't be held by any external source.
//
//*****************************************************************************
bool
pin_independency_check(uint32_t ui32PinNum)
{
    uint32_t ui32PullUpVal, ui32PullDownVal;
    am_hal_gpio_pincfg_t sPinConfig = AM_HAL_GPIO_PINCFG_INPUT;

    //
    // Configure this pin as input with 50k pullup.
    //
    sPinConfig.GP.cfg_b.ePullup = AM_HAL_GPIO_PIN_PULLUP_50K;
    am_hal_gpio_pinconfig(ui32PinNum, sPinConfig);

    //
    // Read the input value, it should be 1.
    //
    am_hal_delay_us(10);
    am_hal_gpio_state_read(ui32PinNum, AM_HAL_GPIO_INPUT_READ, &ui32PullUpVal);

    //
    // Configure this pin as input with 50k pulldown.
    //
    sPinConfig.GP.cfg_b.ePullup = AM_HAL_GPIO_PIN_PULLDOWN_50K;
    am_hal_gpio_pinconfig(ui32PinNum, sPinConfig);

    //
    // Read the input value, it should be 0.
    //
    am_hal_delay_us(10);
    am_hal_gpio_state_read(ui32PinNum, AM_HAL_GPIO_INPUT_READ, &ui32PullDownVal);

    //
    // Disable this pin.
    //
    am_hal_gpio_pinconfig(ui32PinNum, am_hal_gpio_pincfg_disabled);

    if ((ui32PullUpVal == 1) && (ui32PullDownVal == 0))
    {
        return true;
    }
    else
    {
        am_util_stdio_printf("ERROR: GPIO_%d is pulled by another source.\n", ui32PinNum);
        return false;
    }
}

//*****************************************************************************
//
// Pins connectivity checking function.
//
//*****************************************************************************
bool
pins_connectivity_check(uint32_t ui32Pin1Num, uint32_t ui32Pin2Num)
{
    uint32_t ui32SetVal, ui32ClearVal;

    am_hal_gpio_pincfg_t sPin1Config = AM_HAL_GPIO_PINCFG_OUTPUT;
    am_hal_gpio_pincfg_t sPin2Config = AM_HAL_GPIO_PINCFG_INPUT;

    //
    // Configure pin1 and pin2.
    //
    am_hal_gpio_pinconfig(ui32Pin1Num, sPin1Config);
    am_hal_gpio_pinconfig(ui32Pin2Num, sPin2Config);

    //
    // Set pin1 and read pin2
    //
    am_hal_gpio_state_write(ui32Pin1Num, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_delay_us(10);
    am_hal_gpio_state_read(ui32Pin2Num, AM_HAL_GPIO_INPUT_READ, &ui32SetVal);

    //
    // Clear pin1 and read pin2
    //
    am_hal_gpio_state_write(ui32Pin1Num, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_delay_us(10);
    am_hal_gpio_state_read(ui32Pin2Num, AM_HAL_GPIO_INPUT_READ, &ui32ClearVal);

    //
    // If the two pins connect properly, this should return true.
    //
    if ((ui32SetVal == 1) && (ui32ClearVal == 0))
    {
        return true;
    }
    else
    {
        am_util_stdio_printf("ERROR: GPIO_%d and GPIO_%d are not connected.\n", ui32Pin1Num, ui32Pin2Num);
        return false;
    }
}

//*****************************************************************************
//
// I2S loopback pins checking function.
//
//*****************************************************************************
bool
i2s_loopback_pins_check()
{
    bool bRet = true;

    am_bsp_i2s_pins_disable(I2S_MODULE_0, false);
    am_bsp_i2s_pins_disable(I2S_MODULE_1, false);

    bRet &= pin_independency_check(AM_BSP_GPIO_I2S0_CLK);
    bRet &= pin_independency_check(AM_BSP_GPIO_I2S0_WS);
    bRet &= pin_independency_check(AM_BSP_GPIO_I2S0_SDIN);
    bRet &= pin_independency_check(AM_BSP_GPIO_I2S0_SDOUT);
    bRet &= pin_independency_check(AM_BSP_GPIO_I2S1_CLK);
    bRet &= pin_independency_check(AM_BSP_GPIO_I2S1_WS);
    bRet &= pin_independency_check(AM_BSP_GPIO_I2S1_SDIN);
    bRet &= pin_independency_check(AM_BSP_GPIO_I2S1_SDOUT);

    bRet &= pins_connectivity_check(AM_BSP_GPIO_I2S0_CLK,   AM_BSP_GPIO_I2S1_CLK);
    bRet &= pins_connectivity_check(AM_BSP_GPIO_I2S0_WS,    AM_BSP_GPIO_I2S1_WS);
    bRet &= pins_connectivity_check(AM_BSP_GPIO_I2S0_SDIN,  AM_BSP_GPIO_I2S1_SDOUT);
    bRet &= pins_connectivity_check(AM_BSP_GPIO_I2S0_SDOUT, AM_BSP_GPIO_I2S1_SDIN);

    return bRet;
}

//*****************************************************************************
//
// Print I2S0 channel and data information.
//
//*****************************************************************************
void
print_i2s0_configuration()
{
    if (I2Sn(0)->I2SDATACFG_b.PH == 0)
    {
        am_util_stdio_printf("-- Single Phase\n");
        am_util_stdio_printf("-- Channel:    %2d\n", I2Sn(0)->I2SDATACFG_b.FRLEN1 + 1);
        am_util_stdio_printf("-- Word Width: %2d\n", ui32I2sWordLength[I2Sn(0)->I2SDATACFG_b.WDLEN1]);
        am_util_stdio_printf("-- Bit Depth:  %2d\n", ui32I2sWordLength[I2Sn(0)->I2SDATACFG_b.SSZ1]);
    }
    else
    {
        am_util_stdio_printf("-- Dual Phase\n");
        am_util_stdio_printf("-- Channel:    %2d, %2d\n", I2Sn(0)->I2SDATACFG_b.FRLEN1 + 1, I2Sn(0)->I2SDATACFG_b.FRLEN2 + 1);
        am_util_stdio_printf("-- Word Width: %2d, %2d\n", ui32I2sWordLength[I2Sn(0)->I2SDATACFG_b.WDLEN1], ui32I2sWordLength[I2Sn(0)->I2SDATACFG_b.WDLEN2]);
        am_util_stdio_printf("-- Bit Depth:  %2d, %2d\n", ui32I2sWordLength[I2Sn(0)->I2SDATACFG_b.SSZ1], ui32I2sWordLength[I2Sn(0)->I2SDATACFG_b.SSZ2]);
    }
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
    // Initialize the printf interface for ITM output.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();

    #if (USE_I2S_MASTER == I2S_MODULE_0)
    am_util_stdio_printf("TDM Full Duplex Loopback Test: Master = I2S0, Slave = I2S1.\n\n");
    #else
    am_util_stdio_printf("TDM Full Duplex Loopback Test: Master = I2S1, Slave = I2S0.\n\n");
    #endif

    #ifndef APOLLO5_FPGA
    if (i2s_loopback_pins_check() != true)
    {
        am_util_stdio_printf("ERROR: I2S loopback pins checking failed.\n");
        return 0;
    }
    #endif

    //
    // I2S DMACPL interrupts timeout value.
    //
    uint32_t ui32Timeout0P1Ms = DMACPL_TIMEOUT_MS * 10;

    for (uint32_t i = 0; i < sizeof(g_sI2SDataConfig) / sizeof(am_hal_i2s_data_format_t); i++)
    {
        am_util_stdio_printf("\nTDM Loopback Test with Configuration %d...\n", i);

        g_ui32TestLoop = 0;
        g_ui32I2SDmaCpl[0] = g_ui32I2SDmaCpl[1] = g_ui32I2SDmaCpl[2] = g_ui32I2SDmaCpl[3] = 0;
        g_sI2S0Config.eData = &g_sI2SDataConfig[i];
        g_sI2S1Config.eData = &g_sI2SDataConfig[i];

        i2s_init();
        print_i2s0_configuration();
        am_hal_i2s_dma_configure(pI2S0Handle, &g_sI2S0Config, &sTransfer0);
        am_hal_i2s_dma_configure(pI2S1Handle, &g_sI2S1Config, &sTransfer1);
        NVIC_EnableIRQ(i2s_interrupts[I2S_MODULE_0]);
        NVIC_EnableIRQ(i2s_interrupts[I2S_MODULE_1]);
        am_hal_interrupt_master_enable();

        //
        // Prepare the 1st and the 2nd block of data.
        //
        for (uint32_t i = 0; i < SIZE_SAMPLES; i++)
        {
            g_ui32I2s0TxDmaBufPing[i] = rand();
            g_ui32I2s1TxDmaBufPing[i] = rand();
            g_ui32I2s0TxDmaBufPong[i] = rand();
            g_ui32I2s1TxDmaBufPong[i] = rand();
        }
        am_hal_i2s_flush_tx_buffer((uint32_t)g_ui32I2s0TxDmaBufPing, sizeof(g_ui32I2s0TxDmaBufPing));
        am_hal_i2s_flush_tx_buffer((uint32_t)g_ui32I2s1TxDmaBufPing, sizeof(g_ui32I2s1TxDmaBufPing));
        am_hal_i2s_flush_tx_buffer((uint32_t)g_ui32I2s0TxDmaBufPong, sizeof(g_ui32I2s0TxDmaBufPong));
        am_hal_i2s_flush_tx_buffer((uint32_t)g_ui32I2s1TxDmaBufPong, sizeof(g_ui32I2s1TxDmaBufPong));

        //
        // Start DMA transaction.
        //
        #if (USE_I2S_MASTER == I2S_MODULE_0)
        am_hal_i2s_dma_transfer_start(pI2S1Handle, &g_sI2S1Config);
        am_hal_i2s_dma_transfer_start(pI2S0Handle, &g_sI2S0Config);
        #else
        am_hal_i2s_dma_transfer_start(pI2S0Handle, &g_sI2S0Config);
        am_hal_i2s_dma_transfer_start(pI2S1Handle, &g_sI2S1Config);
        #endif

        while (1)
        {
            if (g_ui32I2SDmaCpl[0] && g_ui32I2SDmaCpl[1] && g_ui32I2SDmaCpl[2] && g_ui32I2SDmaCpl[3])
            {
                g_ui32TestLoop++;
                g_ui32I2SDmaCpl[0] = g_ui32I2SDmaCpl[1] = g_ui32I2SDmaCpl[2] = g_ui32I2SDmaCpl[3] = 0;
                ui32Timeout0P1Ms = DMACPL_TIMEOUT_MS * 10;

                //
                // RX DMA buffer that has completed reception.
                //
                uint32_t* i2s0RxBuf = (uint32_t*)am_hal_i2s_dma_get_buffer(pI2S0Handle, AM_HAL_I2S_XFER_RX);
                uint32_t* i2s1RxBuf = (uint32_t*)am_hal_i2s_dma_get_buffer(pI2S1Handle, AM_HAL_I2S_XFER_RX);

                //
                // TX DMA buffer that has completed sending.
                //
                uint32_t* i2s0TxBuf = ((g_ui32TestLoop % 2) == 1) ? g_ui32I2s0TxDmaBufPing : g_ui32I2s0TxDmaBufPong;
                uint32_t* i2s1TxBuf = ((g_ui32TestLoop % 2) == 1) ? g_ui32I2s1TxDmaBufPing : g_ui32I2s1TxDmaBufPong;

                //
                // Compare the TX DMA buffer and the RX DMA buffer to check if the data matches.
                //
                bool bI2S0to1Transfer = check_i2s_data(SIZE_SAMPLES * 4, i2s1RxBuf, i2s0TxBuf, &g_sI2SDataConfig[i], (g_sI2S1Config.eMode == AM_HAL_I2S_IO_MODE_MASTER));
                bool bI2S1to0Transfer = check_i2s_data(SIZE_SAMPLES * 4, i2s0RxBuf, i2s1TxBuf, &g_sI2SDataConfig[i], (g_sI2S0Config.eMode == AM_HAL_I2S_IO_MODE_MASTER));

                if (bI2S0to1Transfer && bI2S1to0Transfer)
                {
                    am_util_stdio_printf("TDM Loopback Iteration %2d PASSED!\n", g_ui32TestLoop);
                    if (g_ui32TestLoop >= 10)
                    {
                        am_hal_interrupt_master_disable();
                        NVIC_DisableIRQ(i2s_interrupts[I2S_MODULE_0]);
                        NVIC_DisableIRQ(i2s_interrupts[I2S_MODULE_1]);
                        am_hal_i2s_dma_transfer_complete(pI2S0Handle);
                        am_hal_i2s_dma_transfer_complete(pI2S1Handle);
                        i2s_deinit();
                        break;
                    }
                }
                else
                {
                    am_util_stdio_printf("---ERROR--- TDM Loopback Iteration %2d FAILED!\n", g_ui32TestLoop);
                    break;
                }

                //
                // The (n)th block of the buffer is completed; the upper code is for comparing them.
                // The (n+1)th block of the buffer is ongoing.
                // The (n+2)th block of the buffer will be prepared in the code below.
                //
                uint32_t* i2s0TxBufNext = (uint32_t*)am_hal_i2s_dma_get_buffer(pI2S0Handle, AM_HAL_I2S_XFER_TX);
                uint32_t* i2s1TxBufNext = (uint32_t*)am_hal_i2s_dma_get_buffer(pI2S1Handle, AM_HAL_I2S_XFER_TX);
                for (uint32_t i = 0; i < SIZE_SAMPLES; i++)
                {
                    i2s0TxBufNext[i] = rand();
                    i2s1TxBufNext[i] = rand();
                }
                am_hal_i2s_flush_tx_buffer((uint32_t)i2s0TxBufNext, SIZE_SAMPLES * 4);
                am_hal_i2s_flush_tx_buffer((uint32_t)i2s1TxBufNext, SIZE_SAMPLES * 4);
            }

            am_util_delay_us(100);
            ui32Timeout0P1Ms--;
            if (ui32Timeout0P1Ms == 0)
            {
                am_util_stdio_printf("---ERROR--- TDM Loopback Timeout.\n");
                break;
            }
        }

        am_util_delay_ms(100);
    }
    am_util_stdio_printf("Ran to End!\n");
    while (1)
    {
    }
}
