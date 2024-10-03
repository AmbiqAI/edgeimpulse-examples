//*****************************************************************************
//
//! @file pwr_iom_spi.c
//!
//! @brief Example for IOM SPI DMA power measurements.
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup pwr_iom_spi SPI Power Example
//! @ingroup power_examples
//! @{
//!
//! Purpose: Example for IOM SPI DMA power measurements.
//!
//! @details Simple example that uses HAL to queue multiple SPI DMA transfers.
//!
//!    1. Enter deepsleep during DMA transfer.
//!    2. DMA write-loop or read-loop every 1s.
//!    3. Have configurable SPI buffer size.
//!    4. GPIO indicator for CPU wake state.
//!
//! Pin connections depend on IOM chosen
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
#if defined(apollo4l_evb) || defined(apollo4l_blue_evb)
#define IOM_MODULE       5   //!
#else
#define IOM_MODULE       7   //!< Choose iom module, take care with potential SPI and debug pin conflicts
#endif

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
//! IOM Pin function defs, each pin can be set to
//!     - disabled (off),
//!     - GPIO High
//!     - GPIO Low
//!     - Normal IOM behavior
//! This stuct and the pin enums used are declared in pwr_iom_utils.h
//
static const iom_pins_setup_t iomPinSetup =
{
    .ui32Iom_number = IOM_MODULE,
    .tMosi_pin = eIOM_PIN_NOMRAL,
    .tMiso_pin = eIOM_PIN_NOMRAL,
    .tSCK_pin  = eIOM_PIN_NOMRAL,
    .tCS_pin   = eIOM_PIN_NOMRAL,
    .tIOM_mode = AM_HAL_IOM_SPI_MODE,
};


//
//! Collect the ram variables used in this file in this struct
//
typedef struct
{
    void *g_IOMHandle;                          //!< IOM handle
    uint32_t ui32TestData[TESTDATASIZE + 8];    //!< Test data tx buff,
    uint32_t ui32RxBuff[TESTDATASIZE + 8];      //!< Test data rx buff
    uint32_t DMATCBBuffer[2048];                //!< Command buffer
    int32_t i32TxRxCount;                       //!< counts the number of started transfers the current sequence
    int32_t i32NumCompleteCount;                //!< counts the number of completed transfers the current sequence
    uint32_t ui32DebugPinNum[eMAX_DEBUG_PINS];  //!< Debug pin numbers needs to be set early in startup

    volatile bool bIomComplete;     //!< set (at isr level) in iom isr when transfer completes
    volatile bool bTimerFlag;       //!< set (at isr level) in timer callback when timer expires
    bool bTransferRunning;          //!< an spi transfer has been started and is in progress
    //
    // This code alternates between read and write SPI io, These variables track the mode
    //
    transfer_options_e eCurrentTransfer;         //!< current transfer mode
    transfer_options_e eLastTransfer;            //!< last transfer mode, used to toggle transfer mode
}
globasx_t;

//
//! allocate RAM variables used
//
static globasx_t g_tGlb;

//!
//! precompute IOM isr vector number
//!
const IRQn_Type gc_iomIrq = (IRQn_Type) (IOM_MODULE + IOMSTR0_IRQn);

//
//! This is an SPI example, this is always true
//
static const bool g_bSpi = true;

//
//! IOM setup struct, set up for SPI init
//
static const am_hal_iom_config_t tIomConfig =
{
    .eInterfaceMode = AM_HAL_IOM_SPI_MODE,
    .ui32ClockFreq  = IOM_FREQ,
    .eSpiMode = AM_HAL_IOM_SPI_MODE_0,
    .pNBTxnBuf = g_tGlb.DMATCBBuffer,
    .ui32NBTxnBufLength = sizeof(g_tGlb.DMATCBBuffer) / 4,
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

//! @}
// end of global variable defs

//!
//! Static function prototypes
//! @{
//!
static uint32_t iom_dma_read(bool bSpi, uint32_t offset, uint32_t *pTxBuf, uint32_t *pRxBuf, uint32_t size);
static uint32_t iom_dma_write(bool bSpi, uint32_t offset, uint32_t *pTxBuf, uint32_t *pRxBuf, uint32_t size);
static void timerCallback(void);
static void init_pattern(void);
static uint32_t iom_init(void);
static uint32_t pwr_data_transfer_sequence(transfer_options_e eTransferType);
static uint32_t pwr_terminate_transfer(uint32_t ui32Status);

//! @}
// end of static function prototypes

//*****************************************************************************
//
//! @brief     Timer callback
//!
//! This is called from the timer isr when the timer expires
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
//! @brief     IOM Complete callback
//! Called when the iom non-blocking transfer completes
//!
//! @param pCallbackCtxt  - unused
//! @param ui32TransactionStatus  - unused
//
//*****************************************************************************
static void
iomCompleteCallback(void *pCallbackCtxt, uint32_t ui32TransactionStatus)
{
    (void) pCallbackCtxt;
    (void) ui32TransactionStatus;

    g_tGlb.bIomComplete = true;

}

//
//! Take over the interrupt handler for whichever IOM is used.
//
#define iom_isr am_iom_isrx(IOM_MODULE)
#define am_iom_isrx(n) am_iom_isr(n)
#define am_iom_isr(n)  am_iomaster ## n ## _isr


void iom_isr(void);     //!< iom isr prototype
//*****************************************************************************
//
//! IOM ISRs
//! Take over correct IOM ISR.
//
//*****************************************************************************
void
iom_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_tGlb.g_IOMHandle, true, &ui32Status))
    {
        if (ui32Status)
        {
            am_hal_iom_interrupt_clear(g_tGlb.g_IOMHandle, ui32Status);
            am_hal_iom_interrupt_service(g_tGlb.g_IOMHandle, ui32Status);
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
    pwrctrl_fill_buff_alternating_bits((uint8_t *) g_tGlb.ui32TestData, 1, TESTDATASIZE * 4);

}

//*****************************************************************************
//
//! @brief init the IOM
//!
//! @return standard hal status
//
//*****************************************************************************
static uint32_t
iom_init(void)
{
    uint32_t ui32IomStatus;
    do
    {
        //
        // get iom pins set-up
        //
        am_bsp_iom_pins_enable(IOM_MODULE, AM_HAL_IOM_SPI_MODE);

        ui32IomStatus = am_hal_iom_initialize(IOM_MODULE, &g_tGlb.g_IOMHandle);
        if (ui32IomStatus)
        {
            break;
        }

        ui32IomStatus = am_hal_iom_power_ctrl(g_tGlb.g_IOMHandle, AM_HAL_SYSCTRL_WAKE, false);
        if (ui32IomStatus)
        {
            break;
        }

        ui32IomStatus = am_hal_iom_configure(g_tGlb.g_IOMHandle, &tIomConfig);
        if (ui32IomStatus)
        {
            break;
        }
        ui32IomStatus = am_hal_iom_enable(g_tGlb.g_IOMHandle);
        if (ui32IomStatus)
        {
            break;
        }

        ui32IomStatus = pwr_iom_gpio_setup(&iomPinSetup);
        if (ui32IomStatus)
        {
            break;
        }

    }
    while (false);

    return ui32IomStatus;
}

//*****************************************************************************
//
//! @brief iom_dma_read, spi read data buffer
//!
//! @param bSpi         - true (spi) in this example
//! @param offset       - not used
//! @param pTxBuff      - input/output buffer used
//! @param pRxBuff      - input/output buffer used
//! @param size         - number of samples to transfer
//!
//! @return             - standard hal status
//
//*****************************************************************************
static uint32_t
iom_dma_read(bool bSpi, uint32_t offset, uint32_t *pTxBuf, uint32_t *pRxBuf, uint32_t size)
{
    am_hal_iom_transfer_t Transaction;
    Transaction.ui8Priority = 1;        // High priority for now.
    Transaction.eDirection = AM_HAL_IOM_RX;
    Transaction.ui32InstrLen = 0;
    Transaction.ui64Instr = 0; //(ui32ReadAddress & 0x0000FFFF);
    Transaction.ui32NumBytes = size;
    Transaction.pui32TxBuffer = pTxBuf;
    Transaction.pui32RxBuffer = pRxBuf;
    Transaction.uPeerInfo.ui32I2CDevAddr = 0;
    Transaction.ui8RepeatCount = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    Transaction.bContinue = false;

    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

    return am_hal_iom_nonblocking_transfer(g_tGlb.g_IOMHandle,
                                           &Transaction,
                                           iomCompleteCallback,
                                           0);    //iom_transfer_complete
}

//*****************************************************************************
//
//! @brief    iom_dma_write, spi write buffer
//!
//! @param bSpi         - true (spi) in this example
//! @param offset       - not used
//! @param pTxBuf       - output buffer used
//! @param pRxBuf       - input buffer used  - unused here
//! @param size         - number of samples to transfer
//!
//! @return             - standard hal status
//
//*****************************************************************************
static uint32_t
iom_dma_write(bool bSpi, uint32_t offset, uint32_t *pTxBuf, uint32_t *pRxBuf, uint32_t size)
{
    am_hal_iom_transfer_t Transaction;
    Transaction.ui8Priority = 1;        // High priority for now.
    Transaction.eDirection = AM_HAL_IOM_TX;
    Transaction.ui32InstrLen = 0;
    Transaction.ui64Instr = 0; //(ui32ReadAddress & 0x0000FFFF);
    Transaction.ui32NumBytes = size;
    Transaction.pui32TxBuffer = (uint32_t *) pTxBuf;
    Transaction.pui32RxBuffer = (uint32_t *) pRxBuf;
    Transaction.uPeerInfo.ui32I2CDevAddr = 0;
    Transaction.ui8RepeatCount = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    Transaction.bContinue = false;

    Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;

    return am_hal_iom_nonblocking_transfer(g_tGlb.g_IOMHandle,
                                           &Transaction,
                                           iomCompleteCallback,
                                           0);    //iom_transfer_complete
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
    STARTUP_PRINT("IOM SPI PWR Example\n");


#if USE_PERIPHERAL != 0
    ui32Stat = iom_init();
    if (ui32Stat)
    {
        STARTUP_PRINT("IOM Init failure %d\nExample will Hang\n", ui32Stat);
        while (1);

    }
#endif

    // init the debug pins
    if (g_bGpioEnabled)
    {
        //
        // populate the debug pin number array
        //
        pwr_iom_getDebugPins(&iomPinSetup, g_tGlb.ui32DebugPinNum, eMAX_DEBUG_PINS);

        //
        // the RAM array g_tGlb.ui32DebugPinNum must be populated before this call
        //
        ui32Stat = gpio_dbg_pin_init(g_tGlb.ui32DebugPinNum);
        if (ui32Stat)
        {
            STARTUP_PRINT("Gpio Init failure %d\nExample will Hang\n", ui32Stat);
            while (true);
        }
    }

    //
    // TimerA0 init., 1 second callback, use 1024Khz XT clock
    //
    ui32Stat = timer_init(TMR_TICKS_PER_SECOND * 1, timerCallback);
    if (ui32Stat)
    {
        STARTUP_PRINT("Timer Init failure %d\nExample will Hang\n", ui32Stat);
        while (1);
    }

    NVIC_ClearPendingIRQ(gc_iomIrq);
    NVIC_EnableIRQ(gc_iomIrq);

    init_pattern();

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
            GPIO_DBG2_HI; //  Set debug 2 high during transmission period
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
        GPIO_SLEEP_HI;      // this would be a bit better if this were in the waking ISRs

    } // while(1)

    DEBUG_PRINT("\nEnd of SPI PWR Example\n");

    //
    // Disable the timer Interrupt.
    //
    am_hal_timer_interrupt_disable(0);

    //
    // disable the interrupts in the NVIC.
    //
    timer_disable();
    NVIC_DisableIRQ(gc_iomIrq);
    while (1);
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
            ui32TxRxStat = iom_dma_read(g_bSpi, 0, g_tGlb.ui32TestData, g_tGlb.ui32RxBuff, TESTDATASIZE * 4 - 8);
            g_tGlb.bTransferRunning = true;

        }
        else if (eTransferType == e_transfer_write)
        {
            GPIO_DBG1_RW_HI; //  Set debug 1 high when starting transmission
            ui32TxRxStat = iom_dma_write(g_bSpi, 0, g_tGlb.ui32TestData, g_tGlb.ui32RxBuff, TESTDATASIZE * 4 - 8);
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
// End Doxygen group.
//! @}
//
//*****************************************************************************
