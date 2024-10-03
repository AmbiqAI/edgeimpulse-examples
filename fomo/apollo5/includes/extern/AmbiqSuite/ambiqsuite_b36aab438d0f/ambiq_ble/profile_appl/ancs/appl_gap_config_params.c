//*****************************************************************************
//
//! @file appl_gap_config_params.c
//!
//! @brief This file contains GAP Configuration Parameters used by the application.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

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

#ifdef ANCS

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
             *      Battery Service (0x180F)
             *      DeviceInformation Service (0x180A)
             */
            0x05, 0x03, 0x0F, 0x18, 0x0A, 0x18,

            /**
             *  Shortened Device Name: AM
             */
            0x03, 0x08, 0x41, 0x4d,

            /**
             *  List of 16-bit Service Solicitation UUIDs(ANCS UUID)
             */
            0x11, 0x15, 0xD0, 0x00, 0x2D, 0x12, 0x1E, 0x4B, 0x0F, 0xA4, 0x99, 0x4E, 0xCE, 0xB5, 0x31, 0xF4, 0x05, 0x79
        },

        31
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
#endif /* ANCS */
