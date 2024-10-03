//*****************************************************************************
//
//! @file am_devices_ambt53.c
//!
//! @brief The implementation of Apollo interface to AMBT53, including the general
//!        MSPI driver, and System Controller and Power Management (SCPM) module
//!        I2C driver. Also includes the AMBT53 firmware loading and booting functions.
//!
//! @addtogroup ambt53 AMBT53 Device Driver
//! @ingroup devices
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
#include <string.h>
#include "am_bsp.h"
#include "am_mcu_apollo.h"
#include "am_util_stdio.h"
#include "am_devices_ambt53.h"

//*****************************************************************************
//
//! @name Macro definitions.
//! @{
//
//*****************************************************************************
#define ERROR_RETURN(status)                                                     \
    if ((status))                                                                \
    {                                                                            \
        am_util_debug_printf("Error detect: 0x%08x\n", (status));                \
        return (status);                                                         \
    }

//*****************************************************************************
//! AMBT53 module handle.
//*****************************************************************************
void* g_RpmsgDevHdl     = NULL;
void* pvRpmsgMspiHandle = NULL;
void* g_ScpmDevHdl      = NULL;
void* pvScpmI2cHandle   = NULL;
//*****************************************************************************
//! SCPM configuration.
//*****************************************************************************
am_devices_scpm_config_t stSCPMConfig =
{
    .ui32ClockFreq = SCPM_I2C_FREQ,
    .pNBTxnBuf = NULL,
    .ui32NBTxnBufLength = 0
};

//*****************************************************************************
//! MSPI configuration.
//*****************************************************************************
AM_SHARED_RW uint32_t g_DMABuffer[4096];

am_devices_mspi_ambt53_config_t stRPMsgConfig =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_QUAD_CE1,
    .eClockFreq = AMBT53_MSPI_FREQ,
    .pNBTxnBuf = g_DMABuffer,
    .ui32NBTxnBufLength = sizeof(g_DMABuffer) / sizeof(uint32_t),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
    .ui16ReadInstr = AM_DEVICES_MSPI_AMBT53_READ_BL4,
    .ui16WriteInstr = AM_DEVICES_MSPI_AMBT53_WRITE_UL,
    .bChangeInstr = false
};

//*****************************************************************************
//
//  Hold the ambt53 core reset signal.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_core_hold(void)
{
#if (AMBT53_RESET_GPIO_CONTROL)
    // Core reset should be assert for loading FW
    am_hal_scpm_set_cxrst();
    // Global reset should be assert to reset the whole system to default status
    am_hal_scpm_set_globrst();
    am_util_delay_ms(1);
    // Global reset should be released before loading FW
    am_hal_scpm_release_globrst();
    return 0;
#elif SCPM_IS_SUPPORTED
    uint32_t ui32Status;
    uint32_t ui32ScpmCtrl = 0;

    ui32Status = AMBT53_REG32_RD(SCPM_SW_RST_CTRL_ADDR, &ui32ScpmCtrl);
    CHECK_REG_READ(ui32Status, SCPM_SW_RST_CTRL_ADDR);

#if (LOAD_FW_DEBUG_LOG)
    am_util_stdio_printf("scpm sw rst ctrl addr 0x%x, value: 0x%08x\n", SCPM_SW_RST_CTRL_ADDR, ui32ScpmCtrl);
#endif

    // Hold the core reset and start to load the ambt53 program
    ui32ScpmCtrl |= (1 << SCPM_SW_RST_CTRL_CX_SRST_Pos);

    ui32Status = AMBT53_REG32_WR(SCPM_SW_RST_CTRL_ADDR, ui32ScpmCtrl);
    CHECK_REG_WRITE(ui32Status, SCPM_SW_RST_CTRL_ADDR);

#if (LOAD_FW_DEBUG_LOG)
    ui32Status = AMBT53_REG32_RD(SCPM_SW_RST_CTRL_ADDR, &ui32ScpmCtrl);
    CHECK_REG_READ(ui32Status, SCPM_SW_RST_CTRL_ADDR);

    am_util_stdio_printf("scpm sw rst ctrl addr 0x%x, value: 0x%08x\n", SCPM_SW_RST_CTRL_ADDR, ui32ScpmCtrl);
#endif

    am_util_delay_us(10);

    return ui32Status;
#else
    return 0;
#endif
}

//*****************************************************************************
//
//  Release the ambt53 core reset signal to start running the core.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_core_start(void)
{
    // Add some delay for the ambt53 core ready to run
    am_util_delay_ms(50);

    am_hal_scpm_release_cxrst();

#if SCPM_IS_SUPPORTED
    uint32_t ui32Status;
    uint32_t ui32ScpmCtrl = 0;

    ui32Status = AMBT53_REG32_RD(SCPM_SW_RST_CTRL_ADDR, &ui32ScpmCtrl);
    CHECK_REG_READ(ui32Status, SCPM_SW_RST_CTRL_ADDR);

#if (LOAD_FW_DEBUG_LOG)
    am_util_stdio_printf("scpm sw rst ctrl addr 0x%x, value: 0x%08x\n", SCPM_SW_RST_CTRL_ADDR, ui32ScpmCtrl);
#endif

    // Release the core reset and start to run the loaded ambt53 firmware
    ui32ScpmCtrl &= ~(1 << SCPM_SW_RST_CTRL_CX_SRST_Pos);

    ui32Status = AMBT53_REG32_WR(SCPM_SW_RST_CTRL_ADDR, ui32ScpmCtrl);
    CHECK_REG_WRITE(ui32Status, SCPM_SW_RST_CTRL_ADDR);

#if (LOAD_FW_DEBUG_LOG)
    ui32Status = AMBT53_REG32_RD(SCPM_SW_RST_CTRL_ADDR, &ui32ScpmCtrl);
    CHECK_REG_READ(ui32Status, SCPM_SW_RST_CTRL_ADDR);

    am_util_stdio_printf("scpm sw rst ctrl addr 0x%x, value: 0x%08x\n", SCPM_SW_RST_CTRL_ADDR, ui32ScpmCtrl);
#endif

    // Add some delay for the ambt53 core ready to run
    am_util_delay_ms(50);

    return ui32Status;
#else
    return 0;
#endif
}

//*****************************************************************************
//! am_devices_ambt53_int_service
//!
//! @brief The interrupt service of ambt53.
//!
//*****************************************************************************
static void
am_devices_ambt53_int_service(void* pArg)
{
    am_devices_scpm_int_service();
}

//*****************************************************************************
//
// GPIO interrupt handler.
//
//*****************************************************************************
void
ambt53_sys_irq_isr(void)
{
    uint32_t    ui32IntStatus;

    am_hal_gpio_interrupt_irq_status_get(AMBT53_SYS_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(AMBT53_SYS_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_service(AMBT53_SYS_IRQn, ui32IntStatus);
}

//*****************************************************************************
//
// Initialize ambt53 interface and start to boot.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_boot(void)
{
    uint32_t ui32Status = 1;

    am_util_debug_printf("***** PLEASE do the board connection as below!!! *****\n");
    am_util_debug_printf("*            Apollo4p_evb  <->  AMBT53 FPGA           *\n");
    am_util_debug_printf("*SCPM IRQ        : GPIO%d  <->  J4-41(connected)      *\n", AMBT53_SYS_IRQ_PIN);
    am_util_debug_printf("*SCPM I2C_SDA    : GPIO%d   <->  TP12                  *\n", SCPM_I2C_SDA_PIN);
    am_util_debug_printf("*SCPM I2C_SCL    : GPIO%d   <->  TP11                  *\n\n", SCPM_I2C_SCL_PIN);

    // Init SCPM I2C
    ui32Status = am_devices_scpm_init(SCPM_I2C_MODULE, &stSCPMConfig, &g_ScpmDevHdl, &pvScpmI2cHandle);
    ERROR_RETURN(ui32Status);

    am_hal_scpm_set_cxrst();
    am_hal_scpm_set_globrst();
    am_util_delay_ms(10);
    // Release global reset
    am_hal_scpm_release_globrst();

    // AMBT53_SYS_IRQ_PIN is initiated in am_devices_scpm_init() if SCPM_IS_SUPPORTED is enabled
    am_hal_gpio_pinconfig(AMBT53_SYS_IRQ_PIN, am_hal_gpio_pincfg_input);

    // Init mspi driver
    ui32Status = am_devices_mspi_ambt53_init(MSPI_AMBT53_MODULE, &stRPMsgConfig, &g_RpmsgDevHdl, &pvRpmsgMspiHandle);
    ERROR_RETURN(ui32Status);

    // Enable XIPMM
    ui32Status = am_devices_mspi_ambt53_enable_xip(g_RpmsgDevHdl);
    ERROR_RETURN(ui32Status);

    ui32Status = am_devices_ambt53_mailbox_init(MAILBOX_D2M_THRESHOLD, MAILBOX_INT_THRESHOLD_Msk | MAILBOX_INT_ERROR_Msk);
    ERROR_RETURN(ui32Status);
#if SCPM_IS_SUPPORTED
    // Config the IRQ controller
    am_devices_scpm_control(g_ScpmDevHdl, AM_DEVICES_SCPM_REQ_IRQ_CTRL, NULL);
#endif
    uint32_t IntNum = AMBT53_SYS_IRQ_PIN;
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, IntNum, am_devices_ambt53_int_service, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void*)&IntNum);
#ifdef AM_IRQ_PRIORITY_DEFAULT
    NVIC_SetPriority(AMBT53_SYS_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_SetPriority(AMBT53_MSPI_IRQn, AM_IRQ_PRIORITY_DEFAULT);
#endif // AM_IRQ_PRIORITY_DEFAULT
    NVIC_EnableIRQ(AMBT53_SYS_IRQn);
    NVIC_EnableIRQ(AMBT53_MSPI_IRQn);
    am_hal_interrupt_master_enable();

#if ((LOAD_AMBT53_FIRMWARE == LOAD_ELF) || (LOAD_AMBT53_FIRMWARE == LOAD_HEADER_FILE))
#if (LOAD_AMBT53_FIRMWARE == LOAD_HEADER_FILE)
    ui32Status = am_devices_ambt53_get_image(&g_loadImage);
    ERROR_RETURN(ui32Status);
#endif
    ui32Status = am_devices_ambt53_firmware_load(g_RpmsgDevHdl);
    ERROR_RETURN(ui32Status);
#else
    // Release core reset
    am_hal_scpm_release_cxrst();
#endif
    return ui32Status;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

