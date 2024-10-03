
/**
 *  \file BT_assigned_numbers.h
 *
 *  \brief This header file describes various definitions from
 *  the Bluetooth Assigned Numbers Specification.
 */

/*
 *  Copyright (C) 2014. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_ASSIGNED_NUMBERS_
#define _H_BT_ASSIGNED_NUMBERS_

/* --------------------------------- Header File Inclusion */

/* --------------------------------- Global Definitions */

/* =============================================  AD Type 'Flags' Definitions */
/*
 *  Flags (Bit Field):-
 *
 *  07 06 05 04 03 02 01 00
 * _________________________
 * |  |  |  |  |  |  |  |  |
 * |__|__|__|__|__|__|__|__|
 *
 *   0  0  0  0  0  0  0  1 -> LE Limited Discoverable Mode
 *
 *   0  0  0  0  0  0  1  0 -> LE General Discoverable Mode
 *
 *   0  0  0  0  0  1  0  0 -> BR/EDR Not Supported
 *                             [Bit 37 of LMP Feature Mask Definition. (Page 0)]
 *
 *   0  0  0  0  1  0  0  0 -> Simultaneous LE and BR/EDR to Same Device
 *                             Capable (Controller)
 *                             [Bit 49 of LMP Feature Mask Definition. (Page 0)]
 *
 *   0  0  0  1  0  0  0  0 -> Simultaneous LE and BR/EDR to Same Device
 *                             Capable (Host)
 *                             [Bit 66 of LMP Feature Mask Definition. (Page 1)]
 *
 *   X  X  X  X  X  X  X  X -> All other values reserved
 */
#define BT_AD_FLAGS_LE_LIMITED_DISC_MODE          0x01
#define BT_AD_FLAGS_LE_GENERAL_DISC_MODE          0x02
#define BT_AD_FLAGS_BR_EDR_NOT_SUPPORTED          0x04
#define BT_AD_FLAGS_LE_BR_EDR_CONTROLLER          0x08
#define BT_AD_FLAGS_LE_BR_EDR_HOST                0x10


/* =============================================  Link Layer Definitions */
/*
 * The Link Layer Version parameter
 *
 * Parameter Name    Assigned Values
 * VersNr            0 - 5     Reserved
 *                   6         Bluetooth Core Specification 4.0
 *                   7 - 255   Reserved
 */
#define BT_LL_VERSION_4_0                         6


/* ============================  Link Manager Protocol (LMP) Definitions */
/*
 * The Link Manager Version parameter
 *
 * Parameter Name    Assigned Values
 * VersNr            0         Bluetooth Core Specification 1.0b
 *                   1         Bluetooth Core Specification 1.1
 *                   2         Bluetooth Core Specification 1.2
 *                   3         Bluetooth Core Specification 2.0 + EDR
 *                   4         Bluetooth Core Specification 2.1 + EDR
 *                   5         Bluetooth Core Specification 3.0 + HS
 *                   6         Bluetooth Core Specification 4.0
 *                   7 - 255   Reserved
 */
#define BT_LMP_VERSION_1_0_B                       0
#define BT_LMP_VERSION_1_1                         1
#define BT_LMP_VERSION_1_2                         2
#define BT_LMP_VERSION_2_0_EDR                     3
#define BT_LMP_VERSION_2_1_EDR                     4
#define BT_LMP_VERSION_3_0_HS                      5
#define BT_LMP_VERSION_4_0                         6


/* ===============================  HCI Controller Interface Definitions */
/*
 * Parameter Name    Assigned Values
 * HCI_Version       0         Bluetooth Core Specification 1.0b
 *                   1         Bluetooth Core Specification 1.1
 *                   2         Bluetooth Core Specification 1.2
 *                   3         Bluetooth Core Specification 2.0 + EDR
 *                   4         Bluetooth Core Specification 2.1 + EDR
 *                   5         Bluetooth Core Specification 3.0 + HS
 *                   6         Bluetooth Core Specification 4.0
 *                   7 - 255   Reserved
 */
#define BT_HCI_VERSION_1_0_B                       0
#define BT_HCI_VERSION_1_1                         1
#define BT_HCI_VERSION_1_2                         2
#define BT_HCI_VERSION_2_0_EDR                     3
#define BT_HCI_VERSION_2_1_EDR                     4
#define BT_HCI_VERSION_3_0_HS                      5
#define BT_HCI_VERSION_4_0                         6


/* ==========================================  BD_ADDR type Definitions */
/* Types of Bluetooth device Addresses */
/**
 * The public address is the Bluetooth hardware address. The public device
 * address shall be created in accordance with section 9.2 ("48-bit universal
 * LAN MAC addresses") of the IEEE 802-2001 standard
 * (http://standards.ieee.org/getieee802/download/802-2001.pdf)
 * and using a valid Organizationally Unique Identifier (OUI) obtained from
 *  the IEEE Registration Authority
 * (http://standards.ieee.org/regauth/oui/forms/ and sections 9 and 9.1
 * of the IEEE 802-2001 specification).
 */
#define BT_BD_PUBLIC_ADDRESS_TYPE               0

/**
 * The random device address may be of either of two type:
 *  <ol>
 *  <li> Static address:\n
 *    A device may choose to initialize its static address to a new value after
 *    each power cycle. A device shall not change its static address value once
 *    initialized until the device is power cycled.
 *    Note: If the static address of a device is changed then the address
 *    stored in peer devices will not be valid and the ability to reconnect
 *    using the old address will be lost.
 *    A static address is a 48-bit randomly generated address and shall meet
 *    the following requirements:
 *    - The two most significant bits of the static address shall be equal to
 *       1
 *    - All bits of the random part of the static address shall not be equal
 *       to 1
 *    - All bits of the random part of the static address shall not be equal
 *       to 0
 *   </li>
 *
 * <li> Private address:\n
 *    The private address may be of either of the following two sub-types:
 *      <ol type='a'>
 *       <li> Non-resolvable private address:\n
 *       To generate a non-resolvable address or a reconnection address, the
 *       Host shall generate a 48-bit address with the following requirements:
 *        - The two most significant bits of the address shall be equal to 0
 *        - All bits of the random part of the address shall not be equal
 *           to 0
 *        - All bits of the random part of the address shall not be equal to
 *           1
 *        - The address shall not be equal to the static address
 *        - The address shall not be equal to the public address
 *       </li>
 *       <li> Resolvable private address:\n
 *       To generate a resolvable private address the Host must have its
 *       Identity Resolving Key (IRK). The resolvable private address shall be
 *       generated with the IRK and a randomly generated 24-bit random number.
 *       The random number is known as prand and shall meet the following
 *       requirements:
 *        - The two most significant bits of prand shall be equal to
 *           0 and 1
 *        - All bits of the random field of prand shall not be equal to 0
 *        - All bits of the random field of prand shall not be equal to 1
 *       </li>
 *      </ol>
 *   </li>
 *   </ol>
 */
#define BT_BD_RANDOM_ADDRESS_TYPE               1

/**
 *  Random Address Type can be
 *  - Static
 *  - Resolvable Private
 *  - Non-Resolvable Private
 *
 *  No Specific values for the above three sub-types are defined in
 *  Bluetooth Assigned Number.
 *
 *  Defining these sub-types, which can be used by the application, if required.
 *
 *  Note: These definitions can be placed in BT_hci_api.h.
 *  Keeping this non-assigned number defined values in this file
 *  - To have all possible Device Address Type defintions in a single header
 *  - In future, the assigned number might define them, if required
 *    # In that case corresponding defined values might vary
 */
#define BT_BD_STATIC_ADDRESS_TYPE              1
#define BT_BD_RESOLVALE_PVT_ADDRESS_TYPE       2
#define BT_BD_NON_RESOLVALE_PVT_ADDRESS_TYPE   3

/* ==== EIR Data Type/Advertising Data Type (AD Type) and OOB Data Type Definitions */
/* Flags */
#define HCI_AD_TYPE_FLAGS                       0x01

/* Service Class UUIDs */
#define HCI_AD_TYPE_INCOMPLETE_16_BIT_UUIDS     0x02
#define HCI_AD_TYPE_COMPLETE_16_BIT_UUIDS       0x03
#define HCI_AD_TYPE_INCOMPLETE_32_BIT_UUIDS     0x04
#define HCI_AD_TYPE_COMPLETE_32_BIT_UUIDS       0x05
#define HCI_AD_TYPE_INCOMPLETE_128_BIT_UUIDS    0x06
#define HCI_AD_TYPE_COMPLETE_128_BIT_UUIDS      0x07

/* Local Name */
#define HCI_AD_TYPE_SHORTENED_LOCAL_NAME        0x08
#define HCI_AD_TYPE_COMPLETE_LOCAL_NAME         0x09

/* TX Power Level */
#define HCI_AD_TYPE_TX_POWER_LEVEL              0x0A

/* Secure Simple Pairing Out of Band (OOB) */
#define HCI_AD_TYPE_CLASS_OF_DEVICE             0x0D
#define HCI_AD_TYPE_SIMPLE_PAIRING_HASH_C       0x0E
#define HCI_AD_TYPE_SIMPLE_PAIRING_RANDOMIZER_R 0x0F

/* Device ID */
#define HCI_AD_TYPE_DEVICE_ID                   0x10

/* Advertising Data Type - Service Data - 16-bit UUID */
#define HCI_AD_TYPE_SERVICE_DATA_16BIT_UUID     0x16

/* Advertising Data Type - Target Address */
#define HCI_AD_TYPE_PUBLIC_TARGET_ADDRESS       0x17
#define HCI_AD_TYPE_RANDOM_TARGET_ADDRESS       0x18

/* Advertising Data Type - Appearance */
#define HCI_AD_TYPE_APPEARANCE                  0x19

/* Advertising Data Type - Indoor Positioning */
#define HCI_AD_TYPE_INDOOR_POSITIONING          0x25

/* Advertising Data Type - Transport Discovery */
#define HCI_AD_TYPE_TRANSPORT_DISCOVERY         0x26

/* Manufacturer Specific Data */
#define HCI_AD_TYPE_MANUFACTURER_SPECIFIC_DATA  0xFF

/* ==========================================  L2CAP Definitions */
/** L2CAP Fixed channel Connection Identifiers (CID) */
#define L2CAP_SIG_CID                           0x0001
#define L2CAP_GRP_CID                           0x0002
#define L2CAP_ATT_CID                           0x0004
#define L2CAP_LE_CID                            0x0005
#define L2CAP_SMP_CID                           0x0006

/** L2CAP PSM for LE Profiles */
#define LE_PSM_IPSP                             0x0023

#endif /* _H_BT_ASSIGNED_NUMBERS_ */

