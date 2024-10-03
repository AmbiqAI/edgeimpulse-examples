//*****************************************************************************
//
//! @file am_widget_pdm.h
//!
//! @brief
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Structure description for each test.
//
//*****************************************************************************
typedef struct
{
    am_hal_pdm_config_t sConfig;
    uint32_t ui32NumBytes;
    uint32_t ui32ExpectedResult;
    uint32_t ui32Delta;
}
am_widget_pdm_dma_test_config_t;

//*****************************************************************************
//
// Test widget functions.
//
//*****************************************************************************
extern uint32_t am_widget_pdm_config_test(am_hal_pdm_config_t *psConfig, char *pErrStr);
extern uint32_t am_widget_pdm_dma_test(void *psTestConfig, char *pErrStr);

