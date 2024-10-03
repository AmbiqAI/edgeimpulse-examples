/**
 ******************************************************************************
 *
 * @file acore_mutex.h
 *
 * @brief Acore mutex header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_MUTEX_H_
#define _ACORE_MUTEX_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/mutex.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/mutex.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_MUTEX_H_ */
