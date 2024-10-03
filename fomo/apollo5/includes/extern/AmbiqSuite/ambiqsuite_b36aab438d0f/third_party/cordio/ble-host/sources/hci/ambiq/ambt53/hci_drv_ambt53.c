//*****************************************************************************
//
//! @file hci_drv_ambt53.c
//!
//! @brief HCI driver interface.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"

#include "wsf_types.h"
#include "wsf_timer.h"
#include "bstream.h"
#include "wsf_msg.h"
#include "wsf_cs.h"
#include "hci_drv.h"
#include "hci_cmd.h"
#include "hci_drv_apollo.h"
#include "hci_tr_apollo.h"
#include "hci_core.h"
#include "dm_api.h"

#include "hci_drv_ambt53.h"
#include "hci_dbg_trc.h"

#include <string.h>
#include "rpc_client_hci.h"

// set the BLE MAC address to a special value
uint8_t g_BLEMacAddress[6] = {0};

// Global handle used to send BLE events about the Hci driver layer.
wsfHandlerId_t g_HciDrvHandleID = 0;


//extern struct rpmsg_rcv_msg rpmsg_data;

//*****************************************************************************
//
// Boot the radio.
//
//*****************************************************************************
uint32_t
HciDrvRadioBoot(bool bColdBoot)
{
    // When it's bColdBoot, it will use Apollo's Device ID to form Bluetooth address.
    if (bColdBoot)
    {
        am_hal_mcuctrl_device_t sDevice;
        am_hal_mcuctrl_info_get(AM_HAL_MCUCTRL_INFO_DEVICEID, &sDevice);

        // Bluetooth address formed by ChipID1 (32 bits) and ChipID0 (8-23 bits).
        memcpy(g_BLEMacAddress, &sDevice.ui32ChipID1, sizeof(sDevice.ui32ChipID1));
        // ui32ChipID0 bit 8-31 is test time during chip manufacturing
        g_BLEMacAddress[4] = (sDevice.ui32ChipID0 >> 8) & 0xFF;
        g_BLEMacAddress[5] = (sDevice.ui32ChipID0 >> 16) & 0xFF;
    }
    return 0;
}

//*****************************************************************************
//
// Shut down the BLE core.
//
//*****************************************************************************
void
HciDrvRadioShutdown(void)
{
}

//*****************************************************************************
//
// Function used by the BLE stack to send HCI messages to the BLE controller.
//
// Internally, the Cordio BLE stack will allocate memory for an HCI message,
//
//*****************************************************************************

uint16_t
hciDrvWrite(uint8_t type, uint16_t len, uint8_t *pData)
{
    int length = 0;

    length = rpc_client_hci_send_packet(type, pData, len);

    if (length > 0)
    {
        return len;
    }
    else
    {
        return 0;
    }
}

//*****************************************************************************
//
// Save the handler ID of the HciDrvHandler so we can send it events through
// the WSF task system.
//
// Note: These two lines need to be added to the exactle initialization
// function at the beginning of all Cordio applications:
//
//     handlerId = WsfOsSetNextHandler(HciDrvHandler);
//     HciDrvHandler(handlerId);
//
//*****************************************************************************
void
HciDrvHandlerInit(wsfHandlerId_t handlerId)
{
    g_HciDrvHandleID = handlerId;
}

//*****************************************************************************
//
// Event handler for HCI-related events.
//
// This handler can perform HCI reads or writes, and keeps the actions in the
// correct order.
//
//*****************************************************************************
void
HciDrvHandler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
    //
    //need to check the each bit of event mask
    //
    if (event & BLE_TRANSFER_NEEDED_EVENT)
    {
        // Nothing to do at the moment
        //app_process_rpmsg_rx(rpmsg_data.data);
        //RPMsgDrvReleaseRxBuffer(rpmsg_data.ept, rpmsg_data.data);
    }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVscSetRfPowerLevelEx
 *
 *  \brief  Vendor specific command for settting Radio transmit power level
 *          for Ambiq.
 *
 *  \param  txPowerlevel    valid range from 0 to 15 in decimal.
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
bool HciVscSetRfPowerLevelEx(txPowerLevel_t txPowerlevel)
{
    // make sure it's 8 bit
    uint8_t tx_power_level = (uint8_t)txPowerlevel;

    if(tx_power_level < TX_POWER_LEVEL_INVALID) {
      HciVendorSpecificCmd(HCI_VSC_SET_TX_POWER_LEVEL_CFG_CMD_OPCODE, HCI_VSC_SET_TX_POWER_LEVEL_CFG_CMD_LENGTH, &tx_power_level);
      return true;
    }
    else {
      return false;
    }
}

/*************************************************************************************************/
/*!
 *  \brief  read memory variable
 *
 *  \param  start_addr   Start address to read
 *  \param  size         Access size
 *  \param  length       Length to read
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
bool HciVscReadMem(uint32_t start_addr, eMemAccess_type size,uint8_t length)
{
    hciRdMemCmd_t rdMemCmd =
    {
        .start_addr = start_addr,
        .type = size,
        .length = length,
    };

    if((length > MAX_MEM_ACCESS_SIZE)
        || ((size!=RD_8_Bit)&&(size!=RD_16_Bit)&&(size!=RD_32_Bit)))
    {
        return false;
    }

    HciVendorSpecificCmd(HCI_VSC_RD_MEM_CMD_OPCODE, HCI_VSC_RD_MEM_CMD_LENGTH, (uint8_t *)&rdMemCmd);

    return true;
}

/*************************************************************************************************/
/*!
 *  \brief  write memory variable
 *
 *  \param  start_addr   Start address to write
 *  \param  size         Access size
 *  \param  length       Length to write
 *  \param  data         Data to write
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
bool HciVscWriteMem(uint32_t start_addr, eMemAccess_type size,uint8_t length, uint8_t *data)
{
    hciWrMemCmd_t wrMemCmd =
    {
        .start_addr = start_addr,
        .type = size,
        .length = length
    };

    if((length > MAX_MEM_ACCESS_SIZE)
        || ((size!=RD_8_Bit)&&(size!=RD_16_Bit)&&(size!=RD_32_Bit)))
    {
        return false;
    }

    memset(wrMemCmd.data, 0x0, MAX_MEM_ACCESS_SIZE);
    memcpy(wrMemCmd.data, data, length);

    HciVendorSpecificCmd(HCI_VSC_WR_MEM_CMD_OPCODE, HCI_VSC_WR_MEM_CMD_LENGTH, (uint8_t *)&wrMemCmd);

    return true;
}


/*************************************************************************************************/
/*!
 *  \brief  Get flash ID
 *
 *  \param  NULL
 *
 *  \return None
 */
/*************************************************************************************************/
void HciVscGetFlashId(void)
{
    HciVendorSpecificCmd(HCI_VSC_ID_FLASH_CMD_OPCODE, HCI_VSC_ID_FLASH_CMD_LENGTH, NULL);
}


/*************************************************************************************************/
/*!
 *  \brief  Erase specifide flash space
 *
 *  \param  type     Flash type
 *  \param  offset   Start offset address
 *  \param  size     Size to erase
 *
 *  \return None
 */
/*************************************************************************************************/
void HciVscEraseFlash(uint8_t type, uint32_t offset,uint32_t size)
{
    hciErFlashCmd_t erFlashCmd =
    {
        .flashtype = type,
        .startoffset = offset,
        .size = size
    };

    HciVendorSpecificCmd(HCI_VSC_ER_FLASH_CMD_OPCODE, HCI_VSC_ER_FLASH_CMD_LENGTH, (uint8_t *)&erFlashCmd);
}


/*************************************************************************************************/
/*!
 *  \brief  write flash
 *
 *  \param  type     Flash type
 *  \param  offset   Start offset address
 *  \param  length   Length to write
 *  \param  data     Data to write
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
bool HciVscWriteFlash(uint8_t type, uint32_t offset,uint32_t length, uint8_t *data)
{
    hciWrFlashCmd_t wrFlashCmd =
    {
        .flashtype = type,
        .length = length,
        .startoffset = offset
    };

    if(data == NULL)
    {
        return false;
    }

    memset(wrFlashCmd.data, 0x0, MAX_FLASH_ACCESS_SIZE);
    memcpy(wrFlashCmd.data, data, length);

    HciVendorSpecificCmd(HCI_VSC_WR_FLASH_CMD_OPCODE, HCI_VSC_WR_FLASH_CMD_LENGTH, (uint8_t *)&wrFlashCmd);

    return true;
}



/*************************************************************************************************/
/*!
 *  \brief  Read flash
 *
 *  \param  type     Flash type
 *  \param  offset   Start offset address
 *  \param  size     Size to read
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
bool HciVscReadFlash(uint8_t type, uint32_t offset,uint32_t size)
{
    hciRdFlashCmd_t rdFlashCmd =
    {
        .flashtype = type,
        .startoffset = offset,
        .size = size
    };

    HciVendorSpecificCmd(HCI_VSC_RD_FLASH_CMD_OPCODE, HCI_VSC_RD_FLASH_CMD_LENGTH, (uint8_t *)&rdFlashCmd);

    return true;
}

/*************************************************************************************************/
/*!
 *  \brief  Read Register value
 *
 *  \param  reg_addr  Register address to read
 *
 *  \return None
 */
/*************************************************************************************************/
void HciVscReadReg(uint32_t reg_addr)
{
    hciRegRdCmd_t rdRegCmd =
    {
        .addr = reg_addr
    };

    HciVendorSpecificCmd(HCI_VSC_REG_RD_CMD_OPCODE, HCI_VSC_REG_RD_CMD_LENGTH, (uint8_t *)&rdRegCmd);
}

/*************************************************************************************************/
/*!
 *  \brief  Write Register value
 *
 *  \param  reg_addr   Register address to read
 *  \param  value      Value to write
 *
 *  \return None
 */
/*************************************************************************************************/
void HciVscWriteReg(uint32_t reg_addr, uint32_t value)
{
    hciRegWrCmd_t wrRegCmd =
    {
        .addr = reg_addr,
        .value = value
    };

    HciVendorSpecificCmd(HCI_VSC_REG_WR_CMD_OPCODE, HCI_VSC_REG_WR_CMD_LENGTH, (uint8_t *)&wrRegCmd);
}

/*************************************************************************************************/
/*!
 *
 *  \brief  Vendor specific command for updating firmware
 *
 *  \param  update_sign    firmware type to update
 *
 *  \return None
 */
/*************************************************************************************************/
void HciVscUpdateFw(uint32_t update_sign)
{
    HciVendorSpecificCmd(HCI_VSC_UPDATE_FW_CFG_CMD_OPCODE, HCI_VSC_UPDATE_FW_CFG_CMD_LENGTH, (uint8_t *)&update_sign);
}

/*************************************************************************************************/
/*!
 *  \brief  Get device ID
 *
 *  \param  NULL
 *
 *  \return None
 */
/*************************************************************************************************/
void HciVscGetDeviceId(void)
{
    HciVendorSpecificCmd(HCI_VSC_GET_DEVICE_ID_CFG_CMD_OPCODE, HCI_VSC_GET_DEVICE_ID_CFG_CMD_LENGTH, NULL);
}

/*************************************************************************************************/
/*!
 *  \brief  platform reset
 *
 *  \param  reson   Reson to reset platform
 *
 *  \return None
 */
/*************************************************************************************************/
void HciVscPlfReset(ePlfResetReason_type reason)
{
    hciPlfResetCmd_t resetPlfCmd =
    {
        .reason = reason
    };

    HciVendorSpecificCmd(HCI_VSC_PLF_RESET_CMD_OPCODE, HCI_VSC_PLF_RESET_CMD_LENGTH, (uint8_t *)&resetPlfCmd);
}

#ifdef ENABLE_BLE_CTRL_TRACE
/*************************************************************************************************/
/*!
 *  \brief  Set trace bitmap to enable which traces to output to host.
 *
 *  \param  bit_map    bit map for trace module
 *
 *  \return None.
 */
/*************************************************************************************************/
void HciVscSetTraceBitMap(ble_trace_cfg bit_map)
{
    HciVendorSpecificCmd(HCI_VSC_SET_LOG_BITMAP_CFG_CMD_OPCODE, HCI_VSC_SET_LOG_BITMAP_CFG_CMD_LENGTH, (uint8_t *)&bit_map);
}
#endif

/*************************************************************************************************/
/*!
 *  \fn     HciVscSetCustom_BDAddr
 *
 *  \brief  This procedure is to set customer-provided Bluetooth address if needed.
 *
 *  \param  bd_addr  pointer to a bluetooth address customer allocates or NULL to use Apollo Device ID.
 *
 *  \return true when success
 */
/*************************************************************************************************/
bool_t HciVscSetCustom_BDAddr(uint8_t *bd_addr)
{
    uint8_t invalid_bd_addr[6] = {0};

    // When bd_addr is null, it will use Apollo's Device ID to form Bluetooth address.
    if ((bd_addr == NULL) || (memcmp(invalid_bd_addr, bd_addr, 6) == 0))
        return false;
    else {
        memcpy(g_BLEMacAddress, bd_addr, 6);
        return true;
    }
}

void HciVscUpdateBDAddress(void)
{
    HciVendorSpecificCmd(HCI_VSC_SET_BD_ADDR_CFG_CMD_OPCODE, HCI_VSC_SET_BD_ADDR_CFG_CMD_LENGTH, g_BLEMacAddress);
}


uint8_t nvds_data[HCI_VSC_UPDATE_NVDS_CFG_CMD_LENGTH]=
{
    NVDS_PARAMETER_MAGIC_NUMBER,
    NVDS_PARAMETER_SLEEP_ALGO_DUR,
    NVDS_PARAMETER_LPCLK_DRIFT,
    NVDS_PARAMETER_EXT_WAKEUP_TIME,
    NVDS_PARAMETER_OSC_WAKEUP_TIME
    //NVDS_PARAMETER_SLEEP_DISABLE
};

void HciVscUpdateNvdsParam(void)
{
    HciVendorSpecificCmd(HCI_VSC_UPDATE_NVDS_CFG_CMD_OPCODE, HCI_VSC_UPDATE_NVDS_CFG_CMD_LENGTH, nvds_data);
}

uint8_t ll_local_feats[LE_FEATS_LEN] = {0};

void HciVscUpdateLinklayerFeature(void)
{
    ll_local_feats[0] = (uint8_t)LL_FEATURES_BYTE0;
    ll_local_feats[1] = (uint8_t)(LL_FEATURES_BYTE1>>8);
    ll_local_feats[2] = (uint8_t)(LL_FEATURES_BYTE2>>16);
    ll_local_feats[3] = (uint8_t)(LL_FEATURES_BYTE3>>24);

    HciVendorSpecificCmd(HCI_VSC_UPDATE_LL_FEATURE_CFG_CMD_OPCODE, LE_FEATS_LEN, ll_local_feats);
}

