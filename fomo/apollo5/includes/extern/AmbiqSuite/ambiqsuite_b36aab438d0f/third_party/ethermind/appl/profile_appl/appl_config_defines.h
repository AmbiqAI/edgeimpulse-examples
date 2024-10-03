
/**
 *  \file appl_config_defines.h
 *
 *  ATT Server Application Header File
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_APPL_CONFIG_DEFINES_
#define _H_APPL_CONFIG_DEFINES_

/* --------------------------------------------- Header File Inclusion */

/* --------------------------------------------- Global Definitions */

#define APPL_GAP_MAX_ADV_DATA_OPTIONS   1
#define APPL_GAP_MAX_ADV_PARAM_OPTIONS  3
#define APPL_GAP_MAX_SCAN_PARAM_OPTIONS 3
#define APPL_GAP_MAX_CONN_PARAM_OPTIONS 1

#define APPL_GAP_NRML_ADV_PARAM_INDEX   0
#define APPL_GAP_FAST_ADV_PARAM_INDEX   1
#define APPL_GAP_LP_ADV_PARAM_INDEX     2

/**
 *  LE and BR/EDR specific bit fields for 'Flags' AD Type.
 *
 *  For LE Single Mode: 'BR/EDR Not Supported'.
 *  For LE & BR/EDR Dual Mode: 'Simultaneous LE and BR/EDR (Host) and
 *  (Controller).
 */
#ifdef BT_DUAL_MODE
#define BT_AD_FLAGS_LE_BR_EDR_SUPPORT \
        (BT_AD_FLAGS_LE_BR_EDR_CONTROLLER | BT_AD_FLAGS_LE_BR_EDR_HOST)
#else
#define BT_AD_FLAGS_LE_BR_EDR_SUPPORT  \
        (BT_AD_FLAGS_BR_EDR_NOT_SUPPORTED)
#endif /* BT_DUAL_MODE */

#endif /* _H_APPL_CONFIG_DEFINES_ */

