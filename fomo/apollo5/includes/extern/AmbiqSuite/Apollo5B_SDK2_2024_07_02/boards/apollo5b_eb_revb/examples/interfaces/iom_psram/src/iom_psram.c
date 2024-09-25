//*****************************************************************************
//
//! @file iom_psram.c
//!
//! @brief Example that demostrates IOM, connecting to a SPI PSRAM
//! PSRAM is initialized with a known pattern data using Blocking IOM Write.
//! This example starts a 1 second timer. At each 1 second period, it initiates
//! reading a fixed size block from the PSRAM device using Non-Blocking IOM
//! Read, and comparing againts the predefined pattern
//!
//! Define one of PSRAM_DEVICE_ macros to select the PSRAM device
//!
//! SWO is configured in 1M baud, 8-n-1 mode.
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
#include "am_devices_spipsram.h"

//*****************************************************************************
// Customize the following for the test
//*****************************************************************************
// Control the example execution
#define PSRAM_IOM_MODULE        2
#define PATTERN_BUF_SIZE        128
#define NUM_ITERATIONS          16

//*****************************************************************************
//*****************************************************************************

#if defined APS6404L
#define PSRAM_DEVICE_ID          AM_DEVICES_SPIPSRAM_KGD_PASS
#define am_iom_test_devices_t    am_devices_spipsram_config_t
#else
#error "Unknown PSRAM Device"
#endif

#define IOM_INTERRUPT1(n)       AM_HAL_INTERRUPT_IOMASTER ## n
#define IOM_INTERRUPT(n)        IOM_INTERRUPT1(n)
#define PSRAM_IOM_IRQn          ((IRQn_Type)(IOMSTR0_IRQn + PSRAM_IOM_MODULE))
#define IOM_TEST_FREQ           AM_HAL_IOM_16MHZ

//
// Typedef - to encapsulate device driver functions
//
typedef struct
{
    uint8_t  devName[20];
    uint32_t (*psram_init)(uint32_t ui32Module, am_iom_test_devices_t *pDevConfig, void **ppHandle, void **ppIomHandle);
    uint32_t (*psram_term)(void *pHandle);

    uint32_t (*psram_read_id)(void *pHandle, uint32_t *pDeviceID);

    uint32_t (*psram_nonblocking_write)(void *pHandle, uint8_t *ui8TxBuffer,
                                uint32_t ui32WriteAddress,
                                uint32_t ui32NumBytes,
                                am_hal_iom_callback_t pfnCallback,
                                void *pCallbackCtxt);

    uint32_t (*psram_nonblocking_read)(void *pHandle, uint8_t *pui8RxBuffer,
                                                      uint32_t ui32ReadAddress,
                                                      uint32_t ui32NumBytes,
                                                      am_hal_iom_callback_t pfnCallback,
                                                      void *pCallbackCtxt);
} psram_device_func_t;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
volatile bool                           g_bReadPsram = false;
volatile bool                           g_bVerifyReadData = false;
void                                    *g_IomDevHdl;
void                                    *g_pIOMHandle;
AM_SHARED_RW am_hal_iom_buffer(PATTERN_BUF_SIZE)     gPatternBuf __attribute__((aligned(32)));   //algined 32 byte to match a cache line
AM_SHARED_RW am_hal_iom_buffer(PATTERN_BUF_SIZE)     gRxBuf __attribute__((aligned(32)));   //algined 32 byte to match a cache line

// Buffer for non-blocking transactions
AM_SHARED_RW uint32_t                                DMATCBBuffer[4096];
psram_device_func_t device_func =
{
#if defined APS6404L
    // Fireball installed SPI PSRAM device
    .devName = "SPI PSRAM APS6404L",
    .psram_init = am_devices_spipsram_init,
    .psram_term = am_devices_spipsram_term,
    .psram_read_id = am_devices_spipsram_read_id,
    .psram_nonblocking_write = am_devices_spipsram_nonblocking_write,
    .psram_nonblocking_read = am_devices_spipsram_nonblocking_read,
#else
#error "Unknown PSRAM Device"
#endif
};

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

//*****************************************************************************
//
// Function to initialize Timer0 to interrupt every 1/2 second.
//
//*****************************************************************************
void
timer_init(void)
{
    am_hal_timer_config_t       TimerConfig;

    //
    // Set up TIMER0
    //
    am_hal_timer_default_config_set(&TimerConfig);
    TimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    TimerConfig.ui32PatternLimit = 0;
    TimerConfig.ui32Compare1 = 6000000;  // Default Clock is HFRC/16 or 6MHz.  1 sec.
    if (AM_HAL_STATUS_SUCCESS != am_hal_timer_config(0, &TimerConfig))
    {
      am_util_stdio_printf("Failed to configure TIMER0\n");
    }
    am_hal_timer_clear(0);

    //
    // Clear the timer Interrupt
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));

} // timer_init()

//*****************************************************************************
//
// Timer Interrupt Service Routine (ISR)
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
    //
    // Clear TimerA0 Interrupt (write to clear).
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
    am_hal_timer_clear(0);
    g_bReadPsram = true;

} // am_ctimer_isr()

//
// Take over the interrupt handler for whichever IOM we're using.
//
#define psram_iom_isr                                                          \
    am_iom_isr1(PSRAM_IOM_MODULE)
#define am_iom_isr1(n)                                                        \
    am_iom_isr(n)
#define am_iom_isr(n)                                                         \
    am_iomaster ## n ## _isr

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
//
//! Take over default ISR. (Queue mode service)
//
void psram_iom_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle, ui32Status);
        }
    }
}

void
init_pattern(void)
{
    am_hal_cachectrl_range_t sRange;

    uint32_t i;
    for (i = 0; i < PATTERN_BUF_SIZE; i++)
    {
        gPatternBuf.bytes[i] = i & 0xFF;
    }
    sRange.ui32StartAddr = (uint32_t)gPatternBuf.bytes;
    sRange.ui32Size = PATTERN_BUF_SIZE;
    am_hal_cachectrl_dcache_clean(&sRange);
}

int
psram_init(void)
{
    uint32_t ui32Status;

    // Set up IOM
    am_iom_test_devices_t stPsramConfig;
    stPsramConfig.ui32ChipSelectNum = 0;
    stPsramConfig.ui32ClockFreq = IOM_TEST_FREQ;
    stPsramConfig.pNBTxnBuf = DMATCBBuffer;
    stPsramConfig.ui32NBTxnBufLength = sizeof(DMATCBBuffer) / 4;
    // Initialize the Device
    ui32Status = device_func.psram_init(PSRAM_IOM_MODULE, &stPsramConfig, &g_IomDevHdl, &g_pIOMHandle);
    if (0 == ui32Status)
    {
        am_util_stdio_printf("%s Found\n", device_func.devName);
        // Set up a pattern data in PSRAM memory
        am_util_stdio_printf("Setting up data pattern in PSRAM using nonblocking write\n");
        return device_func.psram_nonblocking_write(g_IomDevHdl, &gPatternBuf.bytes[0], 0, PATTERN_BUF_SIZE, NULL, NULL);
    }
    else
    {
        return -1;
    }
}

void
read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    am_hal_cachectrl_range_t sRange;

    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nPSRAM Read Failed 0x%x\n", transactionStatus);
    }
    else
    {
        am_util_stdio_printf(".");
        g_bVerifyReadData = true;
    }
    sRange.ui32StartAddr = (uint32_t)gRxBuf.bytes;
    sRange.ui32Size = PATTERN_BUF_SIZE;
    am_hal_cachectrl_dcache_invalidate(&sRange, false);
}

int
verify_psram_data(void)
{
    uint32_t i;
    g_bVerifyReadData = false;
    // Verify Read PSRAM data
    for (i = 0; i < PATTERN_BUF_SIZE; i++)
    {
        if (gPatternBuf.bytes[i] != gRxBuf.bytes[i])
        {
            am_util_stdio_printf("Receive Data Compare failed at offset %d - Expected = 0x%x, Received = 0x%x\n",
                i, gPatternBuf.bytes[i], gRxBuf.bytes[i]);
            return -1;
        }
    }
    return 0;
}

void
read_psram(void)
{
    uint32_t ui32Status;
    // Initiate read of a block of data from PSRAM
    ui32Status = device_func.psram_nonblocking_read(g_IomDevHdl, &gRxBuf.bytes[0], 0, PATTERN_BUF_SIZE, read_complete, 0);
    if (ui32Status == 0)
    {
        g_bReadPsram = false;
    }
}

//*****************************************************************************
//
// Deinit the PSRAM driver.
//
//*****************************************************************************
void
psram_term(void)
{
    device_func.psram_term(g_IomDevHdl);
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    int iRet;
    uint32_t numRead = 0;

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
    // Enable the ITM print interface.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("IOM PSRAM Example\n");

#if 0
    am_hal_gpio_out_bit_clear(42);
    am_hal_gpio_pin_config(42, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(43);
    am_hal_gpio_pin_config(43, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(49);
    am_hal_gpio_pin_config(49, AM_HAL_PIN_OUTPUT);
#endif

    //
    // TimerA0 init.
    //
    timer_init();

   //
    // Enable the timer Interrupt.
    //
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(TIMER_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(TIMER_IRQn);
    am_hal_interrupt_master_enable();

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_ClearPendingIRQ(PSRAM_IOM_IRQn);
    NVIC_EnableIRQ(PSRAM_IOM_IRQn);

    init_pattern();

    //
    // Initialize the PSRAM Device
    //
    iRet = psram_init();
    if (iRet)
    {
        am_util_stdio_printf("Unable to initialize PSRAM\n");
        while(1);
    }

    //
    // Start timer 0
    //
    am_hal_timer_start(0);

    am_util_stdio_printf("Periodically Reading data from PSRAM using non-blocking read - %d times\n", NUM_ITERATIONS);
    //
    // Loop forever.
    //
    while(1)
    {
        //
        // Disable interrupt while we decide whether we're going to sleep.
        //
        uint32_t ui32IntStatus = am_hal_interrupt_master_disable();

        if (!g_bReadPsram && !g_bVerifyReadData)
        {
            // Wait for Baud rate detection
            am_hal_sysctrl_sleep(true);
            //
            // Enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
        }
        else if (g_bReadPsram)
        {
            //
            // Enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
            read_psram();
        }
        else if (g_bVerifyReadData)
        {
            //
            // Enable interrupts
            //
            am_hal_interrupt_master_set(ui32IntStatus);
            verify_psram_data();
            if (++numRead >= NUM_ITERATIONS)
            {
                am_util_stdio_printf("\n%d Reads done\n", NUM_ITERATIONS);
                break;
            }
        }
    }
    // Cleanup
    am_util_stdio_printf("\nEnd of PSRAM Example\n");

    //
    // Disable the timer Interrupt.
    //
    am_hal_timer_interrupt_disable(0);

    //
    // disable the interrupts in the NVIC.
    //
    NVIC_DisableIRQ(TIMER_IRQn);
    NVIC_DisableIRQ(PSRAM_IOM_IRQn);
    psram_term();
    while(1);
}

