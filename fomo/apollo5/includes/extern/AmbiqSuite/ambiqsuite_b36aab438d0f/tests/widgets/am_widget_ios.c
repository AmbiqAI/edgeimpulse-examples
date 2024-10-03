//*****************************************************************************
//
//! @file am_widget_ios.c
//!
//! @brief Widget to test IOS using IOM as stimulus
//!
//! This program initiates a SPI/I2C transaction at specified address/size,
//! For access to LRAM locations (address != 0x7F), IOM sends a pattern to IOS,
//! IOS receives it, manipulates the data and send it back to IOM for
//! verification.
//! For access to FIFO (address == 0x7F), a pattern is send from IOS to IOM,
//! which is then verified for accuracy.
//!
//! SWO is configured in 1M baud, 8-n-1 mode.
//! @verbatim
//! PIN Fly Lead Assumptions for the I/O Master (IOM) Board (the other one):
//!        GPIO[5] == IOM0 SPI SCK/ I2C SCL
//!        GPIO[6] == IOM0 SPI MISO/I2C SDA
//!        GPIO[7] == IOM0 SPI MOSI
//!        GPIO[11] == IOM0 SPI nCE
//!        GPIO[10] == Handshake Interrupt
//!
//! PIN Fly Lead Assumptions for the I/O Slave (IOS) Board (this one):
//!        GPIO[0] == IOS SPI SCK/I2C SCL
//!        GPIO[1] == IOS SPI MISO/ I2C SDA
//!        GPIO[2] == IOS SPI MOSI
//!        GPIO[3] == IOS SPI nCE
//!        GPIO[4] == IOS Interrupt
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

#include <string.h>
#include <ctype.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "am_widget_ios.h"


// Hardware Issues - fixed in B2
// SHELBY_1643
// SHELBY_1652
// SHELBY_1653
// SHELBY_1657
// SHELBY_1654

// Not fixed in B2
//  SHELBY_1655
//  SHELBY_1658

// IOINT bits for handshake
#define HANDSHAKE_IOM_TO_IOS     0x1
#define HANDSHAKE_IOS_TO_IOM     0x2
#define HANDSHAKE_PIN            10 // GPIO used to connect the IOSINT to host side

//*****************************************************************************
//
// Widget Config parameters
//
//*****************************************************************************
#define AM_TEST_TIMEOUT     1 // 1 Seconds
#define TEST_XOR_BYTE       0x0 // 0 Will not change, 0xFF will invert
//#define TEST_IOM_QUEUE      // if not defined uses IOM in polling mode - would cause underflows for higher speeds
#ifdef NO_SHELBY_1655
#define TEST_ACC_INT      // if defined uses ACC ISR. SHELBY-1655 interferes with this
#endif
#ifdef NO_SHELBY_1653
#define TEST_IOINTCTL      // if defined, uses IOINTCTL - SHELBY-1653 interferes with this
#endif
#ifndef AM_PART_APOLLO
#define TEST_IOS_XCMP_INT  // XCMP flags defined only for Apollo2 onwards
#endif
//#define TEST_ACC_ISR_RANDOMIZE
//*****************************************************************************
//
// Message buffers.
//
// data from the IOS interface, which is only 8 bits wide.
//
//*****************************************************************************
#define AM_IOS_TX_BUFSIZE_MAX  1023
uint8_t g_pui8TxFifoBuffer[AM_IOS_TX_BUFSIZE_MAX];
uint8_t g_pIosSendBuf[AM_IOS_TX_BUFSIZE_MAX];
uint8_t g_pRefData[AM_IOS_TX_BUFSIZE_MAX];
uint8_t g_pIomRecvBuf[AM_IOS_TX_BUFSIZE_MAX];
bool    bContinuous = false;
struct {
    uint32_t size;
    uint32_t totalWritten;
} ios_feed_state;

#ifdef TEST_IOM_QUEUE
am_hal_iom_queue_entry_t g_psQueueMemory[32];
#endif

//*****************************************************************************
//
// Configure GPIOs for communicating with a SPI fram
// TODO: This should ideally come from BSP to keep the code portable
//
//*****************************************************************************
static const uint32_t apollo2_iomce0[AM_REG_IOMSTR_NUM_MODULES][2] =
{
    {11, AM_HAL_PIN_11_M0nCE0},
    {12, AM_HAL_PIN_12_M1nCE0},
#ifdef AM_PART_APOLLO2
    { 3, AM_HAL_PIN_3_M2nCE0},
    {26, AM_HAL_PIN_26_M3nCE0},
    {29, AM_HAL_PIN_29_M4nCE0},
    {24, AM_HAL_PIN_24_M5nCE0}
#endif
};
//*****************************************************************************
//
//! IOM interrupts.
//
//*****************************************************************************
#if AM_CMSIS_REGS
static const IRQn_Type iomaster_interrupt[] = {
    IOMSTR0_IRQn,
    IOMSTR1_IRQn,
#ifdef AM_PART_APOLLO2
    IOMSTR2_IRQn,
    IOMSTR3_IRQn,
    IOMSTR4_IRQn,
    IOMSTR5_IRQn,
#endif
#else // AM_CMSIS_REGS
static const uint32_t iomaster_interrupt[] = {
    AM_HAL_INTERRUPT_IOMASTER0,
    AM_HAL_INTERRUPT_IOMASTER1,
#ifdef AM_PART_APOLLO2
    AM_HAL_INTERRUPT_IOMASTER2,
    AM_HAL_INTERRUPT_IOMASTER3,
    AM_HAL_INTERRUPT_IOMASTER4,
    AM_HAL_INTERRUPT_IOMASTER5,
#endif
    };
#endif // AM_CMSIS_REGS

typedef struct
{
    uint8_t                 directSize;
    uint8_t                 roSize;
    uint8_t                 fifoSize;
    uint8_t                 ahbRamSize;
    uint8_t                 roBase;
    uint8_t                 ahbRamBase;
    am_widget_iosspi_test_t testCfg;
    uint32_t                iom;
    uint32_t                i2cAddr;
    bool                    bSpi;
} am_widget_iosspi_t;

am_widget_iosspi_t g_sIosWidget;

// Status flags.
//! Input buffer set to this value when empty.
#define INBUFFER_EMPTY  (0xEE)
//! Output buffer set to this value when empty.
#define OUTBUFFER_EMPTY (0xED)
//! Value for one byte write/read test (188).
#define WRITE_READ_BYTE (0xBC)
#define ROBUFFER_INIT   (0x55)
#define AHBBUF_INIT     (0xAA)

static void
iom_clean_up(void)
{
    uint32_t ioIntEnable = 0;

    // Disable IOCTL interrupts
    if (g_sIosWidget.bSpi)
    {
        am_hal_iom_spi_write(g_sIosWidget.iom,
            0,
            &ioIntEnable, 1, AM_HAL_IOM_OFFSET(0xF8));
    }
    else
    {
        am_hal_iom_i2c_write(g_sIosWidget.iom,
            g_sIosWidget.i2cAddr,
            &ioIntEnable, 1, AM_HAL_IOM_OFFSET(0xF8));
    }

    //
    // Turn on the IOM for this operation.
    //
    am_bsp_iom_disable(g_sIosWidget.iom);

    if (g_sIosWidget.bSpi)
    {
        //
        // Set up IOM SPI pins. Attributes are set in am_bsp_gpio.h.
        //
        am_bsp_iom_spi_pins_disable(g_sIosWidget.iom);

        //
        // Enable the chip-select and data-ready pin.
        //! @note You can enable pins in the HAL or BSP.
        //
        am_hal_gpio_pin_config(apollo2_iomce0[g_sIosWidget.iom][0],
            AM_HAL_GPIO_DISABLE);
    }
    else
    {
        //
        // Set up IOM I2C pins. Attributes are set in am_bsp_gpio.h.
        //
        am_bsp_iom_i2c_pins_disable(g_sIosWidget.iom);
    }

    // Disable interrupts for NB send to work
    am_hal_iom_int_disable(g_sIosWidget.iom,
        AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR);
#if AM_CMSIS_REGS
    NVIC_DisableIRQ(iomaster_interrupt[g_sIosWidget.iom]);
#else // AM_CMSIS_REGS
    am_hal_interrupt_disable(iomaster_interrupt[g_sIosWidget.iom]);
#endif // AM_CMSIS_REGS

    // Set up the host IO interrupt
    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_int_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, HANDSHAKE_PIN));
    am_hal_gpio_pin_config(HANDSHAKE_PIN, AM_HAL_GPIO_DISABLE);
    am_hal_gpio_int_disable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, HANDSHAKE_PIN));
#if AM_CMSIS_REGS
    NVIC_DisableIRQ(GPIO_IRQn);
#else // AM_CMSIS_REGS
    am_hal_interrupt_disable(AM_HAL_INTERRUPT_GPIO);
#endif // AM_CMSIS_REGS
#ifndef AM_PART_APOLLO
    //
    // Disable power to IOM.
    //
    am_hal_iom_pwrctrl_disable(g_sIosWidget.iom);
#endif
}

//*****************************************************************************
//
// Internal Helper functions
//
//*****************************************************************************
void
i2c_pins_enable(uint32_t ui32Module)
{
    switch(ui32Module)
    {
        case 0:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(5);
            am_hal_gpio_out_bit_set(6);

            am_hal_gpio_pin_config(5, AM_HAL_PIN_5_M0SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(6, AM_HAL_PIN_6_M0SDA | AM_HAL_GPIO_PULLUP);
            break;

        case 1:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(8);
            am_hal_gpio_out_bit_set(9);

            am_hal_gpio_pin_config(8, AM_HAL_PIN_8_M1SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(9, AM_HAL_PIN_9_M1SDA | AM_HAL_GPIO_PULLUP);
            break;

#ifdef AM_PART_APOLLO2
        case 2:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(27);
            am_hal_gpio_out_bit_set(25);

            am_hal_gpio_pin_config(27, AM_HAL_PIN_27_M2SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(25, AM_HAL_PIN_25_M2SDA | AM_HAL_GPIO_PULLUP);
            break;
        case 3:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(42);
            am_hal_gpio_out_bit_set(43);

            am_hal_gpio_pin_config(42, AM_HAL_PIN_42_M3SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(43, AM_HAL_PIN_43_M3SDA | AM_HAL_GPIO_PULLUP);
            break;
        case 4:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(39);
            am_hal_gpio_out_bit_set(40);

            am_hal_gpio_pin_config(39, AM_HAL_PIN_39_M4SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(40, AM_HAL_PIN_40_M4SDA | AM_HAL_GPIO_PULLUP);
            break;
        case 5:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(48);
            am_hal_gpio_out_bit_set(49);

            am_hal_gpio_pin_config(48, AM_HAL_PIN_48_M5SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(49, AM_HAL_PIN_49_M5SDA | AM_HAL_GPIO_PULLUP);
            break;
#endif
        //
        // If we get here, the caller's selected IOM interface couldn't be
        // found in the BSP GPIO definitions. Halt program execution for
        // debugging.
        //
        default:
            while(1);
    }
}

static void
iom_set_up(am_widget_iosspi_stimulus_t *pCfg)
{
    uint32_t ioIntEnable = 0x3;
#ifndef AM_PART_APOLLO
    //
    // Enable power to IOM.
    //
    am_hal_iom_pwrctrl_enable(pCfg->iomModule);
#endif
    //
    // Set the required configuration settings for the IOM.
    //
    am_hal_iom_config(pCfg->iomModule, &pCfg->iomHalCfg);

    if (g_sIosWidget.bSpi)
    {
        //
        // Set up IOM SPI pins. Attributes are set in am_bsp_gpio.h.
        //
        am_bsp_iom_spi_pins_enable(pCfg->iomModule);

        //
        // Enable the chip-select and data-ready pin.
        //! @note You can enable pins in the HAL or BSP.
        //
        am_hal_gpio_pin_config(apollo2_iomce0[pCfg->iomModule][0],
            apollo2_iomce0[pCfg->iomModule][1]);
    }
    else
    {
        i2c_pins_enable(pCfg->iomModule);
    }
#ifdef TEST_IOM_QUEUE
    am_hal_iom_queue_init(pCfg->iomModule,
        g_psQueueMemory, sizeof(g_psQueueMemory));
#endif
    // Enable interrupts for NB send to work
//    am_hal_iom_int_enable(pCfg->iomModule,
//        AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR);
    am_hal_iom_int_enable(pCfg->iomModule,
        0xFF);
#if AM_CMSIS_REGS
    NVIC_EnableIRQ(iomaster_interrupt[pCfg->iomModule]);
#else // AM_CMSIS_REGS
    am_hal_interrupt_enable(iomaster_interrupt[pCfg->iomModule]);
#endif // AM_CMSIS_REGS

    //
    // Turn on the IOM for this operation.
    //
    am_bsp_iom_enable(pCfg->iomModule);

    // Set up the host IO interrupt
    am_hal_gpio_pin_config(HANDSHAKE_PIN, AM_HAL_GPIO_INPUT);
    am_hal_gpio_int_polarity_bit_set(HANDSHAKE_PIN, AM_HAL_GPIO_RISING);
    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_int_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, HANDSHAKE_PIN));

    am_hal_gpio_int_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, HANDSHAKE_PIN));
#if AM_CMSIS_REGS
    NVIC_EnableIRQ(GPIO_IRQn);
#else // AM_CMSIS_REGS
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);
#endif // AM_CMSIS_REGS

    // Set up IOCTL interrupts
    // IOS ==> IOM & IOM ==> IOS
    if (g_sIosWidget.bSpi)
    {
        am_hal_iom_spi_write(pCfg->iomModule,
            0,
            &ioIntEnable, 1, AM_HAL_IOM_OFFSET(0xF8));
    }
    else
    {
        am_hal_iom_i2c_write(pCfg->iomModule,
            g_sIosWidget.i2cAddr,
            &ioIntEnable, 1, AM_HAL_IOM_OFFSET(0xF8));
    }
}

static void
ios_clean_up(void)
{
    if (g_sIosWidget.bSpi)
    {
        // Configure SPI interface
        am_hal_gpio_pin_config(0, AM_HAL_PIN_DISABLE);
        am_hal_gpio_pin_config(1, AM_HAL_PIN_DISABLE);
        am_hal_gpio_pin_config(2, AM_HAL_PIN_DISABLE);
        am_hal_gpio_pin_config(3, AM_HAL_PIN_DISABLE);
    }
    else
    {
        // Configure I2C interface
        am_hal_gpio_pin_config(0, AM_HAL_PIN_DISABLE);
        am_hal_gpio_pin_config(1, AM_HAL_PIN_DISABLE);
    }

    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_access_int_disable(AM_HAL_IOS_ACCESS_INT_ALL);
    am_hal_ios_int_disable(AM_HAL_IOS_INT_ALL);

    // Preparation of FIFO
    am_hal_ios_fifo_buffer_init(NULL, 0);

    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
#if AM_CMSIS_REGS
    NVIC_DisableIRQ(IOSLAVEACC_IRQn);
    NVIC_DisableIRQ(IOSLAVE_IRQn);
#else // AM_CMSIS_REGS
    am_hal_interrupt_disable(AM_HAL_INTERRUPT_IOSACC);
    am_hal_interrupt_disable(AM_HAL_INTERRUPT_IOSLAVE);
#endif // AM_CMSIS_REGS

    // Set up the IOSINT interrupt pin
    am_hal_gpio_pin_config(4, AM_HAL_PIN_DISABLE);

}

#define AM_HAL_IOS_INT_ERR  (AM_HAL_IOS_INT_FOVFL | AM_HAL_IOS_INT_FUNDFL | AM_HAL_IOS_INT_FRDERR | AM_HAL_IOS_INT_GENAD)
#define AM_HAL_IOS_XCMP_INT (AM_HAL_IOS_INT_XCMPWR | AM_HAL_IOS_INT_XCMPWF | AM_HAL_IOS_INT_XCMPRR | AM_HAL_IOS_INT_XCMPRF)
//*****************************************************************************
//
// Configure the SPI slave.
//
//*****************************************************************************
static void
ios_set_up(am_hal_ios_config_t *pHalCfg)
{
    if (g_sIosWidget.bSpi)
    {
        // Configure SPI interface
        am_hal_gpio_pin_config(0, AM_HAL_PIN_0_SLSCK);
        am_hal_gpio_pin_config(1, AM_HAL_PIN_1_SLMISO);
        am_hal_gpio_pin_config(2, AM_HAL_PIN_2_SLMOSI);
        am_hal_gpio_pin_config(3, AM_HAL_PIN_3_SLnCE);
    }
    else
    {
        // Configure I2C interface
        am_hal_gpio_pin_config(0, AM_HAL_PIN_0_SLSCL);
        am_hal_gpio_pin_config(1, AM_HAL_PIN_1_SLSDA);
    }

    //
    // Configure the IOS interface and LRAM structure.
    //
    am_hal_ios_config(pHalCfg);

    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_access_int_clear(AM_HAL_IOS_ACCESS_INT_ALL);
    am_hal_ios_int_clear(AM_HAL_IOS_INT_ALL);
#ifdef TEST_ACC_INT
    am_hal_ios_access_int_enable(AM_HAL_IOS_ACCESS_INT_ALL);
#endif
    am_hal_ios_int_enable(AM_HAL_IOS_INT_ERR | AM_HAL_IOS_INT_FSIZE);
#ifdef TEST_IOINTCTL
    am_hal_ios_int_enable(AM_HAL_IOS_INT_IOINTW);
#endif
#ifdef TEST_IOS_XCMP_INT
    am_hal_ios_int_enable(AM_HAL_IOS_XCMP_INT);
#endif
    // Preparation of FIFO
    am_hal_ios_fifo_buffer_init( &g_pui8TxFifoBuffer[0], AM_IOS_TX_BUFSIZE_MAX);

    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
#if AM_CMSIS_REGS
    NVIC_EnableIRQ(IOSLAVEACC_IRQn);
    NVIC_EnableIRQ(IOSLAVE_IRQn);
#else // AM_CMSIS_REGS
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_IOSACC);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_IOSLAVE);
#endif // AM_CMSIS_REGS

    // Set up the IOSINT interrupt pin
    am_hal_gpio_pin_config(4, AM_HAL_PIN_4_SLINT);

}

volatile uint32_t g_ui32AccIsr = 0;
volatile uint32_t g_ui32IosIsr = 0;
volatile bool bIomSendComplete = false;
volatile bool bIosSendComplete = false;
volatile bool bIomRecvComplete = false;
uint32_t accIsrStatus[32] = {0};
uint32_t accIsrIdx = 0;

uint32_t missingAccIsr[32] = {0};

uint8_t g_random[] =
{
    0x15, 0x2f, 0x8f, 0xdf, 0xc3, 0xa0, 0x6c, 0x16, 0x19, 0xa5, 0x18, 0xc1, 0x18, 0x65, 0xa8, 0x44,
    0xd2, 0x53, 0x3b, 0x14, 0xb0, 0x59, 0xae, 0x00, 0x67, 0xf0, 0x6e, 0x30, 0x35, 0x21, 0x50, 0x02,
    0xf1, 0x5b, 0x1c, 0xcc, 0x4d, 0x1e, 0xd1, 0x5c, 0x30, 0xea, 0xdf, 0xf9, 0xf6, 0x39, 0x06, 0x78,
    0xd1, 0xf3, 0xef, 0x79, 0xc8, 0x34, 0x95, 0xe5, 0x38, 0x2a, 0x75, 0xb4, 0xc9, 0x76, 0x58, 0xc3,
    0xe0, 0x7f, 0x7e, 0x82, 0xa0, 0xd0, 0xac, 0x02, 0x43, 0x2d, 0xdc, 0x3d, 0x45, 0xb2, 0x97, 0x82,
    0xb5, 0x7f, 0x9c, 0x5a, 0xcf, 0x46, 0x81, 0xd4, 0x4d, 0xf9, 0xa7, 0xcc, 0xcb, 0x6e, 0xb3, 0x1d,
    0x09, 0x04, 0x6d, 0x36, 0x57, 0xc0, 0x52, 0xa6, 0x8a, 0x0c, 0xd5, 0x66, 0xeb, 0x56, 0xaa, 0x68,
    0xec, 0x25, 0x78, 0x01, 0x88, 0x63, 0x7b, 0xbf, 0xe2, 0x01, 0xbf, 0xed, 0xc5, 0x9a, 0x3c, 0xbb,
    0x45, 0x2f, 0xac, 0x8e, 0x4e, 0x57, 0x10, 0x0c, 0x29, 0x4d, 0xfe, 0xd2, 0xd6, 0x2b, 0x48, 0xad,
    0x06, 0xef, 0x51, 0xd6, 0x95, 0xa4, 0xb6, 0xe2, 0x5a, 0x30, 0x63, 0x56, 0xcd, 0x28, 0x52, 0xf6,
    0x60, 0x79, 0xa3, 0x71, 0xde, 0x02, 0xf5, 0x63, 0x5c, 0xe6, 0xe3, 0x59, 0x71, 0x7a, 0xfa, 0xd9,
    0x9d, 0x45, 0x25, 0xbc, 0xe7, 0xbe, 0x84, 0xf7, 0x48, 0xf5, 0x60, 0x37, 0x72, 0x59, 0xc6, 0xbb,
    0x22, 0xa4, 0xdd, 0xb0, 0xcb, 0x73, 0x84, 0xed, 0xa2, 0x74, 0xb6, 0x94, 0x01, 0x20, 0xcb, 0xa4,
    0xf5, 0x9d, 0xab, 0x20, 0xb8, 0xff, 0x07, 0x71, 0x51, 0xee, 0x9b, 0x08, 0x4a, 0xd8, 0x81, 0x31,
    0xd7, 0xfb, 0xa9, 0xef, 0x93, 0x13, 0x7c, 0xc6, 0xb8, 0x13, 0x53, 0x83, 0x49, 0xd9, 0xc3, 0x84,
    0xe6, 0x54, 0x43, 0x54, 0x24, 0xea, 0x32, 0xbb, 0x62, 0x4d, 0x08, 0x0c, 0x70, 0x43, 0xc5, 0xcb,
};
uint8_t g_randomIdx = 0;

//*****************************************************************************
//
// IO Slave Register Access ISR.
//
//*****************************************************************************
void
am_ioslave_acc_isr(void)
{
    uint32_t ui32Status;

#ifdef TEST_ACC_ISR_RANDOMIZE
    static uint8_t shift = 0;
    uint8_t randomVal = (g_random[g_randomIdx] ^ g_random[(g_randomIdx + 5) & 0xFF]) >> (shift++);
    g_randomIdx +=7 ;
    shift &= 0x3;
    ui32Status = AM_REGVAL(0x50000214);

    if (randomVal) ((void (*)(uint32_t)) 0x0800009d)(randomVal);
    AM_REGVAL(0x50000218) = ui32Status;

#else
    ui32Status = am_hal_ios_access_int_status_get(false);
//    am_hal_gpio_out_bit_set(23);
    am_hal_ios_access_int_clear(ui32Status);
//    am_hal_gpio_out_bit_clear(23);

#endif
    accIsrStatus[accIsrIdx++] = ui32Status;
    g_ui32AccIsr |= ui32Status;
}

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void
am_gpio_isr(void)
{
    uint64_t ui64Status;

    //
    // Read and clear the GPIO interrupt status.
    //
    ui64Status = am_hal_gpio_int_status_get(false);
    am_hal_gpio_int_clear(ui64Status);
    am_hal_gpio_int_service(ui64Status);
}

bool bHandshake = false;
uint32_t numHostInt = 0, numIoInt = 0;

// ISR callback for the host IOINT
static void hostint_handler(void)
{
    bHandshake = true;
    numHostInt++;
#if 0
    // TODO: Do we need to clear the bit by writing to IOS?
    if (g_sIosWidget.bSpi)
    {
        am_hal_iom_spi_read(g_sIosWidget.iom, 0,
            &data,1,AM_HAL_IOM_OFFSET(0xF9));
        if (data & HANDSHAKE_IOS_TO_IOM)
        {
            // Set bIosSendComplete
            bIosSendComplete = true;
        }
        am_hal_iom_spi_write_nb(g_sIosWidget.iom, 0,
            &data,1,AM_HAL_IOM_OFFSET(0xFA), 0);
    }
    else
    {
        am_hal_iom_i2c_write(g_sIosWidget.iom,
            g_sIosWidget.i2cAddr,
            &data,1,AM_HAL_IOM_OFFSET(0xFA), 0);
    }
#endif
}

uint32_t ioInt[8];
uint32_t ioIntIdx = 0;
uint32_t numXCMPRR = 0;
uint32_t numXCMPWR = 0;
uint32_t numXCMPRF = 0;
uint32_t numXCMPWF = 0;
uint32_t numGenAd = 0;
//*****************************************************************************
//
// IO Slave Main ISR.
//
//*****************************************************************************
void
am_ioslave_ios_isr(void)
{
    uint32_t ui32Status;

    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //
    ui32Status = am_hal_ios_int_status_get(true);

    am_hal_ios_int_clear(ui32Status);

    g_ui32IosIsr |= ui32Status;

#ifdef AM_PART_APOLLO2
    if (ui32Status & AM_HAL_IOS_INT_XCMPWR)
    {
        // Accumulate data from Register space
        numXCMPWR++;
    }
    if (ui32Status & AM_HAL_IOS_INT_XCMPWF)
    {
        // Accumulate data from FIFO
        numXCMPWF++;
    }
    if (ui32Status & AM_HAL_IOS_INT_XCMPRR)
    {
        // Host completed Read from register space
        numXCMPRR++;
    }
    if (ui32Status & AM_HAL_IOS_INT_XCMPRF)
    {
        // Host completed Read from FIFO
        numXCMPRF++;
    }
#endif
    if (ui32Status & AM_HAL_IOS_INT_IOINTW)
    {
        // Host interrupting us
        // Read the IOINT register for appropriate bits
        // Set bIomSendComplete based on this
        if (am_hal_ios_host_int_get() & HANDSHAKE_IOM_TO_IOS)
        {
            bIomSendComplete = true;
            am_hal_ios_host_int_clear(HANDSHAKE_IOM_TO_IOS);
        }
    }
    if (ui32Status & AM_HAL_IOS_INT_FUNDFL)
    {
        // We should never hit this case unless the threshold has beeen set
        // incorrect
        // ERROR!
//        am_hal_debug_assert_msg(0,
//            "Hitting underflow for the requested IOS FIFO transfer.");
    }
    if (ui32Status & AM_HAL_IOS_INT_FOVFL)
    {
        // We should never hit this case unless the threshold has beeen set
        // incorrect
        // ERROR!
//        am_hal_debug_assert_msg(0,
//            "Hitting overflow for the requested IOS FIFO transfer.");
    }
    if (ui32Status & AM_HAL_IOS_INT_FRDERR)
    {
        // We should never hit this case
//        am_hal_debug_assert_msg(0,
//            "Hitting Read Error for the requested IOS FIFO transfer.");
    }

    if (ui32Status & AM_HAL_IOS_INT_FSIZE)
    {
        if (bContinuous)
        {
            // Add more data to SRAM
        }
        //
        // Service the I2C slave FIFO if necessary.
        //
        am_hal_ios_fifo_service(ui32Status);
    }

    if (ui32Status & AM_HAL_IOS_INT_GENAD)
    {
        numGenAd++;
    }

}

//
//! Take over default ISR for IOM 0. (Queue mode service)
//
void
am_iomaster0_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(0, true);

    am_hal_iom_int_clear(0, ui32Status);

#ifdef TEST_IOM_QUEUE
    am_hal_iom_queue_service(0, ui32Status);
#endif
}

//
//! Take over default ISR for IOM 1. (Queue mode service)
//
void
am_iomaster1_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(1, true);

    am_hal_iom_int_clear(1, ui32Status);

#ifdef TEST_IOM_QUEUE
    am_hal_iom_queue_service(1, ui32Status);
#endif
}

//
//! Take over default ISR for IOM 1. (Queue mode service)
//
void
am_iomaster2_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(2, true);

    am_hal_iom_int_clear(2, ui32Status);

#ifdef TEST_IOM_QUEUE
    am_hal_iom_queue_service(2, ui32Status);
#endif
}

//
//! Take over default ISR for IOM 3. (Queue mode service)
//
void
am_iomaster3_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(3, true);

    am_hal_iom_int_clear(3, ui32Status);

#ifdef TEST_IOM_QUEUE
    am_hal_iom_queue_service(3, ui32Status);
#endif
}

//
//! Take over default ISR for IOM 4. (Queue mode service)
//
void
am_iomaster4_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(4, true);

    am_hal_iom_int_clear(4, ui32Status);

#ifdef TEST_IOM_QUEUE
    am_hal_iom_queue_service(4, ui32Status);
#endif
}

//
//! Take over default ISR for IOM 5. (Queue mode service)
//
void
am_iomaster5_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(5, true);

    am_hal_iom_int_clear(5, ui32Status);

#ifdef TEST_IOM_QUEUE
    am_hal_iom_queue_service(5, ui32Status);
#endif
}

// Returns expected ACC ISR value based on packet size and address for received
// data on LRAM region
static uint32_t expectedAccIsr(uint8_t address, uint8_t size)
{
    uint32_t bitMask = 0, temp = 0;
    int32_t i;
    // Identify 16 most significant bits
    if (address < 16)
    {
        for (i = address; ((i < 16) && (i < (address + size))); i++)
        {
            bitMask |= (1 << (15 - i));
        }
        temp = 0 ;
    }
    else
    {
        bitMask = 0;
        temp = (address - 16) >> 2;
    }
    bitMask <<= 16;

    // Now the least significant 16 bits
    for (i = temp + 1; (i <= (address + size - 16)>>2); i++)
    {
        bitMask |= 0x8000 >> (i-1);
    }
    return bitMask;
}

//
//! Empty the buffers.
//! Load output buffer with repeated text string.
//! Text string without 0 termination.
//
static int load_buffers(int num_addr)
{
    int i = 0; // Return number of bytes loaded.
    for (i = 0; i < sizeof(g_pIomRecvBuf); i++)
    {
        g_pIomRecvBuf[i] = INBUFFER_EMPTY;
    }
    for (i = 0; i < sizeof(g_pIosSendBuf); i++)
    {
        g_pIosSendBuf[i] = INBUFFER_EMPTY;
    }
    for (i = 0; i < sizeof(g_pRefData); i++)
    {
        g_pRefData[i] = INBUFFER_EMPTY;
    }
    for (i = 0; i < num_addr; i++)
    {
        g_pRefData[i] = i & 0xFF;
    }
    return i;
}

// Prepare IOS data with a pattern
static void
ios_buf_init(uint32_t size)
{
    uint32_t i;
    size = (size > AM_IOS_TX_BUFSIZE_MAX) ? AM_IOS_TX_BUFSIZE_MAX: size;
    for (i = 0; i < size; i++)
    {
        // Read data and prepare to be sent back after processing
        g_pIosSendBuf[i] = g_pRefData[i] ^ TEST_XOR_BYTE;
    }
}

// Verify received data from IOS against the reference
// return non-zero if fail
static bool
verify_result(am_widget_iosspi_t *pIosSpiWidget)
{
    uint32_t i;
    uint32_t address = pIosSpiWidget->testCfg.address;
    uint32_t size = pIosSpiWidget->testCfg.size;
    uint32_t fifoCtr, fifoSize;
    // Special handling for RO Data space
    if ((address != 0x7F) && ((address + size) > pIosSpiWidget->directSize))
    {
        // Check the trailing data portion here, and remaining will be checked later
        for (i = pIosSpiWidget->directSize; i < address + size; i++)
        {
            if (g_pIomRecvBuf[i] != ROBUFFER_INIT)
            {
                return true;
            }
        }
        // Remaining data to be verified
        size = pIosSpiWidget->directSize - address;
    }
    for (i = 0; i < size; i++)
    {
        if (g_pRefData[i] != (g_pIomRecvBuf[i] ^ TEST_XOR_BYTE))
        {
            am_util_stdio_printf("Buffer Validation failed @i=%d Rcvd 0x%x Expected 0x%x\n",
                i, g_pIomRecvBuf[i], g_pRefData[i] ^ TEST_XOR_BYTE);
            return true;
        }
    }
    // Verify that the RO & AHBRAM data is intact
    for (i = 0; i < pIosSpiWidget->roSize; i++)
    {
#ifndef NO_SHELBY_1643
        // SHELBY-1643
        if (((pIosSpiWidget->roBase + i) >= 0x78) && ((pIosSpiWidget->roBase + i) <= 0x7B))
        {
            continue;
        }
#endif
        if (am_hal_ios_lram_read(pIosSpiWidget->roBase + i) != ROBUFFER_INIT)
        {
            return true;
        }
    }
    for (i = 0; i < pIosSpiWidget->ahbRamSize; i++)
    {
        if (am_hal_ios_lram_read(pIosSpiWidget->ahbRamBase + i) != AHBBUF_INIT)
        {
            return true;
        }
    }
    // Make sure the Hardware FIFOSIZ is 0
    fifoCtr = AM_BFR(IOSLAVE, FIFOCTR, FIFOCTR);
    fifoSize = AM_BFR(IOSLAVE, FIFOPTR, FIFOSIZ);
    if (fifoCtr || fifoSize)
    {
        am_util_stdio_printf("Hardware Pointers not correct FIFOCTR 0x%x FIFOSIZ 0x%x\n",
            fifoCtr, fifoSize);
        return true;
    }
    return false;
}

// Indicate the slave
static void iom_send_complete(void)
{
    uint32_t data = 0x1;
    // Send data using the FIFO
    // In the completion callback - write to IOS to cause IOINT
    if (g_sIosWidget.bSpi)
    {
        am_hal_iom_spi_write(g_sIosWidget.iom, 0,
            &data,1,AM_HAL_IOM_OFFSET(0xFB));
    }
    else
    {
        am_hal_iom_i2c_write(g_sIosWidget.iom, g_sIosWidget.i2cAddr,
            &data,1,AM_HAL_IOM_OFFSET(0xFB));
    }
}

// Send test data to IOS
static void
iom_recv(uint32_t address, uint32_t size)
{
    if (g_sIosWidget.bSpi)
    {
        am_hal_iom_spi_read(g_sIosWidget.iom,
                    0,
                    (uint32_t *)g_pIomRecvBuf, size,
                    AM_HAL_IOM_OFFSET(address));
        bIomRecvComplete = true;
    }
    else
    {
        am_hal_iom_i2c_read(g_sIosWidget.iom,
                    g_sIosWidget.i2cAddr,
                    (uint32_t *)g_pIomRecvBuf, size,
                    AM_HAL_IOM_OFFSET(address));
        bIomRecvComplete = true;
    }
}

// Send test data to IOS
static void
iom_send(uint32_t address, uint32_t size)
{
    if (g_sIosWidget.bSpi)
    {
#ifdef TEST_IOM_QUEUE
        am_hal_iom_spi_write_nb(g_sIosWidget.iom,
                    0,
                    (uint32_t *)g_pRefData, size,
                    AM_HAL_IOM_OFFSET((0x80|address)), iom_send_complete);
#else
        am_hal_iom_spi_write(g_sIosWidget.iom,
                    0,
                    (uint32_t *)g_pRefData, size,
                    AM_HAL_IOM_OFFSET((0x80|address)));
        iom_send_complete();

#endif
    }
    else
    {
#ifdef TEST_IOM_QUEUE
        am_hal_iom_i2c_write_nb(g_sIosWidget.iom,
                    g_sIosWidget.i2cAddr,
                    (uint32_t *)g_pRefData, size,
                    AM_HAL_IOM_OFFSET((0x80|address)), iom_send_complete);
#else
        am_hal_iom_i2c_write(g_sIosWidget.iom,
                    g_sIosWidget.i2cAddr,
                    (uint32_t *)g_pRefData, size,
                    AM_HAL_IOM_OFFSET((0x80|address)));
        iom_send_complete();
#endif
    }

}


// Receive data from host and prepare the loop back data
static void ios_read(uint32_t address, uint32_t size)
{
    uint32_t i;
    uint8_t readByte;
    // Read only supported from LRAM
    for (i = 0; i < size; i++)
    {
        readByte = am_hal_ios_lram_read(address+i);
        // Read data and prepare to be sent back after processing
        g_pIosSendBuf[i] = readByte ^ TEST_XOR_BYTE;
    }
}

// Send data to host (IOM)
static uint32_t ios_send(uint32_t address, uint32_t size)
{
    // Send data using the LRAM or FIFO
    if (address == 0x7F)
    {
        am_hal_ios_fifo_write(&g_pIosSendBuf[0], size);
        am_hal_ios_update_fifoctr();
    }
    else
    {
        while (size--)
        {
            am_hal_ios_lram_write(address+size, g_pIosSendBuf[size]);
        }
    }
    // Notify the host
    am_hal_ios_host_int_set(HANDSHAKE_IOS_TO_IOM);
    return 0;
}

//*****************************************************************************
//
// Timer handling to implement timeout
//
//*****************************************************************************
static volatile bool g_bTimeOut = 0;
static am_hal_ctimer_config_t g_sTimer0 =
{
    // Don't link timers.
    0,

    // Set up Timer0A.
    (AM_HAL_CTIMER_FN_ONCE |
     AM_HAL_CTIMER_INT_ENABLE    |
     AM_HAL_CTIMER_LFRC_32HZ),

    // No configuration for Timer0B.
    0,
};

// Function to initialize Timer A0 to interrupt every X second.
static void
timeout_check_init(uint32_t seconds)
{
    uint32_t ui32Period;

    //
    // Enable the LFRC.
    //
    am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_LFRC);

    //
    // Set up timer A0.
    //
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_config(0, &g_sTimer0);

    //
    // Set up timerA0 to 32Hz from LFRC divided to desired second period.
    //
    ui32Period = 32 * seconds;
    am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, ui32Period,
                             (ui32Period >> 1));

    //
    // Clear the timer Interrupt
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);

    g_bTimeOut = 0;
    //
    // Enable the timer Interrupt.
    //
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);

    //
    // Enable the timer interrupt in the NVIC.
    //
#if AM_CMSIS_REGS
    NVIC_EnableIRQ(CTIMER_IRQn);
#else // AM_CMSIS_REGS
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);
#endif // AM_CMSIS_REGS
    am_hal_interrupt_master_enable();

    //
    // Start timer A0
    //
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);
}

// Timer Interrupt Service Routine (ISR)
void
am_ctimer_isr(void)
{
    //
    // Clear TimerA0 Interrupt (write to clear).
    //
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);

    // Set Timeout flag
    g_bTimeOut = 1;
}


// We need to shift one extra
#define GET_I2CADDR(cfg)    \
    (((cfg) & AM_REG_IOSLAVE_CFG_I2CADDR_M) >> (AM_REG_IOSLAVE_CFG_I2CADDR_S+1))

uint32_t am_widget_iosspi_setup(am_widget_iosspi_config_t *pIosCfg,
    void **ppWidget, char *pErrStr)
{
    uint32_t ui32Status = 0, i;
    g_sIosWidget.directSize = g_sIosWidget.roBase =
        pIosCfg->iosHalCfg.ui32ROBase;
    g_sIosWidget.roSize =
        pIosCfg->iosHalCfg.ui32FIFOBase - pIosCfg->iosHalCfg.ui32ROBase;
    g_sIosWidget.fifoSize =
        pIosCfg->iosHalCfg.ui32RAMBase - pIosCfg->iosHalCfg.ui32FIFOBase;
    g_sIosWidget.ahbRamBase = pIosCfg->iosHalCfg.ui32RAMBase;
    g_sIosWidget.ahbRamSize = 0x100 - g_sIosWidget.ahbRamBase;
    g_sIosWidget.iom = pIosCfg->stimulusCfg.iomModule;
    g_sIosWidget.bSpi =
        (pIosCfg->iosHalCfg.ui32InterfaceSelect & AM_REG_IOSLAVE_CFG_IFCSEL_SPI);
    g_sIosWidget.i2cAddr =
        GET_I2CADDR(pIosCfg->iosHalCfg.ui32InterfaceSelect);
    am_hal_interrupt_master_enable();
    // Set up the IOS
    ios_set_up(&pIosCfg->iosHalCfg);
    // Set up the stimulus IOM
    iom_set_up(&pIosCfg->stimulusCfg);
    // Set up interrupts
    *ppWidget = &g_sIosWidget;
    // Register handler for IOS => IOM interrupt

    am_hal_gpio_int_register(HANDSHAKE_PIN, hostint_handler);

    // Initialize RO & AHB-RAM data with pattern
    for (i = 0; i < g_sIosWidget.roSize; i++)
    {
        am_hal_ios_lram_write(g_sIosWidget.roBase + i, ROBUFFER_INIT);
    }
    for (i = 0; i < g_sIosWidget.ahbRamSize; i++)
    {
        am_hal_ios_lram_write(g_sIosWidget.ahbRamBase + i, AHBBUF_INIT);
    }
    return ui32Status;
}


uint32_t am_widget_iosspi_cleanup(void *pWidget, char *pErrStr)
{
    // TODO
    // Set up the stimulus IOM
    iom_clean_up();
    // Set up the IOS
    ios_clean_up();
    am_hal_interrupt_master_disable();
    return 0;
}

// IOS interrupt bits to verify
// Mask off FSIZE and IOINT bits
#define IOS_ISR_CHK_MASK (~(AM_HAL_IOS_INT_IOINTW | AM_HAL_IOS_INT_FSIZE | AM_HAL_IOS_INT_GENAD))
uint32_t
am_widget_iosspi_test(void *pWidget, void *pTestCfg, char *pErrStr)
{
    am_widget_iosspi_t *pIosSpiWidget = (am_widget_iosspi_t *)pWidget;
    uint32_t expectedIosIsr = 0;
    uint32_t ui32Status = 0;
    uint32_t checkMask;

    pIosSpiWidget->testCfg = *((am_widget_iosspi_test_t *)pTestCfg);

    // Initialize Data Buffers
    load_buffers(pIosSpiWidget->testCfg.size);
    // Use address to determine the mode - LRAM or FIFO
    if (pIosSpiWidget->testCfg.address != 0x7F)
    {
        g_ui32AccIsr = 0;
        for (int i = 0; i < 32; i++)
        {
            accIsrStatus[i] = 0;
        }
        accIsrIdx = 0;
        g_ui32IosIsr = 0;
        bIomSendComplete = false;
        // Trigger IOM to Write at specified address
        // handshake between IOM and IOS using IOCTL interrupts
        // IOM indicates IOS on completion of each batch
        iom_send(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);
        if (AM_TEST_TIMEOUT)
        {
            timeout_check_init(AM_TEST_TIMEOUT);
        }
        while (1)
        {
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
#ifndef TEST_IOINTCTL
            if (am_hal_ios_host_int_get() & HANDSHAKE_IOM_TO_IOS)
            {
                bIomSendComplete = true;
                am_hal_ios_host_int_clear(HANDSHAKE_IOM_TO_IOS);
            }
#endif
            if (bIomSendComplete || g_bTimeOut)
            {
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }
#ifdef TEST_IOINTCTL
            // Wait for interrupt indicating IOM write complete
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
#endif
            am_hal_interrupt_master_set(ui32IntStatus);
#ifndef TEST_IOINTCTL
            am_hal_flash_delay(1);
#endif
        }
        if (g_bTimeOut)
        {
            // Timeout!
            am_util_stdio_sprintf(pErrStr, "Transaction timed out doing iom_send\n");
            ui32Status = 0xFFFFFFFF;
            return ui32Status;
        }
        bIomSendComplete = false;
        expectedIosIsr = 0;
        // IOS to verify the ACC interrupts for the size received for LRAM writes
#ifndef TEST_ACC_INT
        g_ui32AccIsr = am_hal_ios_access_int_status_get(false);
        am_hal_ios_access_int_clear(g_ui32AccIsr);
#endif
        if (g_ui32AccIsr != expectedAccIsr(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size))
        {
#if 1
            // ACC Interrupts fail!
            am_util_stdio_sprintf(pErrStr, "Unexpected ACC Interrupt 0x%x: expected 0x%x\n",
                g_ui32AccIsr, expectedAccIsr(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size));
            {
                uint32_t bitMiss = g_ui32AccIsr ^ expectedAccIsr(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);
                for (int i = 0; i < 32; i++)
                {
                    if (bitMiss & 0x1)
                    {
                        missingAccIsr[31-i]++;
                    }
                    bitMiss >>= 1;
                    if (bitMiss == 0) break;
                }
            }
            return 1;
#else
            am_util_stdio_printf("Addr:0x%x, Size:0x%x - Unexpected ACC Interrupt 0x%x:Expected 0x%x\n",
                pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size,
                g_ui32AccIsr, expectedAccIsr(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size));
#endif
        }
        checkMask = ~(AM_HAL_IOS_INT_IOINTW | AM_HAL_IOS_INT_FSIZE);
#ifndef AM_PART_APOLLO
        // For Apollo2 - IOS to also verify the CMP interrupts
        expectedIosIsr |= AM_REG_IOSLAVE_INTSTAT_XCMPWR_M;
#ifdef TEST_ACC_INT
#ifndef NO_SHELBY_1658
        // SHELBY-1658
        checkMask &= ~AM_REG_IOSLAVE_INTSTAT_XCMPWR_M;
#endif
#endif
#endif
#ifndef TEST_IOS_XCMP_INT
        g_ui32IosIsr |= am_hal_ios_int_status_get(false);
        am_hal_ios_int_clear(g_ui32IosIsr);
#endif
        if ((g_ui32IosIsr & checkMask) != (expectedIosIsr & checkMask))
        {
#if 0
            // IOS Interrupts fail!
            am_util_stdio_sprintf(pErrStr, "Unexpected IOS Interrupt 0x%x\n",
                g_ui32IosIsr);
            return 1;
#else
            am_util_stdio_printf("\nAddr:0x%x, Size:0x%x - Unexpected IOS Interrupt on Write 0x%x: mask 0x%x: Actual 0x%x: expected 0x%x\n",
                pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size,
                g_ui32IosIsr, checkMask, (g_ui32IosIsr & checkMask), expectedIosIsr);
#endif
        }
        // IOS will accumulate the data if needed on READ side
        // When complete, the data will be turned around back to IOM
        // Read the received data
        ios_read(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);
        // Write the reply (bit inverted of received data)
    }
    else
    {
        // Prepare a patterned data from IOS
        ios_buf_init(pIosSpiWidget->testCfg.size);
        if (pIosSpiWidget->testCfg.size > AM_IOS_TX_BUFSIZE_MAX)
        {
            // This is a special test, to test host-slave continuous data transfer
            // with handshake based on FIFOCTR
            bContinuous = true;
            // Initilize state for the continuous feeding
            ios_feed_state.size = pIosSpiWidget->testCfg.size;
            ios_feed_state.totalWritten = 0;
        }
    }

    if (bContinuous == false)
    {
        bIosSendComplete = false;
        ios_send(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);
        // Handshake will use the interrupt IOCTL - to signal host that data is ready
        if (AM_TEST_TIMEOUT)
        {
            timeout_check_init(AM_TEST_TIMEOUT);
        }
        while (1)
        {
            // Disable interrupts before checking the flags
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
            if (bIosSendComplete || g_bTimeOut)
            {
                // re-enable interrupts
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }

            if (bHandshake == true)
            {
                uint32_t data;
                // re-enable interrupts
                am_hal_interrupt_master_set(ui32IntStatus);
                bHandshake = false;
                // Read & Clear the IOINT status
                if (g_sIosWidget.bSpi)
                {
                    am_hal_iom_spi_read(g_sIosWidget.iom, 0,
                        &data,1,AM_HAL_IOM_OFFSET(0x79));
                    // We need to clear the bit by writing to IOS
                    if (data & HANDSHAKE_IOS_TO_IOM)
                    {
                        data = HANDSHAKE_IOS_TO_IOM;
                        am_hal_iom_spi_write(g_sIosWidget.iom, 0,
                            &data,1,AM_HAL_IOM_OFFSET(0xFA));
                        // Set bIosSendComplete
                        bIosSendComplete = true;
                        break;
                    }
                }
                else
                {
                    am_hal_iom_i2c_read(g_sIosWidget.iom, g_sIosWidget.i2cAddr,
                        &data,1,AM_HAL_IOM_OFFSET(0x79));
                    // We need to clear the bit by writing to IOS
                    if (data & HANDSHAKE_IOS_TO_IOM)
                    {
                        data = HANDSHAKE_IOS_TO_IOM;
                        am_hal_iom_i2c_write(g_sIosWidget.iom,
                            g_sIosWidget.i2cAddr,
                            &data,1,AM_HAL_IOM_OFFSET(0xFA));
                        // Set bIosSendComplete
                        bIosSendComplete = true;
                        break;
                    }
                }
            }
            else
            {
                am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
                // re-enable interrupts
                am_hal_interrupt_master_set(ui32IntStatus);
            }
        }
        if (g_bTimeOut)
        {
            // Timeout!
            am_util_stdio_sprintf(pErrStr, "Transaction timed out doing ios_send\n");
            ui32Status = 0xFFFFFFFF;
            return ui32Status;
        }
        bIosSendComplete = false;

        // For FIFO case, verify the FIFOCTR
        if (pIosSpiWidget->testCfg.address == 0x7F)
        {
            uint32_t iosSize = 0;
            if (g_sIosWidget.bSpi)
            {
                am_hal_iom_spi_read(g_sIosWidget.iom, 0,
                    &iosSize,2,AM_HAL_IOM_OFFSET(0x7C));
            }
            else
            {
                am_hal_iom_i2c_read(g_sIosWidget.iom, g_sIosWidget.i2cAddr,
                    &iosSize,2,AM_HAL_IOM_OFFSET(0x7C));
            }
            if (iosSize != pIosSpiWidget->testCfg.size)
            {
#if 0
                am_util_stdio_sprintf(pErrStr, "FIFOCTR incorrect Actual=0x%x, Expected=0x%x\n",
                    iosSize, pIosSpiWidget->testCfg.size);
                return 0xFFFFFFFF;
#else
                am_util_stdio_printf("\nFIFOCTR incorrect Actual=0x%x, Expected=0x%x\n",
                    iosSize, pIosSpiWidget->testCfg.size);
#endif
            }
        }

        g_ui32IosIsr = 0;
        expectedIosIsr = 0;
        bIomRecvComplete = false;

        iom_recv(pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size);

        if (AM_TEST_TIMEOUT)
        {
            timeout_check_init(AM_TEST_TIMEOUT);
        }
        while (1)
        {
            // Disable interrupts before checking the flags
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();
            if (bIomRecvComplete || g_bTimeOut)
            {
                // re-enable interrupts
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
            // re-enable interrupts
            am_hal_interrupt_master_set(ui32IntStatus);
        }

        if (g_bTimeOut)
        {
            // Timeout!
            am_util_stdio_sprintf(pErrStr, "Transaction timed out doing iom_recv\n");
            ui32Status = 0xFFFFFFFF;
            return ui32Status;
        }

        checkMask = ~(AM_HAL_IOS_INT_IOINTW | AM_HAL_IOS_INT_FSIZE);
#ifndef AM_PART_APOLLO
        // For Apollo2 - IOS to also verify the CMP interrupts
        if (pIosSpiWidget->testCfg.address == 0x7F)
        {
            expectedIosIsr |= AM_REG_IOSLAVE_INTSTAT_XCMPRF_M;
        }
        else
        {
            expectedIosIsr |= (AM_REG_IOSLAVE_INTSTAT_XCMPRR_M);
        }
#ifndef NO_SHELBY_1652
        // SHELBY-1652
        if (pIosSpiWidget->testCfg.address == 0x77)
        {
            checkMask &= ~(AM_REG_IOSLAVE_INTSTAT_XCMPRR_M);
        }
#endif
#endif
#ifndef NO_SHELBY_1657
        // SHELBY-1657
        if (pIosSpiWidget->testCfg.address <= 1)
        {
            checkMask &= ~(AM_REG_IOSLAVE_INTSTAT_GENAD_M);
        }
#endif
#ifndef TEST_IOS_XCMP_INT
        g_ui32IosIsr |= am_hal_ios_int_status_get(false);
        am_hal_ios_int_clear(g_ui32IosIsr);
#endif
        // Check for the interrupt status
        if ((g_ui32IosIsr & checkMask) != (expectedIosIsr & checkMask))
        {
#if 0
            // IOS Interrupts fail!
            am_util_stdio_sprintf(pErrStr, "Unexpected IOS Interrupt 0x%x:0x%x\n",
                g_ui32IosIsr, expectedIosIsr);
            return 1;
#else
            am_util_stdio_printf("\nAddr:0x%x, Size:0x%x - Unexpected IOS Interrupt on Read 0x%x: mask 0x%x: Actual 0x%x: expected 0x%x\n",
                pIosSpiWidget->testCfg.address, pIosSpiWidget->testCfg.size,
                g_ui32IosIsr, checkMask, (g_ui32IosIsr & checkMask), expectedIosIsr);
#endif
        }
        // Verify received data at IOM
        ui32Status = verify_result(pIosSpiWidget);
        if (ui32Status)
        {
            am_util_stdio_sprintf(pErrStr, "Buffer verification failed\n");
            return ui32Status;
        }
    }
    else
    {
        // We implement a continous loop to feed data from Slave to host
        // up to given size
    }
    return ui32Status;
}
