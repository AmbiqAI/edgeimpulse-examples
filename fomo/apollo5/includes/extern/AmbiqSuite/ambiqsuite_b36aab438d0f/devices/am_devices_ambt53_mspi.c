//*****************************************************************************
//
//! @file am_devices_ambt53_mspi.c
//!
//! @brief The implementation of Apollo interface to AMBT53 general
//!        MSPI driver.
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
#define MSPI_AMBT53_TRANSFER_TIMEOUT   1000000
#define USE_XIPMM_FOR_REG_WRITE        1

// MSPIn is MSPI0_Type* type, convert it into address to calculate the register's
// physical address using offset and then convert it into 32bit register
#define AP4P_MSPIn_DEBUGXIPDMA(n) (*((volatile uint32_t*)((uint32_t)MSPIn(n) + 0x310)))
//*****************************************************************************
//! Error-handling wrapper macro.
//*****************************************************************************

#define CRITICAL_PRINT(...)                                                      \
    do                                                                           \
    {                                                                            \
        AM_CRITICAL_BEGIN;                                                       \
        am_util_debug_printf(__VA_ARGS__);                                       \
        AM_CRITICAL_END;                                                         \
    } while (0)
//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************

#if (MSPI_DQS_EN)
am_hal_mspi_dqs_t AMBT53DqsCfg[] =
{
  {
    .bDQSEnable             = 1,
    .bDQSSyncNeg            = 0,
    .bEnableFineDelay       = 0,
    .ui8TxDQSDelay          = 0,
    .ui8RxDQSDelay          = 16,
    .ui8RxDQSDelayNeg       = 0,
    .bRxDQSDelayNegEN       = 0,
    .ui8RxDQSDelayHi        = 0,
    .ui8RxDQSDelayNegHi     = 0,
    .bRxDQSDelayHiEN        = 0,
  },
  {
    .bDQSEnable             = 1,
    .bDQSSyncNeg            = 0,
    .bEnableFineDelay       = 0,
    .ui8TxDQSDelay          = 0,
    .ui8RxDQSDelay          = 16,
    .ui8RxDQSDelayNeg       = 0,
    .bRxDQSDelayNegEN       = 0,
    .ui8RxDQSDelayHi        = 0,
    .ui8RxDQSDelayNegHi     = 0,
    .bRxDQSDelayHiEN        = 0,
  },
  {
    .bDQSEnable             = 1,
    .bDQSSyncNeg            = 0,
    .bEnableFineDelay       = 0,
    .ui8TxDQSDelay          = 0,
    .ui8RxDQSDelay          = 16,
    .ui8RxDQSDelayNeg       = 0,
    .bRxDQSDelayNegEN       = 0,
    .ui8RxDQSDelayHi        = 0,
    .ui8RxDQSDelayNegHi     = 0,
    .bRxDQSDelayHiEN        = 0,
  }
};

am_hal_mspi_dqs_t AMBT53DqsDisable =
{
    .bDQSEnable             = 0,
    .bDQSSyncNeg            = 0,
    .bEnableFineDelay       = 0,
    .ui8TxDQSDelay          = 0,
    .ui8RxDQSDelay          = 0,
    .ui8RxDQSDelayNeg       = 0,
    .bRxDQSDelayNegEN       = 0,
    .ui8RxDQSDelayHi        = 0,
    .ui8RxDQSDelayNegHi     = 0,
    .bRxDQSDelayHiEN        = 0,
};
#endif
am_hal_mspi_xip_config_t AMBT53XipConfig[] =
{
  {
    .ui32APBaseAddr       = MSPI0_APERTURE_START_ADDR,
    .eAPMode              = AM_HAL_MSPI_AP_READ_WRITE,
    .eAPSize              = AM_HAL_MSPI_AP_SIZE64M,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
  },
  {
    .ui32APBaseAddr       = MSPI1_APERTURE_START_ADDR,
    .eAPMode              = AM_HAL_MSPI_AP_READ_WRITE,
    .eAPSize              = AM_HAL_MSPI_AP_SIZE64M,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
  },
  {
    .ui32APBaseAddr       = MSPI2_APERTURE_START_ADDR,
    .eAPMode              = AM_HAL_MSPI_AP_READ_WRITE,
    .eAPSize              = AM_HAL_MSPI_AP_SIZE64M,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
  }
};

#if defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_APOLLO5_API)
am_hal_mspi_xip_misc_t gXipMiscCfg[] =
{
  {
    .ui32CEBreak        = 10,
    .bXIPBoundary       = true,
    .bXIPOdd            = false,
    .bAppndOdd          = false,
    .bBEOn              = false,
    .eBEPolarity        = AM_HAL_MSPI_BE_LOW_ENABLE,
  },
  {
    .ui32CEBreak        = 10,
    .bXIPBoundary       = true,
    .bXIPOdd            = false,
    .bAppndOdd          = false,
    .bBEOn              = false,
    .eBEPolarity        = AM_HAL_MSPI_BE_LOW_ENABLE,
  },
  {
    .ui32CEBreak        = 10,
    .bXIPBoundary       = true,
    .bXIPOdd            = false,
    .bAppndOdd          = false,
    .bBEOn              = false,
    .eBEPolarity        = AM_HAL_MSPI_BE_LOW_ENABLE,
  },
#if defined(AM_PART_APOLLO5_API)
  {
    .ui32CEBreak        = 10,
    .bXIPBoundary       = true,
    .bXIPOdd            = false,
    .bAppndOdd          = false,
    .bBEOn              = false,
    .eBEPolarity        = AM_HAL_MSPI_BE_LOW_ENABLE,
  }
#endif
};

/*
 * The Apollo4 Plus/Lite and Apollo5 design the MSPI DEV0CFG1 register which can be configured
 * more options for RX mode. With the default value 0x00008200 (DQSTURN0 = 2, RXSMP0 = 1),
 * we can see there is one additional cycle of SPI CS active low after reading the SPI data.
 * It will cause the xSPI slave (ambt53) reporting burst error and invalid length error.
 * With the specific configuration 0x00008000 (DQSTURN0 = 2, RXSMP0 = 0), there is no more
 * cycle of CS active after reading, which can be the same as Apollo4 RevB MSPI RX behavior.
 * And the burst error and invalid length error can be cleaned. We have not understood all
 * the behavior/performance of this register with different configurations. Let's keep monitoring
 * this and may reconfigure it if we find other better options for the RX mode.
*/
am_hal_mspi_rxcfg_t gMspiRxCfg =
{
    .ui8DQSturn         = 2,
    .bRxHI              = 0,
    .bTaForth           = 0,
    .bHyperIO           = 0,
    .ui8RxSmp           = 0,
    .bRBX               = 0,
    .bWBX               = 0,
    .bSCLKRxHalt        = 0,
    .bRxCapEXT          = 0,
    .ui8Sfturn          = 0,
};
#endif

//
//! @brief
//
am_hal_mspi_config_t AMBT53MspiCfg =
{
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
#if (AMBT53_FMC)
    .bClkonD4             = 1
#else
    .bClkonD4             = 0
#endif
};

//
//!
//
typedef struct
{
    uint32_t ui32TxCmd;
    uint32_t ui32RxCmd;
    uint32_t ui32LimitSize;
}am_devices_mspi_ambt53_cmd_t;

//
//!
//
am_devices_mspi_ambt53_cmd_t g_cmdlimit[] =
{
    {AM_DEVICES_MSPI_AMBT53_WRITE_1BYTE, AM_DEVICES_MSPI_AMBT53_READ_1BYTE, 1},
    {AM_DEVICES_MSPI_AMBT53_WRITE_2BYTES, AM_DEVICES_MSPI_AMBT53_READ_2BYTES, 2},
    {AM_DEVICES_MSPI_AMBT53_WRITE_BL1, AM_DEVICES_MSPI_AMBT53_READ_BL1, 4},
    {AM_DEVICES_MSPI_AMBT53_WRITE_BL2, AM_DEVICES_MSPI_AMBT53_READ_BL2, 8},
    {AM_DEVICES_MSPI_AMBT53_WRITE_BL4, AM_DEVICES_MSPI_AMBT53_READ_BL4, 16},
    {AM_DEVICES_MSPI_AMBT53_WRITE_BL8, AM_DEVICES_MSPI_AMBT53_READ_BL8, 32},
    {AM_DEVICES_MSPI_AMBT53_WRITE_BL16, AM_DEVICES_MSPI_AMBT53_READ_BL16, 64},
    {AM_DEVICES_MSPI_AMBT53_WRITE_BL32, AM_DEVICES_MSPI_AMBT53_READ_BL32, 128},
    {AM_DEVICES_MSPI_AMBT53_WRITE_BL64, AM_DEVICES_MSPI_AMBT53_READ_BL64, 256},
    {AM_DEVICES_MSPI_AMBT53_WRITE_BL128, AM_DEVICES_MSPI_AMBT53_READ_BL128, 512},
    {AM_DEVICES_MSPI_AMBT53_WRITE_BL256, AM_DEVICES_MSPI_AMBT53_READ_BL256, 1024},
    {AM_DEVICES_MSPI_AMBT53_WRITE_UL, AM_DEVICES_MSPI_AMBT53_READ_UL, AM_DEVICES_MSPI_AMBT53_MAX_PACKET_SIZE}
};
//
//!
//
uint32_t g_ui32CmdNum = sizeof(g_cmdlimit) / sizeof(am_devices_mspi_ambt53_cmd_t);

//
//!
//
typedef struct error_reason
{
    const char *error_reason;
} error_reason_t;

static const error_reason_t mspi_status_error[] =
{
    {"BUSY"},
    {"WRITE_TRANSFER"},
    {"INV_CMD ERROR"},
    {"INV_LEN ERROR"},
    {"BUS ERROR"},
    {"UNDERRUN"},
    {"OVERRUN"},
    {"BURST ERROR"}
};

//*****************************************************************************
//! MSPI instance.
//*****************************************************************************
am_devices_mspi_ambt53_t gAMBT53[AM_DEVICES_MSPI_AMBT53_MAX_DEVICE_NUM];
#define AM_DEVICES_MSPI_AMBT53_CHK_HANDLE(h)    ((((am_devices_mspi_ambt53_t *)(h))->pMspiHandle) && (((am_devices_mspi_ambt53_t *)(h))->bOccupied == true))

//*****************************************************************************
//
// Local function prototypes
//
//*****************************************************************************
static void ambt53_mspi_status_check(void)
{
    ambt53_status_reg_t stStatus = {0};
    am_devices_mspi_ambt53_status_get((void*)&gAMBT53[0], &stStatus);
    for (uint8_t i = 2; i < 8; i++)
    {
        if ((stStatus.STATUS & XSPI_TRANSFER_STATUS_RW_MASK) & (1 << i))
        {
            CRITICAL_PRINT("\nAMBT53 XSPI %s detected!!\n", mspi_status_error[i].error_reason);
        }
    }
    while (stStatus.STATUS_b.BUS_ERR); //assert for bus error
    am_devices_mspi_ambt53_status_clear((void*)&gAMBT53[0], &stStatus);
}

static void pfnMSPI_AMBT53_Callback(void *pCallbackCtxt, uint32_t status) ;

static uint32_t ambt53_nonblocking_transfer(void *pHandle,
                                            bool bHiPrio,
                                            bool bWrite,
                                            uint8_t *pui8Buffer,
                                            uint32_t ui32Address,
                                            uint32_t ui32NumBytes,
                                            uint32_t ui32PauseCondition,
                                            uint32_t ui32StatusSetClr,
                                            am_hal_mspi_callback_t pfnCallback,
                                            void *pCallbackCtxt) ;

//*****************************************************************************
//
//  Generic MSPI Command Write function via PIO method.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_pio_write(void *pHandle,
                                 uint16_t ui16Instr,
                                 bool bSendAddr,
                                 uint32_t ui32Addr,
                                 uint32_t *pData,
                                 uint32_t ui32NumBytes)
{
    am_hal_mspi_pio_transfer_t Transaction = {0};
    am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;
    uint32_t ui32Status = AM_DEVICES_AMBT53_STATUS_SUCCESS;
    uint32_t ui32Timeout = 0;

    //
    // Create the transaction.
    //
    Transaction.bScrambling        = false;
    Transaction.eDirection         = AM_HAL_MSPI_TX;
    Transaction.bSendInstr         = true;
    Transaction.bTurnaround        = false;
    Transaction.bEnWRLatency       = false;
    Transaction.bDCX               = false;
    Transaction.bSendAddr          = bSendAddr;
    Transaction.ui32DeviceAddr     = ui32Addr;
    Transaction.ui16DeviceInstr    = ui16Instr;
    Transaction.pui32Buffer        = pData;
    Transaction.ui32NumBytes       = ui32NumBytes;

    //
    // Execute the transction over MSPI.
    //
    do
    {
        AM_CRITICAL_BEGIN
        ui32Status = am_hal_mspi_blocking_transfer(pAMBT53->pMspiHandle, &Transaction, MSPI_AMBT53_TRANSFER_TIMEOUT);
        AM_CRITICAL_END
        // Some non-blocking transfer is not finished, wait a while
        if ((ui32Status == AM_HAL_STATUS_INVALID_OPERATION) && (ui32Timeout < MSPI_AMBT53_TRANSFER_TIMEOUT))
        {
            ui32Timeout++;
            am_hal_delay_us(1);
        }
        else
        {
            break;
        }
    }while(1);

    return ((ui32Status == AM_HAL_STATUS_SUCCESS) ? AM_DEVICES_AMBT53_STATUS_SUCCESS : AM_DEVICES_AMBT53_STATUS_FAIL);
}

//*****************************************************************************
//
//  Generic MSPI Command Read function via PIO method.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_pio_read(void *pHandle, uint16_t ui16Instr, bool bSendAddr, uint32_t ui32Addr, uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_mspi_pio_transfer_t Transaction = {0};
    am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;
    uint32_t ui32Status = AM_DEVICES_AMBT53_STATUS_SUCCESS;
    uint32_t ui32Timeout = 0;

    //
    // Create the transaction.
    //
    Transaction.bScrambling        = false;
    Transaction.bSendInstr         = true;
    Transaction.bEnWRLatency       = false;
    Transaction.bDCX               = false;
    Transaction.bSendAddr          = bSendAddr;
    Transaction.bTurnaround        = true;
    Transaction.ui32DeviceAddr     = ui32Addr;
    Transaction.eDirection         = AM_HAL_MSPI_RX;
    Transaction.ui16DeviceInstr    = ui16Instr;
    Transaction.pui32Buffer        = pData;
    Transaction.ui32NumBytes       = ui32NumBytes;

    //
    // Execute the transaction over MSPI.
    //
    do
    {
        AM_CRITICAL_BEGIN
        ui32Status = am_hal_mspi_blocking_transfer(pAMBT53->pMspiHandle, &Transaction, MSPI_AMBT53_TRANSFER_TIMEOUT);
        AM_CRITICAL_END
        // Some non-blocking transfer is not finished, wait a while
        if ((ui32Status == AM_HAL_STATUS_INVALID_OPERATION) && (ui32Timeout < MSPI_AMBT53_TRANSFER_TIMEOUT))
        {
            ui32Timeout++;
            am_hal_delay_us(1);
        }
        else
        {
            break;
        }
    }while(1);

    return ((ui32Status == AM_HAL_STATUS_SUCCESS) ? AM_DEVICES_AMBT53_STATUS_SUCCESS : AM_DEVICES_AMBT53_STATUS_FAIL);
}

//*****************************************************************************
//
//  Get the ambt53 xSPI basic configuration.
//
//*****************************************************************************
void
am_devices_mspi_ambt53_basic_config_get(void *pHandle)
{
    uint32_t ui32Ver = 0;
    uint32_t ui32Cfg = 0;
    ambt53_status_reg_t stStatus = {0};
    am_devices_mspi_ambt53_pio_read(pHandle, AM_DEVICES_MSPI_AMBT53_READ_VER, false, 0, &ui32Ver, 4);
    am_devices_mspi_ambt53_pio_read(pHandle, AM_DEVICES_MSPI_AMBT53_READ_CFG, false, 0, &ui32Cfg, 4);
    if ( (ui32Ver == 0) || (ui32Cfg == 0) )
    {
        am_util_stdio_printf("The xSPI IP is not working! Please check your hardware...\n");
        while (1);
    }
    am_util_stdio_printf("xSPI IP version: %d.%d.%d\n", (ui32Ver & 0xFF), ((ui32Ver & 0xFF00) >> 8), ((ui32Ver & 0xFF0000) >> 16));
    am_util_stdio_printf("AHB_DW: %d\n", (ui32Cfg & 0xFF000000) >> 24);

    am_devices_mspi_ambt53_status_get(pHandle, &stStatus);
    am_util_stdio_printf("xSPI transfer status: 0x%08X\r\n", stStatus.STATUS);
    am_devices_mspi_ambt53_status_clear(pHandle, &stStatus);
}

//*****************************************************************************
//
//  Get the MSPI status.
//
//*****************************************************************************
void
am_devices_mspi_ambt53_status_get(void *pHandle, ambt53_status_reg_t *pStatus)
{
    am_devices_mspi_ambt53_pio_read(pHandle, AM_DEVICES_MSPI_AMBT53_READ_STS, false, 0, &pStatus->STATUS, 4);
    pStatus->STATUS = BIG_LITTLE_SWAP32(pStatus->STATUS);
}

//*****************************************************************************
//
//  Clear the MSPI status.
//
//*****************************************************************************
void
am_devices_mspi_ambt53_status_clear(void *pHandle, ambt53_status_reg_t *pStatus)
{
    pStatus->STATUS = BIG_LITTLE_SWAP32(pStatus->STATUS);
    am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_WRITE_STS, false, 0, &pStatus->STATUS, 4);
}

//*****************************************************************************
//
//
//
//*****************************************************************************
void
am_devices_mspi_ambt53_remote_reset(void *pHandle)
{
    //Reset to default
    am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOFT_RESET, false, 0, NULL, 0);
    am_hal_delay_us(1); // Delay at least 5 SPI cycles
}

//*****************************************************************************
//
//  Initialize the MSPI remote driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_remote_init(void *pHandle, am_hal_mspi_dev_config_t *pConfig, bool bDQSEn)
{
    am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;

    if ( !bDQSEn )
    {
        uint32_t DummyBytes = 0;
        if ( !pConfig->bTurnaround )
        {
            DummyBytes = 0;
        }
        else
        {
            // Dummy should between 4-255
            if ( pConfig->ui8TurnAround >= 4 )
            {
                DummyBytes = pConfig->ui8TurnAround;
            }
            else
            {
                return AM_DEVICES_AMBT53_STATUS_INVALID_ARG;
            }
        }
        am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SET_DUMMY, false, 0, &DummyBytes, 1);
    }

    switch ( pConfig->eDeviceConfig )
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOPM_1_1_1, false, 0, NULL, 0);
            break;
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
            am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOPM_2_2_2, false, 0, NULL, 0);
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
            if ( bDQSEn )
            {
                am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOPM_4_4_4_DQS, false, 0, NULL, 0);
            }
            else
            {
                am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOPM_4_4_4, false, 0, NULL, 0);
            }
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            if ( bDQSEn )
            {
                am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOPM_8_8_8_DQS, false, 0, NULL, 0);
            }
            else
            {
                am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOPM_8_8_8, false, 0, NULL, 0);
            }
            break;
        case AM_HAL_MSPI_FLASH_DUAL_CE0_1_2_2:
        case AM_HAL_MSPI_FLASH_DUAL_CE1_1_2_2:
            am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOPM_1_2_2, false, 0, NULL, 0);
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
        case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
            if ( bDQSEn )
            {
                am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOPM_1_4_4_DQS, false, 0, NULL, 0);
            }
            else
            {
                am_devices_mspi_ambt53_pio_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOPM_1_4_4, false, 0, NULL, 0);
            }
            break;
        default:
            return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
    pAMBT53->eDeviceConfig = pConfig->eDeviceConfig;
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Sets up the MSPI into XIP mode.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_enable_xip(void *pHandle)
{
  uint32_t ui32Status;
  am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;

  //
  // Set Aperture XIP range
  //
  ui32Status = am_hal_mspi_control(pAMBT53->pMspiHandle, AM_HAL_MSPI_REQ_XIP_CONFIG, &AMBT53XipConfig[pAMBT53->ui32Module]);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
      return AM_DEVICES_AMBT53_STATUS_FAIL;
  }

#if defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_APOLLO5_API)
    am_hal_mspi_xip_misc_t    xipMiscCfg = gXipMiscCfg[pAMBT53->ui32Module];
    ui32Status = am_hal_mspi_control(pAMBT53->pMspiHandle, AM_HAL_MSPI_REQ_XIP_MISC_CONFIG, &xipMiscCfg);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
#endif
  //
  // Enable XIP on the MSPI.
  //
  ui32Status = am_hal_mspi_control(pAMBT53->pMspiHandle, AM_HAL_MSPI_REQ_XIP_EN, &AMBT53XipConfig[pAMBT53->ui32Module]);

  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
      return AM_DEVICES_AMBT53_STATUS_FAIL;
  }

  return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Removes the MSPI from XIP mode.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_disable_xip(void *pHandle)
{
  uint32_t ui32Status;
  am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;

  //
  // Disable XIP on the MSPI.
  //
  ui32Status = am_hal_mspi_control(pAMBT53->pMspiHandle, AM_HAL_MSPI_REQ_XIP_DIS, &AMBT53XipConfig[pAMBT53->ui32Module]);
  if (AM_HAL_STATUS_SUCCESS != ui32Status)
  {
      return AM_DEVICES_AMBT53_STATUS_FAIL;
  }

  return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Initialize the MSPI local driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_init(uint32_t ui32Module, am_devices_mspi_ambt53_config_t *psMSPIConfig, void **ppHandle, void **ppMSPIHandle)
{
    am_hal_mspi_dev_config_t stConfig = {0};
    ambt53_status_reg_t      stStatus = {0};
    void                     *pMspiHandle;
    uint32_t                 ui32Index = 0;

    if ((ui32Module > AM_REG_MSPI_NUM_MODULES) || (ui32Module != MSPI_AMBT53_MODULE) || (psMSPIConfig == NULL))
    {
        return AM_DEVICES_AMBT53_STATUS_INVALID_ARG;
    }

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < AM_DEVICES_MSPI_AMBT53_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( gAMBT53[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == AM_DEVICES_MSPI_AMBT53_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_AMBT53_STATUS_OUT_OF_RANGE;
    }

    stConfig.eClockFreq           = psMSPIConfig->eClockFreq;
    stConfig.eSpiMode             = AM_HAL_MSPI_SPI_MODE_1;
    stConfig.ui8TurnAround        = AMBT53_MSPI_DUMMY;
    stConfig.eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE;
    stConfig.eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE;
    stConfig.bSendInstr           = true;
    stConfig.bSendAddr            = true;
    stConfig.bTurnaround          = true;
    stConfig.ui16ReadInstr        = psMSPIConfig->ui16ReadInstr;
    stConfig.ui16WriteInstr       = psMSPIConfig->ui16WriteInstr;
    stConfig.ui8WriteLatency      = 0;
    stConfig.bEnWriteLatency      = false;
    stConfig.bEmulateDDR          = false;
    stConfig.ui16DMATimeLimit     = 0;
    stConfig.eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE;

    // Configure the MSPI for Serial operation during initialization.
    switch ( psMSPIConfig->eDeviceConfig )
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE0_1_2_2:
        case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
            stConfig.eDeviceConfig = AM_HAL_MSPI_FLASH_SERIAL_CE0;
            break;
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_DUAL_CE1_1_2_2:
        case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            stConfig.eDeviceConfig = AM_HAL_MSPI_FLASH_SERIAL_CE1;
            break;
        default:
            return AM_DEVICES_AMBT53_STATUS_INVALID_ARG;
    }

    //
    // Enable fault detection.
    //
    am_hal_fault_capture_enable();

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    if (AM_HAL_MSPI_CLK_96MHZ == psMSPIConfig->eClockFreq)
    {
        stConfig.ui8TurnAround++;
    }
#endif

    //
    // Initialize the MSPI instance.
    // Enable power to the MSPI instance.
    // Configure the MSPI for Serial operation during initialization.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_initialize(ui32Module, &pMspiHandle))
    {
        am_util_stdio_printf("Error - Failed to initialize MSPI.\n");
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pMspiHandle, AM_HAL_SYSCTRL_WAKE, false))
    {
        am_util_stdio_printf("Error - Failed to power on MSPI.\n");
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    am_hal_mspi_config_t    mspiCfg = AMBT53MspiCfg;
    mspiCfg.ui32TCBSize = psMSPIConfig->ui32NBTxnBufLength;
    mspiCfg.pTCB = psMSPIConfig->pNBTxnBuf;
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_configure(pMspiHandle, &mspiCfg))
    {
        am_util_stdio_printf("Error - Failed to configure MSPI device.\n");
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(pMspiHandle, &stConfig))
    {
        am_util_stdio_printf("Error - Failed to configure MSPI device.\n");
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    am_hal_mspi_xip_config_t    xipCfg = AMBT53XipConfig[ui32Module];
    xipCfg.scramblingStartAddr = psMSPIConfig->ui32ScramblingStartAddr;
    xipCfg.scramblingEndAddr = psMSPIConfig->ui32ScramblingEndAddr;
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_control(pMspiHandle, AM_HAL_MSPI_REQ_XIP_CONFIG, &xipCfg))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(pMspiHandle))
    {
        am_util_stdio_printf("Error - Failed to enable MSPI.\n");
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    //
    // Configure the MSPI pins.
    //
    am_bsp_mspi_pins_enable(ui32Module, stConfig.eDeviceConfig);
#if (AMBT53_FMC)
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_AMBT53_GPIO_MSPI_CE, g_AM_BSP_GPIO_MSPI2_CE1);
    am_hal_gpio_pinconfig(AM_AMBT53_GPIO_MSPI_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
#endif

    gAMBT53[ui32Index].pMspiHandle = pMspiHandle;
    gAMBT53[ui32Index].ui32Module = ui32Module;
    gAMBT53[ui32Index].eDeviceConfig = stConfig.eDeviceConfig;
    gAMBT53[ui32Index].bOccupied = true;

    // Find the max packet size for the configured read/write commands
    for ( uint8_t i = 0; i < g_ui32CmdNum; i++ )
    {
        if ( stConfig.ui16ReadInstr == g_cmdlimit[i].ui32RxCmd )
        {
            gAMBT53[ui32Index].maxRxSize = g_cmdlimit[i].ui32LimitSize;
        }
        if ( stConfig.ui16WriteInstr == g_cmdlimit[i].ui32TxCmd )
        {
            gAMBT53[ui32Index].maxTxSize = g_cmdlimit[i].ui32LimitSize;
        }
        if ( (gAMBT53[ui32Index].maxRxSize) && (gAMBT53[ui32Index].maxTxSize) )
        {
            break;
        }
    }

    // Resume to actual setting to config the remote
    stConfig.eDeviceConfig = psMSPIConfig->eDeviceConfig;

    // Init the mspi slave
    bool bDQS_valid = false;
    // Remote xSPI supports DQS only for 1:4:4, 4:4:4, 1:8:8, 8:8:8
    switch ( stConfig.eDeviceConfig )
    {
        case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
        case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            bDQS_valid = MSPI_DQS_EN;
            break;
        default:
            break;
    }
#if (MSPI_DQS_EN)
    if ( bDQS_valid )
    {
        am_hal_mspi_dqs_t dqsCfg = AMBT53DqsCfg[ui32Module];
        if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_control(pMspiHandle, AM_HAL_MSPI_REQ_DQS, &dqsCfg))
        {
            return AM_DEVICES_AMBT53_STATUS_FAIL;
        }
    }
#endif
    am_devices_mspi_ambt53_remote_reset((void*)&gAMBT53[ui32Index]);
    am_devices_mspi_ambt53_remote_init((void*)&gAMBT53[ui32Index], &stConfig, bDQS_valid);

    // Disable MSPI defore re-configuring it
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_disable(pMspiHandle))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(pMspiHandle, &stConfig))
    {
        am_util_stdio_printf("Error - Failed to configure MSPI device.\n");
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_control(pMspiHandle, AM_HAL_MSPI_REQ_PIOMIXED_CONFIG, &stConfig.eDeviceConfig))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
    // Re-Enable MSPI
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(pMspiHandle))
    {
        am_util_stdio_printf("Error - Failed to enable MSPI.\n");
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
    //
    // Configure the MSPI pins.
    //
    am_bsp_mspi_pins_enable(ui32Module, stConfig.eDeviceConfig);
#if (AMBT53_FMC)
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_AMBT53_GPIO_MSPI_CE, g_AM_BSP_GPIO_MSPI2_CE1);
    am_hal_gpio_pinconfig(AM_AMBT53_GPIO_MSPI_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
#endif
#if (MSPI_DQS_EN)
    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, g_AM_BSP_GPIO_MSPI2_DQSDM);
#endif
    //
    // Enable MSPI interrupts.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_interrupt_clear(pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR ))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_interrupt_enable(pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR ))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

#if defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L) || defined(AM_PART_APOLLO5_API)
    am_hal_mspi_rxcfg_t RxCfg = gMspiRxCfg;
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_control(pMspiHandle, AM_HAL_MSPI_REQ_RXCFG, &RxCfg))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
#endif
    //
    // Clear the MSIP status after initialization in case if there is unexpected pending error.
    //
    am_devices_mspi_ambt53_status_get((void*)&gAMBT53[ui32Index], &stStatus);
    am_devices_mspi_ambt53_status_clear((void*)&gAMBT53[ui32Index], &stStatus);
    memset(&stStatus, 0, sizeof(ambt53_status_reg_t));

    *ppMSPIHandle = pMspiHandle;
    *ppHandle = (void *)&gAMBT53[ui32Index];

    //
    // Return the status.
    //
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  De-Initialize the mspi master driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_deinit(void *pHandle)
{
    am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;

    if ( !AM_DEVICES_MSPI_AMBT53_CHK_HANDLE(pHandle) )
    {
        return AM_DEVICES_AMBT53_STATUS_INVALID_HANDLE;
    }
    if ( pAMBT53->ui32Module > AM_REG_MSPI_NUM_MODULES )
    {
        return AM_DEVICES_AMBT53_STATUS_INVALID_ARG;
    }

    //
    // Disable and clear the interrupts to start with.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_interrupt_disable(pAMBT53->pMspiHandle, 0xFFFFFFFF))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_interrupt_clear(pAMBT53->pMspiHandle, 0xFFFFFFFF))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    // Reset mspi slave
    am_devices_mspi_ambt53_remote_reset(pHandle);

#if (MSPI_DQS_EN)
    // Disable DQS
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_control(pAMBT53->pMspiHandle, AM_HAL_MSPI_REQ_XIP_CONFIG, &AMBT53DqsDisable))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
#endif
    //
    // Deinit and disable power to the MSPI instance.
    //
    am_hal_mspi_disable(pAMBT53->pMspiHandle);
    am_hal_mspi_power_control(pAMBT53->pMspiHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);
    am_hal_mspi_deinitialize(pAMBT53->pMspiHandle);

    // Free this device handle
    pAMBT53->bOccupied = false;
    pAMBT53->maxRxSize = 0;
    pAMBT53->maxTxSize = 0;

    //
    // Return the status.
    //
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Callback for the AMBT53 MSPI.
//!
//! @param pCallbackCtxt -callback context pointer
//! @param status        - unused
//
//*****************************************************************************
static void pfnMSPI_AMBT53_Callback(void *pCallbackCtxt, uint32_t status)
{
    // Set the DMA complete flag.
    *(volatile bool *)pCallbackCtxt = true;
#if 1
// #### INTERNAL BEGIN ####
//TODO/read status and clear error, for debug use
// #### INTERNAL END ####
    ambt53_status_reg_t stStatus = {0};
    am_devices_mspi_ambt53_status_get(&gAMBT53[0], &stStatus);
    if ( stStatus.STATUS & 0xFC )
    {
        am_devices_mspi_ambt53_status_clear(&gAMBT53[0], &stStatus);
        //am_util_stdio_printf("Status: 0x%x\n", stStatus.STATUS);
    }
#endif
}

//*****************************************************************************
//! @brief  Generic non-blocking transfer.
//! @param pHandle
//! @param bHiPrio
//! @param bWrite
//! @param pui8Buffer
//! @param ui32Address
//! @param ui32NumBytes
//! @param ui32PauseCondition
//! @param ui32StatusSetClr
//! @param pfnCallback
//! @param pCallbackCtxt
//! @return
//*****************************************************************************
static uint32_t
ambt53_nonblocking_transfer(void *pHandle,
                            bool bHiPrio,
                            bool bWrite,
                            uint8_t *pui8Buffer,
                            uint32_t ui32Address,
                            uint32_t ui32NumBytes,
                            uint32_t ui32PauseCondition,
                            uint32_t ui32StatusSetClr,
                            am_hal_mspi_callback_t pfnCallback,
                            void *pCallbackCtxt)
{
    am_device_ambt53_wait_mspi_idle(1000);
    // check if any error on the mspi firstly
    ambt53_mspi_status_check();

    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    am_hal_mspi_dma_transfer_t    Transaction;
    am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;
    uint32_t limit = bWrite ? pAMBT53->maxTxSize : pAMBT53->maxRxSize;

    // Set the DMA priority
    Transaction.ui8Priority = 1;
    // Set the transfer direction to RX (Read)
    Transaction.eDirection = bWrite ? AM_HAL_MSPI_TX: AM_HAL_MSPI_RX;
    // Initialize the CQ stimulus.
    Transaction.ui32PauseCondition = ui32PauseCondition;
    // Initialize the post-processing
    Transaction.ui32StatusSetClr = 0;

    while (ui32NumBytes)
    {
        uint32_t size = (ui32NumBytes > limit) ? limit : ui32NumBytes;
        bool bLast = (size == ui32NumBytes);
        // Set the transfer count in bytes.
        Transaction.ui32TransferCount = size;
        // Set the address to read data from.
        Transaction.ui32DeviceAddress = ui32Address;
        // Set the target SRAM buffer address.
        Transaction.ui32SRAMAddress = (uint32_t)pui8Buffer;

        if (bLast)
        {
            Transaction.ui32StatusSetClr = ui32StatusSetClr;
        }

        if (bHiPrio)
        {
            ui32Status = am_hal_mspi_highprio_transfer(pAMBT53->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA,
                                                       bLast ? pfnCallback : NULL,
                                                       bLast ? pCallbackCtxt : NULL);
        }
        else
        {
            ui32Status = am_hal_mspi_nonblocking_transfer(pAMBT53->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA,
                                                          bLast ? pfnCallback : NULL,
                                                          bLast ? pCallbackCtxt : NULL);
        }
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
// Reads the contents of the external SRAM into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_dma_read(void *pHandle,
                                uint8_t *pui8RxBuffer,
                                uint32_t ui32ReadAddress,
                                uint32_t ui32NumBytes,
                                bool bWaitForCompletion)
{
    uint32_t                      ui32Status;

    if (bWaitForCompletion)
    {
        // Start the transaction.
        volatile bool bDMAComplete = false;
        ui32Status = ambt53_nonblocking_transfer(pHandle, false, false,
                                                 pui8RxBuffer,
                                                 ui32ReadAddress,
                                                 ui32NumBytes,
                                                 0,
                                                 0,
                                                 pfnMSPI_AMBT53_Callback,
                                                 (void *)&bDMAComplete);

        // Check the transaction status.
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_AMBT53_STATUS_FAIL;
        }

        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < MSPI_AMBT53_TRANSFER_TIMEOUT; i++)
        {
            if (bDMAComplete)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_hal_delay_us(1);
        }

        // Check the status.
        if (!bDMAComplete)
        {
            return AM_DEVICES_AMBT53_STATUS_FAIL;
        }
    }
    else
    {
        // Check the transaction status.
        ui32Status = ambt53_nonblocking_transfer(pHandle, false, false,
                                                pui8RxBuffer,
                                                ui32ReadAddress,
                                                ui32NumBytes,
                                                0,
                                                0,
                                                NULL,
                                                NULL);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_AMBT53_STATUS_FAIL;
        }
    }
    //
    // Return the status.
    //
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reads the contents of the external ambt53 SRAM into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_dma_read_adv(void *pHandle,
                                    uint8_t *pui8RxBuffer,
                                    uint32_t ui32ReadAddress,
                                    uint32_t ui32NumBytes,
                                    uint32_t ui32PauseCondition,
                                    uint32_t ui32StatusSetClr,
                                    am_hal_mspi_callback_t pfnCallback,
                                    void *pCallbackCtxt)
{
    uint32_t                      ui32Status;
    am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;

    ui32Status = ambt53_nonblocking_transfer(pAMBT53, false, false,
                                             pui8RxBuffer,
                                             ui32ReadAddress,
                                             ui32NumBytes,
                                             ui32PauseCondition,
                                             ui32StatusSetClr,
                                             pfnCallback,
                                             pCallbackCtxt);

    // Check the transaction status.
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Reads the contents of the external sram into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_dma_read_hiprio(void *pHandle,
                                       uint8_t *pui8RxBuffer,
                                       uint32_t ui32ReadAddress,
                                       uint32_t ui32NumBytes,
                                       am_hal_mspi_callback_t pfnCallback,
                                       void *pCallbackCtxt)
{
    uint32_t                      ui32Status;
    am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;

    ui32Status = ambt53_nonblocking_transfer(pAMBT53, true, false,
                                          pui8RxBuffer,
                                          ui32ReadAddress,
                                          ui32NumBytes,
                                          0,
                                          0,
                                          pfnCallback,
                                          pCallbackCtxt);

    // Check the transaction status.
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Programs the given range of ambt53 sram addresses.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_dma_write(void *pHandle,
                                 uint8_t *pui8TxBuffer,
                                 uint32_t ui32WriteAddress,
                                 uint32_t ui32NumBytes,
                                 bool bWaitForCompletion)
{
    uint32_t                      ui32Status;

    if (bWaitForCompletion)
    {
        // Start the transaction.
        volatile bool bDMAComplete = false;
        ui32Status = ambt53_nonblocking_transfer(pHandle, false, true,
                                                pui8TxBuffer,
                                                ui32WriteAddress,
                                                ui32NumBytes,
                                                0,
                                                0,
                                                pfnMSPI_AMBT53_Callback,
                                                (void *)&bDMAComplete);

        // Check the transaction status.
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_AMBT53_STATUS_FAIL;
        }

        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < MSPI_AMBT53_TRANSFER_TIMEOUT; i++)
        {
            if (bDMAComplete)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_hal_delay_us(1);
        }

        // Check the status.
        if (!bDMAComplete)
        {
            return AM_DEVICES_AMBT53_STATUS_FAIL;
        }
    }
    else
    {
        // Check the transaction status.
        ui32Status = ambt53_nonblocking_transfer(pHandle, false, true,
                                                pui8TxBuffer,
                                                ui32WriteAddress,
                                                ui32NumBytes,
                                                0,
                                                0,
                                                NULL,
                                                NULL);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_AMBT53_STATUS_FAIL;
        }
    }

    //
    // Return the status.
    //
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Programs the given range of ambt53 sram addresses.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_dma_write_adv(void *pHandle,
                                     uint8_t *puiTxBuffer,
                                     uint32_t ui32WriteAddress,
                                     uint32_t ui32NumBytes,
                                     uint32_t ui32PauseCondition,
                                     uint32_t ui32StatusSetClr,
                                     am_hal_mspi_callback_t pfnCallback,
                                     void *pCallbackCtxt)
{
    uint32_t                      ui32Status;
    am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;

    ui32Status = ambt53_nonblocking_transfer(pAMBT53, false, true,
                                             puiTxBuffer,
                                             ui32WriteAddress,
                                             ui32NumBytes,
                                             ui32PauseCondition,
                                             ui32StatusSetClr,
                                             pfnCallback,
                                             pCallbackCtxt);

    // Check the transaction status.
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Programs the given range of sram addresses.
//
//*****************************************************************************
uint32_t
am_devices_mspi_ambt53_dma_write_hiprio(void *pHandle,
                                        uint8_t *pui8TxBuffer,
                                        uint32_t ui32WriteAddress,
                                        uint32_t ui32NumBytes,
                                        am_hal_mspi_callback_t pfnCallback,
                                        void *pCallbackCtxt)
{
    uint32_t                      ui32Status;
    am_devices_mspi_ambt53_t *pAMBT53 = (am_devices_mspi_ambt53_t *)pHandle;

    // Check the transaction status.
    ui32Status = ambt53_nonblocking_transfer(pAMBT53, true, true,
                                             pui8TxBuffer,
                                             ui32WriteAddress,
                                             ui32NumBytes,
                                             0,
                                             0,
                                             pfnCallback,
                                             pCallbackCtxt);

    // Check the transaction status.
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Write the given value to the specific register
//
//*****************************************************************************
uint32_t AMBT53_REG32_WR(uint32_t ui32WriteAddr, uint32_t ui32Value)
{
    am_device_ambt53_wait_mspi_idle(1000);
    ambt53_mspi_status_check();
#if (USE_XIPMM_FOR_REG_WRITE)
    *(volatile uint32_t*)(MSPI_XIPMM_BASE_ADDRESS + ui32WriteAddr) = ui32Value;
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
#else
    return am_devices_mspi_ambt53_pio_write((void*)&gAMBT53[0], AM_DEVICES_MSPI_AMBT53_WRITE_BL1, true, ui32WriteAddr, &ui32Value, 4);
#endif
}

//*****************************************************************************
//
//  Read the value from the specific register
//
//*****************************************************************************
uint32_t AMBT53_REG32_RD(uint32_t ui32ReadAddr, uint32_t* pui32Value)
{
    am_device_ambt53_wait_mspi_idle(1000);
    ambt53_mspi_status_check();
    return am_devices_mspi_ambt53_pio_read((void*)&gAMBT53[0], AM_DEVICES_MSPI_AMBT53_READ_BL1, true, ui32ReadAddr, pui32Value, 4);
}

uint32_t am_device_ambt53_wait_mspi_idle(uint32_t block_in_us)
{
    uint32_t mspi_idle = 0;

    do
    {
        // On Apollo4p bit 11: mspi idle (no operation at all, tx/rx/xip-axi handshake etc
        // everything in idle state)
#if defined(AM_PART_APOLLO4P)
        mspi_idle = (AP4P_MSPIn_DEBUGXIPDMA(MSPI_AMBT53_MODULE) >> 11) & 0x01;
        if (mspi_idle)
        {
            break;
        }
#else
        // On Apollo4b, while the register value equals 0, means MSPI is in IDLE
        mspi_idle = AP4P_MSPIn_DEBUGXIPDMA(MSPI_AMBT53_MODULE);
        if (mspi_idle == 0)
        {
            break;
        }
#endif
        am_util_delay_us(1);
    } while (block_in_us--);

    if (block_in_us != 0)
    {
        return AM_DEVICES_AMBT53_STATUS_SUCCESS;
    }
    am_util_stdio_printf("Error - Wait MSPI Idle Timeout.\n");
    return AM_DEVICES_AMBT53_STATUS_TIMEOUT;
}
// #### INTERNAL BEGIN ####
//*****************************************************************************
//   Generic IOM Command Write function.
//*****************************************************************************
uint32_t
am_device_iom_ambt53_command_write(void *pHandle, uint64_t ui64Instr, bool bSendAddr, uint32_t ui32Addr, bool bCont,
                                   uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    am_devices_mspi_ambt53_t *pIom = (am_devices_mspi_ambt53_t *)pHandle;
    uint32_t              ui32Status = AM_DEVICES_AMBT53_STATUS_SUCCESS;

    //
    // Create the transaction.
    //
    if ( bSendAddr )
    {
        Transaction.ui32InstrLen    = 5;
        Transaction.ui64Instr       = (ui64Instr << 32 | ui32Addr);
    }
    else
    {
        Transaction.ui32InstrLen    = 1;
        Transaction.ui64Instr       = ui64Instr;
    }
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = pData;
    Transaction.uPeerInfo.ui32SpiChipSelect = 0;
    Transaction.bContinue       = bCont;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS, g_AM_BSP_GPIO_IOM2_CS);

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pMspiHandle, &Transaction))
    {
        ui32Status = AM_DEVICES_AMBT53_STATUS_FAIL;
    }
    if ( bCont )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS, am_hal_gpio_pincfg_output);
        am_hal_gpio_output_set(AM_BSP_GPIO_IOM2_CS);
    }
    return ui32Status;
}

//*****************************************************************************
//  Generic IOM Command Read function.
//*****************************************************************************
uint32_t
am_device_iom_ambt53_command_read(void *pHandle, uint64_t ui64Instr, bool bSendAddr, uint32_t ui32Addr, bool bCont,
                                  uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t  Transaction;
    am_devices_mspi_ambt53_t *pIom = (am_devices_mspi_ambt53_t *)pHandle;
    uint32_t dummy_data[20];
    uint32_t              ui32Status = AM_DEVICES_AMBT53_STATUS_SUCCESS;

    //
    // Create the transaction.
    //
    if ( bSendAddr )
    {
        Transaction.ui32InstrLen    = 5;
        Transaction.ui64Instr       = (ui64Instr << 32 | ui32Addr);
    }
    else
    {
        Transaction.ui32InstrLen    = 1;
        Transaction.ui64Instr       = ui64Instr;
    }
    Transaction.eDirection        = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes      = ui32NumBytes + AMBT53_MSPI_DUMMY / 8;
    Transaction.pui32RxBuffer     = dummy_data;
    Transaction.uPeerInfo.ui32SpiChipSelect = 0;
    Transaction.bContinue         = bCont;
    Transaction.ui8RepeatCount    = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS, g_AM_BSP_GPIO_IOM2_CS);

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pMspiHandle, &Transaction))
    {
        ui32Status = AM_DEVICES_AMBT53_STATUS_FAIL;
    }
    if ( bCont )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS, am_hal_gpio_pincfg_output);
        am_hal_gpio_output_set(AM_BSP_GPIO_IOM2_CS);
    }
    memcpy(pData, &dummy_data[AMBT53_MSPI_DUMMY / 32], ui32NumBytes);
    return ui32Status;
}

//*****************************************************************************
//! @brief Reset the IOM remote driver.
//! @param pHandle
//*****************************************************************************
static void
am_devices_iom_ambt53_remote_reset(void *pHandle)
{
    //Reset to default
    am_device_iom_ambt53_command_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOFT_RESET, false, 0, false, NULL, 0);
    am_hal_delay_us(1); // Delay at least 5 SPI cycles
}

//*****************************************************************************
//
//! @brief Initialize the IOM remote driver.
//! @param pHandle
//! @param bDQSEn
//! @return
//
//*****************************************************************************
static uint32_t
am_devices_iom_ambt53_remote_init(void *pHandle, bool bDQSEn)
{
    if ( !bDQSEn )
    {
        uint32_t DummyBytes = AMBT53_MSPI_DUMMY;
        am_device_iom_ambt53_command_write(pHandle, AM_DEVICES_MSPI_AMBT53_SET_DUMMY, false, 0, false, &DummyBytes, 1);
    }

    am_device_iom_ambt53_command_write(pHandle, AM_DEVICES_MSPI_AMBT53_SOPM_1_1_1, false, 0, false, NULL, 0);
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//  Initialize the IOM local driver.
//*****************************************************************************
uint32_t
am_devices_iom_ambt53_init(uint32_t ui32Module, am_devices_mspi_ambt53_config_t *pDevConfig, void **ppHandle, void **ppIomHandle)
{
    void *pIomHandle;
    am_hal_iom_config_t     stIOMAMBT53Settings;
    uint32_t      ui32Index = 0;

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < AM_DEVICES_MSPI_AMBT53_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( gAMBT53[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == AM_DEVICES_MSPI_AMBT53_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_AMBT53_STATUS_OUT_OF_RANGE;
    }

    if ( (ui32Module > AM_REG_IOM_NUM_MODULES) || (pDevConfig == NULL) )
    {
        return AM_DEVICES_AMBT53_STATUS_INVALID_ARG;
    }

    //
    // Enable fault detection.
    //
    am_hal_fault_capture_enable();
    stIOMAMBT53Settings.ui32ClockFreq        = AM_HAL_IOM_3MHZ;
    stIOMAMBT53Settings.eInterfaceMode       = AM_HAL_IOM_SPI_MODE,
    stIOMAMBT53Settings.eSpiMode             = AM_HAL_IOM_SPI_MODE_1,
    stIOMAMBT53Settings.ui32NBTxnBufLength   = pDevConfig->ui32NBTxnBufLength;
    stIOMAMBT53Settings.pNBTxnBuf            = pDevConfig->pNBTxnBuf;

    //
    // Initialize the IOM instance.
    // Enable power to the IOM instance.
    // Configure the IOM for Serial operation during initialization.
    // Enable the IOM.
    //
    if (am_hal_iom_initialize(ui32Module, &pIomHandle) ||
        am_hal_iom_power_ctrl(pIomHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_iom_configure(pIomHandle, &stIOMAMBT53Settings) ||
        am_hal_iom_enable(pIomHandle))
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }
    else
    {
        //
        // Configure the IOM pins.
        //
        am_bsp_iom_pins_enable(ui32Module, AM_HAL_IOM_SPI_MODE);

        gAMBT53[ui32Index].bOccupied = true;
        gAMBT53[ui32Index].ui32Module = ui32Module;
        *ppIomHandle = gAMBT53[ui32Index].pMspiHandle = pIomHandle;
        *ppHandle = (void *)&gAMBT53[ui32Index];

        am_devices_iom_ambt53_remote_reset((void*)&gAMBT53[ui32Index]);
        am_devices_iom_ambt53_remote_init((void*)&gAMBT53[ui32Index], false);

        //
        // Return the status.
        //
        return AM_DEVICES_AMBT53_STATUS_SUCCESS;
    }
}

//*****************************************************************************
//
//  De-initialize the IOM local driver.
//
//*****************************************************************************
uint32_t
am_devices_iom_ambt53_deinit(void *pHandle)
{
    am_devices_mspi_ambt53_t *pIom = (am_devices_mspi_ambt53_t *)pHandle;

    if ( !AM_DEVICES_MSPI_AMBT53_CHK_HANDLE(pHandle) )
    {
        return AM_DEVICES_AMBT53_STATUS_INVALID_HANDLE;
    }
    if ( pIom->ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return AM_DEVICES_AMBT53_STATUS_FAIL;
    }

    //
    // Disable the IOM.
    //
    am_hal_iom_disable(pIom->pMspiHandle);

    //
    // Disable power to and uninitialize the IOM instance.
    //
    am_hal_iom_power_ctrl(pIom->pMspiHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);

    am_hal_iom_uninitialize(pIom->pMspiHandle);

    // Free this device handle
    pIom->bOccupied = false;

    //
    // Return the status.
    //
    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}
// #### INTERNAL END ####

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

