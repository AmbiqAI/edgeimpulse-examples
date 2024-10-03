
/*
 *  \file BT_bottom_half.h
 *
 *  This file lists all the Write Task Bottom Half function handlers
 *  under EtherMind Protocol Suite.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_BOTTOM_HALF_
#define _H_BT_BOTTOM_HALF_

/*
 *  Theory:
 *  There are three kinds of Bottom Halves, as provided by the EtherMind
 *  Write Task module:
 *    1. User Data Bottom Halves (eg, RFCOMM/BNEP Write)
 *    2. User API Bottom Halves
 *    3. Internal (eg, Timer/Internal Loopback handlers)
 *
 *  As required by the Write Task module, all EtherMind Bottom Halves
 *  have the same format:
 *      void bh_handler_function (void *args, UINT16 length);
 */

/* Macro to declare a Bottom Half handler */
#define BT_BOTTOM_HALF(x) \
        void x (void *, UINT16)


/* ============================================= User Data Bottom Halves */


/* ===================================================================== */


/* ============================================= User API Bottom Halves  */
#ifdef __cplusplus
extern "C"{
#endif

/* --------------------------------------------- Section 'HCI' */
BT_BOTTOM_HALF ( hci_common_api_bh );

/* --------------------------------------------- Section 'L2CAP' */
BT_BOTTOM_HALF ( l2ca_channel_data_write_bh );

/* --------------------------------------------- Section 'SMP' */
BT_BOTTOM_HALF ( smp_tbx_commands_bh );

/* ===================================================================== */


/* ============================================= Internal Bottom Halves  */

/* --------------------------------------------- Section 'HCI' */
BT_BOTTOM_HALF ( hci_command_tx_bh );

/* --------------------------------------------- Section 'L2CAP' */
BT_BOTTOM_HALF ( l2cap_data_transmitter_bh );

/* ===================================================================== */

#ifdef __cplusplus
};
#endif

#endif /* _H_BT_BOTTOM_HALF_ */

