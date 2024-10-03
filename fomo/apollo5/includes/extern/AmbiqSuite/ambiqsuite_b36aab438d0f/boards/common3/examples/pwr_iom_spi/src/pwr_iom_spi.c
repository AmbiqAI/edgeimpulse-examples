//*****************************************************************************
//
//! @file pwr_iom_spi.c
//!
//! @brief Example for IOM SPI DMA power measurements.
//!
//! Purpose: Simple example that uses HAL to queue multiple SPI DMA transfers.
//!
//!
//! @verbatim
//!    1. Enter deepsleep during DMA transfer.
//!    2. DMA write and read-loop every 1s.
//!    3. Have configurable SPI buffer size.
//!    4. GPIO indicator for CPU wake state.
//!    5. Run for a configurable number of loops.
//!
//!
//! Pin connections
//! SPI:
//!     This varies depending on IOM selected
//!     pins used for default IOM 0:
//!         GPIO[5]  IOM0 SPI SCK
//!         GPIO[7]  IOM0 SPI MOSI
//!         GPIO[6]  IOM0 SPI MISO
//!         GPIO[50] IOM0 SPI nCE0
//!
//! Debug Pins Used
//!     ReadLow_WriteHigh 0
//!     Sequence          3
//!     Sleep             2
//! @endverbatim
//!
//! @addtogroup ap3x_power_examples Power Examples
//!
//! @defgroup pwr_iom_spi SPI Power Example
//! @ingroup ap3x_power_examples
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

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "timer_utils.h"
#include "pwr_gpio_utils.h"
#include "pwr_control_utils.h"
#include "pwr_iom_utils.h"


//*****************************************************************************
// Customize the following for the test
//*****************************************************************************

#define PRINT_ENABLE 0       //!< Enabled printing during startup

#define PRINT_IN_LOOP 0      //!< Enables printing in the main loop, usually off for low power

#define GPIO_DEBUG_ENABLE 0  //!< when set to 1 the debug pins are enabled

#define NUM_TX_LOOPS    10   //!< number of non-blocking (read or write) transmissions run per sequence

//! @note choose default IOM depending on board used
//! When using a different IOM, or custom board,
//! the user is advised to check schematic for pin conflicts:
//! pins are defined in pwr_iom_utils.c
#define IOM_MODULE       0

#define IOM_FREQ AM_HAL_IOM_1MHZ       //!< This is the clock frequency for the IOM SPI

#define TESTDATASIZE (192*4)           //!< Number of 32bit words allocated for test data

//! setting USE_PERIPHERAL to zero will disable the IOM module and put the device into
//! low power deepsleep
//! setting USE_PERIPHERAL to 1 will enable the peripheral and allow peripheral power measurement
//! The power and retention settings are specified in pwr_pwerControl_utils.c
//! enable PERIPHERAL (IOM SPI) (this should usually be 1)
#define USE_PERIPHERAL 1

#if IOM_MODULE >= AM_REG_IOM_NUM_MODULES
#error "invalid iom value"
#endif

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

const IRQn_Type g_iomIrq = IOMSTR0_IRQn;


typedef enum
{
    e_no_transfer,
    e_transfer_read,
    e_transfer_write,
}
transfer_options_e;


typedef struct
{
    void *g_IOMHandle;
    volatile uint32_t  ui32IsrCount;
    volatile uint32_t  ui32IsrPCount;
    uint32_t  ui32MainLoopCount;
    uint32_t  ui32TxSampleCount;
    uint32_t  ui32RxSampleCount;
    uint32_t  ui32TransferLoops;
    volatile uint32_t  ui32CallbackCount;
    volatile uint32_t ui32StartIdx;

    volatile bool bIomComplete;
    volatile bool bTimerFlag;
    int32_t i32TxRxCount;                       //!< counts the number of started transfers the current sequence
    int32_t i32NumCompleteCount;                //!< counts the number of completed transfers the current sequence
    uint32_t ui32DebugPinNum[eMAX_DEBUG_PINS];  //!< Debug pin numbers needs to be set early in startup

    bool bTransferRunning;          //!< an spi transfer has been started and is in progress
    //
    // This code alternates between read and write SPI io, These variables track the mode
    //
    transfer_options_e eCurrentTransfer;         //!< current transfer mode
    transfer_options_e eLastTransfer;            //!< last transfer mode, used to toggle transfer mode

}
globasx_t;

//
//! Apollo3x: memory used with DMA should not be in TCM.
//
typedef struct 
{
    uint32_t DMATCBBuffer[2048];                //!< Command buffer
    uint32_t ui32TestData[TESTDATASIZE + 8];    //!< Test data tx buff,
    uint32_t ui32RxBuff[TESTDATASIZE + 8];      //!< Test data rx buff
}
dma_buffers_t;


//
//! allocate ram variables used, target TCM (DTCM) fast RAM
//
#ifdef AM_PART_APOLLO3P
__attribute__((section(".tcm")))
#endif
globasx_t g_tGlb;

//
//! Apollo3x: memory used with DMA should not be in TCM.
//
#ifdef AM_PART_APOLLO3P
__attribute__((section("readwrite")))
#endif
dma_buffers_t g_tDMA;

static const uint32_t ui32IomModuleNumber = IOM_MODULE;

//
//! IOM Pin function defs, each pin can be set to
//!     - disabled (off),
//!     - GPIO High
//!     - GPIO Low
//!     - Normal IOM behavior
//! This stuct and the pin enums used are declared in pwr_iom_utils.h
//
static const iom_pins_setup_t iomPinSetup =
{
    .ui32IomNumber = IOM_MODULE,
    .eMosiPin = IOM_PIN_NOMRAL,
    .eMisoPin = IOM_PIN_NOMRAL,
    .eSCKPin  = IOM_PIN_NOMRAL,
    .eCSPin   = IOM_PIN_NOMRAL,
    .eIomMode = AM_HAL_IOM_SPI_MODE,
};

//*****************************************************************************
//
// Configuration structure for the IO Master.
//
//*****************************************************************************
static am_hal_iom_config_t g_sIOMSpiConfig =
{
    .eInterfaceMode = AM_HAL_IOM_SPI_MODE,
          .ui32ClockFreq = AM_HAL_IOM_24MHZ,
    //      .ui32ClockFreq = AM_HAL_IOM_12MHZ,
      //.ui32ClockFreq = AM_HAL_IOM_8MHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_6MHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_4MHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_3MHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_2MHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_1_5MHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_1MHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_750KHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_500KHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_400KHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_375KHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_250KHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_100KHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_50KHZ,
    //    .ui32ClockFreq = AM_HAL_IOM_10KHZ,
    .eSpiMode = AM_HAL_IOM_SPI_MODE_0,
    .ui32NBTxnBufLength = sizeof(g_tDMA.DMATCBBuffer) / 4, // compute number of longwords
    .pNBTxnBuf = &g_tDMA.DMATCBBuffer[0],
};



//
//! set up debug function #defines\n
//! (disable gpio functions if debug pins aren't used)
//
#if GPIO_DEBUG_ENABLE == 1
const bool g_bGpioEnabled = true;
#define GPIO_DBG1_RW_HI am_hal_gpio_output_set(g_tGlb.ui32DebugPinNum[eDEBUG_PIN_RD_LOW_WRT_HI])
#define GPIO_DBG1_RW_LOW am_hal_gpio_output_clear(g_tGlb.ui32DebugPinNum[eDEBUG_PIN_RD_LOW_WRT_HI])
#define GPIO_DBG2_HI am_hal_gpio_output_set(g_tGlb.ui32DebugPinNum[eDEBUG_PIN_2])
#define GPIO_DBG2_LOW am_hal_gpio_output_clear(g_tGlb.ui32DebugPinNum[eDEBUG_PIN_2])
#define GPIO_SLEEP_HI am_hal_gpio_output_set(g_tGlb.ui32DebugPinNum[eDEBUG_PIN_SLEEP])
#define GPIO_SLEEP_LOW am_hal_gpio_output_clear(g_tGlb.ui32DebugPinNum[eDEBUG_PIN_SLEEP])
#else
const bool g_bGpioEnabled = false;
#define GPIO_DBG1_RW_HI
#define GPIO_DBG1_RW_LOW
#define GPIO_DBG2_HI
#define GPIO_DBG2_LOW
#define GPIO_SLEEP_HI
#define GPIO_SLEEP_LOW
#endif


static void iom_transfer_complete(void *pCallbackCtxt, uint32_t transactionStatus);
void am_iomaster0_isr(void);
static uint32_t iom_dma_read(uint32_t offset, uint32_t *pTxBuf, uint32_t *pRxBuf, uint32_t size);
static uint32_t iom_dma_write(uint32_t offset, uint32_t *pTxBuf, uint32_t *pRxBuf, uint32_t size);
static void timerCallback(void *x);
static void init_pattern(void);
static uint32_t pwr_data_transfer_sequence(transfer_options_e eTransferType);
static uint32_t pwr_terminate_transfer(uint32_t ui32Status);

//
//! prototype for read-write function pointer
//
//! called when the timer expires (1hz)
//
static void 
timerCallback(void *x)
{
    g_tGlb.bTimerFlag = true;
}

//*****************************************************************************
//
// am_iomaster0_isr  
//
//*****************************************************************************
void 
am_iomaster0_isr(void)
{
    g_tGlb.ui32IsrCount++;
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_tGlb.g_IOMHandle, true, &ui32Status))
    {
        if (ui32Status)
        {
            g_tGlb.ui32IsrPCount++;
            am_hal_iom_interrupt_clear(g_tGlb.g_IOMHandle, ui32Status);
            am_hal_iom_interrupt_service(g_tGlb.g_IOMHandle, ui32Status);
        }
    }
    //am_hal_gpio_output_clear (DEBUG_PIN_1);//
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
    pwrctrl_fill_buff_alternating_bits((uint8_t *) g_tDMA.ui32TestData, 1, TESTDATASIZE * 4);

}
//*****************************************************************************
//
// IOM Write Callback - Send next block of data.
//
//*****************************************************************************
static void
iom_transfer_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{
    g_tGlb.bIomComplete = true;
    g_tGlb.ui32CallbackCount++;
}

//*****************************************************************************
//
//! @brief iom_dma_read
//!
//! @param offset
//! @param pTxBuf
//! @param pRxBuf
//! @param size
//
//*****************************************************************************
static uint32_t
iom_dma_read(uint32_t offset, uint32_t *pTxBuff, uint32_t *pRxBuf, uint32_t size)
{
    am_hal_iom_transfer_t Transaction;

    Transaction.ui32InstrLen = 0;
    Transaction.ui32Instr = offset;
    Transaction.eDirection = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes = size;
    Transaction.pui32TxBuffer = pTxBuff;
    Transaction.pui32RxBuffer = pRxBuf;
    Transaction.bContinue = false;   // keep chip select asserted
    Transaction.ui8RepeatCount = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

    return am_hal_iom_nonblocking_transfer(g_tGlb.g_IOMHandle,
                                           &Transaction,
                                           iom_transfer_complete,
                                           0);    //iom_transfer_complete
}

//*****************************************************************************
//
//! @brief    iom_dma_write spi write buffer
//! @param offset - unused
//! @param pBuf
//! @param size
//
//*****************************************************************************
static uint32_t
iom_dma_write(uint32_t offset, uint32_t *pTxBuff, uint32_t *pRxBuf, uint32_t size)
{
    am_hal_iom_transfer_t Transaction;

    Transaction.ui32InstrLen = 1;
    Transaction.ui32Instr = offset;
    Transaction.eDirection = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes = size;
    Transaction.pui32TxBuffer = pTxBuff;
    Transaction.pui32RxBuffer = pRxBuf;
    Transaction.bContinue = false;
    Transaction.ui8RepeatCount = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

    return am_hal_iom_nonblocking_transfer(g_tGlb.g_IOMHandle,
                                           &Transaction,
                                           iom_transfer_complete,
                                           0);    //iom_transfer_complete
}

//*****************************************************************************
//
//! @brief
//! @param iomModule
//! @param bSpi
//
//*****************************************************************************
static uint32_t 
iom_init(uint32_t iomModule)
{
    uint32_t ui32IomStatus;

    do
    {
        //
        // Initialize the IOM.
        //
        ui32IomStatus = am_hal_iom_initialize(iomModule, &g_tGlb.g_IOMHandle);
        if (ui32IomStatus)
        {
            break;
        }

        ui32IomStatus = am_hal_iom_power_ctrl(g_tGlb.g_IOMHandle, AM_HAL_SYSCTRL_WAKE, false);
        if (ui32IomStatus)
        {
            break;
        }

        //
        // Set the required configuration settings for the IOM.
        //
        ui32IomStatus = am_hal_iom_configure(g_tGlb.g_IOMHandle, &g_sIOMSpiConfig);
        if (ui32IomStatus)
        {
            break;
        }

        //
        // Configure the SPI/IOM pins.
        //
        ui32IomStatus = pwr_iom_gpio_setup(&iomPinSetup);
        if (ui32IomStatus)
        {
            break;
        }

        if (g_bGpioEnabled)
        {
            //
            // init the debug pins, extract from the Specific IOM pin assignment table
            //
            ui32IomStatus = pwr_iom_getDebugPins(&iomPinSetup, g_tGlb.ui32DebugPinNum, eMAX_DEBUG_PINS);
            if (ui32IomStatus)
            {
                break;
            }
            //
            // debug pin setups
            //
            ui32IomStatus = gpio_dbg_pin_init(g_tGlb.ui32DebugPinNum);
            if (ui32IomStatus)
            {
                break;
            }


            am_hal_gpio_output_clear (DEBUG_PIN_RD_LO);
            am_hal_gpio_output_clear (DEBUG_PIN_2);
            am_hal_gpio_output_clear (DEBUG_PIN_SLEEP);
        }


        //
        // Enable the IOM.
        //
        ui32IomStatus = am_hal_iom_enable(g_tGlb.g_IOMHandle);
        if (ui32IomStatus)
        {
            break;
        }
    } while(false);

    return ui32IomStatus;
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
    //
    // This flag should not be set at this point, but clear it, just-in-case.
    // This is set in the timer isr, and here it is used to timeout this
    // function's main loop.
    //
    if (g_tGlb.bTimerFlag)
    {
        //
        // this is a timout error, took too long
        //
        g_tGlb.bTimerFlag = false;
        DEBUG_PRINT("\nspi io timeout %lu %lu %lu\n", g_tGlb.i32TxRxCount, g_tGlb.i32NumCompleteCount, eTransferType);
        return pwr_terminate_transfer(1);
    }

    if (g_tGlb.bIomComplete)
    {
        //
        // last transfer completed
        //
        g_tGlb.bIomComplete = false;

        if (g_tGlb.bTransferRunning)
        {
            g_tGlb.bTransferRunning = false;

            if (++g_tGlb.i32NumCompleteCount >= NUM_TX_LOOPS)
            {
                //
                // all transfers complete
                //
                DEBUG_PRINT("\nspi io transmit sequence complete %lu\n", eTransferType);
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
            GPIO_DBG1_RW_LOW; //  Set debug 1 high when starting transmission
            ui32TxRxStat = iom_dma_read(0, g_tDMA.ui32TestData, g_tDMA.ui32RxBuff, TESTDATASIZE * 4 - 8);
            g_tGlb.bTransferRunning = true;

        }
        else if (eTransferType == e_transfer_write)
        {
            GPIO_DBG1_RW_HI; //  Set debug 1 high when starting transmission
            ui32TxRxStat = iom_dma_write(0, g_tDMA.ui32TestData, g_tDMA.ui32RxBuff, TESTDATASIZE * 4 - 8);
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
}

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
// Main function.
//
//*****************************************************************************
int main(void)
{
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

#if PRINT_ENABLE == 1
    am_bsp_itm_printf_enable();
    am_bsp_debug_printf_enable();
#endif
    uint32_t ui32Stat = pwrctrl_set_low_power();
    if (ui32Stat)
    {
        STARTUP_PRINT("Pwr Control setup failure %d\nExample will Hang\n", ui32Stat);
        while (true);
    }

#if PRINT_ENABLE == 1
        am_util_stdio_terminal_clear();
#endif
    STARTUP_PRINT("IOM SPI PWR Example\n");

    init_pattern();

    //
    // Enable Interrupts.
    //
    am_hal_interrupt_master_enable();
#if USE_PERIPHERAL != 0

    ui32Stat = iom_init(ui32IomModuleNumber);
    if (ui32Stat)
    {
        STARTUP_PRINT("IOM Init failure %d\nExample will Hang\n", ui32Stat);
        while (1);
    }
#endif

    timer_init( TMR_TICKS_PER_SECOND, timerCallback);

    g_tGlb.bIomComplete = false;

    //
    // Enable IOM Interrupts
    //
    // Available IOM interrupts: IOM0_INTEN_DCMP_Pos | IOM0_INTEN_CMDCMP_Pos | IOM0_INTEN_THR_Pos | IOM0_INTEN_CQUPD_Pos) );
    am_hal_iom_interrupt_enable(g_tGlb.g_IOMHandle,IOM0_INTEN_DCMP_Pos);    //Enable only the

    // IOM0 DMA Complete Interrupt
    NVIC_SetPriority(g_iomIrq, SPI_TIMER_PRIORITY);
    NVIC_ClearPendingIRQ(g_iomIrq);
    NVIC_EnableIRQ(g_iomIrq);  //Enanble main IOM interrupt

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    STARTUP_PRINT("Periodically Driving (read/write) SPI data - %d times\n", NUM_TX_LOOPS);

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
                DEBUG_PRINT("\nspi io error mode: %u; WrtStat %lu\n", g_tGlb.eCurrentTransfer, ioStatus);
                break;
            }
#endif
        }
        else if (g_tGlb.bTimerFlag)
        {
            //
            // timer interrupt: start spi sequence
            //
#if USE_PERIPHERAL != 0
            GPIO_DBG2_HI; //  Set debug 2 high during the transmission period
            g_tGlb.bTimerFlag = false;
            // do read or write sequence
            g_tGlb.eCurrentTransfer = g_tGlb.eLastTransfer <= e_transfer_read ? e_transfer_write : e_transfer_read;
            g_tGlb.eLastTransfer = g_tGlb.eCurrentTransfer;
            uint32_t ioStatus = pwr_data_transfer_sequence(g_tGlb.eCurrentTransfer);
            if (ioStatus)
            {
                DEBUG_PRINT("\nstart spi io error mode: %u; WrtStat %lu\n", g_tGlb.eCurrentTransfer, ioStatus);
                break;
            }
#endif

            DEBUG_PRINT("\nspi sequence complete\n");
        }

        GPIO_SLEEP_LOW;
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        GPIO_SLEEP_HI;
    } // while

    DEBUG_PRINT("\nEnd of SPI PWR Example\n");

    //
    // Disable the timer and its Interrupt.
    //
    
    timer_disable();
    
    NVIC_DisableIRQ(g_iomIrq);
    while (1);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************


