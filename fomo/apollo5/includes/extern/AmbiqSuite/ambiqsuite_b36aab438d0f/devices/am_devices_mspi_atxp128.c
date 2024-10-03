//*****************************************************************************
//
//! @file am_devices_mspi_atxp128.c
//!
//! @brief Micron Serial NOR SPI Flash driver.
//!
//! @addtogroup atxp128 ATXP128 MSPI Driver
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
#include "am_mcu_apollo.h"
#include "am_devices_mspi_atxp128.h"
#include "am_util_stdio.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
#define AM_DEVICES_MSPI_ATXP128_TIMEOUT                     1000000
#define AM_DEVICES_MSPI_ATXP128_ERASE_TIMEOUT               1000000
#define AM_DEVICES_MSPI_ATXP128_SECTOR_FOR_TIMING_CHECK     30       // max 31

#define ATXP128_TIMING_SCAN_MIN_ACCEPTANCE_LENGTH           (8)     // there should be at least
                                                                    // this amount of consecutive
                                                                    // passing settings to be accepted.
#define FLASH_CHECK_DATA_SIZE_BYTES                         AM_DEVICES_MSPI_ATXP128_PAGE_SIZE   // Data trunk size
#define FLASH_TIMING_SCAN_SIZE_BYTES                        AM_DEVICES_MSPI_ATXP128_SECTOR_SIZE // Total scan size
#define FLASH_TEST_PATTERN_NUMBER                           5       // 5 patterns

typedef struct
{
    uint32_t                    ui32Module;
    void                        *pMspiHandle;
    am_hal_mspi_dev_config_t    stSetting;
    bool                        bOccupied;
} am_devices_mspi_atxp128_t;

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
am_hal_mspi_xip_config_t gXipConfig[] =
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

am_hal_mspi_config_t gMspiCfg =
{
  .ui32TCBSize          = 0,
  .pTCB                 = NULL,
  .bClkonD4             = 0
};
#endif

am_devices_mspi_atxp128_t gAmAtxp128[AM_DEVICES_MSPI_ATXP128_MAX_DEVICE_NUM];

am_hal_mspi_dev_config_t MSPI_ATXP128_Serial_CE0_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_16MHZ,
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
  .ui16ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui16WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#else
  .ui8ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#endif
#if defined(AM_PART_APOLLO3P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#endif
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#if defined(AM_PART_APOLLO4)
    .eDeviceNum           = AM_HAL_MSPI_DEVICE0,
#endif
#else
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
#endif
};

am_hal_mspi_dev_config_t MSPI_ATXP128_Serial_CE1_MSPIConfig =
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
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
  .ui16ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui16WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#else
  .ui8ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#endif
#if defined(AM_PART_APOLLO3P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#endif
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#if defined(AM_PART_APOLLO4)
    .eDeviceNum           = AM_HAL_MSPI_DEVICE0,
#endif
#else
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
#endif
};

am_hal_mspi_dev_config_t MSPI_ATXP128_Quad_CE0_MSPIConfig =
{
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
  .ui8TurnAround        = 8,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_QUAD_CE0,
  .bSendInstr           = true,
  .bSendAddr            = true,
  .bTurnaround          = true,
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
  .ui16ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui16WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#else
  .ui8ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#endif
#if defined(AM_PART_APOLLO3P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#endif
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#if defined(AM_PART_APOLLO4)
    .eDeviceNum           = AM_HAL_MSPI_DEVICE0,
#endif
#else
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
#endif
};

am_hal_mspi_dev_config_t MSPI_ATXP128_Quad_CE1_MSPIConfig =
{
    .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
    .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
    .ui8TurnAround        = 8,
    .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
    .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
    .eDeviceConfig        = AM_HAL_MSPI_FLASH_QUAD_CE1,
    .bSendInstr           = true,
    .bSendAddr            = true,
    .bTurnaround          = true,
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
  .ui16ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui16WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#else
  .ui8ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#endif
#if defined(AM_PART_APOLLO3P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#endif
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#if defined(AM_PART_APOLLO4)
    .eDeviceNum           = AM_HAL_MSPI_DEVICE0,
#endif
#else
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
#endif
};

am_hal_mspi_dev_config_t MSPI_ATXP128_Octal_CE0_MSPIConfig =
{
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
  .ui8TurnAround        = 8,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_OCTAL_CE0,
  .bSendInstr           = true,
  .bSendAddr            = true,
  .bTurnaround          = true,
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
  .ui16ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui16WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#else
  .ui8ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#endif
#if defined(AM_PART_APOLLO3P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#endif
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#if defined(AM_PART_APOLLO4)
    .eDeviceNum           = AM_HAL_MSPI_DEVICE0,
#endif
#else
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
#endif
};

am_hal_mspi_dev_config_t MSPI_ATXP128_Octal_CE1_MSPIConfig =
{
  .eSpiMode             = AM_HAL_MSPI_SPI_MODE_0,
  .eClockFreq           = AM_HAL_MSPI_CLK_24MHZ,
  .ui8TurnAround        = 8,
  .eAddrCfg             = AM_HAL_MSPI_ADDR_4_BYTE,
  .eInstrCfg            = AM_HAL_MSPI_INSTR_1_BYTE,
  .eDeviceConfig        = AM_HAL_MSPI_FLASH_OCTAL_CE1,
  .bSendInstr           = true,
  .bSendAddr            = true,
  .bTurnaround          = true,
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
  .ui16ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui16WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#else
  .ui8ReadInstr         = AM_DEVICES_MSPI_ATXP128_FAST_READ,
  .ui8WriteInstr        = AM_DEVICES_MSPI_ATXP128_PAGE_PROGRAM,
#endif
#if defined(AM_PART_APOLLO3P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#endif
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    .ui8WriteLatency      = 0,
    .bEnWriteLatency      = false,
    .bEmulateDDR          = false,
    .ui16DMATimeLimit     = 0,
    .eDMABoundary         = AM_HAL_MSPI_BOUNDARY_NONE,
#if defined(AM_PART_APOLLO4)
    .eDeviceNum           = AM_HAL_MSPI_DEVICE0,
#endif
#else
    .ui32TCBSize          = 0,
    .pTCB                 = NULL,
    .scramblingStartAddr  = 0,
    .scramblingEndAddr    = 0,
#endif
};

struct
{
    am_hal_mspi_device_e eHalDeviceEnum;
    am_hal_mspi_dev_config_t *psDevConfig;
}g_ATXP128_DevConfig[] =
{
    {AM_HAL_MSPI_FLASH_SERIAL_CE0,                  &MSPI_ATXP128_Serial_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_SERIAL_CE1,                  &MSPI_ATXP128_Serial_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_QUAD_CE0,                    &MSPI_ATXP128_Quad_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_QUAD_CE1,                    &MSPI_ATXP128_Quad_CE1_MSPIConfig},
    {AM_HAL_MSPI_FLASH_OCTAL_CE0,                   &MSPI_ATXP128_Octal_CE0_MSPIConfig},
    {AM_HAL_MSPI_FLASH_OCTAL_CE1,                   &MSPI_ATXP128_Octal_CE1_MSPIConfig},
};

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
//
// SDR timing default setting
//
am_devices_mspi_atxp128_sdr_timing_config_t SDRTimingConfigDefault =
{
    .ui32Turnaround     = 9,
    .ui32Rxneg          = 0,
    .ui32Rxdqsdelay     = 15
};
//
// SDR timing stored setting
//
static bool bSDRTimingConfigSaved = false;
static am_devices_mspi_atxp128_sdr_timing_config_t SDRTimingConfigStored;
#endif

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
#if defined(AM_PART_APOLLO3P) || defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

static const uint32_t ui32MspiXipBaseAddress[3] =
{
    0x14000000, // mspi0
    0x18000000, // mspi1
    0x1C000000, // mspi2
};

//
// Forward declarations.
//
static uint32_t am_devices_mspi_atxp128_command_write(void *pHandle,
                                                      uint8_t ui8Instr,
                                                      bool bSendAddr,
                                                      uint32_t ui32Addr,
                                                      uint32_t *pData,
                                                      uint32_t ui32NumBytes);
static uint32_t am_devices_mspi_atxp128_command_read(void *pHandle,
                                                     uint8_t ui8Instr,
                                                     bool bSendAddr,
                                                     uint32_t ui32Addr,
                                                     uint32_t *pData,
                                                     uint32_t ui32NumBytes);

static bool flash_write(void* flashHandle, uint32_t length) ;

static uint32_t mspi_atxp128_disable_xip(void *pHandle) ;

static bool flash_check(void* flashHandle, uint32_t length) ;

static uint32_t count_consecutive_ones(uint32_t* pVal) ;

static uint32_t find_mid_point(uint32_t* pVal) ;

//*****************************************************************************
//
// Adesto ATXP128 Support
//
//*****************************************************************************
//*****************************************************************************
//
// Device specific initialization function.
//
//*****************************************************************************
static uint32_t
am_device_init_flash(void *pHandle)
{
    uint32_t    ui32PIOBuffer[32] = {0};
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

    //
    // Set the Dummy Cycles in Status/Control register 3 to 8.
    //
    am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    ui32PIOBuffer[0] = 0x00000003;
    am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_ATXP128_WRITE_STATUS_CTRL, false, 0, ui32PIOBuffer, 2);

    //
    // Configure the ATXP128 mode based on the MSPI configuration.
    //
    am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);

    switch ( pFlash->stSetting.eDeviceConfig )
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_ATXP128_RETURN_TO_SPI_MODE, false, 0, ui32PIOBuffer, 0);
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
            am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_ATXP128_ENTER_QUAD_MODE, false, 0, ui32PIOBuffer, 0);
            break;
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_ATXP128_ENTER_OCTAL_MODE, false, 0, ui32PIOBuffer, 0);
            break;
        default:
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Device specific de-initialization function.
//
//*****************************************************************************
static uint32_t
am_device_deinit_flash(void *pHandle)
{
    uint32_t      ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
       return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Configure the Adesto ATXP128 Device mode.
    //
    switch (pFlash->stSetting.eDeviceConfig)
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
            // Nothing to do.  Device defaults to SPI mode.
            break;
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_ATXP128_RETURN_TO_SPI_MODE, false, 0, ui32PIOBuffer, 0);
            if (AM_HAL_STATUS_SUCCESS != ui32Status)
            {
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }
            break;
        default:
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            //break;
    }

    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_DISABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
       return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Generic Command Write function.
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
static uint32_t
am_devices_mspi_atxp128_command_write(void *pHandle, uint8_t ui8Instr, bool bSendAddr,
                                      uint32_t ui32Addr, uint32_t *pData,
                                      uint32_t ui32NumBytes)
{
    uint32_t ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;
    am_hal_mspi_pio_transfer_t  stMSPIFlashPIOTransaction = {0};
    am_hal_mspi_clock_e clkCfg;
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    am_hal_mspi_dqs_t dqsCfg;
#endif

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    // Check if we are trying to send the command at 96MHz.
    if (AM_HAL_MSPI_CLK_96MHZ == pFlash->stSetting.eClockFreq)
    {
      clkCfg = AM_HAL_MSPI_CLK_48MHZ;  // Set the clock to 48MHz for commmands.
      ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_CLOCK_CONFIG, &clkCfg);
      if (AM_HAL_STATUS_SUCCESS != ui32Status)
      {
        return ui32Status;
      }
    }
#endif
    // Create the individual write transaction.
    stMSPIFlashPIOTransaction.ui32NumBytes       = ui32NumBytes;
    stMSPIFlashPIOTransaction.eDirection         = AM_HAL_MSPI_TX;
    stMSPIFlashPIOTransaction.bSendAddr          = bSendAddr;
    stMSPIFlashPIOTransaction.ui32DeviceAddr     = ui32Addr;
    stMSPIFlashPIOTransaction.bSendInstr         = true;
    stMSPIFlashPIOTransaction.ui16DeviceInstr    = ui8Instr;
    stMSPIFlashPIOTransaction.bTurnaround        = false;
#if 0 // MSPI CONT is deprecated for Apollo3
    stMSPIFlashPIOTransaction.bContinue          = false;
#endif
    stMSPIFlashPIOTransaction.pui32Buffer        = pData;

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
#if defined(AM_PART_APOLLO4)
    stMSPIFlashPIOTransaction.eDeviceNum         = AM_HAL_MSPI_DEVICE0;
#endif
    stMSPIFlashPIOTransaction.bDCX               = false;
    stMSPIFlashPIOTransaction.bEnWRLatency       = false;
// #### INTERNAL BEGIN ####
// FALCSW-426 7/29/22 Deprecate MSPI CONT bit. (See also A3DS-25.)
// #### INTERNAL END ####
    stMSPIFlashPIOTransaction.bContinue = false;    // MSPI CONT is deprecated for Apollo4
#endif

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    if ( ui8Instr == AM_DEVICES_ATXP128_WRITE_STATUS_CTRL )
    {
        // Write status/control register command uses 1 byte address
        am_hal_mspi_instr_addr_t sInstAddrCfg;
        sInstAddrCfg.eAddrCfg = AM_HAL_MSPI_ADDR_1_BYTE;
        sInstAddrCfg.eInstrCfg = pFlash->stSetting.eInstrCfg;   // keep instruction setting the same
        am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SET_INSTR_ADDR_LEN, &sInstAddrCfg);
    }

    // do not use enable fine delay for command read
    dqsCfg.bDQSEnable           = false;
    dqsCfg.bDQSSyncNeg          = false;
    dqsCfg.bEnableFineDelay     = false;
    dqsCfg.bOverrideRXDQSDelay  = false;
    dqsCfg.bOverrideTXDQSDelay  = false;
    dqsCfg.bRxNeg               = 0;
    dqsCfg.ui8DQSDelay          = 0;        // not used
    dqsCfg.ui8PioTurnaround     = 4;        // Read command 0x77 and 0x5A requrest 8 dummy bytes (not supported)
    dqsCfg.ui8XipTurnaround     = 4;        // Read command 0x77 and 0x5A requrest 8 dummy bytes (not supported)
    dqsCfg.ui8RxDQSDelay        = 15;       // not used
    dqsCfg.ui8TxDQSDelay        = 0;
    am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_DQS, &dqsCfg);

#endif

    // Execute the transction over MSPI.
    ui32Status = am_hal_mspi_blocking_transfer(pFlash->pMspiHandle, &stMSPIFlashPIOTransaction,
                                         AM_DEVICES_MSPI_ATXP128_TIMEOUT);

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)

    // restore enable fine delay timing settings
    if ( bSDRTimingConfigSaved == true )
    {
        am_devices_mspi_atxp128_apply_sdr_timing(pFlash, &SDRTimingConfigStored);
    }

    // restore the address length setting
    if ( ui8Instr == AM_DEVICES_ATXP128_WRITE_STATUS_CTRL )
    {
        am_hal_mspi_instr_addr_t sInstAddrCfg;
        sInstAddrCfg.eAddrCfg = pFlash->stSetting.eAddrCfg;
        sInstAddrCfg.eInstrCfg = pFlash->stSetting.eInstrCfg;
        am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SET_INSTR_ADDR_LEN, &sInstAddrCfg);
    }
#endif

    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return ui32Status;
    }

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    // Check if we had to step down the command to 48MHz.
    if (AM_HAL_MSPI_CLK_96MHZ == pFlash->stSetting.eClockFreq)
    {
      clkCfg = AM_HAL_MSPI_CLK_96MHZ;  // Reset the clock to 96MHz.
      ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_CLOCK_CONFIG, &clkCfg);
    }
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
static uint32_t
am_devices_mspi_atxp128_command_read(void *pHandle, uint8_t ui8Instr, bool bSendAddr,
                                     uint32_t ui32Addr, uint32_t *pData,
                                     uint32_t ui32NumBytes)
{
    uint32_t ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;
    am_hal_mspi_pio_transfer_t      stMSPIFlashPIOTransaction = {0};
    am_hal_mspi_clock_e clkCfg;
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    am_hal_mspi_dqs_t dqsCfg;
#endif

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    // Check if we are trying to send the command at 96MHz.
    if ((AM_HAL_MSPI_CLK_96MHZ == pFlash->stSetting.eClockFreq)
        && (ui8Instr != AM_DEVICES_ATXP128_ECHO_WITH_INVSERSION))
    {
      clkCfg = AM_HAL_MSPI_CLK_48MHZ;  // Set the clock to 48MHz for commmands.
      ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_CLOCK_CONFIG, &clkCfg);
      if (AM_HAL_STATUS_SUCCESS != ui32Status)
      {
        return ui32Status;
      }
    }
#endif
    // Create the individual write transaction.
    stMSPIFlashPIOTransaction.eDirection         = AM_HAL_MSPI_RX;
    stMSPIFlashPIOTransaction.bSendAddr          = bSendAddr;
    stMSPIFlashPIOTransaction.ui32DeviceAddr     = ui32Addr;
    stMSPIFlashPIOTransaction.bSendInstr         = true;
    stMSPIFlashPIOTransaction.ui16DeviceInstr    = ui8Instr;

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)

    if ( ui8Instr == AM_DEVICES_MSPI_ATXP128_READ_STATUS )
    {
        // Read status/control register command uses 1 byte address
        am_hal_mspi_instr_addr_t sInstAddrCfg;
        sInstAddrCfg.eAddrCfg = AM_HAL_MSPI_ADDR_1_BYTE;
        sInstAddrCfg.eInstrCfg = pFlash->stSetting.eInstrCfg;   // keep instruction setting the same
        am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SET_INSTR_ADDR_LEN, &sInstAddrCfg);
    }
#endif

    if ( (pFlash->stSetting.eDeviceConfig == AM_HAL_MSPI_FLASH_OCTAL_CE0) ||
         (pFlash->stSetting.eDeviceConfig == AM_HAL_MSPI_FLASH_OCTAL_CE1) )
    {
        stMSPIFlashPIOTransaction.bTurnaround    = true;
    }
    else
    {
        stMSPIFlashPIOTransaction.bTurnaround    = false;
    }

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)

    // do not use enable fine delay for command read
    dqsCfg.bDQSEnable           = false;
    dqsCfg.bDQSSyncNeg          = false;
    dqsCfg.bEnableFineDelay     = false;
    dqsCfg.bOverrideRXDQSDelay  = false;
    dqsCfg.bOverrideTXDQSDelay  = false;
    dqsCfg.bRxNeg               = 0;
    dqsCfg.ui8DQSDelay          = 0;        // not used
    dqsCfg.ui8PioTurnaround     = 4;        // Read command 0x77 and 0x5A requrest 8 dummy bytes (not supported) others 4
    dqsCfg.ui8XipTurnaround     = 4;        // Read command 0x77 and 0x5A requrest 8 dummy bytes (not supported) others 4
    dqsCfg.ui8RxDQSDelay        = 15;       // not used
    dqsCfg.ui8TxDQSDelay        = 0;
    am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_DQS, &dqsCfg);
#endif

#if 0 // MSPI CONT is deprecated for Apollo3
    stMSPIFlashPIOTransaction.bContinue          = false;
#endif
    stMSPIFlashPIOTransaction.ui32NumBytes       = ui32NumBytes;
    stMSPIFlashPIOTransaction.pui32Buffer        = pData;

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
#if defined(AM_PART_APOLLO4)
    stMSPIFlashPIOTransaction.eDeviceNum         = AM_HAL_MSPI_DEVICE0;
#endif
    stMSPIFlashPIOTransaction.bDCX               = false;
    stMSPIFlashPIOTransaction.bEnWRLatency       = false;
// #### INTERNAL BEGIN ####
// FALCSW-426 7/29/22 Deprecate MSPI CONT bit. (See also A3DS-25.)
// #### INTERNAL END ####
    stMSPIFlashPIOTransaction.bContinue          = false;   // MSPI CONT is deprecated for Apollo4
#endif

    // Execute the transction over MSPI.
    ui32Status = am_hal_mspi_blocking_transfer(pFlash->pMspiHandle, &stMSPIFlashPIOTransaction,
                                         AM_DEVICES_MSPI_ATXP128_TIMEOUT);

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)

    // restore the address length setting
    if ( ui8Instr == AM_DEVICES_MSPI_ATXP128_READ_STATUS )
    {
        am_hal_mspi_instr_addr_t sInstAddrCfg;
        sInstAddrCfg.eAddrCfg = pFlash->stSetting.eAddrCfg;
        sInstAddrCfg.eInstrCfg = pFlash->stSetting.eInstrCfg;
        am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SET_INSTR_ADDR_LEN, &sInstAddrCfg);
    }

    // restore enable fine delay timing settings
    if ( bSDRTimingConfigSaved == true )
    {
        am_devices_mspi_atxp128_apply_sdr_timing(pFlash, &SDRTimingConfigStored);
    }
#endif

    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return ui32Status;
    }

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    // Check if we had to step down the command to 48MHz.
    if ((AM_HAL_MSPI_CLK_96MHZ == pFlash->stSetting.eClockFreq)
        && (ui8Instr != AM_DEVICES_ATXP128_ECHO_WITH_INVSERSION))
    {
      clkCfg = AM_HAL_MSPI_CLK_96MHZ;  // Reset the clock to 96MHz.
      ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_CLOCK_CONFIG, &clkCfg);
    }
#endif
    return ui32Status;
}

static void
pfnMSPI_ATXP128_Callback(void *pCallbackCtxt, uint32_t status)
{
    // Set the DMA complete flag.
    *(volatile uint32_t *)pCallbackCtxt = status;
}

//*****************************************************************************
//
//  Initialize the mspi_flash driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_init(uint32_t ui32Module,
                             am_devices_mspi_atxp128_config_t *psMSPISettings,
                             void **ppHandle,
                             void **ppMspiHandle)
{
    uint32_t      ui32Status;
    am_hal_mspi_dev_config_t *psConfig = g_ATXP128_DevConfig[0].psDevConfig;
    am_hal_mspi_dev_config_t    tempDevCfg;
    void                     *pMspiHandle;
    uint32_t      ui32Index = 0;

    if ((ui32Module > AM_REG_MSPI_NUM_MODULES) || (psMSPISettings == NULL))
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    // Allocate a vacant device handle
    for ( ui32Index = 0; ui32Index < AM_DEVICES_MSPI_ATXP128_MAX_DEVICE_NUM; ui32Index++ )
    {
        if ( gAmAtxp128[ui32Index].bOccupied == false )
        {
            break;
        }
    }
    if ( ui32Index == AM_DEVICES_MSPI_ATXP128_MAX_DEVICE_NUM )
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    for ( uint32_t i = 0; i < (sizeof(g_ATXP128_DevConfig) / sizeof(g_ATXP128_DevConfig[0])); i++ )
    {
        if ( psMSPISettings->eDeviceConfig == g_ATXP128_DevConfig[i].eHalDeviceEnum )
        {
            psConfig = g_ATXP128_DevConfig[i].psDevConfig;
            psConfig->eClockFreq = psMSPISettings->eClockFreq;
#if !defined(AM_PART_APOLLO4) && !defined(AM_PART_APOLLO4B) && !defined(AM_PART_APOLLO4P)
            psConfig->pTCB = psMSPISettings->pNBTxnBuf;
            psConfig->ui32TCBSize = psMSPISettings->ui32NBTxnBufLength;
            psConfig->scramblingStartAddr = psMSPISettings->ui32ScramblingStartAddr;
            psConfig->scramblingEndAddr = psMSPISettings->ui32ScramblingEndAddr;
#endif
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
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
            gAmAtxp128[ui32Index].stSetting = MSPI_ATXP128_Serial_CE0_MSPIConfig;
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_initialize(ui32Module, &pMspiHandle))
            {
                am_util_debug_printf("Error - Failed to initialize MSPI.\n");
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pMspiHandle, AM_HAL_SYSCTRL_WAKE, false))
            {
                am_util_debug_printf("Error - Failed to power on MSPI.\n");
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }
            tempDevCfg = MSPI_ATXP128_Serial_CE0_MSPIConfig;
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
            {
              am_hal_mspi_config_t    mspiCfg = gMspiCfg;
              mspiCfg.ui32TCBSize = psMSPISettings->ui32NBTxnBufLength;
              mspiCfg.pTCB = psMSPISettings->pNBTxnBuf;
              if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_configure(pMspiHandle, &mspiCfg))
              {
                am_util_debug_printf("Error - Failed to configure MSPI device.\n");
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
              }
            }
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
            // Adjust turnaround for the highest clock speed as part of MSPI SW Workaround.
            if (AM_HAL_MSPI_CLK_96MHZ == MSPI_ATXP128_Serial_CE0_MSPIConfig.eClockFreq)
            {
              tempDevCfg.ui8TurnAround++;
            }
#endif
#endif
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(pMspiHandle, &tempDevCfg))
            {
                am_util_debug_printf("Error - Failed to configure MSPI.\n");
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }

            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(pMspiHandle))
            {
                am_util_debug_printf("Error - Failed to enable MSPI.\n");
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }
            am_bsp_mspi_pins_enable(ui32Module, MSPI_ATXP128_Serial_CE0_MSPIConfig.eDeviceConfig);
            break;
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
            gAmAtxp128[ui32Index].stSetting = MSPI_ATXP128_Serial_CE1_MSPIConfig;
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_initialize(ui32Module, &pMspiHandle))
            {
                am_util_debug_printf("Error - Failed to initialize MSPI.\n");
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }

            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pMspiHandle, AM_HAL_SYSCTRL_WAKE, false))
            {
                am_util_debug_printf("Error - Failed to power on MSPI.\n");
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }
            tempDevCfg = MSPI_ATXP128_Serial_CE1_MSPIConfig;
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
            {
              am_hal_mspi_config_t    mspiCfg = gMspiCfg;
              mspiCfg.ui32TCBSize = psMSPISettings->ui32NBTxnBufLength;
              mspiCfg.pTCB = psMSPISettings->pNBTxnBuf;
              if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_configure(pMspiHandle, &mspiCfg))
              {
                am_util_debug_printf("Error - Failed to configure MSPI device.\n");
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
              }
            }
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
            // Adjust turnaround for the highest clock speed as part of MSPI SW Workaround.
            if (AM_HAL_MSPI_CLK_96MHZ == MSPI_ATXP128_Serial_CE1_MSPIConfig.eClockFreq)
            {
              tempDevCfg.ui8TurnAround++;
            }
#endif
#endif
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_device_configure(pMspiHandle, &tempDevCfg))
            {
                am_util_debug_printf("Error - Failed to configure MSPI.\n");
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }
            if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_enable(pMspiHandle))
            {
                am_util_debug_printf("Error - Failed to enable MSPI.\n");
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }
            am_bsp_mspi_pins_enable(ui32Module, MSPI_ATXP128_Serial_CE1_MSPIConfig.eDeviceConfig);
            break;
        default:
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            //break;
    }

    gAmAtxp128[ui32Index].pMspiHandle = pMspiHandle;
    gAmAtxp128[ui32Index].ui32Module = ui32Module;

    if (AM_HAL_STATUS_SUCCESS != am_devices_mspi_atxp128_reset((void*)&gAmAtxp128[ui32Index]))
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    gAmAtxp128[ui32Index].stSetting = *psConfig;

    //
    // Device specific MSPI Flash initialization.
    //
    ui32Status = am_device_init_flash((void*)&gAmAtxp128[ui32Index]);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
    am_devices_mspi_atxp128_enable_xip((void*)&gAmAtxp128[ui32Index]);
    // Disable MSPI defore re-configuring it
    ui32Status = am_hal_mspi_disable(pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
    // Adjust turnaround for the highest clock speed as part of MSPI SW Workaround.
    am_hal_mspi_dev_config_t psTempConfig = *psConfig;  // We cannot change the static global config.
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    if (AM_HAL_MSPI_CLK_96MHZ == psTempConfig.eClockFreq)
    {
      psTempConfig.ui8TurnAround++;
    }
#endif

    //
    // Re-Configure the MSPI for the requested operation mode.
    //
    ui32Status = am_hal_mspi_device_configure(pMspiHandle, &psTempConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
    // Re-Enable MSPI
    ui32Status = am_hal_mspi_enable(pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
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
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

// #### INTERNAL BEGIN ####
//    TODO: Should register for RXF here instead of relying on application
// #### INTERNAL END ####
//  ui32Status = am_hal_mspi_interrupt_enable(pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR | AM_HAL_MSPI_INT_RX_FIFO_FULL);
    ui32Status = am_hal_mspi_interrupt_enable(pMspiHandle, AM_HAL_MSPI_INT_CQUPD | AM_HAL_MSPI_INT_ERR );
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Return the handle.
    //
    gAmAtxp128[ui32Index].bOccupied = true;
    *ppMspiHandle = pMspiHandle;
    *ppHandle = (void *)&gAmAtxp128[ui32Index];

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//   De-Initialization the mspi_flash driver.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_deinit(void *pHandle)
{
    uint32_t      ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

    //
    // Device specific MSPI Flash de-initialization.
    //
    ui32Status = am_device_deinit_flash(pHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    if (AM_HAL_STATUS_SUCCESS != am_devices_mspi_atxp128_reset(pHandle))
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Disable and clear the interrupts to start with.
    //
    ui32Status = am_hal_mspi_interrupt_disable(pFlash->pMspiHandle, 0xFFFFFFFF);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
    ui32Status = am_hal_mspi_interrupt_clear(pFlash->pMspiHandle, 0xFFFFFFFF);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Disable the MSPI instance.
    //
    ui32Status = am_hal_mspi_disable(pFlash->pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    if (AM_HAL_STATUS_SUCCESS != am_hal_mspi_power_control(pFlash->pMspiHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false))
    {
        am_util_debug_printf("Error - Failed to power on MSPI.\n");
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Deinitialize the MSPI instance.
    //
    ui32Status = am_hal_mspi_deinitialize(pFlash->pMspiHandle);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    // Free this device handle
    pFlash->bOccupied = false;

    //
    // Clear the Flash Caching.
    //
#if !defined(AM_PART_APOLLO4) && !defined(AM_PART_APOLLO4B) && !defined(AM_PART_APOLLO4P)
#if AM_CMSIS_REGS
    CACHECTRL->CACHECFG = 0;
#else // AM_CMSIS_REGS
    AM_REG(CACHECTRL, CACHECFG) = 0;
#endif // AM_CMSIS_REGS
#endif // !AM_PART_APOLLO4
    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Reads the current status of the external flash
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_reset(void *pHandle)
{
    uint32_t      ui32Status;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Enable write.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
       return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Return the device to SPI mode.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_ATXP128_RETURN_TO_SPI_MODE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Disable write.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_DISABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
       return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
//*****************************************************************************
//
//  Test signal integrity and delays on IO lines using echo with inversion command .
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_echo_with_inversion(void *pHandle,
                                            uint8_t pattern,
                                            uint32_t length,
                                            uint8_t *pui8RxBuffer)
{
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

    // Read status/control register command uses 1 byte address
    am_hal_mspi_instr_addr_t sInstAddrCfg;
    sInstAddrCfg.eAddrCfg = AM_HAL_MSPI_ADDR_1_BYTE;
    sInstAddrCfg.eInstrCfg = pFlash->stSetting.eInstrCfg;   // keep instruction setting the same
    am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SET_INSTR_ADDR_LEN, &sInstAddrCfg);

    //
    // Send and read back the echo with inversion using the given patten
    //
    uint32_t ui32Status = am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_ATXP128_ECHO_WITH_INVSERSION, true, pattern, (uint32_t *)pui8RxBuffer, length);

    sInstAddrCfg.eAddrCfg = pFlash->stSetting.eAddrCfg;
    sInstAddrCfg.eInstrCfg = pFlash->stSetting.eInstrCfg;
    am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SET_INSTR_ADDR_LEN, &sInstAddrCfg);

    if ( AM_HAL_STATUS_SUCCESS == ui32Status )
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
    }
    else
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
}
#endif

//*****************************************************************************
//
//   Reads the ID of the external flash and returns the value.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_id(void *pHandle)
{
    uint32_t      ui32Status;
    uint32_t      ui32DeviceID;

    //
    // Send the command sequence to read the Device ID and return status.
    //
    uint8_t       ui8Response[5];
    ui32Status = am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_ID, false, 0, (uint32_t *)&ui8Response[0], 5);
    ui32DeviceID = (ui8Response[0] << 16) | (ui8Response[1] << 8) | ui8Response[2];
    if ( ((ui32DeviceID & AM_DEVICES_MSPI_ATXP128_ID_MASK) == AM_DEVICES_MSPI_ATXP128_ID) &&
       (AM_HAL_STATUS_SUCCESS == ui32Status) )
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
    }
    else
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
}

//*****************************************************************************
//
// Reads the current status of the external flash
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_status(void *pHandle, uint32_t *pStatus)
{
    uint32_t      ui32Status;

    //
    // Send the command sequence to read the device status.
    //
    ui32Status = am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, pStatus, 1);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_read_adv(void *pHandle, uint8_t *pui8RxBuffer,
                                 uint32_t ui32ReadAddress,
                                 uint32_t ui32NumBytes,
                                 uint32_t ui32PauseCondition,
                                 uint32_t ui32StatusSetClr,
                                 am_hal_mspi_callback_t pfnCallback,
                                 void *pCallbackCtxt)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    uint32_t                      ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

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

#if defined(AM_PART_APOLLO4)
    Transaction.eDeviceNum         = AM_HAL_MSPI_DEVICE0;
#endif

    // Check the transaction status.
    ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction,
                                                  AM_HAL_MSPI_TRANS_DMA, pfnCallback, pCallbackCtxt);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Reads the contents of the external flash into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_read(void *pHandle,
                             uint8_t *pui8RxBuffer,
                             uint32_t ui32ReadAddress,
                             uint32_t ui32NumBytes,
                             bool bWaitForCompletion)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    uint32_t                      ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

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

#if defined(AM_PART_APOLLO4)
    Transaction.eDeviceNum         = AM_HAL_MSPI_DEVICE0;
#endif

    if (bWaitForCompletion)
    {
        // Start the transaction.
        volatile uint32_t ui32DMAStatus = 0xFFFFFFFF;
        ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA, pfnMSPI_ATXP128_Callback, (void *)&ui32DMAStatus);

        // Check the transaction status.
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }

        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_ATXP128_TIMEOUT; i++)
        {
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
            if ( (AM_HAL_STATUS_SUCCESS == ui32DMAStatus) || (AM_HAL_MSPI_FIFO_FULL_CONDITION == ui32DMAStatus) )
            {
                break;
            }
#else
            if (AM_HAL_STATUS_SUCCESS == ui32DMAStatus)
            {
                break;
            }
#endif

            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_util_delay_us(1);
        }

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
        if (AM_HAL_MSPI_FIFO_FULL_CONDITION == ui32DMAStatus)
        {
          am_hal_gpio_output_toggle(22);
          return AM_HAL_MSPI_FIFO_FULL_CONDITION;
        }
        else if (AM_HAL_STATUS_SUCCESS == ui32DMAStatus)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
        }
#else
        if (AM_HAL_STATUS_SUCCESS == ui32DMAStatus)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
        }
#endif
        else
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }
    }
    else
    {
        // Check the transaction status.
        ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction,
                                                      AM_HAL_MSPI_TRANS_DMA, NULL, NULL);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param pui8RxBuffer
//! @param ui32ReadAddress
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
static uint32_t
mspi_atxp128_dma_read(void *pHandle, uint8_t *pui8RxBuffer,
                      uint32_t ui32ReadAddress,
                      uint32_t ui32NumBytes)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    uint32_t                      ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

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

#if defined(AM_PART_APOLLO4)
    Transaction.eDeviceNum         = AM_HAL_MSPI_DEVICE0;
#endif

    // Start the transaction.
    volatile uint32_t ui32DMAStatus = 0xFFFFFFFF;
    ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA, pfnMSPI_ATXP128_Callback, (void *)&ui32DMAStatus);

    // Check the transaction status.
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    // Wait for DMA Complete or Timeout
    for (uint32_t i = 0; i < AM_DEVICES_MSPI_ATXP128_TIMEOUT; i++)
    {

        // check DMA status without using ISR
        am_hal_mspi_interrupt_status_get(pFlash->pMspiHandle, &ui32Status, false);
        am_hal_mspi_interrupt_clear(pFlash->pMspiHandle, ui32Status);
        am_hal_mspi_interrupt_service(pFlash->pMspiHandle, ui32Status);

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
        if ( (AM_HAL_STATUS_SUCCESS == ui32DMAStatus) || (AM_HAL_MSPI_FIFO_FULL_CONDITION == ui32DMAStatus) )
        {
            break;
        }
#else
        if (AM_HAL_STATUS_SUCCESS == ui32DMAStatus)
        {
            break;
        }
#endif

        //
        // Call the BOOTROM cycle function to delay for about 1 microsecond.
        //
        am_util_delay_us(1);
    }

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    if (AM_HAL_MSPI_FIFO_FULL_CONDITION == ui32DMAStatus)
    {
        return AM_HAL_MSPI_FIFO_FULL_CONDITION;
    }
    else if (AM_HAL_STATUS_SUCCESS == ui32DMAStatus)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
    }
#else
    if (AM_HAL_STATUS_SUCCESS == ui32DMAStatus)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
    }
#endif
    else
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
}

// ##### INTERNAL BEGIN #####
//*****************************************************************************
//
//   Reads the contents of the external flash into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_read_cb(void *pHandle, uint8_t *pui8RxBuffer,
                                uint32_t ui32ReadAddress,
                                uint32_t ui32NumBytes,
                                am_hal_mspi_callback_t pfnCallback,
                                void *pCallbackCtxt)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    uint32_t                      ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

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

#if defined(AM_PART_APOLLO4)
    Transaction.eDeviceNum         = AM_HAL_MSPI_DEVICE0;
#endif

    // Check the transaction status.
    ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction,
                                                  AM_HAL_MSPI_TRANS_DMA, pfnCallback, pCallbackCtxt);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}
// ##### INTERNAL END #####

//*****************************************************************************
//
//  Reads the contents of the external flash into a buffer.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_read_hiprio(void *pHandle, uint8_t *pui8RxBuffer,
                                    uint32_t ui32ReadAddress,
                                    uint32_t ui32NumBytes,
                                    bool bWaitForCompletion)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    uint32_t                      ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

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

#if defined(AM_PART_APOLLO4)
    Transaction.eDeviceNum         = AM_HAL_MSPI_DEVICE0;
#endif

    if (bWaitForCompletion)
    {
        // Start the transaction.
        volatile bool bDMAComplete = false;
        ui32Status = am_hal_mspi_highprio_transfer(pFlash->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA, pfnMSPI_ATXP128_Callback, (void*)&bDMAComplete);

        // Check the transaction status.
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }

        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_ATXP128_TIMEOUT; i++)
        {
            if (bDMAComplete)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_util_delay_us(1);
        }

        // Check the status.
        if (!bDMAComplete)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }
    }
    else
    {
        // Check the transaction status.
        ui32Status = am_hal_mspi_highprio_transfer(pFlash->pMspiHandle, &Transaction,
                                                      AM_HAL_MSPI_TRANS_DMA, NULL, NULL);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

// ##### INTERNAL BEGIN #####

//*****************************************************************************
//
//
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_read_callback(void *pHandle, uint8_t *pui8RxBuffer,
                                      uint32_t ui32ReadAddress,
                                      uint32_t ui32NumBytes)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    uint32_t                      ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

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

#if defined(AM_PART_APOLLO4)
    Transaction.eDeviceNum         = AM_HAL_MSPI_DEVICE0;
#endif

    // Start the transaction.
    volatile bool bDMAComplete = false;
    ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction,
                                                  AM_HAL_MSPI_TRANS_DMA,
                                                  pfnMSPI_ATXP128_Callback, (void*)&bDMAComplete);

    // Check the transaction status.
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}
// ##### INTERNAL END #####

//*****************************************************************************
//
// Programs the given range of flash addresses.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_write(void *pHandle, uint8_t *pui8TxBuffer,
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
    am_devices_mspi_atxp128_t     *pFlash = (am_devices_mspi_atxp128_t *)pHandle;
    uint32_t                      ui32PIOBuffer[32] = {0};

    while (ui32BytesLeft > 0)
    {
        //
        // Send the command sequence to enable writing.
        //
        ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }

        // Set the DMA priority
        Transaction.ui8Priority = 1;

        // Set the transfer direction to TX (Write)
        Transaction.eDirection = AM_HAL_MSPI_TX;

        if (ui32BytesLeft > AM_DEVICES_MSPI_ATXP128_PAGE_SIZE)
        {
            // Set the transfer count in bytes.
            Transaction.ui32TransferCount = AM_DEVICES_MSPI_ATXP128_PAGE_SIZE;
            ui32BytesLeft -= AM_DEVICES_MSPI_ATXP128_PAGE_SIZE;
        }
        else
        {
            // Set the transfer count in bytes.
            Transaction.ui32TransferCount = ui32BytesLeft;
            ui32BytesLeft = 0;
        }

        // Set the address to read data to.
        Transaction.ui32DeviceAddress = ui32PageAddress;
        ui32PageAddress += AM_DEVICES_MSPI_ATXP128_PAGE_SIZE;

        // Set the source SRAM buffer address.
        Transaction.ui32SRAMAddress = ui32BufferAddress;
        ui32BufferAddress += AM_DEVICES_MSPI_ATXP128_PAGE_SIZE;

        // Clear the CQ stimulus.
        Transaction.ui32PauseCondition = 0;
        // Clear the post-processing
        Transaction.ui32StatusSetClr = 0;

#if defined(AM_PART_APOLLO4)
    Transaction.eDeviceNum         = AM_HAL_MSPI_DEVICE0;
#endif

        // Start the transaction.
        volatile uint32_t ui32DMAStatus = 0xFFFFFFFF;
        ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA, pfnMSPI_ATXP128_Callback, (void*)&ui32DMAStatus);

        // Check the transaction status.
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }

        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_ATXP128_TIMEOUT; i++)
        {
            if (AM_HAL_STATUS_SUCCESS == ui32DMAStatus)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_util_delay_us(1);
        }

        // Check the status.
        if (AM_HAL_STATUS_SUCCESS != ui32DMAStatus)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }

        //
        // Wait for the Write In Progress to indicate the erase is complete.
        //
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_ATXP128_TIMEOUT; i++)
        {
            // ATXP128 has different number of bytes for each speed of status read.
            switch ( pFlash->stSetting.eDeviceConfig )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, ui32PIOBuffer, 2);
                    bWriteComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_ATXP128_WIP));
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                    am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, ui32PIOBuffer, 4);
                    bWriteComplete = (0 == ((ui32PIOBuffer[0] >> 16) & AM_DEVICES_ATXP128_WIP));
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, ui32PIOBuffer, 6);
                    bWriteComplete = (0 == (ui32PIOBuffer[1] & AM_DEVICES_ATXP128_WIP));
                    break;
                default:
                    return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }

            am_util_delay_us(100);
            if (bWriteComplete)
            {
                break;
            }
        }

        //
        // Send the command sequence to disable writing.
        //
        ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_DISABLE, false, 0, ui32PIOBuffer, 0);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }
    }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//! @param pui8TxBuffer
//! @param ui32WriteAddress
//! @param ui32NumBytes
//!
//! @return
//
//*****************************************************************************
static uint32_t
mspi_atxp128_dma_write(void *pHandle, uint8_t *pui8TxBuffer,
                       uint32_t ui32WriteAddress,
                       uint32_t ui32NumBytes)
{
    am_hal_mspi_dma_transfer_t    Transaction;
    bool                          bWriteComplete = false;
    uint32_t                      ui32BytesLeft = ui32NumBytes;
    uint32_t                      ui32PageAddress = ui32WriteAddress;
    uint32_t                      ui32BufferAddress = (uint32_t)pui8TxBuffer;
    uint32_t                      ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    while (ui32BytesLeft > 0)
    {
        //
        // Send the command sequence to enable writing.
        //
        ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }

        // Set the DMA priority
        Transaction.ui8Priority = 1;

        // Set the transfer direction to TX (Write)
        Transaction.eDirection = AM_HAL_MSPI_TX;

        if (ui32BytesLeft > AM_DEVICES_MSPI_ATXP128_PAGE_SIZE)
        {
            // Set the transfer count in bytes.
            Transaction.ui32TransferCount = AM_DEVICES_MSPI_ATXP128_PAGE_SIZE;
            ui32BytesLeft -= AM_DEVICES_MSPI_ATXP128_PAGE_SIZE;
        }
        else
        {
            // Set the transfer count in bytes.
            Transaction.ui32TransferCount = ui32BytesLeft;
            ui32BytesLeft = 0;
        }

        // Set the address to read data to.
        Transaction.ui32DeviceAddress = ui32PageAddress;
        ui32PageAddress += AM_DEVICES_MSPI_ATXP128_PAGE_SIZE;

        // Set the source SRAM buffer address.
        Transaction.ui32SRAMAddress = ui32BufferAddress;
        ui32BufferAddress += AM_DEVICES_MSPI_ATXP128_PAGE_SIZE;

        // Clear the CQ stimulus.
        Transaction.ui32PauseCondition = 0;
        // Clear the post-processing
        Transaction.ui32StatusSetClr = 0;

#if defined(AM_PART_APOLLO4)
    Transaction.eDeviceNum         = AM_HAL_MSPI_DEVICE0;
#endif

        // Start the transaction.
        volatile uint32_t ui32DMAStatus = 0xFFFFFFFF;
        ui32Status = am_hal_mspi_nonblocking_transfer(pFlash->pMspiHandle, &Transaction, AM_HAL_MSPI_TRANS_DMA, pfnMSPI_ATXP128_Callback, (void*)&ui32DMAStatus);

        // Check the transaction status.
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }

        // Wait for DMA Complete or Timeout
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_ATXP128_TIMEOUT; i++)
        {
            // check DMA status without using ISR
            am_hal_mspi_interrupt_status_get(pFlash->pMspiHandle, &ui32Status, false);
            am_hal_mspi_interrupt_clear(pFlash->pMspiHandle, ui32Status);
            am_hal_mspi_interrupt_service(pFlash->pMspiHandle, ui32Status);

            if (AM_HAL_STATUS_SUCCESS == ui32DMAStatus)
            {
                break;
            }
            //
            // Call the BOOTROM cycle function to delay for about 1 microsecond.
            //
            am_util_delay_us(1);
        }

        // Check the status.
        if (AM_HAL_STATUS_SUCCESS != ui32DMAStatus)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }

        //
        // Wait for the Write In Progress to indicate the erase is complete.
        //
        for (uint32_t i = 0; i < AM_DEVICES_MSPI_ATXP128_TIMEOUT; i++)
        {
            // ATXP128 has different number of bytes for each speed of status read.
            switch ( pFlash->stSetting.eDeviceConfig )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, ui32PIOBuffer, 2);
                    bWriteComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_ATXP128_WIP));
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                    am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, ui32PIOBuffer, 4);
                    bWriteComplete = (0 == ((ui32PIOBuffer[0] >> 16) & AM_DEVICES_ATXP128_WIP));
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, ui32PIOBuffer, 6);
                    bWriteComplete = (0 == (ui32PIOBuffer[1] & AM_DEVICES_ATXP128_WIP));
                    break;
                default:
                    return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }

            am_util_delay_us(100);
            if (bWriteComplete)
            {
                break;
            }
        }

        //
        // Send the command sequence to disable writing.
        //
        ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_DISABLE, false, 0, ui32PIOBuffer, 0);
        if (AM_HAL_STATUS_SUCCESS != ui32Status)
        {
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }
    }

  //
  // Return the status.
  //
  return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//   Erases the entire contents of the external flash
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_mass_erase(void *pHandle)
{
    bool          bEraseComplete = false;
    uint32_t      ui32Status;
    //am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Send the command sequence to enable writing.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Send the command sequence to do the mass erase.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_BULK_ERASE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
    //
    // Wait for the Write In Progress to indicate the erase is complete.
    //
    for (uint32_t i = 0; i < AM_DEVICES_MSPI_ATXP128_ERASE_TIMEOUT; i++)
    {
        ui32PIOBuffer[0] = 0;
        am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, ui32PIOBuffer, 1);
        bEraseComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_MSPI_ATXP128_WIP));
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
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Send the command sequence to disable writing.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_DISABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//   Erases the contents of a single sector of flash
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_sector_erase(void *pHandle, uint32_t ui32SectorAddress)
{
    bool          bEraseComplete = false;
    uint32_t      ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Send the command sequence to enable writing.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Send the command to remove protection from the sector.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_ATXP128_UNPROTECT_SECTOR, true, ui32SectorAddress, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Send the command sequence to enable writing.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Send the command sequence to do the sector erase.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_SECTOR_ERASE, true, ui32SectorAddress, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Wait for the Write In Progress to indicate the erase is complete.
    //
    for (uint32_t i = 0; i < AM_DEVICES_MSPI_ATXP128_ERASE_TIMEOUT; i++)
    {
        // ATXP128 has different number of bytes for each speed of status read.
        switch ( pFlash->stSetting.eDeviceConfig )
        {
            case AM_HAL_MSPI_FLASH_SERIAL_CE0:
            case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, ui32PIOBuffer, 2);
                bEraseComplete = (0 == (ui32PIOBuffer[0] & AM_DEVICES_ATXP128_WIP));
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE0:
            case AM_HAL_MSPI_FLASH_QUAD_CE1:
                am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, ui32PIOBuffer, 4);
                bEraseComplete = (0 == ((ui32PIOBuffer[0] >> 16) & AM_DEVICES_ATXP128_WIP));
                break;
            case AM_HAL_MSPI_FLASH_OCTAL_CE0:
            case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                am_devices_mspi_atxp128_command_read(pHandle, AM_DEVICES_MSPI_ATXP128_READ_STATUS_BYTE1, false, 0, ui32PIOBuffer, 6);
                bEraseComplete = (0 == (ui32PIOBuffer[1] & AM_DEVICES_ATXP128_WIP));
                break;
            default:
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
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
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Send the command sequence to disable writing.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_DISABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
    //
    // Return the status.
    //
    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Sets up the MSPI and external FLASH into XIP mode.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_enable_xip(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    //
    // Set Aperture XIP range
    //
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_CONFIG, &gXipConfig[pFlash->ui32Module]);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
#endif

    //
    // Enable XIP on the MSPI.
    //
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_EN, &gXipConfig[pFlash->ui32Module]);
#else
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_EN, NULL);
#endif
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

#if !MSPI_USE_CQ
    // Disable the DMA interrupts.
    ui32Status = am_hal_mspi_interrupt_disable(pFlash->pMspiHandle,
                                               AM_HAL_MSPI_INT_DMAERR |
                                               AM_HAL_MSPI_INT_DMACMP );
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
#endif

    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Removes the MSPI and external FLASH from XIP mode.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_disable_xip(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Send the command to enable writing.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Disable XIP on the MSPI.
    //
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_DIS, &gXipConfig[pFlash->ui32Module]);
#else
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_DIS, NULL);
#endif
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//    Sets up the MSPI and external FLASH into scrambling mode.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_enable_scrambling(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

    //
    // Enable scrambling on the MSPI.
    //
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SCRAMB_EN, &gXipConfig[pFlash->ui32Module]);
#else
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SCRAMB_EN, NULL);
#endif
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//   Removes the MSPI and external FLASH from scrambling mode.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_disable_scrambling(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;
    uint32_t      ui32PIOBuffer[32] = {0};

    //
    // Send the command to enable writing.
    //
    ui32Status = am_devices_mspi_atxp128_command_write(pHandle, AM_DEVICES_MSPI_ATXP128_WRITE_ENABLE, false, 0, ui32PIOBuffer, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Disable Scrambling on the MSPI.
    //
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P)
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SCRAMB_DIS, &gXipConfig[pFlash->ui32Module]);
#else
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_SCRAMB_DIS, NULL);
#endif
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

// #### INTERNAL BEGIN ####
#if 0
//
// Static helper function:
//  Generate the test data patterns.
//  Length has to be multiple of 4bytes
//  returns true if length error, otherwise returns false
//
const uint8_t test_pattern[FLASH_TEST_PATTERN_NUMBER] =
{
    0x55,   // inversion = 0xAA
    0x5A,   // inversion = 0xA5
    0x3C,   // inversion = 0xC3
    0xF0,   // inversion = 0x0F
    0x00    // inversion = 0xFF
};

//
// Static function:
//  verifies the data pattern in the echo-with-inversion way
//
static bool
flash_verify_pattern(uint8_t pattern, uint8_t* buffer, uint32_t length)
{
    uint8_t current_byte;
    for ( uint32_t i = 0; i < length; i++ )
    {
        if (i % 2 )
        {
            // odd, byte = ~pattern
            current_byte = ~pattern;
        }
        else
        {
            // even, byte = pattern
            current_byte = pattern;
        }

        if ( current_byte != buffer[i] )
        {
            return true;
        }
    }

    return false;

}

//
// Static function:
//  checks the timing parameters set for octal communication
//  returns true if verify failure happens, otherwise returns false
//
static bool
flash_check(void* pFlashHandle, uint32_t length)
{
    // Try to use as less ram as possible in stack
    uint32_t ui32NumberOfBytesLeft = length;
    uint32_t ui32TestBytes = 0;
    uint32_t ui32AddressOffset = 0;
    uint8_t ui8PatternCounter = 0;
    uint8_t ui8RxBuffer[FLASH_CHECK_DATA_SIZE_BYTES] = {0};

    while ( ui32NumberOfBytesLeft )
    {
        if ( ui32NumberOfBytesLeft > FLASH_CHECK_DATA_SIZE_BYTES )
        {
            ui32TestBytes = FLASH_CHECK_DATA_SIZE_BYTES;
            ui32NumberOfBytesLeft -= FLASH_CHECK_DATA_SIZE_BYTES;
        }
        else
        {
            ui32TestBytes = ui32NumberOfBytesLeft;
            ui32NumberOfBytesLeft = 0;
        }

        //
        // Test using echo with inversion
        // Note: we cannot use this for read array command timing scan
        // This command takes 4 dummy bytes but read array command takes 8
        //
        am_devices_mspi_atxp128_echo_with_inversion(pFlashHandle,
                                                    test_pattern[ui8PatternCounter%FLASH_TEST_PATTERN_NUMBER],
                                                    ui32TestBytes,    // size has to be lower than AM_HAL_MSPI_MAX_PIO_TRANS_SIZE
                                                    ui8RxBuffer);

        //
        // Verify the result
        //
        if ( flash_verify_pattern(test_pattern[ui8PatternCounter%FLASH_TEST_PATTERN_NUMBER], ui8RxBuffer, ui32TestBytes) )
        {
//            am_util_debug_printf("    Failed to verify at offset 0x%08x!\n", ui32AddressOffset);
            // verify failed, return directly
          return true;
        }
        ui32AddressOffset += ui32TestBytes;
        ui8PatternCounter++;
    }

    return false;
}
#endif // use echo-with-inversion
// #### INTERNAL END ####

//*****************************************************************************
//
//! @brief
//!
//! @param pattern_index
//! @param buff
//! @param len
//!
//! @return
//
//*****************************************************************************
static int prepare_test_pattern(uint32_t pattern_index, uint8_t* buff, uint32_t len)
{
    uint32_t *pui32TxPtr = (uint32_t*)buff;
    uint8_t  *pui8TxPtr  = (uint8_t*)buff;

    // length has to be multiple of 4 bytes
    if ( len % 4 )
    {
        return -1;
    }

    switch ( pattern_index )
    {
        case 0:
            // 0x5555AAAA
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0x5555AAAA);
            }
            break;
        case 1:
            // 0xFFFF0000
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0xFFFF0000);
            }
            break;
        case 2:
            // walking
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = 0x01 << (i % 8);
            }
            break;
        case 3:
            // incremental from 1
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i + 1) & 0xFF);
            }
            break;
        case 4:
            // decremental from 0xff
            for ( uint32_t i = 0; i < len; i++ )
            {
                // decrement starting from 0xff
                pui8TxPtr[i] = (0xff - i) & 0xFF;
            }
            break;
        default:
            // incremental from 1
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i + 1) & 0xFF);
            }
            break;

    }

    return 0;
}

//*****************************************************************************
//
//! @brief
//! @param flashHandle
//! @param length
//! @return bool false on success
//
//*****************************************************************************
static bool
flash_write(void* flashHandle, uint32_t length)
{
    // Try to use as less ram as possible in stack
    uint32_t ui32NumberOfBytesLeft = length;
    uint32_t ui32TestBytes = 0;
    uint32_t ui32AddressOffset = 0;
    uint8_t  ui8PatternCounter = 0;
    uint8_t  ui8TxBuffer[FLASH_CHECK_DATA_SIZE_BYTES] AM_BIT_ALIGNED(128);
    uint32_t ui32Status = AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;

    while ( ui32NumberOfBytesLeft )
    {
        if ( ui32NumberOfBytesLeft > FLASH_CHECK_DATA_SIZE_BYTES )
        {
            ui32TestBytes = FLASH_CHECK_DATA_SIZE_BYTES;
            ui32NumberOfBytesLeft -= FLASH_CHECK_DATA_SIZE_BYTES;
        }
        else
        {
            ui32TestBytes = ui32NumberOfBytesLeft;
            ui32NumberOfBytesLeft = 0;
        }

        //
        // Write to target address with test pattern with given length
        // Use 5 patterns: 0x5555AAAA, 0xFFFF0000, Walking, incremental and decremental
        //

        prepare_test_pattern((ui8PatternCounter) % FLASH_TEST_PATTERN_NUMBER, ui8TxBuffer, ui32TestBytes);
        ui8PatternCounter++;

        // write to target address
        ui32Status = mspi_atxp128_dma_write(flashHandle, ui8TxBuffer,
                                            (AM_DEVICES_MSPI_ATXP128_SECTOR_FOR_TIMING_CHECK << 16) + ui32AddressOffset,
                                            ui32TestBytes);
        if ( ui32Status ==  AM_DEVICES_MSPI_ATXP128_STATUS_ERROR)
        {
            return true;
        }

        ui32AddressOffset += ui32TestBytes;
    }

    return false;
}

//*****************************************************************************
//
//! @brief disable XIP
//! @param pHandle  - Pointer to device driver handle
//! @return status from am_devices_mspi_atxp128_status_t
//
//*****************************************************************************
static uint32_t
mspi_atxp128_disable_xip(void *pHandle)
{
    uint32_t ui32Status;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;

    //
    // Disable XIP on the MSPI.
    //
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_DIS, &gXipConfig[pFlash->ui32Module]);
#else
    ui32Status = am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_XIP_DIS, NULL);
#endif
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//! @param flashHandle
//! @param length
//! @return
//
//*****************************************************************************
static bool
flash_check(void* flashHandle, uint32_t length)
{
    // Try to use as less ram as possible in stack
    uint32_t ui32NumberOfBytesLeft = length;
    uint32_t ui32TestBytes = 0;
    uint32_t ui32AddressOffset = 0;
    uint8_t  ui8PatternCounter = 0;
    uint8_t  ui8TxBuffer[FLASH_CHECK_DATA_SIZE_BYTES] AM_BIT_ALIGNED(128);
    uint8_t  ui8RxBuffer[FLASH_CHECK_DATA_SIZE_BYTES] AM_BIT_ALIGNED(128);
    uint32_t ui32Status = AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)flashHandle;

    uint32_t module = pFlash->ui32Module;

    while ( ui32NumberOfBytesLeft )
    {
        if ( ui32NumberOfBytesLeft > FLASH_CHECK_DATA_SIZE_BYTES )
        {
            ui32TestBytes = FLASH_CHECK_DATA_SIZE_BYTES;
            ui32NumberOfBytesLeft -= FLASH_CHECK_DATA_SIZE_BYTES;
        }
        else
        {
            ui32TestBytes = ui32NumberOfBytesLeft;
            ui32NumberOfBytesLeft = 0;
        }

        //
        // Write to target address with test pattern with given length
        // Use 5 patterns: 0x5555AAAA, 0xFFFF0000, Walking, incremental and decremental
        //
        prepare_test_pattern((ui8PatternCounter) % FLASH_TEST_PATTERN_NUMBER, ui8TxBuffer, ui32TestBytes);
        ui8PatternCounter++;

        //
        // Read back data
        //
#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
        if ( pFlash->stSetting.eClockFreq == AM_HAL_MSPI_CLK_96MHZ )
        {
            //
            // Read the data back into the RX buffer using XIP
            //
            ui32Status = am_devices_mspi_atxp128_enable_xip(flashHandle);
            if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
            {
                am_util_debug_printf("Failed to put the MSPI into XIP mode!\n");
            }
            am_hal_daxi_control(AM_HAL_DAXI_CONTROL_FLUSH, NULL);
            am_hal_daxi_control(AM_HAL_DAXI_CONTROL_INVALIDATE, NULL);
            uint8_t * xipPointer = (uint8_t *)(ui32MspiXipBaseAddress[module] + (AM_DEVICES_MSPI_ATXP128_SECTOR_FOR_TIMING_CHECK << 16) + ui32AddressOffset);
            memcpy((uint8_t*)ui8RxBuffer, xipPointer, ui32TestBytes);

            //
            // Quit XIP mode
            //
            ui32Status = mspi_atxp128_disable_xip(flashHandle);
            if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
            {
                am_util_debug_printf("Failed to disable XIP mode in the MSPI!\n");
            }
        }
        else
        {
            ui32Status = mspi_atxp128_dma_read(flashHandle, ui8RxBuffer,
                                                (AM_DEVICES_MSPI_ATXP128_SECTOR_FOR_TIMING_CHECK << 16) + ui32AddressOffset,
                                                ui32TestBytes);
        }
#else
        ui32Status = mspi_atxp128_dma_read(flashHandle, ui8RxBuffer,
                                            (AM_DEVICES_MSPI_ATXP128_SECTOR_FOR_TIMING_CHECK << 16) + ui32AddressOffset,
                                            ui32TestBytes);
#endif

        if ( ui32Status ==  AM_DEVICES_MSPI_ATXP128_STATUS_ERROR)
        {
            return true;
        }

        //
        // Verify the result
        //
        if ( memcmp(ui8RxBuffer, ui8TxBuffer, ui32TestBytes) )
        {
//            am_util_debug_printf("    Failed to verify at offset 0x%08x!\n", ui32AddressOffset);
            // verify failed, return directly
            return true;
        }

        ui32AddressOffset += ui32TestBytes;
    }

    return false;
}
//*****************************************************************************
//
//! @brief Count the longest consecutive 1s in a 32bit word
//! @param pVal
//! @return
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
//! @brief Find and return the mid point of the longest continuous 1s in a 32bit word
//! @param pVal
//! @return
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
                if ( pattern_len > max_len )
                {
                    max_len = pattern_len;
                    pick_point = i - 1 - pattern_len / 2;
                    remainder = pattern_len % 2;
                }
                pattern_len = 0;
            }
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
        // window is likely on low side
        pick_point = pick_point - remainder;    // minus only when pattern length is odd
    }
    else if ( (pick_point > 15) && (val & 0x40000000) )
    {
        // window is likely on high side
        pick_point = pick_point + 1;
    }
    else
    {
        // window is in the middle, no action
    }

    return pick_point;
}

#if defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
const am_devices_mspi_atxp128_sdr_timing_config_t atxp128_sConfigArray[8] =
{
    {7 , 0, 1}, // Turnaround=7 , RXNEG=0, RXDQSDELAY=Dummy
    {7 , 1, 1}, // Turnaround=7 , RXNEG=1, RXDQSDELAY=Dummy
    {8 , 0, 1}, // Turnaround=8 , RXNEG=0, RXDQSDELAY=Dummy
    {8 , 1, 1}, // Turnaround=8 , RXNEG=1, RXDQSDELAY=Dummy
    {9 , 0, 1}, // Turnaround=9 , RXNEG=0, RXDQSDELAY=Dummy
    {9 , 1, 1}, // Turnaround=9 , RXNEG=1, RXDQSDELAY=Dummy
    {10, 0, 1}, // Turnaround=10, RXNEG=0, RXDQSDELAY=Dummy
    {10, 1, 1}, // Turnaround=10, RXNEG=1, RXDQSDELAY=Dummy
};

//*****************************************************************************
//
// Checks PSRAM timing and determine a delay setting
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_sdr_init_timing_check(uint32_t module,
                                              am_devices_mspi_atxp128_config_t *pDevCfg,
                                              am_devices_mspi_atxp128_sdr_timing_config_t *pDevSdrCfg)
{
    uint32_t ui32Status;
    void *pDevHandle;
    void *pHandle;
    uint32_t ui32ResultArray[8] =
    {
        0,  // Turnaround = 7 , RXNEG = 0
        0,  // Turnaround = 7 , RXNEG = 1
        0,  // Turnaround = 8 , RXNEG = 0
        0,  // Turnaround = 8 , RXNEG = 1
        0,  // Turnaround = 9 , RXNEG = 0
        0,  // Turnaround = 9 , RXNEG = 1
        0,  // Turnaround = 10, RXNEG = 0
        0,  // Turnaround = 10, RXNEG = 1
    };

    am_hal_mspi_dqs_t scanCfg =
    {
        .bDQSEnable             = 0,
        .bEnableFineDelay       = 1,
        .bOverrideRXDQSDelay    = 1,
        .ui8RxDQSDelay          = 15,
        .bOverrideTXDQSDelay    = 0,
        .ui8TxDQSDelay          = 0,
        .bDQSSyncNeg            = 0,
        .ui8DQSDelay            = 0,
        .ui8PioTurnaround       = 8,
        .ui8XipTurnaround       = 8,
        .bRxNeg                 = 0,
    };

    // clear previous saved config, rescan
    if ( bSDRTimingConfigSaved == true )
    {
        bSDRTimingConfigSaved                   = false;
        SDRTimingConfigStored.ui32Rxdqsdelay    = SDRTimingConfigDefault.ui32Rxdqsdelay;
        SDRTimingConfigStored.ui32Rxneg         = SDRTimingConfigDefault.ui32Rxneg;
        SDRTimingConfigStored.ui32Turnaround    = SDRTimingConfigDefault.ui32Turnaround;
    }

    //
    // initialize interface
    //
    ui32Status = am_devices_mspi_atxp128_init(module, pDevCfg, &pDevHandle, &pHandle);
    if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
    {
        am_util_debug_printf("    Failed to configure the MSPI and Flash Device correctly!\n");
        return ui32Status;
    }

    //
    // erase target sector first (each "sector is 64Kbyte block")
    //
    if ( FLASH_TIMING_SCAN_SIZE_BYTES % AM_DEVICES_MSPI_ATXP128_SECTOR_SIZE )
    {
        // scan size shall be at block boundary
        am_util_debug_printf("ERROR: Timing scan data size shall be at sector boundary!\n");
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
    for ( uint8_t i = 0; i < (FLASH_TIMING_SCAN_SIZE_BYTES / AM_DEVICES_MSPI_ATXP128_SECTOR_SIZE); i++ )
    {
        ui32Status = am_devices_mspi_atxp128_sector_erase(pDevHandle,
                                                        (AM_DEVICES_MSPI_ATXP128_SECTOR_FOR_TIMING_CHECK << 16) + i*AM_DEVICES_MSPI_ATXP128_SECTOR_SIZE);

        if (AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS != ui32Status)
        {
            am_util_debug_printf("Failed to erase Flash Device sector!\n");
            return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
        }
    }

    // write test pattern into target sector
    if ( flash_write(pDevHandle, FLASH_TIMING_SCAN_SIZE_BYTES) )
    {
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }

    //
    // Start scan loop
    //
    for ( uint8_t i = 0; i < 8; i++ )
    {
        // set Turnaround and RXNEG
        scanCfg.ui8PioTurnaround    = scanCfg.ui8XipTurnaround = atxp128_sConfigArray[i].ui32Turnaround;
        scanCfg.bRxNeg              = atxp128_sConfigArray[i].ui32Rxneg;
        for ( uint8_t RxDqs_Index = 1; RxDqs_Index < 31; RxDqs_Index++ )
        {
            // set RXDQSDELAY0 value
            scanCfg.ui8RxDQSDelay   = RxDqs_Index;
            // apply settings
            ui32Status = am_hal_mspi_control(pHandle, AM_HAL_MSPI_REQ_DQS, &scanCfg);
            if (AM_HAL_STATUS_SUCCESS != ui32Status)
            {
                return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
            }

            // run data check
            if ( false == flash_check(pDevHandle, FLASH_TIMING_SCAN_SIZE_BYTES) )
            {
                // data check pass
                ui32ResultArray[i] |= 0x01 << RxDqs_Index;
            }
            else
            {
                // data check failed
            }
        }
    }

    //
    // Check result
    //
    uint32_t ui32MaxOnesIndex = 0;
    uint32_t ui32MaxOnes = 0;
    uint32_t ui32Result = 0;
    for ( uint32_t i = 0; i < 8; i++ )
    {
        ui32Result = count_consecutive_ones(&ui32ResultArray[i]);
        if ( ui32Result > ui32MaxOnes )
        {
            ui32MaxOnes = ui32Result;
            ui32MaxOnesIndex = i;
        }

        //
        // print result for test
        //
        am_util_debug_printf("    Setting %d = 0x%08X\n", i, ui32ResultArray[i]);
    }
    am_util_debug_printf("Timing Scan found a window %d fine steps wide.\n", ui32MaxOnes);

    //
    // Find RXDQSDELAY Value
    //
    uint32_t dqsdelay = find_mid_point(&ui32ResultArray[ui32MaxOnesIndex]);

    //
    // Deinitialize the MSPI interface
    //
    am_devices_mspi_atxp128_deinit(pDevHandle);
    NVIC_ClearPendingIRQ(mspi_interrupts[module]);

    //
    // Check consecutive passing settings
    //
    if ( ui32MaxOnes < ATXP128_TIMING_SCAN_MIN_ACCEPTANCE_LENGTH )
    {
        // too short is the passing settings, use default setting
        pDevSdrCfg->ui32Rxdqsdelay = SDRTimingConfigDefault.ui32Rxdqsdelay;
        pDevSdrCfg->ui32Rxneg = SDRTimingConfigDefault.ui32Rxneg;
        pDevSdrCfg->ui32Turnaround = SDRTimingConfigDefault.ui32Turnaround;
        return AM_DEVICES_MSPI_ATXP128_STATUS_ERROR;
    }
    else
    {
        //
        // Set output values
        //
        pDevSdrCfg->ui32Rxdqsdelay = dqsdelay;
        pDevSdrCfg->ui32Rxneg = atxp128_sConfigArray[ui32MaxOnesIndex].ui32Rxneg;
        pDevSdrCfg->ui32Turnaround = atxp128_sConfigArray[ui32MaxOnesIndex].ui32Turnaround;

        return AM_DEVICES_MSPI_ATXP128_STATUS_SUCCESS;
    }
}

//*****************************************************************************
//
//  Apply given SDR timing settings to target MSPI instance.
//
//*****************************************************************************
uint32_t
am_devices_mspi_atxp128_apply_sdr_timing(void *pHandle,
                                         am_devices_mspi_atxp128_sdr_timing_config_t *pDevSdrCfg)
{
    am_devices_mspi_atxp128_t *pFlash = (am_devices_mspi_atxp128_t *)pHandle;
    am_hal_mspi_dqs_t applyCfg =
    {
        .bDQSEnable             = 0,
        .bEnableFineDelay       = 1,
        .bOverrideRXDQSDelay    = 1,
        .bOverrideTXDQSDelay    = 0,
        .ui8TxDQSDelay          = 0,
        .bDQSSyncNeg            = 0,
        .ui8DQSDelay            = 0,
    };

    // apply timing settings: Turnaround, RXNEG and RXDQSDELAY
    applyCfg.ui8RxDQSDelay      = pDevSdrCfg->ui32Rxdqsdelay;
    applyCfg.ui8PioTurnaround   = pDevSdrCfg->ui32Turnaround;
    applyCfg.ui8XipTurnaround   = pDevSdrCfg->ui32Turnaround;
    applyCfg.bRxNeg             = pDevSdrCfg->ui32Rxneg;

    // save a local copy of the timing settings
    if ( bSDRTimingConfigSaved == false )
    {
        bSDRTimingConfigSaved                   = true;
        SDRTimingConfigStored.ui32Rxdqsdelay    = pDevSdrCfg->ui32Rxdqsdelay;
        SDRTimingConfigStored.ui32Rxneg         = pDevSdrCfg->ui32Rxneg;
        SDRTimingConfigStored.ui32Turnaround    = pDevSdrCfg->ui32Turnaround;
    }

    return am_hal_mspi_control(pFlash->pMspiHandle, AM_HAL_MSPI_REQ_DQS, &applyCfg);

}
#endif

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

