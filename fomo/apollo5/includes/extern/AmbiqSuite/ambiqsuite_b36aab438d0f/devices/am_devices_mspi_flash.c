//*****************************************************************************
//
//! @file am_devices_mspi_flash.c
//!
//! @brief General Multibit SPI Flash driver.
//!
//! @addtogroup mspi_flash Generic MSPI FLASH Driver
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
#include "am_devices_mspi_flash.h"
#include "am_util_stdio.h"
#include "am_bsp.h"
#include "am_util_delay.h"

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
#define AM_DEVICES_MSPI_FLASH_TIMEOUT             1000000
#define AM_DEVICES_MSPI_FLASH_ERASE_TIMEOUT       1000000

//
//!
//
typedef struct
{
    uint32_t                    ui32Module;
    void                        *pMspiHandle;
    am_hal_mspi_dev_config_t    stSetting;
    bool                        bOccupied;
} am_devices_mspi_flash_t;

//
//!
//
am_devices_mspi_flash_t gAmFlash[AM_DEVICES_MSPI_FLASH_MAX_DEVICE_NUM];

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Serial_CE0_MSPIConfig =
{
#if defined(MICRON_N25Q256A)
  .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (MACRONIX_MX25U12835F)
  .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (ADESTO_ATXP032)
  .ui8TurnAround        = 8,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
#endif
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .bSendAddr            = true,
    .bSendInstr           = true,
    .bSeparateIO          = true,
    .bTurnaround          = true,
#if !defined(AM_PART_APOLLO3)
    // #### INTERNAL BEGIN ####
    // TODO - Fixme.  What is the proper value? (ui8WriteLatency = 0?)
    // TODO - Fixme.  What is the proper value? (bEnWriteLatency = false?)
    // TODO - Fixme.  What is the proper value? (.ui16DMATimeLimit = 0?)
    // #### INTERNAL END ####
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#endif
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Serial_CE1_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
  .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (MACRONIX_MX25U12835F)
  .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (ADESTO_ATXP032)
  .ui8TurnAround        = 8,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
#endif
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE1,
    .bSeparateIO          = true,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Dual_CE0_MSPIConfig =
{
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
  .ui8TurnAround        = 3,
#elif defined (CYPRESS_S25FS064S)
  .ui8TurnAround        = 3,
#elif defined (MACRONIX_MX25U12835F)
  .ui8TurnAround        = 8,
#endif
  .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_DUAL_CE0,
  .bSeparateIO          = false,
  .bSendInstr           = true,
  .bSendAddr            = true,
  .bTurnaround          = true,
  .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
  .ui32TCBSize          = 0,
  .pTCB                 = NULL,
  .scramblingStartAddr  = 0,
  .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Dual_CE1_MSPIConfig =
{
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
  .ui8TurnAround        = 3,
#elif defined (CYPRESS_S25FS064S)
  .ui8TurnAround        = 3,
#elif defined (MACRONIX_MX25U12835F)
  .ui8TurnAround        = 8,
#endif
  .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_DUAL_CE1,
  .bSeparateIO          = false,
  .bSendInstr           = true,
  .bSendAddr            = true,
  .bTurnaround          = true,
  .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
  .ui32TCBSize          = 0,
  .pTCB                 = NULL,
  .scramblingStartAddr  = 0,
  .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Quad_CE0_MSPIConfig =
{
#if defined(MICRON_N25Q256A)
  .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (MACRONIX_MX25U12835F)
  .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (ADESTO_ATXP032)
  .ui8TurnAround        = 8,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
#endif
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_QUAD_CE0,
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .bSendAddr            = true,
    .bSendInstr           = true,
    .bSeparateIO          = false,
    .bTurnaround          = true,
#if !defined(AM_PART_APOLLO3)
    // #### INTERNAL BEGIN ####
    // TODO - Fixme.  What is the proper value? (ui8WriteLatency = 0?)
    // TODO - Fixme.  What is the proper value? (bEnWriteLatency = false?)
    // TODO - Fixme.  What is the proper value? (ui16DMATimeLimit = 0?)
    // #### INTERNAL END ####
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#endif
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Quad_CE1_MSPIConfig =
{
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
  .ui8TurnAround        = 3,
#elif defined (CYPRESS_S25FS064S)
  .ui8TurnAround        = 5,
#elif defined (MACRONIX_MX25U12835F)
  .ui8TurnAround        = 4,
// #### INTERNAL BEGIN ####
// TODO - Flag an error if another part.
// #### INTERNAL END ####
#endif
  .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_QUAD_CE1,
  .bSeparateIO          = false,
  .bSendInstr           = true,
  .bSendAddr            = true,
  .bTurnaround          = true,
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F))
  .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
#elif defined (CYPRESS_S25FS064S)
  .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_QUAD_IO_READ,
// #### INTERNAL BEGIN ####
// TODO - Flag an error if another part.
// #### INTERNAL END ####
#endif
  .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
  .ui32TCBSize          = 0,
  .pTCB                 = NULL,
  .scramblingStartAddr  = 0,
  .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Octal_CE0_MSPIConfig =
{
  .ui8TurnAround        = 8,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_OCTAL_CE0,
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
  .bSendAddr            = true,
  .bSendInstr           = true,
  .bSeparateIO          = false,
  .bTurnaround          = true,
#if !defined(AM_PART_APOLLO3)
    // #### INTERNAL BEGIN ####
    // TODO - Fixme.  What is the proper value? (ui8WriteLatency = 0?)
    // TODO - Fixme.  What is the proper value? (bEnWriteLatency = false?)
    // TODO - Fixme.  What is the proper value? (ui16DMATimeLimit = 0?)
    // #### INTERNAL END ####
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#endif
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Octal_CE1_MSPIConfig =
{
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
  .ui8TurnAround        = 8,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_OCTAL_CE1,
  .bSeparateIO          = false,
  .bSendInstr           = true,
  .bSendAddr            = true,
  .bTurnaround          = true,
  .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
  .ui32TCBSize          = 0,
  .pTCB                 = NULL,
  .scramblingStartAddr  = 0,
  .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t  MSPI_Flash_QuadPairedSerialMSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL,
    .bSeparateIO          = true,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_QuadPairedMSPIConfig =
{
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
  .ui8TurnAround        = 3,
#elif defined (CYPRESS_S25FS064S)
  .ui8TurnAround        = 5,
#elif defined (MACRONIX_MX25U12835F)
  .ui8TurnAround        = 6,
#endif
  .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_QUADPAIRED,
  .bSeparateIO          = false,
  .bSendInstr           = true,
  .bSendAddr            = true,
  .bTurnaround          = true,
  .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
  .ui32TCBSize          = 0,
  .pTCB                 = NULL,
  .scramblingStartAddr  = 0,
  .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Dual_1_2_2_CE0_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
    .ui8TurnAround        = 3,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 4,
#elif defined (MACRONIX_MX25U12835F)
    .ui8TurnAround        = 8,
#endif
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE0, //should be serial mode to enable XIPMIXED
    .eXipMixedMode        = AM_HAL_MSPI_XIPMIXED_AD2,
    .bSeparateIO          = false,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F))
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
#elif defined (CYPRESS_S25FS064S)
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_DUAL_IO_READ,
// #### INTERNAL BEGIN ####
// TODO - Flag an error if another part.
// #### INTERNAL END ####
#endif
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Dual_1_1_2_CE0_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
    .ui8TurnAround        = 3,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 4,
#elif defined (MACRONIX_MX25U12835F)
    .ui8TurnAround        = 8,
#endif
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE0, //should be serial mode to enable XIPMIXED
    .eXipMixedMode        = AM_HAL_MSPI_XIPMIXED_D2,
    .bSeparateIO          = false,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F))
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
#elif defined (CYPRESS_S25FS064S)
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_DUAL_READ,
// #### INTERNAL BEGIN ####
// TODO - Flag an error if another part.
// #### INTERNAL END ####
#endif
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Quad_1_1_4_CE0_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
    .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 4,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (MACRONIX_MX25U12835F)
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (ADESTO_ATXP032)
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
#endif
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE0, //should be serial mode to enable XIPMIXED
    .eXipMixedMode        = AM_HAL_MSPI_XIPMIXED_D4,
    .bSeparateIO          = false,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F) || defined(ADESTO_ATXP032))
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
#elif defined (CYPRESS_S25FS064S)
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_QUAD_IO_READ,
// #### INTERNAL BEGIN ####
// TODO - Flag an error if another part.
// #### INTERNAL END ####
#endif
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Quad_1_4_4_CE0_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
    .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 4,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (MACRONIX_MX25U12835F)
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (ADESTO_ATXP032)
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
#endif
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE0, //should be serial mode to enable XIPMIXED
    .eXipMixedMode        = AM_HAL_MSPI_XIPMIXED_AD4,
    .bSeparateIO          = false,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F) || defined(ADESTO_ATXP032))
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
#elif defined (CYPRESS_S25FS064S)
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_QUAD_READ,
// #### INTERNAL BEGIN ####
// TODO - Flag an error if another part.
// #### INTERNAL END ####
#endif
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Dual_1_2_2_CE1_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
    .ui8TurnAround        = 3,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 4,
#elif defined (MACRONIX_MX25U12835F)
    .ui8TurnAround        = 8,
#endif
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE1, //should be serial mode to enable XIPMIXED
    .eXipMixedMode        = AM_HAL_MSPI_XIPMIXED_AD2,
    .bSeparateIO          = false,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F))
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
#elif defined (CYPRESS_S25FS064S)
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_DUAL_IO_READ,
// #### INTERNAL BEGIN ####
// TODO - Flag an error if another part.
// #### INTERNAL END ####
#endif
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Dual_1_1_2_CE1_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
    .ui8TurnAround        = 3,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 4,
#elif defined (MACRONIX_MX25U12835F)
    .ui8TurnAround        = 8,
#endif
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE1, //should be serial mode to enable XIPMIXED
    .eXipMixedMode        = AM_HAL_MSPI_XIPMIXED_D2,
    .bSeparateIO          = false,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F))
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
#elif defined (CYPRESS_S25FS064S)
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_DUAL_READ,
// #### INTERNAL BEGIN ####
// TODO - Flag an error if another part.
// #### INTERNAL END ####
#endif
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Quad_1_1_4_CE1_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
    .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 4,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (MACRONIX_MX25U12835F)
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (ADESTO_ATXP032)
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
#endif
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE1, //should be serial mode to enable XIPMIXED
    .eXipMixedMode        = AM_HAL_MSPI_XIPMIXED_D4,
    .bSeparateIO          = false,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F) || defined(ADESTO_ATXP032))
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
#elif defined (CYPRESS_S25FS064S)
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_QUAD_IO_READ,
// #### INTERNAL BEGIN ####
// TODO - Flag an error if another part.
// #### INTERNAL END ####
#endif
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//!
//
am_hal_mspi_dev_config_t MSPI_Flash_Quad_1_4_4_CE1_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
#if defined(MICRON_N25Q256A)
    .ui8TurnAround        = 3,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (CYPRESS_S25FS064S)
    .ui8TurnAround        = 4,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (MACRONIX_MX25U12835F)
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_3_BYTE,
#elif defined (ADESTO_ATXP032)
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
#endif
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE1, //should be serial mode to enable XIPMIXED
    .eXipMixedMode        = AM_HAL_MSPI_XIPMIXED_AD4,
    .bSeparateIO          = false,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
#if (defined(MICRON_N25Q256A) || defined(MACRONIX_MX25U12835F) || defined(ADESTO_ATXP032))
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_FAST_READ,
#elif defined (CYPRESS_S25FS064S)
    .ui8ReadInstr         = AM_DEVICES_MSPI_FLASH_QUAD_READ,
// #### INTERNAL BEGIN ####
// TODO - Flag an error if another part.
// #### INTERNAL END ####
#endif
    .ui8WriteInstr        = AM_DEVICES_MSPI_FLASH_PAGE_PROGRAM,
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
};

//
//! MSPI interrupts.
//
static const IRQn_Type mspi_flash_interrupts[] =
{
    MSPI0_IRQn,
#if defined(AM_PART_APOLLO3P)
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

//
//!
//
struct
{
    am_hal_mspi_device_e eHalDeviceEnum;
    am_hal_mspi_xipmixed_mode_e eMixedModeEnum;
    am_hal_mspi_dev_config_t *psDevConfig;
}g_MSPIFlash_DevConfig[] =
{
    {AM_HAL_MSPI_FLASH_SERIAL_CE0,         AM_HAL_MSPI_XIPMIXED_NORMAL,           &MSPI_Flash_Serial_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_SERIAL_CE1,         AM_HAL_MSPI_XIPMIXED_NORMAL,           &MSPI_Flash_Serial_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_DUAL_CE0,           AM_HAL_MSPI_XIPMIXED_NORMAL,           &MSPI_Flash_Dual_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_DUAL_CE1,           AM_HAL_MSPI_XIPMIXED_NORMAL,           &MSPI_Flash_Dual_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_QUAD_CE0,           AM_HAL_MSPI_XIPMIXED_NORMAL,           &MSPI_Flash_Quad_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_QUAD_CE1,           AM_HAL_MSPI_XIPMIXED_NORMAL,           &MSPI_Flash_Quad_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_OCTAL_CE0,          AM_HAL_MSPI_XIPMIXED_NORMAL,           &MSPI_Flash_Octal_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_OCTAL_CE1,          AM_HAL_MSPI_XIPMIXED_NORMAL,           &MSPI_Flash_Octal_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_QUADPAIRED,         AM_HAL_MSPI_XIPMIXED_NORMAL,           &MSPI_Flash_QuadPairedMSPIConfig},
    {AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL,  AM_HAL_MSPI_XIPMIXED_NORMAL,           &MSPI_Flash_QuadPairedSerialMSPIConfig},
    {AM_HAL_MSPI_FLASH_SERIAL_CE0,         AM_HAL_MSPI_XIPMIXED_D2,               &MSPI_Flash_Dual_1_1_2_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_SERIAL_CE1,         AM_HAL_MSPI_XIPMIXED_D2,               &MSPI_Flash_Dual_1_1_2_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_SERIAL_CE0,         AM_HAL_MSPI_XIPMIXED_AD2,              &MSPI_Flash_Dual_1_2_2_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_SERIAL_CE1,         AM_HAL_MSPI_XIPMIXED_AD2,              &MSPI_Flash_Dual_1_2_2_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_SERIAL_CE0,         AM_HAL_MSPI_XIPMIXED_D4,               &MSPI_Flash_Quad_1_1_4_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_SERIAL_CE1,         AM_HAL_MSPI_XIPMIXED_D4,               &MSPI_Flash_Quad_1_1_4_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_SERIAL_CE0,         AM_HAL_MSPI_XIPMIXED_AD4,              &MSPI_Flash_Quad_1_4_4_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_SERIAL_CE1,         AM_HAL_MSPI_XIPMIXED_AD4,              &MSPI_Flash_Quad_1_4_4_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_DUAL_CE0,           AM_HAL_MSPI_XIPMIXED_D2,               &MSPI_Flash_Dual_1_1_2_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_DUAL_CE1,           AM_HAL_MSPI_XIPMIXED_D2,               &MSPI_Flash_Dual_1_1_2_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_DUAL_CE0,           AM_HAL_MSPI_XIPMIXED_AD2,              &MSPI_Flash_Dual_1_2_2_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_DUAL_CE1,           AM_HAL_MSPI_XIPMIXED_AD2,              &MSPI_Flash_Dual_1_2_2_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_QUAD_CE0,           AM_HAL_MSPI_XIPMIXED_D4,               &MSPI_Flash_Quad_1_1_4_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_QUAD_CE1,           AM_HAL_MSPI_XIPMIXED_D4,               &MSPI_Flash_Quad_1_1_4_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_QUAD_CE0,           AM_HAL_MSPI_XIPMIXED_AD4,              &MSPI_Flash_Quad_1_4_4_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_QUAD_CE1,           AM_HAL_MSPI_XIPMIXED_AD4,              &MSPI_Flash_Quad_1_4_4_CE1_MSPIConfig},
};

//
// Forward declarations.
//
static uint32_t am_device_command_write(void *pHandle,
                                        uint8_t ui8Instr,
                                        bool bSendAddr,
                                        uint32_t ui32Addr,
                                        uint32_t *pData,
                                        uint32_t ui32NumBytes);
//
// Forward declarations.
//
static uint32_t am_device_command_read(void *pHandle,
                                       uint8_t ui8Instr,
                                       bool bSendAddr,
                                       uint32_t ui32Addr,
                                       uint32_t *pData,
                                       uint32_t ui32NumBytes);

//*****************************************************************************
//
// Micron N25Q256A Support
//
//*****************************************************************************
#if defined (MICRON_N25Q256A)
//*****************************************************************************
//
//! @brief  Device specific configuration function.
//!
//! @param pHandle
//!
//! @return uint32_t
//
//*****************************************************************************
static uint32_t am_device_init_flash(void *pHandle)
{
    uint32_t      ui32Status;
    uint32_t      ui32PIOBuffer[32] = {0};
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    //
    // Configure the turnaround waitstates, xip, and wrap based on MSPI configuration.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    ui32PIOBuffer[0] =
        AM_DEVICES_MSPI_FLASH_DUMMY_CLOCKS(pFlash->stSetting.ui8TurnAround)   |
                                           AM_DEVICES_MSPI_FLASH_XIP(1)     |
                                           AM_DEVICES_MSPI_FLASH_WRAP(3);

    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_VOL_CFG, false, 0, ui32PIOBuffer, 1);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Configure the MSPI_FLASH mode based on the MSPI configuration.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    switch (pFlash->stSetting.eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            ui32PIOBuffer[0] = AM_DEVICES_MSPI_FLASH_SERIAL_MODE;
            break;
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
            ui32PIOBuffer[0] = AM_DEVICES_MSPI_FLASH_DUAL_MODE;
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_QUADPAIRED:
        case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
            ui32PIOBuffer[0] = AM_DEVICES_MSPI_FLASH_QUAD_MODE;
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            //break;
    }

    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENHVOL_CFG, false, 0, ui32PIOBuffer, 1);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Configure the MSPI_FLASH byte addressing mode.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    switch (pFlash->stSetting.eAddrCfg)
    {
        case AM_HAL_MSPI_ADDR_1_BYTE:
        case AM_HAL_MSPI_ADDR_2_BYTE:
        case AM_HAL_MSPI_ADDR_3_BYTE:
            // Exit 4-byte mode.
            ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_EXIT_4B, false, 0, ui32PIOBuffer, 0);
            if (AM_HAL_STATUS_SUCCESS != ui32Status)
            {
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            break;
        case AM_HAL_MSPI_ADDR_4_BYTE:
            // Exit 4-byte mode.
            ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_ENTER_4B, false, 0, ui32PIOBuffer, 0);
            if (AM_HAL_STATUS_SUCCESS != ui32Status)
            {
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            break;
    }

    // Return status.
    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Device specific configuration function.
//!
//! @param pHandle
//!
//! @return uint32_t
//
//*****************************************************************************
static uint32_t am_device_deinit_flash(void *pHandle)
{
    uint32_t      ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Configure the MSPI_FLASH back to serial mode.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    ui32PIOBuffer[0] = AM_DEVICES_MSPI_FLASH_SERIAL_MODE;
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENHVOL_CFG, false, 0, ui32PIOBuffer, 1);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    else
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
    }
}

#endif

//*****************************************************************************
//
// Cypress S25FS064S Support
//
//*****************************************************************************
#if defined (CYPRESS_S25FS064S)

//*****************************************************************************
//
//! @brief  Device specific initialization function.
//!
//! @param pHandle
//!
//! @return uint32_t
//
//*****************************************************************************
static uint32_t am_device_init_flash(void *pHandle)
{
    uint32_t      ui32Status = AM_HAL_STATUS_SUCCESS;
    uint8_t       ui8Value;
    uint32_t      ui32PIOBuffer[32] = {0};
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    //
    // Configure the Cypress S25FS064S Configuration Register 1 Volatile based on the MSPI configuration.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

#if 0
    switch (psMSPISettings->eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            ui8Value = AM_DEVICES_MSPI_FLASH_SERIAL_MODE;
            break;
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
            ui8Value = AM_DEVICES_MSPI_FLASH_DUAL_MODE;
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_QUADPAIRED:
        case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
            ui8Value = AM_DEVICES_MSPI_FLASH_QUAD_MODE;
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            //break;
    }
#endif

    ui32PIOBuffer[0] = AM_DEVICES_MSPI_FLASH_CR1V | AM_DEVICES_MSPI_FLASH_CR1V_VALUE(AM_DEVICES_MSPI_FLASH_SERIAL_MODE);
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ANY_REG, false, 0, ui32PIOBuffer, 4);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Configure the Cypress S25FS064S Configuration Register 3 Volatile based on the MSPI configuration.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    ui32PIOBuffer[0] = AM_DEVICES_MSPI_FLASH_CR3V | AM_DEVICES_MSPI_FLASH_CR3V_VALUE;
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ANY_REG, false, 0, ui32PIOBuffer, 4);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Configure the Cypress S25FS064S Configuration Register 4 Volatile based on the MSPI configuration.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    ui32PIOBuffer[0] = AM_DEVICES_MSPI_FLASH_CR4V | AM_DEVICES_MSPI_FLASH_CR4V_VALUE;
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ANY_REG, false, 0, ui32PIOBuffer, 4);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Configure the Cypress S25FS064S Configuration Register 2 Volatile based on the MSPI configuration.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Configure the Addressing mode.
    //
    switch (pFlash->stSetting.eAddrCfg)
    {
        case AM_HAL_MSPI_ADDR_1_BYTE:
        case AM_HAL_MSPI_ADDR_2_BYTE:
        case AM_HAL_MSPI_ADDR_3_BYTE:
            ui8Value = 0;           // Use the Cypress MSPI Flash in 3-byte mode.
            break;
        case AM_HAL_MSPI_ADDR_4_BYTE:
            ui8Value = 0x80;        // Use the Cypress MSPI Flash in 4-byte mode.
            break;
    }

    //
    // Configure the read latency turnaround value.
    //
    switch (pFlash->stSetting.eXipMixedMode)
    {
        case AM_HAL_MSPI_XIPMIXED_NORMAL:
        case AM_HAL_MSPI_XIPMIXED_D2:
        case AM_HAL_MSPI_XIPMIXED_D4:
            ui8Value |= pFlash->stSetting.ui8TurnAround;  // Read latency
            break;
        case AM_HAL_MSPI_XIPMIXED_AD2:
            ui8Value |= pFlash->stSetting.ui8TurnAround - AM_DEVICES_MSPI_FLASH_ADDR_DUAL_EXT_DELAY;  // Read latency + Mode Bits
            break;
        case AM_HAL_MSPI_XIPMIXED_AD4:
            ui8Value |= pFlash->stSetting.ui8TurnAround - AM_DEVICES_MSPI_FLASH_ADDR_QUAD_EXT_DELAY;  // Read latency + Mode Bits
            break;
    }

    switch (pFlash->stSetting.eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_QUADPAIRED:
        case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
            ui8Value |= 0x40;
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    ui32PIOBuffer[0] = AM_DEVICES_MSPI_FLASH_CR2V | AM_DEVICES_MSPI_FLASH_CR2V_VALUE(ui8Value);

    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ANY_REG, false, 0, ui32PIOBuffer, 4);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Device specific de-initialization function.
//!
//! @param pHandle
//!
//! @return uint32_t
//
//*****************************************************************************
static uint32_t am_device_deinit_flash(void *pHandle)
{
    uint32_t      ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t      ui32PIOBuffer[32] = {0};
    //
    // Configure the Cypress S25FS064S Configuration Register 1 Volatile back to serial mode.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    ui32PIOBuffer[0] = AM_DEVICES_MSPI_FLASH_CR1V | AM_DEVICES_MSPI_FLASH_CR1V_VALUE(AM_DEVICES_MSPI_FLASH_SERIAL_MODE);
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ANY_REG, false, 0, ui32PIOBuffer, 4);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    else
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
    }
}
#endif

//*****************************************************************************
//
// Macronix MX25U12335F Support
//
//*****************************************************************************
#if defined (MACRONIX_MX25U12835F)
//*****************************************************************************
//
//! @brief Device specific initialization function.
//!
//! @param pHandle
//!
//! @return uint32_t
//
//*****************************************************************************
static uint32_t am_device_init_flash(void *pHandle)
{
    uint32_t      ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t      ui32PIOBuffer[32] = {0};
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

#if 0
    //
    // Enable writing to the Status/Configuration register.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Configure the Macronix MX25U12835F Status/Configuration Register.
    //
    ui32PIOBuffer[0] = (AM_DEVICES_MSPI_FLASH_CONFIG << 8) | AM_DEVICES_MSPI_FLASH_STATUS;
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_STATUS, false, 0, ui32PIOBuffer, 2);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Read the Configuration Register.
    //
    ui32Status = am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_CONFIG, false, 0, ui32PIOBuffer, 1);
#endif

    //
    // Configure the Macronix MX25U2835F Device mode.
    //
    switch (pFlash->stSetting.eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            // Nothing to do.  Device defaults to SPI mode.
            break;
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
            // Device does not support Dual mode.
            //return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_QUADPAIRED:
        case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
            ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_ENABLE_QPI_MODE, false, 0, ui32PIOBuffer, 0);
            if (AM_HAL_STATUS_SUCCESS != ui32Status)
            {
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            //break;
    }

    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Device specific de-initialization function.
//!
//! @param pHandle
//!
//! @return uint32_t
//
//*****************************************************************************
static uint32_t am_device_deinit_flash(void *pHandle)
{
    uint32_t      ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Configure the Macronix MX25U2835F Device mode.
    //
    switch (pFlash->stSetting.eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            // Nothing to do.  Device defaults to SPI mode.
            break;
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
            // Device does not support Dual mode.
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_QUADPAIRED:
        case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
            ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_DISABLE_QPI_MODE, false, 0, ui32PIOBuffer, 0);
            if (AM_HAL_STATUS_SUCCESS != ui32Status)
            {
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            //break;
    }

    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}
#endif

// ****************************************************************************//
//
//
//
// ****************************************************************************
am_devices_mspi_flash_config_t am_devices_mspi_flash_mode_switch(void *pHandle,
                                                                 am_devices_mspi_flash_config_t *psMSPISettings)
{
    uint32_t ui32Status = AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
    am_devices_mspi_flash_config_t MSPISettingsBak;
    am_hal_mspi_dev_config_t *psConfig = NULL;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    for ( uint32_t i = 0; i < (sizeof(g_MSPIFlash_DevConfig) / sizeof(g_MSPIFlash_DevConfig[0])); i++ )
    {
        if ( (psMSPISettings->eDeviceConfig == g_MSPIFlash_DevConfig[i].eHalDeviceEnum) &&
             (psMSPISettings->eMixedMode == g_MSPIFlash_DevConfig[i].eMixedModeEnum) )
        {
            psConfig = g_MSPIFlash_DevConfig[i].psDevConfig;
            psConfig->eClockFreq = psMSPISettings->eClockFreq;
            psConfig->pTCB = psMSPISettings->pNBTxnBuf;
            psConfig->ui32TCBSize = psMSPISettings->ui32NBTxnBufLength;
            psConfig->scramblingStartAddr = psMSPISettings->ui32ScramblingStartAddr;
            psConfig->scramblingEndAddr = psMSPISettings->ui32ScramblingEndAddr;
            break;
        }
    }
    MSPISettingsBak.eClockFreq = pFlash->stSetting.eClockFreq;
    MSPISettingsBak.eDeviceConfig = pFlash->stSetting.eDeviceConfig;
    MSPISettingsBak.eMixedMode = pFlash->stSetting.eXipMixedMode;
    MSPISettingsBak.pNBTxnBuf = pFlash->stSetting.pTCB;
    MSPISettingsBak.ui32NBTxnBufLength = pFlash->stSetting.ui32TCBSize;
    MSPISettingsBak.ui32ScramblingEndAddr = pFlash->stSetting.scramblingEndAddr;
    MSPISettingsBak.ui32ScramblingStartAddr = pFlash->stSetting.scramblingStartAddr;
    pFlash->stSetting = *psConfig;

    ui32Status = am_hal_mspi_disable(pFlash->pMspiHandle);

    ui32Status |= am_hal_mspi_device_configure(pFlash->pMspiHandle, &pFlash->stSetting);

    ui32Status |= am_hal_mspi_enable(pFlash->pMspiHandle);

    am_bsp_mspi_pins_enable(pFlash->ui32Module, pFlash->stSetting.eDeviceConfig);

    ui32Status |= am_hal_mspi_interrupt_clear(pFlash->pMspiHandle, AM_HAL_MSPI_INT_ERR | AM_HAL_MSPI_INT_DMACMP );
    ui32Status |= am_hal_mspi_interrupt_enable(pFlash->pMspiHandle, AM_HAL_MSPI_INT_ERR | AM_HAL_MSPI_INT_DMACMP );

#ifdef AM_IRQ_PRIORITY_DEFAULT
    NVIC_SetPriority(mspi_flash_interrupts[pFlash->ui32Module], AM_IRQ_PRIORITY_DEFAULT);
#endif // AM_IRQ_PRIORITY_DEFAULT
    NVIC_EnableIRQ(mspi_flash_interrupts[pFlash->ui32Module]);

    return MSPISettingsBak;
}

#if defined (MSPI_XIPMIXED)
static am_hal_mspi_dev_config_t am_devices_mspi_flash_mode_switch_static(void *pHandle,
                                                                         am_hal_mspi_dev_config_t *pMSPISettings)
{
    uint32_t ui32Status = AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;
    am_hal_mspi_dev_config_t MSPISettingsBak;
    if ( pMSPISettings )
    {
        MSPISettingsBak = pFlash->stSetting;
        pFlash->stSetting = *pMSPISettings;

        ui32Status = am_hal_mspi_disable(pFlash->pMspiHandle);

        ui32Status |= am_hal_mspi_device_configure(pFlash->pMspiHandle, &pFlash->stSetting);

        ui32Status |= am_hal_mspi_enable(pFlash->pMspiHandle);

        am_bsp_mspi_pins_enable(pFlash->ui32Module, pFlash->stSetting.eDeviceConfig);

        ui32Status |= am_hal_mspi_interrupt_clear(pFlash->pMspiHandle, AM_HAL_MSPI_INT_ERR | AM_HAL_MSPI_INT_DMACMP );
        ui32Status |= am_hal_mspi_interrupt_enable(pFlash->pMspiHandle, AM_HAL_MSPI_INT_ERR | AM_HAL_MSPI_INT_DMACMP );

#ifdef AM_IRQ_PRIORITY_DEFAULT
        NVIC_SetPriority(mspi_flash_interrupts[pFlash->ui32Module], AM_IRQ_PRIORITY_DEFAULT);
#endif // AM_IRQ_PRIORITY_DEFAULT
        NVIC_EnableIRQ(mspi_flash_interrupts[pFlash->ui32Module]);
    }

    return MSPISettingsBak;
}
#endif

//*****************************************************************************
//
//
//
//*****************************************************************************
void am_devices_mspi_mixmode_switch(void *pHandle, uint32_t ui32Mode)
{
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_MIXED_MODE, &ui32Mode);
}

//*****************************************************************************
//
// Adesto ATXP032 Support
//
//*****************************************************************************
#if defined (ADESTO_ATXP032)
//
// Device specific initialization function.
//
static uint32_t am_device_init_flash(void *pHandle)
{
    uint32_t      ui32PIOBuffer[32] = {0};
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    //
    // Set the Dummy Cycles in Status/Control register 3 to 8.
    //
    am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    ui32PIOBuffer[0] = 0x00000003;
    am_device_command_write(pHandle, AM_DEVICES_ATXP032_WRITE_STATUS_CTRL, false, 0, ui32PIOBuffer, 2);

    //
    // Configure the ATXP032 mode based on the MSPI configuration.
    //
    am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);

    switch ( pFlash->stSetting.eDeviceConfig )
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            am_device_command_write(pHandle, AM_DEVICES_ATXP032_RETURN_TO_SPI_MODE, false, 0, ui32PIOBuffer, 0);
            break;
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
        case AM_HAL_MSPI_FLASH_QUADPAIRED:
        case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
            am_device_command_write(pHandle, AM_DEVICES_ATXP032_ENTER_QUAD_MODE, false, 0, ui32PIOBuffer, 0);
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            am_device_command_write(pHandle, AM_DEVICES_ATXP032_ENTER_OCTAL_MODE, false, 0, ui32PIOBuffer, 0);
            break;
    }

    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//
// Device specific de-initialization function.
//
static uint32_t am_device_deinit_flash(void *pHandle)
{
    uint32_t      ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Configure the Adesto ATXP032 Device mode.
    //
    switch (pFlash->stSetting.eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            // Nothing to do.  Device defaults to SPI mode.
            break;
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
            // Device does not support Dual mode.
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            ui32Status = am_device_command_write(pHandle, AM_DEVICES_ATXP032_RETURN_TO_SPI_MODE, false, 0, ui32PIOBuffer, 0);
            if (AM_HAL_STATUS_SUCCESS != ui32Status)
            {
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            break;
        case AM_HAL_MSPI_FLASH_QUADPAIRED:
        case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            //break;
    }

    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}
#endif

//*****************************************************************************
//
//! @brief  Generic Command Write function.
//!
//! @param pHandle
//! @param ui8Instr
//! @param bSendAddr
//! @param ui32Addr
//! @param pData
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
static uint32_t am_device_command_write(void *pHandle, uint8_t ui8Instr, bool bSendAddr,
                                        uint32_t ui32Addr, uint32_t *pData,
                                        uint32_t ui32NumBytes)
{
    uint32_t ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;
    am_hal_mspi_pio_transfer_t      stMSPIFlashPIOTransaction = {0};

    // Create the individual write transaction.
    stMSPIFlashPIOTransaction.ui32NumBytes       = ui32NumBytes;
    stMSPIFlashPIOTransaction.eDirection         = AM_HAL_MSPI_TX;
    stMSPIFlashPIOTransaction.bSendAddr          = bSendAddr;
    stMSPIFlashPIOTransaction.ui32DeviceAddr     = ui32Addr;
    stMSPIFlashPIOTransaction.bSendInstr         = true;
    stMSPIFlashPIOTransaction.ui16DeviceInstr    = ui8Instr;
    stMSPIFlashPIOTransaction.bTurnaround        = false;
#if 0 // A3DS-25 Deprecate MSPI CONT
    stMSPIFlashPIOTransaction.bContinue          = false;
#endif // A3DS-25

    if (AM_HAL_MSPI_FLASH_QUADPAIRED == pFlash->stSetting.eDeviceConfig)
    {
        stMSPIFlashPIOTransaction.bQuadCmd         = true;
    }
    else
    {
        stMSPIFlashPIOTransaction.bQuadCmd         = false;
    }

    stMSPIFlashPIOTransaction.pui32Buffer        = pData;

#if defined (MSPI_XIPMIXED)
    am_hal_mspi_dev_config_t mode = am_devices_mspi_flash_mode_switch_static(pHandle, &MSPI_Flash_Serial_CE0_MSPIConfig);
#endif

    // Execute the transction over MSPI.
    ui32Status = am_hal_mspi_blocking_transfer(pFlash->pMspiHandle, &stMSPIFlashPIOTransaction,
                                         AM_DEVICES_MSPI_FLASH_TIMEOUT);
#if defined (MSPI_XIPMIXED)
    am_devices_mspi_flash_mode_switch_static(pHandle, &mode);
#endif

    return ui32Status;
}

//*****************************************************************************
//
//! @brief Generic Command Read function.
//!
//! @param pHandle
//! @param ui8Instr
//! @param bSendAddr
//! @param ui32Addr
//! @param pData
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
static uint32_t am_device_command_read(void *pHandle, uint8_t ui8Instr, bool bSendAddr,
                                       uint32_t ui32Addr, uint32_t *pData,
                                       uint32_t ui32NumBytes)
{
    uint32_t ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;
    am_hal_mspi_pio_transfer_t      stMSPIFlashPIOTransaction = {0};

    // Create the individual write transaction.
    stMSPIFlashPIOTransaction.eDirection         = AM_HAL_MSPI_RX;
    stMSPIFlashPIOTransaction.bSendAddr          = bSendAddr;
    stMSPIFlashPIOTransaction.ui32DeviceAddr     = ui32Addr;
    stMSPIFlashPIOTransaction.bSendInstr         = true;
    stMSPIFlashPIOTransaction.ui16DeviceInstr    = ui8Instr;
    stMSPIFlashPIOTransaction.bTurnaround        = false;
#if 0 // A3DS-25 Deprecate MSPI CONT
    stMSPIFlashPIOTransaction.bContinue          = false;
#endif // A3DS-25

    if (AM_HAL_MSPI_FLASH_QUADPAIRED == pFlash->stSetting.eDeviceConfig)
    {
        stMSPIFlashPIOTransaction.ui32NumBytes     = ui32NumBytes * 2;
        stMSPIFlashPIOTransaction.bQuadCmd      = true;
    }
    else
    {
        stMSPIFlashPIOTransaction.ui32NumBytes     = ui32NumBytes;
        stMSPIFlashPIOTransaction.bQuadCmd      = false;
    }

    stMSPIFlashPIOTransaction.pui32Buffer        = pData;

#if defined (MSPI_XIPMIXED)
    am_hal_mspi_dev_config_t mode = am_devices_mspi_flash_mode_switch_static(pHandle, &MSPI_Flash_Serial_CE0_MSPIConfig);
#endif

    // Execute the transction over MSPI.
    ui32Status = am_hal_mspi_blocking_transfer(pFlash->pMspiHandle, &stMSPIFlashPIOTransaction,
                                         AM_DEVICES_MSPI_FLASH_TIMEOUT);
#if defined (MSPI_XIPMIXED)
    am_devices_mspi_flash_mode_switch_static(pHandle, &mode);
#endif

    return ui32Status;
}

//*****************************************************************************
//
//! @brief
//! @param pCallbackCtxt
//! @param status
//
//*****************************************************************************
static void pfnMSPI_FLASH_Callback(void *pCallbackCtxt, uint32_t status)
{
    // Set the DMA complete flag.
    *(volatile bool *)pCallbackCtxt = true;
}

//*****************************************************************************
//
// Initialize the mspi_flash driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_init(uint32_t ui32Module, am_devices_mspi_flash_config_t *psMSPISettings, void **ppHandle, void **ppMspiHandle)
{
    uint32_t      ui32Status;
    am_hal_mspi_dev_config_t *psConfig;
    void                     *pMspiHandle;
    uint32_t      ui32Index = 0;

    if ((ui32Module > AM_REG_MSPI_NUM_MODULES) || (psMSPISettings == NULL))
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < AM_DEVICES_MSPI_FLASH_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( gAmFlash[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == AM_DEVICES_MSPI_FLASH_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    for ( uint32_t i = 0; i < (sizeof(g_MSPIFlash_DevConfig) / sizeof(g_MSPIFlash_DevConfig[0])); i++ )
    {
        if ( (psMSPISettings->eDeviceConfig == g_MSPIFlash_DevConfig[i].eHalDeviceEnum) &&
             (psMSPISettings->eMixedMode == g_MSPIFlash_DevConfig[i].eMixedModeEnum) )
        {
            psConfig = g_MSPIFlash_DevConfig[i].psDevConfig;
            psConfig->eClockFreq = psMSPISettings->eClockFreq;
            psConfig->pTCB = psMSPISettings->pNBTxnBuf;
            psConfig->ui32TCBSize = psMSPISettings->ui32NBTxnBufLength;
            psConfig->scramblingStartAddr = psMSPISettings->ui32ScramblingStartAddr;
            psConfig->scramblingEndAddr = psMSPISettings->ui32ScramblingEndAddr;
            break;
        }
    }

    //
    // Enable fault detection.
    //
#if !defined(AM_PART_APOLLO5_API)
#if defined(AM_PART_APOLLO4_API)
    am_hal_fault_capture_enable();
#elif AM_PART_APOLLO3_API
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_ENABLE, 0);
#else
    am_hal_mcuctrl_fault_capture_enable();
#endif
#endif

    //
    // Configure the MSPI for Serial or Quad-Paired Serial operation during initialization.
    //
    switch (psConfig->eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_DUAL_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
            gAmFlash[ui32Index].stSetting = MSPI_Flash_Serial_CE0_MSPIConfig;
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_initialize(ui32Module, &pMspiHandle))
            {
                am_util_stdio_printf("Error - Failed to initialize MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }

            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pMspiHandle, AM_HAL_SYSCTRL_WAKE, false))
            {
                am_util_stdio_printf("Error - Failed to power on MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }

            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(pMspiHandle, &MSPI_Flash_Serial_CE0_MSPIConfig))
            {
                am_util_stdio_printf("Error - Failed to configure MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(pMspiHandle))
            {
                am_util_stdio_printf("Error - Failed to enable MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            am_bsp_mspi_pins_enable(ui32Module, MSPI_Flash_Serial_CE0_MSPIConfig.eDeviceConfig);
            break;
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
        case AM_HAL_MSPI_FLASH_DUAL_CE1:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            gAmFlash[ui32Index].stSetting = MSPI_Flash_Serial_CE1_MSPIConfig;
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_initialize(ui32Module, &pMspiHandle))
            {
                am_util_stdio_printf("Error - Failed to initialize MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }

            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pMspiHandle, AM_HAL_SYSCTRL_WAKE, false))
            {
                am_util_stdio_printf("Error - Failed to power on MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }

            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(pMspiHandle, &MSPI_Flash_Serial_CE1_MSPIConfig))
            {
                am_util_stdio_printf("Error - Failed to configure MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(pMspiHandle))
            {
                am_util_stdio_printf("Error - Failed to enable MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            am_bsp_mspi_pins_enable(ui32Module, MSPI_Flash_Serial_CE1_MSPIConfig.eDeviceConfig);
            break;
        case AM_HAL_MSPI_FLASH_QUADPAIRED:
            gAmFlash[ui32Index].stSetting = MSPI_Flash_QuadPairedSerialMSPIConfig;
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_initialize(ui32Module, &pMspiHandle))
            {
                am_util_stdio_printf("Error - Failed to initialize MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }

            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pMspiHandle, AM_HAL_SYSCTRL_WAKE, false))
            {
                am_util_stdio_printf("Error - Failed to power on MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }

            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(pMspiHandle, &MSPI_Flash_QuadPairedSerialMSPIConfig))
            {
                am_util_stdio_printf("Error - Failed to configure MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(pMspiHandle))
            {
                am_util_stdio_printf("Error - Failed to enable MSPI.\n");
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }
            am_bsp_mspi_pins_enable(ui32Module, MSPI_Flash_QuadPairedSerialMSPIConfig.eDeviceConfig);
            break;
        case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            //break;
    }

    gAmFlash[ui32Index].pMspiHandle = pMspiHandle;
    gAmFlash[ui32Index].ui32Module = ui32Module;

    if (AM_HAL_STATUS_SUCCESS != am_devices_mspi_flash_reset((void*)&gAmFlash[ui32Index]))
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Initialize the MSPI settings for the MSPI_FLASH.
    //
    gAmFlash[ui32Index].stSetting = *psConfig;

    //
    // Device specific MSPI Flash initialization.
    //
    ui32Status = am_device_init_flash((void*)&gAmFlash[ui32Index]);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    // Disable MSPI defore re-configuring it
    ui32Status = am_hal_mspi_disable(pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    //
    // Re-Configure the MSPI for the requested operation mode.
    //
    ui32Status = am_hal_mspi_device_configure(pMspiHandle, psConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    // Re-Enable MSPI
    ui32Status = am_hal_mspi_enable(pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Configure the MSPI pins.
    //
    am_bsp_mspi_pins_enable(ui32Module, psConfig->eDeviceConfig);

    //
    // Enable MSPI interrupts.
    //

    ui32Status = am_hal_mspi_interrupt_clear(pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    ui32Status = am_hal_mspi_interrupt_enable(pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Return the handle.
    //
    gAmFlash[ui32Index].bOccupied = true;
    *ppMspiHandle = pMspiHandle;
    *ppHandle = (void *)&gAmFlash[ui32Index];

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
// De-Initialization the mspi_flash driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_deinit(void *pHandle)
{
    uint32_t      ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    //
    // Device specific MSPI Flash de-initialization.
    //
    ui32Status = am_device_deinit_flash(pHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    if (AM_HAL_STATUS_SUCCESS != am_devices_mspi_flash_reset(pHandle))
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Disable the MSPI instance.
    //
    ui32Status = am_hal_mspi_disable(pFlash->pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pFlash->pMspiHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false))
    {
        am_util_stdio_printf("Error - Failed to power on MSPI.\n");
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Deinitialize the MSPI instance.
    //
    ui32Status = am_hal_mspi_deinitialize(pFlash->pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    // Free this device handle
    pFlash->bOccupied = false;

    //
    // Clear the Flash Caching.
    //
#if AM_CMSIS_REGS
    CACHECTRL->CACHECFG = 0;
#else // AM_CMSIS_REGS
    AM_REG(CACHECTRL, CACHECFG) = 0;
#endif // AM_CMSIS_REGS

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
// This will reset the external flash device
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_reset(void *pHandle)
{
    uint32_t      ui32PIOBuffer[32] = {0};

#if defined(ADESTO_ATXP032)
 //
  // Return the device to SPI mode.
  //
  if (AM_HAL_STATUS_SUCCESS != am_device_command_write(pHandle, AM_DEVICES_ATXP032_RETURN_TO_SPI_MODE, false, 0, ui32PIOBuffer, 0))
  {
    return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
  }
#else
    //
    // Send the command sequence to reset the device and return status.
    //
    if (AM_HAL_STATUS_SUCCESS != am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_RESET_ENABLE, false, 0, ui32PIOBuffer, 0))
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    if (AM_HAL_STATUS_SUCCESS != am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_RESET_MEMORY, false, 0, ui32PIOBuffer, 0))
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
#endif

    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Reads the ID of the external flash and returns the value.
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_id(void *pHandle)
{
    uint32_t      ui32Status;
    uint32_t      ui32DeviceID;

    //
    // Send the command sequence to read the Device ID and return status.
    //
#if defined(ADESTO_ATXP032)
  uint8_t       ui8Response[11];
  ui32Status = am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_ID, false, 0, (uint32_t *)&ui8Response[0], 11);
  ui32DeviceID = (ui8Response[7] << 16) | (ui8Response[8] << 8) | ui8Response[9];
#else
    ui32Status = am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_ID, false, 0, &ui32DeviceID, 3);
    am_util_stdio_printf("Flash ID is %8.8X\n", ui32DeviceID);
#endif
    if ( ((ui32DeviceID & AM_DEVICES_MSPI_FLASH_ID_MASK) == AM_DEVICES_MSPI_FLASH_ID) &&
       (AM_HAL_STATUS_SUCCESS == ui32Status) )
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
    }
    else
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
}

//*****************************************************************************
//
// Reads the current status of the external flash
//
//*****************************************************************************

uint32_t
am_devices_mspi_flash_status(void *pHandle, uint32_t *pStatus)
{
    uint32_t      ui32Status;

    //
    // Send the command sequence to read the device status.
    //
    ui32Status = am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS, false, 0, pStatus, 1);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_read_adv(void *pHandle, uint8_t *pui8RxBuffer,
                               uint32_t ui32ReadAddress,
                               uint32_t ui32NumBytes,
                               uint32_t ui32PauseCondition,
                               uint32_t ui32StatusSetClr,
                               am_hal_mspi_callback_t pfnCallback,
                               void *pCallbackCtxt)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    uint32_t                      ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    // Set the DMA priority
    Transaction.ui8Priority = 1;

    // Set the transfer direction to RX (Read)
    Transaction.eDirection = AM_HAL_MSPI_RX;

    // Set the transfer count in bytes.
    Transaction.ui32TransferCount = ui32NumBytes;

    // Set the address to read data from.
    Transaction.ui32DeviceAddress = ui32ReadAddress;

    // Set the target SRAM buffer address.
    Transaction.ui32SRAMAddress = (uint32_t)pui8RxBuffer;

    // Clear the CQ stimulus.
    Transaction.ui32PauseCondition = ui32PauseCondition;
    // Clear the post-processing
    Transaction.ui32StatusSetClr = ui32StatusSetClr;

    // Check the transaction status.
    ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction,
                                                  AM_HAL_MSPI_TRANS_DMA, pfnCallback, pCallbackCtxt);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Reads the contents of the external flash into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_read(void *pHandle, uint8_t *pui8RxBuffer,
                           uint32_t ui32ReadAddress,
                           uint32_t ui32NumBytes,
                           bool bWaitForCompletion)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    uint32_t                      ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    // Set the DMA priority
    Transaction.ui8Priority = 1;

    // Set the transfer direction to RX (Read)
    Transaction.eDirection = AM_HAL_MSPI_RX;

    // Set the transfer count in bytes.
    Transaction.ui32TransferCount = ui32NumBytes;

    // Set the address to read data from.
    Transaction.ui32DeviceAddress = ui32ReadAddress;

    // Set the target SRAM buffer address.
    Transaction.ui32SRAMAddress = (uint32_t)pui8RxBuffer;

    // Clear the CQ stimulus.
    Transaction.ui32PauseCondition = 0;
    // Clear the post-processing
    Transaction.ui32StatusSetClr = 0;

    if (bWaitForCompletion)
    {
        // Start the transaction.
        volatile bool bDMAComplete = false;
        ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA, pfnMSPI_FLASH_Callback, (void *)&bDMAComplete);

        // Check the transaction status.
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }

        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_FLASH_TIMEOUT; i++)
        {
            if (bDMAComplete)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_hal_flash_delay( FLASH_CYCLES_US(1) );
        }

        // Check the status.
        if (!bDMAComplete)
        {
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }
    }
    else
    {
        // Check the transaction status.
        ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction,
                                                      AM_HAL_MSPI_TRANS_DMA, NULL, NULL);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Reads the contents of the external flash into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_read_hiprio(void *pHandle, uint8_t *pui8RxBuffer,
                                  uint32_t ui32ReadAddress,
                                  uint32_t ui32NumBytes,
                                  bool bWaitForCompletion)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    uint32_t                      ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    // Set the DMA priority
    Transaction.ui8Priority = 1;

    // Set the transfer direction to RX (Read)
    Transaction.eDirection = AM_HAL_MSPI_RX;

    // Set the transfer count in bytes.
    Transaction.ui32TransferCount = ui32NumBytes;

    // Set the address to read data from.
    Transaction.ui32DeviceAddress = ui32ReadAddress;

    // Set the target SRAM buffer address.
    Transaction.ui32SRAMAddress = (uint32_t)pui8RxBuffer;

    // Clear the CQ stimulus.
    Transaction.ui32PauseCondition = 0;
    // Clear the post-processing
    Transaction.ui32StatusSetClr = 0;

    if (bWaitForCompletion)
    {
        // Start the transaction.
        volatile bool bDMAComplete = false;
        ui32Status = am_hal_mspi_highprio_transfer(pFlash->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA, pfnMSPI_FLASH_Callback, (void*)&bDMAComplete);

        // Check the transaction status.
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }

        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_FLASH_TIMEOUT; i++)
        {
            if (bDMAComplete)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_hal_flash_delay( FLASH_CYCLES_US(1) );
        }

        // Check the status.
        if (!bDMAComplete)
        {
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }
    }
    else
    {
        // Check the transaction status.
        ui32Status = am_hal_mspi_highprio_transfer(pFlash->pMspiHandle, &Transaction,
                                                      AM_HAL_MSPI_TRANS_DMA, NULL, NULL);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

// ##### INTERNAL BEGIN #####

//*****************************************************************************
//
//
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_read_callback(void *pHandle, uint8_t *pui8RxBuffer,
                                    uint32_t ui32ReadAddress,
                                    uint32_t ui32NumBytes)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    uint32_t                      ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    // Set the DMA priority
    Transaction.ui8Priority = 1;

    // Set the transfer direction to RX (Read)
    Transaction.eDirection = AM_HAL_MSPI_RX;

    // Set the transfer count in bytes.
    Transaction.ui32TransferCount = ui32NumBytes;

    // Set the address to read data from.
    Transaction.ui32DeviceAddress = ui32ReadAddress;

    // Set the target SRAM buffer address.
    Transaction.ui32SRAMAddress = (uint32_t)pui8RxBuffer;

    // Clear the CQ stimulus.
    Transaction.ui32PauseCondition = 0;

    // Clear the post-processing
    Transaction.ui32StatusSetClr = 0;

    // Start the transaction.
    volatile bool bDMAComplete = false;
    ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction,
                                                  AM_HAL_MSPI_TRANS_DMA,
                                                  pfnMSPI_FLASH_Callback, (void*)&bDMAComplete);

    // Check the transaction status.
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}
// ##### INTERNAL END #####

//*****************************************************************************
//
//  Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_write(void *pHandle, uint8_t *pui8TxBuffer,
                            uint32_t ui32WriteAddress,
                            uint32_t ui32NumBytes,
                            bool bWaitForCompletion)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    bool                          bWriteComplete = false;
    uint32_t                      ui32BytesLeft = ui32NumBytes;
    uint32_t                      ui32PageAddress = ui32WriteAddress;
    uint32_t                      ui32BufferAddress = (uint32_t)pui8TxBuffer;
    uint32_t                      ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    while (ui32BytesLeft > 0)
    {
        //
        // Send the command sequence to enable writing.
        //
        ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }

        // Set the DMA priority
        Transaction.ui8Priority = 1;

        // Set the transfer direction to TX (Write)
        Transaction.eDirection = AM_HAL_MSPI_TX;

        if (ui32BytesLeft > AM_DEVICES_MSPI_FLASH_PAGE_SIZE)
        {
            // Set the transfer count in bytes.
            Transaction.ui32TransferCount = AM_DEVICES_MSPI_FLASH_PAGE_SIZE;
            ui32BytesLeft -= AM_DEVICES_MSPI_FLASH_PAGE_SIZE;
        }
        else
        {
            // Set the transfer count in bytes.
            Transaction.ui32TransferCount = ui32BytesLeft;
            ui32BytesLeft = 0;
        }

        // Set the address configuration for the read
        //    Transaction.eAddrCfg = AM_HAL_MSPI_ADDR_3_BYTE;

        //    Transaction.eAddrExtSize = AM_HAL_MSPI_ADDR_EXT_0_BYTE;
        //    Transaction.ui8AddrExtValue = 0;

        // Set the address to read data to.
        Transaction.ui32DeviceAddress = ui32PageAddress;
        ui32PageAddress += AM_DEVICES_MSPI_FLASH_PAGE_SIZE;

        // Set the source SRAM buffer address.
        Transaction.ui32SRAMAddress = ui32BufferAddress;
        ui32BufferAddress += AM_DEVICES_MSPI_FLASH_PAGE_SIZE;

        // Clear the CQ stimulus.
        Transaction.ui32PauseCondition = 0;
        // Clear the post-processing
        Transaction.ui32StatusSetClr = 0;

        // Start the transaction.
        volatile bool bDMAComplete = false;
        ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA, pfnMSPI_FLASH_Callback, (void*)&bDMAComplete);

        // Check the transaction status.
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }

        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_FLASH_TIMEOUT; i++)
        {
            if (bDMAComplete)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_hal_flash_delay( FLASH_CYCLES_US(1) );
        }

        // Check the status.
        if (!bDMAComplete)
        {
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }

#if defined (ADESTO_ATXP032)
        //
        // Wait for the Write In Progress to indicate the erase is complete.
        //
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_FLASH_TIMEOUT; i++)
        {
            // ATXP032 has different number of bytes for each speed of status read.
            switch ( pFlash->stSetting.eDeviceConfig )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS, false, 0, ui32PIOBuffer, 2);
                    bWriteComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_ATXP032_WIP));
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                    am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS, false, 0, ui32PIOBuffer, 4);
                    bWriteComplete = (0 == ((ui32PIOBuffer[0] >> 16) & AM_DEVICES_ATXP032_WIP));
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS, false, 0, ui32PIOBuffer, 6);
                    bWriteComplete = (0 == (ui32PIOBuffer[1] & AM_DEVICES_ATXP032_WIP));
                    break;
                case AM_HAL_MSPI_FLASH_DUAL_CE0:
                case AM_HAL_MSPI_FLASH_DUAL_CE1:
                case AM_HAL_MSPI_FLASH_QUADPAIRED:
                case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
                    return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
            }

            am_util_delay_us(100);
            if (bWriteComplete)
            {
                break;
            }
        }

#else
        //
        // Wait for the Write In Progress to indicate the erase is complete.
        //
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_FLASH_TIMEOUT; i++)
        {
          ui32Status = am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS, false, 0, ui32PIOBuffer, 1);
          if ((AM_HAL_MSPI_FLASH_QUADPAIRED == pFlash->stSetting.eDeviceConfig) ||
              (AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL == pFlash->stSetting.eDeviceConfig))
          {
            bWriteComplete = ((0 == (ui32PIOBuffer[0] & AM_DEVICES_MSPI_FLASH_WIP)) &&
                              (0 == ((ui32PIOBuffer[0] >> 8) & AM_DEVICES_MSPI_FLASH_WIP)));
          }
          else
          {
            bWriteComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_MSPI_FLASH_WIP));
          }
          am_util_delay_us(100);
          if (bWriteComplete)
          {
            break;
          }
        }
#endif

        //
        // Send the command sequence to disable writing.
        //
        ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_DISABLE, false, 0, ui32PIOBuffer, 0);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }
    }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Erases the entire contents of the external flash
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_mass_erase(void *pHandle)
{
    bool          bEraseComplete = false;
    uint32_t      ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Send the command sequence to enable writing.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Send the command sequence to do the mass erase.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_BULK_ERASE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    //
    // Wait for the Write In Progress to indicate the erase is complete.
    //
    for (uint32_t i = 0; i < AM_DEVICES_MSPI_FLASH_ERASE_TIMEOUT; i++)
    {
        ui32PIOBuffer[0] = 0;
        am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS, false, 0, ui32PIOBuffer, 1);
        if ((AM_HAL_MSPI_FLASH_QUADPAIRED == pFlash->stSetting.eDeviceConfig) ||
            (AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL == pFlash->stSetting.eDeviceConfig))
        {
            bEraseComplete = ((0 == (ui32PIOBuffer[0] & AM_DEVICES_MSPI_FLASH_WIP)) &&
                              (0 == ((ui32PIOBuffer[0] >> 8) & AM_DEVICES_MSPI_FLASH_WIP)));
        }
        else
        {
            bEraseComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_MSPI_FLASH_WIP));
        }
        if (bEraseComplete)
        {
            break;
        }
        am_util_delay_ms(10);
    }

    //
    // Check the status.
    //
    if (!bEraseComplete)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Send the command sequence to disable writing.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_DISABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Erases the contents of a single sector of flash
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_sector_erase(void *pHandle, uint32_t ui32SectorAddress)
{
    bool          bEraseComplete = false;
    uint32_t      ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Send the command sequence to enable writing.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

#if defined (ADESTO_ATXP032)
    //
    // Send the command to remove protection from the sector.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_ATXP032_UNPROTECT_SECTOR, true, ui32SectorAddress, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Send the command sequence to enable writing.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
#endif

    //
    // Send the command sequence to do the sector erase.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_SECTOR_ERASE, true, ui32SectorAddress, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

#if defined (ADESTO_ATXP032)
    //
    // Wait for the Write In Progress to indicate the erase is complete.
    //
    for (uint32_t i = 0; i < AM_DEVICES_MSPI_FLASH_ERASE_TIMEOUT; i++)
    {
        // ATXP032 has different number of bytes for each speed of status read.
        switch ( pFlash->stSetting.eDeviceConfig )
        {
            case AM_HAL_MSPI_FLASH_SERIAL_CE0:
            case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS, false, 0, ui32PIOBuffer, 2);
                bEraseComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_ATXP032_WIP));
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE0:
            case AM_HAL_MSPI_FLASH_QUAD_CE1:
                am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS, false, 0, ui32PIOBuffer, 4);
                bEraseComplete = (0 == ((ui32PIOBuffer[0] >> 16) & AM_DEVICES_ATXP032_WIP));
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE0:
            case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS, false, 0, ui32PIOBuffer, 6);
                bEraseComplete = (0 == (ui32PIOBuffer[1] & AM_DEVICES_ATXP032_WIP));
                break;
            case AM_HAL_MSPI_FLASH_DUAL_CE0:
            case AM_HAL_MSPI_FLASH_DUAL_CE1:
            case AM_HAL_MSPI_FLASH_QUADPAIRED:
            case AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL:
                return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
        }

        if (bEraseComplete)
        {
            break;
        }
        am_util_delay_ms(10);
    }
#else
    //
    // Wait for the Write In Progress to indicate the erase is complete.
    //
    for (uint32_t i = 0; i < AM_DEVICES_MSPI_FLASH_ERASE_TIMEOUT; i++)
    {
        ui32PIOBuffer[0] = 0;
        am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS, false, 0, ui32PIOBuffer, 1);
        if ((AM_HAL_MSPI_FLASH_QUADPAIRED == pFlash->stSetting.eDeviceConfig) ||
            (AM_HAL_MSPI_FLASH_QUADPAIRED_SERIAL == pFlash->stSetting.eDeviceConfig))
        {
            bEraseComplete = ((0 == (ui32PIOBuffer[0] & AM_DEVICES_MSPI_FLASH_WIP)) &&
                              (0 == ((ui32PIOBuffer[0] >> 8) & AM_DEVICES_MSPI_FLASH_WIP)));
        }
        else
        {
            bEraseComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_MSPI_FLASH_WIP));
        }
        if (bEraseComplete)
        {
            break;
        }
        am_util_delay_ms(10);
    }

#endif
    //
    // Check the status.
    //
    if (!bEraseComplete)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

#if defined (CYPRESS_S25FS064S)
    uint32_t    ui32EraseStatus = 0;
    //
    // Send the command sequence to check the erase status.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_EVAL_ERASE_STATUS, true, ui32SectorAddress, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Send the command sequence to read the device status.
    //
    ui32Status = am_device_command_read(pHandle, AM_DEVICES_MSPI_FLASH_READ_STATUS2, false, 0, &ui32EraseStatus, 1);
    if ( (AM_HAL_STATUS_SUCCESS != ui32Status) || ((AM_DEVICES_MSPI_FLASH_ERASE_SUCCESS & ui32EraseStatus) == 0) )
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
#endif

    //
    // Send the command sequence to disable writing.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_DISABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Sets up the MSPI and external FLASH into XIP mode.
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_enable_xip(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    //
    // Enable XIP on the MSPI.
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_EN, NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

#if !MSPI_USE_CQ
    // Disable the DMA interrupts.
    ui32Status = am_hal_mspi_interrupt_disable(pFlash->pMspiHandle,
                                               AM_HAL_MSPI_INT_DMAERR |
                                               AM_HAL_MSPI_INT_DMACMP );
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
#endif

    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Removes the MSPI and external FLASH from XIP mode.
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_disable_xip(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Send the command to enable writing.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Disable XIP on the MSPI.
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_DIS, NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Sets up the MSPI and external FLASH into scrambling mode.
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_enable_scrambling(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;

    //
    // Enable scrambling on the MSPI.
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SCRAMB_EN, NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }
    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Removes the MSPI and external FLASH from scrambling mode.
//
//*****************************************************************************
uint32_t
am_devices_mspi_flash_disable_scrambling(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_flash_t *pFlash = (am_devices_mspi_flash_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Send the command to enable writing.
    //
    ui32Status = am_device_command_write(pHandle, AM_DEVICES_MSPI_FLASH_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    //
    // Disable Scrambling on the MSPI.
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SCRAMB_DIS, NULL);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_FLASH_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_FLASH_STATUS_SUCCESS;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

