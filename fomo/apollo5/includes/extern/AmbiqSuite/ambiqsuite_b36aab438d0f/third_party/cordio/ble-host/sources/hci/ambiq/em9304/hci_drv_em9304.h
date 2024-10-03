//*****************************************************************************
//
//! @file hci_drv_em9304.h
//!
//! @brief Support functions for the EM Micro EM9304 BTLE radio.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef HCI_DRV_EM9304_H
#define HCI_DRV_EM9304_H

//*****************************************************************************
//
// EM9304 vendor specific events
//
//*****************************************************************************
typedef struct
{
    uint32_t    EM_ActiveStateEntered;
    uint32_t    EM_TestModeEntered;
    uint32_t    EM_HalNotification;
    uint32_t    EM_DebugPrint;
    uint32_t    EM_DebugStackUsage;
    uint32_t    EM_DebugBacktrace;
    uint32_t    EM_DebugAssert;
} g_EMVSEvent_t;

// Tx power level in dBm.
typedef enum
{
  TX_POWER_LEVEL_MINOR_33P5_dBm, // 0, not compliant with BT spec
  TX_POWER_LEVEL_MINOR_29P0_dBm, // 1, not compliant with BT spec
  TX_POWER_LEVEL_MINOR_17P9_dBm, // 2
  TX_POWER_LEVEL_MINOR_16P4_dBm, // 3
  TX_POWER_LEVEL_MINOR_14P6_dBm, // 4
  TX_POWER_LEVEL_MINOR_13P1_dBm, // 5
  TX_POWER_LEVEL_MINOR_11P4_dBm, // 6
  TX_POWER_LEVEL_MINOR_9P9_dBm, // 7
  TX_POWER_LEVEL_MINOR_8P4_dBm, // 8
  TX_POWER_LEVEL_MINOR_6P9_dBm, // 9
  TX_POWER_LEVEL_MINOR_5P5_dBm, // 10
  TX_POWER_LEVEL_MINOR_4P0_dBm, // 11
  TX_POWER_LEVEL_MINOR_2P6_dBm, // 12
  TX_POWER_LEVEL_MINOR_1P4_dBm, // 13
  TX_POWER_LEVEL_PLUS_0P4_dBm, // 14
  TX_POWER_LEVEL_PLUS_2P5_dBm, // 15
  TX_POWER_LEVEL_PLUS_4P6_dBm, // 16
  TX_POWER_LEVEL_PLUS_6P2_dBm, // 17
  TX_POWER_LEVEL_INVALID
}txPowerLevel_t;

extern g_EMVSEvent_t *getEM9304VSEventCounters(void);
extern uint32_t HciVsEM_SetRfPowerLevelEx(txPowerLevel_t txPowerlevel);
extern void HciVsEM_TransmitterTest(uint8_t test_mode, uint8_t channel_number, uint8_t packet_len, uint8_t packet_payload_type);
extern void HciVsEM_TransmitterTestEnd(void);
extern void HciVsEM_ReadAtAddress(uint32_t addr);
extern void HciVsEM_WriteAtAddress(uint32_t addr, uint32_t value);
extern void HciDrvAssignBDAddress(uint8_t * customer_unique_bd_address);
extern void HciVsEM_SetBDAddress(void);


#endif // HCI_DRV_EM9304_H
