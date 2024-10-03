/**
 ******************************************************************************
 *
 * @file acore_sleep.h
 *
 * @brief Acore sleep header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_SLEEP_H_
#define _ACORE_SLEEP_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/sleep.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/sleep.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_SLEEP_H_ */
