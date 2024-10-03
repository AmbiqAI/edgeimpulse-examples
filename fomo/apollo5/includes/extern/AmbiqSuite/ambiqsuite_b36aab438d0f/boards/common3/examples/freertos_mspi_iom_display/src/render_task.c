//*****************************************************************************
//
//! @file main_task.c
//!
//! @brief Task to handle main operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "freertos_mspi_iom_display.h"

//*****************************************************************************
//
// Render task handle.
//
//*****************************************************************************
TaskHandle_t render_task_handle;

//*****************************************************************************
//
// Handle for Render-related events.
//
//*****************************************************************************
EventGroupHandle_t xRenderEventHandle;

uint32_t g_numDisplay = 0;

// Temp Buffer in SRAM to read PSRAM data to, and write DISPLAY data from
uint32_t        g_TempBuf[2][TEMP_BUFFER_SIZE / 4];

void
psram_read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        DEBUG_PRINT("\nPSRAM Read Failed 0x%x\n", transactionStatus);
    }
    else
    {
        DEBUG_PRINT("\nPSRAM Read Done 0x%x\n", transactionStatus);
    }
}

void
display_write_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        DEBUG_PRINT("\nDisplay# %d:FRAM Write Failed 0x%x\n", g_numDisplay, transactionStatus);
    }
    else
    {
        DEBUG_PRINT_SUCCESS("\nDisplay# %d:FRAM Write Done 0x%x\n", g_numDisplay, transactionStatus);
    }
    g_numDisplay++;
    // Signal main task that display is done
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(xMainEventHandle, MAIN_EVENT_DISPLAY_DONE,
                                        &xHigherPriorityTaskWoken);

    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

#ifdef CQ_RAW
// 2 blocks are includes in head and tail
#define MAX_INT_BLOCKS    (NUM_FRAGMENTS - 2)
// Jump - by reprogramming the CQADDR
typedef struct
{
    uint32_t    ui32CQAddrAddr;
    uint32_t    ui32CQAddrVal;
} am_hal_cq_jmp_t;

// IOM
//
// Command Queue entry structure.
//
typedef struct
{
    uint32_t    ui32PAUSENAddr;
    uint32_t    ui32PAUSEENVal;
    uint32_t    ui32PAUSEN2Addr;
    uint32_t    ui32PAUSEEN2Val;
    // Following 4 fields are only needed for first block
    uint32_t    ui32OFFSETHIAddr;
    uint32_t    ui32OFFSETHIVal;
    uint32_t    ui32DEVCFGAddr;
    uint32_t    ui32DEVCFGVal;

    uint32_t    ui32DMACFGdis1Addr;
    uint32_t    ui32DMACFGdis1Val;
    uint32_t    ui32DMATOTCOUNTAddr;
    uint32_t    ui32DMATOTCOUNTVal;
    uint32_t    ui32DMATARGADDRAddr;
    uint32_t    ui32DMATARGADDRVal;
    uint32_t    ui32DMACFGAddr;
    uint32_t    ui32DMACFGVal;
    // CMDRPT register has been repurposed for DCX
    uint32_t    ui32DCXAddr;
    uint32_t    ui32DCXVal;
//    uint32_t    ui32PAUSEN3Addr;
//    uint32_t    ui32PAUSEEN3Val;
    uint32_t    ui32CMDAddr;
    uint32_t    ui32CMDVal;

    uint32_t    ui32SETCLRAddr;
    uint32_t    ui32SETCLRVal;
} am_hal_iom_txn_head_t;

typedef struct
{
    uint32_t    ui32PAUSENAddr;
    uint32_t    ui32PAUSEENVal;
    uint32_t    ui32PAUSEN2Addr;
    uint32_t    ui32PAUSEEN2Val;

    uint32_t    ui32DMACFGdis1Addr;
    uint32_t    ui32DMACFGdis1Val;
    uint32_t    ui32DMATOTCOUNTAddr;
    uint32_t    ui32DMATOTCOUNTVal;
    uint32_t    ui32DMATARGADDRAddr;
    uint32_t    ui32DMATARGADDRVal;
    uint32_t    ui32DMACFGAddr;
    uint32_t    ui32DMACFGVal;
    // CMDRPT register has been repurposed for DCX
    uint32_t    ui32DCXAddr;
    uint32_t    ui32DCXVal;
    uint32_t    ui32CMDAddr;
    uint32_t    ui32CMDVal;

    uint32_t    ui32SETCLRAddr;
    uint32_t    ui32SETCLRVal;
} am_hal_iom_txn_seg_t;

typedef struct
{
    // Each block will be max size transfer with CONT set
    am_hal_iom_txn_seg_t block[MAX_INT_BLOCKS]; // Fixed
    am_hal_cq_jmp_t  jmp; // Jump always to am_hal_iom_long_txn_t->tail
} am_hal_iom_txn_seq_t;

typedef struct
{
    // Head
    // Max size transfer with CONT Set
    am_hal_iom_txn_head_t    head; // Programmable per transaction: ui32OFFSETHIVal, ui32HdrDMATARGADDRVal, ui32HdrCMDVal
    // Jmp
    am_hal_cq_jmp_t          jmp; // Programmable address to jump inside am_hal_iom_txn_seq_t
    // Tail
    am_hal_iom_txn_seg_t     tail; // Programmable tail length, CMD Value
    am_hal_cq_jmp_t          jmpOut; // Programmable address to jump back to the original CQ
} am_hal_iom_long_txn_t;

// MSPI
//
// Command Queue entry structure.
//
typedef struct
{
    uint32_t                    ui32PAUSENAddr;
    uint32_t                    ui32PAUSEENVal;
    uint32_t                    ui32PAUSEN2Addr;
    uint32_t                    ui32PAUSEEN2Val;
    uint32_t                    ui32DMATARGADDRAddr;
    uint32_t                    ui32DMATARGADDRVal;
    uint32_t                    ui32DMADEVADDRAddr;
    uint32_t                    ui32DMADEVADDRVal;
    uint32_t                    ui32DMATOTCOUNTAddr;
    uint32_t                    ui32DMATOTCOUNTVal;
    uint32_t                    ui32DMACFG1Addr;
    uint32_t                    ui32DMACFG1Val;
    // Need to disable the DMA to prepare for next reconfig
    // Need to have this following the DMAEN for CMDQ
// #### INTERNAL BEGIN ####
    // As otherwise we seem to be hitting CQ Pause condition - CORVETTE-882
// #### INTERNAL END ####
    uint32_t                    ui32DMACFG2Addr;
    uint32_t                    ui32DMACFG2Val;
    uint32_t                    ui32SETCLRAddr;
    uint32_t                    ui32SETCLRVal;
} am_hal_mspi_txn_seg_t;

typedef struct
{
    // Each block will be max size transfer with CONT set
    am_hal_mspi_txn_seg_t block[MAX_INT_BLOCKS]; // Fixed
    am_hal_cq_jmp_t  jmp; // Jump always to am_hal_iom_long_txn_t->tail
} am_hal_mspi_txn_seq_t;


typedef struct
{
    // Head
    // Max size transfer with CONT Set
    am_hal_mspi_txn_seg_t    head; // Programmable per transaction: ui32OFFSETHIVal, ui32HdrDMATARGADDRVal, ui32HdrCMDVal
    // Jmp
    am_hal_cq_jmp_t      jmp; // Programmable address to jump inside am_hal_iom_txn_seq_t
    // Tail
    am_hal_mspi_txn_seg_t     tail; // Programmable tail length, CMD Value
    am_hal_cq_jmp_t      jmpOut; // Programmable address to jump back to the original CQ
} am_hal_mspi_long_txn_t;

am_hal_iom_long_txn_t  gIomLongTxn;
am_hal_iom_txn_seq_t   gIomSequence;
am_hal_mspi_long_txn_t gMspiLongTxn;
am_hal_mspi_txn_seq_t  gMspiSequence;

//*****************************************************************************
//
// Function to build the CMD value.
// Returns the CMD value, but does not set the CMD register.
//
// The OFFSETHI register must still be handled by the caller, e.g.
//      AM_REGn(IOM, ui32Module, OFFSETHI) = (uint16_t)(ui32Offset >> 8);
//
//*****************************************************************************
static uint32_t
build_iom_cmd(uint32_t ui32CS,     uint32_t ui32Dir, uint32_t ui32Cont,
              uint32_t ui32Offset, uint32_t ui32OffsetCnt,
              uint32_t ui32nBytes)
{
    //
    // Initialize the CMD variable
    //
    uint32_t ui32Cmd = 0;

    //
    // If SPI, we'll need the chip select
    //
    ui32Cmd |= _VAL2FLD(IOM0_CMD_CMDSEL, ui32CS);

    //
    // Build the CMD with number of bytes and direction.
    //
    ui32Cmd |= _VAL2FLD(IOM0_CMD_TSIZE, ui32nBytes);

    if (ui32Dir == AM_HAL_IOM_RX)
    {
        ui32Cmd |= _VAL2FLD(IOM0_CMD_CMD, IOM0_CMD_CMD_READ);
    }
    else
    {
        ui32Cmd |= _VAL2FLD(IOM0_CMD_CMD, IOM0_CMD_CMD_WRITE);
    }

    ui32Cmd |= _VAL2FLD(IOM0_CMD_CONT, ui32Cont);

    //
    // Now add the OFFSETLO and OFFSETCNT information.
    //
    ui32Cmd |= _VAL2FLD(IOM0_CMD_OFFSETLO, (uint8_t)ui32Offset);
    ui32Cmd |= _VAL2FLD(IOM0_CMD_OFFSETCNT, ui32OffsetCnt);

    return ui32Cmd;
} // build_cmd()


// One time initialization
void iom_init_cq_long(uint32_t ui32Module)
{
    am_hal_iom_long_txn_t *pIomLong = &gIomLongTxn;
    // Initialize the head block
    // Initialize the tail block
    //
    // Command for OFFSETHI
    //
    pIomLong->head.ui32OFFSETHIAddr  = (uint32_t)&IOMn(ui32Module)->OFFSETHI;

    //
    // Command for I2C DEVADDR field in DEVCFG
    //
    pIomLong->head.ui32DEVCFGAddr    = (uint32_t)&IOMn(ui32Module)->DEVCFG;

    //
    // Command to disable DMA before writing TOTCOUNT.
    //
    pIomLong->tail.ui32DMACFGdis1Addr = pIomLong->head.ui32DMACFGdis1Addr   = (uint32_t)&IOMn(ui32Module)->DMACFG;
    pIomLong->tail.ui32DMACFGdis1Val = pIomLong->head.ui32DMACFGdis1Val    = 0x0;

    //
    // Command to set DMATOTALCOUNT
    //
    pIomLong->tail.ui32DMATOTCOUNTAddr = pIomLong->head.ui32DMATOTCOUNTAddr = (uint32_t)&IOMn(ui32Module)->DMATOTCOUNT;

    //
    // Command to set DMATARGADDR
    //
    pIomLong->head.ui32DMATARGADDRAddr = (uint32_t)&IOMn(ui32Module)->DMATARGADDR;
    pIomLong->tail.ui32DMATARGADDRAddr = (uint32_t)&IOMn(ui32Module)->DMATARGADDR;
    pIomLong->tail.ui32DMATARGADDRVal = (MAX_INT_BLOCKS % 2) ? (uint32_t)&g_TempBuf[0] : (uint32_t)&g_TempBuf[1];
    //
    // Command to set DMACFG to start the DMA operation
    //
    pIomLong->tail.ui32DMACFGAddr = pIomLong->head.ui32DMACFGAddr    = (uint32_t)&IOMn(ui32Module)->DMACFG;

    // CMDRPT register has been repurposed for DCX
    pIomLong->head.ui32DCXAddr = pIomLong->tail.ui32DCXAddr = (uint32_t)&IOMn(ui32Module)->DCX;
#ifdef USE_HW_DCX
    pIomLong->head.ui32DCXVal = pIomLong->tail.ui32DCXVal = IOM0_DCX_DCXEN_Msk | (0x1 << AM_BSP_DISPLAY_DCX_CE);
#else
    pIomLong->head.ui32DCXVal = pIomLong->tail.ui32DCXVal = 0;
#endif

    pIomLong->tail.ui32CMDAddr = pIomLong->head.ui32CMDAddr = (uint32_t)&IOMn(ui32Module)->CMD;

    // Initialize the jumps
    gIomSequence.jmp.ui32CQAddrAddr = pIomLong->jmpOut.ui32CQAddrAddr = pIomLong->jmp.ui32CQAddrAddr = (uint32_t)&IOMn(ui32Module)->CQADDR;
    gIomSequence.jmp.ui32CQAddrVal = (uint32_t)&pIomLong->tail;
    // Initialize the sequence blocks
    for (uint32_t i = 0; i < MAX_INT_BLOCKS; i++)
    {
        gIomSequence.block[i].ui32PAUSENAddr = gIomSequence.block[i].ui32PAUSEN2Addr = (uint32_t)&IOMn(ui32Module)->CQPAUSEEN;
        gIomSequence.block[i].ui32SETCLRAddr = (uint32_t)&IOMn(ui32Module)->CQSETCLEAR;
        //
        // Command to disable DMA before writing TOTCOUNT.
        //
        gIomSequence.block[i].ui32DMACFGdis1Addr = (uint32_t)&IOMn(ui32Module)->DMACFG;
        gIomSequence.block[i].ui32DMACFGdis1Val = 0x0;

        //
        // Command to set DMATOTALCOUNT
        //
        gIomSequence.block[i].ui32DMATOTCOUNTAddr = (uint32_t)&IOMn(ui32Module)->DMATOTCOUNT;

        //
        // Command to set DMACFG to start the DMA operation
        //
        gIomSequence.block[i].ui32DMACFGAddr = (uint32_t)&IOMn(ui32Module)->DMACFG;

        gIomSequence.block[i].ui32DCXAddr = (uint32_t)&IOMn(ui32Module)->DCX;
#ifdef USE_HW_DCX
        gIomSequence.block[i].ui32DCXVal =  IOM0_DCX_DCXEN_Msk | (0x1 << AM_BSP_DISPLAY_DCX_CE);
#else
        gIomSequence.block[i].ui32DCXVal =  0;
#endif
        gIomSequence.block[i].ui32CMDAddr = (uint32_t)&IOMn(ui32Module)->CMD;
        // Pause Conditions
        // This is the Pause Boundary for HiPrio transactions
        gIomSequence.block[i].ui32PAUSEENVal = AM_HAL_IOM_CQP_PAUSE_DEFAULT | ((i % 2) ? IOM_WAIT_FOR_MSPI_BUFFER0 : IOM_WAIT_FOR_MSPI_BUFFER1);
        gIomSequence.block[i].ui32PAUSEEN2Val = AM_HAL_IOM_PAUSE_DEFAULT;
        gIomSequence.block[i].ui32SETCLRVal = (i % 2) ? IOM_SIGNAL_MSPI_BUFFER0 : IOM_SIGNAL_MSPI_BUFFER1;
        gIomSequence.block[i].ui32DMATARGADDRAddr = (uint32_t)&IOMn(ui32Module)->DMATARGADDR;
        gIomSequence.block[i].ui32DMATARGADDRVal = (i % 2) ? (uint32_t)&g_TempBuf[0] : (uint32_t)&g_TempBuf[1];
    }
    // Initialize the Pause conditions
    pIomLong->head.ui32PAUSENAddr = pIomLong->tail.ui32PAUSENAddr = (uint32_t)&IOMn(ui32Module)->CQPAUSEEN;
    pIomLong->head.ui32PAUSEN2Addr = pIomLong->tail.ui32PAUSEN2Addr = (uint32_t)&IOMn(ui32Module)->CQPAUSEEN;
    pIomLong->head.ui32SETCLRAddr = pIomLong->tail.ui32SETCLRAddr = (uint32_t)&IOMn(ui32Module)->CQSETCLEAR;

    pIomLong->tail.ui32PAUSEEN2Val = AM_HAL_IOM_PAUSE_DEFAULT;
    pIomLong->head.ui32PAUSEEN2Val = AM_HAL_IOM_PAUSE_DEFAULT;
    // This is the Pause Boundary for HiPrio transactions
    pIomLong->tail.ui32PAUSEENVal = AM_HAL_IOM_CQP_PAUSE_DEFAULT | ((MAX_INT_BLOCKS % 2) ? IOM_WAIT_FOR_MSPI_BUFFER0 : IOM_WAIT_FOR_MSPI_BUFFER1);
    pIomLong->tail.ui32SETCLRVal = (MAX_INT_BLOCKS % 2) ? IOM_SIGNAL_MSPI_BUFFER0 : IOM_SIGNAL_MSPI_BUFFER1;
}

// Initialize for each type of transacation
void iom_setup_cq_long(uint32_t ui32Module, uint8_t ui8Priority, uint32_t ui32Dir, uint32_t blockSize, uint32_t ui32SpiChipSelect, uint32_t ui32I2CDevAddr)
{
    am_hal_iom_long_txn_t *pIomLong = &gIomLongTxn;
    uint32_t ui32DMACFGVal;
    ui32DMACFGVal     =
        _VAL2FLD(IOM0_DMACFG_DMAPRI, ui8Priority)     |
        _VAL2FLD(IOM0_DMACFG_DMADIR, ui32Dir == AM_HAL_IOM_TX ? 1 : 0) |
        IOM0_DMACFG_DMAEN_Msk;

    // Command for I2C DEVADDR field in DEVCFG
    pIomLong->head.ui32DEVCFGVal     = _VAL2FLD(IOM0_DEVCFG_DEVADDR, ui32I2CDevAddr);
    //
    // Command to set DMATOTALCOUNT
    //
    pIomLong->head.ui32DMATOTCOUNTVal = blockSize;

    //
    // Command to set DMACFG to start the DMA operation
    //
    pIomLong->tail.ui32DMACFGVal = pIomLong->head.ui32DMACFGVal = ui32DMACFGVal;

    // Initialize the sequence blocks
    for (uint32_t i = 0; i < MAX_INT_BLOCKS; i++)
    {
        uint32_t ui32Cmd;
        //
        // Command to start the transfer.
        //
        ui32Cmd = build_iom_cmd(ui32SpiChipSelect, // ChipSelect
                            ui32Dir,          // ui32Dir
                            true,           // ui32Cont
                            0,           // ui32Offset
                            0,        // ui32OffsetCnt
                            blockSize);  // ui32Bytes

        //
        // Command to set DMATOTALCOUNT
        //
        gIomSequence.block[i].ui32DMATOTCOUNTVal = blockSize;

        //
        // Command to set DMACFG to start the DMA operation
        //
        gIomSequence.block[i].ui32DMACFGVal = ui32DMACFGVal;

        // Command
        gIomSequence.block[i].ui32CMDVal = ui32Cmd;
    }
}

static void
create_iom_mspi_read_transaction(uint32_t blockSize,
                                 uint32_t ui32Dir,
                                 uint32_t ui32NumBytes,
                                 uint32_t ui32Instr,
                                 uint32_t ui32InstrLen,
                                 uint32_t ui32SpiChipSelect)
{
    uint32_t ui32Cmd;
    // initialize various fields
    // initialize gIomLongTxn
    // head: ui32OFFSETHIVal, ui32HdrDMATARGADDRVal, ui32HdrCMDVal
    gIomLongTxn.head.ui32OFFSETHIVal   = (uint16_t)(ui32Instr >> 8);

    uint32_t numBlocks = (ui32NumBytes + blockSize - 1) / blockSize;

    // jmp: ui32CQAddrVal
    if (numBlocks > 1)
    {
        ui32Cmd = build_iom_cmd(ui32SpiChipSelect, // ChipSelect
                            ui32Dir,          // ui32Dir
                            true,           // ui32Cont
                            ui32Instr,           // ui32Offset
                            ui32InstrLen,        // ui32OffsetCnt
                            blockSize);  // ui32Bytes
        gIomLongTxn.head.ui32CMDVal   = ui32Cmd;
        if (numBlocks > 2)
        {
            // Identify where in the block array to jump to
            gIomLongTxn.jmp.ui32CQAddrVal = (uint32_t)&gIomSequence.block[MAX_INT_BLOCKS - (numBlocks - 2)];
        }
        else
        {
            // Just jump to tail
            gIomLongTxn.jmp.ui32CQAddrVal = (uint32_t)&gIomLongTxn.tail;
        }
        ui32NumBytes -= blockSize*(numBlocks - 1);
        // tail: ui32HdrCMDVal & ui32DMATOTCOUNTVal
        // Initialize the count & command for tail
        gIomLongTxn.tail.ui32DMATOTCOUNTVal  = ui32NumBytes;
        ui32Cmd = build_iom_cmd(ui32SpiChipSelect, // ChipSelect
                            ui32Dir,          // ui32Dir
                            false,           // ui32Cont
                            0,           // ui32Offset
                            0,        // ui32OffsetCnt
                            ui32NumBytes);  // ui32Bytes
        gIomLongTxn.tail.ui32CMDVal   = ui32Cmd;
    }
    else
    {
        ui32Cmd = build_iom_cmd(ui32SpiChipSelect, // ChipSelect
                            ui32Dir,          // ui32Dir
                            false,           // ui32Cont
                            ui32Instr,           // ui32Offset
                            ui32InstrLen,        // ui32OffsetCnt
                            ui32NumBytes);  // ui32Bytes
        gIomLongTxn.head.ui32CMDVal   = ui32Cmd;
        // Just jump to end
        gIomLongTxn.jmp.ui32CQAddrVal = (uint32_t)&gIomLongTxn.jmpOut;
    }

    // Initialize head - based on how many blocks
    if ((MAX_INT_BLOCKS - numBlocks) % 2)
    {
        gIomLongTxn.head.ui32DMATARGADDRVal = (uint32_t)&g_TempBuf[1];
        // This is the Pause Boundary for HiPrio transactions
        gIomLongTxn.head.ui32PAUSEENVal = AM_HAL_IOM_CQP_PAUSE_DEFAULT | IOM_WAIT_FOR_MSPI_BUFFER1;
        gIomLongTxn.head.ui32SETCLRVal = IOM_SIGNAL_MSPI_BUFFER1;
    }
    else
    {
        gIomLongTxn.head.ui32DMATARGADDRVal = (uint32_t)&g_TempBuf[0];
        // This is the Pause Boundary for HiPrio transactions
        gIomLongTxn.head.ui32PAUSEENVal = AM_HAL_IOM_CQP_PAUSE_DEFAULT | IOM_WAIT_FOR_MSPI_BUFFER0;
        gIomLongTxn.head.ui32SETCLRVal = IOM_SIGNAL_MSPI_BUFFER0;
    }
}

// MSPI
// One time initialization
void mspi_init_cq_long(uint32_t ui32Module)
{
    am_hal_mspi_long_txn_t *pMspiLong = &gMspiLongTxn;
    // Initialize the head block
    // Initialize the tail block
    pMspiLong->head.ui32PAUSENAddr = (uint32_t)&MSPIn(ui32Module)->CQPAUSE;
    pMspiLong->head.ui32PAUSEN2Addr = (uint32_t)&MSPIn(ui32Module)->CQPAUSE;
    pMspiLong->head.ui32DMATARGADDRAddr = (uint32_t)&MSPIn(ui32Module)->DMATARGADDR;
    pMspiLong->head.ui32DMADEVADDRAddr = (uint32_t)&MSPIn(ui32Module)->DMADEVADDR;
    pMspiLong->head.ui32DMATOTCOUNTAddr = (uint32_t)&MSPIn(ui32Module)->DMATOTCOUNT;
    pMspiLong->head.ui32DMACFG1Addr = (uint32_t)&MSPIn(ui32Module)->DMACFG;
    pMspiLong->head.ui32DMACFG2Addr = (uint32_t)&MSPIn(ui32Module)->DMACFG;
    pMspiLong->head.ui32SETCLRAddr = (uint32_t)&MSPIn(ui32Module)->CQSETCLEAR;

    pMspiLong->tail.ui32PAUSENAddr = (uint32_t)&MSPIn(ui32Module)->CQPAUSE;
    pMspiLong->tail.ui32PAUSEN2Addr = (uint32_t)&MSPIn(ui32Module)->CQPAUSE;
    pMspiLong->tail.ui32DMATARGADDRAddr = (uint32_t)&MSPIn(ui32Module)->DMATARGADDR;
    pMspiLong->tail.ui32DMATOTCOUNTAddr = (uint32_t)&MSPIn(ui32Module)->DMATOTCOUNT;
    pMspiLong->tail.ui32DMACFG1Addr = (uint32_t)&MSPIn(ui32Module)->DMACFG;
    pMspiLong->tail.ui32DMACFG2Addr = (uint32_t)&MSPIn(ui32Module)->DMACFG;
    pMspiLong->tail.ui32SETCLRAddr = (uint32_t)&MSPIn(ui32Module)->CQSETCLEAR;
    pMspiLong->tail.ui32DMADEVADDRAddr = (uint32_t)&MSPIn(ui32Module)->DMADEVADDR;

    pMspiLong->head.ui32DMACFG2Val = _VAL2FLD(MSPI_DMACFG_DMAEN, 0);
    pMspiLong->tail.ui32DMACFG2Val = _VAL2FLD(MSPI_DMACFG_DMAEN, 0);

    pMspiLong->tail.ui32DMATARGADDRVal = (MAX_INT_BLOCKS % 2) ? (uint32_t)&g_TempBuf[0] : (uint32_t)&g_TempBuf[1];

    // Initialize the jumps
    gMspiSequence.jmp.ui32CQAddrAddr = pMspiLong->jmpOut.ui32CQAddrAddr = pMspiLong->jmp.ui32CQAddrAddr = (uint32_t)&MSPIn(ui32Module)->CQADDR;
    gMspiSequence.jmp.ui32CQAddrVal = (uint32_t)&pMspiLong->tail;
    // Initialize the sequence blocks
    for (uint32_t i = 0; i < MAX_INT_BLOCKS; i++)
    {
        gMspiSequence.block[i].ui32PAUSENAddr = (uint32_t)&MSPIn(ui32Module)->CQPAUSE;
        gMspiSequence.block[i].ui32PAUSEN2Addr = (uint32_t)&MSPIn(ui32Module)->CQPAUSE;
        gMspiSequence.block[i].ui32DMATARGADDRAddr = (uint32_t)&MSPIn(ui32Module)->DMATARGADDR;
        gMspiSequence.block[i].ui32DMATOTCOUNTAddr = (uint32_t)&MSPIn(ui32Module)->DMATOTCOUNT;
        gMspiSequence.block[i].ui32DMACFG1Addr = (uint32_t)&MSPIn(ui32Module)->DMACFG;
        gMspiSequence.block[i].ui32DMACFG2Addr = (uint32_t)&MSPIn(ui32Module)->DMACFG;
        gMspiSequence.block[i].ui32SETCLRAddr = (uint32_t)&MSPIn(ui32Module)->CQSETCLEAR;
        gMspiSequence.block[i].ui32DMADEVADDRAddr = (uint32_t)&MSPIn(ui32Module)->DMADEVADDR;
        // Pause Conditions
        // This is the Pause Boundary for HiPrio transactions
        gMspiSequence.block[i].ui32PAUSEENVal = AM_HAL_MSPI_CQP_PAUSE_DEFAULT | ((i % 2) ? MSPI_WAIT_FOR_IOM_BUFFER0 : MSPI_WAIT_FOR_IOM_BUFFER1);
        gMspiSequence.block[i].ui32PAUSEEN2Val = AM_HAL_MSPI_PAUSE_DEFAULT;
        gMspiSequence.block[i].ui32SETCLRVal = (i % 2) ? MSPI_SIGNAL_IOM_BUFFER0 : MSPI_SIGNAL_IOM_BUFFER1;
        gMspiSequence.block[i].ui32DMATARGADDRVal = (i % 2) ? (uint32_t)&g_TempBuf[0] : (uint32_t)&g_TempBuf[1];
        gMspiSequence.block[i].ui32DMACFG2Val = _VAL2FLD(MSPI_DMACFG_DMAEN, 0);
    }

    pMspiLong->tail.ui32PAUSEEN2Val = AM_HAL_MSPI_PAUSE_DEFAULT;
    pMspiLong->head.ui32PAUSEEN2Val = AM_HAL_MSPI_PAUSE_DEFAULT;
    // This is the Pause Boundary for HiPrio transactions
    pMspiLong->tail.ui32PAUSEENVal = AM_HAL_MSPI_CQP_PAUSE_DEFAULT | ((MAX_INT_BLOCKS % 2) ? MSPI_WAIT_FOR_IOM_BUFFER0 : MSPI_WAIT_FOR_IOM_BUFFER1);
    pMspiLong->tail.ui32SETCLRVal = (MAX_INT_BLOCKS % 2) ? MSPI_SIGNAL_IOM_BUFFER0 : MSPI_SIGNAL_IOM_BUFFER1;
}

// Initialize for each type of transacation
void mspi_setup_cq_long(uint32_t ui32Module, uint8_t ui8Priority, am_hal_mspi_dir_e eDirection, uint32_t blockSize)
{
    am_hal_mspi_long_txn_t *pMspiLong = &gMspiLongTxn;
    uint32_t ui32DmaCfg =
        _VAL2FLD(MSPI_DMACFG_DMAPWROFF, 0)   |  // DMA Auto Power-off not supported!
        _VAL2FLD(MSPI_DMACFG_DMAPRI, ui8Priority)    |
        _VAL2FLD(MSPI_DMACFG_DMADIR, eDirection)     |
        _VAL2FLD(MSPI_DMACFG_DMAEN, 3);

    pMspiLong->head.ui32DMATOTCOUNTVal = blockSize;
    pMspiLong->head.ui32DMACFG1Val     = pMspiLong->tail.ui32DMACFG1Val = ui32DmaCfg;

    // Initialize the sequence blocks
    for (uint32_t i = 0; i < MAX_INT_BLOCKS; i++)
    {
        gMspiSequence.block[i].ui32DMACFG1Val = ui32DmaCfg;
        gMspiSequence.block[i].ui32DMATOTCOUNTVal = blockSize;
    }
}

static void
create_mspi_iom_write_transaction(uint32_t blockSize,
                                  am_hal_mspi_dir_e eDirection,
                                  uint32_t ui32DevAddr,
                                  uint32_t ui32NumBytes)
{
    am_hal_mspi_long_txn_t *pMspiLong = &gMspiLongTxn;
    uint32_t numBlocks = (ui32NumBytes + blockSize - 1) / blockSize;

    pMspiLong->head.ui32DMADEVADDRVal = ui32DevAddr;
    // jmp: ui32CQAddrVal
    if (numBlocks > 1)
    {
        if (numBlocks > 2)
        {
            // Identify where in the block array to jump to
            gMspiLongTxn.jmp.ui32CQAddrVal = (uint32_t)&gMspiSequence.block[MAX_INT_BLOCKS - (numBlocks - 2)];
            for (uint32_t i = 0; i < (numBlocks - 2); i++)
            {
                gMspiSequence.block[MAX_INT_BLOCKS - (numBlocks - 2) + i].ui32DMADEVADDRVal = ui32DevAddr + (i + 1) * blockSize;
            }
        }
        else
        {
            // Just jump to tail
            gMspiLongTxn.jmp.ui32CQAddrVal = (uint32_t)&gMspiLongTxn.tail;
        }
        ui32NumBytes -= blockSize*(numBlocks - 1);
        // tail: ui32HdrCMDVal & ui32DMATOTCOUNTVal
        // Initialize the count & command for tail
        gMspiLongTxn.tail.ui32DMADEVADDRVal = ui32DevAddr + (numBlocks-1)*blockSize;
        gMspiLongTxn.tail.ui32DMATOTCOUNTVal  = ui32NumBytes;
    }
    else
    {
        // Just jump to end
        gMspiLongTxn.jmp.ui32CQAddrVal = (uint32_t)&gMspiLongTxn.jmpOut;
    }

    // Initialize head - based on how many blocks
    if ((MAX_INT_BLOCKS + 2 - numBlocks) % 2)
    {
        gMspiLongTxn.head.ui32DMATARGADDRVal = (uint32_t)&g_TempBuf[1];
        // This is the Pause Boundary for HiPrio transactions
        gMspiLongTxn.head.ui32PAUSEENVal = AM_HAL_MSPI_CQP_PAUSE_DEFAULT | MSPI_WAIT_FOR_IOM_BUFFER1;
        gMspiLongTxn.head.ui32SETCLRVal = MSPI_SIGNAL_IOM_BUFFER1;
    }
    else
    {
        gMspiLongTxn.head.ui32DMATARGADDRVal = (uint32_t)&g_TempBuf[0];
        // This is the Pause Boundary for HiPrio transactions
        gMspiLongTxn.head.ui32PAUSEENVal = AM_HAL_MSPI_CQP_PAUSE_DEFAULT | MSPI_WAIT_FOR_IOM_BUFFER0;
        gMspiLongTxn.head.ui32SETCLRVal = MSPI_SIGNAL_IOM_BUFFER0;
    }
}

#endif

uint32_t
init_mspi_iom_xfer(void)
{
    uint32_t ui32Status = 0;
    uint32_t      u32Arg;
#ifdef CQ_RAW
    // One time initialization of the preconstructed sequence
    iom_init_cq_long(DISPLAY_IOM_MODULE);
    iom_setup_cq_long(DISPLAY_IOM_MODULE, 1, AM_HAL_IOM_TX, SPI_TXN_SIZE, DISPLAY_SPI_CS, 0);
    mspi_init_cq_long(0);
    mspi_setup_cq_long(0, 1, AM_HAL_MSPI_RX, SPI_TXN_SIZE);
#endif
    // Clear flags
    u32Arg = AM_HAL_IOM_SC_CLEAR(0xFF) & ~AM_HAL_IOM_SC_RESV_MASK;  // clear all flags
    am_hal_iom_control(g_IOMHandle, AM_HAL_IOM_REQ_FLAG_SETCLR, &u32Arg);
    u32Arg = AM_HAL_MSPI_SC_CLEAR(0xFF) & ~AM_HAL_MSPI_SC_RESV_MASK;  // clear all flags
    am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_FLAG_SETCLR, &u32Arg);
    u32Arg = AM_HAL_IOM_SC_PAUSE(IOM_PAUSEFLAG);
    am_hal_iom_control(g_IOMHandle, AM_HAL_IOM_REQ_FLAG_SETCLR, &u32Arg);
    u32Arg = AM_HAL_MSPI_SC_CLEAR(MSPI_PAUSEFLAG);
    am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_FLAG_SETCLR, &u32Arg);
    // Link MSPI and IOM
    u32Arg = DISPLAY_IOM_MODULE;
    am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_LINK_IOM, &u32Arg);
#ifndef CONFIG_DISPLAY_WINDOW // Set the window just once
    display_func.display_set_transfer_window(g_IomDevHdl, false, 0, 0, ROW_NUM - 1, COLUMN_NUM - 1);
#endif
    return ui32Status;
}

int
start_mspi_iom_xfer(uint32_t psramOffset, uint32_t ui32NumBytes, uint32_t startRow, uint32_t startCol, uint32_t endRow, uint32_t endCol)
{
    uint32_t      ui32Status = 0;
    am_hal_iom_callback_t   iomCb = 0;
    am_hal_mspi_callback_t  mspiCb = 0;

    if (xSemaphoreTake(xMSPIMutex, portMAX_DELAY) != pdTRUE)
    {
        return AM_HAL_STATUS_FAIL;
    }
#ifdef SEQLOOP
    bool          bBool;
#endif
    iomCb = display_write_complete;
    mspiCb = psram_read_complete;


    DEBUG_PRINT("\nInitiating MSP -> IOM Transfer\n");
    // Send Display command here
#ifdef CONFIG_DISPLAY_WINDOW
    display_func.display_set_transfer_window(g_IomDevHdl, false, startRow, startCol, endRow, endCol);
#endif

#ifndef USE_HW_DCX
    display_func.display_blocking_write(g_IomDevHdl, NULL, 0);
#endif

#ifdef SEQLOOP
    // Set in Sequence mode
    bBool = true;
    ui32Status = am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_SET_SEQMODE, &bBool);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to set sequence mode on MSPI\n");
        return AM_HAL_STATUS_FAIL;
    }
    ui32Status = am_hal_iom_control(g_IOMHandle, AM_HAL_IOM_REQ_SET_SEQMODE, &bBool);
    if (ui32Status)
    {
        DEBUG_PRINT("\nFailed to set sequence mode on IOM\n");
        return AM_HAL_STATUS_FAIL;
    }
#endif
#ifndef CQ_RAW
    // Queue up Display Writes and PSRAM Reads
    for (uint32_t address = 0, bufIdx = 0; address < ui32NumBytes; address += SPI_TXN_SIZE, bufIdx++)
    {
        uint32_t bufOdd = bufIdx % 2;
        ui32Status = am_devices_mspi_psram_read_adv((uint8_t *)g_TempBuf[bufOdd], psramOffset + address,
                                       (((address + SPI_TXN_SIZE) >= ui32NumBytes) ? (ui32NumBytes - address) : SPI_TXN_SIZE),
                                       (bufOdd ? MSPI_WAIT_FOR_IOM_BUFFER1 : MSPI_WAIT_FOR_IOM_BUFFER0),
                                       (bufOdd ? MSPI_SIGNAL_IOM_BUFFER1 : MSPI_SIGNAL_IOM_BUFFER0),
                                       (((address + SPI_TXN_SIZE) >= ui32NumBytes) ? mspiCb : 0),
                                       0);
        if (ui32Status)
        {
            DEBUG_PRINT("\nFailed to queue up MSPI Read transaction\n");
            break;
        }
        ui32Status = display_func.display_nonblocking_write_adv(g_IomDevHdl, (uint8_t *)g_TempBuf[bufOdd],
                                       (((address + SPI_TXN_SIZE) >= ui32NumBytes) ? (ui32NumBytes - address) : SPI_TXN_SIZE),
                                       (((address + SPI_TXN_SIZE) >= ui32NumBytes) ? false : true),
#ifndef USE_HW_DCX
                                       0,
                                       0,
#else
                                       ((address == 0) ? AM_DEVICES_ST7301_MEMORY_WRITE_CONTINUE : 0),
                                       ((address == 0) ? 1 : 0),
#endif
                                       ((address == 0) ? (IOM_WAIT_FOR_MSPI_BUFFER0 | IOM_PAUSEFLAG) : (bufOdd ? IOM_WAIT_FOR_MSPI_BUFFER1 : IOM_WAIT_FOR_MSPI_BUFFER0)),
//                                       ((bufOdd ? IOM_SIGNAL_MSPI_BUFFER1 : IOM_SIGNAL_MSPI_BUFFER0) | AM_HAL_IOM_SC_PAUSE(IOM_PAUSEFLAG)),
                                       ((bufOdd ? IOM_SIGNAL_MSPI_BUFFER1 : IOM_SIGNAL_MSPI_BUFFER0)),
                                       (((address + SPI_TXN_SIZE) >= ui32NumBytes) ? iomCb : 0),
                                       0);
        if (ui32Status)
        {
            DEBUG_PRINT("\nFailed to queue up SPI Write transaction\n");
             break;
        }
    }
#endif
    if (ui32Status == 0)
    {
#ifdef CQ_RAW
        // Queue up the CQ Raw
        am_hal_cmdq_entry_t jump;
        // MSPI
        create_mspi_iom_write_transaction(SPI_TXN_SIZE,
                                        AM_HAL_MSPI_RX,
                                        psramOffset,
                                        ui32NumBytes);
        jump.address = (uint32_t)&MSPIn(0)->CQADDR;
        jump.value = (uint32_t)&gMspiLongTxn;

        am_hal_mspi_cq_raw_t rawMspiCfg;
        rawMspiCfg.ui32PauseCondition = MSPI_WAIT_FOR_IOM_BUFFER0;
        rawMspiCfg.ui32StatusSetClr = 0;
        rawMspiCfg.pCQEntry = &jump;
        rawMspiCfg.numEntries = sizeof(am_hal_cmdq_entry_t) / 8;
        rawMspiCfg.pfnCallback = mspiCb;
        rawMspiCfg.pCallbackCtxt = 0;
        rawMspiCfg.pJmpAddr = &gMspiLongTxn.jmpOut.ui32CQAddrVal;
        ui32Status = am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_CQ_RAW, &rawMspiCfg);
        if (ui32Status)
        {
            DEBUG_PRINT("\nFailed to queue up MSPI Read transaction\n");
            while(1);
        }
        // IOM
#ifdef USE_HW_DCX
        create_iom_mspi_read_transaction(SPI_TXN_SIZE,
                                        AM_HAL_IOM_TX,
                                        ui32NumBytes,
                                        AM_DEVICES_ST7301_MEMORY_WRITE_CONTINUE,
                                        // #### INTERNAL BEGIN ####
                                        //1, // TODO Address size - dependent on Display device - should be abstracted out in display_device_func_t
                                        // #### INTERNAL END ####
                                        1,
                                        DISPLAY_SPI_CS);
#else
        create_iom_mspi_read_transaction(SPI_TXN_SIZE,
                                        AM_HAL_IOM_TX,
                                        ui32NumBytes,
                                        0,
                                        // #### INTERNAL BEGIN ####
                                        //0, // TODO Address size - dependent on Display device - should be abstracted out in display_device_func_t
                                        // #### INTERNAL END ####
                                        0,
                                        DISPLAY_SPI_CS);
#endif

        if (ui32Status)
        {
            DEBUG_PRINT("\nFailed to queue up SPI Write transaction\n");
            while(1);
        }
        // Queue up the CQ Raw
        jump.address = (uint32_t)&IOMn(DISPLAY_IOM_MODULE)->CQADDR;
        jump.value = (uint32_t)&gIomLongTxn;

        am_hal_iom_cq_raw_t rawIomCfg;
        rawIomCfg.ui32PauseCondition = IOM_WAIT_FOR_MSPI_BUFFER0 | IOM_PAUSEFLAG;
        rawIomCfg.ui32StatusSetClr = 0;
        rawIomCfg.pCQEntry = &jump;
        rawIomCfg.numEntries = sizeof(am_hal_cmdq_entry_t) / 8;
        rawIomCfg.pfnCallback = iomCb;
        rawIomCfg.pCallbackCtxt = 0;
        rawIomCfg.pJmpAddr = &gIomLongTxn.jmpOut.ui32CQAddrVal;
        ui32Status = am_hal_iom_control(g_IOMHandle, AM_HAL_IOM_REQ_CQ_RAW, &rawIomCfg);
        if (ui32Status)
        {
            DEBUG_PRINT("\nFailed to queue up SPI Write transaction\n");
            while(1);
        }

#endif
#ifdef SEQLOOP
        // End the sequence and start
        am_hal_iom_seq_end_t iomLoop;
        am_hal_mspi_seq_end_t mspiLoop;
        iomLoop.bLoop = true;
        // Let IOM be fully controlled by MSPI
        iomLoop.ui32PauseCondition = iomLoop.ui32StatusSetClr = 0;
        am_hal_iom_control(g_IOMHandle, AM_HAL_IOM_REQ_SEQ_END, &iomLoop);
        mspiLoop.bLoop = true;
        // Let MSPI be controlled by a flag
        mspiLoop.ui32PauseCondition = MSPI_PAUSEFLAG;
        mspiLoop.ui32StatusSetClr = AM_HAL_MSPI_SC_PAUSE(MSPI_PAUSEFLAG);
        am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_SEQ_END, &mspiLoop);
#endif
    }
    else
    {
        while(1);
    }
    xSemaphoreGive(xMSPIMutex);
    return ui32Status;
}

#ifdef SEQLOOP
uint32_t
trigger_mspi_iom_xfer(uint32_t startRow, uint32_t startCol, uint32_t endRow, uint32_t endCol)
{
    uint32_t flag = AM_HAL_MSPI_SC_UNPAUSE(MSPI_PAUSEFLAG);
#ifdef CONFIG_DISPLAY_WINDOW
    display_func.display_set_transfer_window(g_IomDevHdl, true, startRow, startCol, endRow, endCol);
#endif
#ifndef USE_HW_DCX
    display_func.display_blocking_cmd_write(g_IomDevHdl, true, AM_DEVICES_ST7301_MEMORY_WRITE_CONTINUE, true);
#endif
    return am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_FLAG_SETCLR, &flag);
}
#endif

int
init_iom_display_data(void)
{
    uint32_t      ui32Status;

    uint32_t i;
    //
    // Generate raw color data into temp buffer
    //
    for (i = 0; i < TEMP_BUFFER_SIZE / 4; i++)
    {
        g_TempBuf[0][i] = COLOR_4P(BACKGROUND_COLOR);
    }

    for (uint32_t address = 0; address < FRAME_SIZE; address += TEMP_BUFFER_SIZE)
    {
        ui32Status = display_func.display_blocking_write(g_IomDevHdl, (uint8_t *)g_TempBuf[0], TEMP_BUFFER_SIZE);
        if (0 != ui32Status)
        {
            DEBUG_PRINT("Failed to read buffer to display!\n");
            return -1;
        }
    }
    return 0;
}

void
RenderTaskSetup(void)
{
    uint32_t iRet;
    am_util_debug_printf("RenderTask: setup\r\n");
    //
    // Create an event handle for our wake-up events.
    //
    xRenderEventHandle = xEventGroupCreate();

    //
    // Make sure we actually allocated space for the events we need.
    //
    while (xRenderEventHandle == NULL);
    // Initialize display data
    iRet = init_iom_display_data();
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize display data\n");
        while(1);
    }

    am_devices_st7301_display_on(g_IomDevHdl);

    init_mspi_iom_xfer();
    // Notify main task that the frame is ready
//    xEventGroupSetBits(xMainEventHandle, MAIN_EVENT_DISPLAY_DONE);

}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
RenderTask(void *pvParameters)
{
    uint32_t eventMask;

    while (1)
    {
        //
        // Wait for an event to be posted to the Radio Event Handle.
        //
        eventMask = xEventGroupWaitBits(xRenderEventHandle, 0xF, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        if (eventMask != 0)
        {
            if (eventMask & RENDER_EVENT_START_NEW_FRAME)
            {
#ifndef NO_RENDERING
                uint32_t psramFbOffset;
                psramFbOffset = (g_numDisplay & 0x1) ? PSRAM_ACTFB2_OFFSET: PSRAM_ACTFB1_OFFSET;
                DEBUG_PRINT("Rendering ActFB %d\n", (g_numDisplay & 0x1));
#if (IOM_PAUSEFLAG != 0)
                AM_HAL_IOM_SETCLR(DISPLAY_IOM_MODULE, AM_HAL_IOM_SC_PAUSE(IOM_PAUSEFLAG));
#endif
#ifdef SEQLOOP
                if (g_numDisplay)
                {
                    if (trigger_mspi_iom_xfer(0, 0, ROW_NUM - 1, COLUMN_NUM - 1))
                    {
                        while(1);
                    }
                }
                else
#else
#endif
                {
                    if (start_mspi_iom_xfer(psramFbOffset, FRAME_SIZE, 0, 0, ROW_NUM - 1, COLUMN_NUM - 1))
                    {
                        while(1);
                    }
                }
#else
                // No Rendering - Just indicate Display Done
                xEventGroupSetBits(xMainEventHandle, MAIN_EVENT_DISPLAY_DONE);
#endif
            }
        }
    }
}

