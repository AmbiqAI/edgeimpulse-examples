//*****************************************************************************
//
//! @file am_assert.h
//!
//! @brief Assert macro.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_ASSERT_H
#define AM_ASSERT_H

#include "am_types.h"


#ifdef __cplusplus
extern "C" {
#endif

void am_assert(const char *pFile, uint16_t line);



#ifdef AM_ASSERT_ENABLED
#define ASSERT_ERR(expr)      if (!(expr)) {am_assert(__FILE__, (uint16_t) __LINE__);}
#else
#define ASSERT_ERR(expr)
#endif



#ifdef __cplusplus
};
#endif

#endif /* AM_ASSERT_H */
