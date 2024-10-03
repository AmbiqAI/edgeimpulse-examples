
/**
 *  \file BT_gatt_db_api.h
 *
 *  \brief Module API Header File contains structure definitions and API definitions
 *  offered to the application.
 */

/*
 *  Copyright (C) 2014. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_BT_GATT_DB_API_
#define _H_BT_GATT_DB_API_

/* --------------------------------------------- Header File Inclusion */
#include "BT_common.h"
#include "BT_att_api.h"
#include "gatt_defines.h"

/* --------------------------------------------- Global Definitions */

/**
 * \defgroup gatt_db_module GATT DB (Generic Attribute Profile Database)
 * \{
 *  This section describes the interfaces & APIs offered by
 *  Generic Attribute Profile Database (GATT_DB) module to the Application and
 *  other upper layers of the stack. GATT Procedures are
 *  realized using ATT APIs. This module abstracts the Server Database.
 */


/**
 *  \defgroup gatt_db_defines Defines
 *  \{
 *  This section describes the various defines for the module.
 */

/**
 *  \defgroup gatt_db_access_operations Access Operations
 *  \{
 *  These abstract acess functions for attributes, including read, write,
 *  prepare. The direction of update is aslo included as this decides
 *  checking needed security for peer and if values are editable by the peer.
 *  Please also note the 'Update' access which is used for local updates of
 *  values based on \ref GATT_DB_HANDLE as againt Attribute Handle.
 */

/** Read operation for Attribute  */
#define GATT_DB_READ                            0x01

/** Write operation for Attribute */
#define GATT_DB_WRITE                           0x02

/** Write Without Response operation for Attribute */
#define GATT_DB_WRITE_WITHOUT_RSP               0x04

/** Read Write operation for Attribute */
#define GATT_DB_READ_WRITE\
        (GATT_DB_READ | GATT_DB_WRITE | GATT_DB_WRITE_WITHOUT_RSP)

/** Prepare Write operation for Attribute */
#define GATT_DB_PREPARE                         0x08

/**
 *  Local update of a Readable Attribute Value using \ref GATT_DB_HANDLE instead
 *  of Attribute Handle. See \ref BT_gatt_db_set_char_val for more details.
 */
#define GATT_DB_UPDATE                          0x10

/* 128 Bit Attribute UUID  */
#define GATT_DB_128_BIT_UUID_FORMAT             0x20

/**
 *  Peer initiated operation, needed when Peer is Reading or Writing Values
 *  either using GATT Read Procedures for Value and Descriptors.
 */
#define GATT_DB_PEER_INITIATED                  0x80

/**
 *  Locally initiated operation, needed for local updates based on Attribute
 *  Handles. In case \ref GATT_DB_HANDLE is known, use of access through
 *  \ref GATT_DB_UPDATE is recommended.
 */
#define GATT_DB_LOCALLY_INITIATED               0x00

/** \} */

/**
 *  \defgroup gatt_db_operations Operations Notified In Characteristic Callback
 *  \{
 *  This section describes the operations notified by the module to the
 *  application through the callback regsitered by the application.
 */

/** Characteristic Value Local Read Operation */
#define GATT_DB_CHAR_VALUE_READ_REQ             0x01

/** Characteristic Value Local Write Operation */
#define GATT_DB_CHAR_VALUE_WRITE_REQ            0x02

/** Characteristic Value Local Write Without Response Operation */
#define GATT_DB_CHAR_VALUE_WRITE_WITHOUT_REQ    0x03

/** Characteristic Client Configuration Local Read Operation */
#define GATT_DB_CHAR_CLI_CNFG_READ_REQ          0x11

/** Characteristic Client Configuration Local Write Operation */
#define GATT_DB_CHAR_CLI_CNFG_WRITE_REQ         0x12

/** Characteristic Server Configuration Local Read Operation */
#define GATT_DB_CHAR_SER_CNFG_READ_REQ          0x21

/** Characteristic Server Configuration Local Write Operation */
#define GATT_DB_CHAR_SER_CNFG_WRITE_REQ         0x22

/** Characteristic Value Peer Read Operation */
#define GATT_DB_CHAR_PEER_READ_REQ              0x81

/** Characteristic Value Peer Write Operation */
#define GATT_DB_CHAR_PEER_WRITE_REQ             0x82

/** Characteristic Value Peer Write Command */
#define GATT_DB_CHAR_PEER_WRITE_CMD             0x84

/** Characteristic Client Configuration Peer Read Operation */
#define GATT_DB_CHAR_PEER_CLI_CNFG_READ_REQ     0x91

/** Characteristic Client Configuration Peer Write Operation */
#define GATT_DB_CHAR_PEER_CLI_CNFG_WRITE_REQ    0x92

/** Characteristic Server Configuration Peer Read Operation */
#define GATT_DB_CHAR_PEER_SER_CNFG_READ_REQ     0xA1

/** Characteristic Server Configuration Peer Write Operation */
#define GATT_DB_CHAR_PEER_SER_CNFG_WRITE_REQ    0xA2

/** Characteristic User Description Peer Read Operation */
#define GATT_DB_CHAR_PEER_USR_DESC_READ_REQ     0xB1

/** Characteristic User Description Peer Write Operation */
#define GATT_DB_CHAR_PEER_USR_DESC_WRITE_REQ    0xB2

/** Characteristic Higher Layer Defined Descriptor Peer Read Operation */
#define GATT_DB_CHAR_PEER_HLD_DESC_READ_REQ     0xF1

/** Characteristic Higher Layer Defined Descriptor Peer Write Operation */
#define GATT_DB_CHAR_PEER_HLD_DESC_WRITE_REQ    0xF2

/** \} */

/**
 *  \defgroup gatt_characteristic_properties Characteristic Properties
 *  \{
 *  Characteristic Properties - Combination of these properties (combined
 *  using bitwise OR) describe properties of the Characteristic. Also
 *  see \ref GATT_DB_ATTR_LIST.
 */

/**
 *  Characteristc support Broadcast of its value to the peer.
 *  Setting this property automatically includes Characteristic Server
 *  Configuration Descriptor to the Characteristic
 */
#define GATT_DB_CHAR_BROADCAST_PROPERTY                0x00000001

/** Characteristc support Reading its value by peer */
#define GATT_DB_CHAR_READ_PROPERTY                     0x00000002

/** Characteristc support Writing its value by peer Without Response  */
#define GATT_DB_CHAR_WRITE_WITHOUT_RSP_PROPERTY        0x00000004

/** Characteristic supports Writing its value by peer */
#define GATT_DB_CHAR_WRITE_PROPERTY                    0x00000008

/**
 *  Characteristic supports Notifying its value to the peer.
 *  Setting this property automatically includes Characteristic Server
 *  Configuration Descriptor to the Characteristic
 */
#define GATT_DB_CHAR_NOTIFY_PROPERTY                   0x00000010

/** Characteristic supports Indicating its value to the peer */
#define GATT_DB_CHAR_INDICATE_PROPERTY                 0x00000020

/** Characteristic supports Signed Write on its value */
#define GATT_DB_CHAR_SIGNED_WRITE_PROPERTY             0x00000040

/** Characteristic supports write on its User Description Descriptor */
#define GATT_DB_CHAR_WRIEABLE_AUX_PROPERTY             0x00000200

/** No Auxillary Property */
#define GATT_DB_NO_AUXILLARY_PROPERTY                  0x00

/** Characteristic Value is Fixed Length */
#define GATT_DB_FIXED_LENGTH_PROPERTY                  0x01

#define GATT_DB_AUTHORIZATION_PROPERTY                 0x02

#define GATT_DB_PEER_SPECIFIC_VAL_PROPERTY             0x40

#define GATT_DB_CONST_ATTR_VAL_PROPERTY                0x80

/** \} */

/**
 *  \defgroup gatt_service_properties Service Properties
 *  \{
 *  Service Properties - Combination of these properties (combined
 *  using bitwise OR) describe properties of the Service. Also
 *  see \ref GATT_DB_SERVICE_DESC.
 */

/**
 *  Security related - combination of Level, Mode and Encryption Key Size
 *  (if applicable) describes complete security needs
 */
/** Service employs Level 0 */
#define GATT_DB_SER_SECURITY_LEVEL0                    0x00000000

/** Service employs Level 1 */
#define GATT_DB_SER_SECURITY_LEVEL1                    0x00000001

/** Service employs Level 2 */
#define GATT_DB_SER_SECURITY_LEVEL2                    0x00000002

/** Service employs Level 3 */
#define GATT_DB_SER_SECURITY_LEVEL3                    0x00000004

/** Service employs Mode 1 */
#define GATT_DB_SER_SECURITY_MODE1                     0x00000010

/** Service employs Mode 2 */
#define GATT_DB_SER_SECURITY_MODE2                     0x00000020

/** \cond ignore_this Unused right now */
#define GATT_DB_SER_MULTIPLE_CLIENTS_SUPPORT           0x00000040
/** \endcond */

/**
 *  Service is a Secondary Service. If this not used, Service is by default
 *  considered to be Primary
 */
#define GATT_DB_SER_SECONDARY_SERVICE_PROPERTY         0x00000080

#ifdef GATT_DB_SUPPORT_128_BIT_UUID

/** Service UUID is 128-bit */
#define GATT_DB_SER_128_BIT_UUID_FORMAT                0x00008000

#endif /* GATT_DB_SUPPORT_128_BIT_UUID */

/** Set this property to relax all Security on the Service */
#define GATT_DB_SER_NO_SECURITY_PROPERTY               0x00000000

/**
 *  This is employed to describe Service is not rigid on any Encryption
 *  Key Size. Any Size used by other Services or on the whole by the device
 *  will do
 */
#define GATT_DB_SER_ENC_KEY_SIZE_DONT_CARE             0x00000000

/** Encyrption Key Size 7 Needed for the Service */
#define GATT_DB_SER_ENCRYPT_KEY_SIZE_7                 0x01000000

/** Encyrption Key Size 8 Needed for the Service */
#define GATT_DB_SER_ENCRYPT_KEY_SIZE_8                 0x02000000

/** Encyrption Key Size 9 Needed for the Service */
#define GATT_DB_SER_ENCRYPT_KEY_SIZE_9                 0x03000000

/** Encyrption Key Size 10 Needed for the Service */
#define GATT_DB_SER_ENCRYPT_KEY_SIZE_10                0x04000000

/** Encyrption Key Size 11 Needed for the Service */
#define GATT_DB_SER_ENCRYPT_KEY_SIZE_11                0x05000000

/** Encyrption Key Size 12 Needed for the Service */
#define GATT_DB_SER_ENCRYPT_KEY_SIZE_12                0x06000000

/** Encyrption Key Size 13 Needed for the Service */
#define GATT_DB_SER_ENCRYPT_KEY_SIZE_13                0x07000000

/** Encyrption Key Size 14 Needed for the Service */
#define GATT_DB_SER_ENCRYPT_KEY_SIZE_14                0x08000000

/** Encyrption Key Size 15 Needed for the Service */
#define GATT_DB_SER_ENCRYPT_KEY_SIZE_15                0x09000000

/** Encyrption Key Size 16 Needed for the Service */
#define GATT_DB_SER_ENCRYPT_KEY_SIZE_16                0x0A000000

/** \cond ignore_this unused */
#define GATT_DB_SER_NO_ENCRYPT_PROPERTY                0x00000000

/**
 *  Macros to describe the Transport access of the Service over BR/EDR only,
 *  LE only or Any Tranport.
 */
/** Service accessible over BR/EDR Link Only */
#define GATT_DB_SER_SUPPORT_BR_LINK_TYPE               0x10000000

/** Service accessble over LE Link Only */
#define GATT_DB_SER_SUPPORT_LE_LINK_TYPE               0x20000000

/** Service accessible over Any Link */
#define GATT_DB_SER_SUPPORT_ANY_LINK_TYPE              0xF0000000
/** \endcond */

/** \} */

/** \cond ignore_this This Block is for internal use */

#define GATT_DB_SECURITY_MASK                          0x00000037
#define GATT_DB_ENC_KEY_SIZE_MASK                      0x0F000000
#define GATT_DB_SER_ENC_KEY_SIZE_LAG                   6

#define GATT_PRIMARY_SERVICE_TYPE_OFFSET      0
#define GATT_SECONDARY_SERVICE_TYPE_OFFSET    2
#define GATT_INCLUDE_TYPE_OFFSET              4
#define GATT_CHARACTERISTIC_TYPE_OFFSET       6

#define GATT_VERIFY_UUID_VALUE(v,o,l)\
        ((0 == BT_mem_cmp((v),GATT_GET_UUID_VALUE(o),(l)))? BT_TRUE : BT_FALSE)

#define GATT_GET_UUID_VALUE(o)\
        &gatt_const_uuid_arr[(o)]

#define GATT_DB_GET_UUID_OFFSET(i)\
        gatt_db_attr_table[(i)].uuid_offset

#define GATT_DB_GET_UUID(i)\
        GATT_GET_UUID_VALUE(GATT_DB_GET_UUID_OFFSET(i))

#define GATT_CHK_ATTR_HNDL_AUTHORIZATION(hndl) \
        (GATT_DB_AUTHORIZATION_PROPERTY == \
        (gatt_db_attr_table[( hndl )].aux_property & GATT_DB_AUTHORIZATION_PROPERTY)) ? BT_TRUE : BT_FALSE

/** \endcond */

/**
 *  This is dummy macro for Characteristic Value UUID used to locally
 *  read or update Characteristic Value using \ref GATT_DB_HANDLE.
 */
#define GATT_DUMMY_CHAR_VALUE_UUID                           0x0000

/** \} */
/* --------------------------------------------- Data Types/ Structures */

/**
 * \defgroup gatt_db_typedefs Typedefs
 * \{
 * This section describes the types used in the module.
 */

/**
 *  \cond ignore_this Unused, was intended for use in
 *  \ref GATT_DB_HANDLE, not being used right now
 */

typedef UCHAR GATT_DB_CHAR_HANDLE;
typedef UCHAR GATT_DB_SERVICE_HANDLE;

/** \endcond */

typedef UCHAR GATT_TYPE_INDEX;

typedef struct
{
    UINT16 uuid_offset;

    UINT16 fo;

    UINT16 lo;

}GATT_ATTR_TYPE;

/**
 *  This abstracts properties of Characteristic, including how the value can be
 *  accessed, supported descriptors. See \ref gatt_characteristic_properties.
 *   - Bit 0-7  Characteristic Property as defined by the Specification.
 *   - Bit 8-23 Characteristic Extended Property as defined by the Specification.
 *   - Bit 24-30 Supported Characteristic Descriptors
 *   - Bit 31   Reserved for Future Extension.
 */
typedef struct
{
    UCHAR   count;

}GATT_DB_ATTR_LIST;

/**
 *  This abstracts properties of service, including needed secuirty, encryption
 *  key size, distinction of primary/secondary service. See
 *  \ref gatt_service_properties.
 *
 *  NOTE: GATT Specification allows Each Characteristic/Service to specify the
 *  endianness of its Characteristic Values.
 */
typedef UINT32 GATT_DB_SERVICE_DESC;

/** \} */

/**
 * \defgroup gatt_db_structures Structures
 * \{
 * This section describes the data structures for the module.
 */

/** Abstracts GATT Characteristic */
typedef struct
{
    /** Characerisic Description */
    GATT_DB_ATTR_LIST       list;

    /**
     *  Characterisic Handle Range
     *  Please Note:
     *  Characteristic Value                              is at start_handle + 1
     *  Client Configuration                (if present)  is at start_handle + 2
     *  Server Configuration                (if present)  is at start_handle + 3
     *  Characteristic Presentation Format  (if present)  is at start_handle + 4
     *  User Description                    (if present)  is at start_handle + 5
     *  Characteristc Aggregate Format      (if present)  is at start_handle + 6
     *  Extended Property Format            (if present)  is at start_handle + 7
     *  Profile Defined Descriptor          (if present)  from start_handle + 8
     */
    /** Start Handle */
    ATT_ATTR_HANDLE        start_handle;

    /** Service to which Characteristic Belongs */
    UCHAR                  service_id;

}GATT_DB_CHARACERISTIC;

/** Abstracts  GATT Service */
typedef struct
{
    /** Characerisic Description */
    GATT_DB_ATTR_LIST       list;

    GATT_DB_SERVICE_DESC    desc;

    /* Service Handle Range */
    ATT_ATTR_HANDLE         start_handle;

    ATT_ATTR_HANDLE         end_handle;

    /* Range of Characeristics grouped in the service */
    UCHAR                   char_start_index;

    UCHAR                   char_end_index;

}GATT_DB_SERVICE;

/**
 *  Abstracts Not So Often Used Descriptor Information
 *  Including descriptors like Characteristic Presentation Format,
 *  Aggregate, User Defined Descriptor, Higher Layer Defined Descriptors.
 *  'length' indicatates size of descriptor value and data_offset contains
 *  offset to the value in gatt_db_desc_data_array.
 *
 *  Note: In case of Higher Layer Defined Descriptors, first 2 octets contain
 *  UUID, next octet contains Descriptor Properties (Read/Write/Indicate etc),
 *  followed  'length' octets of value.
 */
typedef struct
{
    /* Describe how the descriptor could be accessed */
    UCHAR     desc_property;

    /**
     *  Provides information if the desc value is fixed length or the UUID
     *  format info, if the value is maintained peer specific etc.
     */
    UCHAR     aux_property;

    /**  Length of Descriptor Value */
    UINT16    length;

    /* Pointer to Next Attribute of this type */
    UINT16     next;

    /* Offset to UUID value */
    UINT16     uuid_offset;

    /**  Offset where the Descriptor Value is located */
    UCHAR     * data_offset;

}GATT_DB_DESC_DATA;

typedef struct
{
    /** Identifies the peer accessing the database */
    DEVICE_HANDLE    device_id;

    /** Identifies the Service being Accessed */
    UCHAR            service_id;

    /** Identifies the Characteristic being Accessed */
    UCHAR            char_id;

}GATT_DB_HANDLE;

/**
 *  Abstracts the Access Operation when notifying the application of the Access
 *  Request from Peer through the Callback for the Characteristic. See also
 *  \ref gatt_db_operations
 */
typedef UCHAR GATT_DB_OPERATION;

/**
 *  Abstracts parameters for application specific handling for characteristics.
 *  See also \ref GATT_DB_OPERATION and \ref gatt_db_operations
 */
typedef struct
{
    /** Abstracts Value to get/set the Attribute Value */
    ATT_VALUE            value;

    /** Abstracts the handle information */
    ATT_ATTR_HANDLE      handle;

    /** Abstracts the Access Operation */
    GATT_DB_OPERATION    db_op;

}GATT_DB_PARAMS;

/** \} */

/**
 *  \defgroup gatt_db_marcos Utility Macros
 *  \{
 *  Utility Macros offered by the module.
 */
/** Macro to fetch needed security for a Service */
#define GATT_DB_GET_NEEDED_SECURITY(sid)\
        (UCHAR)(gatt_service[(sid)].desc & GATT_DB_SECURITY_MASK)

/** Macro to fetch needed Encryption Key Size for a Service */
#define GATT_DB_GET_NEEDED_ENC_KEY_SIZE(sid)\
        (UCHAR)((GATT_DB_SER_ENC_KEY_SIZE_DONT_CARE == \
        ((gatt_service[(sid)].desc & GATT_DB_ENC_KEY_SIZE_MASK) >> 24)) ? \
        GATT_DB_SER_ENC_KEY_SIZE_DONT_CARE : (((gatt_service[(sid)].desc & \
        GATT_DB_ENC_KEY_SIZE_MASK) >> 24) + GATT_DB_SER_ENC_KEY_SIZE_LAG))

#define GATT_DB_GET_CHAR_VAL_HANDLE(i,j)\
        (j) = (gatt_characteristic[(i)].start_handle + 1)

/** \} */

/** \cond ignore_this Ingore this block */
extern DECL_CONST GATT_DB_SERVICE gatt_service[];
extern DECL_CONST GATT_DB_CHARACERISTIC gatt_characteristic[];
extern DECL_CONST UCHAR gatt_const_value_arr[];
extern DECL_CONST UCHAR gatt_const_uuid_arr[];
extern DECL_CONST GATT_DB_DESC_DATA gatt_db_attr_table[];
extern DECL_CONST GATT_ATTR_TYPE gatt_type_table[];
extern UCHAR gatt_db_peer_specific_val_arr[];
extern DECL_CONST UCHAR gatt_db_const_peer_specific_val_arr[];
extern UCHAR gatt_value_arr[];

#ifndef GATT_DB_HAVE_STATIC_CONFIGURATION
extern DECL_CONST UCHAR  gatt_service_count;
extern DECL_CONST UCHAR  gatt_characteristic_count;
extern DECL_CONST UCHAR  gatt_db_max_type_count;
extern DECL_CONST UCHAR  gatt_db_peer_val_arr_size;

#ifdef STORAGE_RETENTION_SUPPORT
extern DECL_CONST UINT16 gatt_value_array_size;
extern DECL_CONST UINT16 gatt_db_max_peer_config;

#define GET_GATT_VALUE_ARRAY_SIZE()              gatt_value_array_size
#define GET_GATT_DB_MAX_PEER_CONFIG()            gatt_db_max_peer_config
#endif /* STORAGE_RETENTION_SUPPORT */

#define GET_GATT_SERVICE_COUNT()                 gatt_service_count
#define GET_GATT_CHARACTERISTIC_COUNT()          gatt_characteristic_count
#define GET_GATT_MAX_TYPE_COUNT()                gatt_db_max_type_count
#define GET_GATT_DB_PEER_VALUE_ARRAY_SIZE()      gatt_db_peer_val_arr_size
#else
#define GET_GATT_SERVICE_COUNT()             GATT_SERVICE_COUNT
#define GET_GATT_CHARACTERISTIC_COUNT()      GATT_CHARACTERISTIC_COUNT
#define GET_GATT_MAX_TYPE_COUNT()            GATT_DB_MAX_TYPE_COUNT
#define GET_GATT_DB_PEER_VALUE_ARRAY_SIZE()  GATT_DB_PEER_VALUE_ARRAY_SIZE

#ifdef STORAGE_RETENTION_SUPPORT
#define GET_GATT_VALUE_ARRAY_SIZE()              GATT_VALUE_ARRAY_SIZE
#define GET_GATT_DB_MAX_PEER_CONFIG()            GATT_DB_MAX_PEER_CONFIG
#endif /* STORAGE_RETENTION_SUPPORT */

#endif /* GATT_DB_HAVE_STATIC_CONFIGURATION */
/** \endcond */

/* --------------------------------------------- Functions */

#ifdef __cplusplus
extern "C"{
#endif

/**
 *  \defgroup gatt_db_api API Definitions
 *  \{
 *  This section describes the APIs offered by the module to the application.
 */

/**
 *  \brief To Initialize the Module.
 *
 *  \par Description
 *  This API is used to initialize the module and shall be called before using
 *  any of the other module routines
 */
void BT_gatt_db_init (void);

API_RESULT gatt_db_access_value
           (
               /* IN */  ATT_HANDLE        * inst,
               /* OUT */ ATT_VALUE         * value,
               /* IN */  ATT_ATTR_HANDLE   handle,
               /* IN */  UINT16            offset,
               /* IN */  UCHAR             flag
           );

API_RESULT gatt_db_get_group_hndl_val_list
           (
               /* IN */    ATT_HANDLE           * att_inst,
               /* IN */    ATT_HANDLE_RANGE     * range,
               /* OUT */   ATT_HANDLE_RANGE     * group_range,
               /* INOUT */ ATT_VALUE            * value,
               /* IN */    ATT_VALUE            * uuid,
               /* IN */    UCHAR                  compare_val_flag
           );

API_RESULT BT_gatt_db_get_attr_handle_prpty
           (
               /* IN */  GATT_DB_HANDLE      * handle,
               /* IN */  ATT_HANDLE_RANGE    * range,
               /* IN */  UINT16                uuid,
               /* OUT */ ATT_ATTR_HANDLE     * attr_handle,
               /* OUT */ UCHAR               * attr_prpty
           );

/**
 *  \brief To search for group entities or get values of a speicifc group.
 *
 *  \par Description
 *  This routine searches for group entities (services) or get values of a
 *  speicifc group(PRIMARY/SECONDARY). This API is used to formulate response
 *  of Read By Group Type Request. This routine provides one search result at a
 *  time. Note, Currently only Primary Services are Supported.
 *
 *  \param att_inst
 *         Identifies the Peer Entity Requesting the Read of Attribute. This is
 *         important so that necessary Security Checks are performed before the
 *         the value is fetched.
 *
 *  \param range
 *         Requested Range for Group Search
 *
 *  \param group_range
 *         Range of Group Entity on Successful Search.
 *
 *  \param value
 *         Group (Servie) Value.
 *
 *  \param uuid
 *         Identifies the UUID of Group Entity (PRIMARY/SECONDARY).
 *
 *  \return API_SUCCESS on successful search and no more gorup entities
 *          (Services) exist.
 *          GATT_DB_MORE_MATCHING_RESULT_FOUND on successful search and more
 *          Gorup Entities (Services) exist to search further.
 *          Others - Appropriate Code indicating reason why search did not
 *          succeed.
 */
#define BT_gatt_db_get_group_range_val_pair(att_inst,range,uuid,group_range,value)\
        gatt_db_get_group_hndl_val_list\
        (\
            (att_inst),\
            (range),\
            (group_range),\
            (value),\
            (uuid),\
            BT_FALSE\
        )


/**
 *  \brief To get Handle Attribute Type Pair in the requested Range.
 *
 *  \par Description
 *  This routine fetches Handle Attribute Type Pair in the requested Range,
 *  providing one Search Result at a time and is used to formulate the response
 *  for Find Info Request.
 *
 *  \param att_inst
 *         Identifies the Peer Entity Requesting the Read of Attribute. This is
 *         important so that necessary Security Checks are performed before the
 *         the value is fetched.
 *
 *  \param range
 *         Requested Range for Handle Attribute Type List.
 *
 *  \param handle_uuid
 *         On Successful Search, contains Handle Attribute Type (UUID) Pair in
 *         Requested Range.
 *
 *  \return API_SUCCESS on successful search and no more attributes exist.
 *          GATT_DB_MORE_MATCHING_RESULT_FOUND on successful search and more
 *          attributes exist to search further.
 *          Others - Appropriate Code indicating reason why search did not
 *          succeed.
 */
API_RESULT BT_gatt_db_get_handle_uuid_pair
           (
               /* IN */  ATT_HANDLE               * att_inst,
               /* IN */  ATT_HANDLE_RANGE         * range,
               /* OUT */ ATT_HANDLE_VALUE_PAIR    * handle_uuid
           );


/**
 *  \brief To get Handle Range for Attribute Type Value Pair in the requested
 *  Range.
 *
 *  \par Description
 *  This routine fetches Handle Attribute Value Pair in the requested Range of
 *  a particular Attribute Type, providing one Search Result at a time and
 *  is used to formulate the response for Read By Type Request.
 *
 *  \param inst
 *         Identifies the Peer Entity Requesting the Read of Attribute. This is
 *         important so that necessary Security Checks are performed before the
 *         the value is fetched.
 *
 *  \param range
 *         Requested Range for Handle Attribute Value List.
 *
 *  \param uuid
 *         Identifies the Attribute Type whose Values are requested.
 *
 *  \param handle_value_pair
 *         On Successful Search, contains Handle Value Pair in Requested Range
 *         and of requested Attribute Type.
 *
 *  \return API_SUCCESS on successful search and no more attributes exist.
 *          GATT_DB_MORE_MATCHING_RESULT_FOUND on successful search and more
 *          attributes exist to search further.
 *          Others - Appropriate Code indicating reason why search did not
 *          succeed.
 */
API_RESULT BT_gatt_db_get_handle_value_pair
           (
                /* IN */  ATT_HANDLE               * inst,
                /* IN */  ATT_HANDLE_RANGE         * range,
                /* IN */  ATT_VALUE                * uuid,
                /* OUT */ ATT_HANDLE_VALUE_PAIR    * handle_value_pair
           );
/**
 *  \brief To get Handle Attribute Value Pair in the requested Range and of a
 *         particular Atttribute Type.
 *
 *  \par Description
 *  This routine fetches Handle Range for Attribute Value Type Pair in the
 *  requested Range, providing one Search Result at a time and is used to
 *  formulate the response for Find By Type Value Request. Note, if the
 *  Attribute is not a group range, start end handle will be equal to end
 *  handle.
 *
 *  \param inst
 *         Identifies the Peer Entity Requesting the Read of Attribute. This is
 *         important so that necessary Security Checks are performed before the
 *         the value is fetched.
 *
 *  \param range
 *         Requested Range for Attritbute search.
 *
 *  \param type
 *         Identifies 16-bit Type of Attribute to be matched.
 *
 *  \param value
 *         Contains the Value of Attribute to be matched.
 *
 *  \param attr_range
 *         On Successful Search, contains Handle Range of the attribue requested
 *         by Type & Value.
 *
 *  \return API_SUCCESS on successful search and no more attributes exist.
 *          GATT_DB_MORE_MATCHING_RESULT_FOUND on successful search and more
 *          attributes exist to search further.
 *          Others - Appropriate Code indicating reason why search did not
 *          succeed.
 */
API_RESULT BT_gatt_db_get_range_by_type_val
           (
               /* IN */  ATT_HANDLE          * inst,
               /* IN */  ATT_HANDLE_RANGE    * range,
               /* IN */  ATT_VALUE           * type,
               /* IN */  ATT_VALUE           * value,
               /* OUT */ ATT_HANDLE_RANGE    * attr_range
           );

/**
 *
 *  \brief To Get Characteristic Value Attribute Handle.
 *
 *  \par Description
 *  This routine provides Attribute Handle for Characteristic Value of a
 *  Characteristic.
 *
 *  \param handle
 *         \ref GATT_DB_HANDLE, identifying the Characteristic whose Value handle
 *         is to be fetched.
 *
 *  \param attr_handle
 *         On successful return of the routine contains the Attribute handle
 *         as visible to the peer.
 *
 *  \return API_SUCCESS on success or an appropriate error code.
 */
#define BT_gatt_db_get_char_val_hndl(handle,attr_handle)\
        GATT_DB_GET_CHAR_VAL_HANDLE((handle)->char_id,(*(attr_handle)))


API_RESULT gatt_db_access_val_by_db_handle
           (
                /* IN */ GATT_DB_HANDLE    * handle,
                /* IN */ ATT_VALUE         * attr_value,
                /* IN */ UINT16            uuid,
                /* IN */ UCHAR             flag
           );

/**
 *
 *  \brief To Locally Set Characteristic Value based on \ref GATT_DB_HANDLE.
 *
 *  \par Description
 *  This routine updates Characteristic Value of the Characteristic indentified
 *  by handle.
 *
 *  \param [in] handle
 *         \ref GATT_DB_HANDLE, identifying the Characteristic whose value is Set.
 *
 *  \param [in] value
 *         Value to which Characteristic Value is being updated.
 *
 *  \return API_SUCCESS on success or an appropriate error code.
 */
#define BT_gatt_db_set_char_val(handle,value)\
        gatt_db_access_val_by_db_handle\
        (\
                (handle),\
                (value),\
                GATT_DUMMY_CHAR_VALUE_UUID,\
                GATT_DB_UPDATE\
        )\

/**
 *
 *  \brief To Locally Get Characteristic Value based on \ref GATT_DB_HANDLE.
 *
 *  \par Description
 *  This routine Gets/Reads Characteristic Value of the Characteristic
 *  indentified by handle.
 *
 *  \param [in] handle
 *         \ref GATT_DB_HANDLE, identifying the Characteristic whose value is Read.
 *
 *  \param [out] value
 *         Contains Characteristic Value in case of successful return of this
 *         API.
 *
 *  \return API_SUCCESS on success or an appropriate error code.
 */
#define BT_gatt_db_get_char_val(handle,value)\
        gatt_db_access_val_by_db_handle\
        (\
                (handle),\
                (value),\
                GATT_DUMMY_CHAR_VALUE_UUID,\
                GATT_DB_READ\
        )

/**
 *
 *  \brief To Locally Get Client Configuration Descriptor Value of
 *         Characteristic.
 *
 *  \par Description
 *  This routine Gets/Reads Value of the Client Configuration Descriptor for the
 *  characteristic indentified by handle.
 *
 *  \param [in] handle
 *         \ref GATT_DB_HANDLE, identifying the Characteristic whose value is Read.
 *
 *  \param [out] value
 *         Contains Characteristic Value in case of successful return of this
 *         API.
 *
 *  \return API_SUCCESS on success or an appropriate error code.
 */
#define BT_gatt_db_get_char_cli_cnfg(handle,value)\
        gatt_db_access_val_by_db_handle\
        (\
                (handle),\
                (value),\
                GATT_CLIENT_CONFIG,\
                GATT_DB_READ\
        )

/**
 *
 *  \brief To Locally Get Application/Profile defined Characteristic Descriptor
 *         Value based on \ref GATT_DB_HANDLE.
 *
 *  \par Description
 *  This routine Gets/Reads Value of the Application/Profile defined
 *  Characteristic Descriptor (referred to as HLD standing for Higher Layer
 *  Defined Descriptor) for the characteristic indentified by handle.
 *  Note: Used only for characteristic with single Higher Layer Descriptor.
 *
 *  \param [in] handle
 *         \ref GATT_DB_HANDLE, identifying the Characteristic whose value is Read.
 *
 *  \param [out] value
 *         Contains Characteristic Value in case of successful return of this
 *         API.
 *
 *  \param [in] uuid
 *         UUID, identifying the Characteristic whose value is Read.
 *
 *  \return API_SUCCESS on success or an appropriate error code.
 */
#define BT_gatt_db_get_hld_char_desc_val(handle,value,uuid)\
        gatt_db_access_val_by_db_handle\
        (\
                (handle),\
                (value),\
                (uuid),\
                GATT_DB_READ\
        )

/**
 *
 *  \brief To Read/Write Attribute Value identfied by Handle.
 *
 *  \par Description
 *  This routine performs read/write of Attribute Value for the requested
 *  Handle. This is used to formulate the response for Read, Read Blob, Write,
 *  Write Command. Before the access is allowed necessary Security & Permission
 *  Checks are performed for the peer and Application is notified of the access
 *  if the Application has registered a callback for the Characteristic to which
 *  the attribute belongs.
 *  This routine can be used for local updates of the database as well using
 *  necessary flags.
 *
 *  \param handle_value
 *         Contains Attribute Handle to be accessed and if the access is
 *         successful, contains the Attribute Value.
 *
 *  \param att_instance
 *         Identifies the Peer Entity Requesting access of Attribute. This is
 *         important so that necessary Security Checks are performed before the
 *         the value is fetched. In case of Local Read, this is important if
 *         client specific values are maintained.
 *
 *  \param offset
 *         Identifies the value offset from where the access is requested.
 *
 *  \param flags
 *         Describes nature of access, Read, Write, Write, Without Rsp, Local or
 *         Peer.
 *
 *  \return API_SUCCESS on success access or an appropriate error code.
 */
API_RESULT BT_gatt_db_access_handle
           (
                /* INOUT */ ATT_HANDLE_VALUE_PAIR     * handle_value,
                /* IN */    ATT_HANDLE                * att_instance,
                /* IN */    UINT16                    offset,
                /* IN */    UCHAR                     flags
           );


/**
 *
 *  \brief To Convert a 128-bit UUID in BT Space to its 16-bit equivalent.
 *
 *  \par Description
 *  This routine converts a 128-bit UUID in BT Space to its 16-bit equivalent.
 *  If the 128-bit UUID is outside the BT Space, an error is indicated.
 *
 *  \param uuid128
 *         128-bit UUID to be converted to 16-bit equivalent.
 *
 *  \param uuid16
 *         On Success contains the 16-bit equivalent of the 128-bit UUID.
 *
 *  \return API_SUCCESS on success or an appropriate error code.
 */
API_RESULT BT_gatt_db_get_16_bit_uuid
           (
                 /* IN */  ATT_VALUE    * uuid128,
                 /* OUT */ ATT_VALUE    * uuid16
           );


/**
 *
 *  \brief To Get the Secuirty requirement of the Service
 *
 *  \par Description
 *  This routine reads the needed secuirty level on the Service identified by
 *  the handle. Note: Even though this is a macro, it is to be treated as a
 *  function.
 *
 *  \param [in] hndl
 *         Identifies the Service whose Security needs are being read.
 *
 *  \param [out] sec
 *         Security Level and Mode of the Service.
 *
 */
#define BT_gatt_db_get_service_security(hndl,sec)\
        ((*(UCHAR *)(sec)) = GATT_DB_GET_NEEDED_SECURITY((hndl)->service_id))


/**
 *
 *  \brief To Get the Encryption Key Size of the Service
 *
 *  \par Description
 *  This routine reads the needed encryption key size for the Service identified by
 *  the handle. Note: Even though this is a macro, it is to be treated as a
 *  function.
 *
 *  \param [in] hndl
 *         Identifies the Service whose Encryption Key Size is being read.
 *
 *  \param [out] key_size
 *         Needed Encryption Key Size for the Service.
 *
 */
#define BT_gatt_db_get_service_enc_key_size(hndl,key_size)\
        ((*(UCHAR *)(key_size)) = GATT_DB_GET_NEEDED_ENC_KEY_SIZE((hndl)->service_id))

/**
 *
 *  \brief Book Keeping of Peer Specific Data on Disconnect or Shutdown.
 *
 *  \par Description
 *  This routine is used for Book Keeping of Peer Specific Data on Disconnect or
 *  Shutdown. For Bonded Devices, Client Specific Configurations will be stored
 *  persistently while for non bonded devices, all Client specific information
 *  will be reinitialized.
 *
 *  \param inst
 *         Identifies the Peer instance for which Book Keeping is to be
 *         performed.
 *
 *  \return API_SUCCESS on success or an appropriate error code.
 */
API_RESULT BT_gatt_db_peer_session_shutdown_handler
           (
               /* IN */ ATT_HANDLE   * inst
           );

/** \} */

/** \} */

API_RESULT gatt_char_handler (GATT_DB_HANDLE * handle,GATT_DB_PARAMS * param);

#ifdef __cplusplus
};
#endif

#endif /* _H_BT_GATT_DB_API_ */

