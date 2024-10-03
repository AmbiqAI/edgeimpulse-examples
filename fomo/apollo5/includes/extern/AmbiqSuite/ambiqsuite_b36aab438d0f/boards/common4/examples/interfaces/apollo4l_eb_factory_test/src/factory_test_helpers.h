//*****************************************************************************
//
//  factory_test_helpers.h
//! @file
//!
//! @brief Top Include for Apollo1 class devices.
//!
//! This file provides all the includes necessary for an apollo device.
//!
//! @addtogroup hal Hardware Abstraction Layer (HAL)
//
//! @defgroup apollo1hal HAL for Apollo
//! @ingroup hal
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef FACTORY_TEST_HELPERS_H
#define FACTORY_TEST_HELPERS_H

//*****************************************************************************
//
// Required built-ins.
//
//*****************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "am_util.h"

#define UnityPrint am_util_stdio_printf

struct UNITY_STORAGE_T
{
    const char* TestFile;
    const char* CurrentTestName;

    uint32_t CurrentTestLineNumber;
    uint32_t NumberOfTests;
    uint32_t TestFailures;
    uint32_t TestIgnores;
};


extern int  memory_compare(const void* cs, const void* ct, uint32_t count);
extern int UnityEnd(struct UNITY_STORAGE_T *pHandle);
#endif

