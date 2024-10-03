//*****************************************************************************
//
//! @file am_widget_sdio.c
//!
//! @brief This widget allows test cases to exercise the Apollo4 SDIO module.
//!
//!
//!
//! @endverbatim
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdlib.h>
#include "am_mcu_apollo.h"
#include "am_widget_sdio.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_devices_emmc_rpmb.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Global data
//
//*****************************************************************************
am_hal_card_host_t *pSdhcCardHost = NULL;
am_hal_card_t eMMCard;

volatile bool bAsyncWriteIsDone = false;
volatile bool bAsyncReadIsDone  = false;

char *dummy_key_hash =
{
    "AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHH"
};

//*****************************************************************************
//
// Memory allocation.
//
//*****************************************************************************

//*****************************************************************************
//
// Interrupt handler for SDIO
//
//*****************************************************************************
void am_sdio_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(pSdhcCardHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(pSdhcCardHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(pSdhcCardHost->pHandle, ui32IntStatus);
}

//
// Register SD Card power cycle function for card power on/off/reset
//
uint32_t am_widget_sd_card_power_config(am_hal_card_pwr_e eCardPwr)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_POWER_CTRL, am_hal_gpio_pincfg_output);

    if ( eCardPwr == AM_HAL_CARD_PWR_CYCLE )
    {
        //
        // SD Card power cycle or power on
        //
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_POWER_CTRL);
        am_util_delay_ms(20);
        am_hal_gpio_output_set(AM_BSP_GPIO_SD_POWER_CTRL);

        //
        // wait until the power supply is stable
        //
        am_util_delay_ms(20);

#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#endif
    }
    else if ( eCardPwr == AM_HAL_CARD_PWR_OFF )
    {
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_POWER_CTRL);

#ifdef SD_CARD_BOARD_SUPPORT_1_8_V
        am_hal_gpio_output_clear(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#endif
    }
    else if ( eCardPwr == AM_HAL_CARD_PWR_SWITCH )
    {
        //
        // set level shifter to 1.8V
        //
        am_hal_gpio_output_set(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
        am_util_delay_ms(20);
    }

    return true;
}

//*****************************************************************************
//
// Widget Setup Function.
//
//*****************************************************************************
bool am_widget_sdio_setup(void)
{
    uint32_t    i = 10;

    //
    // Get the SDHC card host instance
    //
    pSdhcCardHost = am_hal_get_card_host(AM_HAL_SDHC_CARD_HOST, true);

    if (pSdhcCardHost == NULL)
    {
        am_util_stdio_printf("No such card host !!!\n");
        am_util_stdio_printf("SDIO Test Fail !!!\n");
        return false;
    }

    am_util_stdio_printf("card host is found\n");

    //
    // check if card is present
    //
    while (am_hal_card_host_find_card(pSdhcCardHost, &eMMCard) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("No card is present now\n");
        am_util_delay_ms(1000);
        am_util_stdio_printf("Checking if card is available again\n");
        i--;
        if(i == 0)
        {
          return false;
        }
    }

    //
    // Init the card
    //
    i = 10;
    while (am_hal_card_init(&eMMCard, AM_HAL_CARD_TYPE_UNKNOWN, am_widget_sd_card_power_config, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_delay_ms(1000);
        am_util_stdio_printf("card and host is not ready, try again\n");
        i--;
        if(i == 0)
        {
          return false;
        }
    }

    // Return the result.
    return true;

}

//*****************************************************************************
//
// Widget Cleanup Function.
//
//*****************************************************************************
bool am_widget_sdio_cleanup(void)
{
  uint32_t      ui32Status;

  ui32Status = am_hal_card_deinit(&eMMCard);
  if(ui32Status != AM_HAL_STATUS_SUCCESS)
  {
    am_util_stdio_printf("card deinit fail\n");
    return false;
  }

  // Return the result.
  return true;
}

//*****************************************************************************
//
// Widget Test Execution Functions
//
//*****************************************************************************

bool am_widget_check_data_match(uint8_t *pui8RdBuf, uint8_t *pui8WrBuf, uint32_t ui32Len)
{
    uint32_t i;
    for (i = 0; i < ui32Len; i++)
    {
        if (pui8RdBuf[i] != pui8WrBuf[i])
        {
            am_util_stdio_printf("\npui8RdBuf[%d] = %d and pui8WrBuf[%d] = %d\n", i, pui8RdBuf[i], i, pui8WrBuf[i]);
            return false;
        }
    }

    if (i == ui32Len)
    {
        am_util_stdio_printf("data matched\n");
    }

    return true;
}

void am_hal_card_event_test_cb(am_hal_host_evt_t *pEvt)
{
    am_hal_card_host_t *pHost = (am_hal_card_host_t *)pEvt->pCtx;

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        bAsyncReadIsDone = true;
        am_util_debug_printf("Last Read Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_XFER_COMPLETE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        bAsyncWriteIsDone = true;
        am_util_debug_printf("Last Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_SDMA_DONE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_READ)
    {
        am_util_debug_printf("SDMA Read Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_SDMA_DONE == pEvt->eType &&
        pHost->AsyncCmdData.dir == AM_HAL_DATA_DIR_WRITE)
    {
        am_util_debug_printf("SDMA Write Xfered block %d\n", pEvt->ui32BlkCnt);
    }

    if (AM_HAL_EVT_CARD_PRESENT == pEvt->eType)
    {
        am_util_debug_printf("A card is inserted\n");
    }
}

//*****************************************************************************
//
// Widget prepare test data pattern.
//
//*****************************************************************************
bool am_widget_prepare_data_pattern(uint32_t pattern_index, uint8_t* buff, uint32_t len)
{
    uint32_t *pui32TxPtr = (uint32_t*)buff;
    uint8_t  *pui8TxPtr  = (uint8_t*)buff;

    switch ( pattern_index )
    {
        case 0:
            // 0x5555AAAA
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0x5555AAAA);
            }
            break;
        case 1:
            // 0xFFFF0000
            for (uint32_t i = 0; i < len / 4; i++)
            {
               pui32TxPtr[i] = (0xFFFF0000);
            }
            break;
        case 2:
            // walking
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = 0x01 << (i % 8);
            }
            break;
        case 3:
            // incremental from 1
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i + 1) & 0xFF);
            }
            break;
        case 4:
            // decremental from 0xff
            for ( uint32_t i = 0; i < len; i++ )
            {
                // decrement starting from 0xff
                pui8TxPtr[i] = (0xff - i) & 0xFF;
            }
            break;
        default:
            // incremental from 1
            for (uint32_t i = 0; i < len; i++)
            {
               pui8TxPtr[i] = ((i ) & 0xFF);
            }
            break;
    }

    return true;
}
bool am_widget_set_key(void)
{
    uint32_t ui32Stat;

    //
    //switch to rpmb partition
    //
    if( am_hal_card_get_ext_csd_field(&eMMCard, MMC_EXT_REGS_PARTITON_CONFIG, 1) != AM_DEVICES_EMMC_RPMB_ACCESS )
    {
        if ( am_devices_emmc_rpmb_partition_switch(&eMMCard, AM_DEVICES_EMMC_RPMB_ACCESS) != AM_DEVICES_EMMC_RPMB_STATUS_SUCCESS )
        {
            am_util_stdio_printf("eMMC switch partition failed\n");
            return false;
        }
    }

    //
    //set 256bit key to emmc
    //
    ui32Stat = am_devices_emmc_rpmb_set_key(&eMMCard, (uint8_t *)dummy_key_hash);
    if (ui32Stat != AM_DEVICES_EMMC_RPMB_STATUS_SUCCESS)
    {
        am_util_stdio_printf("eMMC set key failed, Stat: %d\n", ui32Stat);
        return false;
    }

    return true;
}

//*****************************************************************************
//
// Check secure key in rpm mode
//
//*****************************************************************************
bool am_widget_check_key(void)
{
    uint32_t ui32Writecnt;
    uint32_t ui32Status;

    //
    //switch to rpmb partition
    //
    if( am_hal_card_get_ext_csd_field(&eMMCard, MMC_EXT_REGS_PARTITON_CONFIG, 1) != AM_DEVICES_EMMC_RPMB_ACCESS )
    {
        if ( am_devices_emmc_rpmb_partition_switch(&eMMCard, AM_DEVICES_EMMC_RPMB_ACCESS) != AM_DEVICES_EMMC_RPMB_STATUS_SUCCESS )
        {
            am_util_stdio_printf("eMMC switch partition failed\n");
            return false;
        }
    }

    //
    //get counter
    //
    ui32Status= am_devices_emmc_rpmb_get_counter(&eMMCard, &ui32Writecnt, (uint8_t *)dummy_key_hash);
    if ( ui32Status == AM_DEVICES_EMMC_RPMB_STATUS_KEY_NOT_PROGRAMMED_ERROR)
    {
        ui32Status = am_widget_set_key();
        if( ui32Status != true)
        {
            return false;
        } 
    }
    else if (ui32Status == AM_DEVICES_EMMC_RPMB_STATUS_SUCCESS )
    {
        am_util_stdio_printf("eMMC writer counter: %d\n", ui32Writecnt);
    }
    else
    {
        am_util_stdio_printf("eMMC failed to get write counter, Stat:%d\n", ui32Status);
        return false;
    }

    return true;
}

