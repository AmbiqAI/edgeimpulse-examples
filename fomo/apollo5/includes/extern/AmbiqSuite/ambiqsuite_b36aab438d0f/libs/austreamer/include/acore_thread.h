/**
 ******************************************************************************
 *
 * @file acore_thread.h
 *
 * @brief Acore thread header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_THREAD_H_
#define _ACORE_THREAD_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/thread.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/thread.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_THREAD_H_ */
