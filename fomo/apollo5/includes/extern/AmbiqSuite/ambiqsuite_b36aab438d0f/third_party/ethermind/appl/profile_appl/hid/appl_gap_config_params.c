
/**
 *  \file appl_gap_config_params.c
 *
 *  This file contains GAP Configuration Parameters used by the application.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------------------- Header File Inclusion */
#include "appl_gap.h"

#ifdef HID

/* --------------------------------------------- External Global Variables */

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

#if ((defined APPL_GAP_BROADCASTER_SUPPORT) || (defined APPL_GAP_PERIPHERAL_SUPPORT))
/** Advertisement Data Options */
const APPL_GAP_ADV_DATA appl_gap_adv_data[APPL_GAP_MAX_ADV_DATA_OPTIONS] =
{
    /* GAP Advertisement Parameters */
    {
        {
            /**
             *  Flags:
             *      0x01: LE Limited Discoverable Mode
             *      0x02: LE General Discoverable Mode
             *      0x04: BR/EDR Not Supported
             *      0x08: Simultaneous LE and BR/EDR to Same Device
             *            Capable (Controller)
             *      0x10: Simultaneous LE and BR/EDR to Same Device
             *            Capable (Host)
             */
            0x02, 0x01,
            (BT_AD_FLAGS_LE_GENERAL_DISC_MODE | BT_AD_FLAGS_LE_BR_EDR_SUPPORT),

            /**
             *  Service UUID List:
             *      DeviceInformation Service (0x180A)
             *      Battery Service (0x180F)
             *      HumanInterfaceDevice Service (0x1812)
             *      ScanParameter Service (0x1813)
             */
            0x09, 0x03, 0x0A, 0x18, 0x0F, 0x18, 0x12, 0x18, 0x13, 0x18,
           /**
             * Appearence:
             *      HID Mouse & Keyboard (0x03C0)
             */
            0x03, 0x19, 0xC0, 0x03,
            /**
             *  Shortened Device Name: EtherMin
             */
            0x09, 0x08, 0x45, 0x74, 0x68, 0x65, 0x72, 0x4d, 0x69, 0x6e
        },

        27
    }
};

/* Advertisement parameters options */
const APPL_GAP_ADV_PARAM appl_gap_adv_param[APPL_GAP_MAX_ADV_PARAM_OPTIONS] =
{
    /* 0 - Normal Advertising Params */
    {
        32,

        32,

        7,

        0
    },
    /* 1 - Fast Connection Advertising Params */
    {
        32,

        48,

        7,

        0
    },
    /* 2 - Low Power Advertising Params */
    {
        1600,

        4000,

        7,

        0
    }
};

/* Advertisement Table */
APPL_GAP_ADV_INFO appl_gap_adv_table =
{
    appl_gap_adv_data,

    appl_gap_adv_param,

    APPL_GAP_ADV_IDLE
};
#endif /* APPL_GAP_BROADCASTER || APPL_GAP_PERIPHERAL_SUPPORT */

#if ((defined APPL_GAP_OBSERVER_SUPPORT) || (defined APPL_GAP_CENTRAL_SUPPORT))
/* Scan Parameters Option */
const APPL_GAP_SCAN_PARAM appl_gap_scan_param[APPL_GAP_MAX_SCAN_PARAM_OPTIONS] =
{
    /* Normal Scan Params */
    {
        32,

        7,

        0
    },
    /* Fast Connection Scan Params */
    {
        48,

        7,

        0
    },
    /* Low Power Scan Params */
    {
        4000,

        7,

        0
    }
};

/* Scan Table */
APPL_GAP_SCAN_INFO    appl_gap_scan_table =
{
    appl_gap_scan_param,

    APPL_GAP_SCAN_IDLE
};
#endif /* APPL_GAP_OBSERVER_SUPPORT || APPL_GAP_CENTRAL_SUPPORT */

#ifdef APPL_GAP_CENTRAL_SUPPORT
/* Connection Parameters Options */
const APPL_GAP_CONN_PARAM appl_gap_conn_param[APPL_GAP_MAX_CONN_PARAM_OPTIONS] =
{
    {
        4,

        4,

        0,

        40,

        56,

        0,

        955,

        32,

        32
    }
};

/* GAP Connection Table */
APPL_GAP_CONN_INFO    appl_gap_conn_table =
{
    appl_gap_conn_param,

    APPL_GAP_CONN_IDLE
};
#endif /* APPL_GAP_CENTRAL_SUPPORT */
#endif /* HID */

