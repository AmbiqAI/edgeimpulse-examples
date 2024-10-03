//*****************************************************************************
//
//! @file ble_test_cases.c
//!
//! @brief This is the application just test spi hci interface to cooper.
//!
//!
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
#include <string.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_devices_cooper.h"
#include "unity.h"

#include "info0_patch.h"
#include "info1_patch.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
uint32_t DMATCBBuffer[2048];

uint32_t g_clkreqNum = 0;

void *g_IomDevHdl;
void *g_pvHciSpiHandle;

static am_devices_cooper_sbl_update_data_t     g_sBLEInfo0 =
{
    (uint8_t*)&info0_patch_bin,
    sizeof(info0_patch_bin),
    AM_DEVICES_COOPER_SBL_UPDATE_IMAGE_TYPE_INFO_0,
    0
};
/*
static am_devices_cooper_sbl_update_data_t     g_sBLEInfo1 =
{
    (uint8_t*)&info1_patch_bin,
    sizeof(info1_patch_bin),
    AM_DEVICES_COOPER_SBL_UPDATE_IMAGE_TYPE_INFO_1,
    0
};
*/

#if defined(apollo4l_blue_eb_slt)
#include "ble_fw_slt_image.h"
static am_devices_cooper_sbl_update_data_t     g_sBLEImage =
{
    (uint8_t*)&ble_fw_slt_image_bin,
    sizeof(ble_fw_slt_image_bin),
    AM_DEVICES_COOPER_SBL_UPDATE_IMAGE_TYPE_FW,
    0
};
#else
#include "ble_fw_image.h"
static am_devices_cooper_sbl_update_data_t     g_sBLEImage =
{
    (uint8_t*)&ble_fw_image_bin,
    sizeof(ble_fw_image_bin),
    AM_DEVICES_COOPER_SBL_UPDATE_IMAGE_TYPE_FW,
    0
};
#endif

uint8_t nvds_cmd[] =
{
    NVDS_PARAMETER_MEM_WRITE_ENABLE,
    NVDS_PARAMETER_SLEEP_DISABLE,
    NVDS_PARAMETER_EXT_32K_CLK_SOURCE,
    NVDS_PARAMETER_BD_ADDRESS
};

#define BD_ADDRESS_LEN         0x06

//*****************************************************************************
//
// Helper macros.
//
//*****************************************************************************
#define DISABLE_TEST                                                          \
    TEST_IGNORE_MESSAGE("Test disabled.");                                    \
    return

//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
}

void
tearDown(void)
{
}

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{
    am_devices_cooper_config_t stCooperConfig;
    stCooperConfig.pNBTxnBuf = DMATCBBuffer;
    stCooperConfig.ui32NBTxnBufLength = sizeof(DMATCBBuffer) / 4;

#if defined(apollo4l_blue_eb_slt)
    am_util_stdio_printf("*****Update FW*****\n");
    // Update firmware image information to SBL.
    TEST_ASSERT_TRUE(true == am_devices_cooper_get_FwImage(&g_sBLEImage));
#endif
    //
    // Initialize the SPI module.
    //
    am_util_stdio_printf("\n*****Init cooper*****\n");
    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == am_devices_cooper_init(SPI_MODULE, &stCooperConfig, &g_IomDevHdl, &g_pvHciSpiHandle));
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// IRQ interrupt service.
//
//*****************************************************************************
static void HciDrvIntService(void *pArg)
{
#if 0
    am_devices_cooper_buffer(16) uReadData;
    uint32_t ui32NumChars;

    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == am_devices_cooper_blocking_read(g_IomDevHdl, uReadData.words, &ui32NumChars));
    am_util_stdio_printf("\nRX: %d \n", ui32NumChars);
    {
        uint16_t i = 0;
        for ( i = 0; i < ui32NumChars; i++ )
        {
            am_util_stdio_printf("%02x ", uReadData.bytes[i]);
        }
        am_util_stdio_printf("\n\n");
    }
#endif
}

//*****************************************************************************
//
// CLKREQ interrupt service.
//
//*****************************************************************************
static void ClkReqIntService(void *pArg)
{
    if (am_devices_cooper_clkreq_read(g_IomDevHdl))
    {
        // Power up the 32MHz Crystal

        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, (void *) &g_amHalMcuctrlArgBLEDefault);
        g_clkreqNum++;
    }
    else
    {
        am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_DISABLE, (void *) &g_amHalMcuctrlArgBLEDefault);
    }
    am_hal_gpio_intdir_toggle(AM_DEVICES_COOPER_CLKREQ_PIN);
}

//*****************************************************************************
//
// GPIO interrupt handler.
//
//*****************************************************************************
void
am_cooper_irq_isr(void)
{
    uint32_t    ui32IntStatus;

    am_hal_gpio_interrupt_irq_status_get(AM_COOPER_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(AM_COOPER_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_service(AM_COOPER_IRQn, ui32IntStatus);
}

//*****************************************************************************
//
// set specific BD address, based on Cooper chip ID 0/1.
//
//*****************************************************************************
static void set_bd_address(uint8_t* address)
{
    uint32_t i = 0;
    uint8_t BLEMacAddress[BD_ADDRESS_LEN] = {0};
    if(address)
    {
        memcpy(BLEMacAddress, address, BD_ADDRESS_LEN);
    }
    else
    {
        am_hal_mcuctrl_device_t sDevice;

        am_hal_mcuctrl_info_get(AM_HAL_MCUCTRL_INFO_DEVICEID, &sDevice);

        // Bluetooth address formed by ChipID1 (32 bits) and ChipID0 (8-23 bits).
        memcpy(BLEMacAddress, &sDevice.ui32ChipID1, sizeof(sDevice.ui32ChipID1));
        // ui32ChipID0 bit 8-31 is test time during chip manufacturing
        BLEMacAddress[4] = (sDevice.ui32ChipID0 >> 8) & 0xFF;
        BLEMacAddress[5] = (sDevice.ui32ChipID0 >> 16) & 0xFF;
    }

    do
    {
        if (nvds_cmd[i] == PARAM_ID_BD_ADDRESS)
        {
            // searched the head of NVDS_PARAMETER_BD_ADDRESS
            i++; // skip the tag identifier
            i++; // skip the tag status
            if (nvds_cmd[i] == BD_ADDRESS_LEN)
            {
                i++; // skip the tag length byte
                break;
            }
            else
            {
                return;
            }
        }
        else
        {
            // filter out the other parameters
            i++; // skip the tag identifier
            i++; // skip the tag status
            i += nvds_cmd[i]; // skip the tag length
            i++; // skip the tag length byte
            continue;
        }
    }
    while (i < sizeof(nvds_cmd));

    if ((i + BD_ADDRESS_LEN) <= sizeof(nvds_cmd))
    {
        memcpy(&(nvds_cmd[i]), BLEMacAddress, sizeof(BLEMacAddress));
    }
}

static uint32_t get_bd_address(uint8_t *recvpackets)
{
    //
    // Fill the buffer with the specific command we want to write, and send it.
    //
    uint32_t ui32ErrorStatus = AM_DEVICES_COOPER_STATUS_SUCCESS;
    uint8_t cmd[HCI_VSC_CMD_LENGTH(0)] = HCI_RAW_CMD(0x1009, 0);
    uint32_t ui32BytesNum = 0;
    am_devices_cooper_buffer(16) sResponse = {0};

    ui32ErrorStatus = am_devices_cooper_command_write(g_IomDevHdl, (uint32_t*)cmd, sizeof(cmd), sResponse.words, &ui32BytesNum);
    if ( ui32ErrorStatus != AM_DEVICES_COOPER_STATUS_SUCCESS )
    {
        return ui32ErrorStatus;
    }
    memcpy(recvpackets, &sResponse.bytes[7], BD_ADDRESS_LEN);
    return ui32ErrorStatus;
}

//*****************************************************************************
//
// Test firmware update
//
//*****************************************************************************
void ble_test_image_update(void)
{
#if defined(apollo4l_blue_eb_slt)
    DISABLE_TEST;
#endif
    am_devices_cooper_config_t stCooperConfig;
    stCooperConfig.pNBTxnBuf = DMATCBBuffer;
    stCooperConfig.ui32NBTxnBufLength = sizeof(DMATCBBuffer) / 4;

    am_util_stdio_printf("\nBLE Firmware Update Test\n");

#if 0
    am_util_stdio_printf("*****Update INFO0*****\n");
    // Update firmware image information to SBL.
    TEST_ASSERT_TRUE(true == am_devices_cooper_get_info0_patch(&g_sBLEInfo0));
    // write HCI command to trigger Cooper to reboot for SBL to do download.
    am_util_stdio_printf("*****Update signature*****\n");
    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == am_util_ble_update_sign_set(g_IomDevHdl, COOPER_INFO0_UPDATE_SIGN));
    // reset Cooper to get SBL to update
    am_util_stdio_printf("*****Reset cooper to do a forcing upgrade*****\n");
    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == am_devices_cooper_reset_with_sbl_check(g_IomDevHdl, &stCooperConfig));

    am_util_delay_ms(2000);
#endif
#if 0 // We need to unlock the controller before updating INFO1, so skip this step in default
    am_util_stdio_printf("*****Update INFO1*****\n");
    // Update firmware image information to SBL.
    TEST_ASSERT_TRUE(true == am_devices_cooper_get_info1_patch(&g_sBLEInfo1));
    // write HCI command to trigger Cooper to reboot for SBL to do download.
    am_util_stdio_printf("*****Update signature*****\n");
    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == am_util_ble_update_sign_set(g_IomDevHdl, COOPER_INFO1_UPDATE_SIGN));
    // reset Cooper to get SBL to update
    am_util_stdio_printf("*****Reset cooper to do a forcing upgrade*****\n");
    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == am_devices_cooper_reset_with_sbl_check(g_IomDevHdl, &stCooperConfig));

    am_util_delay_ms(2000);
#endif

    am_util_stdio_printf("*****Update FW*****\n");
    // Update firmware image information to SBL.
    TEST_ASSERT_TRUE(true == am_devices_cooper_get_FwImage(&g_sBLEImage));
    // write HCI command to trigger Cooper to reboot for SBL to do download.
    am_util_stdio_printf("*****Update signature*****\n");
    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == am_util_ble_update_sign_set(g_IomDevHdl, COOPER_FW_UPDATE_SIGN));
    // reset Cooper to get SBL to update
    am_util_stdio_printf("*****Reset cooper to do a forcing upgrade*****\n");
    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == am_devices_cooper_reset_with_sbl_check(g_IomDevHdl, &stCooperConfig));

    am_util_stdio_printf("BLE Firmware Update Test Done!\r\n");
}

//*****************************************************************************
//
// Test IRQ and CLKREQ
//
//*****************************************************************************
void ble_test_irq_clkreq(void)
{
#if defined(apollo4l_blue_eb_slt)
    DISABLE_TEST;
#endif
    am_util_stdio_printf("\nBLE IRQ CLKREQ Test\n");

    uint32_t IntNum = AM_DEVICES_COOPER_IRQ_PIN;
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, IntNum, HciDrvIntService, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);

    IntNum = AM_DEVICES_COOPER_CLKREQ_PIN;
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, IntNum, ClkReqIntService, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);
    NVIC_EnableIRQ(AM_COOPER_IRQn);
    //
    // Enable interrupt service routines.
    //
    am_hal_interrupt_master_enable();

    am_util_stdio_printf("\n*****Sleep enabled, wait for a while to confirm clkreq is resumed*****\n");
    am_util_delay_ms(5000);
    TEST_ASSERT_TRUE(g_clkreqNum != 0);

    // disable sleep
    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == am_util_ble_sleep_set(g_IomDevHdl, false));
    am_util_stdio_printf("\n*****Sleep disabled, wait for a while to confirm there's no clkreq*****\n");
    g_clkreqNum = 0;
    am_util_delay_ms(5000);
    TEST_ASSERT_TRUE(g_clkreqNum == 0);
    am_util_stdio_printf("BLE IRQ CLKREQ Test Done\n");
}

//*****************************************************************************
//
// Test hci command
//
//*****************************************************************************
void ble_test_hci_command(void)
{
#if defined(apollo4l_blue_eb_slt)
    DISABLE_TEST;
#endif
    uint32_t data0, data1 = 0;
    uint8_t revc[BD_ADDRESS_LEN] = {0};
    uint8_t BLEMacAddress[BD_ADDRESS_LEN] = {0};
    am_hal_mcuctrl_device_t sDevice;

    am_hal_mcuctrl_info_get(AM_HAL_MCUCTRL_INFO_DEVICEID, &sDevice);

    // Bluetooth address formed by ChipID1 (32 bits) and ChipID0 (8-23 bits).
    memcpy(BLEMacAddress, &sDevice.ui32ChipID1, sizeof(sDevice.ui32ChipID1));
    // ui32ChipID0 bit 8-31 is test time during chip manufacturing
    BLEMacAddress[4] = (sDevice.ui32ChipID0 >> 8) & 0xFF;
    BLEMacAddress[5] = (sDevice.ui32ChipID0 >> 16) & 0xFF;

    am_util_stdio_printf("\nBLE HCI Comand Test\n");

    am_util_stdio_printf("Set BD address\n");
    // set bd address
    set_bd_address(BLEMacAddress);
    am_util_ble_nvds_set(g_IomDevHdl, nvds_cmd, sizeof(nvds_cmd));

    am_util_stdio_printf("Dump info0\n");
    // dump info0
    am_util_ble_info0_dump(g_IomDevHdl);

    am_util_stdio_printf("Get trim version\n");
    // get trim version
    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == am_util_ble_trim_version_get(g_IomDevHdl, &data0, &data1));
    TEST_ASSERT_TRUE(data0 != 0);
    TEST_ASSERT_TRUE(data1 != 0);

    am_util_stdio_printf("Get BD address\n");
    // get bd address
    TEST_ASSERT_TRUE(AM_DEVICES_COOPER_STATUS_SUCCESS == get_bd_address(revc));
    //TEST_ASSERT_TRUE(0 == strcmp(BLEMacAddress, revc));
    am_util_stdio_printf("BLE HCI Comand Test Done\n");
}

//*****************************************************************************
//
// Test adv command
//
//*****************************************************************************
void ble_test_slt(void)
{
#if !defined(apollo4l_blue_eb_slt)
    DISABLE_TEST;
#endif

    am_devices_cooper_buffer(16) sResponse = {0};
    uint32_t ui32BytesNum, ui32Ret = 0;

    // Reset
    ui32Ret = am_util_ble_hci_reset(g_IomDevHdl);
    if(AM_DEVICES_COOPER_STATUS_SUCCESS == ui32Ret)
    {
        am_util_stdio_printf("\n>>> Reset complete <<<\n");
    }
    else
    {
        am_util_stdio_printf("\n>>> Reset failed!!! <<<\n");
        TEST_ASSERT_EQUAL_UINT32(AM_DEVICES_COOPER_STATUS_SUCCESS, ui32Ret);
        return;
    }
    // Adv para
    uint8_t adv_para[HCI_VSC_CMD_LENGTH(15)] = HCI_RAW_CMD(0x2006, 0x0f, 0x30, 0, 0x30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x07, 0);
    TEST_ASSERT_EQUAL_UINT32(AM_DEVICES_COOPER_STATUS_SUCCESS, am_devices_cooper_command_write(g_IomDevHdl, (uint32_t*)adv_para, HCI_VSC_CMD_LENGTH(15), sResponse.words, &ui32BytesNum));
    TEST_ASSERT_EQUAL_UINT8(sResponse.bytes[ui32BytesNum - 1], 0);
    memset(sResponse.words, sizeof(sResponse), 0);
    // Adv data
    uint8_t adv_data[HCI_VSC_CMD_LENGTH(32)] = HCI_RAW_CMD(0x2008, 0x20, 0x0A, 0x02, 0x01, 0x06, 0x06, 0x09, 0x41, 0x4D, 0x42, 0x49, 0x51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    TEST_ASSERT_EQUAL_UINT32(AM_DEVICES_COOPER_STATUS_SUCCESS, am_devices_cooper_command_write(g_IomDevHdl, (uint32_t*)adv_data, HCI_VSC_CMD_LENGTH(32), sResponse.words, &ui32BytesNum));
    TEST_ASSERT_EQUAL_UINT8(sResponse.bytes[ui32BytesNum - 1], 0);
    memset(sResponse.words, sizeof(sResponse), 0);
    // Scan response data
    uint8_t resp_data[HCI_VSC_CMD_LENGTH(32)] = HCI_RAW_CMD(0x2009, 0x20, 0x0A, 0x02, 0x01, 0x06, 0x06, 0x09, 0x41, 0x4D, 0x42, 0x49, 0x51, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    TEST_ASSERT_EQUAL_UINT32(AM_DEVICES_COOPER_STATUS_SUCCESS, am_devices_cooper_command_write(g_IomDevHdl, (uint32_t*)resp_data, HCI_VSC_CMD_LENGTH(32), sResponse.words, &ui32BytesNum));
    TEST_ASSERT_EQUAL_UINT8(sResponse.bytes[ui32BytesNum - 1], 0);
    memset(sResponse.words, sizeof(sResponse), 0);
    // Enable adv
    uint8_t adv_en[HCI_VSC_CMD_LENGTH(1)] = HCI_RAW_CMD(0x200A, 0x01, 0x01);
    TEST_ASSERT_EQUAL_UINT32(AM_DEVICES_COOPER_STATUS_SUCCESS, am_devices_cooper_command_write(g_IomDevHdl, (uint32_t*)adv_en, HCI_VSC_CMD_LENGTH(1), sResponse.words, &ui32BytesNum));

    if(sResponse.bytes[ui32BytesNum - 1] == 0)
    {
        am_util_stdio_printf("\n>>> Advertising started <<<\nThe device name is AMBIQ\n");
    }
    else
    {
        am_util_stdio_printf("\n>>> Advertising failed!!! <<<\nPlease check your environment\n");
        TEST_ASSERT_EQUAL_UINT8(sResponse.bytes[ui32BytesNum - 1], 0);
    }
}

