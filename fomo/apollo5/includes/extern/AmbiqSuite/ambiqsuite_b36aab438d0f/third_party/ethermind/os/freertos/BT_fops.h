
/**
 *  \file BT_fops.h
 *
 *  This file has all definitions of constants and declarations APIs
 *  for File Operations.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_FOPS_
#define _H_BT_FOPS_

/* ------------------------- Header File Inclusion */
/* EtherMind Common Header Files */
#if 0
#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif /* 0 */

#include "BT_common.h"
#include "BT_timer.h"
#include "BT_debug.h"

/* ------------------------- Debug Macros */
#ifndef BT_FOPS_NO_DEBUG
#define BT_FOPS_ERR(...)         BT_debug_error(BT_MODULE_ID_FOPS, __VA_ARGS__)
#else  /* BT_FOPS_NO_DEBUG */
#define BT_FOPS_ERR              BT_debug_null
#endif /* BT_FOPS_NO_DEBUG */

#ifdef BT_FOPS_DEBUG

#define BT_FOPS_TRC(...)         BT_debug_trace(BT_MODULE_ID_FOPS, __VA_ARGS__)
#define BT_FOPS_INF(...)         BT_debug_info(BT_MODULE_ID_FOPS, __VA_ARGS__)

#define BT_FOPS_debug_dump_bytes(data, datalen) BT_debug_dump_bytes(BT_MODULE_ID_FOPS, (data), (datalen))

#else /* BT_FOPS_DEBUG */

#define BT_FOPS_TRC              BT_debug_null
#define BT_FOPS_INF              BT_debug_null

#define BT_FOPS_debug_dump_bytes(data, datalen)

#endif /* BT_FOPS_DEBUG */

/* ------------------------- Global Definitions */

typedef FILE * BT_fops_file_handle;


/** Bitmap mask for File Attributes */
#define BT_FOPS_MASK_READONLY                 0x00000001
#define BT_FOPS_MASK_FOLDER                   0x00000002

/** File Seek Positions */
#define BT_FOPS_SEEK_SET                      SEEK_SET
#define BT_FOPS_SEEK_CUR                      SEEK_CUR
#define BT_FOPS_SEEK_END                      SEEK_END

#define BT_FOPS_PATH_SEP                      "\\"

/* ------------------------- Macros */
/** Create File path with seperator */
#define BT_FOPS_PATH_JOIN(a,b)                a BT_FOPS_PATH_SEP b

/** To set/get/reset a Bit in File Operations Bitmaps */
#define BT_FOPS_SET_BIT(bitmap, mask)         (bitmap) |= (mask)
#define BT_FOPS_GET_BIT(bitmap, mask)         (((bitmap) & (mask)) == (mask))
#define BT_FOPS_RESET_BIT(bitmap, mask)       (bitmap) ^= (mask)

/* ------------------------- Function Declarations */

API_RESULT BT_fops_get_current_directory
           (
               /* OUT */  UCHAR  * current_directory
           );

API_RESULT BT_fops_get_file_attributes
           (
               /* IN */   UCHAR   * object_name,
               /* OUT */  UINT32  * file_attribute
           );

API_RESULT BT_fops_set_file_attributes
           (
               /* IN */   UCHAR   * object_name,
               /* IN */   UINT32    file_attribute
           );

API_RESULT BT_fops_set_path_forward
           (
               /* IN */  UCHAR *folder_name
           );

API_RESULT BT_fops_set_path_backward( void );

API_RESULT BT_fops_create_folder
           (
               /* IN */  UCHAR * folder_name
           );

API_RESULT BT_fops_file_open
           (
               /* IN */  UCHAR                * file_name,
               /* IN */  UCHAR                * mode,
               /* OUT */ BT_fops_file_handle  *file_handle
           );

API_RESULT BT_fops_file_write
           (
               /* IN */   UCHAR               * buffer,
               /* IN */   UINT16                buf_length,
               /* IN */   BT_fops_file_handle   file_handle,
               /* OUT */  UINT16              * bytes_written
           );

API_RESULT BT_fops_file_read
           (
               /* IN */   UCHAR               * buffer,
               /* IN */   UINT16                buf_length,
               /* IN */   BT_fops_file_handle   file_handle,
               /* OUT */  UINT16              * bytes_read
           );

API_RESULT BT_fops_file_close
           (
               /* IN */  BT_fops_file_handle  file_handle
           );

API_RESULT BT_fops_object_delete
           (
               /* IN */  UCHAR * object_name
           );

API_RESULT BT_fops_file_size
           (
               /* IN */  BT_fops_file_handle   file_handle,
               /* OUT */ UINT32              * file_size
           );

API_RESULT BT_fops_file_seek
           (
               /* IN */  BT_fops_file_handle   file_handle,
               /* IN */  INT32                 offset,
               /* IN */  INT32                 whence
           );

API_RESULT BT_fops_file_copy
           (
               /* IN */  UCHAR * existing_file_name,
               /* IN */  UCHAR * new_file_name,
               /* IN */  UCHAR   fail_if_exists
           );

API_RESULT BT_fops_file_move
           (
               /* IN */  UCHAR * existing_file_name,
               /* IN */  UCHAR * new_file_name
           );

API_RESULT BT_vfops_create_object_name
           (
               /* IN */  UCHAR * path,
               /* IN */  UCHAR * object,
               /* OUT */ UCHAR * obj_name
           );

API_RESULT BT_vfops_set_path_backward
           (
               /* OUT */ UCHAR *path
           );

API_RESULT BT_vfops_set_path_forward
           (
               /* INOUT */  UCHAR * path,
               /* IN */     UCHAR * folder
           );

#endif /* _H_BT_FOPS_ */

