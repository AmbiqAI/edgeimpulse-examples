//*****************************************************************************
//
//! @file nemadc_test_common.c
//!
//! @brief NemaGFX test cases common source code.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "nemadc_test_common.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define NemaDC_rcmd16     (1U << 28)
#define NemaDC_rcmd24     (1U << 29)
#define NemaDC_rcmd32     ((1U << 29) | (1U << 28))
#define NemaDC_DBI_read   (1U << 26)

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
#ifdef USE_DPHYPLL
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_DPHYPLL, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_PLLCLKSEL_HFRC12, NULL); 
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_PLLCLK_ENABLE, NULL);
#else
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC48, NULL);
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
#endif
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);
#ifdef USE_DPHYPLL
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISPPHY);
#endif

    CPU->DAXICFG_b.BUFFERENABLE = CPU_DAXICFG_BUFFERENABLE_ONE; // only enable a single buffer

    PWRCTRL->SSRAMPWREN = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL; // enable all
    while(PWRCTRL->SSRAMPWRST == 0);
    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 NemaDC Test Cases\n\n");

    nema_sys_init();
    //
    // Initialize NemaDC
    //
    if(nemadc_init() != 0)
    {
        am_util_stdio_printf("NemaDC init failed!\n");
    }
    //
    // Initialize DSI
    //
#if defined(ENABLE_DSI) || defined(USE_DPHYPLL)
    uint8_t ui8LanesNum = 1;
    uint8_t ui8DbiWidth = 8;
    uint32_t ui32FreqTrim = 0x06; // 36MHz x 4 = 144MHz
    if ( am_hal_dsi_para_config(ui8LanesNum, ui8DbiWidth, ui32FreqTrim, false) != 0 )
    {
        am_util_stdio_printf("DSI init failed!\n");
    }
#endif
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
//! @brief Read data from display module.
//!
//! @param eCmd - Read command.
//! @param ui32Length - Number of read bytes.
//!
//! @return ui32RData - the data read back.
//
//*****************************************************************************
uint32_t
am_nemadc_read(uint32_t eCmd, uint32_t ui32Length)
{
    uint32_t ui32RData = 0;

#if ((defined ENABLE_DSPI) || (defined ENABLE_QSPI))
    uint32_t ui32DBIConfig;
#endif

#if ((defined ENABLE_SPI4) || (defined ENABLE_DSPI))
    uint32_t ui32Cfg;
    switch(ui32Length)
    {
        case 1:
            ui32Cfg = 0;
            break;
        case 2:
            ui32Cfg = NemaDC_rcmd16;
            break;
        case 3:
            ui32Cfg = NemaDC_rcmd24;
            break;
        case 4:
            ui32Cfg = NemaDC_rcmd32;
            break;
        default:
            return 0;
    }
#endif

//
// 12MHz FPGA image requires longer delay than 48MHz image, so changed delay from 20us to 100us, added some margin.
//
#ifdef ENABLE_SPI4
    nemadc_MIPI_out(MIPI_DBIB_CMD | NemaDC_DBI_read | ui32Cfg | eCmd);
    while((nemadc_reg_read(NEMADC_REG_STATUS) & 0x1c00U) != 0U);
    am_util_delay_us(100);
    ui32RData = nemadc_reg_read(NEMADC_REG_DBIB_RDAT);
#endif

#ifdef ENABLE_DSPI
    ui32DBIConfig = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    nemadc_MIPI_CFG_out(ui32DBIConfig & (~MIPICFG_DSPI) & (~MIPICFG_QSPI));
    nemadc_MIPI_out(MIPI_DBIB_CMD | NemaDC_DBI_read | ui32Cfg | eCmd);
    while((nemadc_reg_read(NEMADC_REG_STATUS) & 0x1c00U) != 0U);
    am_util_delay_us(100);
    ui32RData = nemadc_reg_read(NEMADC_REG_DBIB_RDAT);
    nemadc_MIPI_CFG_out(ui32DBIConfig);
#endif

#ifdef ENABLE_QSPI // QSPI only supports 1 byte read.
    ui32DBIConfig = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    nemadc_MIPI_CFG_out(ui32DBIConfig | MIPICFG_SPI_HOLD);
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_MASK_QSPI | SPI_READ);
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_MASK_QSPI | MIPI_CMD16 | (eCmd));
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_MASK_QSPI | NemaDC_DBI_read);
    while((nemadc_reg_read(NEMADC_REG_STATUS) & 0x1c00U) != 0U);
    nemadc_MIPI_CFG_out(ui32DBIConfig);
    am_util_delay_us(100);
    ui32RData = nemadc_reg_read(NEMADC_REG_DBIB_RDAT);
#endif
    return ui32RData;
}