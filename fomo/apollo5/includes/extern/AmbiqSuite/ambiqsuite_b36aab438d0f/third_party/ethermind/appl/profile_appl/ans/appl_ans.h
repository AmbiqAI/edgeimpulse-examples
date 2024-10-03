
/**
 *  \file appl_ans.h
 *
 *  Application Header File for Alert Notification.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_ANS_
#define _H_APPL_ANS_

/* ----------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_db_api.h"
#include "gatt_db.h"
#include "appl.h"

/* Specification Defined Alert Categories */
#define ANS_SIMPLE_ALRT_CTGRY                      0x01
#define ANS_EMAIL_ALRT_CTGRY                       0x02
#define ANS_NEWS_ALRT_CTGRY                        0x04
#define ANS_CALL_ALRT_CTGRY                        0x08
#define ANS_MISSED_CALL_ALRT_CTGRY                 0x10
#define ANS_SMS_MMS_ALRT_CTGRY                     0x20
#define ANS_VOICE_MAIL_ALRT_CTGRY                  0x40
#define ANS_SCHEDULE_ALRT_CTGRY                    0x80

/* Specification Defined Category ID */
#define ANS_HIGH_PRIORITY_ALRT                     0x01
#define ANS_INSTANT_MESSAGE                        0x02

/* Supported Alert Category Selection Defines */

#if 0
#define ANS_ALL_ALRT_CTGRY\
        ANS_SIMPLE_ALRT_CTGRY | ANS_EMAIL_ALRT_CTGRY | ANS_NEWS_ALRT_CTGRY |\
        ANS_CALL_ALRT_CTGRY | ANS_MISSED_CALL_ALRT_CTGRY |\
        ANS_SMS_MMS_ALRT_CTGRY | ANS_VOICE_MAIL_ALRT_CTGRY |\
        ANS_SCHEDULE_ALRT_CTGRY
#endif /* 0 */

/**
 * Changed number of Supported Alert Categories from 8 to 7 in order to
 * Operate certain testcases with Test Tool
 */
#define ANS_ALL_ALRT_CTGRY\
        ANS_SIMPLE_ALRT_CTGRY | ANS_EMAIL_ALRT_CTGRY | ANS_NEWS_ALRT_CTGRY |\
        ANS_CALL_ALRT_CTGRY | ANS_MISSED_CALL_ALRT_CTGRY |\
        ANS_SMS_MMS_ALRT_CTGRY | ANS_VOICE_MAIL_ALRT_CTGRY

#define ANS_CTGRY_ID_NONE                         0x00

#define ANS_SUPPORTED_NEW_ALRT_CTGRY    ANS_SIMPLE_ALRT_CTGRY /* ANS_ALL_ALRT_CTGRY */
#define ANS_SUPPORTED_NEW_ALRT_CTGRY_ID ANS_CTGRY_ID_NONE

#define ANS_SUPPORTED_UNREAD_ALRT_CTGRY    ANS_SIMPLE_ALRT_CTGRY /* ANS_ALL_ALRT_CTGRY */
#define ANS_SUPPORTED_UNREAD_ALRT_CTGRY_ID ANS_CTGRY_ID_NONE

/** Command ID */
#define ENABLE_NEW_INCOMING_ALRT_NTF                  0
#define ENABLE_UNREAD_CATEGORY_STATUS_NTF             1
#define DISABLE_NEW_INCOMING_ALRT_NTF                 2
#define DISABLE_UNREAD_CATEGORY_STATUS_NTF            3
#define NOTIFY_NEW_INCOMING_ALRT_IMMEDIATELY          4
#define NOTIFY_UNREAD_CATEGORY_STATUS_IMMEDIATELY     5

#define ALRT_NTF_CNTRL_PNT_CMD_NOT_SUPPORTED      (APPL_ERR_ID | (0xA0))

#define appl_set_bit_value(var,pos)\
        (var) = (var) | (1 << (pos))

#define appl_clear_bit_value(var,pos)\
        (var) = (var) & (~(1 << (pos)))

#define appl_get_bit_value(var,pos)\
        ((var) & (1 << (pos)))?1:0

/* --------------------------------------------- Functions */
void appl_ans_init (void);
void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config);
void appl_new_alert_timer_expiry_handler (void *data, UINT16 datalen);
void appl_unread_alert_timer_expiry_handler (void *data, UINT16 datalen);
void appl_ans_connect (APPL_HANDLE  * appl_handle);
API_RESULT appl_alrt_ntf_cntrl_point
           (GATT_DB_HANDLE handle, UCHAR command_id, UCHAR category_id);
void appl_send_alert_ntf
     (
         APPL_HANDLE * handle,
         ATT_ATTR_HANDLE * appl_alert_handle,
         UCHAR * alert_value,
         UINT16 len
     );

void appl_send_notification (APPL_HANDLE * handle);
void appl_ans_server_reinitialize (void);
void appl_profile_menu_handler (void);

/* Profile handling */
#define APPL_PROFILE_INIT(...) appl_ans_init()
#define APPL_PROFILE_CONNECT(x) appl_ans_connect(x)
#define APPL_SEND_MEASUREMENT(x) appl_send_notification(x)
#define APPL_PROFILE_DISCONNECT_HANDLER(x) appl_ans_server_reinitialize()
#define GATT_DB_PROFILE_HANDLER  gatt_db_ans_handler
#define APPL_PROFILE_HVN_NTF_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_HVN_IND_COMPLETE_HANDLER(x,y,z)
#define APPL_PROFILE_MTU_UPDT_COMPLETE_HANDLER(x,y)

#define APPL_USE_IDLE_TIMER
#define APPL_IDLE_TIMEOUT           30
#endif /* _H_APPL_ANS_ */

