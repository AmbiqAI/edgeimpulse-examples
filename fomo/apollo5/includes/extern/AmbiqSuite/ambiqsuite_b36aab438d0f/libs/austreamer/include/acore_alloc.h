/**
 ******************************************************************************
 *
 * @file acore_alloc.h
 *
 * @brief Acore alloc header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_ALLOC_H_
#define _ACORE_ALLOC_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/alloc.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/alloc.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_ALLOC_H_ */
