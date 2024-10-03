/**
 ******************************************************************************
 *
 * @file acore_time.h
 *
 * @brief Acore time header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_TIME_H_
#define _ACORE_TIME_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/time.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/time.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_TIME_H_ */
