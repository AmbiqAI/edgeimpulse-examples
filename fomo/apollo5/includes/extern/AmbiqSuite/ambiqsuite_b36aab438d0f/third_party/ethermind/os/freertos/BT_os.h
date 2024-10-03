
/**
 *  \file BT_os.h
 *
 *  This header file is part of EtherMind OS Abstraction module.
 *  In this file the platform specific data types are abstracted and
 *  mapped to data types used within the EtherMind Stack.
 *
 *  Version: FreeRTOS
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_OS_
#define _H_BT_OS_

/* --------------------------------------------------- Header File Inclusion */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* EtherMind Features */
#include "BT_features.h"

/* --------------------------------------------------- Global Definitions */

/* --------------------------------------------------- Data Type Definitions */
/* 'signed' datatype of size '1 octet' */
typedef char CHAR;
/* 'signed' datatype of size '1 octet' */
typedef signed char INT8;
/* 'unsigned' datatype of size '1 octet' */
typedef unsigned char UCHAR;
/* 'unsigned' datatype of size '1 octet' */
typedef unsigned char UINT8;
/* 'signed' datatype of size '2 octet' */
typedef short int INT16;
/* 'unsigned' datatype of size '2 octet' */
typedef unsigned short int UINT16;
/* 'signed' datatype of size '4 octet' */
typedef int INT32;
/* 'unsigned' datatype of size '4 octet' */
typedef unsigned int  UINT32;
/* 'unsigned' datatype of size '8 octet' */
/* typedef unsigned long long UINT64; */

/* 'unsigned' datatype of size '1 octet' */
typedef unsigned char BOOLEAN;

/*
 * Generic handle can used for the unknow
 * datatypes (e.g. Mutex Type, Conditional Type etc.)
 */
typedef void * EM_GENERIC_HANDLE;

/* Datatype to represent Thread or Task Identifier */
typedef xTaskHandle BT_thread_type;
/* Datatype to represent Thread or Task Attribute */
typedef /* pthread_attr_t */ void BT_thread_attr_type;
/* Datatype to represent Mutex object */
typedef xSemaphoreHandle BT_thread_mutex_type;
/* Datatype to represent Attributes of a Mutex object */
typedef /* pthread_mutexattr_t */ void BT_thread_mutex_attr_type;
/* Datatype to represent Conditional Variable object */
typedef xSemaphoreHandle BT_thread_cond_type;
/* Datatype to represent Attributes of a Conditional Variable object */
typedef /* pthread_condattr_t */ void BT_thread_cond_attr_type;

/* Datatype to represent File Handle */
typedef EM_GENERIC_HANDLE BT_FILE_HANDLE;

/* Data type to represent the OS time for the platform */
typedef int              BT_time_type;

/* Data types for task parameters and retun types */
typedef void * BT_THREAD_RETURN_TYPE;
typedef void * BT_THREAD_ARGS;
typedef BT_THREAD_RETURN_TYPE (*BT_THREAD_START_ROUTINE)(BT_THREAD_ARGS);


/* --------------------------------------------------- Macros */

#define BT_THREAD_RETURN_VAL_SUCCESS NULL
#define BT_THREAD_RETURN_VAL_FAILURE NULL

/* Declaration of 'static' variable/function */
#ifdef BT_HAVE_STATIC_DECL
#define DECL_STATIC                     static
#else  /* BT_HAVE_STATIC_DECL */
#define DECL_STATIC
#endif /* BT_HAVE_STATIC_DECL */

/* Declaration of 'const' variable */
#ifdef BT_HAVE_CONST_DECL
#define DECL_CONST                      const
#else  /* BT_HAVE_CONST_DECL */
#define DECL_CONST
#endif /* BT_HAVE_CONST_DECL */

/* Reentrant Function Declaration */
#ifdef BT_HAVE_REENTRANT_DECL
#define DECL_REENTRANT                  reentrant
#else /* BT_HAVE_REENTRANT_DECL */
#define DECL_REENTRANT
#endif /* BT_HAVE_REENTRANT_DECL */

#define DECL_CONST_QUALIFIER

/* Declaration of Patchable Functions */
#ifdef BT_HAVE_PATCHABILITY

#define DECL_PATCHABLE_DEFN(type, fn)           type _ROM_##fn
#define DECL_PATCHABLE_PROTO(type, fn, args)    \
        type _ROM_##fn args;                    \
        extern type (* fn) args

#else  /* BT_HAVE_PATCHABILITY */

#define DECL_PATCHABLE_DEFN(type, fn)           type fn
#define DECL_PATCHABLE_PROTO(type, fn, args)    type fn args

#endif /* BT_HAVE_PATCHABILITY */

/* Abstractions for String library functions */
#define BT_str_len(s)                 strlen((char *)(s))
#define BT_str_copy(d, s)             strcpy((char *)(d), (char *)(s))
#define BT_str_n_copy(d, s, n)        strncpy((char *)(d), (char *)(s), n)
#define BT_str_cmp(s1, s2)            strcmp((char *)(s1), (char *)(s2))
#define BT_str_n_cmp(s1, s2, n)       strncmp((char *)(s1), (char *)(s2), n)
#define BT_str_cat(d, s)              strcat((char *)(d), (char *)(s))
#define BT_str_str(s, ss)             strstr((char *)(s), (char *)(ss))
#define BT_str_n_casecmp(s1, s2, n)   _strnicmp ((char *)(s1), (char *)(s2), n)

/* Abstractions for memory functions */
#define BT_mem_move(d, s, n)          memmove((d), (s), (n))
#define BT_mem_cmp(p1, p2, n)         memcmp((p1), (p2), (n))
#define BT_mem_set(p, v, n)           memset((p), (v), (n))
#define BT_mem_copy(p1, p2, n)        memcpy((p1), (p2), (n))

/* -------------------------------------------- Function Declarations */
#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the OS */
void BT_os_init(void);

/* Task/Thread Creation Primitives */
INT32 BT_thread_create
      (
          /* OUT */ BT_thread_type *         thread,
          /* IN */  BT_thread_attr_type *    thread_attr,
          /* IN */  BT_THREAD_START_ROUTINE  start_routine,
          /* IN */  void *                   thread_args
      );

INT32 BT_thread_attr_init
      (
          /* OUT */ BT_thread_attr_type *    thread_attr
      );

/* Task/Thread Synchronization Primitives */
INT32 BT_thread_mutex_init
      (
          /* OUT */ BT_thread_mutex_type *         mutex,
          /* IN */  BT_thread_mutex_attr_type *    mutex_attr
      );

INT32 BT_thread_mutex_lock
      (
          /* INOUT */ BT_thread_mutex_type *    mutex
      );

INT32 BT_thread_mutex_unlock
      (
          /* INOUT */ BT_thread_mutex_type *    mutex
      );

INT32 BT_thread_cond_init
      (
          /* OUT */ BT_thread_cond_type *         cond,
          /* IN */  BT_thread_cond_attr_type *    cond_attr
      );

INT32 BT_thread_cond_wait
      (
          /* INOUT */ BT_thread_cond_type *     cond,
          /* INOUT */ BT_thread_mutex_type *    cond_mutex
      );

INT32 BT_thread_cond_signal
      (
          /* INOUT */ BT_thread_cond_type *    cond
      );

/* Memory Management Primitives */
void * BT_alloc_mem (/* IN */ UINT32 size);
void BT_free_mem (/* IN */ void * ptr);

/* Task/Thread Delay Primitives */
void BT_sleep ( /* IN */ UINT32 tm );
void BT_usleep ( /* IN */ UINT32 tm );
void BT_get_current_time (/* OUT */ BT_time_type * curtime);

/* Process termination handling */
void BT_process_term_notify(void(*handler)(void));

#ifdef __cplusplus
};
#endif


#endif /* _H_BT_OS_ */


