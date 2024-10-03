
/**
 *  \file BT_features.h
 *
 *  \brief EtherMind Compilation Switches Configuration File.
 *
 *  This file lists all the Compilation Flags available in various
 *  EtherMind Bluetooth Stack modules.
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_FEATURES_
#define _H_BT_FEATURES_

/* ----------------------------------------------------------------------- */
/* ========== Platform =================================================== */
/* ----------------------------------------------------------------------- */
/*
 *  FREERTOS
 *
 *  This flag is used when the EtherMind stack is being compiled on
 *  FreeRTOS platforms.
 *
 *  Dependency: None.
 */
/* Added ifndef in case of undefinition in compiler option */
#ifndef FREERTOS
#define FREERTOS
#endif /* FREERTOS */


/* ----------------------------------------------------------------------- */
/* ==== Bluetooth Specification Flags ==================================== */
/* ----------------------------------------------------------------------- */
/*
 *  BT_5_0
 *
 *  This flag is used when the EtherMind stack is being compiled for
 *  Bluetooth specification version 5.0.
 */
#define BT_5_0

/*
 *  BT_4_2
 *
 *  This flag is used when the EtherMind stack is being compiled for
 *  Bluetooth specification version 4.2.
 */
#define BT_4_2

/*
 *  BT_4_1
 *
 *  This flag is used when the EtherMind stack is being compiled for
 *  Bluetooth specification version 4.1.
 */
#define BT_4_1


/* ----------------------------------------------------------------------- */
/* ==== Bluetooth Feature Flags ========================================== */
/* ----------------------------------------------------------------------- */
/*
 *  BT_LE
 *
 *  This flag is used when the EtherMind stack is being compiled with/for
 *  Bluetooth Low Energy.
 *
 *  Currently, this flag only affects the L2CAP and HCI implementation.
 *  The appropriate source code must be chosen for compilation
 *  (the l2cap_le & hci_le source code directories).
 *  Dependency: None.
 */
#define BT_LE


/* ----------------------------------------------------------------------- */
/* ==== Stack Architecture Flags ========================================= */
/* ----------------------------------------------------------------------- */
/*
 *  This flag is used to perform the EtherMind Read Task's job using the
 *  EtherMind Write Task, and hence only one single task is created and used.
 *  For details on EtherMind Tasks and their usage/function, refer to
 *  EtherMind Stack Architecture Document, and EtherMind Programmer's Guide.
 *
 *  Dependency: None.
 */
/* #define BT_TRANSPORT_SINGLE_TASK */

/*
 *  BT_NO_STATIC_DATA_SIZE
 *
 *  This flag removes the usage of static buffers of size
 *  'BT_STATIC_DATA_SIZE' from the EtherMind stack.
 *
 *  Dependency: None.
 */
#define BT_NO_STATIC_DATA_SIZE

/*
 *  BT_SUPPORT_CONTROLLER_INIT_SKIP
 *
 *  This flag enables support for API to enable/disable
 *  controller initialization during Bluetooth ON.
 *
 *  The default behaviour of stack will be to initialize the
 *  controller.
 *
 *  Dependency: None.
 */
#define BT_SUPPORT_CONTROLLER_INIT_SKIP

/*
 *  BT_SUPPORT_GET_STACK_INIT_STATE
 *
 *  This flag enables support for BT_bluetooth_get_stack_init_state API
 *  used to get initialization state of Stack.
 *
 *  Dependency: None.
 */
#define BT_SUPPORT_GET_STACK_INIT_STATE


/* ----------------------------------------------------------------------- */
/* ==== HCI Transport Flags ============================================== */
/* ----------------------------------------------------------------------- */
/*
 *  BT_UART, BT_USB, BT_BCSP, BT_SOCKET, BT_SW_TXP
 *
 *  These flags are used to choose the HCI Transport module for communication
 *  with the Bluetooth hardware, in a hosted Stack.
 *
 *  For some platforms, these flags are defined in Makefiles & Workspaces.
 *  In such cases, these must not be enabled here.
 *
 *  Dependency: None.
 */
/* #define BT_UART */
/* #define BT_USB */
/* #define BT_BCSP */
/* #define BT_SOCKET */
#define BT_SPI
/* #define BT_SW_TXP */

/*
 *  HCI_LIBUSB_SUPPORT
 *
 *  This flag enables user space USB driver 'libusb' support.
 *
 *  Dependency: BT_USB
 */
#ifdef BT_USB
#define HCI_LIBUSB_SUPPORT
#endif /* BT_USB */

/*
 *  BT_CUSTOM_SERIAL_SETTINGS
 *
 *  This flag enables application to make use of hci_uart_set_serial_settings()
 *  API (defined in BT_api.h), to configure the COM/Serial Port (and, Baud
 *  Rate) on which the Bluetooth hardware is available or connected.
 *
 *  Dependency: The BT_UART must be defined.
 */
#define BT_CUSTOM_SERIAL_SETTINGS

/*
 *  HCI_UART_COLLECT_AND_WR_COMPLETE_PKT
 *
 *  This flag enables application to instruct HCI-UART module to recombine
 *  fragments of a HCI Command or Data packet before the same is send over
 *  to the underlying Bluetooth Controller.
 *
 *  Dependency: The BT_UART must be defined.
 */
#ifdef BT_UART
/* #define HCI_UART_COLLECT_AND_WR_COMPLETE_PKT */
#endif /* BT_UART */

/*
 *  HCI_SPI_COLLECT_AND_WR_COMPLETE_PKT
 *
 *  This flag enables application to instruct HCI-UART module to recombine
 *  fragments of a HCI Command or Data packet before the same is send over
 *  to the underlying Bluetooth Controller.
 *
 *  Dependency: The BT_UART must be defined.
 */
#ifdef BT_SPI
#define HCI_SPI_COLLECT_AND_WR_COMPLETE_PKT */
#endif /* BT_UART */

/*
 *  HT_DUMP_RX_BUF_ON_SYNC_LOSS
 *
 *  This flag enables dumping the entire transport receive buffer
 *  to help in analyzing when synchronization loss is detected.
 *
 *  Dependency: None.
 */
#define HT_DUMP_RX_BUF_ON_SYNC_LOSS

/*
 *  HT_THREAD_SAFE_ENQUEUE
 *
 *  Using hci_transport_enqueue() interface, HCI transport driver provides
 *  HCI Events and Data packets from HCI Controller as byte stream to
 *  the EtherMind stack. Typically, hci_transport_enqueue() is called from
 *  a single thread/execution context.
 *
 *  If in some scenario, the hci_transport_enqueue() can be called from more
 *  than one thread/execution context, like in the case of 'libusb' based
 *  implementation, where the HCI Event and Data packets are received from two
 *  separate threads on USB 'Interrupt' and 'Bulk In' endpoints, thread safe
 *  version of hci_transport_enqueue() shall be enabled, using this feature
 *  flag.
 */
#define HT_THREAD_SAFE_ENQUEUE

/*
 *  HT_ENQUEUE_WITH_RETURN
 *
 *  This flag enables hci_transport_enqueue() function to return the
 *  number of bytes successfully enqueued into the HT buffer.
 *
 *  Dependency: None.
 */
/* #define HT_ENQUEUE_WITH_RETURN */

/* ----------------------------------------------------------------------- */
/* ==== Stack Feature Flags ============================================== */
/* ----------------------------------------------------------------------- */
/*
 *  BT_SUPPORT_STACK_VERSION_INFO
 *
 *  This flag enables EtherMind BT_get_version_number() API.
 *
 *  This API is used to retrieve the current stack build version
 *  information
 *
 *  Dependency: None.
 */
#define BT_SUPPORT_STACK_VERSION_INFO

/*
 *  BT_DEBUG_ASSERT
 *
 *  This flag enables EtherMind BT_assert() macro, for evaluating truth
 *  value of an expression. For details, refer to BT_assert.h for the
 *  platform.
 *
 *  The BT_assert() macro is used for debugging during development, and
 *  can be excluded in builds for the final product.
 *
 *  Dependency: None.
 */
#define BT_DEBUG_ASSERT

/*
 *  BT_STATUS
 *
 *  This flag enables EtherMind Status Flag APIs for reporting various
 *  Status, as described in BT_status_api.h. The EtherMind Status APIs
 *  are designed and to be used for debugging purposes during development
 *  and can be excluded in builds for final products.
 *
 *  Dependency: None.
 */
#define BT_STATUS

/*
 *  BT_DISABLE_MUTEX
 *
 *  This flag disables the use of mutex and conditional variables from all
 *  stack modules (protocols and profiles).
 *  This should be defined in EtherMind builds where synchronization
 *  primitives are not required (such as, single task build of EtherMind).
 *
 *  Dependency: None.
 */
/* #define BT_DISABLE_MUTEX */

/*
 *  BT_HAVE_STATIC_DECL
 *
 *  This flag enables provision for declaring functions and/or globals in
 *  a file as 'static'.
 */
#define BT_HAVE_STATIC_DECL

/*
 *  BT_HAVE_CONST_DECL
 *
 *  This flag enables provision for declaring globals in a file as 'const'.
 */
#define BT_HAVE_CONST_DECL

/*
 *  BT_HAVE_REENTRANT_DECL
 *
 *  This flag enables provision for declaring reentrant functions.
 *  At present used for callback function pointers for 8051 build.
 */
/* #define BT_HAVE_REENTRANT_DECL */

/*
 *  VAR_ARG_IN_MACRO_NOT_SUPPORTED
 *
 *  This flag should be enabled if the compiler tool-chain does not support
 *  variable argument in macro.
 */
/* #define VAR_ARG_IN_MACRO_NOT_SUPPORTED */

/*
 *  BT_STORAGE
 *
 *  This flag enables support of the Storage Module
 *
 *  Dependency: BT_LE
 */
/* #define BT_STORAGE */

/*
 *  STORAGE_PERSISTENT_SUPPORT
 *
 *  This flag enables support of persistent memory storage
 *
 *  Dependency: BT_STORAGE
 */
/* #define STORAGE_PERSISTENT_SUPPORT */
#if (defined STORAGE_PERSISTENT_SUPPORT && !defined BT_STORAGE)
#error "Dependency Error for STORAGE_PERSISTENT_SUPPORT"
#endif /* (defined STORAGE_PERSISTENT_SUPPORT && !defined BT_STORAGE) */

/*
 *  STORAGE_RETENTION_SUPPORT
 *
 *  This flag enables support of retention memory storage
 *
 *  Dependency: BT_STORAGE
 */
/* #define STORAGE_RETENTION_SUPPORT */
#if (defined STORAGE_RETENTION_SUPPORT && !defined BT_STORAGE)
#error "Dependency Error for STORAGE_RETENTION_SUPPORT"
#endif /* (defined STORAGE_RETENTION_SUPPORT && !defined BT_STORAGE) */

/*
 *  STORAGE_HAVE_EVENT_MASK
 *
 *  This flag enables support for masking specific Storage Write
 *  Operations.
 *  This is useful to ensure storage write is done only during shutdown or
 *  during pairing/link key update etc.
 *  This can also be changed runtime, to avoid storage write during audio streaming etc.
 *
 *  Dependency: BT_STORAGE
 */
/* #define STORAGE_HAVE_EVENT_MASK */
#if (defined STORAGE_HAVE_EVENT_MASK && !defined BT_STORAGE)
#error "Dependency Error for STORAGE_HAVE_EVENT_MASK"
#endif /* (defined STORAGE_HAVE_EVENT_MASK && !defined BT_STORAGE) */

/*
 *  STORAGE_CHECK_CONSISTENCY_ON_RESTORE
 *
 *  This flag enables support for checking consistency of data structures of various
 *  modules, which are restored following storage load operation.
 *  For example, if device queue has some stray elements (due to inconsistent power cycle
 *  of the system etc.), which are not present in the security databases, those can be cleaned.
 *
 *  Dependency: BT_STORAGE
 */
/* #define STORAGE_CHECK_CONSISTENCY_ON_RESTORE */
#if (defined STORAGE_CHECK_CONSISTENCY_ON_RESTORE && !defined BT_STORAGE)
#error "Dependency Error for STORAGE_CHECK_CONSISTENCY_ON_RESTORE"
#endif /* (defined STORAGE_CHECK_CONSISTENCY_ON_RESTORE && !defined BT_STORAGE) */

/*
 *  BT_SNOOP
 *
 *  This flag enables logging HCI traffic in btsnoop format.
 */
/* #define BT_SNOOP */

/*
 *  BT_RACP
 *
 *  Use this feature flag for GATT based profiles/services which requires
 *  Record Access Control Point.
 *
 *  Dependency: None.
 */
#define BT_RACP


/* ----------------------------------------------------------------------- */
/* ==== Generic Access Profile Specific Flags ============================ */
/* ----------------------------------------------------------------------- */
/*
 *  GAP_CENTRAL
 *
 *  This flag enables GAP Central Role.
 *
 *  A device that supports the Central role initiates the establishment of
 *  a physical connection. A device operating in the Central role will be
 *  in the Master role in the Link Layer Connection State.
 *
 *  Dependency: Only one of the roles Central, Peripheral, Broadcaster and
 *  Observer shall be defined.
 */
/* #define GAP_CENTRAL */

/*
 *  GAP_PERIPHERAL
 *
 *  This flag enables GAP Peripheral Role.
 *
 *  Any device that accepts the establishment of an LE physical link using
 *  any of the connection establishment procedures is referred to as being
 *  in the Peripheral role. A device operating in the Peripheral role will
 *  be in the Slave role in the Link Layer Connection State.
 *
 *  Dependency: Only one of the roles Central, Peripheral, Broadcaster and
 *  Observer shall be defined.
 */
/* #define GAP_PERIPHERAL */

/*
 *  GAP_BROADCASTER
 *
 *  This flag enables GAP Broadcaster Role.
 *
 *  A device operating in the Broadcaster role is a device that sends
 *  non-connectable advertising events.
 *
 *  Dependency: Only one of the roles Central, Peripheral, Broadcaster and
 *  Observer shall be defined.
 */
/* #define GAP_BROADCASTER */

/*
 *  GAP_OBSERVER
 *
 *  This flag enables GAP Observer Role.
 *
 *  A device operating in the Observer role is a device that receives
 *  advertising events.
 *
 *  Dependency: Only one of the roles Central, Peripheral, Broadcaster and
 *  Observer shall be defined.
 */
/* #define GAP_OBSERVER */


/* ----------------------------------------------------------------------- */
/* ==== Module Inclusion Flags for EtherMind Protocols =================== */
/* ----------------------------------------------------------------------- */
/*
 *  ATT, SMP
 *
 *  These flags enable initialization and shutdown of various Stack protocol
 *  modules, above L2CAP, and should be defined if any one of them is included
 *  in the build.
 *
 *  Dependency: None.
 */

#ifdef BT_LE
/* ----------------------------------------------------------------------- */
/* ==== LE Security Manager Protocol Specific Flags ====================== */
/* ----------------------------------------------------------------------- */
/*
 * SMP
 *
 * SMP module flag.
 *
 * Dependency: GAP_CENTRAL or GAP_PERIPHERAL shall be defined.
 */
#if ((defined GAP_CENTRAL) || (defined GAP_PERIPHERAL))
#define SMP
#endif /* ((defined GAP_CENTRAL) || (defined GAP_PERIPHERAL)) */

#ifdef SMP
/*
 *  SMP_STORAGE
 *
 *  This flag enables Security Manager Protocol(SMP) module to store
 *  device bonding information and configuration data (such as, Long Term Key,
 *  Identity Resolving Key, Signature Key, default configuration etc.) in
 *  persistent storage device. SMP writes on to the persistent storage when
 *  on any update on the device database or configuration.
 *
 *  On next initialization, during BT_bluetooth_on(), SMP reads the stored
 *  configuration data from the persistent storage media.
 *
 *  If BT_STORAGE is enabled, then SMP_STORAGE also has to be enabled in order
 *  for SMP to record bonded device information persistently.
 *
 *  Dependency: BT_STORAGE
 */
#ifdef BT_STORAGE
#define SMP_STORAGE
#endif /* BT_STORAGE */

/*
 *  SMP_MASTER
 *
 *  This flag enables SMP module master role functionalities.
 *
 *  Dependency: BT_LE && SMP && GAP_CENTRAL
 */
#ifdef GAP_CENTRAL
#define SMP_MASTER
#endif /* GAP_CENTRAL */

/*
 *  SMP_SLAVE
 *
 *  This flag enables SMP module slave role functionalities.
 *
 *  Dependency: BT_LE && SMP && GAP_PERIPHERAL
 */
#ifdef GAP_PERIPHERAL
#define SMP_SLAVE
#endif /* GAP_PERIPHERAL */

/*
 *  SMP_HAVE_ENCRYPT_API
 *
 *  This flag enables BT_smp_encrypt API that can be used by application
 *
 *  Dependency: BT_LE && SMP
 */
/* #define SMP_HAVE_ENCRYPT_API */

/*
 *  SMP_IGNORE_REAUTHENTICATION
 *
 *  This flag ignores authentication request on an encrypted link
 *
 *  Dependency: BT_LE && SMP
 */
/* #define SMP_IGNORE_REAUTHENTICATION */

/*
 *  SMP_NO_STORED_KEYS
 *
 *  This flag disables the Authentication API to check for bonded/authentication
 *  status of peer device. Also peer's shared key information is not stored in
 *  persistent storage. This API instead enables BT_smp_encrypt API that
 *  can be used by application
 *
 *  Dependency: BT_LE && SMP
 */
/* #define SMP_NO_STORED_KEYS */

#ifdef SMP_NO_STORED_KEYS
#define SMP_HAVE_ENCRYPT_API
#endif /* SMP_NO_STORED_KEYS */

/*
 *  SMP_NO_PARAM_CHECK
 *
 *  This flag disables the check in SMP APIs to verify the parameter memory
 *  and if the parameter values are within specification defined valid range
 *
 *  When application using SMP APIs are validated and
 *  it is ensured that none of the API call path is using
 *  parameters with invalid value, this flag can be enabled
 *  to reduce the code size.
 *
 *  Dependency: None.
 */
/* #define SMP_NO_PARAM_CHECK */

/*
 *  SMP_DST_ENCRYPT_INFO
 *
 *  This flag enables SMP module encryption information distribution
 *  functionalities.
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_DST_ENCRYPTION_INFO

/*
 *  SMP_DST_IRK
 *
 *  This flag enables SMP module Identity information distribution
 *  functionalities.
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_DST_IDENTITY_INFO

/*
 *  SMP_DST_SRK
 *
 *  This flag enables SMP module SRK distribution functionalities.
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_DST_SRK

/*
 *  SMP_HNDL_ENCRYPT_INFO
 *
 *  This flag enables SMP module to handle remote Signature Key information.
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_HNDL_ENCRYPTION_INFO

/*
 *  SMP_HNDL_IDENTITY
 *
 *  This flag enables SMP module to handle remote Identity Information.
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_HNDL_IDENTITY_INFO

/*
 *  SMP_HNDL_SRK
 *
 *  This flag enables SMP module to handle remote SRK and aslo enable to the
 *  signature related APIs to send signed data.
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_HNDL_SRK

/*
 *  SMP_SUPPORT_UTIL_APIS
 *
 *  This flag enables SMP Utility APIs.
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_SUPPORT_UTIL_APIS


/*
 *  SMP_CHECK_ENCRYPTION_KEY_DIST_FOR_BONDING
 *
 *  This flag enables verification of received peer Encryption Information
 *  before initiating encryption procedure for bonded device
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_CHECK_ENCRYPTION_KEY_DIST_FOR_BONDING

/*
 *  SMP_FORCE_CLEAR_ENTITY
 *
 *  This flag enables SMP to free the entity and its device queue reference on disconnection
 *  of a device irrespective of bonded or non-bonded
 *
 *  Dependency: BT_LE && SMP
 */
/* #define SMP_FORCE_CLEAR_ENTITY */

/*
 *  SMP_CLEAR_UNBONDED_ENTITY
 *
 *  This flag enables SMP to free the entity and its device queue reference on disconnection
 *  of an unbonded device. This flag does not have any effect if the
 *  SMP_FORCE_CLEAR_ENTITY is defined.
 *
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_CLEAR_UNBONDED_ENTITY

/*
 *  SMP_HAVE_OOB_SUPPORT
 *
 *  This flag enables SMP to support OOB data for authentication
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_HAVE_OOB_SUPPORT

/*
 *  SMP_DATA_SIGNING
 *
 *  This flag enables SMP data signing feature
 *
 *  Dependency: BT_LE && SMP
 */
#define SMP_DATA_SIGNING

/*
 *  SMP_LESC
 *
 *  This flag governs the Bluetooth 4.2 SMP Secure connections Feature for Host
 *
 *  Dependency: BT_LE_4_2 && SMP
 */
#define SMP_LESC

/*
 *  SMP_HAVE_TBX_PL_ENC
 *
 *  This flag enables the use of platform specific Encrypt function
 *
 *  Dependency:
 */
/* #define SMP_HAVE_TBX_PL_ENC */

/*
 *  SMP_HAVE_TBX_PL_ECDH
 *
 *  This flag enables the use of platform specific
 *  Elliptic Curve Diffe-Hellman Functions.
 *
 *  Dependency:
 */
#ifdef SMP_LESC
/* #define SMP_HAVE_TBX_PL_ECDH */
#endif /* SMP_LESC */

/*
 *  SMP_HAVE_TBX_CMD_WT_BH
 *
 *  This flag enables the use of SMP toolbox specific WriteTask BottomHalf function
 *
 *  Dependency: BT_LE && SMP && SMP_HAVE_TBX_PL_ENC
 */
#if (defined SMP_HAVE_TBX_PL_ENC) || (defined SMP_HAVE_TBX_PL_ECDH)
#define SMP_HAVE_TBX_CMD_WT_BH
#endif /* SMP_HAVE_TBX_PL_ENC || SMP_HAVE_TBX_PL_ECDH */

/*
 *  SMP_TBX_TEST_LESC_FUNCTIONS
 *
 *  This flag enables to test the SMP LESC tool box functions
 *
 *  Dependency: SMP_LESC
 */
#ifdef SMP_LESC
/* #define SMP_TBX_TEST_LESC_FUNCTIONS */
#endif /* SMP_LESC */

/*
 *  AES_CMAC_UTIL
 *
 *  This flag enables the AES-CMAC utility module
 *
 *  Dependency: BT_LE && SMP && (SMP_DATA_SIGNING || SMP_LESC)
 */
#if (defined SMP_DATA_SIGNING) || (defined SMP_LESC)
#define AES_CMAC_UTIL
#endif /* SMP_DATA_SIGNING || SMP_LESC */

/*
 *  SMP_AUTH_REQ_RFU_TESTING
 *
 *  This flag enables tesing SMP with RFU bits of Auth Req paramteres set.
 *
 *  Dependency: BT_LE && SMP
 */
/* #define SMP_AUTH_REQ_RFU_TESTING */

/*
 *  SMP_LESC_TESTER_BEHAVIOUR
 *
 *  This flag enables Lower Tester Flexibility in SMP module.
 *
 *  Dependency: BT_LE && SMP
 */
/* #define SMP_LESC_TESTER_BEHAVIOUR */


/*
 *  HOST_RESOLVE_PVT_ADDR
 *
 *  This flag should be enabled if support for Private Address Resolution
 *  of a peer connected device is required in the host stack
 *
 *  Dependency: BT_LE && SMP
 */
#define HOST_RESOLVE_PVT_ADDR

/*
 *  HOST_PURGE_ON_DB_FULL
 *
 *  This flag should be enabled if the stack requires to purge
 *  the internal database when it becomes full for connections
 *
 *  Dependency: BT_LE && SMP
 */
#define HOST_PURGE_ON_DB_FULL

/*
 *  SMP_HANDLE_PEER_KEY_XCHG_SEQUENCE_VIOLATION
 *
 *  This flag enables the SMP module to send pairing failure when peer
 *  device violates the Key Exchange sequence defined in the BT SMP
 *  specification.
 *
 *  Dependency: BT_LE && SMP
 */
/* #define SMP_HANDLE_PEER_KEY_XCHG_SEQUENCE_VIOLATION */

#endif /* SMP */

#endif /* BT_LE */


/* ----------------------------------------------------------------------- */
/* ==== Device Queue Module Specific Flags =============================== */
/* ----------------------------------------------------------------------- */
/*
 *  DQ_NO_NULL_PARAM_CHECK
 *
 *  Disable null check for Device Queue APIs.
 *
 *  Use of this feature flag is recommended for final release/production
 *  versions of applications for optimized code size.
 *
 *  Dependency: None.
 */
/* #define DQ_NO_NULL_PARAM_CHECK */


/* ----------------------------------------------------------------------- */
/* ==== Write Task Module Specific Flags ================================= */
/* ----------------------------------------------------------------------- */
/*
 *  WT_SERVICE_Q_ON_PRIORITY
 *
 *  If this feature flag is defined, Write Task after servicing
 *  each enqueued element from a specific priority queue, will check
 *  if there is any enqueued element in higher priority queue to be processed.
 *
 *  Default behaviour is to process all the enqueued elements in a specific
 *  queue, before processing elements from another priority queue.
 *
 *  Dependency: None.
 */
/* #define WT_SERVICE_Q_ON_PRIORITY */


/* ----------------------------------------------------------------------- */
/* ==== HCI Module Specific Flags ======================================== */
/* ----------------------------------------------------------------------- */
/*
 *  HCI_VENDOR_SPECIFIC_COMMANDS
 *
 *  This flag enables HCI module to provide applications the ability to send
 *  vendor-specific HCI commands.
 *
 *  Specifically, the BT_hci_vendor_specific_command() API is available only
 *  if this flag is defined.
 *
 *  Dependency: None.
 */
#define HCI_VENDOR_SPECIFIC_COMMANDS

/*
 *  HCI_HOST_CONTROLLER_FLOW_ON
 *
 *  This flag enables HCI module to switch on HCI Host Controller to Host Flow
 *  Control feature offered by the Bluetooth device.
 *
 *  If switched on, this feature will enable HCI to send HCI Host Number of
 *  Completed Packets Command to the Bluetooth Hardware on reception of every
 *  ACL packet.
 *
 *  Dependency: None.
 *
 *  NOTE: This feature is not fully supported.
 */
/* #define HCI_HOST_CONTROLLER_FLOW_ON */

/*
 *  HCI_TESTING_COMMANDS
 *
 *  This flag enables HCI Testing Commands APIs:
 *  - BT_hci_read_loopback_mode()
 *  - BT_hci_write_loopback_mode()
 *  - BT_hci_enable_device_under_test_mode()
 *
 *  Dependency: None.
 */
#define HCI_TESTING_COMMANDS

/*
 *  HCI_HAVE_INIT_COMMAND_MASK
 *
 *  This flag enables BT_hci_set_init_command_mask() API.
 *
 *  Using the BT_hci_set_init_command_mask() API, application
 *  can control the HCI commands sent during BT_bluetooth_on()
 *
 *  Dependency: None.
 */
#define HCI_HAVE_INIT_COMMAND_MASK

/*
 *  HCI_NO_NULL_PARAM_CHECK
 *
 *  This flag disables 'null' parameter check in HCI APIs
 *
 *  When application using HCI APIs are validated and
 *  it is ensured that none of the API call path is using
 *  an unexpected 'null' parameter, this flag can be enabled
 *  to reduce the code size.
 *
 *  Dependency: None.
 */
/* #define HCI_NO_NULL_PARAM_CHECK */

/*
 *  HCI_NO_RANGE_CHECK
 *
 *  This flag disables the check in HCI APIs to verify if the parameter
 *  values are within specification defined valid range
 *
 *  When application using HCI APIs are validated and
 *  it is ensured that none of the API call path is using
 *  parameters with invalid value, this flag can be enabled
 *  to reduce the code size.
 *
 *  Otherwise also this flag can be enabled, if the overall system can
 *  depend on the HCI parameter validation performed
 *  by the underlying HCI controller.
 *
 *  Dependency: None.
 */
/* #define HCI_NO_RANGE_CHECK */

/*
 * Feature flags for HCI Commands defined for BLE Single Mode
 *
 * Note:
 * ## Commands primarily required for Master(M) and Slave(S) are marked
 * ## Remaining commands are applicable for both roles
 */
/* Reset Command */
#define HCI_RESET_SUPPORT

/* Read Local Version Information Command */
#define HCI_READ_LOCAL_VERSION_INFORMATION_SUPPORT

/* Read Local Supported Features Command */
#define HCI_READ_LOCAL_SUPPORTED_FEATURES_SUPPORT

/* Read BD_ADDR Command */
#define HCI_READ_BD_ADDR_SUPPORT

/* Set Event Mask Command */
#define HCI_SET_EVENT_MASK_SUPPORT

/* Read Buffer Size Command */
#define HCI_READ_BUFFER_SIZE_SUPPORT

/* Host Buffer Size Command */
#define HCI_HOST_BUFFER_SIZE_SUPPORT

/* Set Controller To Host Flow Control Command */
#define HCI_SET_HOST_CONTROLLER_TO_HOST_FLOW_CONTROL_SUPPORT

/* Host Number Of Completed Packets Command */
/* Read Local Supported Commands Command */
#define HCI_READ_LOCAL_SUPPORTED_COMMANDS_SUPPORT

/* LE Read Buffer Size Command */
#define HCI_LE_READ_BUFFER_SIZE_SUPPORT

/* LE Read Local Supported Features Command */
#define HCI_LE_READ_LOCAL_SUPPORTED_FEATURES_SUPPORT

/* LE Read Supported States Command */
#define HCI_LE_READ_SUPPORTED_STATES_SUPPORT

/* LE Set Event Mask Command */
#define HCI_LE_SET_EVENT_MASK_SUPPORT

/* LE Test End Command */
#define HCI_LE_TEST_END_SUPPORT

/* LE Set Random Address Command */
#define HCI_LE_SET_RANDOM_ADDRESS_SUPPORT

/* LE Read Advertising Channel TX Power Command */
#define HCI_LE_READ_ADVERTISING_CHANNEL_TX_POWER_SUPPORT

/* Read Transmit Power Level Command */
#define HCI_READ_TRANSMIT_POWER_LEVEL_SUPPORT

/* LE Transmitter Test Command */
#define HCI_LE_TRANSMITTER_TEST_COMMAND_SUPPORT

/* LE Receiver Test Command */
#define HCI_LE_RECEIVER_TEST_COMMAND_SUPPORT

/* LE Encrypt Command */
#define HCI_LE_ENCRYPT_SUPPORT

#if ((defined GAP_PERIPHERAL) || (defined GAP_BROADCASTER))
/* LE Set Advertise Enable Command (S) */
#define HCI_LE_SET_ADVERTISING_ENABLE_SUPPORT

/* LE Set Advertising Data Command (S) */
#define HCI_LE_SET_ADVERTISING_DATA_SUPPORT

/* LE Set Advertising Parameters Command (S) */
#define HCI_LE_SET_ADVERTISING_PARAMETERS_SUPPORT

/* LE Set Scan Response Data Command (S) */
#define HCI_LE_SET_SCAN_RESPONSE_DATA_SUPPORT

#endif /* ((defined GAP_PERIPHERAL) || (defined GAP_BROADCASTER)) */

#if ((defined GAP_CENTRAL) || (defined GAP_OBSERVER))
/* LE Set Scan Enable Command (M) */
#define HCI_LE_SET_SCAN_ENABLE_SUPPORT

/* LE Set Scan Parameters Command (M) */
#define HCI_LE_SET_SCAN_PARAMETERS_SUPPORT
#endif /* ((defined GAP_CENTRAL) || (defined GAP_OBSERVER)) */

#ifdef GAP_CENTRAL
/* LE Create Connection Cancel Command (M) */
#define HCI_LE_CREATE_CONNECTION_CANCEL_SUPPORT

/* LE Create Connection Command (M) */
#define HCI_LE_CREATE_CONNECTION_SUPPORT

/* LE Set Host Channel Classification Command (M) */
#define HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION_SUPPORT

/* LE Start Encryption Command (M) */
#define HCI_LE_START_ENCRYPTION_SUPPORT

#endif /* GAP_CENTRAL */

#if ((defined GAP_CENTRAL) || (defined GAP_PERIPHERAL))
/* Disconnect Command */
#define HCI_DISCONNECT_SUPPORT

/* Read Remote Version Information Command */
#define HCI_READ_REMOTE_VERSION_INFORMATION_SUPPORT

/* Read RSSI Command */
#define HCI_READ_RSSI_SUPPORT

/* LE Add Device To White List Command */
#define HCI_LE_ADD_DEVICE_TO_WHITE_LIST_SUPPORT

/* LE Clear White List Command */
#define HCI_LE_CLEAR_WHITE_LIST_SUPPORT

/* LE Read White List Size Command */
#define HCI_LE_READ_WHITE_LIST_SIZE_SUPPORT

/* LE Remove Device From White List Command */
#define HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST_SUPPORT

/* LE Read Remote Used Features Command */
#define HCI_LE_READ_REMOTE_USED_FEATURES_SUPPORT

/* LE Long Term Key Requested Reply Command */
#define HCI_LE_LONG_TERM_KEY_REQUESTED_REPLY_SUPPORT

/* LE Long Term Key Requested Negative Reply Command */
#define HCI_LE_LONG_TERM_KEY_REQUESTED_NEGATIVE_REPLY_SUPPORT

/* LE Rand Command */
#define HCI_LE_RAND_SUPPORT

/* LE Read Channel Map Command */
#define HCI_LE_READ_CHANNEL_MAP_SUPPORT

#endif /* ((defined GAP_CENTRAL) || (defined GAP_PERIPHERAL)) */

#if ((defined GAP_CENTRAL) || (defined BT_4_1))
/* LE Connection Update Command (M) */
#define HCI_LE_CONNECTION_UPDATE_SUPPORT
#endif /* ((defined GAP_CENTRAL) || (defined BT_4_1)) */

/* Support API to get device Role */
#define HCI_SUPPORT_GET_DEVICE_ROLE

#ifdef BT_4_1
/*
 *  HCI_LOW_DUTY_CYCLE_DIRECTED_ADVERTISING
 *
 *  This flag enables configuration of Low Duty Cycle Directed Advertising
 *  mode through BT_hci_le_set_advertising_parameters() API.
 *
 *  Dependency: BT_4_1 must be defined.
 */
#define HCI_LOW_DUTY_CYCLE_DIRECTED_ADVERTISING

/*
 * HCI_LE_PING_SUPPORT
 *
 * This flag enables LE Ping Support.
 * Dependency: BT_4_1 must be defined.
 */
#define HCI_LE_PING_SUPPORT

#ifdef HCI_LE_PING_SUPPORT
#define HCI_READ_AUTHENTICATED_PAYLOAD_TIMEOUT_SUPPORT
#define HCI_WRITE_AUTHENTICATED_PAYLOAD_TIMEOUT_SUPPORT
#endif /* HCI_LE_PING_SUPPORT */

/* TBD: Add Comment */
#define HCI_LL_TOPOLOGY_CONN_UPDATE_SUPPORT

/* TBD: Add Comment */
#define HCI_SET_EVENT_MASK_PAGE_2_SUPPORT

#endif /* BT_4_1 */

#ifdef BT_4_2

/*
 * HCI_LE_DATA_LENGTH_EXT_SUPPORT
 *
 * This flag enables LE Data Length Extension Support.
 * Dependency: BT_4_2 must be defined.
 */
#define HCI_LE_DATA_LENGTH_EXT_SUPPORT

/*
 * HCI_LE_SECURE_CONNECTIONS_SUPPORT
 *
 * This flag enables LE Secure Connections Support.
 * Dependency: BT_4_2 must be defined.
 */
#define HCI_LE_SECURE_CONNECTIONS_SUPPORT

/*
 * HCI_LE_PRIVACY_1_2_SUPPORT
 *
 * This flag enables LE Privacy 1.2 Support.
 * Dependency: BT_4_2 must be defined.
 */
#define HCI_LE_PRIVACY_1_2_SUPPORT

#ifdef HCI_LE_DATA_LENGTH_EXT_SUPPORT
/* LE Set Data Length command support */
#define HCI_LE_SET_DATA_LENGTH_SUPPORT

/* LE Read suggested Default Data Length */
#define HCI_LE_READ_SUGGESTED_DEFAULT_DATA_LEN_SUPPORT

/* LE Write Suggested Degault Data Length */
#define HCI_LE_WRITE_SUGGESTED_DEFAULT_DATA_LEN_SUPPORT

/* LE Read Maximum Data Length */
#define HCI_LE_READ_MAXIMUM_DATA_LEN_SUPPORT

#endif /* HCI_LE_DATA_LENGTH_EXT_SUPPORT */

#ifdef HCI_LE_SECURE_CONNECTIONS_SUPPORT
/* LE Read Local P256 Public Key */
#define HCI_LE_READ_LOCAL_P_256_PUBLIC_KEY_SUPPORT

/* LE Generate DH Key */
#define HCI_LE_GENERATE_DHKEY_SUPPORT
#endif /* HCI_LE_SECURE_CONNECTIONS_SUPPORT */

#ifdef HCI_LE_PRIVACY_1_2_SUPPORT
/* LE Add Device to Resolving List command */
#define HCI_LE_ADD_DEV_TO_RESOLVING_LIST_SUPPORT

/* LE Remove Device From Resolving List */
#define HCI_LE_REMOVE_DEV_FROM_RESOLVING_LIST_SUPPORT

/* LE Clear Resolving List */
#define HCI_LE_CLR_RESOLVING_LIST_SUPPORT

/* LE Read Resolving List Size */
#define HCI_LE_READ_RESOLVING_LIST_SIZE_SUPPORT

/* LE Read Peer Resolvable Address */
#define HCI_LE_READ_PEER_RESOLVABLE_ADDR_SUPPORT

/* LE Read Local Resolvable Address */
#define HCI_LE_READ_LOCAL_RESOLVABLE_ADDR_SUPPORT

/* LE Set Address Resolution Enable */
#define HCI_LE_SET_ADDR_RESOLUTION_ENABLE_SUPPORT

/* LE Set Resolvable Private Address Timeout */
#define HCI_LE_SET_RESOLVABLE_PRIVATE_ADDR_TIMEOUT_SUPPORT
#endif /* HCI_LE_PRIVACY_1_2_SUPPORT */

#endif /* BT_4_2 */

#ifdef BT_5_0
/*
 * HCI_LE_2MBPS_PHY_SUPPORT
 *
 * This flag enables LE 2Mbps PHY Support.
 * Dependency: BT_5_0 must be defined.
 */
#define HCI_LE_2MBPS_PHY_SUPPORT

#ifdef HCI_LE_2MBPS_PHY_SUPPORT

/* LE Read PHY */
#define HCI_LE_READ_PHY_SUPPORT

/* LE Set Host Preferred PHY */
#define HCI_LE_SET_DEFAULT_PHY_SUPPORT

/* LE Set PHY */
#define HCI_LE_SET_PHY_SUPPORT

/* LE Enhanced Receiver Test */
#define HCI_LE_ENHANCED_RECEIVER_TEST_SUPPORT

/* LE Enhanced Transmitter Test */
#define HCI_LE_ENHANCED_TRANSMITTER_TEST_SUPPORT

#endif /* HCI_LE_2MBPS_PHY_SUPPORT */

/*
 *  HCI_LE_ADVERTISING_EXTENSION_SUPPORT
 *
 *  This flag enables HCI LE Advertising Extension support
 *  Dependency:
 */
#define HCI_LE_ADVERTISING_EXTENSION_SUPPORT

#ifdef HCI_LE_ADVERTISING_EXTENSION_SUPPORT
/* LE Set Extended Advertising Parameters */
#define HCI_LE_SET_EXTENDED_ADV_PARAMS_SUPPORT

/* LE Set Periodic Advertising Parameters*/
#define HCI_LE_SET_PERIODIC_ADV_PARAMS_SUPPORT

/* LE Set Extended Advertising Data */
#define HCI_LE_SET_EXTENDED_ADVERTISING_DATA_SUPPORT

/* LE Set Extended Scan Response Data */
#define HCI_LE_SET_EXTENDED_SCAN_RESPONSE_DATA_SUPPORT

/* LE Set Extended Advertise Enable */
#define HCI_LE_SET_EXTENDED_ADVERTISE_ENABLE_SUPPORT

/* LE Read Maximum Advertising Data Length */
#define HCI_LE_READ_MAX_ADV_DATA_LENGTH_SUPPORT

/* LE Read Num Supported Advertising Sets */
#define HCI_LE_READ_NUM_SUPPORTED_ADV_SETS_SUPPORT

/* LE Read Transmit Power */
#define HCI_LE_READ_TRANSMIT_POWER_SUPPORT

/* LE Remove Advertising Set */
#define HCI_LE_REMOVE_ADVERTISING_SET_SUPPORT

/* LE Clear Advertising Sets */
#define HCI_LE_CLEAR_ADVERTISING_SETS_SUPPORT

/* LE Set Extended Scan Parameters */
#define HCI_LE_SET_EXTENDED_SCAN_PARAMETERS_SUPPORT

/* LE Set Extended Scan Enable */
#define HCI_LE_SET_EXTENDED_SCAN_ENABLE_SUPPORT

/* LE Periodic Advertising Receive */
#define HCI_LE_PERIODIC_ADVERTISING_CREATE_SYNC_SUPPORT

/* LE Add Device To Periodic Advertiser List */
#define HCI_LE_ADD_DEV_TO_PERIODIC_ADVSER_LIST_SUPPORT

/* LE Remove Device From Periodic Advertiser List */
#define HCI_LE_RM_DEV_FROM_PERIODIC_ADVSER_LIST_SUPPORT

/* LE Clear Periodic Advertiser List */
#define HCI_LE_CLEAR_PERIODIC_ADVSER_LIST_SUPPORT

/* LE Read Periodic Advertiser List Size */
#define HCI_LE_READ_PERIODIC_ADVSER_LIST_SIZE_SUPPORT

/* LE Set Scan Request Notification Enable */
#define HCI_LE_SET_SCAN_REQUEST_NTF_ENABLE_SUPPORT

/* LE Read RF Path Compensation */
#define HCI_LE_READ_RF_PATH_COMPENSATION_SUPPORT

/* LE Write RF Path Compensation */
#define HCI_LE_WRITE_RF_PATH_COMPENSATION_SUPPORT

/* LE Set Advertising Random Address */
#define HCI_LE_SET_ADVERTISING_SET_RANDOM_ADDRESS_SUPPORT

/* LE Set Periodic Advertising Data */
#define HCI_LE_SET_PERIODIC_ADVERTISING_DATA_SUPPORT

/* LE Set Periodic Advertising Enable */
#define HCI_LE_SET_PERIODIC_ADVERTISING_ENABLE_SUPPORT

/* LE Periodic Advertising Receive Cancel */
#define HCI_LE_PERIODIC_ADV_CREATE_SYNC_CANCEL_SUPPORT

/* LE Periodic Advertising Receive End */
#define HCI_LE_PERIODIC_ADVERTISING_TERMINATE_SYNC_SUPPORT

/* LE Extended Create Connection */
#define HCI_LE_EXTENDED_CREATE_CONNECTION_SUPPORT

#endif /* HCI_LE_ADVERTISING_EXTENSION_SUPPORT */

/* LE Set Privacy Mode */
#define HCI_LE_SET_PRIVACY_MODE_SUPPORT

#endif /* BT_5_0 */


/* ----------------------------------------------------------------------- */
/* ==== L2CAP Module Specific Flags ====================================== */
/* ----------------------------------------------------------------------- */
/*
 * L2CAP
 *
 * L2CAP module flag.
 *
 * Dependency: GAP_CENTRAL or GAP_PERIPHERAL shall be defined.
 */
#if ((defined GAP_CENTRAL) || (defined GAP_PERIPHERAL))
#define L2CAP
#endif /* ((defined GAP_CENTRAL) || (defined GAP_PERIPHERAL)) */

#ifdef L2CAP

/*
 *  L2CAP_DATA_TX_NO_WRITE_TASK
 *
 *  If this flag is defined then L2CAP avoids using Write Task for
 *  transmission of its PDUs on explicitly created Write Task's context.
 *  The data transmission is performed as and when the data are queued
 *  in its transmission queue.
 *
 *  On the other hand, if this flag is not defined, L2CAP queues a Bottom Half
 *  function (BH) to Write Task, to trigger the data transmission, as and when
 *  data are queued in its transmission queue. Actual transmission of data is
 *  done when the Write Task calls the queued BH.
 *
 *  [Refer EtherMind Programmer's Guide, for relevant details on the Write
 *  Task and the concept of Bottom Half function]
 *
 *  Note: This flag is disabled by default. Enable only if the implications of
 *  this are completely understood. With this flag enabled, it is possible that
 *  the HCI Command and L2CAP data packets get interleaved while sending to
 *  the HCI transport layer.
 */
/* #define L2CAP_DATA_TX_NO_WRITE_TASK */

/*
 *  L2CAP_TX_QUEUE_FLOW
 *
 *  This flag enables L2CAP provides additional APIs and processing to
 *  application so that it is possible to know and control buffer occupancy
 *  of the L2CAP Transmission Queue.
 */
#define L2CAP_TX_QUEUE_FLOW

/*
 *  BT_L2CAP_FIXED_CHANNEL_TX_COMPLETE
 *
 *  This flag enables L2CAP Fixed Channel Tx Complete Event
 *  through callback.
 *
 *  Dependency: BT_L2CAP_FIXED_CHANNEL.
 */
#define BT_L2CAP_FIXED_CHANNEL_TX_COMPLETE

/*
 *  L2CAP_LE_SLAVE
 *
 *  This flag enables L2CAP Connection Parameter Update Request Command
 *
 *  Dependency: GAP_PERIPHERAL.
 */
#ifdef GAP_PERIPHERAL
#define L2CAP_LE_SLAVE
#endif /* GAP_PERIPHERAL */

/*
 *  L2CAP_LE_MASTER
 *
 *  This flag enables L2CAP Connection Parameter Update Response
 *
 *  Dependency: GAP_CENTRAL.
 */
#ifdef GAP_CENTRAL
#define L2CAP_LE_MASTER
#endif /* GAP_CENTRAL */

/*
 *  L2CAP_NO_PARAM_CHECK
 *
 *  This flag disables the check in L2CAP APIs to verify the parameter memory
 *  and if the parameter values are within specification defined valid range
 *
 *  When application using L2CAP APIs are validated and
 *  it is ensured that none of the API call path is using
 *  parameters with invalid value, this flag can be enabled
 *  to reduce the code size.
 *
 *  Dependency: None.
 */
/* #define L2CAP_NO_PARAM_CHECK */

/*
 *  L2CAP_SUPPORT_CBFC_MODE
 *
 *  This flag enables the L2CAP Credit Based Flow Control mode defined in
 *  Bluetooth Specification 4.1
 *
 *  Dependency: BT_4_1
 */
#ifdef BT_4_1
#define L2CAP_SUPPORT_CBFC_MODE
#endif /* BT_4_1 */

/*
 *  L2CAP_CBFC_SUPPORT_UL_CREDIT_LWM
 *
 *  For L2CAP CBFC, this flag enables the Upper Layer(UL) to define Low Water Mark
 *  for receive credits in the L2CAP_PSM_CBFC structure, while registering
 *  L2CAP LE_PSM.
 *
 *  In CBFC mode, L2CAP will inform application/UL about receive credits
 *  reached low water mark once credits is less than or equal to
 *  the Low Water Mark defined by the application/UL.
 *
 *  Dependency: L2CAP_SUPPORT_CBFC_MODE
 */
#ifdef L2CAP_SUPPORT_CBFC_MODE
#define L2CAP_CBFC_SUPPORT_UL_CREDIT_LWM
#endif /* L2CAP_SUPPORT_CBFC_MODE */

/*
 *  L2CAP_ALLOC_FOR_CBFC_SDU
 *
 *  For L2CAP CBFC, this flag enables allocation of memory to frame
 *  complete L2CAP SDU, which includes Information Payload, along with the
 *  associated L2CAP Header in the l2ca_channel_data_write() interface.
 *
 *  See the description of 'l2ca_channel_data_write()', for more details.
 *
 *  Dependency: L2CAP_SUPPORT_CBFC_MODE
 */
#ifdef L2CAP_SUPPORT_CBFC_MODE
/* #define L2CAP_ALLOC_FOR_CBFC_SDU */
#endif /* L2CAP_SUPPORT_CBFC_MODE */

#endif /* L2CAP */


/* ----------------------------------------------------------------------- */
/* ==== ATT Protocol Related Flags ======================================= */
/* ----------------------------------------------------------------------- */

/*
 * ATT
 *
 * ATT module flag.
 *
 * Dependency: GAP_CENTRAL or GAP_PERIPHERAL shall be defined.
 */
#if ((defined GAP_CENTRAL) || (defined GAP_PERIPHERAL))
#define ATT
#endif /* ((defined GAP_CENTRAL) || (defined GAP_PERIPHERAL)) */

#ifdef ATT

/*
 *  ATT_CLIENT
 *  This flag is defined to enable support for ATT Client Specific
 *  Functionality, which by specification is optional to support.
 *
 *  Dependency:  None
 *
 */
/* #define ATT_CLIENT */

/*
 *  ATT_NO_PARAM_CHECK
 *
 *  This flag disables the check in ATT APIs to verify the parameter memory
 *  and if the parameter values are within specification defined valid range
 *
 *  When application using ATT APIs are validated and it is ensured that none
 *  of the API call path is using parameters with invalid value, this flag can
 *  be enabled to reduce the code size.
 *
 *  Dependency: None.
 */
/* #define ATT_NO_PARAM_CHECK */

/**
 *  ATT_DEFAULT_CONFIGURATION
 *
 *  Used to define the default configuration of ATT for the application.
 *
 *  Dependency: None.
 */
#define ATT_DEFAULT_CONFIGURATION

/**
 *  ATT_MTU_EXCHANGE_SUPPORT
 *
 *  Enables Support for MTU Exchange Procedure.
 *
 *  Dependency: None.
 */
#define ATT_MTU_EXCHANGE_SUPPORT

/**
 *  ATT_MTU_UPDATE
 *
 *  Enables support to update MTU other than the default MTU size.
 *  Please note that this feature flag is not the same as
 *  ATT_MTU_EXCHANGE_SUPPORT. For servers processing a peer request to update
 *  MTU is mandatory, however, it may respond with default MTU size and would
 *  never have to update the MTU. Hence this flag is only needed for
 *  applications that need an update of the MTU.
 *  Also note that, ATT_MTU_EXCHANGE_SUPPORT should be enabled as well to
 *  ensure exchange of MTU and hence the update can occur.
 *
 *  Dependency:  ATT_MTU_EXCHANGE_SUPPORT
 */
#define ATT_MTU_UPDATE

/**
 *  ATT_FIND_INFO_SUPPORT
 *
 *  Enables Support for Find Information Procedure.
 *
 *  Dependency: None.
 */
#define ATT_FIND_INFO_SUPPORT

/**
 *  ATT_FIND_BY_TYPE_VALUE_SUPPORT
 *
 *  Enables Support for Find by Type Procedure.
 *
 *  Dependency: None.
 */
#define ATT_FIND_BY_TYPE_VALUE_SUPPORT

/**
 *  ATT_READ_BY_TYPE_SUPPORT
 *
 *  Enables Support for Read by Type Procedure.
 *
 *  Dependency: None.
 */
#define ATT_READ_BY_TYPE_SUPPORT

/**
 *  ATT_READ_REQUEST_SUPPORT
 *
 *  Enables Support for Read Procedure.
 *
 *  Dependency: None.
 */
#define ATT_READ_REQUEST_SUPPORT

/**
 *  ATT_READ_BLOB_SUPPORT
 *
 *  Enables Support for Read Blob Procedure.
 *
 *  Dependency: None.
 */
#define ATT_READ_BLOB_SUPPORT

/**
 *  ATT_READ_MULTIPLE_SUPPORT
 *
 *  Enables Support for Read Multiple Procedure.
 *
 *  Dependency: None.
 */
#define ATT_READ_MULTIPLE_SUPPORT

/**
 *  ATT_READ_BY_GROUP_TYPE_SUPPORT
 *
 *  Enables Support for Read by Group Type Procedure.
 *
 *  Dependency: None.
 */
#define ATT_READ_BY_GROUP_TYPE_SUPPORT

/**
 *  ATT_WRITE_REQUEST_SUPPORT
 *
 *  Enables Support for Write Request Procedure.
 *
 *  Dependency: None.
 */
#define ATT_WRITE_REQUEST_SUPPORT

/**
 *  ATT_WRITE_COMMAND_SUPPORT
 *
 *  Enables Support Write Command Procedure.
 *
 *  Dependency: None.
 */
#define ATT_WRITE_COMMAND_SUPPORT

/**
 *  ATT_TX_COMPLETE_EVENT_SUPPORT
 *
 *  Enables support for indicating completion of following procedures,
 *  which are not acknowledged by the peer ATT
 *  - Write Command
 *  - Signed Write Command
 *  - Handle Value Notification
 *
 *  Transmission Complete event is generated once the corresponding
 *  ATT PDU is sent to the peer device (to the Bluetooth Controller).
 *
 *  Dependency: ATT_WRITE_COMMAND_SUPPORT, ATT_SIGNED_WRITE_SUPPORT,
 *              ATT_HNDL_VAL_NOTIFICATION_SUPPORT
 */
#define ATT_TX_COMPLETE_EVENT_SUPPORT

/**
 *  ATT_SIGNED_WRITE_SUPPORT
 *
 *  Enables Support for Signed Write Procedure.
 *
 *  Dependency: None.
 */
#define ATT_SIGNED_WRITE_SUPPORT

/**
 *  ATT_QUEUED_WRITE_SUPPORT
 *
 *  Enables Support for Confirmed Write Procedure = Prepare Write and Execute.
 *
 *  Dependency: None.
 */
#define ATT_QUEUED_WRITE_SUPPORT

/**
 *  ATT_HNDL_VAL_NOTIFICATION_SUPPORT
 *
 *  Enables Support for Handle Value Notifcation Procedure.
 *
 *  Dependency: None.
 */
#define ATT_HNDL_VAL_NOTIFICATION_SUPPORT

/**
 *  ATT_HNDL_VAL_INDICATION_SUPPORT
 *
 *  Enables Support for Handle Value Indication Procedure.
 *
 *  Dependency: None.
 */
#define ATT_HNDL_VAL_INDICATION_SUPPORT

/**
 *  ATT_SUPPORT_128_BIT_UUID
 *
 *  Enables Support for 128-bit UUIDs. This feature flags only when ATT as a
 *  Server has UUIDs that are/cannot be maintained as 16-bit ones or if as a
 *  Client Requests will use 128-bit values instead of 16-bit ones. This feature
 *  flag should not be enabled if Server only wishes to support accepting
 *  requests from Client with 128-bit UUID values.
 *
 *  Dependency: None.
 */
#define ATT_SUPPORT_128_BIT_UUID

/* ----------------------------------------------------------------------- */
/* ==== GATT Protocol Related Flags ====================================== */
/* ----------------------------------------------------------------------- */
/**
 *  GATT_DB_NO_NULL_PARAM_CHECK
 *
 *  Disables Null parameter check for GATT DB APIs.
 *
 *  Use of this feature flag is recommended for final release/production
 *  versions of applications for optimized code size.
 *
 *  Dependency: None.
 */
/* #define GATT_DB_NO_NULL_PARAM_CHECK */

/**
 *  GATT_DB_HAVE_STATIC_CONFIGURATION
 *
 *  Enables the GATT Database to be static, with fixed number of services,
 *  characteristics etc.
 *  If this flag is not defined, the GATT Database can be defined and extended
 *  without requiring to rebuild the core GATT Database Engine.
 *
 *  For the host stack library build, this flag should not be defined.
 *
 *  Dependency: None.
 */
/* #define GATT_DB_HAVE_STATIC_CONFIGURATION */

/**
 *  GATT_DB_SUPPORT_128_BIT_UUID
 *
 *  Enables support for 128 bit UUIDs in GATT Data Base.
 *
 *  Dependency: ATT_SUPPORT_128_BIT_UUID
 */
#define GATT_DB_SUPPORT_128_BIT_UUID

#if ((defined GATT_DB_SUPPORT_128_BIT_UUID) && !(defined ATT_SUPPORT_128_BIT_UUID))
#error Please Enable ATT_SUPPORT_128_BIT_UUID if you need GATT_DB_SUPPORT_128_BIT_UUID
#endif /* GATT_DB_SUPPORT_128_BIT_UUID && ! ATT_SUPPORT_128_BIT_UUID */

/**
 *  GATT_DB_CHAR_DESC_DATA
 *
 *  Enables support for Profile/Higher Layer defined Descriptors.
 *
 *  Dependency: None.
 */
#define GATT_DB_CHAR_DESC_DATA

/**
 *  GATT_DB_VALIDATE_TRANSPORT_ACCESS
 *
 *  Enables transport validation support for GATT Service access.
 *  Typically required for Dual Mode implementation.
 *
 *  Dependency: BT_DUAL_MODE.
 */
/* #define GATT_DB_VALIDATE_TRANSPORT_ACCESS */

#endif /* ATT */


/* ----------------------------------------------------------------------- */
/* ==== Module Profiling Flags =========================================== */
/* ----------------------------------------------------------------------- */
/*
 *  Profiling related definitions for all the layers (protocol and profiles)
 *  shall be defined in this section.
 */

/*
 *  BT_ENABLE_SPY
 *
 *  If defined, this flag enables module profiling.
 *
 *  Dependency: None
 */
/* #define BT_ENABLE_SPY */

#ifdef BT_ENABLE_SPY
#define HT_ENABLE_SPY
#define WT_ENABLE_SPY
#define L2CAP_ENABLE_SPY
#endif /* BT_ENABLE_SPY */


/* ----------------------------------------------------------------------- */
/* ==== Debug Specification Flags ======================================== */
/* ----------------------------------------------------------------------- */
/*
 *  Debug definitions for all the layers (protocol and profiles) should be
 *  defined in this section.
 */

/*
 *  Global Debug Disable Flag : BT_DISABLE_ALL_DEBUG
 *  If this flag is defined, debug logging will be disabled.
 *  If this flag is not defined, then the debug logging will be controlled
 *  by other debug flags as described below.
 */
//#define BT_DISABLE_ALL_DEBUG


/*
 *  Flag to control debug logging at run time.
 *  Debug level can be set using BT_set_debug_level() API.
 *  In addtion there are interfaces to enable/disable module specific flag at run time.
 */
/* #define BT_ENABLE_DISABLE_RUNTIME_DEBUG */


/*
 *  BT_DISABLE_DEBUG_LOG_ON_STARTUP
 *
 *  Flag to control debug logging during Bluetooth ON.
 *
 *  Dependency: BT_ENABLE_DISABLE_RUNTIME_DEBUG
 */
/* #define BT_DISABLE_DEBUG_LOG_ON_STARTUP */
#if ((defined BT_DISABLE_DEBUG_LOG_ON_STARTUP) && !(defined BT_ENABLE_DISABLE_RUNTIME_DEBUG))
#error Please Enable BT_ENABLE_DISABLE_RUNTIME_DEBUG if you need BT_DISABLE_DEBUG_LOG_ON_STARTUP
#endif /* BT_DISABLE_DEBUG_LOG_ON_STARTUP && ! BT_ENABLE_DISABLE_RUNTIME_DEBUG */


/*
 * BT_DEBUG_DONT_LOG_FILE_PATH
 *
 * By default, debug log prints the file name with the path.
 * This flag enables logging only the file name, without the path.
 */
#define BT_DEBUG_DONT_LOG_FILE_PATH

/*
 *  BT_LOG_TIMESTAMP
 *
 *  If defined, this flag enables logging timestamp along
 *  with the debug prints.
 *
 *  Dependency: None
 */
/* #define BT_LOG_TIMESTAMP */

/*
 * By default, the Error Logs of all the layers are enabled.
 * To disable error logging of a module, define <module>_NO_DEBUG flag.
 * Example: Define HCI_NO_DEBUG to disable error logging of HCI layer.
 *
 * By default, the Trace, Information, Data and other Logs
 * of all the layers are disabled.
 * To enable debug logging of a module, define <module>_DEBUG flag.
 * Example: Define HCI_DEBUG to enable debug logging (Trace and Information)
 * of HCI layer.
 */

/* Protocol Modules */
#define COMMON_NO_DEBUG
/* #define COMMON_DEBUG */

/* #define HCI_NO_DEBUG */
#define HCI_DEBUG
#define HCI_DEBUG_DB
/* #define HCI_DDEBUG */

/* #define HT_NO_DEBUG */
/* #define HT_DEBUG */
/* #define HT_INTR_DEBUG */

/* #define DQ_NO_DEBUG */
#define DQ_DEBUG

/* #define L2CAP_NO_DEBUG */
#define L2CAP_DEBUG
#define L2CAP_LP_DEBUG
/* #define L2CAP_TX_Q_DEBUG */

/* #define SMP_NO_DEBUG */
#define SMP_DEBUG

#ifdef SMP_DEBUG
#define SMP_DATA_DEBUG
#endif /* SMP_DEBUG */

/* #define WT_NO_DEBUG */
/* #define WT_DEBUG */

/* #define ATT_NO_DEBUG */
#define ATT_DEBUG

/* #define GATT_DB_NO_DEBUG */
#define GATT_DB_DEBUG

/* #define FSM_NO_DEBUG */
#define FSM_DEBUG

/* Profile Modules */

#endif /* _H_BT_FEATURES_ */

