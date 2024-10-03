//*****************************************************************************
//
//! @file am_widget_iom.c
//!
//! @brief Test widget for testing IOM channels by data transfer using fram.
//
//! This widget performs a write followed by read operations to an external
//! fram after configuring the IOM using specified configuration.
//! Success or failure is detected based on buffer matching.
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
#include "am_hal_iom.h"
#include "am_bsp.h"
#include "am_devices.h"
#include "am_util.h"

#include "am_widget_iom.h"

// BSP defines
//*****************************************************************************
//
// These should ideally come from BSP to keep the code portable
//
//*****************************************************************************
// Pin settings for I2C bit-bang
#define I2C_BB_SCK_GPIO 14
#define I2C_BB_SDA_GPIO 15

// Configure GPIOs for communicating with a SPI fram
static const uint32_t apollo2_iomce0[AM_REG_IOMSTR_NUM_MODULES][2] =
{
#ifdef AM_PART_APOLLO
    {11, AM_HAL_PIN_11_M0nCE0},
    {12, AM_HAL_PIN_12_M1nCE0},
#endif
#ifdef AM_PART_APOLLO2
    {11, AM_HAL_PIN_11_M0nCE0},
    { 6, AM_HAL_PIN_6_M1nCE0},
    { 3, AM_HAL_PIN_3_M2nCE0},
    {26, AM_HAL_PIN_26_M3nCE0},
    {29, AM_HAL_PIN_29_M4nCE0},
    {24, AM_HAL_PIN_24_M5nCE0},
#endif
};

//*****************************************************************************
//
// Widget Config parameters
//
//*****************************************************************************
#define AM_TEST_TIMEOUT     10 // 10 Seconds
//! Number of buffer compare errors tolerated before failing.
#define COMPARE_TOLERANCE   (0)
#define XOR_BYTE            (0x0)

//! End of Buffer Tolerance
#define EOB_TOLERANCE       (0)
#define ERROUT_CHAR         (0)

//! Number of entries in IOM Queue.
//! We queue max 10 transactions - 2 Block writes, each taking 3,
//! and 2 block reads, each taking 2
#define IOM_QUEUESIZE       (10)

#define MAX_TRANS_SIZE      (4096)

// Status flags.
//! Input buffer set to this value when empty.
#define INBUFFER_EMPTY      (0xEE)
//! Output buffer set to this value when empty.
#define OUTBUFFER_EMPTY     (0xED)
//! Value for one byte write/read test (188).
#define WRITE_READ_BYTE     (0xBC)

//! Log levels of verbosity.
enum log_levels
{
    LOG_LVL_SILENT = -3,
    LOG_LVL_OUTPUT = -2,
    LOG_LVL_USER = -1,
    LOG_LVL_ERROR = 0,
    LOG_LVL_WARNING = 1,
    LOG_LVL_INFO = 2,
    LOG_LVL_DEBUG = 3
};

//! Define error logging level.
//#define LOG_LVL LOG_LVL_DEBUG
//#define LOG_LVL LOG_LVL_INFO
#define LOG_LVL LOG_LVL_ERROR
//#define LOG_LVL LOG_LVL_USER
//#define LOG_LVL LOG_LVL_OUTPUT
//#define LOG_LVL LOG_LVL_SILENT

//! @todo Break this out into va_list routine.
#define LOG_DEBUG(expr ...)             \
    do {                                \
        if (LOG_LVL >= LOG_LVL_DEBUG)   \
            am_util_stdio_printf(expr); \
    } while (0)

#define LOG_INFO(expr ...)              \
    do {                                \
        if (LOG_LVL >= LOG_LVL_INFO)    \
            am_util_stdio_printf(expr); \
    } while (0)

#define LOG_WARNING(expr ...)           \
    do {                                \
        if (LOG_LVL >= LOG_LVL_WARNING) \
            am_util_stdio_printf(expr); \
    } while (0)

#define LOG_ERROR(expr ...)             \
    do {                                \
        if (LOG_LVL >= LOG_LVL_ERROR)   \
            am_util_stdio_printf(expr); \
    } while (0)

#define LOG_USER(expr ...)              \
    do {                                \
        if (LOG_LVL >= LOG_LVL_USER)    \
            am_util_stdio_printf(expr); \
    } while (0)

#define LOG_OUTPUT(expr ...)            \
    do {                                \
        if (LOG_LVL >= LOG_LVL_OUTPUT)  \
            am_util_stdio_printf(expr); \
    } while (0)

//*****************************************************************************
//
// Internal Data Structures
//
//*****************************************************************************
typedef struct
{
    AM_HAL_IOM_MODE_E       mode;
    bool                    bSpi;
    uint32_t                iomModule;
} am_widget_iom_t;

//*****************************************************************************
//
// Memory for handling IOM transactions.
//
//*****************************************************************************
static am_hal_iom_queue_entry_t g_psQueueMemory[IOM_QUEUESIZE];
static unsigned char g_pucInBuffer[MAX_TRANS_SIZE*2];
static unsigned char g_pucOutBuffer[MAX_TRANS_SIZE*2];

//*****************************************************************************
//
// Global Variables.
//
//*****************************************************************************
static am_widget_iom_t g_IomSpiWidget;
static volatile uint32_t g_IomIsrErr = 0;

//! IOM interrupts.
#if AM_CMSIS_REGS
static const IRQn_Type iomaster_interrupt[] =
{
    IOMSTR0_IRQn,
    IOMSTR1_IRQn,
#ifndef AM_PART_APOLLO
    IOMSTR2_IRQn,
    IOMSTR3_IRQn,
    IOMSTR4_IRQn,
    IOMSTR5_IRQn,
#endif
    };
#else // AM_CMSIS_REGS
static const uint32_t iomaster_interrupt[] =
{
    AM_HAL_INTERRUPT_IOMASTER0,
    AM_HAL_INTERRUPT_IOMASTER1,
#ifndef AM_PART_APOLLO
    AM_HAL_INTERRUPT_IOMASTER2,
    AM_HAL_INTERRUPT_IOMASTER3,
    AM_HAL_INTERRUPT_IOMASTER4,
    AM_HAL_INTERRUPT_IOMASTER5,
#endif
    };
#endif // AM_CMSIS_REGS

static volatile uint32_t g_bComplete = 0;

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
//
//! Take over default ISR for IOM 0. (Queue mode service)
//
void
am_iomaster0_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(0, true);

    am_hal_iom_int_clear(0, ui32Status);

    if (ui32Status & ~(AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR))
    {
        // Unexpected interrupt
        g_IomIsrErr = ui32Status;
    }

    am_hal_iom_queue_service(0, ui32Status);
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
    if (ui32Status & ~(AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR))
    {
        // Unexpected interrupt
        g_IomIsrErr = ui32Status;
    }


    am_hal_iom_queue_service(1, ui32Status);
}

#ifndef AM_PART_APOLLO
//
//! Take over default ISR for IOM 1. (Queue mode service)
//
void
am_iomaster2_isr(void)
{
    uint32_t ui32Status;

    ui32Status = am_hal_iom_int_status_get(2, true);

    am_hal_iom_int_clear(2, ui32Status);
    if (ui32Status & ~(AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR))
    {
        // Unexpected interrupt
        g_IomIsrErr = ui32Status;
    }


    am_hal_iom_queue_service(2, ui32Status);
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
    if (ui32Status & ~(AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR))
    {
        // Unexpected interrupt
        g_IomIsrErr = ui32Status;
    }


    am_hal_iom_queue_service(3, ui32Status);
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
    if (ui32Status & ~(AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR))
    {
        // Unexpected interrupt
        g_IomIsrErr = ui32Status;
    }


    am_hal_iom_queue_service(4, ui32Status);
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
    if (ui32Status & ~(AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR))
    {
        // Unexpected interrupt
        g_IomIsrErr = ui32Status;
    }


    am_hal_iom_queue_service(5, ui32Status);
}
#endif

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
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM0_SCL);
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM0_SDA);

            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM0_SCL, AM_HAL_PIN_5_M0SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM0_SDA, AM_HAL_PIN_6_M0SDA | AM_HAL_GPIO_PULLUP);
            break;

        case 1:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM1_SCL);
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM1_SDA);

            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM1_SCL, AM_HAL_PIN_8_M1SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM1_SDA, AM_HAL_PIN_9_M1SDA | AM_HAL_GPIO_PULLUP);
            break;

#ifndef AM_PART_APOLLO
        case 2:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM2_SCL);
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM2_SDA);

            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM2_SCL, AM_HAL_PIN_27_M2SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM2_SDA, AM_HAL_PIN_25_M2SDA | AM_HAL_GPIO_PULLUP);
            break;
        case 3:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM3_SCL);
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM3_SDA);

            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM3_SCL, AM_HAL_PIN_42_M3SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM3_SDA, AM_HAL_PIN_43_M3SDA | AM_HAL_GPIO_PULLUP);
            break;
        case 4:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM4_SCL);
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM4_SDA);

            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM4_SCL, AM_HAL_PIN_39_M4SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM4_SDA, AM_HAL_PIN_40_M4SDA | AM_HAL_GPIO_PULLUP);
            break;
        case 5:
            //
            // Set pins high to prevent bus dips.
            //
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM5_SCL);
            am_hal_gpio_out_bit_set(AM_BSP_GPIO_IOM5_SDA);

            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM5_SCL, AM_HAL_PIN_48_M5SCL | AM_HAL_GPIO_PULLUP);
            am_hal_gpio_pin_config(AM_BSP_GPIO_IOM5_SDA, AM_HAL_PIN_49_M5SDA | AM_HAL_GPIO_PULLUP);
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

//
//! Configure gpio and iom pins for iom.
//
static void configure_pins(int module, bool bSpi)
{
    if (bSpi)
    {
        //
        // Set up IOM SPI pins. Attributes are set in am_bsp_gpio.h.
        //
        am_bsp_iom_spi_pins_enable(module);

        //
        // Enable the chip-select and data-ready pin.
        //! @note You can enable pins in the HAL or BSP.
        //
        am_hal_gpio_pin_config(apollo2_iomce0[module][0], apollo2_iomce0[module][1]);
        //    am_hal_gpio_pin_config(23, AM_HAL_GPIO_OUTPUT);
    }
    else
    {
        i2c_pins_enable(module);
    }

}

//
//! Initialize IOM Controller.
//
static uint32_t iom_ctl_init(uint32_t ui32IOMModule, am_hal_iom_config_t *psIOMConfig,
            AM_HAL_IOM_MODE_E hal_iom_mode, bool bSpi)
{
    uint32_t status = 0;
    if (ui32IOMModule < AM_REG_IOMSTR_NUM_MODULES)
    {
        //
        // Enable power to IOM.
        //
        am_hal_iom_pwrctrl_enable(ui32IOMModule);

        //
        // Set the required configuration settings for the IOM.
        //
        am_hal_iom_config(ui32IOMModule, psIOMConfig);

        //
        // Setup the pins for SPI mode (BSP).
        //
        configure_pins(ui32IOMModule, bSpi);

        if (hal_iom_mode == AM_HAL_IOM_MODE_QUEUE)
        {
            //
            // Attach the IOM queue system to the memory we just allocated.
            //
            am_hal_iom_queue_init(ui32IOMModule,
                g_psQueueMemory, sizeof(g_psQueueMemory));
        }
        // Enable all interrupts to check for any errors
        am_hal_iom_int_enable(ui32IOMModule, 0xFF);
#if AM_CMSIS_REGS
        NVIC_EnableIRQ(iomaster_interrupt[ui32IOMModule]);
#else // AM_CMSIS_REGS
        am_hal_interrupt_enable(iomaster_interrupt[ui32IOMModule]);
#endif // AM_CMSIS_REGS
        am_hal_interrupt_master_enable();
        //
        // Turn on the IOM for this operation.
        //
        am_bsp_iom_enable(ui32IOMModule);
    }
    else if (!bSpi && (ui32IOMModule == AM_HAL_IOM_I2CBB_MODULE))
    {
        status = am_hal_i2c_bit_bang_init(I2C_BB_SCK_GPIO,
                                 I2C_BB_SDA_GPIO);
    }
    else
    {
        status = 0xFFFFFFFF;
    }
    return status;
}

static void
iom_clean_up(uint32_t iom, bool bSpi)
{
    //
    // Disable the IOM
    //
    if (iom < AM_REG_IOMSTR_NUM_MODULES)
    {
        am_bsp_iom_disable(iom);

        if (bSpi)
        {
            //
            // Set up IOM SPI pins. Attributes are set in am_bsp_gpio.h.
            //
            am_bsp_iom_spi_pins_disable(iom);

            //
            // Disable the chip-select.
            //! @note You can enable pins in the HAL or BSP.
            //
            am_hal_gpio_pin_config(apollo2_iomce0[iom][0],
                AM_HAL_GPIO_DISABLE);
        }
        else
        {
            am_bsp_iom_i2c_pins_disable(iom);
        }

        // Disable interrupts
        am_hal_iom_int_disable(iom, 0xFF);
#if AM_CMSIS_REGS
        NVIC_DisableIRQ(iomaster_interrupt[iom]);
#else // AM_CMSIS_REGS
        am_hal_interrupt_disable(iomaster_interrupt[iom]);
#endif // AM_CMSIS_REGS

        //
        // Disable power to IOM.
        //
        am_hal_iom_pwrctrl_disable(iom);
    }
    else
    {
    }

}


//
//! Empty the buffers.
//! Load output buffer with repeated text string.
//! Text string without 0 termination.
//
static int load_buffers(int num_addr)
{
    int i = 0; // Return number of bytes loaded.

    memset(g_pucInBuffer, INBUFFER_EMPTY, sizeof(g_pucInBuffer));
    memset(g_pucOutBuffer, OUTBUFFER_EMPTY, sizeof(g_pucOutBuffer));

    for (i = 0; i < num_addr; i++)
    {
        g_pucOutBuffer[i] = i & 0xFF;
        if ((i & 0x0F) == 0)
        {
          g_pucOutBuffer[i] = 0xFF;
        }
        g_pucOutBuffer[i] ^= XOR_BYTE;
    }
    return i;
}

//! Compare input and output buffers.
static bool compare_buffers(int num_addr)
{
    bool title_printed = false;
    bool bAssertion = true;
    int lastfail = 0;
    int compare_tolerance = 0;
    for (int i = 0; i < (num_addr - EOB_TOLERANCE); i++)
    {
        if (g_pucInBuffer[i] != g_pucOutBuffer[i])
        {
            if (!title_printed)
            {
                LOG_ERROR("\r\nCompare %d bytes failed\n", num_addr);
                LOG_DEBUG("\r\nInput Empty 0x%2X, Output Empty 0x%2X\r\n"
                    "g_pucInBuffer 0x%08X, g_pucOutBuffer 0x%08X\r\n",
                    INBUFFER_EMPTY, OUTBUFFER_EMPTY,
                    (uintptr_t) g_pucInBuffer, (uintptr_t) g_pucOutBuffer);
                title_printed = true;
            }

#if (ERROUT_CHAR == 1)
            if ((isprint(g_pucInBuffer[i])) && (isprint(g_pucOutBuffer[i])))
            {
                LOG_ERROR("rcvd[%d]:%c:, expected[%d]:%c:, lastfail: %d\r\n",
                    (i), g_pucInBuffer[i], i, g_pucOutBuffer[i], i-lastfail);
            }
            else
            {
                LOG_ERROR("rcvd[%d]:0x%x:, expected[%d]:0x%x:, lastfail: %d\r\n",
                    (i), g_pucInBuffer[i], i, g_pucOutBuffer[i], i-lastfail);
            }
#else
            LOG_ERROR("rcvd[%d]:0x%x:, expected[%d]:0x%x:, lastfail: %d\r\n",
                (i), g_pucInBuffer[i], i, g_pucOutBuffer[i], i-lastfail);
#endif

            // How long ago was the last fail. First time = index.
            lastfail = i;
            if ( ++compare_tolerance > COMPARE_TOLERANCE )
            {
                bAssertion = false;
            }
        }
    }
    return bAssertion;
}

static void
read_complete_callbackFn(void)
{
    g_bComplete = 1;
}

//*****************************************************************************
//
// Widget functions
//
//*****************************************************************************
uint32_t am_widget_iom_setup(uint32_t iomModule,
    am_widget_iom_config_t *pIomCfg,
    void **ppWidget, char *pErrStr)
{
    uint32_t ui32Status = 0;

    *ppWidget = NULL;
    //
    //! Device structure for the SPI fram (IOM channel, MxnCE(x) chip select)
    //
    LOG_DEBUG("%s: %d\n", __FUNCTION__, iomModule);

    // Build Widget Context
    g_IomSpiWidget.iomModule = iomModule;
    g_IomSpiWidget.mode = pIomCfg->mode;
    g_IomSpiWidget.bSpi = pIomCfg->halCfg.ui32InterfaceMode & AM_HAL_IOM_SPIMODE;
    *ppWidget = &g_IomSpiWidget;

    //
    // Configure the IOM, and initialize the driver with IOM information.
    //
    ui32Status = iom_ctl_init(iomModule, &pIomCfg->halCfg, pIomCfg->mode, g_IomSpiWidget.bSpi);
    if (ui32Status)
    {
        am_util_stdio_sprintf(pErrStr, "Failed to initialize IOM\n");
        LOG_ERROR("%s\n", pErrStr);
        return ui32Status;
    }

    if (pIomCfg->mode == AM_HAL_IOM_MODE_QUEUE)
    {
        am_hal_iom_queue_flush(iomModule);
    }
    return ui32Status;
}

uint32_t am_widget_iom_cleanup(void *pWidget, char *pErrStr)
{
    uint32_t ui32Status = 0;
    am_widget_iom_t *pIomSpiWidget = (am_widget_iom_t *)pWidget;
    if (pIomSpiWidget->mode == AM_HAL_IOM_MODE_QUEUE)
    {
        am_hal_iom_queue_flush(pIomSpiWidget->iomModule);
    }

    iom_clean_up(pIomSpiWidget->iomModule, pIomSpiWidget->bSpi);
    return ui32Status;
}

uint32_t
am_widget_iom_test(void *pWidget, void *pTestCfg, char *pErrStr)
{
    bool bAssertion;
    uint32_t ui32IntStatus;
    uint32_t ui32Status = 0;
    am_widget_iom_t *pIomSpiWidget = (am_widget_iom_t *)pWidget;
    am_widget_iom_test_t *pIomSpiTestCfg = (am_widget_iom_test_t *)pTestCfg;

    if (pIomSpiWidget->mode == AM_HAL_IOM_MODE_QUEUE)
    {
        am_hal_iom_queue_flush(pIomSpiWidget->iomModule);
    }
    //
    // Empty Buffers. Load Outbuffer for write.
    //
    load_buffers(pIomSpiTestCfg->size*2);

    g_bComplete = 0;
    g_IomIsrErr = 0;

    if (pIomSpiWidget->bSpi)
    {
        //
        // Write (and read) to SpiFram.
        // We'll write two sets and verify. FRAM supports 8K so it is okay
        //
        am_devices_spifram_write_mode(g_pucOutBuffer, 0, pIomSpiTestCfg->size,
                pIomSpiWidget->mode, 0);
        am_devices_spifram_write_mode(g_pucOutBuffer + pIomSpiTestCfg->size,
                pIomSpiTestCfg->size, pIomSpiTestCfg->size,
                pIomSpiWidget->mode, 0);
        am_devices_spifram_read_mode(g_pucInBuffer, 0, pIomSpiTestCfg->size,
                pIomSpiWidget->mode, 0);
        am_devices_spifram_read_mode(g_pucInBuffer + pIomSpiTestCfg->size,
                pIomSpiTestCfg->size, pIomSpiTestCfg->size,
                pIomSpiWidget->mode, read_complete_callbackFn);
    }
    else
    {
        //
        // Write (and read) to I2cFram.
        // We'll write two sets and verify. FRAM supports 8K so it is okay
        //
        am_devices_i2cfram_write_mode(g_pucOutBuffer, 0, pIomSpiTestCfg->size,
                pIomSpiWidget->mode, 0);
        am_devices_i2cfram_write_mode(g_pucOutBuffer + pIomSpiTestCfg->size,
                pIomSpiTestCfg->size, pIomSpiTestCfg->size,
                pIomSpiWidget->mode, 0);
        am_devices_i2cfram_read_mode(g_pucInBuffer, 0, pIomSpiTestCfg->size,
                pIomSpiWidget->mode, 0);
        am_devices_i2cfram_read_mode(g_pucInBuffer + pIomSpiTestCfg->size,
                pIomSpiTestCfg->size, pIomSpiTestCfg->size,
                pIomSpiWidget->mode, read_complete_callbackFn);
    }
    if (AM_TEST_TIMEOUT)
    {
        timeout_check_init(AM_TEST_TIMEOUT);
    }
    // Wait for Reads to finish
    while (1)
    {
        ui32IntStatus = am_hal_interrupt_master_disable();
        if (g_bComplete || g_bTimeOut || g_IomIsrErr)
        {
            am_hal_interrupt_master_set(ui32IntStatus);
            break;
        }
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        am_hal_interrupt_master_set(ui32IntStatus);
    }
    if (g_bTimeOut)
    {
        am_util_stdio_sprintf(pErrStr, "Transaction timed out\n");
        LOG_ERROR("%s\n", pErrStr);
        ui32Status = 0xFFFFFFFF;
        return ui32Status;
    }
    if (g_IomIsrErr)
    {
        am_util_stdio_sprintf(pErrStr, "Unexpected Interrupt 0x%x\n", g_IomIsrErr);
        LOG_ERROR("%s\n", pErrStr);
        ui32Status = 0xFFFFFFFF;
        return ui32Status;
    }
#if 0
    // temp - induce error
    g_pucInBuffer[pIomSpiTestCfg->size/8] = 0xEE;
    g_pucInBuffer[pIomSpiTestCfg->size/6] = 0xEE;
    g_pucInBuffer[pIomSpiTestCfg->size/4] = 0xEE;
    g_pucInBuffer[pIomSpiTestCfg->size/2] = 0xEE;
    g_pucInBuffer[pIomSpiTestCfg->size/3] = 0xEE;
#endif
    //
    // Compare Buffers.
    //
    bAssertion = compare_buffers(pIomSpiTestCfg->size);

    if (!bAssertion)
    {
        am_util_stdio_sprintf(pErrStr, "Buffer Compare Failed for size %d", pIomSpiTestCfg->size);
        LOG_DEBUG("%s\n", pErrStr);
        ui32Status = 0xFFFFFFFF;
        return ui32Status;
    }
    return ui32Status;

}
