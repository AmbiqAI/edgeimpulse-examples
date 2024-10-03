//*****************************************************************************
//
//! @file ambt53_reg_test.h
//!
//! @brief ambt53 register definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AMBT53_REG_TEST_H
#define AMBT53_REG_TEST_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_devices_ambt53.h"

extern ambt53_reg_test_t ambt53_reg_list[];
extern ambt53_reg_test_t scpm_all_reg_list[];
extern uint32_t START_REG_INDEX;
extern uint32_t END_REG_INDEX;
extern uint32_t START_SCPM_REG_INDEX;
extern uint32_t END_SCPM_REG_INDEX;
extern ambt53_reg_t scpm_clk_test_list[32];


#ifdef __cplusplus
}
#endif

#endif // AMBT53_REG_TEST_H

