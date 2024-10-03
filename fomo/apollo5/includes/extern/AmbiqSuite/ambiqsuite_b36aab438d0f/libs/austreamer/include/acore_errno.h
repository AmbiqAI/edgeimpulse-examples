/**
 ******************************************************************************
 *
 * @file acore_errno.h
 *
 * @brief Acore errno header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_ERRNO_H_
#define _ACORE_ERRNO_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/errno.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/errno.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_ERRNO_H_ */
