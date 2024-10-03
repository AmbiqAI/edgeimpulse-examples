/**
 ******************************************************************************
 *
 * @file acore_atomic.h
 *
 * @brief Acore atomic header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_ATOMIC_H_
#define _ACORE_ATOMIC_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/atomic.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/atomic.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_ATOMIC_H_ */
