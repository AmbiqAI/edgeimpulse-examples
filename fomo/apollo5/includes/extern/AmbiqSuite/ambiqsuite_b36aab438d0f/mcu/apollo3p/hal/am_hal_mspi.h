//*****************************************************************************
//
//! @file am_hal_mspi.h
//!
//! @brief Functions for Interfacing with the MSPI.
//!
//! @addtogroup mspi3p MSPI - Multi-bit SPI
//! @ingroup apollo3p_hal
//! @{
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_HAL_MSPI_H
#define AM_HAL_MSPI_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! CMSIS-Style macro for handling a variable MSPI module number.
//
//*****************************************************************************
#define MSPIn(n)                    ((MSPI0_Type*)(MSPI0_BASE + (n * (MSPI1_BASE - MSPI0_BASE))))
#define MSPI_IRQn(n)                MSPI_IRQn_temp(n)
#define MSPI_XIP_BASEADDRn(n)       MSPI_XIP_BASEADDRn_temp(n)
#define MSPI_XIPMM_BASEADDRn(n)     MSPI_XIPMM_BASEADDRn_temp(n)

#define AM_HAL_MSPI_MAX_PIO_TRANS_SIZE          65535   // 2^16-1
#define AM_HAL_MSPI_MAX_TRANS_SIZE              16777215    // 2^24 - 1
#define AM_HAL_MSPI_MAX_FIFO_SIZE               16
#define AM_HAL_MSPI_DEFAULT_BURST_COUNT         32
#define AM_HAL_MSPI_HIGH_SPEED_OFFSET           4

// Size guideline for allocation of application supplied buffers
#if 0
#define AM_HAL_MSPI_CQ_ENTRY_SIZE               (28 * sizeof(uint32_t))
#else
#define AM_HAL_MSPI_CQ_ENTRY_SIZE               (22 * sizeof(uint32_t))
#endif

#define AM_HAL_MSPI_HIPRIO_ENTRY_SIZE           (6 * sizeof(uint32_t))

#define AM_HAL_MSPI_SC_CLEAR(flag)              ((flag) << 16)
#define AM_HAL_MSPI_SC_SET(flag)                ((flag))

//! For MSPI - Need to Set the flag for unpausing
#define AM_HAL_MSPI_SC_UNPAUSE(flag)            AM_HAL_MSPI_SC_SET(flag)
#define AM_HAL_MSPI_SC_PAUSE(flag)              AM_HAL_MSPI_SC_CLEAR(flag)

//! Use this macro to directly control the flags
#define AM_HAL_MSPI_SETCLR(module, scVal)       \
    do {                                        \
        MSPIn(module)->CQSETCLEAR = (scVal);    \
    } while (0);

//! Flags 5, 7 & 6 are reserved by HAL
#define AM_HAL_MSPI_PAUSE_FLAG_RESV             (MSPI0_CQFLAGS_CQFLAGS_SWFLAG7 | MSPI0_CQFLAGS_CQFLAGS_SWFLAG6 | MSPI0_CQFLAGS_CQFLAGS_SWFLAG5)
#define AM_HAL_MSPI_SC_RESV_MASK                (AM_HAL_MSPI_PAUSE_FLAG_RESV | (AM_HAL_MSPI_PAUSE_FLAG_RESV << 8) | (AM_HAL_MSPI_PAUSE_FLAG_RESV << 16))

//! We use SWFLAGEN7 to control SW pausing Command Queue - default unPause
//! We use SWFLAGEN6 to pause on the sequece loopback - default Pause
//! We use SWFLAGEN5 to pause CQ while a block is building
#define AM_HAL_MSPI_PAUSE_FLAG_IDX      (_VAL2FLD(MSPI0_CQFLAGS_CQFLAGS, MSPI0_CQFLAGS_CQFLAGS_CQIDX))
#define AM_HAL_MSPI_PAUSE_FLAG_CQ       (_VAL2FLD(MSPI0_CQFLAGS_CQFLAGS, MSPI0_CQFLAGS_CQFLAGS_SWFLAG7))
#define AM_HAL_MSPI_PAUSE_FLAG_SEQLOOP  (_VAL2FLD(MSPI0_CQFLAGS_CQFLAGS, MSPI0_CQFLAGS_CQFLAGS_SWFLAG6))
#define AM_HAL_MSPI_PAUSE_FLAG_BLOCK    (_VAL2FLD(MSPI0_CQFLAGS_CQFLAGS, MSPI0_CQFLAGS_CQFLAGS_SWFLAG5))

//! By default - we Pause CQ for no more entries, or force pause from SW
#define AM_HAL_MSPI_PAUSE_DEFAULT       (AM_HAL_MSPI_PAUSE_FLAG_IDX)
#define AM_HAL_MSPI_CQP_PAUSE_DEFAULT   (AM_HAL_MSPI_PAUSE_FLAG_IDX | AM_HAL_MSPI_PAUSE_FLAG_CQ)

//*****************************************************************************
//
//! @brief Macro definitions for MSPI interrupt status bits.
//!
//! These macros correspond to the bits in the MSPI interrupt status register.
//!
//! @name MSPI Interrupts
//! @{
//
//*****************************************************************************
#define AM_HAL_MSPI_INT_SCRERR               MSPI0_INTEN_SCRERR_Msk
#define AM_HAL_MSPI_INT_CQERR                MSPI0_INTEN_CQERR_Msk
#define AM_HAL_MSPI_INT_CQPAUSED             MSPI0_INTEN_CQPAUSED_Msk
#define AM_HAL_MSPI_INT_CQUPD                MSPI0_INTEN_CQUPD_Msk
#define AM_HAL_MSPI_INT_CQCMP                MSPI0_INTEN_CQCMP_Msk
#define AM_HAL_MSPI_INT_DMAERR               MSPI0_INTEN_DERR_Msk
#define AM_HAL_MSPI_INT_DMACMP               MSPI0_INTEN_DCMP_Msk
#define AM_HAL_MSPI_INT_RX_FIFO_FULL         MSPI0_INTEN_RXF_Msk
#define AM_HAL_MSPI_INT_RX_FIFO_OVFL         MSPI0_INTEN_RXO_Msk
#define AM_HAL_MSPI_INT_RX_FIFO_UNFL         MSPI0_INTEN_RXU_Msk
#define AM_HAL_MSPI_INT_TX_FIFO_OVFL         MSPI0_INTEN_TXO_Msk
#define AM_HAL_MSPI_INT_TX_FIFO_EMPTY        MSPI0_INTEN_TXE_Msk
#define AM_HAL_MSPI_INT_CMDCMP               MSPI0_INTEN_CMDCMP_Msk
#define AM_HAL_MSPI_INT_ALL                  0xFFFFFFFF

#define AM_HAL_MSPI_INT_ERR  (AM_HAL_MSPI_INT_DMAERR | AM_HAL_MSPI_INT_CQERR | AM_HAL_MSPI_INT_SCRERR)


#define AM_HAL_MSPI_LINK_NONE            0x7
#define AM_HAL_MSPI_LINK_BASE            0x8

//! @}  MSPI Interrupts


//*****************************************************************************
//
//! Internal macros to support CMSIS-Style macros
//! These should never be used from callers
//
//*****************************************************************************
#define MSPI_IRQn_temp(n)               (MSPI##n##_IRQn)
#define MSPI_XIP_BASEADDRn_temp(n)      (MSPI##n##_XIP_BASEADDR)
#define MSPI_XIPMM_BASEADDRn_temp(n)    (MSPI##n##_XIPMM_BASEADDR)
//*****************************************************************************
//
//! @brief Configuration structure for the MSPI.
//
//*****************************************************************************

//
//! Number of bytes in the address
//
typedef enum
{
    AM_HAL_MSPI_ADDR_1_BYTE,
    AM_HAL_MSPI_ADDR_2_BYTE,
    AM_HAL_MSPI_ADDR_3_BYTE,
    AM_HAL_MSPI_ADDR_4_BYTE
}
am_hal_mspi_addr_e;

//
//! Number of bytes in the instruction
//
typedef enum
{
    AM_HAL_MSPI_INSTR_1_BYTE,
    AM_HAL_MSPI_INSTR_2_BYTE
}
am_hal_mspi_instr_e;

//
//! Transmit or receive
//
typedef enum
{
    AM_HAL_MSPI_RX = 0,
    AM_HAL_MSPI_TX = 1
}
am_hal_mspi_dir_e;

//
//! Mode of Transfer.
//
typedef enum
{
    AM_HAL_MSPI_TRANS_PIO,
    AM_HAL_MSPI_TRANS_DMA
}
am_hal_mspi_trans_e;

//
//! MSPI interface mode and chip enable selection
//
typedef enum
{
    AM_HAL_MSPI_FLASH_SERIAL_CE0,
    AM_HAL_MSPI_FLASH_SERIAL_CE1,
    AM_HAL_MSPI_FLASH_DUAL_CE0,
    AM_HAL_MSPI_FLASH_DUAL_CE1,
    AM_HAL_MSPI_FLASH_QUAD_CE0,
    AM_HAL_MSPI_FLASH_QUAD_CE1,
    AM_HAL_MSPI_FLASH_OCTAL_CE0,
    AM_HAL_MSPI_FLASH_OCTAL_CE1,
    AM_HAL_MSPI_FLASH_QUADPAIRED,
    AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL,
    AM_HAL_MSPI_FLASH_DUAL_CE0_1_1_2,
    AM_HAL_MSPI_FLASH_DUAL_CE1_1_1_2,
    AM_HAL_MSPI_FLASH_DUAL_CE0_1_2_2,
    AM_HAL_MSPI_FLASH_DUAL_CE1_1_2_2,
    AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4,
    AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4,
    AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4,
    AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4,
    AM_HAL_MSPI_FLASH_SERIAL_CE0_3WIRE,
    AM_HAL_MSPI_FLASH_SERIAL_CE1_3WIRE,
    AM_HAL_MSPI_FLASH_MAX = AM_HAL_MSPI_FLASH_SERIAL_CE1_3WIRE
}
am_hal_mspi_device_e;

//
//! Enumerate the SPI modes.  Note that these are arranged per the ordering of
//! SPHA (bit1) and SPOL (bit0) in the IOM.MSPICFG register.
//
typedef enum
{
    AM_HAL_MSPI_SPI_MODE_0,                  // CPOL = 0; CPHA = 0
    AM_HAL_MSPI_SPI_MODE_2,                  // CPOL = 1; CPHA = 0
    AM_HAL_MSPI_SPI_MODE_1,                  // CPOL = 0; CPHA = 1
    AM_HAL_MSPI_SPI_MODE_3,                  // CPOL = 1; CPHA = 1
}
am_hal_mspi_spi_mode_e;

typedef enum
{
    AM_HAL_MSPI_CLK_48MHZ = 1,
    AM_HAL_MSPI_CLK_24MHZ = 2,
    AM_HAL_MSPI_CLK_16MHZ = 3,
    AM_HAL_MSPI_CLK_12MHZ = 4,
    AM_HAL_MSPI_CLK_8MHZ = 6,
    AM_HAL_MSPI_CLK_6MHZ = 8,
    AM_HAL_MSPI_CLK_4P8MHZ = 10,
    AM_HAL_MSPI_CLK_4MHZ = 12,
    AM_HAL_MSPI_CLK_3P2MHZ = 15,
    AM_HAL_MSPI_CLK_3MHZ = 16,
    AM_HAL_MSPI_CLK_1P5MHZ = 32
}
am_hal_mspi_clock_e;

//
//! Transfer callback function prototype
//
typedef void (*am_hal_mspi_callback_t)(void *pCallbackCtxt, uint32_t status);

typedef struct
{
    bool bDQSEnable;
    bool bOverrideRXDQSDelay;
    uint8_t ui8RxDQSDelay;
    bool bOverrideTXDQSDelay;
    uint8_t ui8TxDQSDelay;
    bool bDQSSyncNeg;
    uint8_t ui8DQSDelay;
}
am_hal_mspi_dqs_t;

//
//! MSPI Timing Scan
//
typedef struct
{
    bool bTxNeg;
    bool bRxNeg;
    bool bRxCap;
    uint8_t ui8Turnaround;
    uint8_t ui8WriteLatency;
}
am_hal_mspi_timing_scan_t;

typedef struct
{
    bool bLoop;
    //! Command Queue Transaction Gating
    uint32_t ui32PauseCondition;
    //! Command Queue Post-Transaction status setting
    uint32_t ui32StatusSetClr;
}
am_hal_mspi_seq_end_t;

typedef struct
{
    uint8_t *pBuf; //!< Buffer provided to store the high priority transaction context
    uint32_t size;  //!< Size of buffer in bytes
}
am_hal_mspi_hiprio_cfg_t;

typedef struct
{
    //! Command Queue Advanced control on gating conditions for transaction to start
    uint32_t ui32PauseCondition;
    //! Command Queue Advanced Post-Transaction status setting
    uint32_t ui32StatusSetClr;
    am_hal_cmdq_entry_t *pCQEntry;
    uint32_t numEntries;
    am_hal_mspi_callback_t pfnCallback;
    void *pCallbackCtxt;
    uint32_t *pJmpAddr;
}
am_hal_mspi_cq_raw_t;

typedef enum
{
    //! Pass uint32_t * as pConfig
    AM_HAL_MSPI_REQ_APBCLK,
    //! Used to set/clear 8 CQ Pause flags - reserved flags are defined as AM_HAL_MSPI_PAUSE_FLAG_RESV
    AM_HAL_MSPI_REQ_FLAG_SETCLR,
    //! Pass uint32_t * as pConfig indicating the IOM# to link to. AM_HAL_MSPI_LINK_IOM_NONE indicates no IOM linked
    AM_HAL_MSPI_REQ_LINK_IOM,
    //! Pass uint32_t * as pConfig indicating the MSPI# to link to. AM_HAL_MSPI_LINK_NONE indicates no MSPI linked
    AM_HAL_MSPI_REQ_LINK_MSPI,
    //! pConfig N/A
    AM_HAL_MSPI_REQ_DCX_DIS,
    //! pConfig N/A
    AM_HAL_MSPI_REQ_DCX_EN,
    //! pConfig N/A
    AM_HAL_MSPI_REQ_SCRAMB_DIS,
    //! pConfig N/A
    AM_HAL_MSPI_REQ_SCRAMB_EN,
    //! Pass uint32_t * as pConfig
    AM_HAL_MSPI_REQ_XIPACK,
    //! pConfig N/A
    AM_HAL_MSPI_REQ_DDR_DIS,
    //! pConfig N/A
    AM_HAL_MSPI_REQ_DDR_EN,
    //! Pass am_hal_mspi_dqs_t * as pConfig
    AM_HAL_MSPI_REQ_DQS,
    //! pConfig N/A
    AM_HAL_MSPI_REQ_XIP_DIS,
    //! pConfig N/A
    AM_HAL_MSPI_REQ_XIP_EN,
    //! Pass am_hal_mspi_device_e * as pConfig
    AM_HAL_MSPI_REQ_DEVICE_CONFIG,
    //! Pass am_hal_mspi_clock_e * as pConfig
    AM_HAL_MSPI_REQ_CLOCK_CONFIG,
    //! Pause the CQ gracefully
    AM_HAL_MSPI_REQ_PAUSE,
    //! Unpause the CQ
    AM_HAL_MSPI_REQ_UNPAUSE,
    //! Get in and out of Sequence Mode - which allows building a sequence, which either runs once, or repeats
    //! Pass in bool * as pConfig - true/false
    AM_HAL_MSPI_REQ_SET_SEQMODE,
    //! Pass am_hal_mspi_seq_end_t * as pConfig
    AM_HAL_MSPI_REQ_SEQ_END,
    //! Initialize configuration for high priority trasactions
    //! These transactions take precedence over existing CQ transactions
    //! Pass am_hal_mspi_hiprio_cfg_t * as pConfig
    AM_HAL_MSPI_REQ_INIT_HIPRIO,
    //! Create a block of transactions which are not paused in between
    //! pConfig N/A
    AM_HAL_MSPI_REQ_START_BLOCK,
    //! pConfig N/A
    AM_HAL_MSPI_REQ_END_BLOCK,
    //! Raw CQ transaction
    //! Pass am_hal_mspi_cq_raw_t * as pConfig
    AM_HAL_MSPI_REQ_CQ_RAW,
    //! Alter the ISIZE of the CFG register
    AM_HAL_MSPI_REQ_ISIZE_SET,
    //! Alter the ASIZE of the CFG register
    AM_HAL_MSPI_REQ_ASIZE_SET,
    // Set writlatncy when write date to nand flash
    AM_HAL_MSPI_REQ_NAND_FLASH_SET_WLAT,
    //! Set the timing parameters
    AM_HAL_MSPI_REQ_TIMING_SCAN,

    AM_HAL_MSPI_REQ_MAX

}
am_hal_mspi_request_e;

typedef enum
{
    AM_HAL_MSPI_BOUNDARY_NONE,          //!< Disable DMA address boundary breaks
    AM_HAL_MSPI_BOUNDARY_BREAK32,       //!< Break at 32 byte boundary (0x20 increments)
    AM_HAL_MSPI_BOUNDARY_BREAK64,       //!< Break at 64 byte boundary (0x40 increments)
    AM_HAL_MSPI_BOUNDARY_BREAK128,      //!< Break at 128 byte boundary (0x80 increments)
    AM_HAL_MSPI_BOUNDARY_BREAK256,      //!< Break at 256 byte boundary (0x100 increments)
    AM_HAL_MSPI_BOUNDARY_BREAK512,      //!< Break at 512 byte boundary (0x200 increments)
    AM_HAL_MSPI_BOUNDARY_BREAK1K,       //!< Break at 1KB boundary (0x400 increments)
    AM_HAL_MSPI_BOUNDARY_BREAK2K,       //!< Break at 2KB boundary (0x800 increments)
    AM_HAL_MSPI_BOUNDARY_BREAK4K,       //!< Break at 4KB boundary (0x1000 increments)
    AM_HAL_MSPI_BOUNDARY_BREAK8K,       //!< Break at 8KB boundary (0x2000 increments)
    AM_HAL_MSPI_BOUNDARY_BREAK16K,      //!< Break at 16KB boundary (0x4000 increments)
    AM_HAL_MSPI_BOUNDARY_MAX
}
am_hal_mspi_dma_boundary_e;

//
//! Device configuration structure
//
typedef struct
{
    //
    //! MSPI device configuration for Polling I/O (PIO) Operation.
    //

    //! Number of turn around cycles between an Address write and Data read.
    uint8_t ui8TurnAround;

    //! Address Configuration
    am_hal_mspi_addr_e eAddrCfg;

    //! Instruction Configuration
    am_hal_mspi_instr_e eInstrCfg;

    //! Read instruction sent to flash device
    uint8_t ui8ReadInstr;

    //! Write instruction sent to flash device
    uint8_t ui8WriteInstr;

    //! External Flash Device configuration
    am_hal_mspi_device_e eDeviceConfig;

    //! Write Latency
    uint8_t ui8WriteLatency;

    //
    //! MSPI clock configuration.
    //

    //! SPI Mode.
    am_hal_mspi_spi_mode_e eSpiMode;

    //! Clock frequency
    am_hal_mspi_clock_e eClockFreq;

    //! XIPENWLAT configure
    bool bEnWriteLatency;

    //
    //! MSPI device configuration for XIP/DMA/Scrambling operations.
    //

    //! Send Device Address
    bool bSendAddr;

    //! Send Device Instruction
    bool bSendInstr;

    //! Enable Turnaround between Address write and Data read.
    bool bTurnaround;

    //! Emulate DDR mode.
    bool bEmulateDDR;

    //
    //! MSPI device DMA Boundary configuration.
    //

    //! Maximum time a DMA transaction is allowed before releasing CE.
    uint16_t ui16DMATimeLimit;       // 100nsec units

    //! Maximum number of bytes DMA transaction is allowed before releasing CE.
    am_hal_mspi_dma_boundary_e eDMABoundary;
    //
    //! MSPI DMA TCB/Command Queue memory allocation.
    //

    //! DMA Transfer Control Buffer size in words.
    uint32_t ui32TCBSize;

    //! DMA Transfer Control Buffer
    uint32_t *pTCB;

    //
    //! MSPI Scrambling configuration.
    //

    //! Scrambling Start Address
    uint32_t scramblingStartAddr;

    //! Scrambling End Address
    uint32_t scramblingEndAddr;

}
am_hal_mspi_dev_config_t;

//
//! MSPI Capabilities structure
//
typedef struct
{
    am_hal_mspi_device_e eDeviceConfig;
}
am_hal_mspi_capabilities_t;

//
//! Device PIO transfer structure
//
typedef struct
{
    //! Number of bytes to transfer
    uint32_t ui32NumBytes;

    //! Enable scrambling.
    bool bScrambling;

    //! Enable DCX
    bool bDCX;

    //! Transfer Direction (Transmit/Receive)
    am_hal_mspi_dir_e eDirection;

    //! Send Device Address
    bool bSendAddr;

    //! Device Address
    uint32_t ui32DeviceAddr;

    //! Send Device Instruction
    bool bSendInstr;

    //! Device Instruction
    uint16_t ui16DeviceInstr;

    //! Enable Turnaround between Address write and Data read.
    bool bTurnaround;

    //! Enable Write Latency Counter
    bool bEnWRLatency;

    //! Paired-Quad
    bool bQuadCmd;

    //! Continuation
    bool bContinue;

    //! Buffer
    uint32_t *pui32Buffer;

}
am_hal_mspi_pio_transfer_t;

//
//! DMA transfer structure
//
typedef struct
{
    //! Priority 0 = Low (best effort); 1 = High (service immediately)
    uint8_t ui8Priority;

    //! Direction RX: 0 = Peripheral to Memory; TX: 1 = Memory to Peripheral
    am_hal_mspi_dir_e eDirection;

    //! Transfer Count
    uint32_t ui32TransferCount;

    //! External Flash Device Address
    uint32_t ui32DeviceAddress;

    //! Internal SRAM Address
    uint32_t ui32SRAMAddress;

    //! Command Queue Transaction Gating
    uint32_t ui32PauseCondition;
    //! Command Queue Post-Transaction status setting
    uint32_t ui32StatusSetClr;

}
am_hal_mspi_dma_transfer_t;


//
//! MSPI status structure.
//
typedef struct
{
    uint32_t ui32NumCQEntries;

    //
    //! DMA status.
    //
    bool bErr;
    bool bCmp;
    bool bTIP;

    //
    //! This is true when the current command queue index matches the end index.
    //! A DMA transfer will be complete when this is true.
    //
    bool bCQEndMatch;

    //
    //! This mirrors the CMDCMP bit. Transfer complete
    //
    bool bTransferComplete;


}
am_hal_mspi_status_t;


#define am_hal_mspi_buffer(A)                                               \
  union                                                                     \
    {                                                                         \
      uint32_t words[(A + 3) >> 2];                                         \
        uint8_t bytes[A];                                                     \
    }

//*****************************************************************************
//
//! @brief MSPI initialization function
//!
//! @param ui32Module   - Module instance.
//! @param ppHandle     - Returns the handle for the module instance.
//!
//! This function accepts a module instance, allocates the interface and then
//! returns a handle to be used by the remaining interface functions.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_initialize(uint32_t ui32Module,
                                       void **ppHandle);

//*****************************************************************************
//
//! @brief MSPI deinitialization function
//!
//! @param pHandle       - The handle for the module instance.
//!
//! This function accepts a handle to an instance and de-initializes the
//! interface.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_deinitialize(void *pHandle);

//*****************************************************************************
//
//! @brief MSPI device configuration function
//!
//! @param pHandle      - Handle for the interface.
//! @param pConfig      - Pointer to the configuration structure.
//!
//! This function configures the MSPI settings for a particular external flash device.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_device_configure(void *pHandle,
                                             am_hal_mspi_dev_config_t *pConfig);

//*****************************************************************************
//
//! @brief MSPI enable function
//!
//! @param pHandle       - The handle for the module instance.
//!
//! This function accepts a handle to an instance and enables the
//! interface.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_enable(void *pHandle);

//*****************************************************************************
//
//! @brief MSPI disable function
//!
//! @param pHandle       - The handle for the module instance.
//!
//! This function accepts a handle to an instance and disables the
//! interface.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_disable(void *pHandle);

//*****************************************************************************
//
//! @brief MSPI device specific control function.
//!
//! @param pHandle      - Handle for the interface.
//! @param eRequest     - Device specific special request code.
//! @param pConfig      - Pointer to the request specific configuration.
//!
//! @note This function configures the MSPI settings for XIP or DMA operation.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_control(void *pHandle,
                                    am_hal_mspi_request_e eRequest,
                                    void *pConfig);

//*****************************************************************************
//
//! @brief MSPI capability interrogation function
//!
//! @param pHandle       - Handle for the interface.
//! @param pCapabilities - Pointer to an interface specific structure used to
//!                       return the capabilities of the interface.
//!
//! This function returns the specific capabilities of the MSPI.  In some
//! cases the capabilities may be instance specific (e.g. maximum data rate).
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_capabilities_get(void *pHandle,
                                             am_hal_mspi_capabilities_t **pCapabilities);

//*****************************************************************************
//
//! @brief MSPI blocking transfer function
//!
//! @param pHandle      - Handle for the interface.
//! @param pTransaction - Pointer to the transaction control structure.
//! @param ui32Timeout  - Timeout in usecs.
//!
//! This function performs a transaction on the MSPI in PIO mode.  It handles
//! half duplex transactions only (TX or RX).
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_blocking_transfer(void *pHandle,
                                              am_hal_mspi_pio_transfer_t *pTransaction,
                                              uint32_t ui32Timeout);

//*****************************************************************************
//
//! @brief MSPI Non-Blocking transfer function
//!
//! @param pHandle       - Handle for the interface.
//! @param pTransfer     - Pointer to the transaction control structure.
//! @param eMode         - Mode
//! @param pfnCallback   - Pointer the callback function to be executed when
//!                        transaction is complete.
//! @param pCallbackCtxt - Passed to callback function
//!
//! This function performs a transaction on the MSPI using either DMA or the
//! Command Queue with DMA.  It handles half duplex transactions.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_nonblocking_transfer(void *pHandle,
                                                 void *pTransfer,
                                                 am_hal_mspi_trans_e eMode,
                                                 am_hal_mspi_callback_t pfnCallback,
                                                 void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief MSPI status function
//!
//! @param pHandle       - Handle for the interface.
//! @param pStatus       - Returned mspi status
//!
//! This function returns the current status of the DMA operation.
//!
//! @return status      - DMA status flags.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_status_get(void *pHandle,
                                       am_hal_mspi_status_t *pStatus);

//*****************************************************************************
//
//! @brief MSPI enable interrupts function
//!
//! @param pHandle      - Handle for the interface.
//! @param ui32IntMask  - MSPI interrupt mask.
//!
//! This function enables the specific indicated interrupts.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_interrupt_enable(void *pHandle,
                                             uint32_t ui32IntMask);

//*****************************************************************************
//
//! @brief MSPI disable interrupts function
//!
//! @param pHandle      - Handle for the interface.
//! @param ui32IntMask  - MSPI interrupt mask.
//!
//! This function disable the specific indicated interrupts.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_interrupt_disable(void *pHandle,
                                              uint32_t ui32IntMask);

//*****************************************************************************
//
//! @brief MSPI interrupt status function
//!
//! @param pHandle      - Handle for the interface.
//! @param pui32Status  - Returns the interrupt status value.
//! @param bEnabledOnly - TRUE: only report interrupt status for enalbed ints.
//!                       FALSE: report all interrupt status values.
//!
//! This function returns the specific indicated interrupt status.
//!
//! @return status      - Interrupt status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_interrupt_status_get(void *pHandle,
                                                 uint32_t *pui32Status,
                                                 bool bEnabledOnly);

//*****************************************************************************
//
//! @brief MSPI interrupt clear
//!
//! @param pHandle        - Handle for the interface.
//! @param ui32IntMask    - uint32_t for interrupts to clear
//!
//! This function clears the interrupts for the given peripheral.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_interrupt_clear(void *pHandle,
                                            uint32_t ui32IntMask);

//*****************************************************************************
//
//! @brief MSPI interrupt service routine
//!
//! @param pHandle       - Handle for the interface.
//! @param ui32IntStatus - Interrupt status.
//!
//! This function is designed to be called from within the user defined ISR
//! in order to service the non-blocking, queued, or DMA processing for a given
//! module instance.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_interrupt_service(void *pHandle,
                                              uint32_t ui32IntStatus);

//*****************************************************************************
//
//! @brief MSPI power control function
//!
//! @param pHandle      - Handle for the interface.
//! @param ePowerState  - The desired power state to move the peripheral to.
//! @param bRetainState - Flag (if true) to save/restore peripheral state upon
//!                       power state change.
//!
//! This function updates the peripheral to a given power state.
//!
//! @return status      - Generic or interface specific status.
//
//*****************************************************************************
extern uint32_t am_hal_mspi_power_control(void *pHandle,
                                          am_hal_sysctrl_power_state_e ePowerState,
                                          bool bRetainState);

//*****************************************************************************
//
//! @brief  MSPI High Priority transfer function
//!
//! @param pHandle        - Handle for the interface
//! @param pTransfer      - Pointer to the transaction control structure
//! @param eMode          - PIO or DMA
//!                             - AM_HAL_MSPI_TRANS_PIO
//!                             - AM_HAL_MSPI_TRANS_DMA
//! @param pfnCallback    - Pointer to function executed when
//!                         transaction is complete (can be set to NULL).
//! @param pCallbackCtxt  - Context registered which is passed on to the callback
//!                         function
//! @return uint32_t      - Generic or interface specific status
//
//*****************************************************************************
extern uint32_t am_hal_mspi_highprio_transfer(void *pHandle,
                                              am_hal_mspi_dma_transfer_t *pTransfer,
                                              am_hal_mspi_trans_e eMode,
                                              am_hal_mspi_callback_t pfnCallback,
                                              void *pCallbackCtxt);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_MSPI_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************