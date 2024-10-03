//*****************************************************************************
//
//! @file am_devices_mspi_mx25um51245g.c
//!
//! @brief General Multibit SPI Mx25um51245g driver.
//!
//! @addtogroup mspi_mx25um51245g MX25UM51245G MSPI Driver
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

#include "am_mcu_apollo.h"
#include "am_devices_mspi_mx25um51245g.h"
#include "am_bsp.h"
#include "am_util.h"
#include "string.h"

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************

#define AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT             1000000
#define AM_DEVICES_MSPI_MX25UM51245G_ERASE_TIMEOUT       1000000

static am_hal_mspi_dev_config_t MSPI_MX25UM51245G_Serial_CE0_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
    .ui16ReadInstr        = AM_DEVICES_MSPI_MX25UM51245G_SPI_FREAD4B,
    .ui16WriteInstr       = AM_DEVICES_MSPI_MX25UM51245G_SPI_PP4B,
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .bNewDDR              = false,
    .eCeLatency           = AM_HAL_MSPI_CE_LATENCY_NORMAL,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
};

static am_hal_mspi_dev_config_t MSPI_MX25UM51245G_Serial_CE1_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE1,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
    .ui16ReadInstr        = AM_DEVICES_MSPI_MX25UM51245G_SPI_FREAD4B,
    .ui16WriteInstr       = AM_DEVICES_MSPI_MX25UM51245G_SPI_PP4B,
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .bNewDDR              = false,
    .eCeLatency           = AM_HAL_MSPI_CE_LATENCY_NORMAL,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
};

static am_hal_mspi_dev_config_t MSPI_MX25UM51245G_OCTAL_CE0_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .ui8TurnAround        = 6,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_2_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_OCTAL_CE0,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
    .ui16ReadInstr        = AM_DEVICES_MSPI_MX25UM51245G_OPI_8READ,
    .ui16WriteInstr       = AM_DEVICES_MSPI_MX25UM51245G_OPI_PP,
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .bNewDDR              = false,
    .eCeLatency           = AM_HAL_MSPI_CE_LATENCY_NORMAL,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
};

static am_hal_mspi_dev_config_t MSPI_MX25UM51245G_OCTAL_CE1_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .ui8TurnAround        = 6,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_2_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_OCTAL_CE1,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
    .ui16ReadInstr        = AM_DEVICES_MSPI_MX25UM51245G_OPI_8READ,
    .ui16WriteInstr       = AM_DEVICES_MSPI_MX25UM51245G_OPI_PP,
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .bNewDDR              = false,
    .eCeLatency           = AM_HAL_MSPI_CE_LATENCY_NORMAL,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
};

static am_hal_mspi_dev_config_t  MSPI_MX25UM51245G_OCTAL_DDR_CE0_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .ui8TurnAround        = 12,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_2_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
    .ui16ReadInstr        = AM_DEVICES_MSPI_MX25UM51245G_OPI_8DTRD,
    .ui16WriteInstr       = AM_DEVICES_MSPI_MX25UM51245G_OPI_PP,
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = true,
    .bNewDDR              = false,
    .eCeLatency           = AM_HAL_MSPI_CE_LATENCY_NORMAL,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
};

static am_hal_mspi_dev_config_t  MSPI_MX25UM51245G_OCTAL_DDR_CE1_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .ui8TurnAround        = 12,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_2_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
    .ui16ReadInstr        = AM_DEVICES_MSPI_MX25UM51245G_OPI_8DTRD,
    .ui16WriteInstr       = AM_DEVICES_MSPI_MX25UM51245G_OPI_PP,
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = true,
    .bNewDDR              = false,
    .eCeLatency           = AM_HAL_MSPI_CE_LATENCY_NORMAL,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
};

static am_hal_mspi_xip_config_t gMx25umXipConfig[] =
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
  },
#if AM_REG_MSPI_NUM_MODULES == 4
  {
    .ui32APBaseAddr       = MSPI3_APERTURE_START_ADDR,
    .eAPMode              = AM_HAL_MSPI_AP_READ_WRITE,
    .eAPSize              = AM_HAL_MSPI_AP_SIZE64M,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
  }
#endif
};

static am_hal_mspi_dqs_t gMx25umDqsCfg[] =
{
  {
    .bDQSEnable             = 0,
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
    .bDQSEnable             = 0,
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
    .bDQSEnable             = 0,
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
    .bDQSEnable             = 0,
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

static am_hal_mspi_timing_scan_t gMx25umTimeCfg[] =
{
    {
// #### INTERNAL BEGIN ####
#if defined(AM_PART_APOLLO5B) || defined(AM_PART_APOLLO510L)
        .bTxDataDelay      = false,
#endif
// #### INTERNAL END ####
        .bTxNeg            = 1,
        .bRxNeg            = 0,
        .bRxCap            = 0,
        .ui8TxDQSDelay     = 4,
        .ui8RxDQSDelay     = 16,
        .ui8Turnaround     = 8,
    },
    {
// #### INTERNAL BEGIN ####
#if defined(AM_PART_APOLLO5B) || defined(AM_PART_APOLLO510L)
        .bTxDataDelay      = false,
#endif
// #### INTERNAL END ####
        .bTxNeg            = 1,
        .bRxNeg            = 0,
        .bRxCap            = 0,
        .ui8TxDQSDelay     = 4,
        .ui8RxDQSDelay     = 16,
        .ui8Turnaround     = 8,
    },
    {
// #### INTERNAL BEGIN ####
#if defined(AM_PART_APOLLO5B) || defined(AM_PART_APOLLO510L)
        .bTxDataDelay      = false,
#endif
// #### INTERNAL END ####
        .bTxNeg            = 1,
        .bRxNeg            = 0,
        .bRxCap            = 0,
        .ui8TxDQSDelay     = 4,
        .ui8RxDQSDelay     = 16,
        .ui8Turnaround     = 8,
    },
    {
// #### INTERNAL BEGIN ####
#if defined(AM_PART_APOLLO5B) || defined(AM_PART_APOLLO510L)
        .bTxDataDelay      = false,
#endif
// #### INTERNAL END ####
        .bTxNeg            = 1,
        .bRxNeg            = 0,
        .bRxCap            = 0,
        .ui8TxDQSDelay     = 4,
        .ui8RxDQSDelay     = 16,
        .ui8Turnaround     = 8,
    },
};

static am_hal_mspi_xip_misc_t gMx25umXipMiscCfg[] =
{
  {
    .ui32CEBreak        = 10,
    .bXIPBoundary       = true, //Deprecated
    .bXIPOdd            = true,
    .bAppndOdd          = false,
    .bBEOn              = false,
    .eBEPolarity        = AM_HAL_MSPI_BE_LOW_ENABLE,
  },
  {
    .ui32CEBreak        = 10,
    .bXIPBoundary       = true, //Deprecated
    .bXIPOdd            = true,
    .bAppndOdd          = false,
    .bBEOn              = false,
    .eBEPolarity        = AM_HAL_MSPI_BE_LOW_ENABLE,
  },
  {
    .ui32CEBreak        = 10,
    .bXIPBoundary       = true, //Deprecated
    .bXIPOdd            = true,
    .bAppndOdd          = false,
    .bBEOn              = false,
    .eBEPolarity        = AM_HAL_MSPI_BE_LOW_ENABLE,
  },
  {
    .ui32CEBreak        = 10,
    .bXIPBoundary       = true, //Deprecated
    .bXIPOdd            = true,
    .bAppndOdd          = false,
    .bBEOn              = false,
    .eBEPolarity        = AM_HAL_MSPI_BE_LOW_ENABLE,
  }
};

static am_hal_mspi_rxcfg_t gMx25umRxCfg =
{
    .ui8DQSturn         = 2,
    .bRxHI              = 0,
    .bTaForth           = 0,
    .bHyperIO           = 0,
    .ui8RxSmp           = 1,
    .bRBX               = 0,
    .bWBX               = 0,
    .bSCLKRxHalt        = 0,
    .bRxCapEXT          = 0,
    .ui8Sfturn          = 0,
};

static am_hal_mspi_config_t gMx25umMspiCfg[] =
{
  {
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .bClkonD4             = 0
  },
  {
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .bClkonD4             = 0
  },
  {
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .bClkonD4             = 0
  },
  {
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .bClkonD4             = 0
  },
};

static am_devices_mspi_mx25um51245g_cmd_t gAmMx25umSPICmdList[] =
{
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_WREN,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_WRDI,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_PAUSE_PGM_ERS,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_RESUME_PGM_ERS,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_DP,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_RDP,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_SBL,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_ENSO,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_EXSO,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_RSTEN,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_RST,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_RDSR,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 2,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_RDCR,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 1,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_WRSR,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 2,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_WRSR,    //WRCR is the second byte of WRSR
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 2,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_RDCR2,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 1,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_WRCR2,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 1,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_PP4B,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE,     //recommended multiple of 16 bytes.
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_SE4B,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_BE4B,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_CE,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_RDID,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 3,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    }
};

static am_devices_mspi_mx25um51245g_cmd_t gAmMx25umOPICmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_MAXNUM] =
{
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_WREN,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_WRDI,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_PAUSE_PGM_ERS,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_RESUME_PGM_ERS,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_DP,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_RDP,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_SBL,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 1,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_ENSO,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_EXSO,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_RSTEN,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_RST,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_RDSR,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 2,
        .bTurnaround = true,
        .ui8Turnaround = 4,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_RDCR,
        .bSendAddr = true,
        .ui32Addr = 0x1,
        .pData = 0,
        .ui32NumBytes = 1,
        .bTurnaround = true,
        .ui8Turnaround = 4,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_WRSR,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 1,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_WRCR,
        .bSendAddr = true,
        .ui32Addr = 0x1,
        .pData = 0,
        .ui32NumBytes = 1,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_RDCR2,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 1,
        .bTurnaround = true,
        .ui8Turnaround = 4,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_WRCR2,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 1,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_PP,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_SE,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_BE,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_CE,
        .bSendAddr = false,
        .ui32Addr = 0,
        .pData = 0,
        .ui32NumBytes = 0,
        .bTurnaround = false,
        .ui8Turnaround = 0,
    },
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_OPI_RDID,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = 0,
        .ui32NumBytes = 3,                                  //SOPI=3, DOPI=6
        .bTurnaround = true,
        .ui8Turnaround = 4,
    }
};

typedef struct
{
    uint32_t                            ui32Module;     //MSPI instance number
    void                                *pMspiHandle;   //MSPI instance handle
    am_hal_mspi_dev_config_t            stSetting;      //MSPI instance setting
    bool                                bOccupied;      //Is device occupied
    uint32_t                            ui32DeviceID;   //Device ID
    am_devices_mspi_mx25um51245g_regs_t sRegisters;     //Device sRegisters
    am_devices_mspi_mx25um51245g_cmd_t  *pCmdList;      //SPI or OPI commands

    am_hal_mspi_config_t                *pMspiCfg;      //pointer to global storage
    am_hal_mspi_xip_config_t            *pXipCfg;       //pointer to global storage
    am_hal_mspi_xip_misc_t              *pXipMiscCfg;   //pointer to global storage
    am_hal_mspi_dqs_t                   *pDqsCfg;       //pointer to global storage
    am_hal_mspi_rxcfg_t                 *pRxCfg;        //pointer to global storage
    am_hal_mspi_timing_scan_t           *pTimingCfg;    //pointer to global storage

    bool                                bTimingValid;   //Are timing scan params valid

} am_devices_mspi_mx25um51245g_t;

static am_devices_mspi_mx25um51245g_t gAmMx25um51245g[AM_DEVICES_MSPI_MX25UM51245G_MAX_DEVICE_NUM];

//*****************************************************************************
//
// Macronix MX25U12335F Support
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief  Set memory storage of default registers for mx25um51245g.
//! @param  pFlash
//! @return ui32Status
//
//*****************************************************************************
static uint32_t
am_devices_mspi_mx25um51245g_default_regs_set(am_devices_mspi_mx25um51245g_t *pFlash)
{
    memset(&pFlash->sRegisters, 0, sizeof(pFlash->sRegisters));

    pFlash->sRegisters.CR_b.ODS = AM_DEVICES_MSPI_MX25UM51245G_ODS_24_Ohms;

    pFlash->sRegisters.CR2_00000300_b.DCn = AM_DEVICES_MSPI_MX25UM51245G_DC_6; //setting dummy cycles to 6

    pFlash->sRegisters.CR2_40000000 = 0xFF;

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}
//*****************************************************************************
//
//! @brief  Get dummy cycle for mx25um51245g.
//! @param  pFlash
//! @param  eTargetClockFreq
//! @param  eDummyCycle
//! @return ui32Status
//
//*****************************************************************************
static uint32_t
am_devices_mspi_mx25um51245g_cal_dummy_cycle(am_devices_mspi_mx25um51245g_t     *pFlash,
                                             am_hal_mspi_clock_e                eTargetClockFreq,
                                             am_devices_mspi_mx25um51245g_DC_e  *eDummyCycle)
{

    if ( pFlash->stSetting.bNewDDR )
    {
        am_util_debug_printf("newDDR currently not supported\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Setting for MX25UM51245GXDIH0 high speed part
    //

    *eDummyCycle = AM_DEVICES_MSPI_MX25UM51245G_DC_20;
    switch(pFlash->stSetting.eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            *eDummyCycle = AM_DEVICES_MSPI_MX25UM51245G_DC_8;
            break;

        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            switch(eTargetClockFreq)
            {
                case AM_HAL_MSPI_CLK_250MHZ:
                    *eDummyCycle = AM_DEVICES_MSPI_MX25UM51245G_DC_20;
                case AM_HAL_MSPI_CLK_192MHZ:
                    *eDummyCycle = AM_DEVICES_MSPI_MX25UM51245G_DC_18;
                    break;
                case AM_HAL_MSPI_CLK_125MHZ:
                    *eDummyCycle = AM_DEVICES_MSPI_MX25UM51245G_DC_14;
                    break;
                case AM_HAL_MSPI_CLK_96MHZ:
                    *eDummyCycle = AM_DEVICES_MSPI_MX25UM51245G_DC_10;
                    break;
                default:
                    *eDummyCycle = AM_DEVICES_MSPI_MX25UM51245G_DC_6;
                    break;
            }
            break;

        case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
            switch(eTargetClockFreq)
            {
                case AM_HAL_MSPI_CLK_250MHZ:    //real output frequency is 125M
                    *eDummyCycle = AM_DEVICES_MSPI_MX25UM51245G_DC_14;
                    break;
                case AM_HAL_MSPI_CLK_192MHZ:    //real output frequency is 96M
                    *eDummyCycle = AM_DEVICES_MSPI_MX25UM51245G_DC_10;
                    break;
                default:
                    *eDummyCycle = AM_DEVICES_MSPI_MX25UM51245G_DC_6;
                    break;
            }
            break;

        default:
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Enter command mode for mx25um51245g.
//! @param  pFlash
//! @return ui32Status
//
//*****************************************************************************
static inline uint32_t
am_devices_mspi_mx25um51245g_enter_command_mode(am_devices_mspi_mx25um51245g_t *pFlash)
{
// #### INTERNAL BEGIN ####
// Purposely avoid using PIO at higher frequency to evade any timing issue
// #### INTERNAL END ####
    am_hal_mspi_clock_e eTargetFreq;
    if (pFlash->stSetting.eClockFreq != AM_HAL_MSPI_CLK_24MHZ)
    {
        eTargetFreq = AM_HAL_MSPI_CLK_24MHZ;
        if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_CLOCK_CONFIG, &eTargetFreq))
        {
            am_util_debug_printf("  Failed to enter command mode\n");
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
    }
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Exit command mode for mx25um51245g.
//! @param  pFlash
//! @return ui32Status
//
//*****************************************************************************
static inline uint32_t
am_devices_mspi_mx25um51245g_exit_command_mode(am_devices_mspi_mx25um51245g_t *pFlash)
{
    am_hal_mspi_clock_e eTargetFreq;
    if (pFlash->stSetting.eClockFreq != AM_HAL_MSPI_CLK_24MHZ)
    {
        eTargetFreq = pFlash->stSetting.eClockFreq;
        if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_CLOCK_CONFIG, &eTargetFreq))
        {
            am_util_debug_printf("  Failed to exit command mode\n");
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }

        if (pFlash->bTimingValid)
        {
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_TIMING_SCAN, pFlash->pTimingCfg))
            {
                return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
        }
    }
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Reconfigure MSPI utility
//! @param  pFlash
//! @return ui32Status
//
//*****************************************************************************
static inline uint32_t
am_devices_mspi_mx25um51245g_reconfigure_mspi(am_devices_mspi_mx25um51245g_t *pFlash, am_hal_mspi_device_e eDeviceConfig)
{
    switch (eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
            switch (pFlash->sRegisters.CR2_00000000_b.MODE)
            {
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_Serial_CE0_MSPIConfig;
                    break;
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_OCTAL_CE0_MSPIConfig;
                    break;
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_OCTAL_DDR_CE0_MSPIConfig;
                    break;
                default:
                    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
            break;
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
            switch (pFlash->sRegisters.CR2_00000000_b.MODE)
            {
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_Serial_CE1_MSPIConfig;
                    break;
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_OCTAL_CE1_MSPIConfig;
                    break;
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_OCTAL_DDR_CE1_MSPIConfig;
                    break;
                default:
                    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
            break;
        default:
            am_util_stdio_printf("Error - Device Config is unavailable.\n");
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Disable MSPI defore re-configuring it
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_disable(pFlash->pMspiHandle))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(pFlash->pMspiHandle, &pFlash->stSetting))
    {
        am_util_stdio_printf("Error - Failed to configure MSPI.\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Re-Enable MSPI
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(pFlash->pMspiHandle))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    am_bsp_mspi_pins_enable(pFlash->ui32Module, pFlash->stSetting.eDeviceConfig);

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Generic Command Write function.
//! @param  pFlash
//! @param  pCmd
//! @return ui32Status
//
//*****************************************************************************
static uint32_t
am_devices_mspi_mx25um51245g_command_write(am_devices_mspi_mx25um51245g_t *pFlash,
                                           am_devices_mspi_mx25um51245g_cmd_t *pCmd)
{
    uint32_t ui32Status;
    am_hal_mspi_pio_transfer_t      stMSPIFlashPIOTransaction = {0};

    //
    // Create the individual write transaction.
    //
    stMSPIFlashPIOTransaction.eDirection         = AM_HAL_MSPI_TX;
    stMSPIFlashPIOTransaction.bSendAddr          = pCmd->bSendAddr;
    stMSPIFlashPIOTransaction.ui32DeviceAddr     = pCmd->ui32Addr;
    stMSPIFlashPIOTransaction.bSendInstr         = true;
    stMSPIFlashPIOTransaction.ui16DeviceInstr    = pCmd->ui16Cmd;
    stMSPIFlashPIOTransaction.pui32Buffer        = pCmd->pData;
    stMSPIFlashPIOTransaction.ui32NumBytes       = pCmd->ui32NumBytes;
    stMSPIFlashPIOTransaction.bTurnaround        = false;

    stMSPIFlashPIOTransaction.bDCX               = false;
    stMSPIFlashPIOTransaction.bEnWRLatency       = false;

    //
    // Execute the transction over MSPI.
    //
    ui32Status = am_hal_mspi_blocking_transfer(pFlash->pMspiHandle,
                                               &stMSPIFlashPIOTransaction,
                                               AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT);

    return ui32Status;
}

//*****************************************************************************
//
//! @brief  Generic Command Read function.
//! @param  pFlash
//! @param  pCmd
//! @return ui32Status
//
//*****************************************************************************
static uint32_t
am_devices_mspi_mx25um51245g_command_read(am_devices_mspi_mx25um51245g_t *pFlash,
                                          am_devices_mspi_mx25um51245g_cmd_t *pCmd)
{
    uint32_t ui32Status;
    uint8_t ui8TempTurnAround;
    am_hal_mspi_pio_transfer_t      stMSPIFlashPIOTransaction = {0};
    am_hal_mspi_timing_scan_t       sTimingParam;

    //
    //get timing scan param configured by init & hal
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_TIMING_SCAN_GET, &sTimingParam);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return ui32Status;
    }
    ui8TempTurnAround = sTimingParam.ui8Turnaround;

    //
    // Create the individual write transaction.
    //
    stMSPIFlashPIOTransaction.eDirection         = AM_HAL_MSPI_RX;
    stMSPIFlashPIOTransaction.bSendAddr          = pCmd->bSendAddr;
    stMSPIFlashPIOTransaction.ui32DeviceAddr     = pCmd->ui32Addr;
    stMSPIFlashPIOTransaction.bSendInstr         = true;
    stMSPIFlashPIOTransaction.ui16DeviceInstr    = pCmd->ui16Cmd;
    stMSPIFlashPIOTransaction.pui32Buffer        = pCmd->pData;
    stMSPIFlashPIOTransaction.ui32NumBytes       = pCmd->ui32NumBytes;
    stMSPIFlashPIOTransaction.bTurnaround        = pCmd->bTurnaround;

    stMSPIFlashPIOTransaction.bDCX               = false;
    stMSPIFlashPIOTransaction.bEnWRLatency       = false;

    if ( pCmd->bTurnaround )
    {
        if ( pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI )
        {
            sTimingParam.ui8Turnaround = pCmd->ui8Turnaround * 2;
        }
        else
        {
            sTimingParam.ui8Turnaround = pCmd->ui8Turnaround;
        }
    }

    //
    // reconfigure timing parameter.
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_TIMING_SCAN, &sTimingParam);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return ui32Status;
    }

    //
    // Execute the transction over MSPI.
    //
    ui32Status = am_hal_mspi_blocking_transfer(pFlash->pMspiHandle,
                                               &stMSPIFlashPIOTransaction,
                                               AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT);

    sTimingParam.ui8Turnaround = ui8TempTurnAround;
    //
    // restore timing parameter.
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_TIMING_SCAN, &sTimingParam);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return ui32Status;
    }

    return ui32Status;
}

//*****************************************************************************
//
//! @brief  Read the status register entry of the mspi_mx25um51245g driver.
//! @param  pHandle
//! @return ui32Status
//
//*****************************************************************************
static uint32_t
am_devices_mspi_mx25um51245g_status(void *pHandle)
{
    uint32_t        ui32Status;
    uint32_t        ui32PIObuffer;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Send the command sequence to read the device status.
    //
    am_devices_mspi_mx25um51245g_cmd_t *pCmd = &(pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_RDSR]);
    pCmd->pData = &ui32PIObuffer;
    ui32Status = am_devices_mspi_mx25um51245g_command_read(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
    pFlash->sRegisters.SR = (uint8_t)ui32PIObuffer;

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Write register entry of the mspi_mx25um51245g driver.
//! @param  pFlash
//! @param  pCmd
//! @return ui32Status
//
//*****************************************************************************
static uint32_t
am_devices_mspi_mx25um51245g_write_register(am_devices_mspi_mx25um51245g_t *pFlash,
                                            am_devices_mspi_mx25um51245g_cmd_t *pCmd)
{
    uint32_t ui32Status;
    //
    // Write enable.
    //
    am_devices_mspi_mx25um51245g_cmd_t *pWRENCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WREN];
    ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pWRENCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Read write enable status.
    //
    ui32Status = am_devices_mspi_mx25um51245g_status(pFlash);
    if ( AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != ui32Status || !pFlash->sRegisters.SR_b.WEL )
    {
        am_util_debug_printf("Read status failed\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  MSPI initialization.
//! @param  ui32Module
//! @param  psMSPISettings
//! @param  pFlash
//! @return ui32Status
//
//*****************************************************************************
static uint32_t am_devices_mspi_peripheral_init(uint32_t ui32Module,
                                                am_devices_mspi_mx25um51245g_config_t *psMSPISettings,
                                                am_devices_mspi_mx25um51245g_t *pFlash)
{
    //
    // Configure the MSPI mode based on device default
    //
    switch (psMSPISettings->eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
            switch (pFlash->sRegisters.CR2_00000000_b.MODE)
            {
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_Serial_CE0_MSPIConfig;
                    break;
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_OCTAL_CE0_MSPIConfig;
                    break;
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_OCTAL_DDR_CE0_MSPIConfig;
                    break;
                default:
                    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
            break;
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
            switch (pFlash->sRegisters.CR2_00000000_b.MODE)
            {
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_Serial_CE1_MSPIConfig;
                    break;
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_OCTAL_CE1_MSPIConfig;
                    break;
                case AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI:
                    pFlash->stSetting = MSPI_MX25UM51245G_OCTAL_DDR_CE1_MSPIConfig;
                    break;
                default:
                    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
            break;
        default:
            am_util_stdio_printf("Error - Device Config is unavailable.\n");
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_initialize(ui32Module, &pFlash->pMspiHandle))
    {
        am_util_stdio_printf("Error - Failed to initialize MSPI.\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pFlash->pMspiHandle, AM_HAL_SYSCTRL_WAKE, false))
    {
        am_util_stdio_printf("Error - Failed to power on MSPI.\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    pFlash->ui32Module = ui32Module;
    pFlash->pMspiCfg->ui32TCBSize = psMSPISettings->ui32NBTxnBufLength;
    pFlash->pMspiCfg->pTCB = psMSPISettings->pNBTxnBuf;
    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_configure(pFlash->pMspiHandle, pFlash->pMspiCfg))
    {
        am_util_stdio_printf("Error - Failed to configure MSPI device.\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(pFlash->pMspiHandle, &pFlash->stSetting))
    {
        am_util_stdio_printf("Error - Failed to configure MSPI.\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(pFlash->pMspiHandle))
    {
        am_util_stdio_printf("Error - Failed to enable MSPI.\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
    am_bsp_mspi_pins_enable(ui32Module, pFlash->stSetting.eDeviceConfig);

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Device specific initialization function.
//! @param  pFlash
//! @param  psMSPISettings
//! @return ui32Status
//
//*****************************************************************************
static uint32_t
am_device_init_flash(am_devices_mspi_mx25um51245g_t *pFlash, am_devices_mspi_mx25um51245g_config_t *psMSPISettings)
{
    uint32_t      ui32Status;
    uint32_t      ui32PIOBuffer[32] = {0};
    am_devices_mspi_mx25um51245g_cmd_t *pCmd;

    //
    // Auto detect device mode.
    //
    for ( am_devices_mspi_mx25um51245g_MODE_e eMode = AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI; eMode <= AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI; eMode++ )
    {
        if ( eMode != pFlash->sRegisters.CR2_00000000_b.MODE )
        {
            pFlash->sRegisters.CR2_00000000_b.MODE = eMode;
            if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_reconfigure_mspi(pFlash, psMSPISettings->eDeviceConfig))
            {
                return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
            if ( pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI )
            {
                pFlash->pCmdList = gAmMx25umSPICmdList;
            }
            else
            {
                pFlash->pCmdList = gAmMx25umOPICmdList;
            }
        }
        ui32Status = am_devices_mspi_mx25um51245g_id(pFlash);
        if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS == ui32Status)
        {
            break;
        }
    }

    //
    // All trials failed
    //
    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Configure the Macronix MX25U2835F Device mode.
    //
    pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WRCR2];
    pCmd->pData = ui32PIOBuffer;
    switch (pFlash->sRegisters.CR2_00000000_b.MODE)
    {
        case AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI:
            switch (psMSPISettings->eDeviceConfig)
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    //
                    // Nothing to do
                    //
                    pFlash->pCmdList = gAmMx25umSPICmdList;
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    if ( psMSPISettings->bDQSEnable )
                    {
                        pFlash->sRegisters.CR2_00000200_b.DOS = 1;
                        pFlash->sRegisters.CR2_00000200_b.DQSPRC = 1;
                        ui32PIOBuffer[0] = pFlash->sRegisters.CR2_00000200;
                        pCmd->ui32Addr = 0x00000200;
                        ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
                        if (AM_HAL_STATUS_SUCCESS != ui32Status)
                        {
                            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
                        }
                    }
                    ui32PIOBuffer[0] = AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI;
                    pCmd->ui32Addr = 0x00000000;
                    ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
                    if (AM_HAL_STATUS_SUCCESS != ui32Status)
                    {
                        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
                    }
                    pFlash->sRegisters.CR2_00000000_b.MODE = AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI;
                    pFlash->pCmdList = gAmMx25umOPICmdList;
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    ui32PIOBuffer[0] = AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI;
                    pCmd->ui32Addr = 0x00000000;
                    ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
                    if (AM_HAL_STATUS_SUCCESS != ui32Status)
                    {
                        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
                    }
                    pFlash->sRegisters.CR2_00000000_b.MODE = AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI;
                    pFlash->pCmdList = gAmMx25umOPICmdList;
                    break;
                default:
                    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
            break;
        case AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI:
            switch (psMSPISettings->eDeviceConfig)
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    ui32PIOBuffer[0] = AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI;
                    pCmd->ui32Addr = 0x00000000;
                    ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
                    if (AM_HAL_STATUS_SUCCESS != ui32Status)
                    {
                        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
                    }
                    pFlash->sRegisters.CR2_00000000_b.MODE = AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI;
                    pFlash->pCmdList = gAmMx25umSPICmdList;
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    if ( psMSPISettings->bDQSEnable )
                    {
                        pFlash->sRegisters.CR2_00000200_b.DOS = 1;
                        pFlash->sRegisters.CR2_00000200_b.DQSPRC = 1;
                        ui32PIOBuffer[0] = pFlash->sRegisters.CR2_00000200;
                        pCmd->ui32Addr = 0x00000200;
                        ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
                        if (AM_HAL_STATUS_SUCCESS != ui32Status)
                        {
                            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
                        }
                    }
                    pFlash->pCmdList = gAmMx25umOPICmdList;
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    ui32PIOBuffer[0] = AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI;
                    pCmd->ui32Addr = 0x00000000;
                    ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
                    if (AM_HAL_STATUS_SUCCESS != ui32Status)
                    {
                        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
                    }
                    pFlash->sRegisters.CR2_00000000_b.MODE = AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI;
                    pFlash->pCmdList = gAmMx25umOPICmdList;
                    break;
                default:
                    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
            break;
        case AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI:
            switch (psMSPISettings->eDeviceConfig)
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    ui32PIOBuffer[0] = AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI;
                    pCmd->ui32Addr = 0x00000000;
                    ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
                    if (AM_HAL_STATUS_SUCCESS != ui32Status)
                    {
                        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
                    }
                    pFlash->sRegisters.CR2_00000000_b.MODE = AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI;
                    pFlash->pCmdList = gAmMx25umSPICmdList;
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    if ( psMSPISettings->bDQSEnable )
                    {
                        pFlash->sRegisters.CR2_00000200_b.DOS = 1;
                        pFlash->sRegisters.CR2_00000200_b.DQSPRC = 1;
                        ui32PIOBuffer[0] = pFlash->sRegisters.CR2_00000200;
                        pCmd->ui32Addr = 0x00000200;
                        ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
                        if (AM_HAL_STATUS_SUCCESS != ui32Status)
                        {
                            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
                        }
                    }
                    ui32PIOBuffer[0] = AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI;
                    pCmd->ui32Addr = 0x00000000;
                    ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
                    if (AM_HAL_STATUS_SUCCESS != ui32Status)
                    {
                        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
                    }
                    pFlash->sRegisters.CR2_00000000_b.MODE = AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI;
                    pFlash->pCmdList = gAmMx25umOPICmdList;
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    //
                    // Nothing to do
                    //
                    pFlash->pCmdList = gAmMx25umOPICmdList;
                    break;
                default:
                    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
            break;
        default:
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if ( psMSPISettings->eDeviceConfig != pFlash->stSetting.eDeviceConfig )
    {
        //
        // Configure the MSPI mode based on current device mode
        //
        if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_reconfigure_mspi(pFlash, psMSPISettings->eDeviceConfig))
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
    }

    //
    // Verify device mode
    //
    pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_RDCR2];
    pCmd->ui32Addr = 0x00000000;
    pCmd->pData = ui32PIOBuffer;
    ui32Status = am_devices_mspi_mx25um51245g_command_read(pFlash, pCmd);
    am_util_debug_printf("Read CR2, Mode %x\n", ui32PIOBuffer[0]);
    if (AM_HAL_STATUS_SUCCESS != ui32Status || ui32PIOBuffer[0] != pFlash->sRegisters.CR2_00000000)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Set Dummy cycle to device
    //
    am_devices_mspi_mx25um51245g_DC_e eDummyCycle;
    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_cal_dummy_cycle(pFlash, psMSPISettings->eClockFreq, &eDummyCycle))
    {
        am_util_debug_printf("Get dummy cycle failed\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
#if defined(APOLLO5_FPGA)
    pFlash->sRegisters.CR2_00000300_b.DCn = AM_DEVICES_MSPI_MX25UM51245G_DC_6;
#else
    pFlash->sRegisters.CR2_00000300_b.DCn = eDummyCycle;
#endif

    ui32PIOBuffer[0] = pFlash->sRegisters.CR2_00000300;
    pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WRCR2];
    pCmd->ui32Addr = 0x00000300;
    pCmd->pData = ui32PIOBuffer;
    ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Verify device dummy cycle
    //
    pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_RDCR2];
    pCmd->ui32Addr = 0x00000300;
    pCmd->pData = ui32PIOBuffer;
    ui32Status = am_devices_mspi_mx25um51245g_command_read(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status || ui32PIOBuffer[0] != pFlash->sRegisters.CR2_00000300)
    {
        am_util_debug_printf("CR2 Dummy Cycle:%x\n", ui32PIOBuffer[0]);
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Device specific de-initialization function.
//! @param  pFlash
//! @return ui32Status
//
//*****************************************************************************
static uint32_t
am_device_deinit_flash(am_devices_mspi_mx25um51245g_t *pFlash)
{
    uint32_t      ui32Status;
    uint32_t      ui32PIOBuffer[2] = {0};

    am_devices_mspi_mx25um51245g_cmd_t *pCmd;

    //
    // Configure the Macronix MX25U2835F Device mode.
    //
    switch (pFlash->stSetting.eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            //
            // Nothing to do.
            //
            pFlash->pCmdList = gAmMx25umSPICmdList;
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
            ui32PIOBuffer[0] = AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI;
            pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WRCR2];
            pCmd->ui32Addr = 0x00000000;
            pCmd->pData = ui32PIOBuffer;
            ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
            if (AM_HAL_STATUS_SUCCESS != ui32Status)
            {
                return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
            pFlash->sRegisters.CR2_00000000_b.MODE = AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI;
            pFlash->pCmdList = gAmMx25umSPICmdList;
            break;
        default:
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief This function takes care of splitting the transaction as needed,
//! if the transaction crosses flash page boundary and if hardware does not support it
//!
//! @param  pFlash
//! @param  bHiPrio
//! @param  eDirection
//! @param  pui8Buffer
//! @param  ui32Address
//! @param  ui32NumBytes
//! @param  ui32PauseCondition
//! @param  ui32StatusSetClr
//! @param  pfnCallback
//! @param  pCallbackCtxt
//!
//! @return ui32Status
//
//*****************************************************************************
static uint32_t
am_devices_mspi_mx25um51245g_nonblocking_transfer(am_devices_mspi_mx25um51245g_t *pFlash,
                                                  bool bHiPrio,
                                                  am_hal_mspi_dir_e eDirection,
                                                  uint8_t *pui8Buffer,
                                                  uint32_t ui32Address,
                                                  uint32_t ui32NumBytes,
                                                  uint32_t ui32PauseCondition,
                                                  uint32_t ui32StatusSetClr,
                                                  am_hal_mspi_callback_t pfnCallback,
                                                  void *pCallbackCtxt)
{
    uint32_t ui32Status = AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
    am_hal_mspi_dma_transfer_t    Transaction;

    if ( pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI && ui32Address % 2 == 1 )
    {
        //
        //Address not even
        //
        return AM_HAL_STATUS_INVALID_OPERATION;
    }

    //
    // Set the DMA priority
    //
    Transaction.ui8Priority = 1;

    //
    // Set the transfer direction to RX (Read)
    //
    Transaction.eDirection = eDirection;

    //
    // Initialize the CQ stimulus.
    //
    Transaction.ui32PauseCondition = ui32PauseCondition;

    //
    // Initialize the post-processing
    //
    Transaction.ui32StatusSetClr = ui32StatusSetClr;

    //
    // Set the transfer count in bytes.
    //
    Transaction.ui32TransferCount = ui32NumBytes;

    //
    // Set the address to read data from.
    //
    Transaction.ui32DeviceAddress = ui32Address;

    //
    // Set the target SRAM buffer address.
    //
    Transaction.ui32SRAMAddress = (uint32_t)pui8Buffer;

    if (bHiPrio)
    {
        ui32Status = am_hal_mspi_highprio_transfer(pFlash->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA,
                                                   pfnCallback,
                                                   pCallbackCtxt);
    }
    else
    {
        ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA,
                                                      pfnCallback,
                                                      pCallbackCtxt);
    }

    return ui32Status;
}

//*****************************************************************************
//
//! @brief example user callback for DMA operation
//! @param pCallbackCtxt
//! @param status
//
//*****************************************************************************
static void
pfnMSPI_MX25UM51245G_Callback(void *pCallbackCtxt, uint32_t status)
{
#if defined(AM_PART_APOLLO5_API)
    //
    // Flush and invalidate whole cache
    // Recommend user to manage cache coherency based on application usage
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);
#endif
    //
    // Set the DMA complete flag.
    //
    *(volatile uint32_t *)pCallbackCtxt = status;
}

//*****************************************************************************
//
// Initialization entry of the mspi_mx25um51245g driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_init(uint32_t ui32Module,
                                  am_devices_mspi_mx25um51245g_config_t *psMSPISettings,
                                  void **ppHandle, void **ppMspiHandle)
{
    uint32_t                        ui32Status;
    am_hal_mspi_dev_config_t        *psConfig;
    am_devices_mspi_mx25um51245g_t  *pFlash = NULL;

    if ((ui32Module > AM_REG_MSPI_NUM_MODULES) || (psMSPISettings == NULL))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if ( psMSPISettings->ui32DeviceNum > AM_DEVICES_MSPI_MX25UM51245G_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if ( gAmMx25um51245g[psMSPISettings->ui32DeviceNum].bOccupied == false )
    {
        pFlash = &gAmMx25um51245g[psMSPISettings->ui32DeviceNum];
        pFlash->pMspiCfg     = &gMx25umMspiCfg[ui32Module];
        pFlash->pXipCfg      = &gMx25umXipConfig[ui32Module];
        pFlash->pXipMiscCfg  = &gMx25umXipMiscCfg[ui32Module];
        pFlash->pDqsCfg      = &gMx25umDqsCfg[ui32Module];
        pFlash->pRxCfg       = &gMx25umRxCfg;
        pFlash->pTimingCfg   = &gMx25umTimeCfg[ui32Module];

        am_devices_mspi_mx25um51245g_default_regs_set(pFlash);  //user may choose to set some defaults based on OTP
        if ( pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_SPI )
        {
            pFlash->pCmdList = gAmMx25umSPICmdList;
        }
        else
        {
            pFlash->pCmdList = gAmMx25umOPICmdList;
        }
        psConfig = &pFlash->stSetting;
    }
    else
    {
        am_util_debug_printf("Device %d occupied!!\n", psMSPISettings->ui32DeviceNum);
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Enable fault detection.
    //
#if !defined(AM_PART_APOLLO5_API)
    am_hal_fault_capture_enable();
#endif

    if (AM_HAL_STATUS_SUCCESS != am_devices_mspi_peripheral_init(ui32Module, psMSPISettings, pFlash))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    *ppHandle = (void *)pFlash;
    *ppMspiHandle = pFlash->pMspiHandle;

    //
    // Device specific MSPI Mx25um51245g initialization.
    //
    if (AM_HAL_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_reset((void*)pFlash))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    ui32Status = am_device_init_flash(pFlash, psMSPISettings);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Disable MSPI defore re-configuring it
    //
    ui32Status = am_hal_mspi_disable(pFlash->pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    psConfig->eClockFreq = psMSPISettings->eClockFreq;

    switch(pFlash->sRegisters.CR2_00000300_b.DCn)
    {
        case AM_DEVICES_MSPI_MX25UM51245G_DC_20:
            psConfig->ui8TurnAround = 20;
            break;
        case AM_DEVICES_MSPI_MX25UM51245G_DC_18:
            psConfig->ui8TurnAround = 18;
            break;
        case AM_DEVICES_MSPI_MX25UM51245G_DC_16:
            psConfig->ui8TurnAround = 16;
            break;
        case AM_DEVICES_MSPI_MX25UM51245G_DC_14:
            psConfig->ui8TurnAround = 14;
            break;
        case AM_DEVICES_MSPI_MX25UM51245G_DC_12:
            psConfig->ui8TurnAround = 12;
            break;
        case AM_DEVICES_MSPI_MX25UM51245G_DC_10:
            psConfig->ui8TurnAround = 10;
            break;
        case AM_DEVICES_MSPI_MX25UM51245G_DC_8:
            psConfig->ui8TurnAround = 8;
            break;
        case AM_DEVICES_MSPI_MX25UM51245G_DC_6:
            psConfig->ui8TurnAround = 6;
            break;
        default:
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
    if ( pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI )
    {
        psConfig->ui8TurnAround *= 2;
    }

    //
    // Re-Configure the MSPI for the requested operation mode.
    //
    ui32Status = am_hal_mspi_device_configure(pFlash->pMspiHandle, psConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if ( (pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_SOPI && psMSPISettings->bDQSEnable) ||
         pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI )
    {
        pFlash->pDqsCfg->bDQSEnable = psMSPISettings->bDQSEnable;
        ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_DQS, pFlash->pDqsCfg);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
    }

    if ( pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI )
    {
        if (AM_HAL_MSPI_CLK_250MHZ == psConfig->eClockFreq
         || AM_HAL_MSPI_CLK_192MHZ == psConfig->eClockFreq
         || AM_HAL_MSPI_CLK_125MHZ == psConfig->eClockFreq
         || AM_HAL_MSPI_CLK_96MHZ == psConfig->eClockFreq)
        {
            pFlash->pRxCfg->ui8RxSmp = 2;
        }
    }
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_RXCFG, pFlash->pRxCfg);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    pFlash->pXipCfg->scramblingStartAddr = psMSPISettings->ui32ScramblingStartAddr;
    pFlash->pXipCfg->scramblingEndAddr = psMSPISettings->ui32ScramblingEndAddr;
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_CONFIG, pFlash->pXipCfg);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_MISC_CONFIG, pFlash->pXipMiscCfg);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if ( pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI )
    {
        ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_BIG_ENDIAN, NULL);
    }
    else
    {
        ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_LITTLE_ENDIAN, NULL);
    }
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Re-Enable MSPI
    //
    ui32Status = am_hal_mspi_enable(pFlash->pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Configure the MSPI pins.
    //
    am_bsp_mspi_pins_enable(ui32Module, psConfig->eDeviceConfig);

    //
    // Enable MSPI interrupts.
    //

    ui32Status = am_hal_mspi_interrupt_clear(pFlash->pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    ui32Status = am_hal_mspi_interrupt_enable(pFlash->pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Return the handle.
    //
    pFlash->bOccupied = true;

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Deinitialization entry of the mspi_mx25um51245g driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_deinit(void *pHandle)
{
    uint32_t      ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
#if !defined(TEST_NO_DEVICE_DEINIT)
    //
    // Device specific MSPI Flash de-initialization.
    //
    ui32Status = am_device_deinit_flash(pFlash);
    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Deinit flash failed\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
#endif
    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_reset(pHandle))
    {
        am_util_stdio_printf("Reset flash failed\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Disable and clear the interrupts to start with.
    //
    ui32Status = am_hal_mspi_interrupt_disable(pFlash->pMspiHandle, 0xFFFFFFFF);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
    ui32Status = am_hal_mspi_interrupt_clear(pFlash->pMspiHandle, 0xFFFFFFFF);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Disable the MSPI instance.
    //
    ui32Status = am_hal_mspi_disable(pFlash->pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pFlash->pMspiHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false))
    {
        am_util_stdio_printf("Error - Failed to power on MSPI.\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Deinitialize the MSPI instance.
    //
    ui32Status = am_hal_mspi_deinitialize(pFlash->pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Free this device handle
    //
    pFlash->bOccupied = false;

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reset entry of the mspi_mx25um51245g driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_reset(void *pHandle)
{
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;
    am_devices_mspi_mx25um51245g_cmd_t *pCmd;
    uint32_t ui32ResetPin;
    am_hal_gpio_pincfg_t sPinCfg;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
// #### INTERNAL BEGIN ####
// hard reset in case of debug or cpu reset
// #### INTERNAL END ####
    am_bsp_mspi_reset_pincfg_get(pFlash->ui32Module, pFlash->stSetting.eDeviceConfig, &ui32ResetPin, &sPinCfg);

    am_hal_gpio_output_set(ui32ResetPin);
    am_hal_gpio_pinconfig(ui32ResetPin, sPinCfg);
    am_hal_gpio_output_clear(ui32ResetPin);
    am_hal_delay_us(20);
    am_hal_gpio_output_set(ui32ResetPin);

    //
    // Send the command sequence to reset the device and return status.
    //
    pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_RSTEN];
    if (AM_HAL_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_command_write(pFlash, pCmd))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
    pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_RST];
    if (AM_HAL_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_command_write(pFlash, pCmd))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Delay long enough to satisfy tREADY2
    //
    am_util_delay_ms(120);

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Read device ID entry of the mspi_mx25um51245g driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_id(void *pHandle)
{
    uint32_t      ui32Status;
    uint32_t      ui32DeviceID = 0;
    uint32_t      ui32PIObuffer[2];

    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Send the command sequence to read the Device ID and return status.
    //
    am_devices_mspi_mx25um51245g_cmd_t *pCmd = &(pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_RDID]);
    pCmd->pData = ui32PIObuffer;
    if ( pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI )
    {
        pCmd->ui32NumBytes = 6;
    }
    else
    {
        pCmd->ui32NumBytes = 3;
    }
    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_enter_command_mode(pFlash))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    ui32Status = am_devices_mspi_mx25um51245g_command_read(pFlash, pCmd);

    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_exit_command_mode(pFlash))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if ( pFlash->sRegisters.CR2_00000000_b.MODE == AM_DEVICES_MSPI_MX25UM51245G_MODE_DOPI )
    {
        ui32DeviceID = (ui32PIObuffer[0] & 0x000000FF) |
                       ((ui32PIObuffer[0] & 0x00FF0000) >> 8) |
                       ((ui32PIObuffer[1] & 0x000000FF) << 16) ;
    }
    else
    {
        ui32DeviceID = ui32PIObuffer[0];
    }
    am_util_debug_printf("Flash ID is %8.8X\n", ui32DeviceID);
    if ( ((ui32DeviceID & AM_DEVICES_MSPI_MX25UM51245G_ID_MASK) == AM_DEVICES_MSPI_MX25UM51245G_ID) &&
       (AM_HAL_STATUS_SUCCESS == ui32Status) )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
    }
    else
    {
        am_util_debug_printf("Failed to read flash ID\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
}

//*****************************************************************************
//
// DMA read entry of the mspi_mx25um51245g driver for advance application.
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_read_adv(void *pHandle, uint8_t *pui8RxBuffer,
                                      uint32_t ui32ReadAddress,
                                      uint32_t ui32NumBytes,
                                      uint32_t ui32PauseCondition,
                                      uint32_t ui32StatusSetClr,
                                      am_hal_mspi_callback_t pfnCallback,
                                      void *pCallbackCtxt)
{
    uint32_t                      ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    ui32Status = am_devices_mspi_mx25um51245g_nonblocking_transfer(pFlash,
                                                                   false,
                                                                   AM_HAL_MSPI_RX,
                                                                   pui8RxBuffer,
                                                                   ui32ReadAddress,
                                                                   ui32NumBytes,
                                                                   ui32PauseCondition,
                                                                   ui32StatusSetClr,
                                                                   pfnCallback,
                                                                   pCallbackCtxt);

    //
    // Check the transaction status.
    //
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// DMA read entry of the mspi_mx25um51245g driver for common application.
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_read(void *pHandle, uint8_t *pui8RxBuffer,
                                  uint32_t ui32ReadAddress,
                                  uint32_t ui32NumBytes,
                                  bool bWaitForCompletion)
{
    uint32_t                      ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if (pHandle == NULL)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if (bWaitForCompletion)
    {
        //
        // Start the transaction.
        //
        volatile uint32_t ui32DMAStatus = 0xFFFFFFFF;
        ui32Status = am_devices_mspi_mx25um51245g_nonblocking_transfer(pFlash, false, AM_HAL_MSPI_RX, pui8RxBuffer,
                                                                       ui32ReadAddress, ui32NumBytes, 0, 0,
                                                                       pfnMSPI_MX25UM51245G_Callback, (void *)&ui32DMAStatus);

        //
        // Check the transaction status.
        //
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }

        //
        // Wait for DMA Complete or Timeout
        //
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT; i++)
        {
            if (ui32DMAStatus != 0xFFFFFFFF)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_util_delay_us(1);
        }

        //
        // Check the status.
        //
        if (ui32DMAStatus != AM_HAL_STATUS_SUCCESS)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
    }
    else
    {
        //
        // Check the transaction status.
        //
        ui32Status = am_devices_mspi_mx25um51245g_nonblocking_transfer(pFlash, false, AM_HAL_MSPI_RX, pui8RxBuffer,
                                                                       ui32ReadAddress, ui32NumBytes, 0, 0,
                                                                       NULL, NULL);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// High priority DMA read entry of the mspi_mx25um51245g driver for common application.
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_read_hiprio(void *pHandle, uint8_t *pui8RxBuffer,
                                         uint32_t ui32ReadAddress,
                                         uint32_t ui32NumBytes,
                                         bool bWaitForCompletion)
{
    uint32_t                      ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if (bWaitForCompletion)
    {
        //
        // Start the transaction.
        //
        volatile uint32_t ui32DMAStatus = 0xFFFFFFFF;

        ui32Status = am_devices_mspi_mx25um51245g_nonblocking_transfer(pFlash,
                                                                       true,
                                                                       AM_HAL_MSPI_RX,
                                                                       pui8RxBuffer,
                                                                       ui32ReadAddress,
                                                                       ui32NumBytes,
                                                                       0,
                                                                       0,
                                                                       pfnMSPI_MX25UM51245G_Callback,
                                                                       (void*)&ui32DMAStatus);

        //
        // Check the transaction status.
        //
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }

        //
        // Wait for DMA Complete or Timeout
        //
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT; i++)
        {
            if (ui32DMAStatus != 0xFFFFFFFF)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_util_delay_us(1);
        }
        //
        // Check the status.
        //
        if (ui32DMAStatus != AM_HAL_STATUS_SUCCESS)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
    }
    else
    {
        //
        // Check the transaction status.
        //
        ui32Status = am_devices_mspi_mx25um51245g_nonblocking_transfer(pFlash,
                                                                true,
                                                                AM_HAL_MSPI_RX,
                                                                pui8RxBuffer,
                                                                ui32ReadAddress,
                                                                ui32NumBytes,
                                                                0,
                                                                0,
                                                                NULL,
                                                                NULL);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// DMA write entry of the mspi_mx25um51245g driver for common application.
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_write(void *pHandle, uint8_t *pui8TxBuffer,
                                   uint32_t ui32WriteAddress,
                                   uint32_t ui32NumBytes,
                                   bool bWaitForCompletion)
{
    uint32_t                      ui32Status;
    uint32_t                      ui32BytesLeft = ui32NumBytes;
    uint32_t                      ui32TransferCount = 0;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if ( ui32WriteAddress & 0x000000FF )
    {
        am_util_debug_printf("  Error - Must ensure the A7~A0 of write address is 0\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    am_devices_mspi_mx25um51245g_cmd_t *pCmdWREN = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WREN];
    am_devices_mspi_mx25um51245g_cmd_t *pCmdWRDI = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WRDI];

    while (ui32BytesLeft > 0)
    {
        //
        // Send the command sequence to enable writing.
        //
        ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmdWREN);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }

        if (ui32BytesLeft > AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE)
        {
            //
            // Set the transfer count in bytes.
            //
            ui32TransferCount = AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE;
            ui32BytesLeft -= AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE;
        }
        else
        {
            //
            // Set the transfer count in bytes.
            //
            ui32TransferCount = ui32BytesLeft;
            ui32BytesLeft = 0;
        }

        volatile uint32_t ui32DMAStatus = 0xFFFFFFFF;

        ui32Status = am_devices_mspi_mx25um51245g_nonblocking_transfer(pFlash,
                                                                false,
                                                                AM_HAL_MSPI_TX,
                                                                pui8TxBuffer,
                                                                ui32WriteAddress,
                                                                ui32TransferCount,
                                                                0,
                                                                0,
                                                                pfnMSPI_MX25UM51245G_Callback,
                                                                (void *)&ui32DMAStatus);
        //
        // Check the transaction status.
        //
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }

        ui32WriteAddress += AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE;
        pui8TxBuffer += AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE;

        //
        // Wait for DMA Complete or Timeout
        //
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT; i++)
        {
            if (ui32DMAStatus != 0xFFFFFFFF)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_hal_delay_us(1);
        }

        //
        // Check the status.
        //
        if (ui32DMAStatus != AM_HAL_STATUS_SUCCESS)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }

        if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_enter_command_mode(pFlash))
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
        //
        // Wait for the Write In Progress to indicate completion.
        //
        bool bWriteComplete = false;
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT; i++)
        {

            ui32Status = am_devices_mspi_mx25um51245g_status(pFlash);
            if ( AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != ui32Status )
            {
                am_util_debug_printf("Read status failed\n");
                return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
            }
            if ( !pFlash->sRegisters.SR_b.WIP )
            {
                bWriteComplete = true;
                break;
            }
            am_hal_delay_us(150);
        }

        if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_exit_command_mode(pFlash))
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }

        if ( !bWriteComplete )
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }

        //
        // Send the command sequence to disable writing.
        //
        ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmdWRDI);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
    }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Chip erase entry of the mspi_mx25um51245g driver
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_chip_erase(void *pHandle)
{
    uint32_t      ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Send the command sequence to enable writing.
    //
    am_devices_mspi_mx25um51245g_cmd_t *pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WREN];
    ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Send the command sequence to do the mass erase.
    //
    pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_CE];
    ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_enter_command_mode(pFlash))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Wait for the Write In Progress to indicate completion.
    //
    bool bEraseComplete = false;
    for (uint32_t i = 0; i < AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT; i++)
    {
        ui32Status = am_devices_mspi_mx25um51245g_status(pFlash);
        if ( AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != ui32Status )
        {
            am_util_debug_printf("Read status failed\n");
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
        if ( !pFlash->sRegisters.SR_b.WIP )
        {
            bEraseComplete = true;
            break;
        }
        am_hal_delay_us(150);
    }

    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_exit_command_mode(pFlash))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Check the status.
    //
    if (!bEraseComplete)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Send the command sequence to disable writing.
    //
    pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WRDI];
    ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Sector erase entry of the mspi_mx25um51245g driver
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_sector_erase(void *pHandle, uint32_t ui32SectorAddress)
{
    uint32_t      ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Send the command sequence to enable writing.
    //
    am_devices_mspi_mx25um51245g_cmd_t *pCmd = &(pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WREN]);
    ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Send the command sequence to do the sector erase.
    //
    pCmd = &(pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_SE]);
    pCmd->ui32Addr = ui32SectorAddress;
    ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_enter_command_mode(pFlash))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
    //
    // Wait for the Write In Progress to indicate completion.
    //
    bool bEraseComplete = false;
    for (uint32_t i = 0; i < AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT; i++)
    {
        ui32Status = am_devices_mspi_mx25um51245g_status(pFlash);
        if ( AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != ui32Status )
        {
            am_util_debug_printf("Read status failed\n");
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
        if ( !pFlash->sRegisters.SR_b.WIP )
        {
            bEraseComplete = true;
            break;
        }
        am_hal_delay_us(150);
    }

    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_exit_command_mode(pFlash))
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Check the status.
    //
    if (!bEraseComplete)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Send the command sequence to disable writing.
    //
    pCmd = &(pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WRDI]);
    ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// MSPI XIP enable entry of the mspi_mx25um51245g driver
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_enable_xip(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Enable XIP on the MSPI.
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_EN, NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

#if !MSPI_USE_CQ
    //
    // Disable the DMA interrupts.
    //
    ui32Status = am_hal_mspi_interrupt_disable(pFlash->pMspiHandle,
                                               AM_HAL_MSPI_INT_DMAERR |
                                               AM_HAL_MSPI_INT_DMACMP );
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
#endif

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// MSPI XIP disable entry of the mspi_mx25um51245g driver
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_disable_xip(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Disable XIP on the MSPI.
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_DIS, NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// MSPI scrambing enable entry of the mspi_mx25um51245g driver
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_enable_scrambling(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Enable scrambling on the MSPI.
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SCRAMB_EN, NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

//*****************************************************************************
//
// MSPI scrambing disable entry of the mspi_mx25um51245g driver
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_disable_scrambling(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // Disable Scrambling on the MSPI.
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SCRAMB_DIS, NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

#define FLASH_TIMING_SCAN_SIZE_BYTES                        AM_DEVICES_MSPI_MX25UM51245G_BLOCK_SIZE
#define FLASH_TIMING_SCAN_SECTOR                            32
#define FLASH_TIMING_SCAN_MIN_ACCEPTANCE_LENGTH             (6)     // there should be at least
                                                                    // this amount of consecutive
                                                                    // passing settings to be accepted.

#define SCAN_TXDQSDELAY
#define SCAN_RXDQSDELAY
//#define SCAN_TXNEG
#define SCAN_RXNEG
#define SCAN_RXCAP

static AM_SHARED_RW uint8_t ui8TxBuffer[FLASH_TIMING_SCAN_SIZE_BYTES] __attribute__((aligned(32)));
static AM_SHARED_RW uint8_t ui8RxBuffer[FLASH_TIMING_SCAN_SIZE_BYTES] __attribute__((aligned(32)));

//*****************************************************************************
//
//! @brief  Static helper: count the longest consecutive 1s in a 32bit word
//! @param  pVal
//! @return count
//
//*****************************************************************************
static uint32_t
count_consecutive_ones(uint32_t* pVal)
{
    uint32_t count = 0;
    uint32_t data = *pVal;

    while ( data )
    {
        data = (data & (data << 1));
        count++;
    }
    return count;
}

//*****************************************************************************
//
//! @brief  Static helper: find and return the mid point of the longest continuous 1s in a 32bit word
//! @param  pVal
//! @return midpoint
//
//*****************************************************************************
static uint32_t
find_mid_point(uint32_t* pVal)
{
    uint32_t pattern_len = 0;
    uint32_t max_len = 0;
    uint32_t pick_point = 0;
    bool pattern_start = false;
    uint32_t val = *pVal;
    uint8_t remainder = 0;
    bool pick_point_flag = false;

    for ( uint32_t i = 0; i < 32; i++ )
    {
        if ( val & (0x01 << i) )
        {
            pattern_start = true;
            pattern_len++;
        }
        else
        {
            if ( pattern_start == true )
            {
                pattern_start = false;
                pick_point_flag = true;
            }
        }
        if ( (i == 31) && ( pattern_start == true ) )
        {
            pick_point_flag = true;
        }

        if (pick_point_flag == true)
        {
            if ( pattern_len > max_len )
            {
                max_len = pattern_len;
                pick_point = i - 1 - pattern_len / 2;
                remainder = pattern_len % 2;
            }
            pattern_len = 0;
            pick_point_flag = false;
        }
    }

    //
    // check the passing window side
    //
// #### INTERNAL BEGIN ####
//
// HSP20-339: Try to locate and move the middle value found one bit further away from the failure window.
// In our current tests, most of the time, we see the failure window is in the middle of 1-30 range.
// Passing windows are likely to be on either end of the 1-30 range.
// Therefore, adding comparison here, if we see the middle point is below 16, it is likely that we found
// a passing window starting from 1 at the lower side, if it is true (setting 1 passed),
// we further move the middle value to the lower side by 1.
// And the same for the higher values.
//
// #### INTERNAL END ####

    if ( (pick_point < 16) && (val & 0x00000002) )
    {
        //
        // window is likely on low side
        //
        pick_point = pick_point - remainder;    // minus only when pattern length is odd
    }
    else if ( (pick_point > 15) && (val & 0x40000000) )
    {
        //
        // window is likely on high side
        //
        pick_point = pick_point + 1;
    }
    else
    {
        //
        // window is in the middle, no action
        //
    }

    return pick_point;
}

//*****************************************************************************
//
//! @brief
//! @param pattern_index
//! @param buff
//! @param len
//! @return
//
//*****************************************************************************
static int
prepare_test_pattern(uint32_t pattern_index, uint8_t* buff, uint32_t len)
{
    uint32_t *pui32TxPtr = (uint32_t*)buff;
    uint8_t  *pui8TxPtr  = (uint8_t*)buff;

    //
    // length has to be multiple of 4 bytes
    //
    if ( len % 4 )
    {
        return -1;
    }

    switch ( pattern_index )
    {
        case 0:
            //
            // 0x5555AAAA
            //
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0x5555AAAA);
            }
            break;
        case 1:
            //
            // 0xFFFF0000
            //
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0xFFFF0000);
            }
            break;
        case 2:
            //
            // walking
            //
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = 0x01 << (i % 8);
            }
            break;
        case 3:
            //
            // incremental from 1
            //
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i + 1) & 0xFF);
            }
            break;
        case 4:
            //
            // decremental from 0xff
            //
            for ( uint32_t i = 0; i < len; i++ )
            {
                // decrement starting from 0xff
                pui8TxPtr[i] = (0xff - i) & 0xFF;
            }
            break;
        default:
            //
            // incremental from 1
            //
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i ) & 0xFF);
            }
            break;

    }

    return 0;
}

//*****************************************************************************
//
//! @brief  Static helper: flash write
//! @param  pFlash
//! @param  ui32WriteAddress
//! @param  ui32NumBytes
//! @return bStatus
//
//*****************************************************************************
static bool
flash_write(am_devices_mspi_mx25um51245g_t *pFlash, uint32_t ui32WriteAddress, uint32_t ui32NumBytes)
{
    uint32_t                      ui32Status;
    uint32_t                      counter = 0;
    uint32_t                      ui32BytesLeft = ui32NumBytes;
    uint32_t                      ui32TransferCount = 0;
    uint8_t                       *pui8TxBuffer = ui8TxBuffer;

    am_devices_mspi_mx25um51245g_cmd_t *pCmdWREN = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WREN];
    am_devices_mspi_mx25um51245g_cmd_t *pCmdWRDI = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WRDI];

    while (ui32BytesLeft > 0)
    {
        if (ui32BytesLeft > AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE)
        {
            //
            // Set the transfer count in bytes.
            //
            ui32TransferCount = AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE;
            ui32BytesLeft -= AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE;
        }
        else
        {
            //
            // Set the transfer count in bytes.
            //
            ui32TransferCount = ui32BytesLeft;
            ui32BytesLeft = 0;
        }

        prepare_test_pattern((counter % 5), pui8TxBuffer, ui32TransferCount);
#if defined(AM_PART_APOLLO5_API)
        am_hal_cachectrl_range_t sRange =
        {
            .ui32StartAddr = (uint32_t)pui8TxBuffer,
            .ui32Size = ui32TransferCount,
        };
        am_hal_cachectrl_dcache_clean(&sRange);
#endif

        //
        // Send the command sequence to enable writing.
        //
        ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmdWREN);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return false;
        }

        volatile uint32_t ui32DMAStatus = 0xFFFFFFFF;

        ui32Status = am_devices_mspi_mx25um51245g_nonblocking_transfer(pFlash,
                                                                false,
                                                                AM_HAL_MSPI_TX,
                                                                pui8TxBuffer,
                                                                ui32WriteAddress,
                                                                ui32TransferCount,
                                                                0,
                                                                0,
                                                                pfnMSPI_MX25UM51245G_Callback,
                                                                (void *)&ui32DMAStatus);
        //
        // Check the transaction status.
        //
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return false;
        }

        ui32WriteAddress += AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE;
        pui8TxBuffer += AM_DEVICES_MSPI_MX25UM51245G_PAGE_SIZE;
        counter++;

        //
        // Wait for DMA Complete or Timeout
        //
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT; i++)
        {
            //
            // check DMA status without using ISR
            //
            am_hal_mspi_interrupt_status_get(pFlash->pMspiHandle, &ui32Status, false);
            am_hal_mspi_interrupt_clear(pFlash->pMspiHandle, ui32Status);
            am_hal_mspi_interrupt_service(pFlash->pMspiHandle, ui32Status);

            if (ui32DMAStatus != 0xFFFFFFFF)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_hal_delay_us(1);
        }

        //
        // Check the status.
        //
        if (ui32DMAStatus != AM_HAL_STATUS_SUCCESS)
        {
            return false;
        }

        if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_enter_command_mode(pFlash))
        {
            return false;
        }
        //
        // Wait for the Write In Progress to indicate completion.
        //
        bool bWriteComplete = false;
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT; i++)
        {

            ui32Status = am_devices_mspi_mx25um51245g_status(pFlash);
            if ( AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != ui32Status )
            {
                am_util_debug_printf("Read status failed\n");
                return false;
            }
            if ( !pFlash->sRegisters.SR_b.WIP )
            {
                bWriteComplete = true;
                break;
            }
            am_hal_delay_us(150);
        }

        if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != am_devices_mspi_mx25um51245g_exit_command_mode(pFlash))
        {
            return false;
        }

        if ( !bWriteComplete )
        {
            return false;
        }

        //
        // Send the command sequence to disable writing.
        //
        ui32Status = am_devices_mspi_mx25um51245g_command_write(pFlash, pCmdWRDI);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return false;
        }
    }

    return true;
}
//*****************************************************************************
//
//! @brief  Static helper: flash check (read)
//! @param  pFlash
//! @param  ui32ReadAddress
//! @param  ui32NumBytes
//! @return bStatus
//
//*****************************************************************************
static bool
flash_check(am_devices_mspi_mx25um51245g_t *pFlash, uint32_t ui32ReadAddress, uint32_t ui32NumBytes)
{
    uint32_t ui32Status;
    uint8_t  *pui8RxBuffer = ui8RxBuffer;
    //
    // Start the transaction.
    //
    volatile uint32_t ui32DMAStatus = 0xFFFFFFFF;
    ui32Status = am_devices_mspi_mx25um51245g_nonblocking_transfer(pFlash, false, AM_HAL_MSPI_RX, pui8RxBuffer,
                                                                    ui32ReadAddress, ui32NumBytes, 0, 0,
                                                                    pfnMSPI_MX25UM51245G_Callback, (void *)&ui32DMAStatus);
    //
    // Check the transaction status.
    //
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return false;
    }

    //
    // Wait for DMA Complete or Timeout
    //
    for (uint32_t i = 0; i < AM_DEVICES_MSPI_MX25UM51245G_TIMEOUT; i++)
    {
        // check DMA status without using ISR
        am_hal_mspi_interrupt_status_get(pFlash->pMspiHandle, &ui32Status, false);
        am_hal_mspi_interrupt_clear(pFlash->pMspiHandle, ui32Status);
        am_hal_mspi_interrupt_service(pFlash->pMspiHandle, ui32Status);

        if (ui32DMAStatus != 0xFFFFFFFF)
        {
            break;
        }
        //
        // Call the BOOTROM cycle function to delay for about 1 microsecond.
        //
        am_util_delay_us(1);
    }

    //
    // Check the status.
    //
    if (ui32DMAStatus != AM_HAL_STATUS_SUCCESS)
    {
        return false;
    }

    if ( memcmp(ui8RxBuffer, ui8TxBuffer, FLASH_TIMING_SCAN_SIZE_BYTES) !=0 )
    {
        //
        // verify failed
        //
        return false;
    }

    return true;
}

//*****************************************************************************
//
// Timing scan entry of the mspi_mx25um51245g driver
//
//*****************************************************************************
#if defined(SCAN_TXNEG)
#define SCAN_TXNEG_START 0
#define SCAN_TXNEG_END   1
#endif

#if defined(SCAN_RXNEG)
#define SCAN_RXNEG_START 0
#define SCAN_RXNEG_END   1
#endif

#if defined(SCAN_RXCAP)
#define SCAN_RXCAP_START 0
#define SCAN_RXCAP_END   1
#endif

#if defined(SCAN_TURNAROUND)
#define SCAN_TURNAROUND_START 0
#define SCAN_TURNAROUND_END   1
#endif

#if defined(SCAN_TXDQSDELAY)
#define SCAN_TXDQSDELAY_START 0
#define SCAN_TXDQSDELAY_END   7
#endif

#if defined(SCAN_RXDQSDELAY)
#define SCAN_RXDQSDELAY_START 0
#define SCAN_RXDQSDELAY_END   31
#endif

uint32_t
am_devices_mspi_mx25um51245g_init_timing_check(uint32_t ui32Module,
                                               am_devices_mspi_mx25um51245g_config_t *pDevCfg,
                                               am_devices_mspi_mx25um51245g_timing_config_t *pDevTimingCfg)
{
    uint32_t ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash;
    void *pHandle;
    uint32_t Txdqsdelay = 0;
    uint32_t Rxdqsdelay = 0;

    uint32_t ui32CheckAddress = FLASH_TIMING_SCAN_SECTOR * AM_DEVICES_MSPI_MX25UM51245G_SECTOR_SIZE;
    uint32_t ui32CCOResult = 0;
    uint32_t ui32TxResult = 0;
    uint32_t ui32RxResultArray[32];

    am_hal_mspi_timing_scan_t scanCfg;

    //
    // initialize interface
    //
    ui32Status = am_devices_mspi_mx25um51245g_init(ui32Module, pDevCfg, (void *)&pFlash, &pHandle);
    if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != ui32Status)
    {
        am_util_debug_printf("    Failed to configure the MSPI and Flash Device correctly!\n");
        return ui32Status;
    }

    //
    //get timing scan param configured by init & hal
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_TIMING_SCAN_GET, &scanCfg);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    bool bTimingValid = pFlash->bTimingValid;
    pFlash->bTimingValid = false;

    //
    // erase target sector first (each "sector is 4Kbyte block")
    //
    if ( FLASH_TIMING_SCAN_SIZE_BYTES % AM_DEVICES_MSPI_MX25UM51245G_SECTOR_SIZE )
    {
        // scan size shall be at block boundary
        am_util_debug_printf("ERROR: Timing scan data size shall be at sector boundary!\n");
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    for ( uint8_t i = 0; i < (FLASH_TIMING_SCAN_SIZE_BYTES / AM_DEVICES_MSPI_MX25UM51245G_SECTOR_SIZE); i++ )
    {
        ui32Status = am_devices_mspi_mx25um51245g_sector_erase(pFlash, (FLASH_TIMING_SCAN_SECTOR + i) * AM_DEVICES_MSPI_MX25UM51245G_SECTOR_SIZE);

        if (AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS != ui32Status)
        {
            am_util_debug_printf("Failed to erase Flash Device sector!\n");
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }
    }

    //
    // write test pattern into target sector
    //
    if ( !flash_write(pFlash, ui32CheckAddress, FLASH_TIMING_SCAN_SIZE_BYTES) )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

#if defined(FAST_TIMING_SCAN)
    if ( bTimingValid )
    {
        //
        // apply settings
        //
        ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_TIMING_SCAN, pFlash->pTimingCfg);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
        }

        //
        // run data check
        //
        if ( flash_check(pFlash, ui32CheckAddress, FLASH_TIMING_SCAN_SIZE_BYTES) )
        {
            pDevTimingCfg->bTxNeg          = pFlash->pTimingCfg->bTxNeg;
            pDevTimingCfg->bRxNeg          = pFlash->pTimingCfg->bRxNeg;
            pDevTimingCfg->bRxCap          = pFlash->pTimingCfg->bRxCap;
            pDevTimingCfg->ui8Turnaround   = pFlash->pTimingCfg->ui8Turnaround;
            pDevTimingCfg->ui8TxDQSDelay   = pFlash->pTimingCfg->ui8TxDQSDelay;
            pDevTimingCfg->ui8RxDQSDelay   = pFlash->pTimingCfg->ui8RxDQSDelay;
            //
            // Deinitialize the MSPI interface
            //
            am_devices_mspi_mx25um51245g_deinit(pFlash);
            am_util_debug_printf("Skipping timing scan.\n");
            return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
        }
    }
#endif

    am_util_debug_printf("\nStart MSPI Timing Scan!\n");

    uint32_t ui32TxNeg = scanCfg.bTxNeg;
    uint32_t ui32RxNeg = scanCfg.bRxNeg;
    uint32_t ui32RxCap = scanCfg.bRxCap;
    uint32_t ui32Turnaround = scanCfg.ui8Turnaround;
#if defined(SCAN_TXNEG)
    for ( ui32TxNeg = SCAN_TXNEG_START; ui32TxNeg <= SCAN_TXNEG_END; ui32TxNeg++ )
#endif
    {
        scanCfg.bTxNeg = (bool)ui32TxNeg;
#if defined(SCAN_RXNEG)
        for ( ui32RxNeg = SCAN_RXNEG_START; ui32RxNeg <= SCAN_RXNEG_END; ui32RxNeg++ )
#endif
        {
            scanCfg.bRxNeg = (bool)ui32RxNeg;
#if defined(SCAN_RXCAP)
            for ( ui32RxCap = SCAN_RXCAP_START; ui32RxCap <= SCAN_RXCAP_END; ui32RxCap++ )
#endif
            {
                scanCfg.bRxCap = (bool)ui32RxCap;
#if defined(SCAN_TURNAROUND)
                for ( scanCfg.ui8Turnaround = ui32Turnaround + SCAN_TURNAROUND_START; scanCfg.ui8Turnaround <= ui32Turnaround + SCAN_TURNAROUND_END; scanCfg.ui8Turnaround++ )
#endif
                {
                    am_util_debug_printf("    TxNeg=%d, RxNeg=%d, RxCap=%d, Turnaround=%d\n", scanCfg.bTxNeg, scanCfg.bRxNeg, scanCfg.bRxCap, scanCfg.ui8Turnaround)
                    ui32TxResult = 0;
                    memset(ui32RxResultArray, 0, sizeof(ui32RxResultArray));
#if defined(SCAN_TXDQSDELAY)
                    for (scanCfg.ui8TxDQSDelay = SCAN_TXDQSDELAY_START; scanCfg.ui8TxDQSDelay <= SCAN_TXDQSDELAY_END; scanCfg.ui8TxDQSDelay++)
#endif
                    {
#if defined(SCAN_RXDQSDELAY)
                        for (scanCfg.ui8RxDQSDelay = SCAN_RXDQSDELAY_START; scanCfg.ui8RxDQSDelay <= SCAN_RXDQSDELAY_END; scanCfg.ui8RxDQSDelay++)
#endif
                        {
                            //
                            // apply settings
                            //
                            ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_TIMING_SCAN, &scanCfg);
                            if (AM_HAL_STATUS_SUCCESS != ui32Status)
                            {
                                return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
                            }

                            //
                            // run data check
                            //
                            if ( flash_check(pFlash, ui32CheckAddress, FLASH_TIMING_SCAN_SIZE_BYTES) )
                            {
                                //
                                // data check pass
                                //
                                ui32RxResultArray[scanCfg.ui8TxDQSDelay] |= 0x01 << scanCfg.ui8RxDQSDelay;
                            }
                            else
                            {
                                //
                                // data check failed
                                //
                            }
                        }
#if defined(SCAN_RXDQSDELAY)
                        ui32CCOResult = count_consecutive_ones(&ui32RxResultArray[scanCfg.ui8TxDQSDelay]);
                        if ( ui32CCOResult > FLASH_TIMING_SCAN_MIN_ACCEPTANCE_LENGTH )
                        {
                            ui32TxResult |= 0x01 << scanCfg.ui8TxDQSDelay;
                        }
                        am_util_debug_printf("    TxDQSDelay: %d, RxDQSDelay Scan = 0x%08X, Window Size = %d\n", scanCfg.ui8TxDQSDelay, ui32RxResultArray[scanCfg.ui8TxDQSDelay], ui32CCOResult);
#else
                        if ( ui32RxResultArray[scanCfg.ui8TxDQSDelay] != 0 )
                        {
                            ui32TxResult |= 0x01 << scanCfg.ui8TxDQSDelay;
                        }
                        am_util_debug_printf("    TxDQSDelay: %d, RxDQSDelay Scan = 0x%08X\n", scanCfg.ui8TxDQSDelay, ui32RxResultArray[scanCfg.ui8TxDQSDelay]);
#endif
                    }
                    //
                    // Check Result
                    //
                    if ( ui32TxResult == 0 )
                    {
                        //
                        // no window is found
                        //
#if defined(SCAN_TXNEG) || defined(SCAN_RXNEG) || defined(SCAN_RXCAP) || defined(SCAN_TURNAROUND)
                        continue;
#else
                        am_util_debug_printf("Timing Scan found no window!\n");
                        //
                        // Deinitialize the MSPI interface
                        //
                        am_devices_mspi_mx25um51245g_deinit(pFlash);
                        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
#endif
                    }
#if defined(SCAN_TXDQSDELAY)
                    //
                    // Find TXDQSDELAY Value
                    //
                    Txdqsdelay = find_mid_point(&ui32TxResult);
#else
                    Txdqsdelay = scanCfg.ui8TxDQSDelay;
#endif

#if defined(SCAN_RXDQSDELAY)
                    //
                    // Find RXDQSDELAY Value
                    //
                    Rxdqsdelay = find_mid_point(&ui32RxResultArray[Txdqsdelay]);
#else
                    Rxdqsdelay = scanCfg.ui8RxDQSDelay;
#endif

                    am_util_debug_printf("Selected timing scan setting: TxNeg=%d, RxNeg=%d, RxCap=%d, Turnaround=%d, TxDQSDelay=%d, RxDQSDelay=%d\n", scanCfg.bTxNeg, scanCfg.bRxNeg, scanCfg.bRxCap, scanCfg.ui8Turnaround, Txdqsdelay, Rxdqsdelay);

                    scanCfg.ui8TxDQSDelay = Txdqsdelay;
                    scanCfg.ui8RxDQSDelay = Rxdqsdelay;

                    *pFlash->pTimingCfg = scanCfg;
                    //
                    // Set output values
                    //
                    pDevTimingCfg->bTxNeg          = pFlash->pTimingCfg->bTxNeg;
                    pDevTimingCfg->bRxNeg          = pFlash->pTimingCfg->bRxNeg;
                    pDevTimingCfg->bRxCap          = pFlash->pTimingCfg->bRxCap;
                    pDevTimingCfg->ui8Turnaround   = pFlash->pTimingCfg->ui8Turnaround;
                    pDevTimingCfg->ui8TxDQSDelay   = pFlash->pTimingCfg->ui8TxDQSDelay;
                    pDevTimingCfg->ui8RxDQSDelay   = pFlash->pTimingCfg->ui8RxDQSDelay;

                    am_devices_mspi_mx25um51245g_deinit(pFlash);
                    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
                }
            }
        }
    }
    am_util_debug_printf("Timing Scan found no window!\n");
    //
    // Deinitialize the MSPI interface
    //
    am_devices_mspi_mx25um51245g_deinit(pFlash);
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
}

//*****************************************************************************
//
// Apply given SDR timing settings entry of the mspi_mx25um51245g driver
//
//*****************************************************************************
uint32_t
am_devices_mspi_mx25um51245g_apply_timing(void *pHandle,
                                          am_devices_mspi_mx25um51245g_timing_config_t *pDevTimingCfg)
{
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    //
    // apply timing setting
    //
    pFlash->pTimingCfg->bTxNeg                = pDevTimingCfg->bTxNeg;
    pFlash->pTimingCfg->bRxNeg                = pDevTimingCfg->bRxNeg;
    pFlash->pTimingCfg->bRxCap                = pDevTimingCfg->bRxCap;
    pFlash->pTimingCfg->ui8TxDQSDelay         = pDevTimingCfg->ui8TxDQSDelay;
    pFlash->pTimingCfg->ui8RxDQSDelay         = pDevTimingCfg->ui8RxDQSDelay;
    pFlash->pTimingCfg->ui8Turnaround         = pDevTimingCfg->ui8Turnaround;
    pFlash->bTimingValid = true;

    am_util_debug_printf("Following MSPI timing setting is applied.\n");
    am_util_debug_printf("TxNeg       = %d\n",   pFlash->pTimingCfg->bTxNeg);
    am_util_debug_printf("RxNeg       = %d\n",   pFlash->pTimingCfg->bRxNeg);
    am_util_debug_printf("RxCap       = %d\n",   pFlash->pTimingCfg->bRxCap);
    am_util_debug_printf("Turnaround  = %d\n",   pFlash->pTimingCfg->ui8Turnaround);
    am_util_debug_printf("TxDQSDelay  = %d\n",   pFlash->pTimingCfg->ui8TxDQSDelay);
    am_util_debug_printf("RxDQSDelay  = %d\n\n", pFlash->pTimingCfg->ui8RxDQSDelay);

    return am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_TIMING_SCAN, pFlash->pTimingCfg);

}

// ##### INTERNAL BEGIN #####
uint32_t
am_devices_mspi_mx25um51245g_set_default_mode(void *pHandle, am_devices_mspi_mx25um51245g_DEFMODE_e eDefMode)
{
    uint32_t ui32Status;
    uint32_t ui32PIObuffer;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;
    am_devices_mspi_mx25um51245g_cmd_t *pCmd;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_RDCR2];
    pCmd->pData = &ui32PIObuffer;
    pCmd->ui32Addr = 0x40000000;
    ui32Status = am_devices_mspi_mx25um51245g_command_read(pFlash, pCmd);
    am_util_debug_printf("CR2 DefMode %X\n", ui32PIObuffer);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    pFlash->sRegisters.CR2_40000000 = ui32PIObuffer;
    pFlash->sRegisters.CR2_40000000_b.DEFMODE = eDefMode;
    ui32PIObuffer = pFlash->sRegisters.CR2_40000000;

    pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_WRCR2];
    pCmd->pData = &ui32PIObuffer;
    pCmd->ui32Addr = 0x40000000;
    ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

uint32_t
am_devices_mspi_mx25um51245g_set_burst_length(void *pHandle, am_devices_mspi_mx25um51245g_burst_len_e eBurstLen)
{
    uint32_t ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;
    uint32_t ui32PIObuffer = eBurstLen;
    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    am_devices_mspi_mx25um51245g_cmd_t *pCmd = &pFlash->pCmdList[AM_DEVICES_MSPI_MX25UM51245G_CMD_SBL];
    pCmd->pData = &ui32PIObuffer;
    ui32Status = am_devices_mspi_mx25um51245g_write_register(pFlash, pCmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}

uint32_t
am_devices_mspi_mx25um51245g_read_SFDP(void *pHandle, uint8_t *pData, uint32_t ui32NumBytes)
{
    uint32_t ui32Status;
    am_devices_mspi_mx25um51245g_t *pFlash = (am_devices_mspi_mx25um51245g_t *)pHandle;

    if ( pHandle == NULL )
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }
    //SPI param
    am_devices_mspi_mx25um51245g_cmd_t Cmd =
    {
        .ui16Cmd = AM_DEVICES_MSPI_MX25UM51245G_SPI_RDSFDP,
        .bSendAddr = true,
        .ui32Addr = 0x0,
        .pData = (uint32_t *)pData,
        .ui32NumBytes = ui32NumBytes,
        .bTurnaround = true,
        .ui8Turnaround = 8,
    };

    ui32Status = am_devices_mspi_mx25um51245g_command_read(pFlash, &Cmd);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_MX25UM51245G_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_MX25UM51245G_STATUS_SUCCESS;
}
// ##### INTERNAL END #####
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
