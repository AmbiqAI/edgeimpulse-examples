/**
 ******************************************************************************
 *
 * @file acore_sys.h
 *
 * @brief Acore sys header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_SYS_H_
#define _ACORE_SYS_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/sys.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/sys.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_SYS_H_ */
