/**
 ******************************************************************************
 *
 * @file acore_list.h
 *
 * @brief Acore list header is for libmetal to adapt to different platform.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _ACORE_LIST_H_
#define _ACORE_LIST_H_

#if defined(SYSTEM_PROCESSOR_CEVA)
#include <metal/list.h>

#elif defined(SYSTEM_PROCESSOR_ARM)
#include <third_party/open-amp/libmetal/lib/metal_list.h>

#else
#error "Define SYSTEM_PROCESSOR_* first!"
#endif

#endif /* _ACORE_LIST_H_ */
