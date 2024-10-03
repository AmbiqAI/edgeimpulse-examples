//*****************************************************************************
//
//! @file am_logger.c
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#include "am_logger.h"
#include "am_assert.h"
#include "am_alloc.h"


logger_level_e curr_level;

static logger_context_t g_logger_default_ctx = {.name              = "",
                                                .name_print_enable = 0,
                                                .curr_level = LOG_LEVEL_INFO,
                                                .p_handler_list = NULL};



int logger_init_handler(logger_handler_t *p_log_handler, logger_ops_t *ops,
                        const char *name)
{
    void *handle = NULL;

    ASSERT_ERR(p_log_handler != NULL);
    ASSERT_ERR(ops != NULL);

    handle = (ops->open)(name, "w");
    if (handle == NULL)
        return 0;

    p_log_handler->type           = LOGGER_STREAM_TYPE_USER;
    p_log_handler->p_handle       = handle;
    p_log_handler->ops            = ops;
    p_log_handler->p_next_handler = NULL;
    return 1;
}

void logger_init(logger_level_e level)
{
    curr_level = level;
}

void logger_set_default_context(logger_context_t *p_ctx)
{
    g_logger_default_ctx = *p_ctx;
}

void logger_init_context(logger_context_t *p_ctx, const char *name,
                         logger_level_e level, logger_handler_t *p_log_handler)
{
    if (p_ctx == LOGGER_DEFAULT_CTX)
        p_ctx = &g_logger_default_ctx;

    strncpy(p_ctx->name, name, (LOGGER_MAX_CONTEXT_NAME_LEN - 1));
    p_ctx->curr_level        = level;
    p_ctx->name_print_enable = 0;
    p_ctx->p_handler_list    = p_log_handler;
}

int logger_print(logger_context_t *p_ctx, logger_level_e level, const char *msg, ...)
{
    int res   = 0;
    int error = false;
    if (p_ctx == LOGGER_DEFAULT_CTX)
        p_ctx = &g_logger_default_ctx;


    if (level <= p_ctx->curr_level)
    {
        logger_handler_t *p_curr_handler;
        va_list           arg;

        // scan the list and print to all streams
        p_curr_handler = p_ctx->p_handler_list;
        while (p_curr_handler != NULL)
        {
            int ret_val;
            va_start(arg, msg);
            ret_val = (p_curr_handler->ops->printf)(p_curr_handler->p_handle,
                                                    msg, arg);
            if (ret_val >= 0)
            {
                res = ret_val;
            }
            else
            {
                error = true;
            }
            va_end(arg);
            p_curr_handler = p_curr_handler->p_next_handler;
        }
    }
    return (error ? -1 : res);
}


void logger_flush(logger_context_t *p_ctx)
{
    logger_handler_t *p_curr_handler;
    if (p_ctx == LOGGER_DEFAULT_CTX)
        p_ctx = &g_logger_default_ctx;

    // scan the list and flush all files
    p_curr_handler = p_ctx->p_handler_list;
    while (p_curr_handler != NULL)
    {
        (p_curr_handler->ops->flush)(p_curr_handler->p_handle);
        p_curr_handler = p_curr_handler->p_next_handler;
    }
}

