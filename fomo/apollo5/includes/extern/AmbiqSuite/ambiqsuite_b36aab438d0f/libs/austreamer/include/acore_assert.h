/**
 ******************************************************************************
 *
 * @file acore_assert.h
 *
 * @brief Acore assert header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_ASSERT_H_
#define _ACORE_ASSERT_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/assert.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/assert.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_ASSERT_H_ */
