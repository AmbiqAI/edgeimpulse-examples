
/**
 *  \file BT_serial.h
 *
 *  Serial Header file for Windows User
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_SERIAL_
#define _H_BT_SERIAL_

/* -------------------------------------------- Header Inclusion */
#include "BT_os.h"

/* -------------------------------------------- Type Definitions */

/* -------------------------------------------- Structures & Data Types */
/* Serial File Descriptor Type */
typedef int BT_SERIAL_FD;



/* Serial Parameters */
typedef struct
{
    /* Serial Baudrate */
    int baudrate;

    /*
     *  Flag - To indicate use of Hardware Flow Control (CTS/RTS).
     *  Values: 0x00 - No Hardware Flow Control.
     *          0x01 - Hardware Flow will be configured, if possible.
     */
    unsigned char use_hw_flow;

    /*
     *  Flag - To indicate use of Parity.
     *  Values: 0x00 - Parity Generation & Check will be Disabled.
     *          0x01 - Parity Generation & Check will be Enabled.
     */
    unsigned char use_parity;

    /*
     *  Parity Type to use.
     *  Values: 0x00 - Even Parity.
     *          0x01 - Odd Parity.
     */
    unsigned char parity_type;

    /*
     *  Parity Error Replacement Character.
     */
    unsigned char parity_char;

} BT_SERIAL_PARAMS;



/* -------------------------------------------- API/Function Declarations */

INT32 BT_set_serial_params (BT_SERIAL_FD fd, BT_SERIAL_PARAMS *params);


#endif /* _H_BT_SERIAL_ */


