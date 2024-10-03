//*****************************************************************************
//
//! @file am_devices_ambt53_scpm.c
//!
//! @brief The implementation of Apollo interface to AMBT53 System Controller
//!        and Power Management (SCPM) module I2C driver.
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
// Allow level mode SCPM interrupt to be registerd or not
#define AM_DEVICES_SCPM_LEVEL_INTERRUPT_EN              (0)
typedef struct
{
    // Bit map stored which irq is enabled, cache it in local variable to shorten
    // I2C operation while executing ISR
    uint32_t irq_enabled[2];
    // IRQ's handler
    am_devices_ambt53_handler irq_handler[SCPM_IRQ_NUM];
}am_devices_ambt53_scpm_int_env;

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
//*****************************************************************************
//! AMBT53 SCPM interrupt handle.
//*****************************************************************************
static am_devices_ambt53_scpm_int_env g_scpm_int_env;

//*****************************************************************************
//! SCPM instance.
//*****************************************************************************
am_devices_scpm_t gAmSCPM[AM_DEVICES_SCPM_MAX_DEVICE_NUM];
#define AM_DEVICES_SCPM_CHK_HANDLE(h)    ((h) && (((am_devices_scpm_t *)(h))->pIomHandle) && (((am_devices_scpm_t *)(h))->bOccupied == true))

//*****************************************************************************
//! SCPM I2C configuration.
//*****************************************************************************
am_hal_iom_config_t     g_sI2cScpmCfg =
{
    .eInterfaceMode       = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq        = AM_HAL_IOM_1MHZ,
    .eSpiMode             = AM_HAL_IOM_SPI_MODE_0,
    .ui32NBTxnBufLength   = 0,
    .pNBTxnBuf = NULL,
};

//*****************************************************************************
//! SCPM I2C clock signal.
//*****************************************************************************
am_hal_gpio_pincfg_t g_AM_DEVICES_SCPM_SCL =
{
    .GP.cfg_b.uFuncSel             = AM_HAL_PIN_8_M1SCL,
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
    .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.uSlewRate            = 0,
    .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_1_5K,
    .GP.cfg_b.uNCE                 = 0,
    .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,
    .GP.cfg_b.uRsvd_0              = 0,
    .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE,
    .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.uRsvd_1              = 0,
};

//*****************************************************************************
//! SCPM I2C data signal.
//*****************************************************************************
am_hal_gpio_pincfg_t g_AM_DEVICES_SCPM_SDA =
{
    .GP.cfg_b.uFuncSel             = AM_HAL_PIN_9_M1SDAWIR3,
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
    .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.uSlewRate            = 0,
    .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_1_5K,
    .GP.cfg_b.uNCE                 = 0,
    .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,
    .GP.cfg_b.uRsvd_0              = 0,
    .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE,
    .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.uRsvd_1              = 0,
};

//*****************************************************************************
//! BT_32K_CLK (45) - BT 32K CLK OUT.
//*****************************************************************************
am_hal_gpio_pincfg_t g_AM_DEVICES_SCPM_32K_CLK =
{
    .GP.cfg_b.uFuncSel             = AM_HAL_PIN_64_32KHzXT,
    .GP.cfg_b.eGPInput             = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero            = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir              = AM_HAL_GPIO_PIN_INTDIR_NONE,
    .GP.cfg_b.eGPOutCfg            = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength       = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .GP.cfg_b.uSlewRate            = 0,
    .GP.cfg_b.ePullup              = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.uNCE                 = 0,
    .GP.cfg_b.eCEpol               = AM_HAL_GPIO_PIN_CEPOL_ACTIVELOW,
    .GP.cfg_b.uRsvd_0              = 0,
    .GP.cfg_b.ePowerSw             = AM_HAL_GPIO_PIN_POWERSW_NONE,
    .GP.cfg_b.eForceInputEn        = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.eForceOutputEn       = AM_HAL_GPIO_PIN_FORCEEN_NONE,
    .GP.cfg_b.uRsvd_1              = 0,
};

//*****************************************************************************
//
//! @brief Generic Command Write function.
//
//*****************************************************************************
static uint32_t
am_device_command_write(void *pHandle, uint8_t ui8DevAddr, uint32_t ui32InstrLen,
                        uint64_t ui64Instr, bool bCont,
                        uint8_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = ui32InstrLen;
    Transaction.ui64Instr       = ui64Instr;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = (uint32_t *)pData;
    Transaction.uPeerInfo.ui32I2CDevAddr = ui8DevAddr;
    Transaction.bContinue       = bCont;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pHandle, &Transaction))
    {
        return AM_DEVICES_SCPM_STATUS_FAIL;
    }
    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Generic Command Read function.
//
//*****************************************************************************
static uint32_t
am_device_command_read(void *pHandle, uint8_t ui8DevAddr, uint32_t ui32InstrLen, uint64_t ui64Instr,
                       bool bCont, uint8_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t  Transaction;
    memset(pData, 0, ui32NumBytes);

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = ui32InstrLen;
    Transaction.ui64Instr       = ui64Instr;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32RxBuffer   = (uint32_t *)pData;
    Transaction.uPeerInfo.ui32I2CDevAddr = ui8DevAddr;
    Transaction.bContinue       = bCont;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pHandle, &Transaction))
    {
        return AM_DEVICES_SCPM_STATUS_FAIL;
    }

    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Set up pins for SCPM.
//!
//! This function configures RESET, IRQ pins for SCPM
//
//*****************************************************************************
static void
am_devices_scpm_pins_enable(void)
{
    am_hal_gpio_pinconfig(SCPM_I2C_SCL_PIN, g_AM_DEVICES_SCPM_SCL);
    am_hal_gpio_pinconfig(SCPM_I2C_SDA_PIN, g_AM_DEVICES_SCPM_SDA);
    //TODO
    //am_hal_gpio_pinconfig(AMBT53_SYS_IRQ_PIN, am_hal_gpio_pincfg_input);
    //am_hal_gpio_pinconfig(AMBT53_32K_CLK_PIN, g_AM_DEVICES_SCPM_32K_CLK);
}

//*****************************************************************************
//
//! @brief Disable pins for SCPM.
//
//*****************************************************************************
static void
am_devices_scpm_pins_disable(void)
{
    am_hal_gpio_pinconfig(AMBT53_SYS_IRQ_PIN, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AMBT53_32K_CLK_PIN, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(SCPM_I2C_SCL_PIN, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(SCPM_I2C_SDA_PIN, am_hal_gpio_pincfg_disabled);
}

//*****************************************************************************
//
//  Initialize the SCPM driver.
//
//*****************************************************************************
uint32_t
am_devices_scpm_init(uint32_t ui32Module,
                     am_devices_scpm_config_t *pDevConfig,
                     void **ppHandle,
                     void **ppIomHandle)
{
    void *pIomHandle;
    am_hal_iom_config_t     stIOMSCPMSettings;
    uint32_t      ui32Index = 0;

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < AM_DEVICES_SCPM_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( gAmSCPM[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == AM_DEVICES_SCPM_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_SCPM_STATUS_OUT_OF_RANGE;
    }

    if ( (ui32Module > AM_REG_IOM_NUM_MODULES) || (pDevConfig == NULL) )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }

    //
    // Enable fault detection.
    //
    am_hal_fault_capture_enable();

    stIOMSCPMSettings = g_sI2cScpmCfg;
    stIOMSCPMSettings.ui32NBTxnBufLength = pDevConfig->ui32NBTxnBufLength;
    stIOMSCPMSettings.pNBTxnBuf = pDevConfig->pNBTxnBuf;
    stIOMSCPMSettings.ui32ClockFreq = pDevConfig->ui32ClockFreq;

    //
    // Initialize the IOM instance.
    // Enable power to the IOM instance.
    // Configure the IOM for Serial operation during initialization.
    // Enable the IOM.
    //
    if (am_hal_iom_initialize(ui32Module, &pIomHandle) ||
        am_hal_iom_power_ctrl(pIomHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_iom_configure(pIomHandle, &stIOMSCPMSettings) ||
        am_hal_iom_enable(pIomHandle))
    {
        return AM_DEVICES_SCPM_STATUS_FAIL;
    }
    else
    {
        //
        // Configure the IOM pins.
        //
        am_devices_scpm_pins_enable();

        // TODO Enable LFCLK
        // am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_ENABLE, 0);

        gAmSCPM[ui32Index].bOccupied = true;
        gAmSCPM[ui32Index].ui32Module = ui32Module;
        *ppIomHandle = gAmSCPM[ui32Index].pIomHandle = pIomHandle;
        *ppHandle = (void *)&gAmSCPM[ui32Index];

        // #### INTERNAL BEGIN ####
        //TODO/Need to switch to AHB clk for FPGA
        // #### INTERNAL END ####
        //TODO update according to the latest scpm register definition
        // SCPM_REG32_WR(SCPM_SYS_CLK_SEL_ADDR, (SCPM_SYS_CLK_SEL_RESET | (SCPM_SYS_CLK_SEL_SCPM_CSR_AHB << SCPM_SYS_CLK_SEL_SCPM_CSR_Pos)));
        am_util_delay_ms(1);

        //
        // Return the status.
        //
        return AM_DEVICES_SCPM_STATUS_SUCCESS;
    }
}

//*****************************************************************************
//
//  De-Initialize the SCPM driver.
//
//*****************************************************************************
uint32_t
am_devices_scpm_term(void *pHandle)
{
    am_devices_scpm_t *pIom = (am_devices_scpm_t *)pHandle;

    if ( !AM_DEVICES_SCPM_CHK_HANDLE(pHandle) )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_HANDLE;
    }
    if ( pIom->ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return AM_DEVICES_SCPM_STATUS_FAIL;
    }

    am_devices_scpm_pins_disable();
    //
    // Disable the IOM.
    //
    am_hal_iom_disable(pIom->pIomHandle);

    //
    // Disable power to and uninitialize the IOM instance.
    //
    am_hal_iom_power_ctrl(pIom->pIomHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);

    am_hal_iom_uninitialize(pIom->pIomHandle);

    // Free this device handle
    pIom->bOccupied = false;

    //
    // Return the status.
    //
    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_scpm_blocking_write(void *pHandle,
                               uint8_t *pui8TxBuffer,
                               uint32_t ui32WriteAddress,
                               uint32_t ui32NumBytes)
{
    am_devices_scpm_t *pIom = (am_devices_scpm_t *)pHandle;

    if ( !AM_DEVICES_SCPM_CHK_HANDLE(pHandle) )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_HANDLE;
    }
    // Should be 4-bytes alignment
    if ( (pui8TxBuffer == NULL) || (ui32WriteAddress % 4 != 0) || (ui32NumBytes != 4) )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }

    //
    // Write the data to the device.
    //
    // AMBT53 is receiving register address Byte0 at first, then Byte1(total 2 bytes)
    ui32WriteAddress = SCPM_I2C_ADDR(ui32WriteAddress);
    ui32WriteAddress = ((ui32WriteAddress >> 8) & 0xFF) | ((ui32WriteAddress & 0xFF) << 8);
    if (am_device_command_write(pIom->pIomHandle, AM_DEVICES_SCPM_SLAVE_ADDR, 2,
                            ui32WriteAddress, false, pui8TxBuffer, ui32NumBytes))
    {
        return AM_DEVICES_SCPM_STATUS_FAIL;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Reads the contents of the fram into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_scpm_blocking_read(void *pHandle,
                              uint8_t *pui8RxBuffer,
                              uint32_t ui32ReadAddress,
                              uint32_t ui32NumBytes)
{
    am_devices_scpm_t   *pIom = (am_devices_scpm_t *)pHandle;

    if ( !AM_DEVICES_SCPM_CHK_HANDLE(pHandle) )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_HANDLE;
    }
    // Should be 4-bytes alignment
    if ( (pui8RxBuffer == NULL) || (ui32ReadAddress % 4 != 0) || (ui32NumBytes != 4) )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }

    // AMBT53 is receiving register address Byte0 at first, then Byte1(total 2 bytes)
    ui32ReadAddress = SCPM_I2C_ADDR(ui32ReadAddress);
    ui32ReadAddress = ((ui32ReadAddress >> 8) & 0xFF) | ((ui32ReadAddress & 0xFF) << 8);
    if (am_device_command_read(pIom->pIomHandle, AM_DEVICES_SCPM_SLAVE_ADDR, 2,
                           ui32ReadAddress, false, pui8RxBuffer, ui32NumBytes))
    {
        return AM_DEVICES_SCPM_STATUS_FAIL;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  scpm control fucntion
//
//*****************************************************************************
uint32_t
am_devices_scpm_control(void *pHandle,
                        am_devices_scpm_request_e eRequest,
                        void *pArgs)
{
    uint32_t           ui32Status = AM_DEVICES_SCPM_STATUS_SUCCESS;
#if 0 //TODO update according to the latest scpm register definition
    if ( !AM_DEVICES_SCPM_CHK_HANDLE(pHandle) )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_HANDLE;
    }

    //
    // Validate the parameters
    //
    if (eRequest > AM_DEVICES_SCPM_REQ_MAX)
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }

    switch ( eRequest )
    {
        case AM_DEVICES_SCPM_REQ_GET_SLEEP_STATE:
            // #### INTERNAL BEGIN ####
            //TODO
            // #### INTERNAL END ####
            break;
        case AM_DEVICES_SCPM_REQ_WAKEUP:
            // #### INTERNAL BEGIN ####
            //TODO
            // #### INTERNAL END ####
            break;
        case AM_DEVICES_SCPM_REQ_GET_IRQ_STATUS:
            if (!pArgs)
            {
                return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
            }
            ui32Status = SCPM_REG32_RD(SCPM_IRQ_STATUS_ADDR, (uint32_t*)pArgs);
            break;
        case AM_DEVICES_SCPM_REQ_CLR_IRQ_STATUS:
            if (!pArgs)
            {
                return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
            }
            ui32Status = SCPM_REG32_WR(SCPM_IRQ_STATUS_ADDR, *(uint32_t*)pArgs);
            break;
        case AM_DEVICES_SCPM_REQ_IRQ_CTRL:
            {
                uint32_t ui32Config = ((SCPM_IRQ_CTRL_MODE_LEVEL << SCPM_IRQ_CTRL_MODE_Pos) | (SCPM_IRQ_CTRL_POLARITY_HIGHACT << SCPM_IRQ_CTRL_POLARITY_Pos));
                ui32Status = SCPM_REG32_WR(SCPM_IRQ_CTRL_ADDR, ui32Config);
                ui32Config = ~((1 << SCPM_IRQ_MAILBOX_THRESHOLD) | (1 << SCPM_IRQ_GPIO0_Pos) | (1 << SCPM_IRQ_GPIO1_Pos));
                ui32Status = SCPM_REG32_WR(SCPM_IRQ_MASK_ADDR, ui32Config);
            }
            break;
        case AM_DEVICES_SCPM_REQ_SW_RESET:
            if (!pArgs)
            {
                return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
            }
            ui32Status = SCPM_REG32_WR(SCPM_SW_RST_CTRL_ADDR, *(uint32_t*)pArgs);
            break;
        case AM_DEVICES_SCPM_REQ_RET_SRAM:
            // #### INTERNAL BEGIN ####
            //TODO
            // #### INTERNAL END ####
            break;
        case AM_DEVICES_SCPM_REQ_STARTUP:
            // #### INTERNAL BEGIN ####
            //TODO
            // #### INTERNAL END ####
            break;
        case AM_DEVICES_SCPM_REQ_SHUTDOWN:
            // #### INTERNAL BEGIN ####
            //TODO
            // #### INTERNAL END ####
            break;
        case AM_DEVICES_SCPM_REQ_EN_TRIMS_LOAD:
            {
                uint32_t ui32SysCtrl;
                ui32Status = SCPM_REG32_RD(SCPM_SYS_CTRL_ADDR, &ui32SysCtrl);
                if (ui32Status == AM_DEVICES_SCPM_STATUS_SUCCESS)
                {
                    ui32SysCtrl |= (1 << SCPM_SYS_CTRL_TRIMS_LOAD_EN_Pos);
                    ui32Status = SCPM_REG32_WR(SCPM_SYS_CTRL_ADDR, ui32SysCtrl);
                }
            }
            break;
        case AM_DEVICES_SCPM_REQ_DIS_TRIMS_LOAD:
            {
                uint32_t ui32SysCtrl;
                ui32Status = SCPM_REG32_RD(SCPM_SYS_CTRL_ADDR, &ui32SysCtrl);
                if (ui32Status == AM_DEVICES_SCPM_STATUS_SUCCESS)
                {
                    ui32SysCtrl &= ~(1 << SCPM_SYS_CTRL_TRIMS_LOAD_EN_Pos);
                    ui32Status = SCPM_REG32_WR(SCPM_SYS_CTRL_ADDR, ui32SysCtrl);
                }
            }
            break;
        default:
            return AM_DEVICES_SCPM_STATUS_INVALID_OPERATION;
    }
#endif
    return ui32Status;
}

void am_devices_scpm_int_service(void)
{
    uint32_t edge_irq_status[2] = {0};
    uint32_t level_irq_status[2] = {0};
    uint32_t irq_num;

    do
    {
        AM_CRITICAL_BEGIN;
        // Read out the two scpm interrupt status registers in one I2C transaction
        SCPM_REG32_RD(SCPM_IRQ_CTRL_MCUINTRPT0STAT_ADDR, &edge_irq_status[0]);
        SCPM_REG32_RD(SCPM_IRQ_CTRL_MCUINTRPT1STAT_ADDR, &edge_irq_status[1]);
#if AM_DEVICES_SCPM_LEVEL_INTERRUPT_EN
        SCPM_REG32_RD(SCPM_IRQ_CTRL_MCUEXTLVLINTSTAT0_ADDR, &level_irq_status[0]);
        SCPM_REG32_RD(SCPM_IRQ_CTRL_MCUEXTLVLINTSTAT1_ADDR, &level_irq_status[1]);
#endif
        // Clear the edge triggered interrupt at first
        if ( edge_irq_status[0] )
        {
            SCPM_REG32_WR(SCPM_IRQ_CTRL_MCUINTRPT0CLR_ADDR, edge_irq_status[0]);
        }
        if ( edge_irq_status[1] )
        {
            SCPM_REG32_WR(SCPM_IRQ_CTRL_MCUINTRPT1CLR_ADDR, edge_irq_status[1]);
        }
        AM_CRITICAL_END;

        // Check if any new interrupt source has raised
        if ( ((edge_irq_status[0] & g_scpm_int_env.irq_enabled[0]) == 0) && ((edge_irq_status[1] & g_scpm_int_env.irq_enabled[1]) == 0 )
          && ((level_irq_status[0] & g_scpm_int_env.irq_enabled[0]) == 0) && ((level_irq_status[1] & g_scpm_int_env.irq_enabled[1]) == 0) )
        {
            break;
        }

        for (uint32_t i = 0; i < 2; i++)
        {
            // Execute the corresponding ISR
            edge_irq_status[i] &= g_scpm_int_env.irq_enabled[i];
            while (edge_irq_status[i])
            {
                // Get the lowest index SCPM interrupt triggered
                for (irq_num = 0; irq_num < 32; irq_num++)
                {
                    if (edge_irq_status[i] & (1 << irq_num))
                    {
                        edge_irq_status[i] &= ~(0x00000001 << irq_num);
                        if (g_scpm_int_env.irq_handler[irq_num + 32 * i] != NULL )
                        {
                            g_scpm_int_env.irq_handler[irq_num + 32 * i]();
                        }
                    }
                }
            }
        #if AM_DEVICES_SCPM_LEVEL_INTERRUPT_EN
            level_irq_status[i] &= g_scpm_int_env.irq_enabled[i];
            while (level_irq_status[i])
            {
                // Get the lowest index SCPM interrupt triggered
                for (irq_num = 0; irq_num < 32; irq_num++)
                {
                    if (level_irq_status[i] & (1 << irq_num))
                    {
                        level_irq_status[i] &= ~(0x00000001 << irq_num);
                        if ( g_scpm_int_env.irq_handler[irq_num + 32 * i] != NULL )
                        {
                            g_scpm_int_env.irq_handler[irq_num + 32 * i]();
                        }
                    }
                }
            }
        #endif
        }
        // Should repeat this loop one time to check if any new interrupt has been triggered
    } while (1);
}

uint32_t
am_devices_ambt53_interrupt_unregister(scpm_irq_map_e irq_num)
{
    uint32_t irq_idx, irq_reg_idx;
    uint32_t scpm_irq_mask_reg, scpm_irq_mode_reg, scpm_irq_mode_pos;
    uint32_t ui32Val;
    if (irq_num >= SCPM_IRQ_NUM)
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }
    // Divide irq_num into two groups whose size is at max 32.
    irq_idx = irq_num & 0x1F;
    irq_reg_idx = irq_num >> 5;
    // Check if the IRQ is not enabled
    if ((g_scpm_int_env.irq_enabled[irq_reg_idx] & (1 << irq_idx)) == 0)
    {
        return AM_DEVICES_SCPM_STATUS_SUCCESS;
    }
    scpm_irq_mask_reg = irq_reg_idx ? SCPM_IRQ_CTRL_MCUINTRPTMASK1_ADDR : SCPM_IRQ_CTRL_MCUINTRPTMASK0_ADDR;
    // Compute the int mode address, each register is 4 bytes
    scpm_irq_mode_reg = SCPM_IRQ_CTRL_INTRPTMODESEL0_ADDR + (irq_num / SCPM_IRQ_CTRL_INTRPTMODESEL_NUM * 4);
    scpm_irq_mode_pos = (irq_num % SCPM_IRQ_CTRL_INTRPTMODESEL_NUM) * SCPM_IRQ_CTRL_INTRPTMODESEL_BIT_SIZE;
    // Disable this mask
    SCPM_REG32_RD(scpm_irq_mask_reg, &ui32Val);
    ui32Val &= ~(1 << irq_idx);
    SCPM_REG32_WR(scpm_irq_mask_reg, ui32Val);

    // Disable this irq mode
    SCPM_REG32_RD(scpm_irq_mode_reg, &ui32Val);
    ui32Val &= ~(0x5 << scpm_irq_mode_pos);
    SCPM_REG32_WR(scpm_irq_mode_reg, ui32Val);
    // Cache the mask in local variable
    g_scpm_int_env.irq_enabled[irq_reg_idx] = ~ui32Val;

    g_scpm_int_env.irq_handler[irq_num] = NULL;
    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//! am_devices_ambt53_interrupt_register
//!
//! @brief Register an interrupt handler for a specific ambt53 SCPM interrupt
//! source.
//!
//*****************************************************************************
uint32_t
am_devices_ambt53_interrupt_register(scpm_irq_map_e irq_num, scpm_irq_mode_e irq_mode, am_devices_ambt53_handler handler)
{
    uint32_t irq_idx, irq_reg_idx;
    uint32_t scpm_irq_en_reg, scpm_irq_mask_reg, scpm_irq_mode_reg, scpm_irq_mode_pos;
    uint32_t ui32Val;

    if ( (irq_mode == SCPM_IRQ_DIS) || (irq_mode > SCPM_IRQ_HIGH_LEVEL) || (handler == NULL) || (irq_num >= SCPM_IRQ_NUM) )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }
    // Divide irq_num into two groups whose size is at max 32.
    irq_idx = irq_num & 0x1F;
    irq_reg_idx = irq_num >> 5;
    scpm_irq_mask_reg = irq_reg_idx ? SCPM_IRQ_CTRL_MCUINTRPTMASK1_ADDR : SCPM_IRQ_CTRL_MCUINTRPTMASK0_ADDR;
    // Compute the int mode address, each register is 4 bytes
    scpm_irq_mode_reg = SCPM_IRQ_CTRL_INTRPTMODESEL0_ADDR + (irq_num / SCPM_IRQ_CTRL_INTRPTMODESEL_NUM * 4);
    scpm_irq_mode_pos = (irq_num % SCPM_IRQ_CTRL_INTRPTMODESEL_NUM) * SCPM_IRQ_CTRL_INTRPTMODESEL_BIT_SIZE;

    // MCUINTRPTxEN reg is for edge mode, MCUEXTLVLINTENx reg is for level mode
    switch (irq_mode)
    {
        case SCPM_IRQ_NEG_EDGE:
        case SCPM_IRQ_POS_EDGE:
        case SCPM_IRQ_BOTH_EDGE:
            scpm_irq_en_reg = irq_reg_idx ? SCPM_IRQ_CTRL_MCUINTRPT1EN_ADDR : SCPM_IRQ_CTRL_MCUINTRPT0EN_ADDR;
            break;
        case SCPM_IRQ_LOW_LEVEL:
        case SCPM_IRQ_HIGH_LEVEL:
        {
            scpm_irq_en_reg = irq_reg_idx ? SCPM_IRQ_CTRL_MCUEXTLVLINTEN1_ADDR : SCPM_IRQ_CTRL_MCUEXTLVLINTEN0_ADDR;
            #if (!AM_DEVICES_SCPM_LEVEL_INTERRUPT_EN)
                return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
            #endif
        }
            break;
        default:
            break;
    }
    // Enable this irq mode
    SCPM_REG32_RD(scpm_irq_mode_reg, &ui32Val);
    ui32Val &= ~(0x7 << scpm_irq_mode_pos);
    ui32Val |= irq_mode << scpm_irq_mode_pos;
    SCPM_REG32_WR(scpm_irq_mode_reg, ui32Val);
    // Enable this irq
    SCPM_REG32_RD(scpm_irq_en_reg, &ui32Val);
    ui32Val |= 1 << irq_idx;
    SCPM_REG32_WR(scpm_irq_en_reg, ui32Val);
    // Enable this mask
    SCPM_REG32_RD(scpm_irq_mask_reg, &ui32Val);
    ui32Val &= ~(1 << irq_idx);
    SCPM_REG32_WR(scpm_irq_mask_reg, ui32Val);
    // Cache the mask in local variable
    g_scpm_int_env.irq_enabled[irq_reg_idx] = ~ui32Val;

    g_scpm_int_env.irq_handler[irq_num] = handler;

    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief init the scpm interrupt controller
 ****************************************************************************************
 */
void am_hal_scpm_intr_init(void)
{
    //CLR intr 0~63 status
    SCPM_REG32_WR(SCPM_IRQ_CTRL_MCUINTRPT0CLR_ADDR, 0xFFFFFFFF);
    SCPM_REG32_WR(SCPM_IRQ_CTRL_MCUINTRPT1CLR_ADDR, 0xFFFFFFFF);
}

/**
 ****************************************************************************************
 * @brief Update the scpm interrup sources' interrupt mode
 * @param int_idx interrupt index
 * @param intr_mode interrupt mode
 ****************************************************************************************
 */
void am_hal_scpm_intr_mode_update(scpm_irq_map_e int_idx, scpm_irq_mode_e intr_mode)
{
    uint32_t reg_val = 0;
    am_hal_interrupt_master_disable();
    SCPM_REG32_RD(SCPM_IRQ_CTRL_INTRPTMODESEL0_ADDR + (int_idx / 10) * 4, &reg_val);
    reg_val &= (~(7 <<  (int_idx % 10) * 3));   //clr the old mode
    reg_val |= (intr_mode << ((int_idx % 10) * 3));
    SCPM_REG32_WR(SCPM_IRQ_CTRL_INTRPTMODESEL0_ADDR + (int_idx / 10) * 4, reg_val);
    am_hal_interrupt_master_enable();
}

/**
 ****************************************************************************************
 * @brief Enable or disable the interrupt of scpm
 * @param int_idx interrupt index
 * @param enable true: enable, false:disable
 ****************************************************************************************
 */
void am_hal_scpm_intr_enable(scpm_irq_map_e int_idx, bool enable)
{
    uint32_t reg_val = 0;
    am_hal_interrupt_master_disable();
    // Enable interrupt
    SCPM_REG32_RD(SCPM_IRQ_CTRL_MCUINTRPT0EN_ADDR + (int_idx / 32) * 0x10, &reg_val);
    reg_val |= 1 << (int_idx % 32);
    SCPM_REG32_WR(SCPM_IRQ_CTRL_MCUINTRPT0EN_ADDR + (int_idx / 32) * 0x10, reg_val);

    // Unmask interrupt
    SCPM_REG32_RD(SCPM_IRQ_CTRL_MCUINTRPTMASK0_ADDR + (int_idx / 32) * 4, &reg_val);

    if (enable == true )
    {
        reg_val &= (~(1 << (int_idx % 32)));
    }
    else
    {
        reg_val |= (1 << (int_idx % 32));
    }

    SCPM_REG32_WR(SCPM_IRQ_CTRL_MCUINTRPTMASK0_ADDR + (int_idx / 32) * 4, reg_val );
    am_hal_interrupt_master_enable();
}

void am_hal_scpm_irq_clr(scpm_irq_map_e int_idx)
{
    if (int_idx >= SCPM_IRQ_NUM)
    {
        return;
    }
    am_hal_interrupt_master_disable();
    SCPM_REG32_WR(SCPM_IRQ_CTRL_MCUINTRPT0CLR_ADDR +
                  (int_idx / 32) * 0x10, 1 << (int_idx % 32));
    am_hal_interrupt_master_enable();
}

/**
 ****************************************************************************************
 * @brief Get the Ambt53 status register value
 * @return dsp_status      SCPMDSPSTA register value
 ****************************************************************************************
 */
uint32_t am_hal_scpm_get_dsp_status(void)
{
    uint32_t dsp_status;

    am_hal_interrupt_master_disable();
    SCPM_REG32_RD(SCPM_DSP_SCPMDSPSTAT_ADDR, &dsp_status);
    am_hal_interrupt_master_enable();
    return dsp_status;
}

/**
 ****************************************************************************************
 * @brief Set the Ambt53 EVECTOR address
 * @param evector      32bits Ambt53 evector value
 ****************************************************************************************
 */
void am_hal_scpm_set_dsp_evector(uint32_t evector)
{
    SCPM_REG32_WR(SCPM_DSP_DSPEVECTOR_ADDR, evector);
}

/**
 ****************************************************************************************
 * @brief Set external wake up, to wake up the Ambt53
 * @param N/A
 ****************************************************************************************
 */
void am_hal_scpm_set_ext_wakeup(void)
{
    dsp_ctrl1_reg_t dsp_ctrl1_val;

    am_hal_interrupt_master_disable();
    SCPM_REG32_RD(SCPM_DSP_SCPMDSPCTL1_ADDR, &dsp_ctrl1_val.value );
    dsp_ctrl1_val.fields.ext_wakeup = 1;

    SCPM_REG32_WR(SCPM_DSP_SCPMDSPCTL1_ADDR, dsp_ctrl1_val.value);
    am_hal_interrupt_master_enable();
}

/**
****************************************************************************************
* @brief Release external wake up,it need to be released after the osc_en rising edge
*interrupt
* @param N/A
****************************************************************************************
*/
void am_hal_scpm_release_ext_wakeup(void)
{
    dsp_ctrl1_reg_t dsp_ctrl1_val;

    am_hal_interrupt_master_disable();
    SCPM_REG32_RD(SCPM_DSP_SCPMDSPCTL1_ADDR, &dsp_ctrl1_val.value );
    dsp_ctrl1_val.fields.ext_wakeup = 0;

    SCPM_REG32_WR(SCPM_DSP_SCPMDSPCTL1_ADDR, dsp_ctrl1_val.value);
    am_hal_interrupt_master_enable();
}

/**
 ****************************************************************************************
 * @brief Set Ambt53 global reset
 * @param N/A
 ****************************************************************************************
 */
void am_hal_scpm_set_globrst(void)
{
    dsp_ctrl1_reg_t dsp_ctrl1_val;

    am_hal_interrupt_master_disable();
    SCPM_REG32_RD(SCPM_DSP_SCPMDSPCTL1_ADDR, &dsp_ctrl1_val.value );

    dsp_ctrl1_val.fields.globrstn = 1;

    SCPM_REG32_WR(SCPM_DSP_SCPMDSPCTL1_ADDR, dsp_ctrl1_val.value);
    am_hal_interrupt_master_enable();
}

/**
 ****************************************************************************************
 * @brief Release Ambt53 global reset
 * @param N/A
 ****************************************************************************************
 */
void am_hal_scpm_release_globrst(void)
{
    dsp_ctrl1_reg_t dsp_ctrl1_val;
    am_hal_interrupt_master_disable();
    SCPM_REG32_RD(SCPM_DSP_SCPMDSPCTL1_ADDR, &dsp_ctrl1_val.value );

    dsp_ctrl1_val.fields.globrstn = 0;

    SCPM_REG32_WR(SCPM_DSP_SCPMDSPCTL1_ADDR, dsp_ctrl1_val.value);
    am_hal_interrupt_master_enable();
}

/**
 ****************************************************************************************
 * @brief Set ambt53 core reset
 * @param N/A
 ****************************************************************************************
 */
void am_hal_scpm_set_cxrst(void)
{
    dsp_ctrl1_reg_t dsp_ctrl1_val;
    am_hal_interrupt_master_disable();
    SCPM_REG32_RD(SCPM_DSP_SCPMDSPCTL1_ADDR, &dsp_ctrl1_val.value );

    dsp_ctrl1_val.fields.cxrstn = 1;

    SCPM_REG32_WR(SCPM_DSP_SCPMDSPCTL1_ADDR, dsp_ctrl1_val.value);
    am_hal_interrupt_master_enable();
}

/**
 ****************************************************************************************
 * @brief Release Ambt53 core reset
 * @param N/A
 ****************************************************************************************
 */
void am_hal_scpm_release_cxrst(void)
{
    dsp_ctrl1_reg_t dsp_ctrl1_val;

    am_hal_interrupt_master_disable();
    SCPM_REG32_RD(SCPM_DSP_SCPMDSPCTL1_ADDR, &dsp_ctrl1_val.value );

    dsp_ctrl1_val.fields.cxrstn = 0;

    SCPM_REG32_WR(SCPM_DSP_SCPMDSPCTL1_ADDR, dsp_ctrl1_val.value);
    am_hal_interrupt_master_enable();
}

/**
 ****************************************************************************************
 * @brief Info the Ambt53 not enter power down mode
 * @param active_enable true:Ambt53 not enter power down,otherwise the Ambt53 can enter the power down
 ****************************************************************************************
 */
void am_hal_scpm_force_active_enable(bool active_enable)
{
    dsp_ctrl1_reg_t dsp_ctrl1_val;

    am_hal_interrupt_master_disable();
    SCPM_REG32_RD(SCPM_DSP_SCPMDSPCTL1_ADDR, &dsp_ctrl1_val.value );

    dsp_ctrl1_val.fields.force_active_state = active_enable;

    SCPM_REG32_WR(SCPM_DSP_SCPMDSPCTL1_ADDR, dsp_ctrl1_val.value);
    am_hal_interrupt_master_enable();
}

//! @}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

