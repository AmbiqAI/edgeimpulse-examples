
/**
 *  \file BT_api.h
 *
 *  This file contains the API Header Files of the EtherMind Protocols
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_API_
#define _H_BT_API_

/* ---------------------------------------------- Global Definitions */
#include "BT_common.h"

#include "BT_hci_api.h"
#ifdef SMP
#include "BT_smp_api.h"
#endif /* SMP */

#ifdef ATT
#include "BT_att_api.h"
#endif /* ATT */

/* ---------------------------------------------- Global Definitions */


/* ---------------------------------------------- Structures/Data Types */


#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------- API Declarations */
/* To set Serial Port settings for HCI-UART Transport */
void hci_uart_set_serial_settings (CHAR * device, UINT32 speed);

/* To set USB Port settings for HCI-USB Transport */
void hci_usb_set_serial_settings (CHAR * device, UINT32 speed);

#ifdef __cplusplus
};
#endif


#endif /* _H_BT_API_ */

