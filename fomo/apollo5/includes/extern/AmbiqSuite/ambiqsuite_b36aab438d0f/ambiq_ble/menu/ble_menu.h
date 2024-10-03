// ****************************************************************************
//
//  ble_menu.h
//! @file
//!
//! @brief Functions for BLE control menu.
//!
//! @{
//
// ****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef BLE_MENU_H
#define BLE_MENU_H

#include "am_util.h"
#include "am_util_stdio.h"
#include "am_bsp.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    BLE_MENU_ID_MAIN = 0,
    BLE_MENU_ID_GAP,
    BLE_MENU_ID_GATT,
    BLE_MENU_ID_AMDTP,
    BLE_MENU_ID_MAX
}eBleMenuId;

typedef enum
{
    GAP_MENU_ID_NONE = 0,
    GAP_MENU_ID_SCAN_START,
    GAP_MENU_ID_SCAN_STOP,
    GAP_MENU_ID_SCAN_RESULTS,
    GAP_MENU_ID_CONNECT,
    GAP_MENU_ID_MAX
}eGapMenuId;

typedef enum
{
    GATT_MENU_ID_NONE = 0,
    GATT_MENU_ID_TBD,
    GATT_MENU_ID_MAX
}eGattMenuId;

typedef enum
{
    AMDTP_MENU_ID_NONE = 0,
    AMDTP_MENU_ID_SEND,
    AMDTP_MENU_ID_SEND_STOP,
    AMDTP_MENU_ID_SERVER_SEND,
    AMDTP_MENU_ID_SERVER_SEND_STOP,
    AMDTP_MENU_ID_MAX
}eAmdtpMenuId;

typedef struct
{
    eBleMenuId prevMenuId;
    eBleMenuId menuId;
    eGapMenuId gapMenuSelected;
    eGattMenuId gattMenuSelected;
}sBleMenuCb;

extern char menuRxData[20];
extern uint32_t menuRxDataLen;

extern uint32_t am_menu_printf(const char *pcFmt, ...);

void
BleMenuRx(void);

void
BleMenuInit(void);

#ifdef __cplusplus
}
#endif

#endif // BLE_MENU_H
