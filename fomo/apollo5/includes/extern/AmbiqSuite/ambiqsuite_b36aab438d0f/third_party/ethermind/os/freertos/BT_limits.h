
/**
 *  \file BT_limits.h
 *
 *  This file lists all the Tunable constants used in
 *  EtherMind Bluetooth Stack modules.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_LIMITS_
#define _H_BT_LIMITS_


/* ----------------------------------------------------------------------- */
/* =============================  Common  ================================ */
/* ----------------------------------------------------------------------- */
/*
 *  Maximum number of simultaneously connected Remote Devices to be supported.
 *  This define is used by HCI and L2CAP modules.
 *
 *  Other layers (ATT, SMP etc.) defines respective maximum number of
 *  supported remote entities, many of which are derived from this define.
 */
#define BT_MAX_REMOTE_DEVICES                           1

/*
 *  The number of bonded devices in addition to the BT_MAX_REMOTE_DEVICES
 *  that is required in the system. This value determines the number of
 *  entries in the SMP database
 */
#define BT_MAX_BONDED_LE_DEVICES                        0

/*
 *  Static Data Size used in EtherMind Modules.
 *  This is a very "sensitive" global definition. Most of the major
 *  logics in most important situations depend on this logic. So, please
 *  be sure before changing this value.
 *
 *  Probably, it is safer to increase this value. But it is a general
 *  risk if this value is decremented anyway.
 *
 *  Some of the modules who heavily depend on it:
 *    - Write Task
 *        + Queue Data Size is of this value.
 *        + For Internal & API Queues, Write Task will free associated
 *          memory if it is more than this value.
 *        + For Data Queues, Write Task does not free any memory, HCI is
 *          supposed to free when it transfered the data.
 *
 *    - HCI
 *        + Entire Zero Copy implementation logic depends on this value.
 *
 *    - SDP, RFCOMM, BNEP, L2CAP
 *        + For Zero Copy Implementation.
 *
 *    - Timer Library
 *        + For user data storage in Timer Queues.
 *
 *  Minimum Value: 32  (Do NOT change it !)
 *  Maximum Value: anything
 */
#ifndef BT_NO_STATIC_DATA_SIZE
#define BT_STATIC_DATA_SIZE                             32
#endif /* BT_NO_STATIC_DATA_SIZE */


/* ----------------------------------------------------------------------- */
/* =============================  Timer  ================================= */
/* ----------------------------------------------------------------------- */
/*
 *  Number of simultaneous Timer Entities that Timer Library will support.
 *
 *  Minimum Value: 1
 *  Maximum Value: anything
 */
#define TIMER_MAX_ENTITIES                              4


/* ----------------------------------------------------------------------- */
/* =============================  Write Task  ============================ */
/* ----------------------------------------------------------------------- */
/*
 *  The Write Task Queue Sizes for various types of Queues. Number of
 *  element in the User Data Queue must be more, if the application is
 *  expected to write data very frequently. Number of elements in the
 *  User API Queue is a matter of judgement - depends on the application
 *  or profile again - depends on how many simultaneous APIs may be issued
 */
#define WT_INTERNAL_QUEUE_SIZE                          3
#define WT_USER_API_QUEUE_SIZE                          3


/* ----------------------------------------------------------------------- */
/* =============================  HCI Transport ========================== */
/* ----------------------------------------------------------------------- */
/*
 *  This parameter specifies the maximum size of the HCI Transport's
 *  circular buffer to store incoming HCI packets before they are
 *  delivered to HCI. This value should be sufficiently large enough
 *  to store all incoming HCI packets under a given load condition, till
 *  Read Task (2-task model) or Write Task (1-task model) reads the packets.
 *
 *  This parameter must be define to be a value which is 2 raised
 *  to the power of 'some' N. Example: 16, 32, 64, 128, 256, 512, 1024 etc.
 *
 *  Minimum Value: depends on requirement.
 *  Maximum Value: depends on requirement.
 */
#define HCI_TRANSPORT_MAX_BUFFER_SIZE                   4096

#if (((HCI_TRANSPORT_MAX_BUFFER_SIZE) == 0)        \
    ||(((HCI_TRANSPORT_MAX_BUFFER_SIZE) &          \
       ((HCI_TRANSPORT_MAX_BUFFER_SIZE) - 1)) != 0))
    #error "HCI_TRANSPORT_MAX_BUFFER_SIZE value is not in 2^n format"
#endif /* HCI_TRANSPORT_MAX_BUFFER_SIZE */

/*
 *  The HCI Transport Read Task (or, the Write Task) reads one complete
 *  HCI packet from the circular Transport buffer into another static
 *  buffer before HCI's callback for incoming data reception is invoked.
 *  This parameter specifies the size of this static data buffer which
 *  is used to deliver data to HCI. The size of this buffer should be
 *  chosen such a way so that it can accomodate any HCI packet under any
 *  condition.
 *
 *  Minimum Value: depends on the host controller.
 *  Maximum Value: depends on the host controller.
 */
#define HCI_TRANSPORT_MAX_PACKET_SIZE                   1024

/*
 *  Low & High Water Marks for the HCI Transport's circular buffer.
 *
 *  If the amount of free space in the circular buffer becomes less than
 *  or equal to 'HCI_TRANSPORT_THRESHOLD_LOW', HCI Transport Driver's
 *  'Throttle' routine is invoked to signal that incoming data reception
 *  should be throttled at this point.
 *
 *  If the amount of data queued in the circular buffer becomes less than
 *  or equal to 'HCI_TRANSPORT_THRESHOLD_HIGH', HCI Transport Driver's
 *  'Unthrottle' routine is invoked to signal that incoming data reception
 *  should be unthrottled at this point.
 *
 *  Minimum Value: depends on requirement.
 *  Maximum Value: depends on requirement.
 */
#define HCI_TRANSPORT_BUFFER_THRESHOLD_LOW              512
#define HCI_TRANSPORT_BUFFER_THRESHOLD_HIGH             128


/* ----------------------------------------------------------------------- */
/* =============================  UART  ================================== */
/* ----------------------------------------------------------------------- */
/*
 *  Size of the buffer the HCI-UART module uses to recombine fragments of
 *  a HCI ACL Data or HCI Command packet before the same is written to the
 *  underlying driver for physical transport to the Bluetooth chip.
 *
 *  Minimum Value: 260 or (1+4+ Bluetooth Controller's Max ACL Buffer Size),
 *                 whichever is more.
 *  Maximum Value: depends on requirement.
 */
#define HCI_UART_WR_BUF_SIZE                            2048


/* ----------------------------------------------------------------------- */
/* ----------------------------------------------------------------------- */
/* =============================  SPI  ================================== */
/* ----------------------------------------------------------------------- */
/*
 *  Size of the buffer the HCI-SPI module uses to recombine fragments of
 *  a HCI ACL Data or HCI Command packet before the same is written to the
 *  underlying driver for physical transport to the Bluetooth chip.
 *
 *  Minimum Value: 260 or (1+4+ Bluetooth Controller's Max ACL Buffer Size),
 *                 whichever is more.
 *  Maximum Value: depends on requirement.
 */
#define HCI_SPI_WR_BUF_SIZE                            2048


/* ----------------------------------------------------------------------- */
/* =============================  BCSP  ================================== */
/* ----------------------------------------------------------------------- */
/*
 *  Maximum BCSP Packet Payload Length. This value is mainly driven by the
 *  values returned by the Host Controller in response to Read Buffer Size
 *  command, for the ACL & SCO Buffer size. This parameter must have a
 *  value larger than the largest of the Host Controller reported buffer
 *  sizes for ACL & SCO packets.
 */
#define BCSP_MAX_PAYLOAD_LENGTH                         4096

/*
 *  BCSP SLIP Layer transmits packet with SLIP byte stuffing algorithm.
 *  There are two approaches to do this:
 *    1. Each byte is SLIP encoded and written to UART separately.
 *       Result -> Lots of UART write. For every byte UART write API will
 *                 be called once.
 *    2. Each byte is encoded and put into a buffer. When this buffer is
 *       almost full, UART write is called to flush the buffer, and proceed
 *       with further encoding.
 *       Result -> Number of UART write is reduced drastically compared
 *                 to the previous scenario. Faster and efficient.
 *
 *  BCSP adopted the 2nd method, and this parameter defines the size of
 *  the buffer used to store the encoded bytes temporarily before flushing
 *  out on the UART.
 *
 *  Performance can be best optimized if the size of this buffer is twice
 *  the maximum size that SLIP encoding routine can be asked to write, which
 *  in turn depends on the Host Controller's buffer sizes.
 */
#define BCSP_SLIP_TX_SIZE                               1500

/*
 *   BCSP Link Establishment Timeout values and counters. There are two
 *   timers involved in the BCSP Link establishment - Tshy & Tconf. And
 *   one counter - Number of times CONF message has been retransmitted.
 */
#define BCSP_T_SHY_TIMEOUT                              1
#define BCSP_T_CONF_TIMEOUT                             1
#define BCSP_MAX_CONF_TIMEOUT_LIMIT                     20

/*
 *  BCSP Sequencing Layer provides reliable data transfer with the help
 *  packet sequencing and retransmission. To enable possible retransmission
 *  of a transmitted packet, BCSP needs to store them in its queue. This
 *  parameter defines the number of elements in this queue.
 *
 *  This number can be tuned on the basis of number of ACL or SCO
 *  buffers that the Host Controller has. Since HCI is bound to do
 *  flow control on the basis of the number of buffers that Host Controller
 *  has, BCSP need not maintain a very large queue here. A good value
 *  would be (number of HC buffers + 10), assuming HCI, in the meantime,
 *  will not send more than 10 commands.
 */
#define BCSP_MAX_SEQUENCE_TX_BUFFERS                    100

/*
 *  BCSP Sequencing - Packet Retransmission Timeout and Retransmission Limit.
 */
#define BCSP_SEQUENCE_RTX_TIMEOUT                       1
#define BCSP_SEQUENCE_RTX_LIMIT                         20

/*
 *  BCSP Sequencing - Transmit Window Size
 */
#define BCSP_WINDOW_SIZE                                4


/* ----------------------------------------------------------------------- */
/* =============================  HCI  =================================== */
/* ----------------------------------------------------------------------- */
/*
 *  Maximum number of connected Remote Devices maintained by HCI.
 */
#define HCI_MAX_REMOTE_DEVICES                          BT_MAX_REMOTE_DEVICES

/*
 *  HCI stores the outgoing commands to the Host Controller in a Global
 *  Command Queue. Transmission of the commands from this queue is
 *  depended upon the flow control of commands between the Host Controller
 *  and the Host (HCI). This parameter specifies the size of the HCI
 *  Command Queue.
 *
 *  Minimum Value: 2
 *  Maximum Value: can be anything.
 */
#define HCI_COMMAND_QUEUE_SIZE                          2

/*
 *  Length of the buffer packet used to enqueue HCI Events/Data received
 *  during the process of Private Address Resolution of a peer device
 *  address that has connected. Apart from the HCI payload, packet type
 *  and length are also stored.
 *
 *  This constant will be used only when HOST_RESOLVE_PVT_ADDR is defined
 *
 *  Minimum Value: (31+1+1)
 *  Maximum Value: can be anything.
 */
#define HCI_RPA_BUFFER_LEN                              (31+1+1)

/*
 *  Number of buffers of size HCI_RPA_BUFFER_LEN to be used as queue
 *  during the process of Address Resolution
 *
 *  This constant will be used only when HOST_RESOLVE_PVT_ADDR is defined
 *
 *  Minimum Value: 2
 *  Maximum Value: can be anything.
 */
#define HCI_NUM_RPA_BUFFERS                             4

/* ----------------------------------------------------------------------- */
/* =======================  Attribute Protocol============================ */
/* ----------------------------------------------------------------------- */

/**
 * ATT_RESPONSE_TIMEOUT
 *
 * ATT Response-Response TimeOut Value. Currently the value of the timer is set
 * to 30 sec in the specification. Can be changed for testing purposes only.
 *
 * Minimum Value: 30 Seconds.
 * Maximum Value: 30 Seconds.
 */
#define ATT_RESPONSE_TIMEOUT                             30

/**
 * ATT_MAX_CONNECTION_INSTANCES
 *
 * Maximum Instances supported by ATT. This is the maximum number of
 * simultaneous Transactions that the module can have. Typical use cases
 * would need as many ATT instances and connection instances, hence it is
 * mapped with BT_MAX_REMOTE_DEVICES.
 *
 * Minimum Value:   1.
 * Maximum Value: 255.
 */
#define ATT_MAX_CONNECTION_INSTANCES                 BT_MAX_REMOTE_DEVICES

/**
 * ATT_MAX_MTU
 *
 * Maximum MTU supported by local ATT on an Instance.
 * Local device should have the allocatable memory of this size,
 * for local L2CAP reassembly of this large ATT packet and also
 * for transmission of ATT PDU of this size.
 *
 * Minimum Value:  23.
 * Maximum Value:  65535 (Theoretical maximum).
 */
#define ATT_MAX_MTU                                     247


/* ----------------------------------------------------------------------- */
/* ====================  Security Manager Protocol  ====================== */
/* ----------------------------------------------------------------------- */

/*
 *  This constant defines the number of elements Security Manager's
 *  Device Database can have at most. Since the Security Manager should
 *  at least support those devices which the stack can simultaneously connect
 *  to, it must at least be equal to BT_MAX_REMOTE_DEVICES.
 *
 *  Moreover, in general, size of the SM's Device Database is preferred
 *  to be "BT_MAX_REMOTE_DEVICES + X", where X determines the additional
 *  count of bonded devices that might need to be supported in the system.
 *  This additional count is governed by the BT_MAX_BONDED_LE_DEVICES limit.
 *
 */
#define SMP_MAX_DEVICES                                (BT_MAX_REMOTE_DEVICES + BT_MAX_BONDED_LE_DEVICES)

/*
 *  This constant defines the number of elements Security Manager's
 *  table for Security Access Contexts.
 *
 *  Minimum Value: 1
 *  Maximum Value: depends on requirement.
 */
#define SMP_MAX_CONTEXTS                                2

/*
 *  This constant defines the Security Manager pairing procedure timeout
 *  values in seconds.
 *
 *  Minimum Value: depends on requirement
 *  Maximum Value: 30
 */
#define SMP_MAX_AUTHENTICATION_TIMEOUT                  30

/* ----------------------------------------------------------------------- */
/* ========================  Device Queue  =============================== */
/* ----------------------------------------------------------------------- */
/*
 *  This constant defines the number of elements the stacks
 *  device queue can have at most. Since the Device Queue should
 *  at least support those devices which the stack can bond to,
 *  it must at least be equal to SMP_MAX_DEVICES.
 *
 *  Moreover, in general, size of the Device Queue list is preferred
 *  to be "BT_MAX_BONDED_LE_DEVICES + X". X can be between 0 to
 *  "BT_MAX_REMOTE_DEVICES". The X number of entries determines the
 *  number of incoming and outgoing connections that the stack can
 *  connect to in addition to the BT_MAX_BONDED_LE_DEVICES number
 *  of bonded devices but bearing a maximum of BT_MAX_REMOTE_DEVICES
 *  active connections
 *
 *  Minimum Value: (SMP_MAX_DEVICES + 1)
 *  Maximum Value: As required
 */
#define BT_MAX_DEVICE_QUEUE_SIZE                        (SMP_MAX_DEVICES + 1)


/* ----------------------------------------------------------------------- */
/* =============================  L2CAP  ================================= */
/* ----------------------------------------------------------------------- */
/*
 *  Maximum number of L2CAP Buffers. This is maximum number of upper
 *  layer data write allowed at any point of time over L2CAP.
 *
 *  This constant is meaningful only if new L2CAP/HCI (1.2 Stack)
 *  is being used.
 *
 *  Minimum Value: 1
 *  Maximum Value: 254
 */
#define L2CAP_MAX_BUFFER_ELEMENTS                       10

/*
 *  L2CAP employs a Flow Control mechanism in which it is possible to
 *  provide feedback to application when number of available buffers in
 *  L2CAP's ACL Transmission Queue goes too low, or, too high. L2CAP
 *  provides this feedback using the Global ACL Transmission Queue Flow
 *  Control Callback, that can be registered by application after
 *  Bluetooth-ON.
 *
 *  When it is too low (ie, the number of free buffers reaches or goes below
 *  the Low Water Mark, LWM), the application can take necessary steps to stop
 *  making "data transmission requests" using various Stack/Profile APIs,
 *  for some time. L2CAP calls the registered callback again when number
 *  of available buffers reaches a High Water Mark (HWM) - at this point,
 *  application can take necessary steps to restart data transmission.
 *
 *  Typically, the LWM is set to ~10% and ~90% of the total number of
 *  buffers (ie, L2CAP_MAX_BUFFER_ELEMENTS), respectively. However, this
 *  should be fine tuned for the platform/system for various data throughput
 *  tests & observations.
 *
 *  Minimum Value: 1 (LWM), 2 (HWM)
 *  Maximum Value: 252 (LWM), 253 (HWM)
 */
#define L2CAP_NUM_BUFFER_ELEMENTS_LWM                   3
#define L2CAP_NUM_BUFFER_ELEMENTS_HWM                   7

/*
 *  Maximum number of credit based logical channels that L2CAP needs
 *  to support. Normally, for each PSM there might be one L2CAP channel. But
 *  there could also be more than one L2CAP channel for a PSM.
 *
 *  The number of Channels here is not inclusive of 3 fixed channels ATT,
 *  L2CAP and SMP.
 *  So, if the use case requires all 3 fixed channels i.e. ATT, L2CAP & SMP
 *  and 2 L2CAP CBFC channles in addition to those then the value of
 *  L2CAP_MAX_CHANNELS should be  2.
 *
 *  Minimum Value: 1
 *  Maximum Value: as required.
 */
#define L2CAP_MAX_CHANNELS                              2

/*
 *  Maximum number of credit based Flow Control PSM that L2CAP needs
 *  to support.
 *
 *  Minimum Value: 1
 *  Maximum Value: as required.
 */
#define L2CAP_MAX_CBFC_PSM                              2

/*
 *  Maximum number of commands that L2CAP needs to support.
 *
 *  There could be multiple outstanding commands for an L2CAP Connection
 *  Entity for a Remote Bluetooth Device, for which Responses are yet to
 *  be received. L2CAP Command Elements are maintained for this purpose
 *  to match received Response with the transmitted Command.
 *
 *  Minimum Value: 1
 *  Maximum Value: as required.
 */
#define L2CAP_MAX_COMMANDS                              1

/*
 *  After receiving L2CAP data/payload from peer device for a CBFC Channel,
 *  the available credits are calculated. If the credit count goes below this
 *  (low water mark) L2CAP_CBFC_RX_CREDITS_LWM, the application is notified of
 *  of this condition through the callback registered, so that if application
 *  wants can send more credits to the peer device.
 *
 *  Minimum Value: 0
 *  Maximum Value: Less than allowed maximum value L2CAP_MAX_CREDITS (0xFFFF)
 */
#define L2CAP_CBFC_RX_CREDITS_LWM                       0x0001

/**
 * L2CAP_RESPONSE_TIMEOUT
 *
 * L2CAP Response-Response TimeOut Value.
 *
 * Minimum Value: 1 Second.
 * Maximum Value: 60 Seconds.
 */
#define L2CAP_RESPONSE_TIMEOUT                             30


/* ----------------------------------------------------------------------- */
/* ===========================  RACP ===================================== */
/* ----------------------------------------------------------------------- */
/*
 *  Maximum number of modules which can register with RACP.
 *
 *  Minimum Value: 1
 *  Maximum Value: 255
 */
#define RACP_MAX_MODULES                                  1

#endif /* _H_BT_LIMITS_ */

