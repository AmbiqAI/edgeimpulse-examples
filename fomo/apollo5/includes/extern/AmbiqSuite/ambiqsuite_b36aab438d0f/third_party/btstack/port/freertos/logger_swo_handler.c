/**
 ****************************************************************************************
 *
 * @file logger_swo_handler.c
 *
 * @brief Log handler using JLink device
 *
 *
 *
 ****************************************************************************************
 */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "am_logger.h"
#include "am_util_debug.h"
#include "am_util_stdio.h"
#include "am_assert.h"


typedef enum
{
    SWO_DEV_STATUS_IDLE,
    SWO_DEV_STATUS_BUSY
} swo_dev_status_e;

#define MAX_LINE 256 // including the null termination
#define BUF_SIZE 256

typedef struct swo_dev_handle
{
    unsigned int               dev_id;
    volatile swo_dev_status_e status;
    unsigned int               pos;
    unsigned int               work_buf;
    uint8_t                    buf[2][BUF_SIZE];
} swo_dev_handle_t;

static void *swo_open(const char *name, const char *mode);
static int   swo_flush(void *handle);
static int   swo_printf(void *handle, const char *msg, va_list arg);


#define SWO_DEV_INIT(id)                                                      \
    {                                                                          \
        (id), SWO_DEV_STATUS_IDLE, 0, 0,                                      \
        {                                                                      \
            {0},                                                               \
            {                                                                  \
                0                                                              \
            }                                                                  \
        }                                                                      \
    }

static logger_ops_t ops = {
    .open   = swo_open,
    .flush  = swo_flush,
    .printf = swo_printf,
};

static logger_handler_t handler;

swo_dev_handle_t swo_dev = SWO_DEV_INIT(0);

static void *swo_open(const char *name, const char *mode)
{
    if (!strcmp(name, "swo"))
    {
        return (void *)&swo_dev;
    }
    else
    {
        return NULL;
    }
}


static int swo_flush(void *handle)
{
    swo_dev_handle_t *dev = (swo_dev_handle_t *)handle;
    unsigned int       len = dev->pos;

    if (len > 0)
    {
        dev->pos = 0;
		am_util_debug_printf((char *)(dev->buf[dev->work_buf]));
    }
    return 0;
}


static int swo_printf(void *handle, const char *msg, va_list arg)
{
    int                ret;
    char    str_buf[MAX_LINE] = {0};

    uint32_t ui32NumChars;
    ret = am_util_stdio_vsprintf(str_buf, msg, arg);

    ui32NumChars = am_util_debug_printf(str_buf);

    if (!(ui32NumChars < MAX_LINE))
    {
        am_assert(__FILE__, (uint16_t) __LINE__);
    }

    return ret;
}


logger_handler_t *swo_log_handler_get(void)
{
    static unsigned int is_initialized = 0;
    if (!is_initialized)
    {
        is_initialized++;
        logger_init_handler(&handler, &ops, "swo");
    }
    return &handler;
}
