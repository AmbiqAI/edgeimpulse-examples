/**
 ******************************************************************************
 *
 * @file acore_utilities.h
 *
 * @brief Acore utilities header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_UTILITIES_H_
#define _ACORE_UTILITIES_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/utilities.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/utilities.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_UTILITIES_H_ */
