/**
 ****************************************************************************************
 *
 * @file acore_log.h
 *
 * @brief Declaration Audio core log definition.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ACORE_LOG_H_
#define _ACORE_LOG_H_

// Trace log switch. Remove "x" to enable trace log
#define xACORE_LOG_TRACE_ENABLE

#if defined(USE_CEVA_UART_LOG)
#include "logger.h"
#include <stdbool.h>

#define ACORE_LOG_TRACE(x, ...)    am_debug_printf(true, x, ##__VA_ARGS__);
#define ACORE_LOG_INFO(x, ...)     am_info_printf(true, x, ##__VA_ARGS__);
#define ACORE_LOG_WARNING(x, ...)  am_warning_printf(true, x, ##__VA_ARGS__);
#define ACORE_LOG_ERROR(x, ...)    am_error_printf(true, x, ##__VA_ARGS__);
#define ACORE_LOG_CRITICAL(x, ...) am_critical_printf(true, x, ##__VA_ARGS__);

#elif defined(USE_MCU_UTIL_LOG)
#include <stdint.h>
#include "am_util_stdio.h"

#define ACORE_LOG_TRACE(x, ...)    am_util_stdio_printf(x, ##__VA_ARGS__);
#define ACORE_LOG_INFO(x, ...)     am_util_stdio_printf(x, ##__VA_ARGS__);
#define ACORE_LOG_WARNING(x, ...)  am_util_stdio_printf(x, ##__VA_ARGS__);
#define ACORE_LOG_ERROR(x, ...)    am_util_stdio_printf(x, ##__VA_ARGS__);
#define ACORE_LOG_CRITICAL(x, ...) am_util_stdio_printf(x, ##__VA_ARGS__);

#else

#define ACORE_LOG_TRACE(x, ...)
#define ACORE_LOG_INFO(x, ...)
#define ACORE_LOG_WARNING(x, ...)
#define ACORE_LOG_ERROR(x, ...)
#define ACORE_LOG_CRITICAL(x, ...)

#endif

#ifndef ACORE_LOG_TRACE_ENABLE
#undef ACORE_LOG_TRACE
#define ACORE_LOG_TRACE(x, ...)
#endif

#endif /* _ACORE_LOG_H_ */
