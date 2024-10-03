
/**
 *  \file appl_pos.c
 *
 *  This file contains the Sample Pulse Oximeter Sensor application.
 *  Sample applications detailed below:
 *      a. The Sensor, as defined by the Sepcification plays the GAP Peripheral
 *         role.
 *      b. The Sensor application has following sevice records:
 *           - GAP
 *           - GATT
 *           - Battery
 *           - Device Information and
 *           - Pulse Oximeter
 *         [NOTE]: Please see gatt_db.c for more details of the record.
 *      c. The Pulse Oximeter Service exposes:
 *           - Pulse Oximetry Features
 *           - Pulse Oximetry Spot Check Measurements
 *           - Pulse Oximetry Continuous Measurements
 *           - Record Access Control Point
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl.h"

#ifdef POS

/* --------------------------------------------- Global Definitions */
#define APPL_RECORD_TRNSFER_INTERVAL             3
#define APPL_POX_SPOT_CHK_MSRMT_INTERVAL         10
#define APPL_POX_CONTINUOUS_MSRMT_INTERVAL       3
#define APPL_POX_SPOT_CHK_MSRMT_LENGTH           16
#define APPL_POX_CONTINUOUS_MSRMT_LENGTH         20

#define APPL_SPOT_CHK_MSRMT_LENGTH               19
#define APPL_SPOT_CHK_MSRMT_COUNT                3
#define APPL_CONTINUOUS_MSRMT_LENGTH             20
#define APPL_CONTINUOUS_MSRMT_COUNT              4

/* RACP Related Defines */
/* POS Response Codes */
#define APPL_POS_RACP_SUCCESS                    0x01
#define APPL_POS_RACP_OP_CODE_NOT_SUPPORTED      0x02
#define APPL_POS_RACP_INVALID_OPERATOR           0x03
#define APPL_POS_RACP_OPRTR_NOT_SUPPORTED        0x04
#define APPL_POS_RACP_INVALID_OPERAND            0x05
#define APPL_POS_RACP_NO_RECORDS_FOUND           0x06
#define APPL_POS_RACP_ABRT_UNSUCCESSFUL          0x07
#define APPL_POS_RACP_PROC_INCOMPLETE            0x08
#define APPL_POS_RACP_OPERAND_NOT_SUPPORTED      0x09

/* Application RACP States */
#define APPL_RACP_INIT_STATE                     0x00
#define APPL_RACP_TRANSFR_ONGOING_STATE          0x01
#define APPL_RACP_CLR_STATE                      0x02

/* Pulse Oximeter Defined RACP Operations */
#define APPL_POS_RACP_STRD_RECORD_REQ_OPCODE       0x01
#define APPL_POS_RACP_CLR_RECORD_REQ_OPCODE        0x02
#define APPL_POS_RACP_ABRT_RPRTING_REQ_OPCODE      0x03
#define APPL_POS_RACP_STRD_RECORD_CNT_REQ_OPCODE   0x04
#define APPL_POS_RACP_STRD_RECORD_CNT_RSP_OPCODE   0x05
#define APPL_POS_RACP_RSP_OPCODE                   0x06

/* Pulse Oximeter Defined RACP Operators */
#define APPL_POS_RACP_NULL_OPRTR                   0x00
#define APPL_POS_RACP_ALL_RCRD_OPRTR               0x01

#define APPL_STORED_RECORD_COUNT                   3
#define APPL_CONTINUOUS_MSRMT_RECORD_COUNT         4

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

static BT_timer_handle timer_handle;
static BT_timer_handle spot_chk_msrmt_timer_handle;
static BT_timer_handle continus_msrmt_timer_handle;

static GATT_DB_HANDLE  appl_batt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_BATTERY_INST,
    GATT_CHAR_BATTERY_LVL_INST
};

static GATT_DB_HANDLE  appl_pox_features_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_PULSE_OXIMTR_INST,
    GATT_CHAR_PLX_FEATURES_INST
};

static GATT_DB_HANDLE  appl_pox_spot_chk_msrmt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_PULSE_OXIMTR_INST,
    GATT_CHAR_PLX_SPOT_CHECK_MSRMNT_INST
};

static GATT_DB_HANDLE  appl_pox_continus_msrmt_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_PULSE_OXIMTR_INST,
    GATT_CHAR_PLX_CONTINUS_MSRMNT_INST
};

static GATT_DB_HANDLE  appl_pos_racp_db_handle =
{
    DEVICE_HANDLE_INIT_VAL,
    GATT_SER_PULSE_OXIMTR_INST,
    GATT_CHAR_RACP_INST
};

static ATT_ATTR_HANDLE appl_pox_features_handle;
static ATT_ATTR_HANDLE appl_pox_cntrl_pnt_handle;
static ATT_ATTR_HANDLE appl_pox_spot_chk_msrmt_handle;
static ATT_ATTR_HANDLE appl_pox_continus_msrmt_handle;
static ATT_ATTR_HANDLE appl_pos_racp_hndl;
static APPL_HANDLE     appl_hndl;
static UCHAR           pos_cntrl_pnt_enabled;
static UCHAR           pos_spot_chk_msrmt_enabled;
static UCHAR           pos_continus_msrmt_enabled;
static UCHAR           appl_batt_update_count;
static UCHAR           appl_batt_percent;
static UCHAR           appl_pos_operating_state;
static UCHAR           appl_stored_record_count;
static UCHAR           appl_continuous_record_cnt;
static UCHAR           appl_record_count;
static UCHAR           appl_contius_msrmt_record_count;
static UCHAR           appl_clr_all_records;
static UCHAR           pos_racp_id;

/* --------------------------------------------- Static Global Variables */

DECL_STATIC DECL_CONST RACP_MODULE_T pos_racp =
{
    appl_pos_report_stored_records,

    appl_pos_report_stored_number_of_records,

    appl_pos_delete_stored_record,

    appl_pos_abort_operation
};

#ifdef APPL_MENU_OPS
#ifndef BT_SW_TXP
static UCHAR pos_sensor_menu[] =
"\n\
    0 - Exit\n\
    1 - Refresh\n\n\
    -- PLX Spot Check Measurement --\n\
    2 - Indicate PLX Spot Check Mesaurement with only Mandatory fields\n\
    3 - Clear all stored PLX Spot Check Mesaurement records\n\
    -- PLX Continuous Measurement --\n\
    6 - Notify PLX Continuous Measurement with only Mandatory fields\n\n\
Your Option?\n\
";
#else /* BT_SW_TXP */
static const char main_options[] = "\r\n\
======== POS : M A I N   M E N U ========\r\n\
 0. Exit\r\n\
 1. Refresh\r\n\
 2. Power ON Device\r\n\
 3. Disconnect\r\n\
 4. Indicate POS Spot Check Mesaurement[Only Mandatory]\r\n\
 5. Clear all stored PLX Spot Check Mesaurement records\r\n\
 6. Notify PLX Continuous Measurement[Only Mandatory]\r\n\
 \r\n\
 Your Option ?\r\n";
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

static UCHAR spot_chk_msrmt_value[APPL_SPOT_CHK_MSRMT_LENGTH * APPL_SPOT_CHK_MSRMT_COUNT] =
{
    /**
     *  1st Octet - Flag
     *  2nd & 3rd Octet - Fully Qualified SpO2 Readings
     *  4th & 5th Octet - Fully Qualified PR Readings
     *  6th - 12th Octet - TimeStamp (In YYYY-MM-DD:HR-MIN-SEC)
     *  13th & 14th Octet - Measurement Status
     *  15th - 17th Octet - Device & Sensor Status
     *  18th & 19th Octet - Pulse Amplitude Index
     */
    0x0F, 0x48, 0xF3, 0x76, 0xF2, 0xDF, 0x07, 0x06, 0x01, 0x0F, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x40, 0x00, 0x95, 0xE0,

    0x0F, 0x49, 0xF3, 0x76, 0xF2, 0xDF, 0x07, 0x06, 0x01, 0x10, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x40, 0x00, 0x95, 0xE0,

    0x0F, 0x50, 0xF3, 0x76, 0xF2, 0xDF, 0x07, 0x06, 0x01, 0x11, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x40, 0x00, 0x95, 0xE0
};

static UCHAR continuous_msrmt_value[APPL_CONTINUOUS_MSRMT_LENGTH * APPL_CONTINUOUS_MSRMT_COUNT] =
{
    /**
     *  1st Octet - Flag
     *  2nd & 3rd Octet - Fully Qualified SpO2PR-Normal SpO2
     *  4th & 5th Octet - Fully Qualified SpO2PR-Normal PR
     *  6th & 7th Octet - Fully Qualified SpO2PR-Fast SpO2
     *  8th & 9th Octet - Fully Qualified SpO2PR-Fast PR
     *  10th & 11th Octet - Fully Qualified SpO2PR-Slow SpO2
     *  12th & 13th Octet - Fully Qualified SpO2PR-Slow PR
     *  14th & 15th Octet - Measurement Status
     *  16th - 18th Octet - Device & Sensor Status
     *  19th & 20th Octet - Pulse Amplitude Index
     */
    0x1F, 0x48, 0xF3, 0x76, 0xF2, 0x52, 0xF3, 0x80, 0xF2, 0x3E, 0xF3, 0x6C, 0xF2,
    0x00, 0x01, 0x00, 0x40, 0x00, 0x96, 0xE0,

    0x1F, 0x49, 0xF3, 0x77, 0xF2, 0x53, 0xF3, 0x81, 0xF2, 0x3F, 0xF3, 0x6D, 0xF2,
    0x00, 0x01, 0x00, 0x40, 0x00, 0x96, 0xE0,

    0x1F, 0x50, 0xF3, 0x78, 0xF2, 0x54, 0xF3, 0x82, 0xF2, 0x4A, 0xF3, 0x6E, 0xF2,
    0x00, 0x01, 0x00, 0x40, 0x00, 0x96, 0xE0,

    0x1F, 0x51, 0xF3, 0x79, 0xF2, 0x55, 0xF3, 0x83, 0xF2, 0x4B, 0xF3, 0x6F, 0xF2,
    0x00, 0x01, 0x00, 0x40, 0x00, 0x96, 0xE0
};

static UCHAR spot_chk_msrmt_with_only_mandtry_flds[] =
{
    /* Flag */
    0x00,

    /* PR-Spot-Check - SpO2 */
    0x70, 0xF3,

    /* SpO2PR-Spot-Check - PR */
    0x80, 0xF2,
};

static UCHAR contins_msrmt_with_only_mandtry_flds[] =
{
    /* Flag */
    0x00,

    /* SpO2PR-Normal - SpO2 */
    0x60, 0xF3,

    /* SpO2PR-Normal - PR */
    0x90, 0xF2,
};

/* --------------------------------------------- Functions */

void appl_pos_init(void)
{
    pos_cntrl_pnt_enabled      = BT_FALSE;
    pos_spot_chk_msrmt_enabled = BT_FALSE;
    pos_continus_msrmt_enabled = BT_FALSE;
    appl_batt_update_count     = 0;
    appl_batt_percent          = 100;

    pos_racp_id                = RACP_MAX_MODULES;

    appl_pos_server_reinitialize ();
}

void pos_racp_init(void)
{
    API_RESULT retval;

    if (RACP_MAX_MODULES == pos_racp_id)
    {
        retval = BT_racp_register_module(&pos_racp, &pos_racp_id);

        if (API_SUCCESS != retval)
        {
            APPL_ERR("[APPL]: RACP Registration Failed!\n");
        }
    }
    else
    {
        APPL_ERR("[APPL]: RACP Re-Registration Attempted!\n");
    }
}

void appl_pos_connect(APPL_HANDLE  * appl_handle)
{
    ATT_VALUE         value;
    API_RESULT        retval;
    UINT16            cli_cnfg;

    /* Calling RACP Registration for POS from here */
    /* This is done as profile init is invoked ahead of BT_bluetooth_on in SingleMode */
    pos_racp_init();

    /* Store the incoming application handle to global */
    appl_hndl = (* appl_handle);

    cli_cnfg = 0;

    /* Check if POX Spot Check Measurement is configured by the client */
    appl_pox_spot_chk_msrmt_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_pox_spot_chk_msrmt_db_handle,
                  &appl_pox_spot_chk_msrmt_handle
             );

    retval = BT_gatt_db_get_char_cli_cnfg
             (
                 &appl_pox_spot_chk_msrmt_db_handle,
                 &value
             );

    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    if (cli_cnfg)
    {
        appl_manage_trasnfer(appl_pox_spot_chk_msrmt_db_handle,cli_cnfg);
    }

    /* Check if POX Continuous Measurement is configured by the client */
    appl_pox_continus_msrmt_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_pox_continus_msrmt_db_handle,
                  &appl_pox_continus_msrmt_handle
             );

    retval = BT_gatt_db_get_char_cli_cnfg
             (
                 &appl_pox_spot_chk_msrmt_db_handle,
                 &value
             );

    BT_UNPACK_LE_2_BYTE (&cli_cnfg,value.val);

    if (cli_cnfg)
    {
        appl_manage_trasnfer(appl_pox_spot_chk_msrmt_db_handle,cli_cnfg);
    }

    appl_pos_racp_db_handle.device_id = APPL_GET_DEVICE_HANDLE((*appl_handle));

    /* Get Glucose RCAP Characteristics Handle */
    retval = BT_gatt_db_get_char_val_hndl
             (
                  &appl_pos_racp_db_handle,
                  &appl_pos_racp_hndl
             );
}


void appl_manage_trasnfer (GATT_DB_HANDLE handle, UINT16 config)
{
    API_RESULT     retval;
    APPL_HANDLE    appl_handle;
    UCHAR          security, ekey_size;

    /* Get required security for service */
    /* Get required security level */
    BT_gatt_db_get_service_security (&handle,&security);
    /* Get required encryption key size */
    BT_gatt_db_get_service_enc_key_size (&handle,&ekey_size);

    /* Verify if security requirements are available with the link */
    retval = appl_smp_assert_security
             (
                 &handle.device_id,
                 security,
                 ekey_size
             );

    /* Security requirements satisfied? */
    if (API_SUCCESS != retval)
    {
        /* No. Return */
        return;
    }

    /* Security requirements satisfied, go ahead with data transfer */

    appl_get_handle_from_device_handle (handle.device_id,&appl_handle);
    if (GATT_CLI_CNFG_INDICATION == config)
    {
        if (GATT_CHAR_PLX_SPOT_CHECK_MSRMNT_INST == handle.char_id)
        {
            pos_spot_chk_msrmt_enabled = BT_TRUE;

            if (BT_TIMER_HANDLE_INIT_VAL != spot_chk_msrmt_timer_handle)
            {
                retval = BT_stop_timer (spot_chk_msrmt_timer_handle);
                APPL_TRC (
                "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
                retval,spot_chk_msrmt_timer_handle);
                spot_chk_msrmt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }

            retval = BT_start_timer
                     (
                         &spot_chk_msrmt_timer_handle,
                         APPL_POX_SPOT_CHK_MSRMT_INTERVAL,
                         appl_spot_chk_msrmt_timer_expiry_handler,
                         &appl_handle,
                         sizeof (appl_handle)
                     );
            APPL_TRC (
            "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
            retval,spot_chk_msrmt_timer_handle);
        }
    }
    else if (GATT_CLI_CNFG_NOTIFICATION == config)
    {
        if (GATT_CHAR_PLX_CONTINUS_MSRMNT_INST == handle.char_id)
        {
            pos_continus_msrmt_enabled = BT_TRUE;

            if (BT_TIMER_HANDLE_INIT_VAL != continus_msrmt_timer_handle)
            {
                retval = BT_stop_timer (continus_msrmt_timer_handle);
                APPL_TRC (
                "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
                retval,continus_msrmt_timer_handle);
                continus_msrmt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }
            retval = BT_start_timer
                     (
                         &continus_msrmt_timer_handle,
                         APPL_POX_CONTINUOUS_MSRMT_INTERVAL,
                         appl_continus_msrmt_timer_expiry_handler,
                         &appl_handle,
                         sizeof (appl_handle)
                     );
            APPL_TRC (
            "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
            retval,continus_msrmt_timer_handle);
        }
    }
    else if (GATT_CLI_CNFG_DEFAULT == config)
    {
        if (GATT_CHAR_PLX_SPOT_CHECK_MSRMNT_INST == handle.char_id)
        {
            if (BT_TIMER_HANDLE_INIT_VAL != spot_chk_msrmt_timer_handle)
            {
                retval = BT_stop_timer (spot_chk_msrmt_timer_handle);
                APPL_TRC (
                "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
                retval,spot_chk_msrmt_timer_handle);
                spot_chk_msrmt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }
            pos_spot_chk_msrmt_enabled = BT_FALSE;
        }
        else if (GATT_CHAR_PLX_CONTINUS_MSRMNT_INST == handle.char_id)
        {
            if (BT_TIMER_HANDLE_INIT_VAL != continus_msrmt_timer_handle)
            {
                retval = BT_stop_timer (continus_msrmt_timer_handle);
                APPL_TRC (
                "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
                retval,continus_msrmt_timer_handle);
                continus_msrmt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
            }
            pos_continus_msrmt_enabled = BT_FALSE;
        }
    }
}

void appl_spot_chk_msrmt_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;
    ATT_VALUE          value;
    API_RESULT         retval;

    fsm_param.handle = (*((APPL_HANDLE *)data));
    if (BT_TIMER_HANDLE_INIT_VAL != spot_chk_msrmt_timer_handle)
    {
        retval = BT_stop_timer (spot_chk_msrmt_timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
        retval,spot_chk_msrmt_timer_handle);
        spot_chk_msrmt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    appl_batt_update_count ++;
    value.len = 1;
    value.val = &appl_batt_percent;

    /* Battery is decreased every 5 minutes (5 * 6) times 10 seconds */
    if (30 == appl_batt_update_count)
    {
        appl_batt_percent--;
        APPL_TRC (
        "[APPL]: Battery Percentage 0x%04X\n",appl_batt_percent);

        appl_batt_update_count = 0;

        BT_gatt_db_set_char_val (&appl_batt_db_handle,&value);
        if (0 == appl_batt_percent)
        {
            /* Resetting battery percentage to 100 */
            appl_batt_percent = 100;
        }
    }

    /* Post Measurement Indication to APPL FSM */
    fsm_post_event
    (
         APPL_FSM_ID,
         ev_appl_measurement_ind,
         &fsm_param
    );

    APPL_TRC (
    "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
    retval,spot_chk_msrmt_timer_handle);
}

void appl_continus_msrmt_timer_expiry_handler (void *data, UINT16 datalen)
{
    APPL_EVENT_PARAM   fsm_param;
    API_RESULT         retval;

    fsm_param.handle = (*((APPL_HANDLE *)data));
    if (BT_TIMER_HANDLE_INIT_VAL != continus_msrmt_timer_handle)
    {
        retval = BT_stop_timer (continus_msrmt_timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
        retval,continus_msrmt_timer_handle);
        continus_msrmt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    retval = BT_start_timer
             (
                 &continus_msrmt_timer_handle,
                 APPL_POX_CONTINUOUS_MSRMT_INTERVAL,
                 appl_continus_msrmt_timer_expiry_handler,
                 &fsm_param.handle,
                 sizeof(fsm_param.handle)
             );

    appl_pos_continus_msrmt_hndlr();

    APPL_TRC (
    "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
    retval,continus_msrmt_timer_handle);
}

API_RESULT appl_pos_continus_msrmt_hndlr()
{
    API_RESULT         retval;
    ATT_HANDLE_VALUE_PAIR hndl_val_param;

    retval = API_SUCCESS;

    if (pos_continus_msrmt_enabled == BT_TRUE)
    {
        hndl_val_param.handle =  appl_pox_continus_msrmt_handle;
        hndl_val_param.value.len = APPL_CONTINUOUS_MSRMT_LENGTH;
        hndl_val_param.value.val = &continuous_msrmt_value
            [APPL_CONTINUOUS_MSRMT_LENGTH * appl_contius_msrmt_record_count];

        retval = BT_att_send_hndl_val_ntf
                 (
                     &APPL_GET_ATT_INSTANCE(appl_hndl),
                     &hndl_val_param
                 );
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send measurement, reason 0x%04X",
            retval);
        }
        appl_contius_msrmt_record_count++;
    }

    if (appl_continuous_record_cnt == appl_contius_msrmt_record_count)
    {
        if (BT_TIMER_HANDLE_INIT_VAL != continus_msrmt_timer_handle)
        {
            retval = BT_stop_timer (continus_msrmt_timer_handle);
            APPL_TRC (
            "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
            retval,continus_msrmt_timer_handle);
            continus_msrmt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }
    }
    return retval;
}

void appl_pos_server_reinitialize (void)
{
    if (BT_TIMER_HANDLE_INIT_VAL != spot_chk_msrmt_timer_handle)
    {
        BT_stop_timer (spot_chk_msrmt_timer_handle);
        spot_chk_msrmt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }
    if (BT_TIMER_HANDLE_INIT_VAL != continus_msrmt_timer_handle)
    {
        BT_stop_timer (continus_msrmt_timer_handle);
        continus_msrmt_timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    appl_pos_operating_state = APPL_RACP_INIT_STATE;
    appl_record_count = 0;
    appl_contius_msrmt_record_count = 0;
    appl_stored_record_count = APPL_STORED_RECORD_COUNT;
    appl_continuous_record_cnt = APPL_CONTINUOUS_MSRMT_RECORD_COUNT;

#if ((defined APPL_GAP_BROACASTER) || defined (APPL_GAP_PERIPHERAL))
    /* Configure and Enable Advertising */
    appl_service_configure_adv (APPL_GAP_PROC_NORMAL, HCI_ADV_IND, 0x00, 0x00, NULL, 0x00);
    appl_service_enable_adv(0x01);
#endif /* ((defined APPL_GAP_BROACASTER) || defined (APPL_GAP_PERIPHERAL)) */

#if ((defined APPL_GAP_OBSERVER) || (defined APPL_GAP_CENTRAL))
    /* Configure and Enable Scanning */
    appl_service_configure_scan (APPL_GAP_PROC_NORMAL, 0x00, 0x00, 0x00);
    appl_service_enable_scan(0x01);
#endif /* ((defined APPL_GAP_OBSERVER) || (defined APPL_GAP_CENTRAL)) */
}

void appl_send_pulse_ox_measurement (APPL_HANDLE    * handle)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;

    if (BT_FALSE != pos_spot_chk_msrmt_enabled)
    {
        hndl_val_param.handle =  appl_pox_spot_chk_msrmt_handle;
        hndl_val_param.value.len = APPL_SPOT_CHK_MSRMT_LENGTH;
        hndl_val_param.value.val = &spot_chk_msrmt_value[0];

        retval = BT_att_send_hndl_val_ind
                 (
                     &APPL_GET_ATT_INSTANCE(*handle),
                     &hndl_val_param
                 );
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send measurement, reason 0x%04X",
            retval);
        }
    }
}

#if 0
API_RESULT appl_pos_racp_write_handler
           (
                GATT_DB_HANDLE    * handle,
                ATT_VALUE         * value
           )
{
    UCHAR               operation;
    UCHAR               oprtr;
    API_RESULT          retval;
    APPL_TIMER_PARAM    appl_timer_param;

    retval = API_SUCCESS;

    if (APPL_RACP_INIT_STATE == appl_pos_operating_state ||
        APPL_POS_RACP_ABRT_RPRTING_REQ_OPCODE == value->val[0])
    {
        appl_get_handle_from_device_handle (handle->device_id,&appl_timer_param.handle);

        /** Validate RACP Operation */
        if (1 < value->len)
        {
            operation = value->val[0];
            oprtr = value->val[1];

            appl_timer_param.appl_req_param.req_op_code = operation;
            appl_timer_param.appl_req_param.oprtr       = oprtr;

            if (BT_FALSE != APPL_RACP_OPCODE_IN_REQ_RANGE(operation))
            {
                /** Minimum length length is 2 */
                switch (operation)
                {
                /*Fall through*/
                case APPL_POS_RACP_STRD_RECORD_REQ_OPCODE:
                case APPL_POS_RACP_CLR_RECORD_REQ_OPCODE:
                case APPL_POS_RACP_STRD_RECORD_CNT_REQ_OPCODE:
                    if(APPL_POS_RACP_ALL_RCRD_OPRTR == oprtr)
                    {
                        if (2 != value->len)
                        {
                            appl_timer_param.rsp_code_val = APPL_POS_RACP_INVALID_OPERAND;
                        }
                        else
                        {
                            appl_timer_param.rsp_code_val = APPL_POS_RACP_SUCCESS;
                        }
                    }
                    else if (APPL_POS_RACP_NULL_OPRTR == oprtr)
                    {
                        appl_timer_param.rsp_code_val = APPL_POS_RACP_INVALID_OPERATOR;
                    }
                    else if( oprtr > APPL_POS_RACP_ALL_RCRD_OPRTR)
                    {
                        appl_timer_param.rsp_code_val = APPL_POS_RACP_OPRTR_NOT_SUPPORTED;;
                    }
                    break;
                case APPL_POS_RACP_ABRT_RPRTING_REQ_OPCODE:
                    if (APPL_RACP_TRANSFR_ONGOING_STATE == appl_pos_operating_state
                        || APPL_RACP_CLR_STATE == appl_pos_operating_state)
                    {
                        /* Stop Ongoing transfer */
                        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
                        {
                            retval = BT_stop_timer (timer_handle);
                            APPL_TRC (
                            "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
                            retval,timer_handle);
                            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
                        }
                    }
                    appl_pos_operating_state = APPL_RACP_INIT_STATE;
                    appl_timer_param.rsp_code_val = APPL_POS_RACP_SUCCESS;
                    break;
                default:
                    break;
                }
            }
            else
            {
                appl_timer_param.rsp_code_val = APPL_POS_RACP_OP_CODE_NOT_SUPPORTED;
                retval = API_SUCCESS;
            }
        }
        appl_racp_manage_transfer(&appl_timer_param);
    }
    else
    {
        retval = ATT_PROCEDURE_ALREADY_IN_PROGRESS | APPL_ERR_ID;
    }
    return retval;
}
#endif

void appl_racp_manage_transfer (/* IN */ APPL_TIMER_PARAM * appl_timer_param)
{
    API_RESULT     retval;

    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        retval = BT_stop_timer (timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
        retval,timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    retval = BT_start_timer
             (
                 &timer_handle,
                 1,
                 appl_racp_timer_expiry_handler,
                 appl_timer_param,
                 sizeof(APPL_TIMER_PARAM)
             );
    APPL_TRC (
    "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
    retval,timer_handle);
}

void appl_racp_timer_expiry_handler (void *data, UINT16 datalen)
{
    API_RESULT         retval;
    APPL_TIMER_PARAM * appl_timer_param;

    appl_timer_param = (APPL_TIMER_PARAM *) data;

    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        retval = BT_stop_timer (timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
        retval,timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    retval = BT_start_timer
             (
                 &timer_handle,
                 1,
                 appl_racp_timer_expiry_handler,
                 appl_timer_param,
                 sizeof(APPL_TIMER_PARAM)
             );
    APPL_TRC (
    "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
    retval,timer_handle);

    appl_pos_racp_operation_hndlr(appl_timer_param);
}

API_RESULT appl_pos_racp_operation_hndlr(APPL_TIMER_PARAM * appl_timer_param)
{
    API_RESULT             retval;

    if (0 != appl_stored_record_count)
    {
        if (APPL_POS_RACP_STRD_RECORD_REQ_OPCODE == appl_timer_param->appl_req_param.req_op_code &&
            APPL_POS_RACP_SUCCESS == appl_timer_param->rsp_code_val)
        {
            /* Update Operating State */
            appl_pos_operating_state = APPL_RACP_TRANSFR_ONGOING_STATE;
            retval = appl_pos_racp_rprt_strd_record (appl_timer_param);
            if (API_SUCCESS != retval)
            {
                APPL_TRC ("[** ERR **]: Failed to send RACP Records,"
                " reason 0x%04X", retval);
            }
        }
        else if (APPL_POS_RACP_CLR_RECORD_REQ_OPCODE == appl_timer_param->appl_req_param.req_op_code &&
            APPL_POS_RACP_SUCCESS == appl_timer_param->rsp_code_val)
        {
            /* Update Operating State */
            appl_pos_operating_state = APPL_RACP_CLR_STATE;
            retval = appl_pos_racp_clear_strd_records (appl_timer_param);
        }
        else
        {
            /* Update Operating State */
            appl_pos_operating_state = APPL_RACP_TRANSFR_ONGOING_STATE;
            retval = appl_pos_racp_send_response (appl_timer_param);
            if (API_SUCCESS != retval)
            {
                APPL_TRC ("[** ERR **]: Failed to send RACP Rsp,"
                " reason 0x%04X", retval);
            }
        }
    }
    else
    {
        if (APPL_POS_RACP_STRD_RECORD_CNT_REQ_OPCODE !=
            appl_timer_param->appl_req_param.req_op_code)
        {
            appl_timer_param->rsp_code_val = APPL_POS_RACP_NO_RECORDS_FOUND;
        }
        retval = appl_pos_racp_send_response (appl_timer_param);
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send RACP Rsp,"
            " reason 0x%04X", retval);
        }
    }
    return retval;
}

API_RESULT appl_pos_racp_rprt_strd_record(APPL_TIMER_PARAM  *appl_timer_param)
{
    ATT_HANDLE_VALUE_PAIR  appl_pos_msrmt_param;
    API_RESULT retval;

    retval = API_SUCCESS;
    appl_pos_msrmt_param.handle = ATT_INVALID_ATTR_HANDLE_VAL;

    if (APPL_POS_RACP_ALL_RCRD_OPRTR == appl_timer_param->appl_req_param.oprtr)
    {
        /* For All Record Operator */
        appl_pos_msrmt_param.handle = appl_pox_spot_chk_msrmt_handle;
        appl_pos_msrmt_param.value.len = APPL_SPOT_CHK_MSRMT_LENGTH;
        appl_pos_msrmt_param.value.val = &spot_chk_msrmt_value
            [APPL_SPOT_CHK_MSRMT_LENGTH * appl_record_count];

        appl_record_count++;
    }
    else
    {
        appl_timer_param->rsp_code_val = APPL_POS_RACP_INVALID_OPERATOR;
        appl_record_count = appl_stored_record_count;
    }

    if ((APPL_POS_RACP_SUCCESS == appl_timer_param->rsp_code_val) &&
        (ATT_INVALID_ATTR_HANDLE_VAL != appl_pos_msrmt_param.handle))
    {
        retval = BT_att_send_hndl_val_ind
                 (
                     &APPL_GET_ATT_INSTANCE(appl_timer_param->handle),
                     &appl_pos_msrmt_param
                 );

        if (API_SUCCESS != retval)
        {
            appl_timer_param->rsp_code_val = APPL_POS_RACP_PROC_INCOMPLETE;
            retval = appl_pos_racp_send_response(appl_timer_param);
            APPL_TRC("[** ERR **]: Failed to send Stored spot check measurement, "
            "reason 0x%04X", retval);
        }
    }

    if (appl_stored_record_count == appl_record_count)
    {
        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
        {
            retval = BT_stop_timer(timer_handle);
            APPL_TRC(
            "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
            retval, timer_handle);
            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }

        /* Clear the records after sending all records */
        appl_stored_record_count = 0;

        retval = BT_start_timer
                 (
                     &timer_handle,
                     1,
                     appl_racp_send_ind_timer_expiry_handler,
                     appl_timer_param,
                     sizeof(APPL_TIMER_PARAM)
                 );
        APPL_TRC(
        "[APPL]: Started Timer with result 0x%04X, timer handle %p\n",
        retval, timer_handle);
    }

    return retval;
}

void appl_racp_send_ind_timer_expiry_handler (void *data, UINT16 datalen)
{
    API_RESULT         retval;
    APPL_TIMER_PARAM * appl_timer_param;

    appl_timer_param = (APPL_TIMER_PARAM *) data;

    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        retval = BT_stop_timer (timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
        retval,timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }
    retval = appl_pos_racp_send_response (appl_timer_param);
    if (API_SUCCESS != retval)
    {
        APPL_TRC ("[** ERR **]: Failed to send RACP Rsp,"
        " reason 0x%04X", retval);
    }
}

API_RESULT appl_pos_racp_clear_strd_records(APPL_TIMER_PARAM  *appl_timer_param)
{
    API_RESULT retval;

    retval = API_SUCCESS;

    if (APPL_POS_RACP_ALL_RCRD_OPRTR == appl_timer_param->appl_req_param.oprtr)
    {
        appl_stored_record_count = 0;
        retval = appl_pos_racp_send_response (appl_timer_param);
        if (API_SUCCESS != retval)
        {
            APPL_TRC ("[** ERR **]: Failed to send RACP Rsp,"
            " reason 0x%04X", retval);
        }
    }
    return retval;
}

API_RESULT appl_pos_racp_send_response (APPL_TIMER_PARAM * appl_timer_param)
{
    API_RESULT             retval;
    ATT_HANDLE_VALUE_PAIR  hndl_val_param;
    UCHAR                  val[4];
    UINT16                 count;

    /* Initialize record count */
    appl_record_count = 0;

    if (APPL_POS_RACP_STRD_RECORD_CNT_REQ_OPCODE == appl_timer_param->appl_req_param.req_op_code
        && APPL_POS_RACP_SUCCESS == appl_timer_param->rsp_code_val)
    {
        val[0] = APPL_POS_RACP_STRD_RECORD_CNT_RSP_OPCODE;
        val[1] = APPL_POS_RACP_NULL_OPRTR;

        if ((APPL_POS_RACP_ALL_RCRD_OPRTR == appl_timer_param->appl_req_param.oprtr)
            && (appl_clr_all_records != BT_TRUE))
        {
            count = appl_stored_record_count;
        }
        else if((APPL_POS_RACP_ALL_RCRD_OPRTR == appl_timer_param->appl_req_param.oprtr)
            && (appl_clr_all_records == BT_TRUE))
        {
            appl_stored_record_count = 0;
            count = appl_stored_record_count;
        }

        val[2] = (UCHAR)(count);
        val[3] = (UCHAR)(count >> 8);
    }
    else
    {
        val[0] = APPL_POS_RACP_RSP_OPCODE;
        val[1] = APPL_POS_RACP_NULL_OPRTR;
        val[2] = appl_timer_param->appl_req_param.req_op_code;
        val[3] = appl_timer_param->rsp_code_val;
    }

    hndl_val_param.handle = appl_pos_racp_hndl;
    hndl_val_param.value.len = 4;
    hndl_val_param.value.val = &val[0];

    retval = BT_att_send_hndl_val_ind
             (
                 &APPL_GET_ATT_INSTANCE(appl_timer_param->handle),
                 &hndl_val_param
             );

    /* Intialize the operating state */
    appl_pos_operating_state = APPL_RACP_INIT_STATE;

    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        retval = BT_stop_timer (timer_handle);
        APPL_TRC (
        "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
        retval,timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }
    return retval;
}

#ifdef APPL_MENU_OPS
#ifndef BT_SW_TXP
void appl_profile_menu_handler (void)
{
    int                   choice;
    API_RESULT            retval;
    ATT_HANDLE_VALUE_PAIR hndl_val_param;

    fsm_post_event
    (
        APPL_FSM_ID,
        ev_appl_device_init_req,
        NULL
    );

    CONSOLE_OUT("\n");
    CONSOLE_OUT("%s", pos_sensor_menu);
    CONSOLE_IN("%d", &choice);

    switch(choice)
    {
    case 0:
        CONSOLE_OUT ("\nExiting BlueLitE Menu Based Application ... \n\n");
        exit(0);

    case 1:
        CONSOLE_OUT ("\nRefreshing ...\r\n\n");
        break;

    case 2:
        printf("Start sending the PLX Spot Check mesument with only mandatory fields......\n");
        if (BT_FALSE != pos_spot_chk_msrmt_enabled)
        {
            hndl_val_param.handle =  appl_pox_spot_chk_msrmt_handle;
            hndl_val_param.value.val = spot_chk_msrmt_with_only_mandtry_flds;
            hndl_val_param.value.len = sizeof(spot_chk_msrmt_with_only_mandtry_flds);

            retval = BT_att_send_hndl_val_ind
                     (
                         &APPL_GET_ATT_INSTANCE(appl_hndl),
                         &hndl_val_param
                     );
        }
        break;

    case 3:
        printf("Start clearing all the stored spot check measurement records.....\n");
        appl_clr_all_records = BT_TRUE;
        break;

    case 6:
        printf("Start sending the PLX Continuous mesument with only mandatory fields");
        if (BT_FALSE != pos_continus_msrmt_enabled)
        {
            hndl_val_param.handle =  appl_pox_continus_msrmt_handle;
            hndl_val_param.value.val = contins_msrmt_with_only_mandtry_flds;
            hndl_val_param.value.len = sizeof(contins_msrmt_with_only_mandtry_flds);

            retval = BT_att_send_hndl_val_ntf
                     (
                         &APPL_GET_ATT_INSTANCE(appl_hndl),
                         &hndl_val_param
                     );
        }
        break;
    }
}
#else /* BT_SW_TXP */
void console_app_read (UCHAR * choice, UCHAR * value, UINT16 length)
{
    ATT_HANDLE_VALUE_PAIR hndl_val_param;
    API_RESULT            retval;

    /* Check the choice */
    switch (*choice)
    {
        case 0:
            *choice = 1;
            return;

        case 1:
            CONSOLE_OUT("%s", main_options);
            break;

        case 2:
            appl_power_on_device();
            *choice = 1;
            break;

        case 3:
            CONSOLE_OUT("Initiated Disconnection\r\n");
            /* Disconnect connected device */
            *choice = 1;
            appl_disconnect_device();
            break;

        case 4:
            CONSOLE_OUT("Start sending the POS Spot Check msrmt......\r\n");
            if (BT_FALSE != pos_spot_chk_msrmt_enabled)
            {
                hndl_val_param.handle =  appl_pox_spot_chk_msrmt_handle;
                hndl_val_param.value.val = spot_chk_msrmt_with_only_mandtry_flds;
                hndl_val_param.value.len = sizeof(spot_chk_msrmt_with_only_mandtry_flds);

                retval = BT_att_send_hndl_val_ind
                         (
                             &APPL_GET_ATT_INSTANCE(appl_hndl),
                             &hndl_val_param
                         );
            }
            *choice = 1;
            break;

        case 5:
            CONSOLE_OUT("Start clearing all the stored spot check measurement records.....\r\n");
            appl_clr_all_records = BT_TRUE;
            *choice = 1;
            break;

        case 6:
            CONSOLE_OUT("Start sending the POS Continuous msrmt...\r\n");
            if (BT_FALSE != pos_continus_msrmt_enabled)
            {
                hndl_val_param.handle =  appl_pox_continus_msrmt_handle;
                hndl_val_param.value.val = contins_msrmt_with_only_mandtry_flds;
                hndl_val_param.value.len = sizeof(contins_msrmt_with_only_mandtry_flds);

                retval = BT_att_send_hndl_val_ntf
                         (
                             &APPL_GET_ATT_INSTANCE(appl_hndl),
                             &hndl_val_param
                         );
            }
            *choice = 1;
            break;

        default:
            CONSOLE_OUT("\r\nInvalid Option\r\n");
            *choice = 1;
            break;
    }
}

const char * appl_get_profile_menu (void)
{
    return main_options;
}
#endif /* BT_SW_TXP */
#endif /* APPL_MENU_OPS */

API_RESULT appl_pos_racp_write_handler
           (
                ATT_VALUE         * value
           )
{
    API_RESULT          retval;
    API_RESULT          retv;
    RACP_REQ_INFO       pos_racp_req_info;

    retval = API_SUCCESS;

    if (APPL_RACP_INIT_STATE == appl_pos_operating_state ||
        APPL_POS_RACP_ABRT_RPRTING_REQ_OPCODE == value->val[0])
    {
        if (1 < value->len)
        {
            retv = BT_racp_req_handler
                   (
                       pos_racp_id,
                       value->val,
                       value->len,
                       &pos_racp_req_info
                   );

            if (API_SUCCESS != retv)
            {
                appl_pos_send_rsp_handler(&pos_racp_req_info, value->len, retv);
            }
        }
        else
        {
            retval = ATT_INVALID_ATTRIBUTE_LEN | APPL_ERR_ID;
        }
    }
    else
    {
        retval = ATT_PROCEDURE_ALREADY_IN_PROGRESS | APPL_ERR_ID;
    }

    return retval;
}

void appl_pos_send_rsp_handler
     (
         RACP_REQ_INFO   * pos_racp_req_info,
         UINT16            len,
         UINT16            result
     )
{
    API_RESULT retval;
    APPL_TIMER_PARAM    appl_timer_param;

    appl_timer_param.handle = appl_hndl;
    appl_timer_param.rsp_code_val = (UCHAR)result;
    appl_timer_param.appl_req_param.req_op_code = pos_racp_req_info->racp_opcode;
    appl_timer_param.appl_req_param.oprtr = pos_racp_req_info->racp_operator;

    BT_mem_copy
    (
        appl_timer_param.appl_req_param.operand,
        pos_racp_req_info->racp_operand,
        len
    );

    retval = BT_start_timer
             (
                 &timer_handle,
                 APPL_RECORD_TRNSFER_INTERVAL,
                 appl_timer_expiry_handler,
                 &appl_timer_param,
                 sizeof(APPL_TIMER_PARAM)
             );
}

void appl_timer_expiry_handler(void *data, UINT16 datalen)
{
    API_RESULT         retval;
    APPL_TIMER_PARAM * appl_timer_param;

    appl_timer_param = (APPL_TIMER_PARAM *)data;

    if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
    {
        retval = BT_stop_timer(timer_handle);
        APPL_TRC(
            "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
            retval, timer_handle);
        timer_handle = BT_TIMER_HANDLE_INIT_VAL;
    }

    retval = appl_pos_racp_send_response(appl_timer_param);
    APPL_TRC("[** ERR **]: Failed to send pos Measurement,"
    " reason 0x%04X", retval);
}

API_RESULT appl_pos_access_record
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           )
{
    API_RESULT retval;
    APPL_TIMER_PARAM appl_timer_param;

    retval = API_SUCCESS;

    appl_timer_param.handle = appl_hndl;
    appl_timer_param.rsp_code_val = APPL_POS_RACP_SUCCESS;
    appl_timer_param.appl_req_param.req_op_code = req_info->racp_opcode;
    appl_timer_param.appl_req_param.oprtr = req_info->racp_operator;

    BT_mem_copy
    (
        appl_timer_param.appl_req_param.operand,
        req_info->racp_operand,
        length
    );

    appl_racp_manage_transfer(&appl_timer_param);

    return retval;
}

API_RESULT appl_pos_report_stored_records
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           )
{
    API_RESULT retval;

    retval = appl_pos_access_record(req_info, length);

    if (API_SUCCESS != retval)
    {
        APPL_TRC(
            "[APPL]: Accessing the records failed with result 0x%04X,", retval);
    }
    return retval;
}

API_RESULT appl_pos_report_stored_number_of_records
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           )
{
    API_RESULT retval;

    retval = appl_pos_access_record(req_info, length);

    if (API_SUCCESS != retval)
    {
        APPL_TRC(
            "[APPL]: Accessing the records failed with result 0x%04X,", retval);
    }
    return retval;
}

API_RESULT appl_pos_delete_stored_record
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           )
{
    API_RESULT retval;

    retval = appl_pos_access_record(req_info, length);

    if (API_SUCCESS != retval)
    {
        APPL_TRC(
            "[APPL]: Accessing the records failed with result 0x%04X,", retval);
    }
    return retval;
}

API_RESULT appl_pos_abort_operation
           (
               RACP_REQ_INFO * req_info,
               UINT16          length
           )
{
    API_RESULT retval;

    if (APPL_RACP_TRANSFR_ONGOING_STATE == appl_pos_operating_state
        || APPL_RACP_CLR_STATE == appl_pos_operating_state)
    {
        /* Stop Ongoing transfer */
        if (BT_TIMER_HANDLE_INIT_VAL != timer_handle)
        {
            retval = BT_stop_timer(timer_handle);
            APPL_TRC(
            "[APPL]: Stopping Timer with result 0x%04X, timer handle %p\n",
            retval, timer_handle);
            timer_handle = BT_TIMER_HANDLE_INIT_VAL;
        }
    }
    appl_pos_operating_state = APPL_RACP_INIT_STATE;

    retval = appl_pos_access_record(req_info, length);

    if (API_SUCCESS != retval)
    {
        APPL_TRC(
            "[APPL]: Accessing the records failed with result 0x%04X,", retval);
    }

    return retval;
}

#endif /* POS */

