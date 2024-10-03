//*****************************************************************************
//
//! @file am_logger.h
//!
//! @brief Debugging log facility.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_LOGGER_H
#define AM_LOGGER_H

#include "am_types.h"

#ifdef __cplusplus
#include <cstdarg> // for va_list
#include <cstddef>
#else
#include <stdarg.h> // for va_list
#include <stddef.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LOGGER_DEFAULT_CTX (NULL)

typedef enum
{
    LOGGER_STREAM_TYPE_STDOUT,
    LOGGER_STREAM_TYPE_STDERR,
    LOGGER_STREAM_TYPE_FILE,
    LOGGER_STREAM_TYPE_USER
} logger_stream_type_e;

typedef void *(*logger_handler_open_t)(const char *name, const char *mode);
typedef int (*logger_handler_close_t)(void *handle);
typedef int (*logger_handler_flush_t)(void *handle);
typedef int (*logger_handler_printf_t)(void *handle, const char *msg,
                                        va_list arg);
typedef int (*logger_handler_putc_t)(int c, void *handle);


typedef struct logger_ops_t_
{
    logger_handler_open_t   open;
    logger_handler_close_t  close;
    logger_handler_flush_t  flush;
    logger_handler_printf_t printf;
    logger_handler_putc_t   putc;
} logger_ops_t;

typedef struct logger_handler_t_
{
    logger_stream_type_e      type;
    void                     *p_handle;
    logger_ops_t             *ops;
    struct logger_handler_t_ *p_next_handler;
} logger_handler_t;

typedef enum
{
    LOG_LEVEL_CRITICAL, // A serious error, indicating that the program
                        // itself may be unable to continue running.
    LOG_LEVEL_ERROR, // Due to a more serious problem, the software has not
                     // been able to perform some function.
    LOG_LEVEL_WARNING, // An indication that something unexpected happened,
                       // or indicative of some problem in the near future
                       // (e.g. "disk space low"). The software is still
                       // working as expected.
    LOG_LEVEL_INFO,    // Confirmation that things are working as expected.
    LOG_LEVEL_DEBUG // Detailed information, typically of interest only when
                    // diagnosing problems.
} logger_level_e;

#define LOGGER_MAX_CONTEXT_NAME_LEN 40

typedef struct logger_context_t_
{
    char              name[LOGGER_MAX_CONTEXT_NAME_LEN];
    int               name_print_enable;
    logger_level_e    curr_level;
    logger_handler_t *p_handler_list;
} logger_context_t;

int logger_print(logger_context_t *p_ctx, logger_level_e level, const char *msg, ...);

// TODO only used for RPMSG debug,will be deleted
#define rpmsg_debug_printf(...)                                                \
logger_print(LOGGER_DEFAULT_CTX, LOG_LEVEL_INFO, __VA_ARGS__)

#define AM_TRACE_LOG(flush, level, ...)                                        \
{                                                                          \
    logger_print(LOGGER_DEFAULT_CTX, level, __VA_ARGS__);                  \
}

/**
 ****************************************************************************************
 * @brief Use uart to prints log immediately or cache it in buffer
 *
 * This function prints log through BT uart1 interface
 * @param  flush set to true to send all messages in buffer immediately not
 *matters buffer is full or not
 ****************************************************************************************
 */
#define am_debug_printf(flush, ...)                                            \
    AM_TRACE_LOG(flush, LOG_LEVEL_DEBUG, __VA_ARGS__)

#define am_info_printf(flush, ...)                                             \
    AM_TRACE_LOG(flush, LOG_LEVEL_INFO, __VA_ARGS__)

#define am_warning_printf(flush, ...)                                          \
    AM_TRACE_LOG(flush, LOG_LEVEL_WARNING, __VA_ARGS__)

#define am_error_printf(flush, ...)                                            \
    AM_TRACE_LOG(flush, LOG_LEVEL_ERROR, __VA_ARGS__)

#define am_critical_printf(flush, ...)                                         \
    AM_TRACE_LOG(flush, LOG_LEVEL_CRITICAL, __VA_ARGS__)

int logger_init_handler(logger_handler_t *p_log_handler, logger_ops_t *ops,
                        const char *name);

void logger_init_context(logger_context_t *p_ctx, const char *name,
                         logger_level_e    level,
                         logger_handler_t *p_log_handler);

void logger_context_set_level(logger_context_t *, logger_level_e level);


void logger_flush(logger_context_t *p_ctx);

void logger_set_default_context(logger_context_t *p_ctx);

logger_context_t *logger_get_default_context(void);

// SWO Handler
logger_handler_t *swo_log_handler_get(void);

#ifdef __cplusplus
};
#endif

#endif /* AM_LOGGER_H */
