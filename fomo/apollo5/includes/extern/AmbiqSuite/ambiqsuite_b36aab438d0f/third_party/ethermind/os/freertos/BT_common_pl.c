
/**
 *  \file BT_common_pl.c
 *
 *  Common routines and start-up initialization & shutdown handlers
 *  (Platform: FREERTOS)
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* ------------------------------------------- Header File Inclusion */
#include "BT_common.h"

#ifdef BT_UART
    #include "hci_uart.h"
#endif /* BT_UART */

#ifdef BT_USB
    #include "hci_usb.h"
#endif /* BT_USB */

#ifdef BT_BCSP
    #include "bcsp.h"
#endif /* BT_BCSP */

#ifdef BT_SOCKET
    #include "hci_socket.h"
#endif /* BT_SOCKET */

#ifdef BT_SPI
    #if defined(AM_PART_APOLLO3) || defined(AM_PART_APOLLO3P)
    #include "hci_drv_apollo3.h"
    #else
	#include "hci_spi_em9304.h"
    #endif
#endif /* BT_SPI */

#ifdef SMP
    #include "smp.h"
#endif /* SMP */

#ifdef ATT
    #include "att.h"
#endif /* ATT */

/* ------------------------------------------- External Global Variables */


/* ------------------------------------------- External Global Variables */


/* ------------------------------------------- External Global Variables */


/* ------------------------------------------- Functions */
/* EtherMind-Init: Platform Lower Handler */
void ethermind_init_lower_pl (void)
{
#ifdef BT_UART
    /* UART Initialization */
    hci_uart_init();
#endif /* BT_UART */

#ifdef BT_USB
    /* USB Initialization */
    hci_usb_init();
#endif /* BT_USB */

#ifdef BT_BCSP
    /* BCSP Initialization */
    bcsp_init();
#endif /* BT_BCSP */

#ifdef BT_SOCKET
    /* Socket Initialization */
    hci_socket_init();
#endif /* BT_SOCKET */

#ifdef BT_SPI
	/* SPI Initialization */
	hci_spi_init();
#endif
}


/* EtherMind-Init: Platform Upper Handler */
void ethermind_init_upper_pl (void)
{
#ifdef SMP
    em_smp_init();
#endif /* SMP */

#ifdef ATT
    em_att_init();
#endif /* ATT */
}


/* Bluetooth-ON: Platform Lower Handler */
void bluetooth_on_lower_pl (void)
{
#ifdef BT_UART
    /* UART BT Init */
    hci_uart_bt_init();
#endif /* BT_UART */

#ifdef BT_USB
    /* USB BT Init */
    hci_usb_bt_init();
#endif /* BT_USB */

#ifdef BT_BCSP
    /* BCSP BT Init */
    bcsp_bt_init();
#endif /* BT_BCSP */

#ifdef BT_SOCKET
    /* Socket BT Init */
    hci_socket_bt_init();
#endif /* BT_SOCKET */

#ifdef BT_SPI
    /* SPI Initialization */
	hci_spi_bt_init(true);
#endif
}


/* Bluetooth-ON: Platform Upper Handler */
void bluetooth_on_upper_pl (void)
{
#ifdef SMP
    smp_bt_init();
#endif /* SMP */

#ifdef ATT
    att_bt_init ();
#endif /* ATT */
}


#ifndef BT_NO_BLUETOOTH_OFF

/* Bluetooth-OFF: Platform Lower Handler */
void bluetooth_off_lower_pl (void)
{
#ifdef BT_SPI
	/* SPI Shutdown */
	hci_spi_bt_shutdown();
#endif

#ifdef BT_SOCKET
    /* SOCKET BT Shutdown */
    hci_socket_bt_shutdown();
#endif /* BT_SOCKET */

#ifdef BT_BCSP
    /* BCSP BT Shutdown */
    bcsp_bt_shutdown();
#endif /* BT_BCSP */

#ifdef BT_USB
    /* USB BT Shutdown */
    hci_usb_bt_shutdown();
#endif /* BT_USB */

#ifdef BT_UART
    /* UART BT Shutdown */
    hci_uart_bt_shutdown();
#endif /* BT_UART */
}


/* Bluetooth-OFF: Platform Upper Handler */
void bluetooth_off_upper_pl (void)
{
#ifdef ATT
    /* Attribute Protocol BT Shutdown */
    att_bt_shutdown ();
#endif /* ATT */

#ifdef SMP
    smp_bt_shutdown ();
#endif /* SMP */
}

#endif /* BT_NO_BLUETOOTH_OFF */

