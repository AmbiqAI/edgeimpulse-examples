//*****************************************************************************
//
//! @file pwr_mspi.c
//!
//! @brief Example for MSPI DMA power measurements.
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup pwr_mspi MSPI Power Example
//! @ingroup power_examples
//! @{
//!
//! Purpose: Example for MSPI DMA power measurements.
//!
//! @details Simple example that uses HAL to queue multiple MSPI DMA transfers.
//!
//! The instance 1 and 2 may not support all of the Quad, Octal and Hex mode,
//! check the corresponding manuals for details<br>
//!
//!   1. Enter deepsleep during DMA transfer.<br>
//!   2. DMA write-loop or read-loop every 1s.<br>
//!   3. Have configurable MSPI buffer size.<br>
//!   4. GPIO indicator for CPU wake state.<br>
//!
//! Debug:<br>
//!     GPIO[0]  GPIO_DBG1_RW Set LOW for start read, set HIGH for start write<br>
//!     GPIO[3]  DEBUG_PIN_2 will keep HIGH during all setting counts transfer<br>
//!     GPIO[2]  DEBUG_PIN_SLEEP will remain LOW during deep sleep mode<br>
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "pwr_timer_utils.h"
#include "pwr_gpio_utils.h"
#include "pwr_control_utils.h"

//*****************************************************************************
// Customize the following for the test
//*****************************************************************************
#define PRINT_ENABLE      0         //!< Enabled printing during startup
#define PRINT_IN_LOOP     0         //!< Enables printing in the main loop, usually off for low power
#define GPIO_DEBUG_ENABLE 0         //!< when set to 1 the debug pins are enabled
#define NUM_TX_LOOPS      10        //!< number of non-blocking (read or write) transmissions run per sequence

#define DEBUG_PIN_1       0        // Set LOW for start read, set HIGH for start write
#define DEBUG_PIN_2       3        // keep HIGH during all setting counts transfer
#define DEBUG_PIN_SLEEP   2        // remain LOW during deep sleep mode
//
//! set up debug function #defines\n
//! (disable gpio functions if debug pins aren't used)
//
#if GPIO_DEBUG_ENABLE == 1
#define GPIO_DBG1_RW_HI am_hal_gpio_output_set(DEBUG_PIN_1)
#define GPIO_DBG1_RW_LOW am_hal_gpio_output_clear(DEBUG_PIN_1)
#define GPIO_DBG2_HI am_hal_gpio_output_set(DEBUG_PIN_2)
#define GPIO_DBG2_LOW am_hal_gpio_output_clear(DEBUG_PIN_2)
#define GPIO_SLEEP_HI am_hal_gpio_output_set(DEBUG_PIN_SLEEP)
#define GPIO_SLEEP_LOW am_hal_gpio_output_clear(DEBUG_PIN_SLEEP)
#else
#define GPIO_DBG1_RW_HI
#define GPIO_DBG1_RW_LOW
#define GPIO_DBG2_HI
#define GPIO_DBG2_LOW
#define GPIO_SLEEP_HI
#define GPIO_SLEEP_LOW
#endif

//
//! Choose MSPI module. The MSPI instance chosen can be 0, 1, or 2.
//
#define MSPI_MODULE       0


#define AM_DEVICES_MSPI_PSRAM_PAGE_SIZE         1024
//
//! Below definitions are borrowed from APS25616 PSRAM driver and are provided for example only
//
#define MSPI_PSRAM_HEX_DDR_READ                 0x2020
#define MSPI_PSRAM_HEX_DDR_WRITE                0xA0A0
//
//! Below definitions are borrowed from APS12808 PSRAM driver and are provided for example only
//
#define MSPI_PSRAM_OCTAL_DDR_READ               0x2020
#define MSPI_PSRAM_OCTAL_DDR_WRITE              0xA0A0
//
//! Below definitions are borrowed from is25wx064 FLASH driver and are provided for example only
//
#define MSPI_PSRAM_OCTAL_READ                   0x03
#define MSPI_PSRAM_OCTAL_WRITE                  0x02
//
//! Below definitions are borrowed from APS6404 PSRAM driver and are provided for example only
//
#define MSPI_PSRAM_QUAD_READ                    0xEB
#define MSPI_PSRAM_QUAD_WRITE                   0x38
//
//! Below definitions are borrowed from is25wx064 Flash driver and are provided for example only
//
#define MSPI_PSRAM_DUAL_READ                    0x03
#define MSPI_PSRAM_DUAL_WRITE                   0x02
//
//! Below definitions are borrowed from is25wx064 Flash driver and are provided for example only
//
#define MSPI_PSRAM_SERIAL_READ                  0x02
#define MSPI_PSRAM_SERIAL_WRITE                 0x03

//
//! Enable one MSPI config setting (Serial, Dual, Quad, Octal, or Hex) within below table
//! @note Bus width settings available depend on chipset and module
//! the actual DDR clock frequency is half of setting due to dual edge sampling
//! Check datasheet and BSP for supported configurations
//
//#define USE_SERIAL_CE0
//#define USE_SERIAL_CE1
//#define USE_DUAL_CE0
//#define USE_DUAL_CE1
//#define USE_QUAD_CE0
//#define USE_QUAD_CE1
#define USE_OCTAL_CE0
//#define USE_OCTAL_CE1
//#define USE_OCTAL_DDR_CE0
//#define USE_OCTAL_DDR_CE1
//#define USE_HEX_DDR_CE0
//#define USE_HEX_DDR_CE1

#if defined(USE_SERIAL_CE0)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_SERIAL_CE0
#define MSPI_BUS_WIDTH              1
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_1_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_SERIAL_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_SERIAL_WRITE
#elif defined(USE_SERIAL_CE1)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_SERIAL_CE1
#define MSPI_BUS_WIDTH              1
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_1_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_SERIAL_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_SERIAL_WRITE

#elif defined(USE_DUAL_CE0)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_DUAL_CE0
#define MSPI_BUS_WIDTH              2
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_1_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_DUAL_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_DUAL_WRITE
#elif defined(USE_DUAL_CE1)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_DUAL_CE1
#define MSPI_BUS_WIDTH              2
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_1_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_DUAL_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_DUAL_WRITE

#elif defined(USE_QUAD_CE0)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_QUAD_CE0
#define MSPI_BUS_WIDTH              4
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_1_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_QUAD_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_QUAD_WRITE
#elif defined(USE_QUAD_CE1)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_QUAD_CE1
#define MSPI_BUS_WIDTH              4
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_1_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_QUAD_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_QUAD_WRITE

#elif defined(USE_OCTAL_CE0)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_OCTAL_CE0
#define MSPI_BUS_WIDTH              8
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_1_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_OCTAL_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_OCTAL_WRITE
#elif defined(USE_OCTAL_CE1)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_OCTAL_CE1
#define MSPI_BUS_WIDTH              8
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_1_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_OCTAL_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_OCTAL_WRITE

#elif defined(USE_OCTAL_DDR_CE0)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0
#define MSPI_BUS_WIDTH              8
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_2_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_OCTAL_DDR_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_OCTAL_DDR_WRITE
#elif defined(USE_OCTAL_DDR_CE1)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1
#define MSPI_BUS_WIDTH              8
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_2_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_OCTAL_DDR_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_OCTAL_DDR_WRITE

#elif defined(USE_HEX_DDR_CE0)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_HEX_DDR_CE0
#define MSPI_BUS_WIDTH              16
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_2_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_HEX_DDR_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_HEX_DDR_WRITE
#elif defined(USE_HEX_DDR_CE1)
#define MSPI_CONFIG                 AM_HAL_MSPI_FLASH_HEX_DDR_CE1
#define MSPI_BUS_WIDTH              16
#define MSPI_ADDR_BYTES_BYTE        AM_HAL_MSPI_ADDR_4_BYTE
#define MSPI_INSTR_BYTES_BYTE       AM_HAL_MSPI_INSTR_2_BYTE
#define MSPI_INSTR_READ             MSPI_PSRAM_HEX_DDR_READ
#define MSPI_INSTR_WRITE            MSPI_PSRAM_HEX_DDR_WRITE
#else
#error "MSPI not supported in power test"
#endif
//
//! Select clock frequency
//! @note Supported clock frequencies dependent on chipset and module
//! Check datasheet for supported clock frequencies
//
#define CLOCK_FREQUENCY   AM_HAL_MSPI_CLK_96MHZ

#define psram_mspi_isr am_mspi_isrx(MSPI_MODULE)
#define am_mspi_isrx(n) am_mspi_isr(n)
#define am_mspi_isr(n) am_mspi ## n ## _isr

//! setting USE_PERIPHERAL to zero will disable the MSPI module and put the device into
//! low power deepsleep
//! setting USE_PERIPHERAL to 1 will enable the peripheral and allow peripheral power measurement
//! The power and retention settings are specified in pwr_pwerControl_utils.c
//! enable PERIPHERAL (this should usually be 1)
#define USE_PERIPHERAL 1

#if (PRINT_ENABLE == 0) && (PRINT_IN_LOOP == 1)
#warning "print config error"
#undef PRINT_IN_LOOP
#define PRINT_IN_LOOP 0
#endif

#if PRINT_ENABLE == 0
#define STARTUP_PRINT(args...)
#else
#define STARTUP_PRINT(args...) am_util_stdio_printf(args)
#endif

#if PRINT_IN_LOOP == 0
#define DEBUG_PRINT(args...)
#else
#define DEBUG_PRINT(args...) am_util_stdio_printf(args)
#endif

am_hal_mspi_dev_config_t  MSPI_DEVICE_Config =
{
    .eAddrCfg             = MSPI_ADDR_BYTES_BYTE,
    .eInstrCfg            = MSPI_INSTR_BYTES_BYTE,
    .ui16ReadInstr        = MSPI_INSTR_READ,
    .ui16WriteInstr       = MSPI_INSTR_WRITE,
    .eDeviceConfig        = MSPI_CONFIG,
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .bSendAddr            = true,
    .bSendInstr           = true,
    .bTurnaround          = true,
    .eClockFreq           = CLOCK_FREQUENCY,
    .bEnWriteLatency      = true,
    .bEmulateDDR          = true,
    .ui16DMATimeLimit     = 20,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_BREAK1K,
};

#if MSPI_MODULE >= AM_REG_MSPI_NUM_MODULES
#error "invalid mspi value"
#endif

#define TESTDATASIZE (192*4)           //!< Number of 32bit words allocated for test data

typedef enum
{
    e_no_transfer,
    e_transfer_read,
    e_transfer_write,
}
transfer_options_e;

//*****************************************************************************
//
//! Global Variables
//! @{
//
//*****************************************************************************

//
//! Collect most of the ram variables used here in this struct
//
typedef struct
{
    void *g_MSPIHandle;
    uint32_t ui32TxBuff[TESTDATASIZE + 8];      //!< Test data tx buff
    uint32_t ui32RxBuff[TESTDATASIZE + 8];      //!< Test data rx buff
    uint32_t DMATCBBuffer[4096];                //!< Command buffer
    int32_t i32TxRxCount;                       //!< counts the number of started transfers the current sequence
    int32_t i32NumCompleteCount;                //!< counts the number of completed transfers the current sequence
    uint32_t ui32DebugPinNum[eMAX_DEBUG_PINS];  //!< Debug pin numbers needs to be set early in startup

    volatile bool bMspiComplete;
    volatile bool bTimerFlag;   //!< set (isr level) in timer callback when timer expires
    bool bTransferRunning;
    //
    // This code alternates between read and write MSPI io, These variables track the mode
    //
    transfer_options_e eCurrentTransfer;         //!< current transfer mode
    transfer_options_e eLastTransfer;            //!< last transfer mode, used to toggle transfer mode
} globasx_t;

//
//! allocate ram variables used
//
static globasx_t g_tGlb;

//!
//! precompute MSPI isr vector number
//!
const IRQn_Type gc_mspiIrq = (IRQn_Type) (MSPI0_IRQn + MSPI_MODULE);

//
//! MSPI setup struct, set up for MSPI init
//
static am_hal_mspi_config_t tMspiConfig =
{
      .ui32TCBSize          = 0,
      .pTCB                 = NULL,
      .bClkonD4             = 0
};

//! @}
// end of global variables


//
//! typedef read-write function pointer
//
typedef uint32_t (*mspi_dma_fcn)(bool bSpi, uint32_t offset, uint32_t *pBuf, uint32_t size);

//!
//! Static function prototypes
//! @{
//!
void psram_mspi_isr(void);
static uint32_t mspi_dma_read(uint32_t ui32Address, uint32_t *pui8Buffer, uint32_t ui32NumBytes);
static uint32_t mspi_dma_write(uint32_t ui32Address, uint32_t *pui8Buffer, uint32_t ui32NumBytes);
static void timerCallback(void);
static void init_pattern(void);
static uint32_t mspi_init(void);
static void debug_pin_init(void);

static uint32_t pwr_data_transfer_sequence(transfer_options_e eTransferType);
static uint32_t pwr_terminate_transfer(uint32_t ui32Status);

//! @}

//*****************************************************************************
//
//! @brief     Timer callback
//!
//! This is called when the timer expires from the timer interrupt
//!
//! @param x   Unused
//
//*****************************************************************************
static void
timerCallback(void)
{
    g_tGlb.bTimerFlag = true;
}

//*****************************************************************************
//
//! @brief     MSPI Complete callback
//! Called when the mspi non-blocking transfer completes
//!
//! @param pCallbackCtxt  - unused
//! @param ui32TransactionStatus  - unused
//
//*****************************************************************************
static void
mspiCompleteCallback(void *pCallbackCtxt, uint32_t ui32TransactionStatus)
{
    (void) pCallbackCtxt;
    (void) ui32TransactionStatus;

    g_tGlb.bMspiComplete = true;
}

//*****************************************************************************
//
//! MSPI ISRs
//! Take over correct MSPI ISR.
//
//*****************************************************************************
void
psram_mspi_isr(void)
{
    uint32_t ui32Status;

    if ( AM_HAL_STATUS_SUCCESS == am_hal_mspi_interrupt_status_get(g_tGlb.g_MSPIHandle,
                                                                   &ui32Status, false))
    {
        if (ui32Status)
        {
            am_hal_mspi_interrupt_clear(g_tGlb.g_MSPIHandle, ui32Status);
            am_hal_mspi_interrupt_service(g_tGlb.g_MSPIHandle, ui32Status);
        }
    }
}

//*****************************************************************************
//
//! @brief init the test output pattern
//! @note There are three options currently available:
//!   -- Sequential (repeating sequence of 0-255)
//!        pwrctrl_fill_buff_incrementing_bytes()
//!   -- Alternating bits 0xA5
//!        pwrctrl_fill_buff_alternating_bits()
//!   -- Random byte values
//!        pwrctrl_fill_buff_random()
//
//*****************************************************************************
static void
init_pattern(void)
{
    //
    // fill with alternating bit pattern
    //
    pwrctrl_fill_buff_alternating_bits((uint8_t *) g_tGlb.ui32TxBuff, MSPI_BUS_WIDTH, TESTDATASIZE * 4);
}


//*****************************************************************************
//
//! @brief init the MSPI
//!
//! @return standard hal status
//
//*****************************************************************************
static uint32_t
mspi_init(void)
{
    uint32_t ui32MspiStatus;
    do
    {
        //
        // get mspi pins set-up
        //
        am_bsp_mspi_pins_enable(MSPI_MODULE, MSPI_DEVICE_Config.eDeviceConfig);

        ui32MspiStatus = am_hal_mspi_initialize(MSPI_MODULE, &g_tGlb.g_MSPIHandle);
        if (AM_HAL_STATUS_SUCCESS != ui32MspiStatus)
        {
            break;
        }

        ui32MspiStatus = am_hal_mspi_power_control(g_tGlb.g_MSPIHandle, AM_HAL_SYSCTRL_WAKE, false);
        if (AM_HAL_STATUS_SUCCESS != ui32MspiStatus)
        {
            break;
        }

        am_hal_mspi_config_t *mspiCfg = &tMspiConfig;
        mspiCfg->ui32TCBSize = TESTDATASIZE;
        mspiCfg->pTCB = g_tGlb.DMATCBBuffer;
        ui32MspiStatus = am_hal_mspi_configure(g_tGlb.g_MSPIHandle, mspiCfg);
        if (AM_HAL_STATUS_SUCCESS != ui32MspiStatus)
        {
            break;
        }

        ui32MspiStatus = am_hal_mspi_device_configure(g_tGlb.g_MSPIHandle, &MSPI_DEVICE_Config);
        if (AM_HAL_STATUS_SUCCESS != ui32MspiStatus)
        {
            break;
        }

        ui32MspiStatus = am_hal_mspi_enable(g_tGlb.g_MSPIHandle);
        if (AM_HAL_STATUS_SUCCESS != ui32MspiStatus)
        {
            break;
        }

        //
        // Enable MSPI interrupts.
        //
        ui32MspiStatus = am_hal_mspi_interrupt_clear(g_tGlb.g_MSPIHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
        if (AM_HAL_STATUS_SUCCESS != ui32MspiStatus)
        {
            return AM_HAL_STATUS_FAIL;
        }

        ui32MspiStatus = am_hal_mspi_interrupt_enable(g_tGlb.g_MSPIHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
        if (AM_HAL_STATUS_SUCCESS != ui32MspiStatus)
        {
            return AM_HAL_STATUS_FAIL;
        }
    } while (false);

    return ui32MspiStatus;
}

//*****************************************************************************
//
//! @brief mspi_dma_read, mspi read data buffer
//!
//! @param ui32Address  - write to address
//! @param pui8Buffer   - data to be write buffer address
//! @param size         - number of samples to transfer
//!
//! @return             - standard hal status
//
//*****************************************************************************
static uint32_t
mspi_dma_read(uint32_t ui32Address, uint32_t *pui8Buffer, uint32_t ui32NumBytes)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    am_hal_mspi_dma_transfer_t Transaction;

    Transaction.ui8Priority = 1;
    Transaction.eDirection = AM_HAL_MSPI_RX;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    // Need to be aware of page size
    while (ui32NumBytes)
    {
        uint32_t size;
        if ((ui32Address & 0x3) &&
           ((AM_DEVICES_MSPI_PSRAM_PAGE_SIZE - (ui32Address & (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE - 1))) < ui32NumBytes))
        {
            // Hardware does not support Page splitting if address is not word aligned
            // Need to split the transaction
            size = 4 - (ui32Address & 0x3);
        }
        else
        {
            size = ui32NumBytes;
        }

        bool bLast = (size == ui32NumBytes);
        Transaction.ui32TransferCount = size;
        Transaction.ui32DeviceAddress = ui32Address;
        Transaction.ui32SRAMAddress = (uint32_t)pui8Buffer;

        if (bLast)
        {
            Transaction.ui32StatusSetClr = 0;
        }

        ui32Status = am_hal_mspi_nonblocking_transfer(g_tGlb.g_MSPIHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA,
                                                      mspiCompleteCallback, NULL);

        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            break;
        }
        ui32Address += size;
        ui32NumBytes -= size;
        pui8Buffer += size;

        Transaction.ui32PauseCondition = 0;
    }
    return ui32Status;
}

//*****************************************************************************
//
//! @brief    mspi_dma_write, mspi write buffer
//!
//! @param ui32Address  - write to address
//! @param pui8Buffer   - data to be write buffer address
//! @param size         - number of samples to transfer
//!
//! @return             - standard hal status
//
//*****************************************************************************
static uint32_t
mspi_dma_write(uint32_t ui32Address, uint32_t *pui8Buffer, uint32_t ui32NumBytes)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    am_hal_mspi_dma_transfer_t    Transaction;
    Transaction.ui8Priority = 1;
    Transaction.eDirection = AM_HAL_MSPI_TX;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    // Need to be aware of page size
    while (ui32NumBytes)
    {
        uint32_t size;
        if ((ui32Address & 0x3) &&
          ((AM_DEVICES_MSPI_PSRAM_PAGE_SIZE - (ui32Address & (AM_DEVICES_MSPI_PSRAM_PAGE_SIZE - 1))) < ui32NumBytes))
        {
            // Hardware does not support Page splitting if address is not word aligned
            // Need to split the transaction
            size = 4 - (ui32Address & 0x3);
        }
        else
        {
            size = ui32NumBytes;
        }

        bool bLast = (size == ui32NumBytes);
        Transaction.ui32TransferCount = size;
        Transaction.ui32DeviceAddress = ui32Address;
        Transaction.ui32SRAMAddress = (uint32_t)pui8Buffer;

        if (bLast)
        {
            Transaction.ui32StatusSetClr = 0;
        }

        ui32Status = am_hal_mspi_nonblocking_transfer(g_tGlb.g_MSPIHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA,
                                                      bLast ? mspiCompleteCallback : NULL,
                                                      NULL);

        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            break;
        }
        ui32Address += size;
        ui32NumBytes -= size;
        pui8Buffer += size;

        Transaction.ui32PauseCondition = 0;
    }
    return ui32Status;
}

static void debug_pin_init(void)
{
#if GPIO_DEBUG_ENABLE == 1
    am_hal_gpio_pinconfig(DEBUG_PIN_1, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_2, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_SLEEP, am_hal_gpio_pincfg_output);
#endif
    GPIO_DBG1_RW_LOW;
    GPIO_DBG2_LOW;
    GPIO_SLEEP_LOW;
    return;
}

//****************************************************************************
//
//! @brief partially generic function to manage buffer transmissions
//!
//! @param eTransferType
//! @param ui32NumIoFunc
//!
//! @return
//
//****************************************************************************
static uint32_t
pwr_data_transfer_sequence(transfer_options_e eTransferType)
{
    if (g_tGlb.bMspiComplete)
    {
        //
        // last transfer completed
        //
        g_tGlb.bMspiComplete = false;

        if (g_tGlb.bTransferRunning)
        {
            g_tGlb.bTransferRunning = false;

            if (++g_tGlb.i32NumCompleteCount >= NUM_TX_LOOPS)
            {
                //
                // all transfers complete
                //
                DEBUG_PRINT("\nmspi io transmit sequence complete %lu\n", eTransferType);
                return pwr_terminate_transfer(0);
            }
        }
    }

    if ((g_tGlb.i32TxRxCount < NUM_TX_LOOPS) && (g_tGlb.bTransferRunning == false))
    {
        //
        // nothing is transmitting, and there is more left to transmit, start another transfer now
        //
        g_tGlb.i32TxRxCount++;

        uint32_t ui32TxRxStat = 0;
        if (eTransferType == e_transfer_read)
        {
            GPIO_DBG1_RW_LOW; //  Set low for start read
            ui32TxRxStat = mspi_dma_read(0, g_tGlb.ui32RxBuff, TESTDATASIZE * 4 - 8);
            g_tGlb.bTransferRunning = true;

        }
        else if (eTransferType == e_transfer_write)
        {
            GPIO_DBG1_RW_HI; //  Set high for start write
            ui32TxRxStat = mspi_dma_write(0, g_tGlb.ui32TxBuff, TESTDATASIZE * 4 - 8);
            g_tGlb.bTransferRunning = true;
        }

        if (ui32TxRxStat)
        {
            //
            // an error starting a transfer
            //
            return pwr_terminate_transfer(2);
        }
    }

    //
    // keep starting transfers
    //
    return 0;
} // while ( true )

//****************************************************************************
//
//! @brief This is called to clean up when a transfer operation is complete
//! or fails
//! @param ui32Status  - the status value that will be returned
//! @return            - ui32Status
//
//****************************************************************************
static uint32_t
pwr_terminate_transfer(uint32_t ui32Status)
{
    g_tGlb.i32NumCompleteCount = 0;
    g_tGlb.i32TxRxCount = 0;
    g_tGlb.bTransferRunning = false;
    g_tGlb.bTransferRunning = false;
    g_tGlb.eCurrentTransfer = e_no_transfer;
    GPIO_DBG1_RW_LOW;
    GPIO_DBG2_LOW;  //  Set debug 2 low when waiting for timer to start transmission sequence

    return ui32Status;
}

//*****************************************************************************
//
//! Main function.
//
//*****************************************************************************
int
main(void)
{

#if PRINT_ENABLE == 1
    am_bsp_debug_printf_enable();
#endif

    uint32_t ui32Stat = pwrctrl_set_low_power();
    if (ui32Stat)
    {
        STARTUP_PRINT("Pwr Control setup failure %d\nExample will Hang\n", ui32Stat);
        while (true);
    }

    //
    // Enable the debug print interface.
    //
#ifdef UNITY_PRINT_OUTPUT_UART
    g_ePrintDefault = AM_BSP_PRINT_IF_UART;
#endif


    //
    // Clear the terminal and print the banner.
    //

#if PRINT_ENABLE == 1
    am_util_stdio_terminal_clear();
#endif
    STARTUP_PRINT("MSPI PWR Example\n");


#if USE_PERIPHERAL != 0
    ui32Stat = mspi_init();
    if (ui32Stat)
    {
        STARTUP_PRINT("IOM Init failure %d\nExample will Hang\n", ui32Stat);
        while (1);

    }
#endif

    if (ui32Stat)
    {
        STARTUP_PRINT("MSPI Init failure %d\nExample will Hang\n", ui32Stat );
        while ( 1 );

    }

    debug_pin_init();

    //
    // TimerA0 init., 1 second callback, use 1024Khz XT clock
    //
    ui32Stat = timer_init(TMR_TICKS_PER_SECOND * 1, timerCallback);
    if (ui32Stat)
    {
        STARTUP_PRINT("Timer Init failure %d\nExample will Hang\n", ui32Stat );
        while ( 1 );
    }

    NVIC_SetPriority(gc_mspiIrq, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(gc_mspiIrq);
    NVIC_EnableIRQ(gc_mspiIrq);

    init_pattern();

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    STARTUP_PRINT("Periodically Driving (read/write) MSPI data - %d times\n", NUM_TX_LOOPS);


    //
    // when this is false, printing will be disabled after startup messages
    //

#if (PRINT_ENABLE == 1) && (PRINT_IN_LOOP == 0)
    //
    // wait for all prints to finish before disabling printing
    //
    am_util_delay_ms(50);

    //
    // printing may have been enabled at startup, but not in loop
    //
    am_bsp_debug_printf_disable();
#endif

    //
    // Loop forever (or until error)
    //
    while (true)
    {
        if (g_tGlb.eCurrentTransfer > e_no_transfer)
        {
            // there is an ongoing transfer
#if USE_PERIPHERAL != 0
            uint32_t ioStatus = pwr_data_transfer_sequence(g_tGlb.eCurrentTransfer);

            if (ioStatus)
            {
                DEBUG_PRINT("\nmspi io error mode: %u; WrtStat %lu\n", g_tGlb.eCurrentTransfer, ioStatus);
                break;
            }
#endif
        }
        else if (g_tGlb.bTimerFlag)
        {
            //
            // timer interrupt: start mspi sequence
            //
#if USE_PERIPHERAL != 0
            GPIO_DBG2_HI; //  Set debug 2 high during transmission period
            g_tGlb.bTimerFlag = false;
            // do read or write sequence
            g_tGlb.eCurrentTransfer = g_tGlb.eLastTransfer <= e_transfer_read ? e_transfer_write : e_transfer_read;
            g_tGlb.eLastTransfer = g_tGlb.eCurrentTransfer;
            uint32_t ioStatus = pwr_data_transfer_sequence(g_tGlb.eCurrentTransfer);
            if (ioStatus)
            {
                DEBUG_PRINT("\nstart mspi io error mode: %u; WrtStat %lu\n", g_tGlb.eCurrentTransfer, ioStatus);
                break;
            }
#endif

            DEBUG_PRINT("\nmspi sequence complete\n");
        }

        GPIO_SLEEP_LOW;
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        GPIO_SLEEP_HI;
    } // while(1)

    DEBUG_PRINT("\nEnd of MSPI PWR Example\n");

    //
    // Disable the timer Interrupt.
    //
    am_hal_timer_interrupt_disable(0);

    //
    // disable the interrupts in the NVIC.
    //
    timer_disable();
    NVIC_DisableIRQ(gc_mspiIrq);
    while (1);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
