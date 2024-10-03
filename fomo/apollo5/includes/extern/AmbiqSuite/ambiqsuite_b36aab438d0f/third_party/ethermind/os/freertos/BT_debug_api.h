
/**
 *  \file BT_debug_api.h
 *
 *  This Header File contains the APIs and the ADTs exported by the
 *  EtherMind Debug Library for Windows (User-mode).
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_DEBUG_API_
#define _H_BT_DEBUG_API_

/* ----------------------------------------------- Header File Inclusion */
#include "BT_os.h"

#define BT_DEBUG_DONT_LOG_FILE_PATH

/* ----------------------------------------------- Debug Macros */
/* Define Debug Message Types */
/* Message Type - ERR */
#define BT_DEBUG_MSG_ERR              1

/* Message Type - TRC */
#define BT_DEBUG_MSG_TRC              2

/* Message Type - INF */
#define BT_DEBUG_MSG_INF              3

/* Define Debug Levels */
/* No runtime error logging */
#define BT_DEBUG_LEVEL_NONE           0

/* Log only ERR messages */
#define BT_DEBUG_LEVEL_ERR            1

/* Log ERR and TRC messages */
#define BT_DEBUG_LEVEL_TRC            2

/* Log ERR, TRC and INF messages */
#define BT_DEBUG_LEVEL_INF            3
#define BT_DEBUG_LEVEL_ALL            3

/* Maximum number of module pages */
#define BT_MODULE_PAGE_BITS_COUNT     4
#define BT_MAX_MODULE_PAGE_COUNT      (1 << BT_MODULE_PAGE_BITS_COUNT)

/* Debug Enable/Disable flag */
#define BT_DEBUG_ENABLE               0x01
#define BT_DEBUG_DISABLE              0x00

/**
 * Special wildcard define to represent all modules.
 * Used to enable/disable all module at once.
 */
#define BT_MODULE_ALL                 0xFFFFFFFF

#define BT_GET_PAGE_IDX_MODULE_BIT_MASK(module_id, page_idx, m_bit_mask) \
        { \
            /* Extract the Page Number and Module Bit Mask */ \
            (page_idx) = ((module_id) >> (32 - BT_MODULE_PAGE_BITS_COUNT)); \
            (m_bit_mask) = (((page_idx) << (32 - BT_MODULE_PAGE_BITS_COUNT)) ^ (module_id)); \
        }

#define BT_GET_MODULE_ID(module_id, page_idx, m_bit_mask) \
        { \
            /* Create Module Id from the Page Number and Module Bit Mask */ \
            (module_id) = (((page_idx) << (32 - BT_MODULE_PAGE_BITS_COUNT)) | (m_bit_mask)); \
        }

#ifdef BT_ENABLE_DISABLE_RUNTIME_DEBUG
#define BT_enable_module_debug_flag(module_id) \
        BT_update_module_debug_flag((module_id), BT_DEBUG_ENABLE)

#define BT_disable_module_debug_flag(module_id) \
        BT_update_module_debug_flag((module_id), BT_DEBUG_DISABLE)
#endif /* BT_ENABLE_DISABLE_RUNTIME_DEBUG */

#ifdef BT_DEBUG_DONT_LOG_FILE_PATH
#define BT_FILE_NAME     \
        (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define BT_FILE_NAME     __FILE__
#endif /* BT_DEBUG_DONT_LOG_FILE_PATH */

/* Debug print macros, based on the debug message types */
#define BT_debug_printf_err(module_id, ...) BT_debug_printf(BT_DEBUG_MSG_ERR, (module_id), __VA_ARGS__)
#define BT_debug_printf_trc(module_id, ...) BT_debug_printf(BT_DEBUG_MSG_TRC, (module_id), __VA_ARGS__)
#define BT_debug_printf_inf(module_id, ...) BT_debug_printf(BT_DEBUG_MSG_INF, (module_id), __VA_ARGS__)

#ifdef BT_LOG_TIMESTAMP
#define BT_debug_error(module_id, ...) \
    BT_debug_printf_err((module_id), "\n[** ERR **]:[%s]:[%s]:[%d]: [<%s>] ", \
    BT_debug_get_current_timestamp(), BT_FILE_NAME, __LINE__); \
    BT_debug_printf_err((module_id), __VA_ARGS__)

#define BT_debug_trace(module_id, ...) \
    BT_debug_printf_trc((module_id), "\n[-- TRC --]:[%s]:[%s]:[%d]: [<%s>] ", \
    BT_debug_get_current_timestamp(), BT_FILE_NAME, __LINE__); \
    BT_debug_printf_trc((module_id), __VA_ARGS__)

#define BT_debug_info(module_id, ...) \
    BT_debug_printf_inf((module_id), "\n[== INF ==]:[%s]:[%s]:[%d]: [<%s>] ", \
    BT_debug_get_current_timestamp(), BT_FILE_NAME, __LINE__); \
    BT_debug_printf_inf((module_id), __VA_ARGS__)

#define BT_debug_dump(module_id, ...) \
    BT_debug_printf_inf((module_id), "\n[++ HEX ++]:[%s]:[%s]:[%d]: [<%s>] ", \
    BT_debug_get_current_timestamp(), BT_FILE_NAME, __LINE__); \
    BT_debug_printf_inf((module_id), __VA_ARGS__)

#else /* BT_LOG_TIMESTAMP */
#define BT_debug_error(module_id, ...) \
    BT_debug_printf_err((module_id), "\n[** ERR **]:[%s]:[%d]: ", \
    BT_FILE_NAME, __LINE__); \
    BT_debug_printf_err((module_id), __VA_ARGS__)

#define BT_debug_trace(module_id, ...) \
    BT_debug_printf_trc((module_id), "\n[-- TRC --]:[%s]:[%d]: ", \
    BT_FILE_NAME, __LINE__); \
    BT_debug_printf_trc((module_id), __VA_ARGS__)

#define BT_debug_info(module_id, ...) \
    BT_debug_printf_inf((module_id), "\n[== INF ==]:[%s]:[%d]: ", \
    BT_FILE_NAME, __LINE__); \
    BT_debug_printf_inf((module_id), __VA_ARGS__)

#define BT_debug_dump(module_id, ...) \
    BT_debug_printf_inf((module_id), "\n[++ HEX ++]:[%s]:[%d]: ", \
    BT_FILE_NAME, __LINE__); \
    BT_debug_printf_inf((module_id), __VA_ARGS__)

#endif  /* BT_LOG_TIMESTAMP */

/* TBD: Check where is this being used */
#define BT_debug_direct(module_id, ...) \
    BT_debug_printf_inf((module_id), "\n[~~ LOG ~~]: "); \
    BT_debug_printf_inf((module_id), __VA_ARGS__)


/* ----------------------------------------------- Global Definitions */
#define BT_debug_null(...)

/* ----------------------------------------------- Structures/Data Types */


/* ----------------------------------------------- Function Declarations */
#ifdef __cplusplus
extern "C"{
#endif

void BT_Init_Debug_Library (void);

/* Debug Library Init & Shutdown Routines */
void debug_bt_init ( void );
void debug_bt_shutdown ( void );

#ifndef BT_DISABLE_ALL_DEBUG
INT32 BT_debug_printf(UCHAR msg_type, UINT32 module_id, const CHAR *fmt, ...);
INT32 BT_debug_dump_bytes(UINT32 module_id, UCHAR *buffer, UINT32 length);
INT32 BT_debug_dump_decimal(UINT32 module_id, UCHAR *buffer, UINT32 length);
#else
#define BT_debug_printf(...)
#define BT_debug_dump_bytes(module_id, buffer, length)
#define BT_debug_dump_decimal(module_id, buffer, length)
#endif /* BT_DISABLE_ALL_DEBUG */

#ifdef BT_LOG_TIMESTAMP
UCHAR * BT_debug_get_current_timestamp (void);
#endif /* BT_LOG_TIMESTAMP */

#ifdef BT_ENABLE_DISABLE_RUNTIME_DEBUG
void BT_set_debug_level(UCHAR level);

void BT_update_module_debug_flag(UINT32 module_id, UCHAR flag);
#endif /* BT_ENABLE_DISABLE_RUNTIME_DEBUG */

#ifdef __cplusplus
};
#endif

#endif /* _H_BT_DEBUG_API_ */

