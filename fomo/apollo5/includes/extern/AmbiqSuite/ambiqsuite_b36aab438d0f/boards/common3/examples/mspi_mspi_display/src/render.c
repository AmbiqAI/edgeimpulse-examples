//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "mspi_mspi_display.h"

//*****************************************************************************
//
// MSPI <==> MSPI Handshake Signals
//
//*****************************************************************************
#define MSPI_SIGNAL_SOURCE_BUFFER0      (MSPI0_CQFLAGS_CQFLAGS_SWFLAG0 << 8)
#define MSPI_SIGNAL_SOURCE_BUFFER1      (MSPI0_CQFLAGS_CQFLAGS_SWFLAG1 << 8)
#define MSPI_SIGNAL_SINK_BUFFER0        (MSPI0_CQFLAGS_CQFLAGS_SWFLAG0 << 8)
#define MSPI_SIGNAL_SINK_BUFFER1        (MSPI0_CQFLAGS_CQFLAGS_SWFLAG1 << 8)

#define MSPI_WAIT_FOR_SOURCE_BUFFER0    (_VAL2FLD(MSPI0_CQPAUSE_CQMASK, MSPI0_CQPAUSE_CQMASK_BUF0XOREN))
#define MSPI_WAIT_FOR_SOURCE_BUFFER1    (_VAL2FLD(MSPI0_CQPAUSE_CQMASK, MSPI0_CQPAUSE_CQMASK_BUF1XOREN))
#define MSPI_WAIT_FOR_SINK_BUFFER0      (_VAL2FLD(MSPI0_CQPAUSE_CQMASK, MSPI0_CQPAUSE_CQMASK_IOM0READY))
#define MSPI_WAIT_FOR_SINK_BUFFER1      (_VAL2FLD(MSPI0_CQPAUSE_CQMASK, MSPI0_CQPAUSE_CQMASK_IOM1READY))

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
volatile uint32_t     g_actFb = PSRAM_ACTFB1_OFFSET;
volatile bool         g_bDisplayDone = false;
volatile bool         g_bTEInt = false;
void                  *g_MSPIDisplayHandle;
void                  *g_DisplayHandle;

//*****************************************************************************
//
// Local Variables
//
//*****************************************************************************
// Buffer for non-blocking transactions for Display MSPI - Needs to be big enough to accomodate
// all the transactions
static uint32_t        g_MspiDisplayQBuffer[(AM_HAL_MSPI_CQ_ENTRY_SIZE / 4) * (NUM_FRAGMENTS + 1)];
// Temp Buffer in SRAM to read PSRAM data to, and write DISPLAY data from
uint32_t        g_TempBuf[2][TEMP_BUFFER_SIZE / 4];

// Display MSPI configuration
am_devices_mspi_rm67162_config_t SerialDisplayMSPICfg =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
    .ui32NBTxnBufLength       = sizeof(g_MspiDisplayQBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_MspiDisplayQBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type mspi_display_interrupts[] =
{
    MSPI0_IRQn,
#if defined(AM_PART_APOLLO3P)
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define display_mspi_isr                                                          \
    am_mspi_isr1(DISPLAY_MSPI_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void display_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPIDisplayHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPIDisplayHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPIDisplayHandle, ui32Status);
}

// TE ISR handler
static void
teInt_handler(void)
{
    static uint32_t count = 0;
    if (g_bDisplayDone && g_bNewDisplay)
    {
        if (++count == TE_DELAY)
        {
            g_bTEInt = true;
            count = 0;
            g_actFb = (g_actFb == PSRAM_ACTFB1_OFFSET) ? PSRAM_ACTFB2_OFFSET : PSRAM_ACTFB1_OFFSET;
        }
    }
}

// Initialization
uint32_t
display_init(void)
{
    uint32_t ui32Status;

    NVIC_SetPriority(DISPLAY_MSPI_IRQn, 0x4);


    // Initialize the MSPI Display
    ui32Status = am_devices_mspi_rm67162_init(DISPLAY_MSPI_MODULE, &SerialDisplayMSPICfg, &g_DisplayHandle, &g_MSPIDisplayHandle);
    if (AM_DEVICES_RM67162_STATUS_SUCCESS != ui32Status)
    {
      DEBUG_PRINT("Failed to init Display device\n");
    }
    NVIC_EnableIRQ(mspi_display_interrupts[DISPLAY_MSPI_MODULE]);

    am_hal_interrupt_master_enable();
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISPLAY_TE, g_AM_BSP_GPIO_DISPLAY_TE);
    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_interrupt_clear( AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_DISPLAY_TE));
    am_hal_gpio_interrupt_register(AM_BSP_GPIO_DISPLAY_TE, teInt_handler);
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_DISPLAY_TE));
    NVIC_EnableIRQ(GPIO_IRQn);

  return ui32Status;
}

// Callbacks
static void
psram_read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
//    DEBUG_GPIO_HIGH(DBG2_GPIO);
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        DEBUG_PRINT("\nMSPI PSRAM Read Failed 0x%x\n", transactionStatus);
    }
    else
    {
        //DEBUG_PRINT("\nMSPI PSRAM Read Done 0x%x\n", transactionStatus);
    }
//    DEBUG_GPIO_LOW(DBG2_GPIO);
}


static void
display_write_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
//    DEBUG_GPIO_HIGH(DBG2_GPIO);
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        DEBUG_PRINT("\nMSPI Display Write Failed 0x%x\n", transactionStatus);
    }
    else
    {
        //DEBUG_PRINT("\nMSPI Display Write Done 0x%x\n", transactionStatus);
        g_bDisplayDone = true;
//        DEBUG_GPIO_LOW(DBG2_GPIO);
    }
}

#ifdef CQ_RAW
// Jump - by reprogramming the CQADDR
typedef struct
{
    uint32_t    ui32CQAddrAddr;
    uint32_t    ui32CQAddrVal;
} am_hal_cq_jmp_t;

//
// Command Queue entry structure for DMA transfer.
//
typedef struct
{
    uint32_t                    ui32DMATARGADDRAddr;
    uint32_t                    ui32DMATARGADDRVal;
    uint32_t                    ui32DMADEVADDRAddr;
    // DEVADDR is don't care for display
    uint32_t                    ui32DMADEVADDRVal;
    uint32_t                    ui32DMATOTCOUNTAddr;
    uint32_t                    ui32DMATOTCOUNTVal;
    uint32_t                    ui32DMACFG1Addr;
    uint32_t                    ui32DMACFG1Val;
    // Need to insert couple of Dummy's
    uint32_t                    ui32DummyAddr1;
    uint32_t                    ui32DummyVal1;
    uint32_t                    ui32DummyAddr2;
    uint32_t                    ui32DummyVal2;
    // Need to disable the DMA to prepare for next reconfig
    // Need to have this following the DMAEN for CMDQ
    uint32_t                    ui32DMACFG2Addr;
    uint32_t                    ui32DMACFG2Val;
} mspi_cq_dma_entry_core_t;

//
// Command Queue entry structure for DMA transfer.
//
typedef struct
{
    uint32_t                    ui32PAUSENAddr;
    uint32_t                    ui32PAUSEENVal;
    uint32_t                    ui32PAUSEN2Addr;
    uint32_t                    ui32PAUSEEN2Val;
    mspi_cq_dma_entry_core_t    core;
    uint32_t                    ui32SETCLRAddr;
    uint32_t                    ui32SETCLRVal;
} mspi_cq_dma_entry_t;

typedef struct
{
    // TODO: Need to take care of alignment restrictions
    mspi_cq_dma_entry_t       block[NUM_FRAGMENTS + 1];
    am_hal_cq_jmp_t           jmpOut; // Programmable address to jump back to the original CQ
} mspi_long_txn_t;

mspi_long_txn_t gMspiDisplayCQ;
mspi_long_txn_t gMspiPsramCQ;

// MSPI
// One time initialization
void mspi_init_cq_long(uint32_t ui32Module, uint8_t ui8Priority, mspi_long_txn_t *pMspiLong, bool bSource, uint32_t blockSize)
{
    uint32_t ui32DmaCfg;
    if (bSource)
    {
        ui32DmaCfg =
            _VAL2FLD(MSPI0_DMACFG_DMAPWROFF, 0)   |  // DMA Auto Power-off not supported!
            _VAL2FLD(MSPI0_DMACFG_DMAPRI, ui8Priority)    |
            _VAL2FLD(MSPI0_DMACFG_DMADIR, AM_HAL_MSPI_RX)     |
            _VAL2FLD(MSPI0_DMACFG_DMAEN, 3);
    }
    else
    {
        ui32DmaCfg =
            _VAL2FLD(MSPI0_DMACFG_DMAPWROFF, 0)   |  // DMA Auto Power-off not supported!
            _VAL2FLD(MSPI0_DMACFG_DMAPRI, ui8Priority)    |
            _VAL2FLD(MSPI0_DMACFG_DMADIR, AM_HAL_MSPI_TX)     |
            _VAL2FLD(MSPI0_DMACFG_DMAEN, 3);
    }
    // Initialize the sequence blocks
    for (uint32_t i = 0; i < (NUM_FRAGMENTS + 1); i++)
    {
        pMspiLong->block[i].ui32PAUSENAddr = (uint32_t)&MSPIn(ui32Module)->CQPAUSE;
        pMspiLong->block[i].ui32PAUSEN2Addr = (uint32_t)&MSPIn(ui32Module)->CQPAUSE;
        pMspiLong->block[i].ui32SETCLRAddr = (uint32_t)&MSPIn(ui32Module)->CQSETCLEAR;
        pMspiLong->block[i].core.ui32DMATARGADDRAddr = (uint32_t)&MSPIn(ui32Module)->DMATARGADDR;
        pMspiLong->block[i].core.ui32DMADEVADDRAddr = (uint32_t)&MSPIn(ui32Module)->DMADEVADDR;
        pMspiLong->block[i].core.ui32DMATOTCOUNTAddr = (uint32_t)&MSPIn(ui32Module)->DMATOTCOUNT;
        pMspiLong->block[i].core.ui32DMACFG1Addr = (uint32_t)&MSPIn(ui32Module)->DMACFG;
        pMspiLong->block[i].core.ui32DMACFG2Addr = (uint32_t)&MSPIn(ui32Module)->DMACFG;
        pMspiLong->block[i].core.ui32DummyAddr1 = (uint32_t)&MSPIn(ui32Module)->CQSETCLEAR;
        pMspiLong->block[i].core.ui32DummyAddr2 = (uint32_t)&MSPIn(ui32Module)->CQSETCLEAR;
        // Pause Conditions
        // This is the Pause Boundary for HiPrio transactions
        if (bSource)
        {
            pMspiLong->block[i].ui32PAUSEENVal = AM_HAL_MSPI_CQP_PAUSE_DEFAULT | ((i % 2) ? MSPI_WAIT_FOR_SINK_BUFFER1 : MSPI_WAIT_FOR_SINK_BUFFER0);
            pMspiLong->block[i].ui32SETCLRVal = (i % 2) ? MSPI_SIGNAL_SINK_BUFFER1 : MSPI_SIGNAL_SINK_BUFFER0;
        }
        else
        {
            pMspiLong->block[i].ui32PAUSEENVal = AM_HAL_MSPI_CQP_PAUSE_DEFAULT | ((i % 2) ? MSPI_WAIT_FOR_SOURCE_BUFFER1 : MSPI_WAIT_FOR_SOURCE_BUFFER0);
            pMspiLong->block[i].ui32SETCLRVal = (i % 2) ? MSPI_SIGNAL_SOURCE_BUFFER1 : MSPI_SIGNAL_SOURCE_BUFFER0;
        }
        pMspiLong->block[i].core.ui32DMATOTCOUNTVal = blockSize;
        pMspiLong->block[i].core.ui32DMATARGADDRVal = (i % 2) ? (uint32_t)&g_TempBuf[1] : (uint32_t)&g_TempBuf[0];
        pMspiLong->block[i].core.ui32DMACFG1Val = ui32DmaCfg;
        pMspiLong->block[i].core.ui32DMACFG2Val = _VAL2FLD(MSPI0_DMACFG_DMAEN, 0);
        pMspiLong->block[i].core.ui32DummyVal1 = 0;
        pMspiLong->block[i].core.ui32DummyVal2 = 0;
        pMspiLong->block[i].core.ui32DMADEVADDRVal = 0;
        pMspiLong->block[i].ui32PAUSEEN2Val = AM_HAL_MSPI_PAUSE_DEFAULT;
    }
    pMspiLong->block[0].core.ui32DMATOTCOUNTVal = blockSize / 2;
    pMspiLong->block[1].core.ui32DMATOTCOUNTVal = blockSize / 2;
    pMspiLong->jmpOut.ui32CQAddrAddr = (uint32_t)&MSPIn(ui32Module)->CQADDR;
}

static void
update_mspi_mspi_transaction(uint32_t blockSize,
                             uint32_t ui32DevAddr,
                             mspi_long_txn_t *pMspiSrc,
                             mspi_long_txn_t *pMspiSink,
                             uint32_t ui32NumBytes)
{
    uint32_t headerSize = ui32DevAddr & 0x3;
    pMspiSrc->block[0].core.ui32DMADEVADDRVal = ui32DevAddr;
    if (headerSize)
    {
        headerSize = 4 - headerSize;
        // Special treatment needed as the address is not word aligned
        // We split the transaction in two - first just for the preceding bytes, and remaining in second one
        pMspiSrc->block[0].core.ui32DMATOTCOUNTVal = headerSize;
        pMspiSink->block[0].core.ui32DMATOTCOUNTVal = headerSize;
        ui32DevAddr += headerSize;
        pMspiSrc->block[1].core.ui32DMADEVADDRVal = ui32DevAddr;
        pMspiSrc->block[1].core.ui32DMATOTCOUNTVal = blockSize;
        pMspiSink->block[1].core.ui32DMATOTCOUNTVal = blockSize;
    }
    else
    {
        pMspiSrc->block[0].core.ui32DMATOTCOUNTVal = blockSize / 2;
        pMspiSink->block[0].core.ui32DMATOTCOUNTVal = blockSize / 2;
        pMspiSrc->block[1].core.ui32DMADEVADDRVal = ui32DevAddr + blockSize / 2;
        pMspiSrc->block[1].core.ui32DMATOTCOUNTVal = blockSize / 2;
        pMspiSink->block[1].core.ui32DMATOTCOUNTVal = blockSize / 2;
    }
    for (uint32_t i = 2; i < (NUM_FRAGMENTS + 1); i++)
    {
        pMspiSrc->block[i].core.ui32DMADEVADDRVal = ui32DevAddr + (i - 1) * blockSize;
    }
    // Initialize the count & command for tail
    if ((ui32NumBytes - headerSize) > (blockSize * (NUM_FRAGMENTS - 1)))
    {
        pMspiSrc->block[NUM_FRAGMENTS].core.ui32DMATOTCOUNTVal  = ui32NumBytes - headerSize - blockSize*(NUM_FRAGMENTS-1);
        pMspiSink->block[NUM_FRAGMENTS].core.ui32DMATOTCOUNTVal  = ui32NumBytes - headerSize - blockSize*(NUM_FRAGMENTS-1);
        pMspiSrc->block[NUM_FRAGMENTS-1].core.ui32DMATOTCOUNTVal = blockSize;
    }
    else
    {
        // Need to make sure there is non-zero last element
        // Adjust the second last
        pMspiSrc->block[NUM_FRAGMENTS-1].core.ui32DMATOTCOUNTVal = blockSize - 4;
        pMspiSrc->block[NUM_FRAGMENTS].core.ui32DMADEVADDRVal -= 4;
        pMspiSrc->block[NUM_FRAGMENTS].core.ui32DMATOTCOUNTVal  = ui32NumBytes - headerSize - blockSize*(NUM_FRAGMENTS-1) + 4;
        pMspiSink->block[NUM_FRAGMENTS].core.ui32DMATOTCOUNTVal  = ui32NumBytes - headerSize - blockSize*(NUM_FRAGMENTS-1) + 4;
    }
}

#endif // CQ_RAW
// Initialization
uint32_t
init_mspi_mspi_xfer(void)
{
    uint32_t ui32Status = 0;
    uint32_t      u32Arg;
    // Clear flags
    u32Arg = AM_HAL_MSPI_SC_CLEAR(0xFF) & ~AM_HAL_MSPI_SC_RESV_MASK;  // clear all flags
    am_hal_mspi_control(g_MSPIPsramHandle, AM_HAL_MSPI_REQ_FLAG_SETCLR, &u32Arg);
    am_hal_mspi_control(g_MSPIDisplayHandle, AM_HAL_MSPI_REQ_FLAG_SETCLR, &u32Arg);
    // Link MSPI Source and Sink instances.
    u32Arg = DISPLAY_MSPI_MODULE;
    am_hal_mspi_control(g_MSPIPsramHandle, AM_HAL_MSPI_REQ_LINK_MSPI, &u32Arg);
    u32Arg = PSRAM_MSPI_MODULE;
    am_hal_mspi_control(g_MSPIDisplayHandle, AM_HAL_MSPI_REQ_LINK_MSPI, &u32Arg);
    ui32Status = am_devices_mspi_rm67162_set_transfer_window(g_DisplayHandle, 0, 0, ROW_NUM - 1, COLUMN_NUM - 1);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to set transfer window\n");
        return ui32Status;
    }
#ifdef CQ_RAW
    mspi_init_cq_long(PSRAM_MSPI_MODULE, 1, &gMspiPsramCQ, true, TEMP_BUFFER_SIZE);
    mspi_init_cq_long(DISPLAY_MSPI_MODULE, 1, &gMspiDisplayCQ, false, TEMP_BUFFER_SIZE);
#endif
    return ui32Status;
}

// Rendering
uint32_t
start_mspi_mspi_xfer(uint32_t psramOffset, uint32_t ui32NumBytes)
{
    uint32_t      ui32Status = 0;
    am_hal_mspi_callback_t  mspiSourceCb = 0;
    am_hal_mspi_callback_t  mspiSinkCb = 0;

    //DEBUG_GPIO_HIGH(DBG1_GPIO);
    mspiSourceCb = psram_read_complete;
    mspiSinkCb = display_write_complete;
    DEBUG_PRINT("\nInitiating MSPI -> MSPI Transfer\n");
#ifdef CONFIG_DISPLAY_WINDOW
    ui32Status = am_devices_mspi_rm67162_set_transfer_window(g_DisplayHandle, 0, 0, ROW_NUM - 1, COLUMN_NUM - 1);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to set transfer window\n");
        return ui32Status;
    }
#endif

#ifdef CQ_RAW
    // Queue up the CQ Raw
    am_hal_cmdq_entry_t jump;
    am_hal_mspi_cq_raw_t rawMspiCfg;

    rawMspiCfg.ui32PauseCondition = 0;
    rawMspiCfg.ui32StatusSetClr = 0;
    rawMspiCfg.pCQEntry = &jump;
    rawMspiCfg.numEntries = sizeof(am_hal_cmdq_entry_t) / 8;
    rawMspiCfg.pCallbackCtxt = 0;

    update_mspi_mspi_transaction(TEMP_BUFFER_SIZE,
                                  psramOffset,
                                  &gMspiPsramCQ,
                                  &gMspiDisplayCQ,
                                  ui32NumBytes);

    rawMspiCfg.pfnCallback = mspiSourceCb;
    rawMspiCfg.pJmpAddr = &gMspiPsramCQ.jmpOut.ui32CQAddrVal;
    jump.address = (uint32_t)&MSPIn(PSRAM_MSPI_MODULE)->CQADDR;
    jump.value = (uint32_t)&gMspiPsramCQ;

    ui32Status = am_hal_mspi_control(g_MSPIPsramHandle, AM_HAL_MSPI_REQ_CQ_RAW, &rawMspiCfg);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to queue up MSPI Read transaction\n");
        while(1);
    }

    rawMspiCfg.pfnCallback = mspiSinkCb;
    rawMspiCfg.pJmpAddr = &gMspiDisplayCQ.jmpOut.ui32CQAddrVal;
    jump.address = (uint32_t)&MSPIn(DISPLAY_MSPI_MODULE)->CQADDR;
    jump.value = (uint32_t)&gMspiDisplayCQ;

    ui32Status = am_hal_mspi_control(g_MSPIDisplayHandle, AM_HAL_MSPI_REQ_CQ_RAW, &rawMspiCfg);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to queue up MSPI Write transaction\n");
        while(1);
    }
#else
    // Queue up Display Writes and PSRAM Reads
    for (uint32_t address = 0, bufIdx = 0; address < ui32NumBytes; address += TEMP_BUFFER_SIZE, bufIdx++)
    {
        uint32_t bufOdd = bufIdx % 2;
        ui32Status = am_devices_mspi_psram_read_adv(g_PsramHandle,
                                                    (uint8_t *)g_TempBuf[bufOdd],
                                                    psramOffset + address,
                                                    (((address + TEMP_BUFFER_SIZE) >= ui32NumBytes) ? (ui32NumBytes - address) : TEMP_BUFFER_SIZE),
                                                    (bufOdd ? MSPI_WAIT_FOR_SINK_BUFFER1 : MSPI_WAIT_FOR_SINK_BUFFER0),
                                                    (bufOdd ? MSPI_SIGNAL_SINK_BUFFER1 : MSPI_SIGNAL_SINK_BUFFER0),
                                                    (((address + TEMP_BUFFER_SIZE) >= ui32NumBytes) ? mspiSourceCb : 0),
                                                    0);
        if (ui32Status)
        {
            DEBUG_PRINT("\nFailed to queue up MSPI Read transaction\n");
            break;
        }
        ui32Status = am_devices_rm67162_nonblocking_write_adv(g_DisplayHandle,
                                                   (uint8_t *)g_TempBuf[bufOdd],
                                                   (((address + TEMP_BUFFER_SIZE) >= ui32NumBytes) ? (ui32NumBytes - address) : TEMP_BUFFER_SIZE),
                                                   (bufOdd ? MSPI_WAIT_FOR_SOURCE_BUFFER1 : MSPI_WAIT_FOR_SOURCE_BUFFER0),
                                                   (bufOdd ? MSPI_SIGNAL_SOURCE_BUFFER1 : MSPI_SIGNAL_SOURCE_BUFFER0),
                                                   (((address + TEMP_BUFFER_SIZE) >= ui32NumBytes) ? mspiSinkCb : 0),
                                                   0);
        if (ui32Status)
        {
           DEBUG_PRINT("\nFailed to queue up MSPI Write transaction\n");
           break;
        }
    }

#endif
    return ui32Status;
}
