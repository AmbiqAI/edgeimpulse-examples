//*****************************************************************************
//
//! @file pwr_iom_utils.c
//!
//! @brief Utilities used to setup IOM pins
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup pwr_iom_spi SPI Power Example
//! @ingroup power_examples
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
#include "pwr_iom_utils.h"
#include "am_bsp.h"
#include "pwr_gpio_utils.h"

#define MAX_DEBUG_PINS 3

//
//! pin config for GPIO debug output pins
//
static const am_hal_gpio_pincfg_t g_AM_GPIO_OFF =
    {
        .GP.cfg_b.uFuncSel         = 3,
        .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
        .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
        .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
        .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_NONE,
        .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
        .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_NONE,
        .GP.cfg_b.uSlewRate        = 0,
        .GP.cfg_b.uNCE             = 0,
        .GP.cfg_b.eCEpol           = 0,
        .GP.cfg_b.ePowerSw         = 0,
        .GP.cfg_b.eForceInputEn    = 0,
        .GP.cfg_b.eForceOutputEn   = 0,
        .GP.cfg_b.uRsvd_0          = 0,
        .GP.cfg_b.uRsvd_1          = 0,
    };

//
//! pin config for GPIO debug output pins
//
static const am_hal_gpio_pincfg_t g_AM_GPIO_OUTPUT_CONFIG =
{
    .GP.cfg_b.uFuncSel         = 3,
    .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
    .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X,
    .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.uSlewRate        = 0,
    .GP.cfg_b.uNCE             = 0,
    .GP.cfg_b.eCEpol           = 0,
    .GP.cfg_b.ePowerSw         = 0,
    .GP.cfg_b.eForceInputEn    = 0,
    .GP.cfg_b.eForceOutputEn   = 0,
    .GP.cfg_b.uRsvd_0          = 0,
    .GP.cfg_b.uRsvd_1          = 0,
};


typedef struct
{
    uint32_t pinNumber;
    am_hal_gpio_pincfg_t *pinCfg;
} pin_defs_t;

typedef struct
{
    pin_defs_t sck;
    pin_defs_t miso;
    pin_defs_t mosi;
    pin_defs_t cs;
    uint32_t  ui32DebugPin[MAX_DEBUG_PINS];
} iom_defs_t;


const iom_defs_t g_iomDefsTable[8] =
{
    [0] =
    {
        .sck.pinNumber = AM_BSP_GPIO_IOM0_SCK,   // 5
        .sck.pinCfg = &g_AM_BSP_GPIO_IOM0_SCK,
        .miso.pinNumber = AM_BSP_GPIO_IOM0_MISO,  // 7
        .miso.pinCfg = &g_AM_BSP_GPIO_IOM0_MISO,
        .mosi.pinNumber = AM_BSP_GPIO_IOM0_MOSI,  // 6
        .mosi.pinCfg = &g_AM_BSP_GPIO_IOM0_MOSI,
        .cs.pinNumber   = AM_BSP_GPIO_IOM0_CS,  // 50
        .cs.pinCfg = &g_AM_BSP_GPIO_IOM0_CS,
        .ui32DebugPin[eDEBUG_PIN_RD_LOW_WRT_HI] = 0,
        .ui32DebugPin[eDEBUG_PIN_2] = 3,
        .ui32DebugPin[eDEBUG_PIN_SLEEP] = 2
    },
    [1] =
    {
        .sck.pinNumber = AM_BSP_GPIO_IOM1_SCK,  //  8
        .sck.pinCfg = &g_AM_BSP_GPIO_IOM1_SCK,
        .miso.pinNumber = AM_BSP_GPIO_IOM1_MISO,  // 10
        .miso.pinCfg = &g_AM_BSP_GPIO_IOM1_MISO,
        .mosi.pinNumber = AM_BSP_GPIO_IOM1_MOSI,  // 9
        .mosi.pinCfg = &g_AM_BSP_GPIO_IOM1_MOSI,
        .cs.pinNumber   = AM_BSP_GPIO_IOM1_CS,  // 51
        .cs.pinCfg = &g_AM_BSP_GPIO_IOM1_CS,
        .ui32DebugPin[eDEBUG_PIN_RD_LOW_WRT_HI] = 0,
        .ui32DebugPin[eDEBUG_PIN_2] = 3,
        .ui32DebugPin[eDEBUG_PIN_SLEEP] = 2
    },
    [2] =
    {
        .sck.pinNumber = AM_BSP_GPIO_IOM2_SCK,  // 25
        .sck.pinCfg = &g_AM_BSP_GPIO_IOM2_SCK,
        .miso.pinNumber = AM_BSP_GPIO_IOM2_MISO,  // 27
        .miso.pinCfg = &g_AM_BSP_GPIO_IOM2_MISO,
        .mosi.pinNumber = AM_BSP_GPIO_IOM2_MOSI,  // 26
        .mosi.pinCfg = &g_AM_BSP_GPIO_IOM2_MOSI,
        .cs.pinNumber   = AM_BSP_GPIO_IOM2_CS,  // 52
        .cs.pinCfg = &g_AM_BSP_GPIO_IOM2_CS,
        .ui32DebugPin[eDEBUG_PIN_RD_LOW_WRT_HI] = 0,
        .ui32DebugPin[eDEBUG_PIN_2] = 3,
        .ui32DebugPin[eDEBUG_PIN_SLEEP] = 2
    },
    [3] =
    {
        .sck.pinNumber = AM_BSP_GPIO_IOM3_SCK,  // 31
        .sck.pinCfg = &g_AM_BSP_GPIO_IOM3_SCK,
        .miso.pinNumber = AM_BSP_GPIO_IOM3_MISO,  // 33
        .miso.pinCfg = &g_AM_BSP_GPIO_IOM3_MISO,
        .mosi.pinNumber = AM_BSP_GPIO_IOM3_MOSI,  // 32
        .mosi.pinCfg = &g_AM_BSP_GPIO_IOM3_MOSI,
        .cs.pinNumber   = AM_BSP_GPIO_IOM3_CS,  // 90
        .cs.pinCfg = &g_AM_BSP_GPIO_IOM3_CS,
        .ui32DebugPin[eDEBUG_PIN_RD_LOW_WRT_HI] = 0,
        .ui32DebugPin[eDEBUG_PIN_2] = 3,
        .ui32DebugPin[eDEBUG_PIN_SLEEP] = 2
    },
    [4] =
    {
        .sck.pinNumber = AM_BSP_GPIO_IOM4_SCK,  // 34
        .sck.pinCfg = &g_AM_BSP_GPIO_IOM4_SCK,
        .miso.pinNumber = AM_BSP_GPIO_IOM4_MISO,  // 36
        .miso.pinCfg = &g_AM_BSP_GPIO_IOM4_MISO,
        .mosi.pinNumber = AM_BSP_GPIO_IOM4_MOSI,  // 35
        .mosi.pinCfg = &g_AM_BSP_GPIO_IOM4_MOSI,
        .cs.pinNumber   = AM_BSP_GPIO_IOM4_CS,  // 54
        .cs.pinCfg = &g_AM_BSP_GPIO_IOM4_CS,
        .ui32DebugPin[eDEBUG_PIN_RD_LOW_WRT_HI] = 0,
        .ui32DebugPin[eDEBUG_PIN_2] = 3,
        .ui32DebugPin[eDEBUG_PIN_SLEEP] = 2
    },
    [5] =
    {
        .sck.pinNumber = AM_BSP_GPIO_IOM5_SCK,  // 47
        .sck.pinCfg = &g_AM_BSP_GPIO_IOM5_SCK,
        .miso.pinNumber = AM_BSP_GPIO_IOM5_MISO,  // 49
        .miso.pinCfg = &g_AM_BSP_GPIO_IOM5_MISO,
        .mosi.pinNumber = AM_BSP_GPIO_IOM5_MOSI,  // 48
        .mosi.pinCfg = &g_AM_BSP_GPIO_IOM5_MOSI,
        .cs.pinNumber   = AM_BSP_GPIO_IOM5_CS,  // 92
        .cs.pinCfg = &g_AM_BSP_GPIO_IOM5_CS,
        .ui32DebugPin[eDEBUG_PIN_RD_LOW_WRT_HI] = 0,
        .ui32DebugPin[eDEBUG_PIN_2] = 3,
        .ui32DebugPin[eDEBUG_PIN_SLEEP] = 2
    },
    [6] =
    {
        .sck.pinNumber = AM_BSP_GPIO_IOM6_SCK,  //  61
        .sck.pinCfg = &g_AM_BSP_GPIO_IOM6_SCK,
        .miso.pinNumber = AM_BSP_GPIO_IOM6_MISO,  //  63
        .miso.pinCfg = &g_AM_BSP_GPIO_IOM6_MISO,
        .mosi.pinNumber = AM_BSP_GPIO_IOM6_MOSI,  //  62
        .mosi.pinCfg = &g_AM_BSP_GPIO_IOM6_MOSI,
        .cs.pinNumber   = AM_BSP_GPIO_IOM6_CS,  // 56
        .cs.pinCfg = &g_AM_BSP_GPIO_IOM6_CS,
        .ui32DebugPin[eDEBUG_PIN_RD_LOW_WRT_HI] = 0,
        .ui32DebugPin[eDEBUG_PIN_2] = 3,
        .ui32DebugPin[eDEBUG_PIN_SLEEP] = 2
    },
    [7] =
    {
        .sck.pinNumber = AM_BSP_GPIO_IOM7_SCK,  // 22
        .sck.pinCfg = &g_AM_BSP_GPIO_IOM7_SCK,
        .miso.pinNumber = AM_BSP_GPIO_IOM7_MISO,  // 24
        .miso.pinCfg = &g_AM_BSP_GPIO_IOM7_MISO,
        .mosi.pinNumber = AM_BSP_GPIO_IOM7_MOSI,  // 23
        .mosi.pinCfg = &g_AM_BSP_GPIO_IOM7_MOSI,
        .cs.pinNumber   = 64, //AM_BSP_GPIO_IOM7_CS,  // 94
        .cs.pinCfg = &g_AM_BSP_GPIO_IOM7_CS,
        .ui32DebugPin[eDEBUG_PIN_RD_LOW_WRT_HI] = 0,
        .ui32DebugPin[eDEBUG_PIN_2] = 3,
        .ui32DebugPin[eDEBUG_PIN_SLEEP] = 2
    },

};

static uint32_t pwr_iom_MOSI_MISO_setup(const pin_defs_t  *ePinDefs,
                                        iom_pin_out_mode_e ePinMode);

//*****************************************************************************
//
//! @brief enable/disable an IOM pin
//!
//! @param ePinDefs  definitions for this pin from  iomDefsTable
//! @param ePinMode pin mode selected by calling program
//!
//! @return standard HAL status
//
//*****************************************************************************
static uint32_t
pwr_iom_MOSI_MISO_setup( const pin_defs_t  *ePinDefs, iom_pin_out_mode_e ePinMode )
{
    am_hal_gpio_pincfg_t pinCfg;
    switch(ePinMode)
    {
        case eIOM_PIN_OFF:
            pinCfg = g_AM_GPIO_OFF;
            break;
        case eIOM_PIN_NOMRAL:
            pinCfg = *ePinDefs->pinCfg;
            break;
        case eIOM_PIN_GPIO_LO:
        case eIOM_PIN_GPIO_HI:
            pinCfg = g_AM_GPIO_OUTPUT_CONFIG;
            break;
    }

    uint32_t ui32PinNUm = ePinDefs->pinNumber;
    uint32_t ui32Stat = am_hal_gpio_pinconfig( ui32PinNUm , pinCfg);
    if (ui32Stat)
    {
        return ui32Stat;
    }

    switch( ePinMode)
    {
        case eIOM_PIN_OFF:
        case eIOM_PIN_NOMRAL:
            //
            // nothing more to do here, the iom will control the pin
            //
            break;
        case eIOM_PIN_GPIO_LO:
            am_hal_gpio_output_clear(ui32PinNUm);
            break;
        case eIOM_PIN_GPIO_HI:
            am_hal_gpio_output_set(ui32PinNUm);
            break;
    }

    return AM_HAL_STATUS_SUCCESS;

}

//*****************************************************************************
//
// Init the IOM pins depending based on pin config preferences
//
//*****************************************************************************
uint32_t
pwr_iom_gpio_setup (const iom_pins_setup_t *psIOM_cfg)
{
    uint32_t ui32IOM_num = psIOM_cfg->ui32Iom_number;

    //
    // get the pin usage map from the table above for each specific IOM
    //
    const iom_defs_t *piomd = &g_iomDefsTable[ui32IOM_num];

    //
    // setup each pin, based on the pin map (piomd) and the
    // pre-configured pin mode (ptIOM_cfg)
    //
    pwr_iom_MOSI_MISO_setup(&piomd->sck,  psIOM_cfg->tSCK_pin);
    pwr_iom_MOSI_MISO_setup(&piomd->miso, psIOM_cfg->tMiso_pin);
    pwr_iom_MOSI_MISO_setup(&piomd->mosi, psIOM_cfg->tMosi_pin);
    pwr_iom_MOSI_MISO_setup(&piomd->cs,   psIOM_cfg->tCS_pin);

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Populate the debug pin number array
//
//*****************************************************************************

uint32_t
pwr_iom_getDebugPins (const iom_pins_setup_t *psIOM_cfg,
                      uint32_t *pui32DbgPinArray,
                      uint32_t ui32MaxDebugPins)
{
    uint32_t ui32IOM_num = psIOM_cfg->ui32Iom_number;
    const uint32_t  *pUI32DebugPinTable = g_iomDefsTable[ui32IOM_num].ui32DebugPin;
    for (uint32_t i = 0; i < ui32MaxDebugPins; i++)
    {
        pui32DbgPinArray[i] = *pUI32DebugPinTable++;
    }

    return AM_HAL_STATUS_SUCCESS;
}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
