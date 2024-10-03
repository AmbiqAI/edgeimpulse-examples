
/**
 *  \file appl_uri.h
 *
 *
 */

/*
 *  Copyright (C) 2015. Mindtree Limited.
 *  All rights reserved.
 */
#ifndef _H_APPL_URI_
#define _H_APPL_URI_

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

/* URI Testing related Compilation Flag */
/* #define APPL_TEST_URI_UTIL */
/* --------------------------------------------- Global Definitions */

/* --------------------------------------------- Structures/Data Types */
typedef struct
{
    UCHAR    utf_code;
    char *   name_str;
} APPL_UTF_CODE_NAME;

/* --------------------------------------------- Error Codes */
/** Error Codes for Application URI Module */
#define APPL_URI_ERR_ID                              0xAA00

#define APPL_URI_INVALID_PARAMETER_VALUE             (0x0001 | APPL_URI_ERR_ID)
#define APPL_URI_PARAMETER_OUTSIDE_RANGE             (0x0002 | APPL_URI_ERR_ID)
#define APPL_URI_NULL_PARAMETER_NOT_ALLOWED          (0x0003 | APPL_URI_ERR_ID)
#define APPL_URI_INSUFFICIENT_BUFFER_LENGTH          (0x0004 | APPL_URI_ERR_ID)
#define APPL_URI_UNKNOWN_SCHEME                      (0x0005 | APPL_URI_ERR_ID)
#define APPL_URI_INCORRECT_ENCODING                  (0x0006 | APPL_URI_ERR_ID)

/* --------------------------------------------- Macros */
#define BT_URI_8_11_BITS_UTF_UB_MASK       0xC0
#define BT_URI_8_11_BITS_UTF_LB_MASK       0x80
#define BT_URI_8_11_BITS_UTF_LB_INIT_MASK  0x3F

#define BT_URI_8_11_BITS_UTF_UB_ENC_LEN    6
#define BT_URI_8_11_BITS_UTF_UB_DEC_LEN    6

#define BT_URI_UTF_SB_MAX_VAL              0x7F
#define BT_URI_UTF_MB_MIN_VAL              0x80
#define BT_URI_UTF_MB_MAX_VAL              0xFF
#define BT_URI_UTF_MB_MASK                 0xC0

/* Used to define Static Array Size, Configurable based on usecase */
#ifdef APPL_TEST_URI_UTIL
#define APPL_URI_MAX_BUFFER                100
#else /* APPL_TEST_URI_UTIL */
#define APPL_URI_MAX_BUFFER                40
#endif /* APPL_TEST_URI_UTIL */

/* --------------------------------------------- Internal Functions */
char *appl_get_utf_str_name (UCHAR code);

API_RESULT appl_encode_octet_to_utf
           (
               /* IN */  UCHAR  ip_uri_code,
               /* OUT */ UCHAR  *op_utf,
               /* OUT */ UCHAR  *op_len
           );

API_RESULT appl_decode_from_utf_string
           (
               /* IN */  UCHAR  * ip_str,
               /* IN */  UINT16 ip_len,
               /* OUT */ UCHAR  * op_utf,
               /* OUT */ UINT16 * op_len
           );

API_RESULT appl_search_uri_scheme_id
           (
               /* IN */  UCHAR  * uri,
               /* IN */  UINT16 uri_len,
               /* OUT */ UCHAR  * scheme_id
           );

API_RESULT appl_uri_map_string_from_octet
           (
               /* IN */    UCHAR flag,
               /* IN */    UCHAR utf_octet,
               /* OUT */   UCHAR * op,
               /* INOUT */ UINT16 * op_len
           );
/* --------------------------------------------- API Declarations */

API_RESULT appl_encode_uri_adv_data
           (
               /* IN */  UCHAR  * uri,
               /* IN */  UINT16 uri_len,
               /* OUT */ UCHAR  * adv_uri,
               /* OUT */ UINT16 * adv_uri_len
           );

API_RESULT appl_decode_uri_adv_data
           (
               /* IN */   UCHAR  * adv_uri,
               /* IN */   UINT16 adv_uri_len,
               /* OUT */  UCHAR  * uri,
               /* OUT */  UINT16 * uri_len
           );

void appl_parse_adv_pkt_for_uri (UCHAR * data, UINT16 data_len);

#ifdef APPL_TEST_URI_UTIL
void appl_uri_test(void);
#endif /* APPL_TEST_URI_UTIL */

#endif /* _H_APPL_URI_ */
