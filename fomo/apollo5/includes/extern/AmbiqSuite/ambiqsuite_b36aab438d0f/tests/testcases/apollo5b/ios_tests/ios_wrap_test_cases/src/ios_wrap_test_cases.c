//*****************************************************************************
//
//! @file ios_wrap_test_cases.c
//!
//! @brief Example used for demonstrating the host burst write feature.
//!
//! Purpose: This slave component runs on one EB and is used in conjunction with
//! the companion host example, which runs on a second board.
//!
//! Printing takes place over the ITM at 1M Baud.
//!
//! Additional Information:
//! In order to run this example, a host device (e.g. a second board) must be set
//! up to run the host example, ios_fifo_host.  The two boards can be connected
//! using fly leads between the two boards as follows.
//!
//! @verbatim
//! Pin connections for the I/O Master board to the I/O Slave board.
//! SPI:
//!     HOST (ios_lram_host - Apollo3)          SLAVE (ios_lram - Apollo5B)
//!     --------------------                    ----------------
//!     GPIO[5]  IOM0 SPI SCK                   GPIO[11]  IOS SPI SCK
//!     GPIO[7]  IOM0 SPI MOSI                  GPIO[52]  IOS SPI MOSI
//!     GPIO[6]  IOM0 SPI MISO                  GPIO[83]  IOS SPI MISO
//!     GPIO[11] IOM0 SPI nCE                   GPIO[13]  IOS SPI nCE
//!     GPIO[42] SEND_REQ (host to slave)       GPIO[8]  SEND_REQ
//!     GPIO[43] REQ_ACK (slave to host)        GPIO[9]  REQ_ACK
//!     GND                                     GND
//!
//! I2C:
//!     HOST (ios_lram_host)                    SLAVE (ios_lram)
//!     --------------------                    ----------------
//!     GPIO[5]  IOM0 I2C SCL                   GPIO[11]  IOS I2C SCL
//!     GPIO[6]  IOM0 I2C SDA                   GPIO[52]  IOS I2C SDA
//!     GPIO[42] SEND_REQ (host to slave)       GPIO[8]  SEND_REQ
//!     GPIO[43] REQ_ACK (slave to host)        GPIO[9]  REQ_ACK
//!     GND                                     GND
//! @endverbatim
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "unity.h"
#include "ios_wrap_test_cases.h"

static void *g_pIOSHandle;

eIosState g_eIosState = IOS_STATE_IDLE;

volatile bool bIosSendReqInt = false;
volatile bool bIosRxComplete = false;
volatile bool bIosRegAcc01 = false;
volatile bool bIosRegAccThre1 = false;
volatile bool bIosRegAccThre2 = false;

uint16_t g_ui16RecvBufIdx = 0;
uint16_t g_ui16LRamReadPtr = 0;
uint16_t g_ui16IosRxLength = 0;

uint16_t g_ui16TestCnt = 0;
uint16_t g_ui16TestPassCnt = 0;

//*****************************************************************************
//
// Message buffers.
//
// data from the IOS interface, which is only 8 bits wide.
//
//*****************************************************************************
uint8_t         g_pIosRecvBuf[AM_IOM_TX_UNIT_SIZE];

//*****************************************************************************
//
// GPIO Configuration
//
//*****************************************************************************
const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_IOS_SEND_REQ =
{
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_8_GPIO,
    .GP.cfg_b.eIntDir        = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.eGPInput       = AM_HAL_GPIO_PIN_INPUT_ENABLE,
};

const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_IOS_REQ_ACK =
{
    .GP.cfg_b.uFuncSel            = AM_HAL_PIN_9_GPIO,
    .GP.cfg_b.eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
    .GP.cfg_b.eCEpol              = AM_HAL_GPIO_PIN_CEPOL_ACTIVEHIGH,
    .GP.cfg_b.eGPOutCfg           = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
    .GP.cfg_b.ePullup             = AM_HAL_GPIO_PIN_PULLUP_24K,
};

static const struct
{
    const uint32_t MHz;
    const char *MHzString;
} spiSpeedString[] =
{
    {AM_HAL_IOM_10KHZ,  "10KHZ"},
    {AM_HAL_IOM_50KHZ,  "50KHZ"},
    {AM_HAL_IOM_100KHZ, "100KHZ"},
    {AM_HAL_IOM_250KHZ, "250KHZ"},
    {AM_HAL_IOM_375KHZ, "375KHZ"},
    {AM_HAL_IOM_400KHZ, "400KHZ"},
    {AM_HAL_IOM_500KHZ, "500KHZ"},
    {AM_HAL_IOM_750KHZ, "750KHZ"},
    {AM_HAL_IOM_1MHZ,   "1MHZ"},
    {AM_HAL_IOM_1_5MHZ, "1.5MHZ"},
    {AM_HAL_IOM_2MHZ,   "2MHZ"},
    {AM_HAL_IOM_3MHZ,   "3MHZ"},
    {AM_HAL_IOM_4MHZ,   "4MHZ"},
    {AM_HAL_IOM_6MHZ,   "6MHZ"},
    {AM_HAL_IOM_8MHZ,   "8MHZ"},
    {AM_HAL_IOM_12MHZ,  "12MHZ"},
    {AM_HAL_IOM_16MHZ,  "16Mhz"},
    {AM_HAL_IOM_24MHZ,  "24Mhz"},
};

#define IOM_SPEED_SPI_MIN       10 // 2 MHz
#define IOM_SPEED_SPI_MAX       15  // 12 MHZ

//*****************************************************************************
//
// SPI Slave Configuration
//
//*****************************************************************************
static am_hal_ios_config_t g_sIOSSpiConfig =
{
    // Configure the IOS in SPI mode.
    .ui32InterfaceSelect = AM_HAL_IOS_USE_SPI,

    // Eliminate the "read-only" section, so an external host can use the
    // entire "direct write" section.
#if (IOS_WRAP_START_ADDRESS == 0x0)
    .ui32ROBase = IOS_WRAP_START_ADDRESS,
#elif (IOS_WRAP_START_ADDRESS == 0x78)
    .ui32ROBase = IOS_WRAP_START_ADDRESS,
#else
    .ui32ROBase = IOS_WRAP_START_ADDRESS - 0x8,
#endif

    // Making the "FIFO" section as big as possible.
#if (IOS_WRAP_START_ADDRESS == 0x78)
    .ui32FIFOBase = IOS_WRAP_START_ADDRESS + 0x8,
#else
    .ui32FIFOBase = IOS_WRAP_START_ADDRESS,
#endif

    // We don't need any RAM space, so extend the FIFO all the way to the end
    // of the LRAM.
    .ui32RAMBase = (IOS_WRAPAROUND_MAX_LRAM_SIZE + 0x8),         // FIFO MAX?

    // FIFO Threshold - set to half the size
    .ui32FIFOThreshold = 0x20,

    // Direct access wraparound enable.
    .ui8WrapEnabled = 1,
};

//*****************************************************************************
//
// I2C Slave Configuration
//
//*****************************************************************************
am_hal_ios_config_t g_sIOSI2cConfig =
{
    // Configure the IOS in I2C mode.
    .ui32InterfaceSelect = AM_HAL_IOS_USE_I2C | AM_HAL_IOS_I2C_ADDRESS(I2C_ADDR << 1),

    // Eliminate the "read-only" section, so an external host can use the
    // entire "direct write" section.
    .ui32ROBase = 0x78,

    // Set the FIFO base to the maximum value, making the "direct write"
    // section as big as possible.
    .ui32FIFOBase = 0x80,

    // We don't need any RAM space, so extend the FIFO all the way to the end
    // of the LRAM.
    .ui32RAMBase = 0x100,
    // FIFO Threshold - set to half the size
    .ui32FIFOThreshold = 0x40,
};

//*****************************************************************************
//
// IO Slave Main ISR.
//
//*****************************************************************************
void am_ioslave_ios_isr(void)
{
    uint32_t ui32Status;

    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //

    am_hal_ios_interrupt_status_get(g_pIOSHandle, false, &ui32Status);

    am_hal_ios_interrupt_clear(g_pIOSHandle, ui32Status);

//    am_hal_gpio_state_write(HANDSHAKE_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);   // clear SLINT anyway

    if (ui32Status & AM_HAL_IOS_INT_FUNDFL)
    {
        am_util_stdio_printf("Hitting underflow for the requested IOS FIFO transfer\n");
        // We should never hit this case unless the threshold has beeen set
        // incorrect, or we are unable to handle the data rate
        // ERROR!
//        am_hal_debug_assert_msg(0,
//            "Hitting underflow for the requested IOS FIFO transfer.");
    }

    if (ui32Status & AM_HAL_IOS_INT_ERR)
    {
        // We should never hit this case
        // ERROR!
//        am_hal_debug_assert_msg(0,
//            "Hitting ERROR case.");
    }

    if (ui32Status & AM_HAL_IOS_INT_FSIZE)
    {
        //
        // Service the I2C slave FIFO if necessary.
        //
        am_hal_ios_interrupt_service(g_pIOSHandle, ui32Status);
    }

    if (ui32Status & AM_HAL_IOS_INT_XCMPWR)
    {
        bIosRxComplete = true;
    }
}

void am_ioslave_acc_isr(void)
{
    uint32_t ui32Status;

    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //

    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTGET, &ui32Status);
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTCLR, &ui32Status);

    if (ui32Status & AM_HAL_IOS_ACCESS_INT_01)
    {
        bIosRegAcc01 = true;
        // Read packet length
        // change state to Accumulate data
    }

    if (ui32Status & IOS_REGACC_THRESHOLD_01)
    {
        bIosRegAccThre1 = true;
        // Read 1st half data from LRAM.
    }

    if (ui32Status & IOS_REGACC_THRESHOLD_02)
    {
        bIosRegAccThre2 = true;
        // Read 1st half data from LRAM.
    }
}

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void am_gpio0_001f_isr(void)
{
    //
    // Read and clear the GPIO interrupt status.
    //
    am_hal_gpio_mask_t IntStatus;
    uint32_t    ui32IntStatus;

    am_hal_gpio_interrupt_status_get(AM_HAL_GPIO_INT_CHANNEL_0,
                                     false,
                                     &IntStatus);
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_INT_CHANNEL_0, &IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_service(GPIO0_001F_IRQn,ui32IntStatus);
}

// ISR callback for the host IOINT
void ios_send_req_handler(void)
{
    bIosSendReqInt = true;
}

//*****************************************************************************
//
// Configure the SPI slave.
//
//*****************************************************************************
static void ios_set_up(bool bSpi)
{
    if (bSpi)
    {
        // Configure SPI interface
        am_bsp_ios_pins_enable(0, AM_HAL_IOS_USE_SPI);
        //
        // Configure the IOS interface and LRAM structure.
        //
        am_hal_ios_initialize(0, &g_pIOSHandle);
        am_hal_ios_power_ctrl(g_pIOSHandle, AM_HAL_SYSCTRL_WAKE, false);
        am_hal_ios_configure(g_pIOSHandle, &g_sIOSSpiConfig);
    }
    else
    {
        // Configure I2C interface
        am_bsp_ios_pins_enable(0, AM_HAL_IOS_USE_I2C);
        //
        // Configure the IOS interface and LRAM structure.
        //
        am_hal_ios_initialize(0, &g_pIOSHandle);
        am_hal_ios_power_ctrl(g_pIOSHandle, AM_HAL_SYSCTRL_WAKE, false);
        am_hal_ios_configure(g_pIOSHandle, &g_sIOSI2cConfig);
    }

    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_interrupt_clear(g_pIOSHandle, AM_HAL_IOS_INT_ALL);
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_INT_ERR | AM_HAL_IOS_INT_FSIZE);

#ifdef TEST_IOS_XCMP_INT
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_XCMP_INT);
#endif

    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
    NVIC_EnableIRQ(IOSLAVE_IRQn);

    uint32_t ui32Status = AM_HAL_IOS_ACCESS_INT_01;
    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTEN, &ui32Status);

    NVIC_EnableIRQ(IOSLAVEACC_IRQn);

    // Set up the IOS_REQ_ACK_PIN interrupt pin as generic output
    am_hal_gpio_pinconfig(IOS_REQ_ACK_PIN, g_AM_BSP_GPIO_IOS_REQ_ACK);
    am_hal_gpio_state_write(IOS_REQ_ACK_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);

    // Set up the IOS_SENT_REQ_PIN interrupt
    am_hal_gpio_pinconfig(IOS_SENT_REQ_PIN, g_AM_BSP_GPIO_IOS_SEND_REQ);

    // Set up the host IO interrupt
    uint32_t IntNum = IOS_SENT_REQ_PIN;
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_INT_CHANNEL_0, (am_hal_gpio_mask_t*)&IntNum);
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, IOS_SENT_REQ_PIN,
                                    (am_hal_gpio_handler_t)ios_send_req_handler, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);

    // Initialize RO & AHB-RAM data with pattern
    for (uint8_t i = 0; i < 8; i++)
    {
        am_hal_ios_pui8LRAM[0x78 + i] = ROBUFFER_INIT;
    }
}

#if IOM_IOS_CHECKSUM
static uint8_t get_checksum(uint8_t* pData, uint8_t len)
{
    uint32_t checksum = 0;
    for(uint8_t i = 0; i < len; i++)
    {
        checksum += pData[i];
    }

    return (uint8_t)(checksum & 0xff);
}
#endif

//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
}

void
tearDown(void)
{
}

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{
    am_bsp_itm_printf_enable();
    am_util_stdio_terminal_clear();

    am_util_stdio_printf("IOS LRAM Wraparound Example\n");
    am_util_stdio_printf("The pin jumpers on apollo5_eb are as follows:\n\n");
    am_util_stdio_printf("SPI:\n\n");
    am_util_stdio_printf("Pins    :   IOS                         IOM0\n");
    am_util_stdio_printf("11 to 5  :  IOS SPI SCK             to  IOM0 SPI SCK\n");
    am_util_stdio_printf("52 to 7  :  IOS SPI MOSI            to  IOM0 SPI MOSI\n");
    am_util_stdio_printf("83 to 6  :  IOS SPI MISO            to  IOM0 SPI MISO\n");
    am_util_stdio_printf("13 to 11 :  IOS SPI nCE             to  IOM0 SPI nCE\n");
    am_util_stdio_printf("8 to 42 :   Host Send Request                       \n");
    am_util_stdio_printf("9 to 43 :   Slave Ack Request                       \n\n");
    am_util_stdio_printf("I2C:\n\n");
    am_util_stdio_printf("Pins    :   IOS                         IOM0\n");
    am_util_stdio_printf("11 to 5  :  IOS I2C SCL             to  IOM0 I2C SCL\n");
    am_util_stdio_printf("52 to 6  :  IOS I2C SDA             to  IOM0 I2C SDA\n");
    am_util_stdio_printf("8 to 42 :   Host Send Request                       \n");
    am_util_stdio_printf("9 to 43 :   Slave Ack Request                       \n\n");
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
void
ios_wrap_test_operation(void)
{
#if IOM_IOS_CHECKSUM
    uint8_t ui8CheckSum = 0;
#endif

    uint32_t ui32Status;
    bool bTestPass = true;

    //
    // Enable the IOS. Choose the correct protocol based on USE_SPI
    //
    ios_set_up(USE_SPI);

    //
    // Enable interrupts so we can receive messages from the boot host.
    //
    am_hal_interrupt_master_enable();

    while(g_ui16TestCnt < ((IOM_SPEED_SPI_MAX - IOM_SPEED_SPI_MIN + 1) * 5))
    {
        switch(g_eIosState)
        {
            case IOS_STATE_IDLE:
                if(bIosSendReqInt)
                {
                    uint32_t inVal;
                    am_hal_gpio_state_read(IOS_SENT_REQ_PIN, AM_HAL_GPIO_INPUT_READ, &inVal);
                    //am_util_stdio_printf(" 1\n");
                    if(inVal == 1)
                    {
                        bIosSendReqInt = false;
                        g_eIosState = IOS_STATE_START;

                        uint32_t ui32Status = AM_HAL_IOS_ACCESS_INT_01;
                        am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTEN, &ui32Status);

                        am_hal_gpio_state_write(IOS_REQ_ACK_PIN, AM_HAL_GPIO_OUTPUT_SET);
                    }
                }
                break;
            case IOS_STATE_START:
                // Read size of length and change state to IOS_STATE_STREAM
                if(bIosRegAcc01)
                {
                    bIosRegAcc01 = false;
                    g_ui16IosRxLength = *((uint16_t *)am_hal_ios_pui8LRAM);
#if IOS_DBG_MSG
                    am_util_stdio_printf("Len = 0x%x\n", g_ui16IosRxLength);
#endif
                    g_eIosState = IOS_STATE_STREAM;
                    g_ui16LRamReadPtr = 2;

                    ui32Status = AM_HAL_IOS_ACCESS_INT_01;
                    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTDIS, &ui32Status);

                    ui32Status = IOS_REGACC_THRESHOLD_01 | IOS_REGACC_THRESHOLD_02;
                    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTEN, &ui32Status);
                }
                break;

            case IOS_STATE_STREAM:
                if(bIosRegAccThre1)
                {
                    bIosRegAccThre1 = false;
                    if(g_ui16RecvBufIdx == 0)
                    {
                        if(IOS_WRAP_START_ADDRESS > g_ui16LRamReadPtr)
                        {
                            ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&am_hal_ios_pui8LRAM[g_ui16LRamReadPtr], IOS_WRAP_START_ADDRESS - g_ui16LRamReadPtr);
                            g_ui16RecvBufIdx = g_ui16RecvBufIdx + IOS_WRAP_START_ADDRESS - g_ui16LRamReadPtr;
                            g_ui16LRamReadPtr = IOS_WRAP_START_ADDRESS;
                        }
                    }
                    else if((g_ui16RecvBufIdx + (IOS_WRAPAROUND_MAX_LRAM_SIZE - IOS_WRAP_HALF_ADDRESS)) < g_ui16IosRxLength)
                    {
                        ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&am_hal_ios_pui8LRAM[g_ui16LRamReadPtr], (IOS_WRAPAROUND_MAX_LRAM_SIZE - IOS_WRAP_HALF_ADDRESS));
                        g_ui16RecvBufIdx = g_ui16RecvBufIdx + (IOS_WRAPAROUND_MAX_LRAM_SIZE - IOS_WRAP_HALF_ADDRESS);
                        g_ui16LRamReadPtr = IOS_WRAP_START_ADDRESS;
                    }
#if IOS_DBG_MSG
                    am_util_stdio_printf("1 Idx=%d LIdx=%d\n", g_ui16RecvBufIdx, g_ui16LRamReadPtr);
#endif
                }
                else if(bIosRegAccThre2)
                {
                    bIosRegAccThre2 = false;
                    if((g_ui16RecvBufIdx + IOS_WRAP_HALF_ADDRESS - g_ui16LRamReadPtr) < g_ui16IosRxLength)
                    {
                        if(g_ui16RecvBufIdx == 0)
                        {
                            ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&am_hal_ios_pui8LRAM[g_ui16LRamReadPtr], IOS_WRAP_HALF_ADDRESS - g_ui16LRamReadPtr);
                            g_ui16RecvBufIdx = g_ui16RecvBufIdx + IOS_WRAP_HALF_ADDRESS - g_ui16LRamReadPtr;
                            g_ui16LRamReadPtr = IOS_WRAP_HALF_ADDRESS;
                        }
                        else
                        {
                            ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&am_hal_ios_pui8LRAM[g_ui16LRamReadPtr], (IOS_WRAP_HALF_ADDRESS - IOS_WRAP_START_ADDRESS));
                            g_ui16RecvBufIdx += (IOS_WRAP_HALF_ADDRESS - IOS_WRAP_START_ADDRESS);
                            g_ui16LRamReadPtr = IOS_WRAP_HALF_ADDRESS;
                        }
                    }
#if IOS_DBG_MSG
                    am_util_stdio_printf("2 Idx=%d LIdx=%d\n", g_ui16RecvBufIdx, g_ui16LRamReadPtr);
#endif
                }
                else if(bIosRxComplete)
                {
                    bIosRxComplete = false;
                    if((IOS_WRAPAROUND_MAX_LRAM_SIZE - g_ui16LRamReadPtr) >= (g_ui16IosRxLength - g_ui16RecvBufIdx))
                    {
                        ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&am_hal_ios_pui8LRAM[g_ui16LRamReadPtr], g_ui16IosRxLength - g_ui16RecvBufIdx);
                        g_ui16LRamReadPtr += (g_ui16IosRxLength - g_ui16RecvBufIdx);
                    }
                    else
                    {
                        ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&am_hal_ios_pui8LRAM[g_ui16LRamReadPtr], IOS_WRAPAROUND_MAX_LRAM_SIZE - g_ui16LRamReadPtr);
                        g_ui16RecvBufIdx = g_ui16RecvBufIdx + (IOS_WRAPAROUND_MAX_LRAM_SIZE - g_ui16LRamReadPtr);
                        g_ui16LRamReadPtr = IOS_WRAP_START_ADDRESS;
                        ios_memcpy(&g_pIosRecvBuf[g_ui16RecvBufIdx], (uint8_t*)&am_hal_ios_pui8LRAM[g_ui16LRamReadPtr], g_ui16IosRxLength - g_ui16RecvBufIdx);
                        g_ui16LRamReadPtr += (g_ui16IosRxLength - g_ui16RecvBufIdx);
                    }
                    g_ui16RecvBufIdx = g_ui16IosRxLength;
#if IOS_DBG_MSG
                    am_util_stdio_printf("3 Idx=%d LIdx=%d\n", g_ui16RecvBufIdx, g_ui16LRamReadPtr);
#endif
                    am_hal_gpio_state_write(IOS_REQ_ACK_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
                    g_ui16RecvBufIdx = g_ui16LRamReadPtr = 0;

#if IOM_IOS_CHECKSUM
                    ui8CheckSum = get_checksum(g_pIosRecvBuf, g_ui16IosRxLength - 1);
                    g_ui16TestCnt++;
                    if(ui8CheckSum != g_pIosRecvBuf[g_ui16IosRxLength - 1])
                    {
                        am_util_stdio_printf("Chk = 0x%x Buf[] = 0x%x\n", ui8CheckSum, g_pIosRecvBuf[g_ui16IosRxLength - 1]);
                        bTestPass = false;
                        //while(1);
                    }
                    else
                    {
                        g_ui16TestPassCnt++;
#if IOS_DBG_MSG
                        am_util_stdio_printf("R %d\n", g_ui16IosRxLength);
#endif
                    }
#endif
                    ui32Status = AM_HAL_IOS_ACCESS_INT_01 | IOS_REGACC_THRESHOLD_01 | IOS_REGACC_THRESHOLD_02;
                    am_hal_ios_control(g_pIOSHandle, AM_HAL_IOS_REQ_ACC_INTDIS, &ui32Status);
                    g_eIosState = IOS_STATE_IDLE;
                    if((g_ui16TestCnt % 5) == 0)
                    {
                        am_util_stdio_printf("%s %d/5 passed.\n", spiSpeedString[IOM_SPEED_SPI_MIN + g_ui16TestCnt / 5 - 1].MHzString, g_ui16TestPassCnt);
                        g_ui16TestPassCnt = 0;
                    }
                    if(g_ui16TestCnt == ((IOM_SPEED_SPI_MAX - IOM_SPEED_SPI_MIN + 1) * 5))
                    {
                        TEST_ASSERT(bTestPass);
                        if(bTestPass)
                            am_util_stdio_printf("Test Passed!\n");
                    }
                }
                break;
            default:
                break;
        }
    }
}

