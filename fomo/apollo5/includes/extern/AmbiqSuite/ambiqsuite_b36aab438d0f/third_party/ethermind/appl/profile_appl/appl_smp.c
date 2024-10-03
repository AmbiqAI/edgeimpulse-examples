/**
 *  \file appl_smp.c
 *
 *
 */

/*
 *  Copyright (C) 2014. Mindtree Ltd.
 *  All rights reserved.
 */


/* --------------------------------------------- Header File Inclusion */
#include "appl.h"
#include "appl_fsm_defines.h"

#ifdef SMP

/* --------------------------------------------- External Global Variables */
/* Flag to control Passkey updation from Application */
/* #define APPL_SMP_UPDATE_PASSKEY */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */
/* Application automatic SMP handling identifier */
DECL_CONST UCHAR automatic_mode = SMP_TRUE;

/* Application SMP pairable mode */
DECL_CONST UCHAR pairable_mode = SMP_TRUE;

#ifdef HAVE_SMP_REPEAT_TRACKING
/* Application Repeat Attempt track timer */
#define SMP_REPEAT_ATTEMPT_TIMEOUT_MIN        8
#define SMP_REPEAT_ATTEMPT_TIMEOUT_MAX        1024
#define SMP_REPEAT_ATTEMPT_TIMEOUT_FRACTION   1 /* Power of 2 */

BT_timer_handle smp_repeat_timer = BT_TIMER_HANDLE_INIT_VAL;
DECL_STATIC void smp_repeat_timeout_handler (void * data_param, UINT16 datalen);
DECL_STATIC UINT32 smp_repeat_attempt_timeout = SMP_REPEAT_ATTEMPT_TIMEOUT_MIN;
#endif /* HAVE_SMP_REPEAT_TRACKING */

#ifdef SMP_DATA_SIGNING
DECL_STATIC UCHAR * sign_data = NULL;
DECL_STATIC UINT16 sign_datalen;
DECL_STATIC UINT32 sign_create_counter;
DECL_STATIC UINT32 sign_verify_counter;
DECL_STATIC UCHAR * sign_key;
DECL_STATIC UCHAR sign_mac[SMP_MAC_SIZE];
DECL_STATIC UCHAR signature[sizeof(sign_create_counter) + SMP_MAC_SIZE];
DECL_STATIC SMP_KEY_DIST peer_key_info;
#endif /* SMP_DATA_SIGNING */
#ifdef APPL_SMP_UPDATE_PASSKEY
#define APPL_SMP_PASSKEY                        999999
#endif /* APPL_SMP_UPDATE_PASSKEY */

/* --------------------------------------------- Functions */
void appl_smp_configure_platform (void)
{
    SMP_BD_ADDR l_addr;

    /* Update local SMP capabilities using smp_pl APIs if required here */
    BT_smp_set_io_cap_pl(SMP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

    /* Setting the local Public Address as Identity Address */
    BT_hci_get_local_bd_addr
    (
        l_addr.addr
    );
    l_addr.type = BT_BD_PUBLIC_ADDRESS_TYPE;

    BT_smp_set_local_identity_addr(&l_addr);
}


API_RESULT appl_smp_assert_security
           (
               SMP_BD_HANDLE * handle,
               UCHAR           security,
               UCHAR           ekey_size
           )
{
    API_RESULT retval;

#ifdef APPL_PERIPHERAL_INITIATE_SECURITY_REQUEST
    SMP_AUTH_INFO  auth;
    APPL_EVENT_PARAM    fsm_param;
    APPL_HANDLE    appl_handle;
#endif /* APPL_PERIPHERAL_INITIATE_SECURITY_REQUEST */

    /* Verify if security requirements are available with the link */
    retval = appl_smp_check_security_access
             (
                 handle,
                 (security & SMP_SEC_LEVEL_MASK),
                 ekey_size
             );

    /* Security requirements not satisfied */
    if (API_SUCCESS != retval)
    {
#ifdef APPL_PERIPHERAL_INITIATE_SECURITY_REQUEST
        APPL_EVENT_PARAM_INIT(&fsm_param);

        /* Get required security for service */
        auth.security = security;
        auth.ekey_size = ekey_size;

        /*
         * Set bond requirement
         *
         * TODO: Take this from some application configuration?
         */
        auth.bonding = SMP_BONDING;

        appl_get_handle_from_device_handle (handle.device_id,&appl_handle);
        fsm_param.handle = appl_handle;
        fsm_param.data = &auth;
        fsm_param.data_len = sizeof (SMP_AUTH_INFO);
        fsm_param.direction = 0x00;

        /* Post Transport Configuration Request for SMP */
        fsm_post_event
        (
             APPL_FSM_ID,
             ev_appl_transport_configuration_req,
             (void *)(&fsm_param)
        );
#endif /* APPL_PERIPHERAL_INITIATE_SECURITY_REQUEST */
    }

    return retval;
}


API_RESULT appl_smp_check_security_access
          (
              SMP_BD_HANDLE * handle,
              UCHAR sec_level,
              UCHAR ekey_size
          )
{
    SMP_AUTH_INFO info;
    API_RESULT retval;

    /* TODO: Have proper application error codes */

    if (0x00 == sec_level)
    {
        return API_SUCCESS;
    }

    /* Get current security state for the link */
    retval = BT_smp_get_device_security_info (handle, &info);

    if (API_SUCCESS == retval)
    {
        /* Check if the security requirement is sufficient */
        if ((info.param != SMP_ENTITY_AUTH_ON) ||
            (info.security < (sec_level & SMP_SEC_LEVEL_MASK)) ||
            (info.ekey_size < ekey_size))
        {
            return API_FAILURE;
        }

        return API_SUCCESS;
    }

    return API_FAILURE;
}

API_RESULT appl_smp_security_request
           (
               SMP_BD_HANDLE * handle,
               SMP_AUTH_INFO * auth
           )
{
    /*
     * TODO:
     *
     * If bonding is remembered in application and BT_smp_encrypt is exposed
     * in SMP, then have check here whether to call BT_smp_authenticate or
     * BT_smp_encrypt by getting db data from the handle reference
     */

#ifndef SMP_STORAGE
     /* If storage not supported, make no-bonding */
     auth->bonding = SMP_BONDING_NONE;
#endif /* SMP_STORAGE */

    /* Request for authentication to SMP */
    return BT_smp_authenticate (handle, auth);
}

#ifdef SMP_DATA_SIGNING
void appl_smp_generate_sign_data (UCHAR * data, UINT16 datalen)
{
    SMP_KEY_DIST * key_info;

    /* Allocate signing data buffer */
    sign_datalen = datalen + sizeof (sign_create_counter);
    sign_data = BT_alloc_mem (sign_datalen);

    if (NULL != sign_data)
    {
        /* Copy the application data */
        BT_mem_copy (sign_data, data, datalen);

        /* Update the sign counter */
        BT_PACK_LE_4_BYTE(&sign_data[datalen], &sign_create_counter);
        BT_PACK_LE_4_BYTE(signature, &sign_create_counter);

        /* Get the platform signing key */
        BT_smp_get_key_exchange_info_pl (&key_info);
        sign_key = key_info->sign_info;

        /* Call to sign the data */
        BT_smp_sign_data(sign_data, sign_datalen, sign_key, sign_mac);
    }
}


void appl_smp_verify_sign_data (UCHAR * data, UINT16 datalen)
{
    UINT32 counter;
    UCHAR index;

    /* Validate counter */
    index = (UCHAR) (datalen - (SMP_MAC_SIZE + sizeof (sign_verify_counter)));
    BT_UNPACK_LE_4_BYTE(&counter, &data[index]);
    if (counter != sign_verify_counter)
    {
        /* Received counter in signature not matching with expected counter */
        return;
    }

    /* Allocate signing data buffer */
    sign_datalen = datalen - SMP_MAC_SIZE;
    sign_data = BT_alloc_mem (sign_datalen);

    if (NULL != sign_data)
    {
        /* Copy the application data */
        BT_mem_copy (sign_data, data, sign_datalen);

        /* Update the MAC with received MAC */
        BT_mem_copy (sign_mac, &data[sign_datalen], SMP_MAC_SIZE);

        /* Get the peer device signing key */
        sign_key = peer_key_info.sign_info;

        /* Call to verify the sign data */
        BT_smp_verify_sign_data (sign_data, sign_datalen, sign_key, sign_mac);
    }
}
#endif /* SMP_DATA_SIGNING */

void appl_smp_reset (void)
{
#ifdef SMP_DATA_SIGNING
    sign_create_counter = 0;
    sign_verify_counter = 0;
#endif /* SMP_DATA_SIGNING */
}

void appl_smp_register (void)
{
    BT_smp_register_user_interface (appl_smp_cb);
}


API_RESULT appl_smp_cb
           (
               /* IN */ SMP_BD_HANDLE   * bd_handle,
               /* IN */ UCHAR      event,
               /* IN */ API_RESULT status,
               /* IN */ void     * eventdata,
               /* IN */ UINT16     data_len
           )
{
    API_RESULT retval;

#ifdef SMP_SLAVE
    UINT16   ediv;
    UCHAR  * peer_rand;
    UCHAR    ltk[SMP_LTK_SIZE];
    UCHAR ltk_null;
#endif /* SMP_SLAVE */

    SMP_KEY_DIST * key_info;
    SMP_KEY_DIST local_key_info;
    SMP_AUTH_INFO * auth, info;
    SMP_BD_ADDR bdaddr;
    SMP_KEY_XCHG_PARAM * kx_param;
    APPL_EVENT_PARAM    fsm_param;
    APPL_HANDLE   appl_handle;

#ifndef DONT_USE_STANDARD_IO
    UCHAR * bd_addr;
    UCHAR   bd_addr_type;
#endif /* DONT_USE_STANDARD_IO */

    UCHAR * event_data = (UCHAR *)eventdata;

    SMP_KEY_DIST  p_key_info;
    UCHAR         p_keys;

    retval = API_SUCCESS;

    /* Get the BD Address from handle */
    BT_smp_get_bd_addr (bd_handle, &bdaddr);

#ifndef DONT_USE_STANDARD_IO
    bd_addr = bdaddr.addr;
    bd_addr_type = bdaddr.type;
#endif /* DONT_USE_STANDARD_IO */

    switch(event)
    {
    case SMP_AUTHENTICATION_COMPLETE:
        APPL_TRC("\nRecvd SMP_AUTHENTICATION_COMPLETE\n");
#ifndef DONT_USE_STANDARD_IO
        APPL_TRC("BD Address : %02X %02X %02X %02X %02X %02X\n",
        bd_addr[0],bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
        APPL_TRC("BD addr type : %s\n",
        (0 == bd_addr_type)? "Public Address": "Random Address");
#endif /* DONT_USE_STANDARD_IO */
        APPL_TRC("Status : %04X\n", status);

        appl_get_handle_from_device_handle (*bd_handle,&appl_handle);
        fsm_param.handle = appl_handle;
        fsm_param.data_param = NULL;
        fsm_param.data_len = 0;
        fsm_param.direction = 0xFF;

        /* The direction member of FSM Event holds the status of Authentication Procedure here */
        if (NULL != event_data)
        {
            if(API_SUCCESS == status)
            {
                auth = (SMP_AUTH_INFO *)event_data;

                APPL_TRC("Authentication type : %s\n",
                (SMP_SEC_LEVEL_2 == (auth->security & 0x0F))?  "With MITM":
                "Encryption Only (without MITM)");

#ifdef SMP_LESC
                APPL_TRC("Pairing Mode : %s\n",
                (SMP_LESC_MODE == (auth->pair_mode))? "LE SEC Pairing Mode":
                "LEGACY Pairing Mode");
#endif /* SMP_LESC */

                APPL_TRC("Bonding type : %s\n",
                (auth->bonding)? "Bonding": "Non-Bonding");

                APPL_TRC("Encryption Key size : %d\n", auth->ekey_size);

#ifdef HAVE_SMP_REPEAT_TRACKING
                /* Decrease timeout value by the fraction */
                if (SMP_REPEAT_ATTEMPT_TIMEOUT_MIN < smp_repeat_attempt_timeout)
                {
                    smp_repeat_attempt_timeout >>= SMP_REPEAT_ATTEMPT_TIMEOUT_FRACTION;
                }
#endif /* HAVE_SMP_REPEAT_TRACKING */

                fsm_param.data_param = (UCHAR *)auth;
                fsm_param.data_len = sizeof (SMP_AUTH_INFO);

                /* Update status in direction parameter */
                fsm_param.direction = 0x00;
            }
#ifdef HAVE_SMP_REPEAT_TRACKING
            else
            {
                if (BT_TIMER_HANDLE_INIT_VAL == smp_repeat_timer)
                {
                    /* Start timer to track repeat attempt */
                    BT_start_timer
                    (
                        &smp_repeat_timer,
                        smp_repeat_attempt_timeout,
                        smp_repeat_timeout_handler,
                        NULL, 0
                    );

                    APPL_TRC ("Repeat Tracking Timer Started for %d seconds.\n",
                        smp_repeat_attempt_timeout);

                    /* Increase timeout value by the fraction */
                    if (SMP_REPEAT_ATTEMPT_TIMEOUT_MAX > smp_repeat_attempt_timeout)
                    {
                        smp_repeat_attempt_timeout <<= SMP_REPEAT_ATTEMPT_TIMEOUT_FRACTION;
                    }
                }
            }
#endif /* HAVE_SMP_REPEAT_TRACKING */
        }
        else
        {
            if (API_SUCCESS == status)
            {
                APPL_TRC("\nConfirmed Authentication using Encryption\n");

                /* Update status in direction parameter */
                fsm_param.direction = 0x00;
            }
            else if (SMP_REMOTE_SIDE_PIN_KEY_MISSING == status)
            {
                APPL_TRC("\nPeer Device Lost previous Bonding Information!\n");
                APPL_TRC("\nDeleting Local Bond Information of Peer...\n");

                retval = BT_smp_mark_device_untrusted_pl(bd_handle);

                APPL_TRC("Marked Device Untrusted with result 0x%04X",retval);

                if (API_SUCCESS == retval)
                {
                    APPL_TRC("Initiate Pairing Again...");
                }
            }
        }

        /* Post Transport Configuration Ind from SMP */
        fsm_post_event
        (
             APPL_FSM_ID,
             ev_appl_transport_configuration_ind,
             (void *)(&fsm_param)
        );

        break;

    case SMP_AUTHENTICATION_REQUEST:
        APPL_TRC("\nRecvd SMP_AUTHENTICATION_REQUEST\n");
#ifndef DONT_USE_STANDARD_IO
        APPL_TRC("BD Address : %02X %02X %02X %02X %02X %02X\n",
        bd_addr[0],bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);

        APPL_TRC("BD addr type : %s\n",
        (0 == bd_addr_type)? "Public Address": "Random Address");
#endif /* DONT_USE_STANDARD_IO */

        auth = (SMP_AUTH_INFO *)event_data;

        APPL_TRC("Authentication type : %s\n",
        (SMP_SEC_LEVEL_2 == (auth->security & 0x0F))?  "With MITM":
        "Encryption Only (without MITM)");

        APPL_TRC("Bonding type : %s\n",
        (auth->bonding)? "Bonding": "Non-Bonding");

#ifdef SMP_LESC
        APPL_TRC("Pairing Mode : %s\n",
        (SMP_LESC_MODE == (auth->pair_mode))? "LE SEC Pairing Mode":
        "LEGACY Pairing Mode");
#endif /* SMP_LESC */

        APPL_TRC("Encryption Key size : %d\n", auth->ekey_size);

        /* Check if automatic mode is set */
        if(SMP_TRUE == automatic_mode)
        {
            /* Get the application handle reference */
            appl_get_handle_from_device_handle (*bd_handle,&appl_handle);

            /* Get current security state for the link */
            retval = BT_smp_get_device_security_info (bd_handle, &info);
            if (API_SUCCESS == retval)
            {
                /* Check if the link is authenticated */
                if (info.param != SMP_ENTITY_AUTH_ON)
                {
                    /**
                     *  Here, the application logic selected is to delete
                     *  bond of an exisitng peer device from the bonded
                     *  device list on receiving an Authentication Request
                     *  from it again on an UnAuthenticated link.
                     *  i.e. If a device initiates Pairing again after a
                     *  disconnection, even though it was previously bonded.
                     *  This could happen, if the peer device has lost the
                     *  Bonding informations inbetween connections.
                     *  Typically, popular smart phones/commercial devices and application
                     *  guidelines will not delete the bonding information here
                     *  without the approval/intervention from the user and
                     *  send out a "PAIRING FAILED" error with error code
                     *   - "SMP_ERROR_UNSPECIFIED_REASON", or
                     *   - "SMP_ERROR_PAIRING_NOT_SUPPORTED".
                     *  Also, such implementations would have some UI control to delete the
                     *  bonding on local side (eg: Button press combinations
                     *  etc.).
                     *  The following application logic is added, in this example
                     *  application which can also run on embedded or constrained system
                     *  without scope of User Intervention.
                     *
                     *  Application writer, needs to choose a better
                     *  alternative approach instead of deleteing peer bond information
                     *  automatically.
                     *
                     *  NOTE-1: This application logic/rational choosen for ease
                     *  of use, has security holes and power inefficiency as
                     *  pairing is allowed to happen multiple times with the
                     *  same device.
                     *
                     *  NOTE-2: Inorder to send pairing failure in case of
                     *  pairing request from an existing bonded device,
                     *  the application can set,
                     *  auth->param = SMP_ERROR_UNSPECIFIED_REASON or
                     *  auth->param = SMP_ERROR_PAIRING_NOT_SUPPORTED
                     *
                     *  while calling BT_smp_authentication_request_reply(...) API
                     *  is called.
                     */
                    /* Check if bonded */
                    if (SMP_BONDING == info.bonding)
                    {
                        /**
                         * The application logic choosen here is to delete the
                         * bonding information of the peer device.
                         * This needs to be modified according to the
                         * suitable requirements and platform capabilities
                         * according to the Application writer as explained in
                         * the above comment.
                         */
                        retval = BT_smp_mark_device_untrusted_pl (bd_handle);

                        if (API_SUCCESS == retval)
                        {
                            /**
                             * On Successfull deletion of bond of a given peer
                             * device. We are clearning the data maintained by
                             * GATT with respect to that peer.
                             * This will reset all the CCCD etc. which are
                             * configured by that peer.
                             * i.e. all the "Indications" or "Notifications"
                             * enabled will be "Turned-OFF" at this location.
                             *
                             * As mentioned above, this section also needs to
                             * be modified according to the requirements and
                             * platform capabilities by the Application writer.
                             */
                            BT_gatt_db_peer_session_shutdown_handler(&(APPL_GET_ATT_INSTANCE(appl_handle)));
                        }
                    }
                }
            }

            /* Accept Authentication */
            auth->param = SMP_ERROR_NONE;

#ifndef SMP_STORAGE
            /* If storage not supported, make no-bonding */
            auth->bonding = SMP_BONDING_NONE;
#endif /* SMP_STORAGE */

#ifdef HAVE_SMP_REPEAT_TRACKING
            /* Check if repeat attempt tracking is on */
            if (BT_TIMER_HANDLE_INIT_VAL != smp_repeat_timer)
            {
                auth->param = SMP_ERROR_REPEATED_ATTEMTS;
            } else
#endif /* HAVE_SMP_REPEAT_TRACKING */
            /* Check if in pairable mode */
            if (SMP_TRUE != pairable_mode)
            {
                auth->param = SMP_ERROR_PAIRING_NOT_SUPPORTED;
            }

            APPL_TRC("\n\nSending +ve Authentication request reply.\n");
            retval = BT_smp_authentication_request_reply
                     (
                         bd_handle,
                         auth
                     );

            /* Notify application state machine */
            fsm_param.handle = appl_handle;
            fsm_param.data_param = NULL;
            fsm_param.data_len = 0;
            fsm_param.direction = 0x01;

            /* Post Transport Configuration Request for SMP */
            fsm_post_event
            (
                 APPL_FSM_ID,
                 ev_appl_transport_configuration_req,
                 (void *)(&fsm_param)
            );
        }
        break;

    case SMP_PASSKEY_ENTRY_REQUEST:
        APPL_TRC("\nEvent   : SMP_PASSKEY_ENTRY_REQUEST\n");
#ifndef DONT_USE_STANDARD_IO
        APPL_TRC("BD Address : %02X %02X %02X %02X %02X %02X\n",
        bd_addr[0],bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
        APPL_TRC("BD addr type : %s\n",
        (0 == bd_addr_type)? "Public Address": "Random Address");
#endif /* DONT_USE_STANDARD_IO */
        break;

    case SMP_PASSKEY_DISPLAY_REQUEST:
        APPL_TRC("\nEvent   : SMP_PASSKEY_DISPLAY_REQUEST\n");
#ifndef DONT_USE_STANDARD_IO
        APPL_TRC("BD Address : %02X %02X %02X %02X %02X %02X\n",
        bd_addr[0],bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
        APPL_TRC("BD addr type : %s\n",
        (0 == bd_addr_type)? "Public Address": "Random Address");
#endif /* DONT_USE_STANDARD_IO */

        APPL_TRC("Passkey Received: %06u", *((UINT32 *)event_data));

#ifdef APPL_SMP_UPDATE_PASSKEY
        /* Update a 6 digit passkey - Max 999999 */
        *((UINT32 *)event_data) = APPL_SMP_PASSKEY;

        APPL_TRC("Updated Passkey: %06u", *((UINT32 *)event_data));

        retval = SMP_DISPLAY_PASSKEY_UPDATED;
#endif /* APPL_SMP_UPDATE_PASSKEY */

        break;

    case SMP_KEY_EXCHANGE_INFO_REQUEST:
        APPL_TRC("\nEvent   : SMP_KEY_EXCHANGE_INFO_REQUEST\n");
#ifndef DONT_USE_STANDARD_IO
        APPL_TRC("BD Address : %02X %02X %02X %02X %02X %02X\n",
        bd_addr[0],bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
        APPL_TRC("BD addr type : %s\n",
        (0 == bd_addr_type)? "Public Address": "Random Address");
#endif /* DONT_USE_STANDARD_IO */

        /* Reference the event data */
        kx_param = (SMP_KEY_XCHG_PARAM *) event_data;

        APPL_TRC("Local keys negotiated - 0x%02X\n", kx_param->keys);
        APPL_TRC("Encryption Key Size negotiated - 0x%02X\n",
                kx_param->ekey_size);

        /* Check if automatic mode is set */
        if(SMP_TRUE == automatic_mode)
        {
            /* Get platform data of key informations */
            BT_smp_get_key_exchange_info_pl (&key_info);

            /* Copy the Local Key Info into a local struct */
            BT_mem_copy(&local_key_info,key_info,sizeof(local_key_info));

            /* Mask the to be exchanged LTK according to the negotiated key size */
            BT_mem_set
            (
                (&local_key_info.enc_info[0] + kx_param->ekey_size),
                0x00,
                (SMP_LTK_SIZE - kx_param->ekey_size)
            );

            BT_smp_key_exchange_info_request_reply (bd_handle, &local_key_info);
        }
        break;

#ifdef SMP_SLAVE
    case SMP_LONG_TERM_KEY_REQUEST:

        /* Copy parameters to local variables */
        smp_unpack_2_byte_param(&ediv, &event_data[8]);
        peer_rand = event_data;

        APPL_TRC("\nEvent   : SMP_LONG_TERM_KEY_REQUEST\n");
#ifndef DONT_USE_STANDARD_IO
        APPL_TRC("BD Address : %02X %02X %02X %02X %02X %02X\n",
        bd_addr[0],bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
        APPL_TRC("BD addr type : %s\n",
        (0 == bd_addr_type)? "Public Address": "Random Address");
#endif /* DONT_USE_STANDARD_IO */

        APPL_TRC("Div  : 0x%04X\n", ediv);
        APPL_TRC("Rand : %02X %02X %02X %02X %02X %02X %02X %02X\n",
        peer_rand[0], peer_rand[1], peer_rand[2], peer_rand[3],
        peer_rand[4], peer_rand[5], peer_rand[6], peer_rand[7]);

        /* Check if automatic mode is set */
        if(SMP_TRUE == automatic_mode)
        {
            retval = BT_smp_get_device_security_info (bd_handle, &info);

            if (API_SUCCESS == retval)
            {
                /* Check if the link is authenticated */
                if ((SMP_ENTITY_AUTH_ON == info.param) ||
                    (SMP_TRUE == info.bonding))
                {
#ifdef SMP_LESC
                    if (info.pair_mode == SMP_LESC_MODE)
                    {
                        retval = BT_smp_get_device_keys
                                 (
                                     bd_handle,
                                     &p_keys,
                                     &p_key_info
                                 );

                        if (API_SUCCESS != retval)
                        {
                            APPL_ERR("Failed to get Peer Device Keys!!\n");
                        }
                        else
                        {
                            BT_mem_copy(ltk, p_key_info.enc_info, 16);

                            /*
                             * NOTE: To check if masking of LTK according to negotiated key size
                             *       is needed when in secure connections only mode.
                             */
                        }
                    }
                    else
#endif /* SMP_LESC */
                    {
                        /* Get LTK for remote device */
                        retval = BT_smp_get_long_term_key_pl
                                 (
                                     peer_rand,
                                     ediv,
                                     ltk
                                 );

                        if(API_SUCCESS == retval)
                        {
                            /* Mask the key according to the negotiated key size */
                            BT_mem_set
                            (
                                (ltk + info.ekey_size),
                                0x00,
                                (SMP_LTK_SIZE - info.ekey_size)
                            );
                        }
                    }

                    if (API_SUCCESS == retval)
                    {
                        APPL_TRC("\n\nSending +ve LTK request reply.\n");
                        retval = BT_smp_long_term_key_request_reply
                                 (
                                     bd_handle,
                                     ltk,
                                     SMP_TRUE
                                 );
                    }
                }
                else
                {
                    retval = API_FAILURE;
                }
            }

            if (API_SUCCESS != retval)
            {
                APPL_TRC("\n\nSending -ve LTK request reply.\n");
                ltk_null = 0;
                retval = BT_smp_long_term_key_request_reply
                         (
                             bd_handle,
                             &ltk_null,
                             SMP_FALSE
                         );
            }

            /* Notify application state machine */
            appl_get_handle_from_device_handle (*bd_handle,&appl_handle);
            fsm_param.handle = appl_handle;
            fsm_param.data_param = NULL;
            fsm_param.data_len = 0;
            fsm_param.direction = 0x01;

            /* Post Transport Configuration Request for SMP */
            fsm_post_event
            (
                 APPL_FSM_ID,
                 ev_appl_transport_configuration_req,
                 (void *)(&fsm_param)
            );
        }
        break;
#endif /* SMP_SLAVE */

    case SMP_RESOLVABLE_PVT_ADDR_CREATE_CNF:
        APPL_TRC("\nEvent   : SMP_RESOLVABLE_PVT_ADDR_CREATE_CNF\n");
        APPL_TRC("Status : %04X\n", status);
        if(API_SUCCESS == status)
        {
#ifndef DONT_USE_STANDARD_IO
            bd_addr = (UCHAR *) event_data;

            APPL_TRC("BD Address : %02X %02X %02X %02X %02X %02X\n",
            bd_addr[0],bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],
            bd_addr[5]);
#endif /* DONT_USE_STANDARD_IO */
        }
        break;

    case SMP_RESOLVABLE_PVT_ADDR_VERIFY_CNF:
        APPL_TRC("\nEvent   : SMP_RESOLVABLE_PVT_ADDR_VERIFY_CNF\n");
        APPL_TRC("Status : %04X\n", status);
        if(API_SUCCESS == status)
        {
#ifndef DONT_USE_STANDARD_IO
            bd_addr = (UCHAR *) event_data;

            APPL_TRC("BD Address : %02X %02X %02X %02X %02X %02X\n",
            bd_addr[0],bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],
            bd_addr[5]);
#endif /* DONT_USE_STANDARD_IO */
        }
        break;

#ifdef SMP_DATA_SIGNING
    case SMP_DATA_SIGNING_COMPLETE:
        APPL_TRC("\nEvent   : SMP_SIGNING_DATA_COMPLETE\n");
        APPL_TRC("Status : %04X\n", status);
        if(API_SUCCESS == status)
        {
            /* Update sign counter */
            sign_create_counter ++;

            APPL_TRC("Message Authentication Code : ");
            appl_dump_bytes
            (
                event_data,
                data_len
            );

            /* Form the signature */
            BT_mem_copy (&signature[sizeof(sign_create_counter)], event_data, data_len);

            APPL_TRC("Message Authentication Code : ");
            appl_dump_bytes
            (
                signature,
                sizeof (signature)
            );
        }

        /* Call gatt signature procedure handler */
        appl_gatt_signing_verification_complete
        (status, signature, sizeof(signature));

        if (NULL != sign_data)
        {
            BT_free_mem (sign_data);
            sign_data = NULL;
        }

        break;

    case SMP_SIGN_DATA_VERIFICATION_COMPLETE:
        APPL_TRC("\nEvent   : SMP_SIGN_DATA_VERIFICATION_COMPLETE\n");
        APPL_TRC("Status : %04X\n", status);

        if (API_SUCCESS == status)
        {
            /* Form the signature */
            BT_PACK_LE_4_BYTE(signature, &sign_verify_counter);
            BT_mem_copy
            (&signature[sizeof(sign_verify_counter)], event_data, data_len);

            /* Update sign counter */
            sign_verify_counter ++;

            APPL_TRC("Message Authentication Code : ");
            appl_dump_bytes
            (
                event_data,
                data_len
            );
        }

        /* Call gatt signature procedure handler */
        appl_gatt_signing_verification_complete
        (status, signature, sizeof(signature));

        if (NULL != sign_data)
        {
            BT_free_mem (sign_data);
            sign_data = NULL;
        }

        break;
#endif /* SMP_DATA_SIGNING */

    case SMP_KEY_EXCHANGE_INFO:
        APPL_TRC ("\nRecvd SMP_KEY_EXCHANGE_INFO\n");
        APPL_TRC ("Status - 0x%04X\n", status);

        /* Reference the event data */
        kx_param = (SMP_KEY_XCHG_PARAM *) event_data;

        APPL_TRC("Remote keys negotiated - 0x%02X\n", kx_param->keys);
        APPL_TRC("Encryption Key Size negotiated - 0x%02X\n",
                kx_param->ekey_size);

        /* Reference the key information */
        key_info = kx_param->keys_info;

#ifdef SMP_DATA_SIGNING
        /* Store the peer keys */
        BT_mem_copy (&peer_key_info, key_info, sizeof (SMP_KEY_DIST));
#endif /* SMP_DATA_SIGNING */

        APPL_TRC ("Encryption Info:\n");
        appl_dump_bytes(key_info->enc_info, sizeof (key_info->enc_info));
        APPL_TRC ("Master Identification Info:\n");
        appl_dump_bytes(key_info->mid_info, sizeof (key_info->mid_info));
        APPL_TRC ("Identity Info:\n");
        appl_dump_bytes(key_info->id_info, sizeof (key_info->id_info));
        APPL_TRC ("Identity Address Info:\n");
        appl_dump_bytes(key_info->id_addr_info, sizeof (key_info->id_addr_info));
        APPL_TRC ("Signature Info:\n");
        appl_dump_bytes(key_info->sign_info, sizeof (key_info->sign_info));

        break;

#ifdef SMP_LESC
    case SMP_NUMERIC_KEY_COMPARISON_CNF_REQUEST:
        APPL_TRC("\nEvent   : SMP_NUMERIC_KEY_COMPARISON_CNF_REQUEST\n");
        APPL_TRC("BD Address : %02X %02X %02X %02X %02X %02X\n",
        bd_addr[0],bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
        APPL_TRC("BD addr type : %s\n",
        (0 == bd_addr_type)? "Public Address": "Random Address");

        APPL_TRC("Numeric Code : %06u\n", (*((UINT32 *)event_data) % 1000000));
        break;

    case SMP_KEY_PRESS_NOTIFICATION_EVENT:
        APPL_TRC("\nEvent   : SMP_KEY_PRESS_NOTIFICATION_EVENT\n");
        APPL_TRC("BD Address : %02X %02X %02X %02X %02X %02X\n",
        bd_addr[0],bd_addr[1],bd_addr[2],bd_addr[3],bd_addr[4],bd_addr[5]);
        APPL_TRC("BD addr type : %s\n",
        (0 == bd_addr_type)? "Public Address": "Random Address");

        APPL_TRC("KeyPress Notification Value is:");

        switch(*(UCHAR *)event_data)
        {
            case SMP_LESC_PASSKEY_ENTRY_STARTED:
                APPL_TRC("SMP_LESC_PASSKEY_ENTRY_STARTED\n");
                break;
            case SMP_LESC_PASSKEY_DIGIT_ENTERED:
                APPL_TRC("SMP_LESC_PASSKEY_DIGIT_ENTERED\n");
                break;
            case SMP_LESC_PASSKEY_DIGIT_ERASED:
                APPL_TRC("SMP_LESC_PASSKEY_DIGIT_ERASED\n");
                break;
            case SMP_LESC_PASSKEY_CLEARED:
                APPL_TRC("SMP_LESC_PASSKEY_CLEARED\n");
                break;
            case SMP_LESC_PASSKEY_ENTRY_COMPLETED:
                APPL_TRC("SMP_LESC_PASSKEY_ENTRY_COMPLETED\n");
                break;
            default:
                APPL_TRC("Unknown KeyPress Value 0x%02X Received\n",*(UCHAR *)event_data);
                break;
        }
        break;
#endif /* SMP_LESC */

    default:
        APPL_TRC("\nERROR!!! Received unknown event. event = %02X\n", event);
        break;
    }

    return retval;
}

#ifdef HAVE_SMP_REPEAT_TRACKING
DECL_STATIC void smp_repeat_timeout_handler (void * data_param, UINT16 datalen)
{
    /* Reinitialize timer handle */
    smp_repeat_timer = BT_TIMER_HANDLE_INIT_VAL;

    APPL_TRC ("Repeat Tracking Timer expired.\n");
}
#endif /* HAVE_SMP_REPEAT_TRACKING */

#endif /* SMP */

