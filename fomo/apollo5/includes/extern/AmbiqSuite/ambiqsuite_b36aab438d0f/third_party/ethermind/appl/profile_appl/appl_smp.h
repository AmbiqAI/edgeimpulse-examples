/**
 *  \file appl_smp.h
 *
 *
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_SMP_
#define _H_APPL_SMP_

/* --------------------------------------------- Header File Inclusion */
#include "BT_smp_api.h"
#include "smp_pl.h"

/* Repeat Tracking Feature Enable */
#define HAVE_SMP_REPEAT_TRACKING

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */
#ifdef HAVE_SMP_REPEAT_TRACKING
extern BT_timer_handle smp_repeat_timer;
#endif /* HAVE_SMP_REPEAT_TRACKING */

/* --------------------------------------------- Static Global Variables */
void appl_smp_reset (void);
void appl_smp_generate_sign_data (UCHAR * data, UINT16 datalen);
void appl_smp_verify_sign_data (UCHAR * data, UINT16 datalen);

API_RESULT appl_smp_cb
           (
                SMP_BD_HANDLE * bd_handle,
                UCHAR         event,
                API_RESULT    status,
                void        * eventdata,
                UINT16        data_len
           );


void appl_smp_register (void);

void appl_smp_configure_platform (void);

API_RESULT appl_smp_assert_security
           (
               SMP_BD_HANDLE * handle,
               UCHAR           security,
               UCHAR           ekey_size
           );

API_RESULT appl_smp_check_security_access
          (
              SMP_BD_HANDLE * handle,
              UCHAR sec_level,
              UCHAR ekey_size
          );

API_RESULT appl_smp_security_request
          (
              SMP_BD_HANDLE * handle,
              SMP_AUTH_INFO * auth
          );


#endif /* _H_APPL_SMP_ */

