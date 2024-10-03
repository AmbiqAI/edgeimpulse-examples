
/**
 *  \file BT_serial.c
 *
 *  This file defines utility routines to set/reset various serial
 *  port parameters.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* --------------------------------- Header File Inclusion */
#include "BT_serial.h"

/* --------------------------------- Static Global Variables */



/* --------------------------------- Function Definitions */


/*
 *  Function Name:
 *    BT_set_serial_params
 *
 *  Description:
 *    This function is used to set the parameters for the serial port.
 *
 *  Input Parameters:
 *    BT_SERIAL_FD fd, BT_SERIAL_PARAMS *params
 *
 *  Output Parameters:
 *    int.
 */

INT32 BT_set_serial_params (BT_SERIAL_FD fd, BT_SERIAL_PARAMS *params)
{
#ifndef FREERTOS

    struct termios tty;
    int mcs = 0, spd = -1;

    /* Save Baudrate */
    spd = params->baudrate;

    if (tcgetattr (fd, &tty) < 0)
    {
        return -1;
    }

    /* ---------------------------------------------- Input Mode Flags */
    /*
     *  IGNBRK = 1
     *    => BREAK Condition Ignored.
     *  IGNBRK = 0
     *    => BREAK Condition NOT Ignored.
     *    BRKINT = 1
     *      => Generate SIGINT on BREAK.
     *    BRKINT = 0
     *      => Read BREAK as '\0'.
     *
     *  INPCK  = 0
     *    => No Parity Checking on Input.
     *  INPCK  = 1
     *    => Enable Input Parity Checking.
     *    IGNPAR = 1
     *      => Parity Errors are Ignored.
     *    IGNPAR = 0
     *      PARMRK = 1
     *        => Read Parity Errors as \377 \0.
     *      PARMRK = 0
     *        => Read Parity Errors as \0.
     *
     *  ISTRIP = 0
     *    => No Stripping of Eighth Bit.
     *
     *  INCLR, IGNCR, ICRNL, IUCLC = 0
     *    => No Input Character Processing/Converting.
     *
     *  IXON, IXOFF, IXANY
     *    => No XON/XOFF Flow Control.
     *
     *  IMAXBEL = 1
     *    => Ring Bell when Input Queue is Full (?!)
     */

    /*
     *  Defaults
     *    - Ignore Break
     *    - Ignore Parity and Framing Errors
     */
    tty.c_iflag = 0x0;
    tty.c_iflag |= IGNBRK;
    tty.c_iflag |= IGNPAR;

    /* Enable Parity Checking */
    if (0x1 == params->use_parity)
    {
        tty.c_iflag |= INPCK;
    }
    /* --------------------------------------------------------------- */


    /* ---------------------------------------------- Output Mode Flags */
    /* No Output Processing */
    tty.c_oflag = 0x0;
    /* --------------------------------------------------------------- */


    /* ---------------------------------------------- Control Mode Flags */
    /*
     *  CSIZE => Character Size Masks
     *    CS5, CS6, CS7, CS8.
     *
     *  CSTOPB = 0
     *    => One Stop Bit.
     *  CSTOPB = 1
     *    => Two Stop Bit.
     *
     *  CREAD = 1
     *    => Enable Receiver.
     *
     *  PARENB = 1
     *    => Enable Parity Generation on Output & Checkin on Input.
     *    => Otherwise, No Parity ?!
     *
     *  PARODD = 1
     *    => Parity for Input & Output is Odd.
     *    => Otherwise, Even ?!
     *
     *  HUPCL = 1
     *    => Lower Modem Control Lines at the time of Hang Up.
     *
     *  CLOCAL = 1
     *    => Ignore Modem Control Lines.
     *
     *  CRTSCTS = 1
     *    => Enable Harware Flow Control.
     */

    /*
     *  Defaults
     *    - 8 Data Bit, No Parity, 1 Stop Bit
     *    - Local Mode, Rx Enabled.
     *    - H/W Flow Disabled.
     */
    tty.c_cflag = 0x0;
    tty.c_cflag |= (CS8 | CREAD | CLOCAL);

    /* Enable Parity */
    if (0x1 == params->use_parity)
    {
       tty.c_cflag |= PARENB;

       if (0x1 == params->parity_type)
       {
           tty.c_cflag |= PARODD;
       }
    }

    /* Enable Hardware Flow Control */
    if (0x1 == params->use_hw_flow)
    {
        tty.c_cflag |= CRTSCTS;
    }

    /* Set Baudrate */
    cfsetospeed(&tty, (speed_t)spd);
    cfsetispeed(&tty, (speed_t)spd);
    /* --------------------------------------------------------------- */


    /* ---------------------------------------------- Local Mode Flags */
    /* Local Mode Flags => No Local Processing */
    tty.c_lflag = 0x0;
    /* --------------------------------------------------------------- */


    /* Minimum Number of Characters to trigger Read */
    tty.c_cc[VMIN] = 1;

    /* Return Read ASAP ! */
    tty.c_cc[VTIME] = 0;

    /* Set New Serial Attributes */
    if (tcsetattr(fd, TCSANOW, &tty) < 0)
    {
        return -1;
    }

    /* Flush */
    tcflush(fd, TCIOFLUSH);

    /* Set CTS/RTS */
    if (0x1 == params->use_hw_flow)
    {
        mcs = 0;
        ioctl(fd, TIOCMGET, &mcs);
        mcs |= TIOCM_RTS;
        ioctl(fd, TIOCMSET, &mcs);
    }
#endif /* FREERTOS */

    return 0;
}

