
/**
 *  \file appl_gatt.h
 *
 *
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_GATT_
#define _H_APPL_GATT_

/* --------------------------------------------- Header File Inclusion */
#include "BT_api.h"
#include "BT_gatt_api.h"
/* --------------------------------------------- Global Definitions */

/* --------------------------------------------- Structures/Data Types */

/* --------------------------------------------- Macros */
#define APPL_GATT_CONFIG_SERVER                 0x01

#define APPL_GATT_READ_CHAR_DESC                0x0A
#define APPL_GATT_READ_CHAR_VAL                 0x0B
#define APPL_GATT_READ_MULTIPLE_CHAR_VAL        0x0C
#define APPL_GATT_READ_LONG_CHAR_DESC           0x0D
#define APPL_GATT_READ_LONG_CHAR_VAL            0x0E
#define APPL_GATT_READ_USING_UUID               0x0F

#define APPL_GATT_WRITE_CHAR_DESC               0x10
#define APPL_GATT_WRITE_CHAR_VAL                0x11
#define APPL_GATT_WRITE_CHAR_VAL_REL            0x12
#define APPL_GATT_WRITE_LONG_CHAR_DESC          0x13
#define APPL_GATT_WRITE_LONG_CHAR_VAL           0x14
#define APPL_GATT_WRITE_WITHOUT_RSP             0x15
#define APPL_GATT_WRITE_WITHOUT_RSP_AUTH        0x16


/* --------------------------------------------- Internal Functions */

/* --------------------------------------------- API Declarations */

/**
 *  \fn main_att_operations
 *
 *  \brief
 *
 *  \Description
 *
 *
 *  \return void 
 */
void main_att_client_operations();

/**
 *  \fn att_cb
 *
 *  \brief
 *
 *  \Description
 *
 *
 *  \param event_type
 *  \param event_result
 *  \param bd_addr
 *  \param event_data
 *  \param event_datalen
 *
 *  \return void 
 */
void gatt_cb
     (
         ATT_CONTEXT      context,
         BT_DEVICE_ADDR * bd_addr,
         UCHAR            command,
         API_RESULT       event_result,
         UCHAR            opcode,
         UCHAR          * event_data,
         UINT16           event_datalen
     );

void appl_gatt_common_cb
     (
         ATT_CONTEXT     context,
         BT_DEVICE_ADDR * bd_addr,
         UCHAR            event_type,
         API_RESULT       retval,
         UCHAR          * event_data,
         UCHAR            data_len
     );

void gatt_print_error
     (
         UCHAR       error_code,
         ATT_HANDLE  handle,
         UCHAR       gatt_command_used
     );

#endif /* _H_APPL_GATT_ */

