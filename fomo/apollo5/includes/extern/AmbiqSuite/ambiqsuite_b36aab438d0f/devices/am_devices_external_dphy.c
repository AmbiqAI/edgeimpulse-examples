//*****************************************************************************
//
//! @file
//!
//! @brief
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
#include <stdbool.h>
#include <string.h>

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"
#include "am_devices_external_dphy.h"

void *g_IomDevHandle;

typedef struct
{
    uint32_t                    ui32Module;
    uint32_t                    ui32CS;
    void                        *pIomHandle;
    bool                        bOccupied;
} am_iom_dphy_t;

am_iom_dphy_t gAmDphy = {0};

static uint32_t
iom_init(uint32_t ui32Module, void **ppHandle)
{
    void *pHandle;
    am_hal_iom_config_t stIOMSettings;

    if ( ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    //
    // Configure the IOM pins.(invalid)
    //
    //am_bsp_iom_pins_enable(ui32Module, AM_HAL_IOM_SPI_MODE);

    stIOMSettings.eInterfaceMode = AM_HAL_IOM_SPI_MODE;
    stIOMSettings.ui32ClockFreq = AM_HAL_IOM_1MHZ;
    stIOMSettings.eSpiMode = AM_HAL_IOM_SPI_MODE_0;
    stIOMSettings.pNBTxnBuf = NULL;
    stIOMSettings.ui32NBTxnBufLength = 0;

    //
    // Initialize the IOM instance.
    // Enable power to the IOM instance.
    // Configure the IOM for Serial operation during initialization.
    // Enable the IOM.
    // HAL Success return is 0
    //
    if (am_hal_iom_initialize(ui32Module, &pHandle) ||
        am_hal_iom_power_ctrl(pHandle, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_iom_configure(pHandle, &stIOMSettings) ||
        am_hal_iom_enable(pHandle))
    {
        return AM_HAL_STATUS_FAIL;
    }
    else
    {
        am_util_delay_us(150);
        gAmDphy.ui32Module = ui32Module;
        gAmDphy.ui32CS = 0;
        //*ppIomHandle =
        gAmDphy.pIomHandle = pHandle;
        gAmDphy.bOccupied = true;
        *ppHandle = (void *)&gAmDphy;
        return AM_HAL_STATUS_SUCCESS;
    }
}

static uint32_t
iom_deinit(void *pHandle)
{
    am_iom_dphy_t *pIom = (am_iom_dphy_t *)pHandle;

    if ( pIom->ui32Module > AM_REG_IOM_NUM_MODULES )
    {
        return AM_HAL_STATUS_OUT_OF_RANGE;
    }

    // Disable the pins
    am_bsp_iom_pins_disable(pIom->ui32Module, AM_HAL_IOM_SPI_MODE);
    //
    // Disable the IOM.
    //
    am_hal_iom_disable(pIom->pIomHandle);

    //
    // Disable power to and uninitialize the IOM instance.
    //
    am_hal_iom_power_ctrl(pIom->pIomHandle, AM_HAL_SYSCTRL_DEEPSLEEP, false);

    am_hal_iom_uninitialize(pIom->pIomHandle);

    pIom->bOccupied = false;
    //
    // Return the status.
    //
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief
//! @param pHandle
//! @param ui32InstrLen
//! @param ui32Instr
//! @param pData
//! @param ui32NumBytes
//! @return
//
//*****************************************************************************
static uint32_t
am_device_command_read(void *pHandle, uint32_t ui32InstrLen, uint32_t ui32Instr,
                       uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    am_iom_dphy_t *pIom = (am_iom_dphy_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = ui32InstrLen;
    Transaction.ui64Instr       = ui32Instr;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32RxBuffer   = pData;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_HAL_STATUS_FAIL;
    }
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Generic Command Write function.
//! @param pHandle
//! @param ui32InstrLen
//! @param ui32Instr
//! @param pData
//! @param ui32NumBytes
//! @return
//
//*****************************************************************************
static uint32_t
am_device_command_write(void *pHandle, uint32_t ui32InstrLen, uint32_t ui32Instr,
                        uint32_t *pData, uint32_t ui32NumBytes)
{
    am_hal_iom_transfer_t Transaction;
    am_iom_dphy_t *pIom = (am_iom_dphy_t *)pHandle;

    //
    // Create the transaction.
    //
    Transaction.ui32InstrLen    = ui32InstrLen;
    Transaction.ui64Instr       = ui32Instr;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = pData;
    Transaction.uPeerInfo.ui32SpiChipSelect = pIom->ui32CS;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    //
    // Execute the transction over IOM.
    //
    if (am_hal_iom_blocking_transfer(pIom->pIomHandle, &Transaction))
    {
        return AM_HAL_STATUS_FAIL;
    }
    return AM_HAL_STATUS_SUCCESS;
}

static uint32_t
dphy_cfg(void *pHandle, dphy_trim_t trim)
{
    uint32_t ui32Status;
    uint32_t ui32ReadData = 0;
    uint32_t ui32Instr_Data[CMD_NUM << 1] =
    {
        0x7F,        0x00,  //reset
        0x10,        0x00,  //trim_0[7:0]
        0x11,        0x22,  //trim_0[15:8]
        0x12,        0x86,  //trim_0[23:16]
        0x13,        0x4E,  //trim_0[31:24]
        0x14,        0x01,  //trim_1[7:0]
        0x15,        0x01,  //trim_1[15:8]
        0x16,        0x1C,  //trim_1[23:16]
        0x17,        0x0E,  //trim_1[31:24]
        0x18,        0xA4,  //trim_2[7:0]
        0x19,        0x39,  //trim_2[15:8]
        0x1A,        0x01,  //trim_2[23:16]
        0x1B,        0x51,  //trim_2[31:24]
        0x1C,        0x01,  //0x41, //0x06, //trim_3[7:0]
        0x1D,        0x00,  //trim_3[15:8]
        0x1E,        0x38,  //0x38, //0x38, //trim_3[23:16]
        0x1F,        0x42,  //trim_3[31:24]
        0x4A,        0x00,  //trim_4[7:0]
        0x4B,        0x00,  //trim_4[15:8]
        0x4C,        0x00,  //trim_4[23:16]
        0x4D,        0x00,  //trim_4[31:24]
        0x4E,        0x00,  //trim_5[7:0]
        0x4F,        0x00,  //trim_5[15:8]
        0x50,        0x00,  //trim_5[23:16]
        0x51,        0x00,  //trim_5[31:24]
        0x00,        0x02,  //dln_cnt_hs_prep
        0x01,        0x03,  //dln_cnt_hs_zero
        0x02,        0x05,  //dln_cnt_hs_trail
        0x03,        0x05,  //dln_cnt_hs_exit
        0x04,        0x04,  //dln_rx_cnt
        0x05,        0xFF,  //dln_sync_cnt
        0x06,        0x03,  //dln_cnt_lpx
        0x07,        0x03,  //cln_cnt_hs_trail
        0x08,        0x05,  //cln_cnt_hs_exit
        0x09,        0x03,  //cln_cnt_lpx
        0x0A,        0x02,  //cln_cnt_prep
        0x0B,        0x0D,  //cln_cnt_zero
        0x20,        0x00,  //TGR_global_Config
        0x21,        0x00,  //TGR_Preamble_Length
        0x22,        0x00,  //TGR_Post_Length
        0x0F,        0x00,  //dln_rx_post_cnt
        0x0C,        0xD0,  //cln_cnt_pll[7:0]
        0x0D,        0x07,  //cln_cnt_pll[15:8]
        0x0E,        0x00,  //functional configuration
        //0x4E,        0x00,  //trim_5[4:1] (bist_lane_trim)
        0x7F,        0xFF,  //Reset
    };
    if ( trim < DPHY_X2 || trim > DPHY_X20 )
    {
        while(1);   //invalid trim
    }
    for ( uint8_t ui8Index = 0; ui8Index < (CMD_NUM << 1); ui8Index += 2 )
    {
        //
        // Register 0x1C and 0x1E control DPHY frequency.
        //
        if (ui32Instr_Data[ui8Index] == 0x1C)
        {
            if (trim % 2)
            {
                ui32Instr_Data[ui8Index + 1] = 0x40 | (trim / 2);
            }
            else
            {
                ui32Instr_Data[ui8Index + 1] = (trim / 2);
            }
        }

        if (ui32Instr_Data[ui8Index] == 0x1E)
        {
            if (trim < DPHY_X11)
            {
                ui32Instr_Data[ui8Index + 1] = 0x38;
            }
            else
            {
                ui32Instr_Data[ui8Index + 1] = 0x34;
            }
        }

        ui32Status = am_device_command_write(&gAmDphy,
                                            1,
                                            ui32Instr_Data[ui8Index],
                                            &(ui32Instr_Data[ui8Index + 1]),
                                            1);

        if (ui32Status != AM_HAL_STATUS_SUCCESS)
        {
            //
            // The layer above this one doesn't understand IOM errors, so we
            // will intercept and rename it here.
            //
            break;
        }
        am_util_delay_ms(10);
    }

    ui32Status = am_device_command_read(&gAmDphy,
                                        1,
                                        0xA0,
                                        &ui32ReadData,
                                        1);
    return ui32Status;
}

//*****************************************************************************
//
// This ranges from address 0x00 to 0x53
//
//*****************************************************************************
static uint32_t
dphy_read_rw_reg(uint32_t ui32Addr)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32ReadData = 0;

    ui32Status = am_device_command_read(&gAmDphy,
                                        1,
                                        ui32Addr | 0x80,
                                        &ui32ReadData,
                                        1);

    return ui32ReadData;
}

uint32_t
dphy_write_reg(uint32_t ui32Addr, uint32_t ui32OutData)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;

    ui32Status = am_device_command_write(&gAmDphy,
                                    1,
                                    ui32Addr,
                                    &ui32OutData,
                                    1);

    return ui32Status;
}

//*****************************************************************************
//
// This ranges from address 0x7F00 to 0x7F3B.The read registers are addressed in
// two cycles.
//
//*****************************************************************************
static uint32_t
dphy_read_ro_reg(uint32_t ui32Addr)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32ReadData = 0;
    //
    // First a address buffer register(At address location 0x7E) is loaded with the
    // offset address ranging from 0x00 to 0x3B.
    //
    ui32Status = am_device_command_write(&gAmDphy,
                                        1,
                                        0x7E,
                                        &ui32Addr,
                                        1);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32Status;
    }
    am_util_delay_ms(10);
    //
    // And to access the register content a read operation is performed with the register
    // address of 0x7F.
    //
    ui32Status = am_device_command_read(&gAmDphy,
                                        1,
                                        0x7F | 0x80,
                                        &ui32ReadData,
                                        1);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32Status;
    }
    return ui32ReadData;
}

//
// This configuration is compatible with previous example or testcases.
//
uint32_t
dphy_init()
{

    return dphy_init_with_para(DPHY_X8);
}

uint32_t
dphy_init_with_para(dphy_trim_t trim)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;

    ui32Status = iom_init(IOM_0, &g_IomDevHandle);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32Status;
    }
    ui32Status = dphy_cfg(g_IomDevHandle, trim);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32Status;
    }
    ui32Status = iom_deinit(g_IomDevHandle);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return ui32Status;
    }
    return ui32Status;
}
