//*****************************************************************************
//
//! @file am_devices_da14580.h
//!
//! @brief Support functions for the Dialog Semiconductor DA14580 BTLE radio.
//!
//! @addtogroup da14580 DA14580 BTLE Radio Device Driver
//! @ingroup devices
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_DEVICES_DA14580_H
#define AM_DEVICES_DA14580_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Dialog GTL byte macros.
//
//*****************************************************************************
//! Message header byte.
#define AM_DEVICES_DA14580_GTL_START             0x05

//! Task IDs
#define AM_DEVICES_DA14580_GTL_TASK              0x3F

//! @name Commands
//! @brief Commands
//! @{

#define AM_DEVICES_DA14580_RESET_CMD             0x3402
#define AM_DEVICES_DA14580_ADVERTISE_CMD         0x340E
#define AM_DEVICES_DA14580_PROXR_ENABLE_REQ      0x4000
#define AM_DEVICES_DA14580_PROXR_CREATE_DB_REQ   0x4003
//! @}

//! @name EventTypes
//! @brief Event Types
//! @{
#define AM_DEVICES_DA14580_LLC_RD_PW_LVL_EVT     0x0410
#define AM_DEVICES_DA14580_GAPM_CMP_EVT          0x3400
#define AM_DEVICES_DA14580_GAPM_STACK_READY      0x3401
#define AM_DEVICES_DA14580_PROXR_DISABLE_IND     0x4001
#define AM_DEVICES_DA14580_PROXR_ALERT_IND       0x4002
#define AM_DEVICES_DA14580_PROXR_ERROR_IND       0x4005
#define AM_DEVICES_DA14580_PROXR_CREATE_DB_CFM   0x4004
#define AM_DEVICES_DA14580_GAPC_CONNECTION_REQ   0x3801
#define AM_DEVICES_DA14580_GAPC_DISCONNECT_EVT   0x3803
//! @}

//*****************************************************************************
//
//! Message structure used by Dialog radio devices.
//
//*****************************************************************************
typedef struct
{
    uint16_t ui16ID;
    uint16_t ui16Dest;
    uint16_t ui16Source;
    uint16_t ui16ParamLength;
    uint8_t *pui8Params;
}
am_devices_da14580_gtl_msg_t;

typedef struct
{
    //! GAPM requested operation:
    uint8_t ui8Operation;

    //! Device Role: Central, Peripheral, Observer or Broadcaster
    uint8_t ui8Role;

    //! Device IRK used for resolvable random BD address generation (LSB first)
    uint8_t ui8Key[16];

    //! Device Appearance (0x0000 - Unknown appearance)
    uint16_t ui16Appearance;

    //! Device Appearance write permission requirements for peer device
    uint8_t  ui8AppearanceWritePerm;

    //! Device Name write permission requirements for peer device
    uint8_t  ui8NameWritePerm;

    //! Maximal MTU
    uint16_t ui16MaxMTU;

    //! Slave preferred Minimum of connection interval
    uint16_t ui16MinConnectInterval;

    //! Slave preferred Maximum of connection interval
    uint16_t ui16MaxConnectInterval;

    //! Slave preferred Connection latency
    uint16_t ui16ConnectLatency;

    //! Slave preferred Link supervision timeout
    uint16_t ui16SupervisionTimeout;

    // Privacy settings bit field (0b1 = enabled, 0b0 = disabled)
    //  - [bit 0]: Privacy Support
    //  - [bit 1]: Multiple Bond Support (Peripheral only); If enabled, privacy
    //             flag is read only.
    //  - [bit 2]: Reconnection address visible.
    uint8_t ui8PrivacyFlags;
}
am_devices_da14580_gapm_dev_cfg_t;

//! Set advertising mode Command
typedef struct
{
    //! Operation code.
    uint8_t ui8Code;

    //! Own BD address source of the device:
    //!     - GAPM_PUBLIC_ADDR: Public Address
    //!     - GAPM_PROVIDED_RND_ADDR: Provided random address
    //!     - GAPM_GEN_STATIC_RND_ADDR: Generated static random address
    //!     - GAPM_GEN_RSLV_ADDR: Generated resolvable private random address
    //!     - GAPM_GEN_NON_RSLV_ADDR: Generated non-resolvable private random address
    //!     - GAPM_PROVIDED_RECON_ADDR: Provided Reconnection address (only for GAPM_ADV_DIRECT)
    uint8_t ui8AddrSrc;

    //! Dummy data use to retrieve internal operation state (should be set to 0).
    uint16_t ui16State;

    //! Duration of resolvable address before regenerate it.
    uint16_t ui16RenewDur;

    //! Provided own static private random address (addr_src = 1 or 5)
    uint8_t ui8BDAddr[6];

    //! Minimum interval for advertising
    uint16_t ui16IntvMin;
    //! Maximum interval for advertising
    uint16_t ui16IntvMax;

    //! Advertising channel map
    uint8_t ui8ChannelMap;

    //! Advertising mode :
    //!     - GAP_NON_DISCOVERABLE: Non discoverable mode
    //!     - GAP_GEN_DISCOVERABLE: General discoverable mode
    //!     - GAP_LIM_DISCOVERABLE: Limited discoverable mode
    //!     - GAP_BROADCASTER_MODE: Broadcaster mode
    uint8_t ui8Mode;

    //! Advertising filter policy:
    //!     - ADV_ALLOW_SCAN_ANY_CON_ANY: Allow both scan and connection requests from anyone
    //!     - ADV_ALLOW_SCAN_WLST_CON_ANY: Allow both scan req from White List devices only and
    //!         connection req from anyone
    //!     - ADV_ALLOW_SCAN_ANY_CON_WLST: Allow both scan req from anyone and connection req
    //!         from White List devices only
    //!     - ADV_ALLOW_SCAN_WLST_CON_WLST: Allow scan and connection requests from White List
    //!         devices only
    uint8_t ui8AdvFiltPolicy;

    //! Advertising data length - maximum 28 bytes, 3 bytes are reserved to set
    //!     - Advertising AD type flags, shall not be set in advertising data
    uint8_t ui8AdvDataLen;

    //! Advertising data
    uint8_t pui8AdvData[28];

    //! Scan response data length- maximum 31 bytes
    uint8_t ui8ScanRspDataLen;

    //! Scan response data
    uint8_t pui8ScanRspData[31];
}
am_devices_da14580_gapm_start_advertise_cmd_t;

//*****************************************************************************
//
// LLC Command Structures
//
//*****************************************************************************

//
//! Read TX Power Level Command
//
typedef struct
{
    //! Connection handle
    uint16_t ui16ConnectionHandle;

    //! Power Level type: current or maximum
    uint8_t ui8Type;
}
am_devices_da14580_llc_rd_pw_lvl_cmd_t;

//
//! LLC Read TX power level command complete event.
//
typedef struct
{
    //! Status for command reception
    uint8_t ui8Status;

    //! Connection handle
    uint16_t ui16ConnectionHandle;

    //! Value of TX power level
    uint8_t ui8PowerLevel;
}
am_devices_da14580_llc_power_reading_t;

//*****************************************************************************
//
// Proximity Profile Command Structures
//
//*****************************************************************************

//
//! Request to enable the proximity profile.
//
typedef struct
{
    uint16_t ui16ConnectionHandle;
    uint8_t ui8SecurityLevel;
    uint8_t ui8LLSAlertLevel;
    int8_t i8TXPLevel;
}
am_devices_da14580_proxr_enable_req_t;

//
//! Request to disable the proximity profile.
//
typedef struct
{
    uint16_t ui16ConnectionHandle;
    uint8_t  ui8LLSAlertLevel;
}
am_devices_da14580_proxr_disable_ind_t;

//
//! Proximity Alert indicator.
//
typedef struct
{
    uint16_t ui16ConnectionHandle;
    uint8_t ui8AlertLevel;
    uint8_t ui8CharCode;
}
am_devices_da14580_proxr_alert_ind_t;

//
//! Create Proximity database request.
//
typedef struct
{
    uint8_t ui8Features;
}
am_devices_da14580_proxr_create_db_req_t;

//
//! Proximity database creation confirmation.
//
typedef struct
{
    uint8_t ui8Status;
}
am_devices_da14580_proxr_create_db_cfm_t;

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Runs a UART based boot sequence for a Dialog radio device.
//!
//! @param pui8BinData   - Pointer to an array of bytes containing the firmware
//! for the DA14580
//! @param ui32NumBytes  - Length of the DA14580 firmware image.
//! @param ui32UartModule - Uart number
//!
//! This function allows the Ambiq device to program a "blank" DA14580 device
//! on startup. It will handle all of the necessary UART negotiation for the
//! Dialog boot procedure, and will verify that the CRC value for the
//! downloaded firmware image is correct.
//!
//! @return true if successful.
//
//*****************************************************************************
extern bool am_devices_da14580_uart_boot(const uint8_t *pui8BinData,
                                         uint32_t ui32NumBytes,
                                         uint32_t ui32UartModule);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_DA14580_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

