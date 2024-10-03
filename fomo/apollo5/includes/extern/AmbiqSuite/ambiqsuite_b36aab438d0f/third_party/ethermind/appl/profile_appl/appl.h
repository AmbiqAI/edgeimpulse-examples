
/**
 *  \file appl.h
 *
 *  Main Application Header File
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_
#define _H_APPL_

/* ----------------------------------------- Header File Inclusion */

#include "BT_common.h"
#include "BT_debug.h"
#include "l2cap.h"
#include "fsm_engine.h"

#include "appl_hci.h"
#include "appl_fsm.h"
#include "appl_fsm_defines.h"

#include "BT_api.h"
#include "BT_config.h"

#include "appl_gap.h"
#include "appl_att_server.h"
#include "appl_uri.h"

#if (defined BT_SW_TXP) && (defined APPL_MENU_OPS)
#include "host_interface.h"
#endif /* (defined BT_SW_TXP) && (defined APPL_MENU_OPS) */

#ifdef SMP
#include "appl_smp.h"
#endif /* SMP */

#ifdef CUSTOM_PROFILE
#include "appl_custom.h"
#endif

#ifdef HCI_TX_RUN_TIME_SELECTION
#ifdef BT_USB
#include "hci_usb.h"
#endif /* BT_USB */

#ifdef BT_UART
#include "hci_uart.h"
#endif /* BT_UART */

#ifdef BT_SOCKET
#include "hci_socket.h"
#endif /* BT_SOCKET */
#endif /* HCI_TX_RUN_TIME_SELECTION */

#ifdef HID
#include "appl_hid.h"
#endif /* HID */

#ifdef HTS
#include "appl_hts.h"
#endif /* HTS */

#ifdef HRS
#include "appl_hrs.h"
#endif /* HRS */

#ifdef GLS
#include "appl_gls.h"
#endif /* GLS */

#ifdef PXR
#include "appl_pxr.h"
#endif /* PXR */

#ifdef FMT
#include "appl_fmt.h"
#endif /* FMT */

#ifdef PASS
#include "appl_pass.h"
#endif /* PASS */

#ifdef CTS
#include "appl_cts.h"
#endif /* CTS */

#ifdef RTUS
#include "appl_rtus.h"
#endif /* RTUS */

#ifdef NDCS
#include "appl_ndcs.h"
#endif /* NDCS */

#ifdef ANS
#include "appl_ans.h"
#endif /* ANS */

#ifdef BPS
#include "appl_bps.h"
#endif /* BPS */

#ifdef RSCS
#include "appl_rscs.h"
#endif /* RSCS */

#ifdef CSCS
#include "appl_cscs.h"
#endif /* CSCS */

#ifdef POS
#include "appl_pos.h"
#endif /* POS */

#ifdef CPU_UTIL_CALC
#include "appl_cpu_util_calc.h"
#endif /* CPU_UTIL_CALC */

#ifdef VSE1
#include "appl_vse1.h"
#endif /* VSE1 */

#ifdef VSE2
#include "appl_vse2.h"
#endif /* VSE2 */

#ifdef VSE3
/* VSE3 related Application Header Inclusion */
#include "appl_vse3.h"
#endif /* VSE3 */

#ifdef VSE4
/* VSE4 related Application Header Inclusion */
#include "appl_vse4.h"
#endif /* VSE4 */

#ifdef VSE5
/* VSE5 related Application Header Inclusion */
#include "appl_vse5.h"
#endif /* VSE5 */

#ifdef VSE6
/* VSE6 related Application Header Inclusion */
#include "appl_vse6.h"
#endif /* VSE6 */

#ifdef VSE7
/* VSE7 related Application Header Inclusion */
#include "appl_vse7.h"
#endif /* VSE7 */

#ifdef VSE8
/* VSE8 related Application Header Inclusion */
#include "appl_vse8.h"
#endif /* VSE8 */

#ifdef SAMPLE_CLIENT
#include "appl_sample_client.h"
#endif /* SAMPLE_CLIENT */

#ifdef AIOS
#include "appl_aios.h"
#endif /* AIOS */

#ifdef CGMS
#include "appl_cgms.h"
#endif /* CGMS */

#ifdef CPMS
#include "appl_cpms.h"
#endif /* CPMS */

#ifdef LNS
#include "appl_lns.h"
#endif /* LNS */

#ifdef SPS
#include "appl_sps.h"
#endif /* SPS */

#ifdef WSS
#include "appl_wss.h"
#endif /* WSS */

#ifdef BMS
#include "appl_bms.h"
#endif /* BMS */

#ifdef IPSPN

#include "appl_ipspn.h"
#endif /* IPSPN */

#ifdef ESS
#include "appl_ess.h"
#endif /* ESS */

#ifdef AMOTAS
#include "appl_amota.h"
#endif /* AMOTAS */

#ifdef AMDTPS
#include "appl_amdtps.h"
#endif /* AMDTPS */

#ifdef TXPOWER_CTRL
#include "appl_txpower_ctrl.h"
#endif /* TXPOWER_CTRL */

#ifdef IBEACON
#include "appl_ibeacon.h"
#endif

#ifdef ANCS
#include "appl_ancs.h"
#endif /* ANCS */

#ifdef AMS
#include "appl_ams.h"
#endif /* AMS */

/* ----------------------------------------- Global Definitions */
#define BLE_TRUE    1
#define BLE_FALSE   0
#define APPL_ERR_ID                          0xEE00

#ifdef DONT_USE_STANDARD_IO
#ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
#define printf
#define sprintf
#define fprintf
#define scanf
#else
#define printf(...)
/*#define putchar(...)*/
#define fprintf(...)
#define scanf(...)
#endif  /*  VAR_ARG_IN_MACRO_NOT_SUPPORTED */
#endif /* DONT_USE_STANDARD_IO */

/* ----------------------------------------- Structures/Data Types */

#ifndef APPL_MENU_OPS
extern DECL_CONST BT_DEVICE_ADDR peer_bd_addr;
#else /* APPL_MENU_OPS */
extern BT_DEVICE_ADDR peer_bd_addr;
#endif /* APPL_MENU_OPS */

#ifdef APPL_CONFIG_LOCAL_DEV_ADDR_TYPE
extern UCHAR appl_local_dev_addr_type;

extern UCHAR local_static_address[];

/* Get the device address type as Public or Random */
#define APPL_LOCAL_DEV_ADDR_TYPE() \
        ((BT_BD_PUBLIC_ADDRESS_TYPE == appl_local_dev_addr_type) ? \
          BT_BD_PUBLIC_ADDRESS_TYPE : BT_BD_RANDOM_ADDRESS_TYPE)

/**
 * Get the detailed device address type as Public or
 * Random - Static, Resolvable Private, Non-resolvable Private
 */
#define APPL_PRECISE_LOCAL_DEV_ADDR_TYPE() appl_local_dev_addr_type

#else
#define APPL_LOCAL_DEV_ADDR_TYPE() BT_BD_PUBLIC_ADDRESS_TYPE
#define APPL_PRECISE_LOCAL_DEV_ADDR_TYPE() BT_BD_PUBLIC_ADDRESS_TYPE
#endif /* APPL_CONFIG_LOCAL_DEV_ADDR_TYPE */

extern APPL_PEER_INSTANCE appl_peer_device_list[];
extern UCHAR              appl_active_connections;
extern BT_timer_handle    appl_idle_timer;

#define SET_ALL_APPL_STATE(state) appl_set_all_state(state)
#define SET_APPL_STATE(inst,state) appl_set_state((inst),(state))
#define GET_APPL_STATE(inst) appl_peer_device_list[(inst)].state

#define APPL_CONFIG_PEER_DEVICE_ADDR()      ((UCHAR *)&peer_bd_addr.addr[0])
#define APPL_CONFIG_PEER_DEVICE_ADDR_TYPE() (peer_bd_addr.type)
#define APPL_DIRECT_ADDR_TYPE()             0
#define APPL_OWN_ADDR_TYPE()                APPL_LOCAL_DEV_ADDR_TYPE()

#define APPL_CONFIG_UART_PORT()             "\\\\.\\COM1"
#define APPL_CONFIG_UART_BAUDRATE()         115200

/* ----------------------------------------- Macros */

/* Encoding of application PDU parameters */
#define appl_pack_1_byte_param(dest, src) \
    *((dest) + 0) = (UCHAR)(*((UCHAR *)(src)));

#define appl_pack_2_byte_param(dest, src) \
    *((dest) + 0) = (UCHAR)(*((UINT16 *)(src))); \
    *((dest) + 1) = (UCHAR)(*((UINT16 *)(src)) >> 8);

#define appl_pack_3_byte_param(dest, src) \
    *((dest) + 0) = (UCHAR)(*((UINT32 *)(src)));\
    *((dest) + 1) = (UCHAR)(*((UINT32 *)(src)) >> 8);\
    *((dest) + 2) = (UCHAR)(*((UINT32 *)(src)) >> 16);

#define appl_pack_4_byte_param(dest, src) \
    *((dest) + 0) = (UCHAR)(*((UINT32 *)(src)));\
    *((dest) + 1) = (UCHAR)(*((UINT32 *)(src)) >> 8);\
    *((dest) + 2) = (UCHAR)(*((UINT32 *)(src)) >> 16);\
    *((dest) + 3) = (UCHAR)(*((UINT32 *)(src)) >> 24);

/* Decoding of application PDU parameters */
#define appl_unpack_1_byte_param(dest, src) \
        (dest) = (src)[0];

#define appl_unpack_2_byte_param(dest, src) \
        (dest)  = (src)[1]; \
        (dest)  = ((dest) << 8); \
        (dest) |= (src)[0];

#define appl_unpack_4_byte_param(dest, src) \
        (dest)  = (src)[3]; \
        (dest)  = ((dest) << 8); \
        (dest) |= (src)[2]; \
        (dest)  = ((dest) << 8); \
        (dest) |= (src)[1]; \
        (dest)  = ((dest) << 8); \
        (dest) |= (src)[0];

#define APPL_DEVICE_INIT(x)\
        ATT_INIT_HANDLE(appl_peer_device_list[(x)].att_handle);\
        appl_peer_device_list[(x)].cxn_handle = HCI_INVALID_CONNECTION_HANDLE;\
        appl_peer_device_list[(x)].mtu        = 0x0000;

#define APPL_GET_CONNECTION_HANDLE(x)\
        appl_peer_device_list[(x)].cxn_handle

/**
 *  Please note that device queue handle actually is independent of ATT
 *  instance handle. But since device queue is a part of ATT Handle, its
 *  being accessed through the ATT handle for avoiding redudant copies.
 */
#define APPL_GET_DEVICE_HANDLE(x)\
        appl_peer_device_list[(x)].att_handle.device_id

#define APPL_GET_ATT_INSTANCE(x)\
        appl_peer_device_list[(x)].att_handle

#define APPL_SET_ATT_INSTANCE(hndl,att_inst)\
        appl_peer_device_list[(hndl)].att_handle = (att_inst)

#define APPL_GET_MTU(x)\
        appl_peer_device_list[(x)].mtu

#define APPL_SET_MTU(h,m)\
        appl_peer_device_list[(h)].mtu = (m)

#define APPL_HANDLE_INIT(x)\
        (x) = BT_MAX_REMOTE_DEVICES

/* Initial Battery level 100% */
#define APPL_BATTERY_PERCENT    0x64

/* ----------------------------------------- Function Declarations */

void appl_set_all_state(APPL_STATE_T state);
void appl_set_state(APPL_HANDLE handle,APPL_STATE_T state);
UCHAR appl_is_device_bonded(void);
API_RESULT appl_battery_level_handler(GATT_DB_HANDLE * handle, UINT16 config);
void appl_update_battery_lvl (DEVICE_HANDLE  * dq_handle);
void appl_batt_timer_expiry_handler (void *data, UINT16 datalen);
void appl_set_target_address_in_adv_data (UCHAR offset);
void appl_set_user_given_target_address_in_adv_data (UCHAR offset);
void appl_set_user_given_service_solicitation_in_adv_data (UCHAR offset);
void appl_set_user_given_appearence_in_adv_data (UCHAR offset);
void appl_set_user_given_service_data_in_adv_data (UCHAR offset);
void appl_set_user_given_slave_ci_in_adv_data (UCHAR offset);
void appl_set_user_given_le_dev_addr_in_adv_data (UCHAR offset);
void appl_set_user_given_le_role_in_adv_data (UCHAR offset);
void appl_set_user_given_adv_int_in_adv_data (UCHAR offset);
void appl_set_user_given_sm_tk_in_adv_data (UCHAR offset);
void appl_set_user_given_oob_in_adv_data (UCHAR offset);
void appl_gap_get_addr_and_type(BT_DEVICE_ADDR * bd_addr);

/* Bluetooth ON Complete Callback */
API_RESULT appl_bluetooth_on_complete ( void );

/* Utility Functions */
void appl_get_bd_addr (UCHAR *bd_addr);
void appl_dump_bytes (UCHAR *buffer, UINT16 length);
void appl_idle_timer_expiry(void * data_param, UINT16 datalen);
#ifndef CUSTOM_PROFILE
void appl_get_local_time (/* OUT */ UCHAR *buf, /* IN */ UINT16 buf_len);
#endif /* CUSTOM_PROFILE */

API_RESULT appl_att_cb
           (
               ATT_HANDLE    * handle,
               UCHAR         att_event,
               API_RESULT    event_result,
               UCHAR         * event_data,
               UINT16        event_datalen
           );

API_RESULT appl_add_device
           (
                /* IN */  ATT_HANDLE     * att_handle,
                /* OUT */ APPL_HANDLE    * handle
           );

API_RESULT appl_get_handle_from_cxn_handle
           (
                /* IN */  UINT16         cxn_handle,
                /* OUT */ APPL_HANDLE    * handle
           );

API_RESULT appl_set_cxn_handle
           (
               /* IN */ UINT16            cxn_handle,
               /* IN */ DEVICE_HANDLE     dq_handle,
               /* OUT */ APPL_HANDLE      * handle
           );

API_RESULT appl_get_handle_from_device_handle
           (
                /* IN */  DEVICE_HANDLE     dq_handle,
                /* OUT */ APPL_HANDLE       * handle
           );

API_RESULT appl_delete_device
           (
                /* IN */ UINT16 cxn_handle
           );

API_RESULT appl_set_att_instance
           (
               /* IN */  ATT_HANDLE     * att_instance,
               /* OUT */ APPL_HANDLE    * handle
           );
#ifdef BT_SW_TXP
int appl_init(void);
void appl_message_handler(BT_MSG_T *btmsg);
#endif /* BT_SW_TXP */

#ifdef APPL_MENU_OPS
#ifdef BT_SW_TXP
const char * appl_get_profile_menu (void);
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

#if defined APPL_MENU_OPS && !defined CONSOLE_MASTER_SLAVE
void appl_store_peer_dev_addr(BT_DEVICE_ADDR * peer_div_addr);
void appl_disconnect_device(void);
void appl_power_on_device(void);
#endif /* defined APPL_MENU_OPS && !defined CONSOLE_MASTER_SLAVE */

#ifdef DONT_USE_STANDARD_IO
#ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
#define APPL_ERR
#define APPL_TRC
#else
#define APPL_ERR(...)
#define APPL_TRC(...)
#endif /* VAR_ARG_IN_MACRO_NOT_SUPPORTED */
#ifndef BT_SW_TXP
#define CONSOLE_OUT(...)
#else
#ifdef APPL_MENU_OPS
#define CONSOLE_OUT console_printf
#else /* APPL_MENU_OPS */
#define CONSOLE_OUT(...)
#endif /* APPL_MENU_OPS */
#endif /* BT_SW_TXP */
#ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
#define CONSOLE_IN
#else
#define CONSOLE_IN(...)
#endif /* VAR_ARG_IN_MACRO_NOT_SUPPORTED */
#else /* DONT_USE_STANDARD_IO */
#define APPL_ERR     printf
#define APPL_TRC     printf
#define CONSOLE_OUT  printf
#define CONSOLE_IN   scanf
#endif /* DONT_USE_STANDARD_IO */

#ifndef APPL_USE_IDLE_TIMER
#define APPL_START_IDLE_TIMER(...)
#define APPL_STOP_IDLE_TIMER(...)
#else /* APPL_USE_IDLE_TIMER */
#ifdef DONT_USE_STANDARD_IO
#define APPL_START_IDLE_TIMER()\
        {\
             APPL_STOP_IDLE_TIMER();\
             BT_start_timer\
             (\
                 &appl_idle_timer,\
                 APPL_IDLE_TIMEOUT,\
                 appl_idle_timer_expiry,\
                 NULL,\
                 0\
             );\
             APPL_TRC (\
             "[APPL]: Started Idle Timer %p with result 0x%04X\n",\
             appl_idle_timer,timer_retval);\
        }
#else /* DONT_USE_STANDARD_IO */
#define APPL_START_IDLE_TIMER()\
        {\
             API_RESULT   timer_retval;\
             APPL_STOP_IDLE_TIMER();\
             timer_retval = BT_start_timer\
                            (\
                                &appl_idle_timer,\
                                APPL_IDLE_TIMEOUT,\
                                appl_idle_timer_expiry,\
                                NULL,\
                                0\
                            );\
             APPL_TRC (\
             "[APPL]: Started Idle Timer %p with result 0x%04X\n",\
             appl_idle_timer,timer_retval);\
        }
#endif /* DONT_USE_STANDARD_IO */

#define APPL_STOP_IDLE_TIMER()\
        if (BT_TIMER_HANDLE_INIT_VAL != appl_idle_timer)\
        {\
             APPL_TRC (\
             "[APPL]: Stopping Idle Timer %p\n",appl_idle_timer);\
             BT_stop_timer (appl_idle_timer);\
             appl_idle_timer = BT_TIMER_HANDLE_INIT_VAL;\
        }
#endif /* APPL_USE_IDLE_TIMER */

#ifndef SMP
#define appl_smp_assert_security(a, b, c) API_SUCCESS
#endif /* SMP */

void appl_set_uri_in_adv_data (UCHAR offset);

#endif /* _H_APPL_ */

