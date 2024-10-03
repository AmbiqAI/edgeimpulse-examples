//*****************************************************************************
//
//! @file iom_dma.c
//!
//! @brief iom dma operations.
//!
//! tips:
//! it's better to make sure that current peripheral's or module's DMA completed
//! then trigger next.
//! it's better to make sure that deinitialize peripherals before ending the test.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "multi_dma.h"

//*****************************************************************************
//
// Test defines
//
//*****************************************************************************
#define IOM_MODULE_TOTAL        8
#define PATTERN_BUF_SIZE        (4095)
#define IOM_TEST_FREQ           AM_HAL_IOM_16MHZ
#define IOM_WAIT_TIMEOUT_MS     (1000)

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
void                                   *g_IomDevHdl[IOM_MODULE_TOTAL];
void                                   *g_pIOMHandle[IOM_MODULE_TOTAL];
am_hal_iom_buffer(PATTERN_BUF_SIZE)     gPatternBuf;
am_hal_iom_buffer(PATTERN_BUF_SIZE)     gRxBuf[IOM_MODULE_TOTAL];

// Buffer for non-blocking transactions
uint32_t                                DMATCBBuffer[IOM_MODULE_TOTAL][1024];

am_hal_iom_config_t     g_sIomSpiCfg =
{
    .eInterfaceMode       = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq        = AM_HAL_IOM_16MHZ,
    .eSpiMode             = AM_HAL_IOM_SPI_MODE_0,
    .ui32NBTxnBufLength   = 0,
    .pNBTxnBuf = NULL,
};

volatile static bool g_bIomDmaFinish[IOM_MODULE_TOTAL] = {false, };

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
void
am_iomaster0_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[0], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[0], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[0], ui32Status);
        }
    }
}

void
am_iomaster1_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[1], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[1], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[1], ui32Status);
        }
    }
}

void
am_iomaster2_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[2], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[2], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[2], ui32Status);
        }
    }
}

void
am_iomaster3_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[3], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[3], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[3], ui32Status);
        }
    }
}

void
am_iomaster4_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[4], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[4], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[4], ui32Status);
        }
    }
}

void
am_iomaster5_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[5], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[5], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[5], ui32Status);
        }
    }
}

void
am_iomaster6_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[6], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[6], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[6], ui32Status);
        }
    }
}

void
am_iomaster7_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pIOMHandle[7], true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pIOMHandle[7], ui32Status);
            am_hal_iom_interrupt_service(g_pIOMHandle[7], ui32Status);
        }
    }
}

void
read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    uint32_t index = (uint32_t)pCallbackCtxt;

    if ( index >= IOM_MODULE_TOTAL )
    {
        am_util_stdio_printf("\nIOM ERROR index:%d!\n", index);
    }

    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nIOM%d: Write Failed 0x%x\n", index, transactionStatus);
    }
    else
    {
        g_bIomDmaFinish[index] = true;

#ifdef USE_DEBUG_PIN
        switch(index)
        {
            case 0:
                am_hal_gpio_output_clear(DEBUG_PIN_1);
                break;
            case 1:
                am_hal_gpio_output_clear(DEBUG_PIN_2);
                break;
            case 2:
                am_hal_gpio_output_clear(DEBUG_PIN_3);
                break;
            case 4:
                am_hal_gpio_output_clear(DEBUG_PIN_5);
                break;
            case 5:
                am_hal_gpio_output_clear(DEBUG_PIN_6);
                break;
            case 6:
                am_hal_gpio_output_clear(DEBUG_PIN_7);
                break;
            case 7:
                am_hal_gpio_output_clear(DEBUG_PIN_8);
                break;
            default:
                break;
        }
#endif
    }
}

//*****************************************************************************
//
// Write data in SPI mode.
//
//*****************************************************************************
static uint32_t
iom_slave_write(void* handle, uint32_t offset, uint32_t *pBuf,
                uint32_t size,
                am_hal_iom_callback_t pfnCallback,
                void *pCallbackCtxt)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui32InstrLen    = 1;
#if defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    Transaction.ui64Instr = offset;
#else
    Transaction.ui32Instr = offset;
#endif
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32TxBuffer   = pBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

    //
    // non-blocking transfer.
    //
    return am_hal_iom_nonblocking_transfer(handle,
                                           &Transaction,
                                           pfnCallback,
                                           pCallbackCtxt);
}
//*****************************************************************************
//
// deinitialize iom
//
//*****************************************************************************
bool
iom_deinit()
{
    uint32_t i;
    uint32_t ui32Status;
    for ( i = 0; i < IOM_MODULE_TOTAL; i++ )
    {
        //skip IOM3 as it shares some IO pins with mspi.
        if ( i == 3 )
        {
            continue;
        }

        IRQn_Type irq_no = ((IRQn_Type)(IOMSTR0_IRQn + i));
        NVIC_DisableIRQ(irq_no);

        //
        // Disable the IOM.
        //
        ui32Status = am_hal_iom_disable(g_pIOMHandle[i]);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to disable iom!\n");
            return false;
        }

        //
        // Configure the IOM pins.
        //
        am_bsp_iom_pins_disable(i, AM_HAL_IOM_SPI_MODE);
        //
        // power off iom
        //
        ui32Status = am_hal_iom_power_ctrl(g_pIOMHandle[i], AM_HAL_SYSCTRL_DEEPSLEEP, false);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to power off iom!\n");
            return false;
        }

        //
        // Uninitialize the interface and return the handle to a known state.
        //
        ui32Status = am_hal_iom_uninitialize(g_pIOMHandle[i]);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to Uninitialize iom!\n");
            return false;
        }
    }
    return true;
}

//*****************************************************************************
//
// iom set up.
//
//*****************************************************************************
bool
iom_set_up(uint32_t iomModule)
{
    uint32_t i = iomModule;
    uint32_t ui32Status;
    g_sIomSpiCfg.ui32ClockFreq = IOM_TEST_FREQ;
    g_sIomSpiCfg.pNBTxnBuf = &DMATCBBuffer[i][0];
    g_sIomSpiCfg.ui32NBTxnBufLength = sizeof(DMATCBBuffer) / IOM_MODULE_TOTAL / 4;

    //
    // Initialize the IOM.
    //
    ui32Status = am_hal_iom_initialize(iomModule, &g_pIOMHandle[i]);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize iom!\n");
        return false;
    }

    ui32Status = am_hal_iom_power_ctrl(g_pIOMHandle[i], AM_HAL_SYSCTRL_WAKE, false);

    //
    // Set the required configuration settings for the IOM.
    //
    ui32Status = am_hal_iom_configure(g_pIOMHandle[i], &g_sIomSpiCfg);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure iom!\n");
        return false;
    }
    //
    // Configure the IOM pins.
    //
    am_bsp_iom_pins_enable(iomModule, AM_HAL_IOM_SPI_MODE);

    //
    // Enable all the interrupts for the IOM module.
    //
    //am_hal_iom_InterruptEnable(g_pIOMHandle[i], 0xFF);

    //
    // Enable the IOM.
    //
    ui32Status = am_hal_iom_enable(g_pIOMHandle[i]);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable iom!\n");
        return false;
    }
    return true;
}

//*****************************************************************************
//
// Init all iom instances.
//
//*****************************************************************************
bool
iom_init(void)
{
    uint32_t i;

    for ( i = 0; i < IOM_MODULE_TOTAL; i++ )
    {
        //skip IOM3 as it shares some IO pins with mspi.
        if ( i == 3 )
        {
            continue;
        }

        g_bIomDmaFinish[i] = true;

        //
        // Enable the interrupt in the NVIC.
        //
        IRQn_Type irq_no = ((IRQn_Type)(IOMSTR0_IRQn + i));
        NVIC_SetPriority(irq_no, AM_IRQ_PRIORITY_DEFAULT);
        NVIC_ClearPendingIRQ(irq_no);
        NVIC_EnableIRQ(irq_no);
    }

    for (i = 0; i < PATTERN_BUF_SIZE; i++)
    {
        gPatternBuf.bytes[i] = i & 0xFF;
    }

    for ( i = 0; i < IOM_MODULE_TOTAL; i++ )
    {
        if ( i == 3 )
        {
            continue;
        }

        if(false == iom_set_up(i))
        {
            return false;
        }
    }

    return true;
}

//*****************************************************************************
//
// Start transfer.
//
//*****************************************************************************
bool
iom_start(void)
{
    uint32_t i;
    uint32_t ui32Status;

    for (i = 0; i < IOM_MODULE_TOTAL; i++)
    {
        //skip IOM3 as it shares some IO pins with mspi.
        if ( i == 3 )
        {
            continue;
        }
        if(g_bIomDmaFinish[i] == false)
        {
            continue;
        }

        g_bIomDmaFinish[i] = false;

        // Initiate non-blocking write
        ui32Status = iom_slave_write(g_pIOMHandle[i], 0, &gRxBuf[i].words[0], PATTERN_BUF_SIZE, read_complete, (void *)i);
        if ( ui32Status != 0 )
        {
            am_util_stdio_printf("IOM%d: Start read failed!\n", i);
            return false;
        }
#ifdef USE_DEBUG_PIN
        switch(i)
        {
            case 0:
                am_hal_gpio_output_set(DEBUG_PIN_1);
                break;
            case 1:
                am_hal_gpio_output_set(DEBUG_PIN_2);
                break;
            case 2:
                am_hal_gpio_output_set(DEBUG_PIN_3);
                break;
            case 4:
                am_hal_gpio_output_set(DEBUG_PIN_5);
                break;
            case 5:
                am_hal_gpio_output_set(DEBUG_PIN_6);
                break;
            case 6:
                am_hal_gpio_output_set(DEBUG_PIN_7);
                break;
            case 7:
                am_hal_gpio_output_set(DEBUG_PIN_8);
                break;
            default:
                break;
        }
#endif
    }

    return true;
}

//*****************************************************************************
//
// Wait IOM complete.
//
//*****************************************************************************
bool
iom_wait(void)
{
    uint32_t count = 0;

    while(((!g_bIomDmaFinish[0])
        || (!g_bIomDmaFinish[1])
        || (!g_bIomDmaFinish[2])
        || (!g_bIomDmaFinish[4])
        || (!g_bIomDmaFinish[5])
        || (!g_bIomDmaFinish[6])
        || (!g_bIomDmaFinish[7]))
        && (count < IOM_WAIT_TIMEOUT_MS))
    {
        am_util_delay_ms(1);
        count ++;
    }

    if(count >= IOM_WAIT_TIMEOUT_MS)
    {
        return false;
    }
    else
    {
        return true;
    }
}
