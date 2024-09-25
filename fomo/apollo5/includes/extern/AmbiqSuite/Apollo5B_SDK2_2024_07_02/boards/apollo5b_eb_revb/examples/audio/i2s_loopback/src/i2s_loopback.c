//*****************************************************************************
//
//! @file i2s_loopback.c
//!
//! @brief An example to show basic I2S operation.
//!
//! Purpose:
//! This example enables the I2S interfaces to loop back data from each other
//! and compares data from the I2S TX and data from the another I2S RX. Either
//! I2S0 or I2S1 can be selected as the master.
//! This example loops forever.
//!
//! I2S configurations:
//!  - 2 channels
//!  - 48 kHz sample rate
//!  - Standard I2S format
//!  - 32 bits word width
//!  - 24 bits bit-depth
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
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Example parameters.
//
//*****************************************************************************
#if defined(AM_PART_APOLLO5A)
#define CLOCK_SOURCE                (NCO)
#elif defined(AM_PART_APOLLO5B)
#define CLOCK_SOURCE                (PLL)
#endif
    #define HFRC                    (0)
    #define PLL                     (1)
    #define NCO                     (2)
    #define HF2ADJ                  (3)
#define I2S_MODULE_0                (0)
#define I2S_MODULE_1                (1)
#define NUM_OF_SAMPLES              (256)
#define USE_I2S_MASTER              I2S_MODULE_1
#define USE_SLEEP_MODE              (USE_SLEEP_MODE_DEEP)
    #define USE_SLEEP_MODE_NONE     (0)
    #define USE_SLEEP_MODE_NORMAL   (1)
    #define USE_SLEEP_MODE_DEEP     (2)
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
volatile uint32_t g_ui32I2SDmaCpl[5] =
{
    0, //I2S0 TX.
    0, //I2S0 RX.
    0, //I2S1 Tx.
    0, //I2S1 Rx.
    0  //Success or Fail.
};

//
// Used as the ping-pong buffer of DMA.
// Aligned to 32 bytes to meet data cache requirements.
//
AM_SHARED_RW uint32_t g_ui32I2S0RxDataBuffer[2 * NUM_OF_SAMPLES] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t g_ui32I2S0TxDataBuffer[2 * NUM_OF_SAMPLES] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t g_ui32I2S1RxDataBuffer[2 * NUM_OF_SAMPLES] __attribute__((aligned(32)));
AM_SHARED_RW uint32_t g_ui32I2S1TxDataBuffer[2 * NUM_OF_SAMPLES] __attribute__((aligned(32)));

//*****************************************************************************
//
// I2S configurations:
//  - 2 channels
//  - Standard I2S format
//  - 32 bits word width
//  - 24 bits bit-depth
//  - sample rate:
//    * HFRC   15625Hz
//    * PLL    48000Hz
//    * NCO    48000Hz
//    * HF2ADJ 48000Hz
//
//*****************************************************************************
static am_hal_i2s_io_signal_t g_sI2SIOConfig =
{
    .sFsyncPulseCfg = {
        .eFsyncPulseType = AM_HAL_I2S_FSYNC_PULSE_ONE_SUBFRAME,
    },
    .eFyncCpol = AM_HAL_I2S_IO_FSYNC_CPOL_LOW,
    .eTxCpol   = AM_HAL_I2S_IO_TX_CPOL_FALLING,
    .eRxCpol   = AM_HAL_I2S_IO_RX_CPOL_RISING
};

static am_hal_i2s_data_format_t g_sI2SDataConfig =
{
    .ePhase                   = AM_HAL_I2S_DATA_PHASE_SINGLE,
    .eChannelLenPhase1        = AM_HAL_I2S_FRAME_WDLEN_32BITS,
    .ui32ChannelNumbersPhase1 = 2,
    .eDataDelay               = 0x1,
    .eSampleLenPhase1         = AM_HAL_I2S_SAMPLE_LENGTH_24BITS,
    .eDataJust                = AM_HAL_I2S_DATA_JUSTIFIED_LEFT,
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
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC_3MHz,
    .eDiv3  = 0,
    #elif (CLOCK_SOURCE == PLL)
    .eClock = eAM_HAL_I2S_CLKSEL_PLL_FOUT4,
    .eDiv3  = 0,
    #elif (CLOCK_SOURCE == NCO)
    .eClock    = eAM_HAL_I2S_CLKSEL_NCO_HFRC_48MHz,
    .f64NcoDiv = 48.0 / 3.072,
    #elif (CLOCK_SOURCE == HF2ADJ)
    #if defined(AM_PART_APOLLO5A)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_2MHz,
    #elif defined(AM_PART_APOLLO5B)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_4MHz,
    #endif
    .eDiv3  = 0,
    #endif
    .eASRC  = 0,
    .eData  = &g_sI2SDataConfig,
    .eIO    = &g_sI2SIOConfig,
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
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC_3MHz,
    .eDiv3  = 0,
    #elif (CLOCK_SOURCE == PLL)
    .eClock = eAM_HAL_I2S_CLKSEL_PLL_FOUT4,
    .eDiv3  = 0,
    #elif (CLOCK_SOURCE == NCO)
    .eClock    = eAM_HAL_I2S_CLKSEL_NCO_HFRC_48MHz,
    .f64NcoDiv = 48.0 / 3.072,
    #elif (CLOCK_SOURCE == HF2ADJ)
    #if defined(AM_PART_APOLLO5A)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_2MHz,
    #elif defined(AM_PART_APOLLO5B)
    .eClock = eAM_HAL_I2S_CLKSEL_HFRC2_APPROX_4MHz,
    #endif
    .eDiv3  = 0,
    #endif
    .eASRC  = 0,
    .eData  = &g_sI2SDataConfig,
    .eIO    = &g_sI2SIOConfig,
};

//
// Ping pong buffer settings.
//
static am_hal_i2s_transfer_t sTransfer0 =
{
    .ui32RxTotalCount        = NUM_OF_SAMPLES,
    .ui32RxTargetAddr        = (uint32_t)(&g_ui32I2S0RxDataBuffer[0]),
    .ui32RxTargetAddrReverse = (uint32_t)(&g_ui32I2S0RxDataBuffer[NUM_OF_SAMPLES]),
    .ui32TxTotalCount        = NUM_OF_SAMPLES,
    .ui32TxTargetAddr        = (uint32_t)(&g_ui32I2S0TxDataBuffer[0]),
    .ui32TxTargetAddrReverse = (uint32_t)(&g_ui32I2S0TxDataBuffer[NUM_OF_SAMPLES]),
};

static am_hal_i2s_transfer_t sTransfer1 =
{
    .ui32RxTotalCount        = NUM_OF_SAMPLES,
    .ui32RxTargetAddr        = (uint32_t)(&g_ui32I2S1RxDataBuffer[0]),
    .ui32RxTargetAddrReverse = (uint32_t)(&g_ui32I2S1RxDataBuffer[NUM_OF_SAMPLES]),
    .ui32TxTotalCount        = NUM_OF_SAMPLES,
    .ui32TxTargetAddr        = (uint32_t)(&g_ui32I2S1TxDataBuffer[0]),
    .ui32TxTargetAddrReverse = (uint32_t)(&g_ui32I2S1TxDataBuffer[NUM_OF_SAMPLES]),
};

const am_hal_pwrctrl_sram_memcfg_t g_SRAMCfgKeepSRAMActive =
{
    .eSRAMCfg           = AM_HAL_PWRCTRL_SRAM_3M,
    .eActiveWithMCU     = AM_HAL_PWRCTRL_SRAM_3M,
    .eActiveWithGFX     = AM_HAL_PWRCTRL_SRAM_NONE,
    .eActiveWithDISP    = AM_HAL_PWRCTRL_SRAM_NONE,
    .eSRAMRetain        = AM_HAL_PWRCTRL_SRAM_3M
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
check_i2s_data(uint32_t rxtx_size, uint32_t* rx_databuf, uint32_t* tx_databuf)
{
    int i, index_0 = 0;

    //
    // Find the first element of Tx buffer in Rx buffer, and return the index.
    // Rx will delay N samples in full duplex mode.
    //
    for (i = 0; i < rxtx_size; i++)
    {
        if (rx_databuf[i] == tx_databuf[0])
        {
            index_0 = i;
            break;
        }
    }

    for (i = 0; i < (rxtx_size-index_0); i++)
    {
        if ( rx_databuf[i + index_0] != tx_databuf[i] )
        {
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

        // Configure and enable PLL.
        am_hal_syspll_config_t pllConfig = AM_HAL_SYSPLL_DEFAULT_CFG_AUDIO_48K_32BIT_2CH;
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
// Helper function to enter sleep
//
//*****************************************************************************
void helper_sleep()
{
    #if (USE_SLEEP_MODE > USE_SLEEP_MODE_NONE)
    #if defined(AM_PART_APOLLO5A)
    PWRCTRL->PWRCNTDEFVAL_b.PWRDEFVALDEVSTMC = 12;
    #endif

    // Disable Interrupt and check whether all expected interrupt has been
    // received. Only enter sleep if we are still expecting interrupt. This is
    // to prevent entering sleep after all expected interrupt has been received.
    uint32_t ui32Critical = am_hal_interrupt_master_disable();

    // Check for all expected interrupt
    if (g_ui32I2SDmaCpl[3] && g_ui32I2SDmaCpl[2] && g_ui32I2SDmaCpl[1] && g_ui32I2SDmaCpl[0])
    {
        // Recover master interrupt
        am_hal_interrupt_master_set(ui32Critical);

        // All expected interrupt is received. Return without sleep
        return;
    }

    #if (USE_SLEEP_MODE == USE_SLEEP_MODE_DEEP)
    //Wait for ITM to be idle and turn it off
    while (!am_hal_itm_print_not_busy() || !am_hal_itm_not_busy());
    am_bsp_itm_printf_disable();

    //Power Off ROM and OTP so that MCU can enter Deep Sleep
    am_hal_pwrctrl_rom_disable();
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_OTP);

    // Enter deep sleep
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

    // MCU Wakeup: re-enable itm printf
    am_bsp_itm_printf_enable();
    #else
    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);
    #endif

    // Recover master interrupt
    am_hal_interrupt_master_set(ui32Critical);

    #endif //(USE_SLEEP_MODE > USE_SLEEP_MODE_NONE)
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
    // Configure SRAM power setting for Deep Sleep
    //
    #if (defined(AM_PART_APOLLO5A) && (USE_SLEEP_MODE == USE_SLEEP_MODE_DEEP))
    am_hal_pwrctrl_sram_config((am_hal_pwrctrl_sram_memcfg_t *)&g_SRAMCfgKeepSRAMActive);
    #endif

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
    am_util_stdio_printf("I2S Full Duplex Loopback Test: Master = I2S0, Slave = I2S1.\n\n");
    #else
    am_util_stdio_printf("I2S Full Duplex Loopback Test: Master = I2S1, Slave = I2S0.\n\n");
    #endif

    //
    // Initialize data.
    //
    for (int i = 0; i < NUM_OF_SAMPLES; i++)
    {
        g_ui32I2S0TxDataBuffer[i]                  = (i & 0xFF) | 0xF50000;
        g_ui32I2S1TxDataBuffer[i]                  = (i & 0xFF) | 0xF50000;
        g_ui32I2S0TxDataBuffer[i + NUM_OF_SAMPLES] = (i & 0xFF) | 0x5F0000;
        g_ui32I2S1TxDataBuffer[i + NUM_OF_SAMPLES] = (i & 0xFF) | 0x5F0000;
    }
    am_hal_i2s_flush_tx_buffer((uint32_t)g_ui32I2S0TxDataBuffer, sizeof(g_ui32I2S0TxDataBuffer));
    am_hal_i2s_flush_tx_buffer((uint32_t)g_ui32I2S1TxDataBuffer, sizeof(g_ui32I2S1TxDataBuffer));

    i2s_init();
    am_hal_i2s_dma_configure(pI2S0Handle, &g_sI2S0Config, &sTransfer0);
    am_hal_i2s_dma_configure(pI2S1Handle, &g_sI2S1Config, &sTransfer1);

    NVIC_EnableIRQ(i2s_interrupts[I2S_MODULE_0]);
    NVIC_EnableIRQ(i2s_interrupts[I2S_MODULE_1]);
    am_hal_interrupt_master_enable();

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

    //
    // I2S DMACPL interrupts timeout value.
    //
    uint32_t ui32Timeout0P1Ms = DMACPL_TIMEOUT_MS / (0.1);

    //
    // Loop forever
    //
    while (1)
    {
        if (g_ui32I2SDmaCpl[0] && g_ui32I2SDmaCpl[1] && g_ui32I2SDmaCpl[2] && g_ui32I2SDmaCpl[3])
        {
            g_ui32TestLoop++;
            g_ui32I2SDmaCpl[0] = g_ui32I2SDmaCpl[1] = g_ui32I2SDmaCpl[2] = g_ui32I2SDmaCpl[3] = 0;

            uint32_t i2s0_rx_buffer = am_hal_i2s_dma_get_buffer(pI2S0Handle, AM_HAL_I2S_XFER_RX);
            uint32_t i2s1_rx_buffer = am_hal_i2s_dma_get_buffer(pI2S1Handle, AM_HAL_I2S_XFER_RX);
            uint32_t i2s0_tx_buffer = am_hal_i2s_dma_get_buffer(pI2S0Handle, AM_HAL_I2S_XFER_TX);
            uint32_t i2s1_tx_buffer = am_hal_i2s_dma_get_buffer(pI2S1Handle, AM_HAL_I2S_XFER_TX);

            if (check_i2s_data(NUM_OF_SAMPLES, (uint32_t*)i2s1_rx_buffer, (uint32_t*)i2s0_tx_buffer) && \
                check_i2s_data(NUM_OF_SAMPLES, (uint32_t*)i2s0_rx_buffer, (uint32_t*)i2s1_tx_buffer))
            {
                g_ui32I2SDmaCpl[4] = 1;
                am_util_stdio_printf("I2S Loopback Iteration %d PASSED!\n", g_ui32TestLoop);
            }
            else
            {
                am_util_stdio_printf("I2S Loopback Iteration %d FAILED!\n", g_ui32TestLoop);
            }

            ui32Timeout0P1Ms = DMACPL_TIMEOUT_MS / (0.1);
        }

        am_util_delay_us(100);
        ui32Timeout0P1Ms--;
        if (ui32Timeout0P1Ms == 0)
        {
            //
            // Disable interrupt.
            //
            NVIC_DisableIRQ(i2s_interrupts[I2S_MODULE_0]);
            NVIC_DisableIRQ(i2s_interrupts[I2S_MODULE_1]);
            am_hal_interrupt_master_disable();

            //
            // Disable DMA and deinitialize I2S.
            //
            am_hal_i2s_dma_transfer_complete(pI2S0Handle);
            am_hal_i2s_dma_transfer_complete(pI2S1Handle);
            i2s_deinit();

            am_util_stdio_printf("---ERROR--- I2S Loopback Timeout.\n");
            return 0;
        }

        // Enter Sleep Mode selected
        helper_sleep();
    }
}
