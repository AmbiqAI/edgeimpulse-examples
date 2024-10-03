//*****************************************************************************
//
//! @file test_sdio_emmc.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "factory_test_helpers.h"


#define ALIGN(x) __attribute__((aligned(1 << x)))

#define START_BLK 3000
#define BLK_NUM 4
#define BUF_LEN 512*BLK_NUM

volatile uint8_t ui8RdBuf[BUF_LEN] ALIGN(12);
volatile uint8_t ui8WrBuf[BUF_LEN] ALIGN(12);

static am_hal_card_host_t *pSdhcCardHost = NULL;
static am_hal_card_t eMMCard;

int emmc_test(uint32_t mode)
{
    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(AM_HAL_HOST_BUS_WIDTH_4);


    //
    // Get the uderlying SDHC card host instance
    //
    pSdhcCardHost = am_hal_get_card_host(AM_HAL_SDHC_CARD_HOST, true);
    if (pSdhcCardHost == NULL)
    {
        am_util_debug_printf("No such card host and stop\n");
        goto _fail;
    }

    am_util_debug_printf("card host is found\n");

    //
    // check if card is present
    //
    if ( am_hal_card_host_find_card(pSdhcCardHost, &eMMCard) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_debug_printf("No card is present now\n");
        goto _fail;
    }

    if (am_hal_card_init(&eMMCard, AM_HAL_CARD_TYPE_EMMC, NULL, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_debug_printf("Failed to initialize the card\n");
        goto _fail;
    }

    //
    // set the card type to eMMC by using 48MHz and 4-bit mode for read and write
    //
    if (am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
        AM_HAL_HOST_BUS_WIDTH_4, 48000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
        AM_HAL_HOST_UHS_NONE) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_debug_printf("Failed to config 48MHz and 4-bit mode\n");
        goto _fail;
    }

    //
    // initialize the test read and write buffers
    //
    for (int i = 0; i < BUF_LEN; i++)
    {
        ui8WrBuf[i] = i % 256;
    }

    //
    // write 512 bytes to emmc flash
    //
    am_hal_card_block_write_sync(&eMMCard, START_BLK, 1, (uint8_t *)ui8WrBuf);
      //
    // read back the first block of emmc flash
    //
    for (int i = 0; i < BUF_LEN; i++)
    {
        ui8RdBuf[i] = 0x0;
    }


    am_hal_card_block_read_sync(&eMMCard, START_BLK, 1, (uint8_t *)ui8RdBuf);

    //
    // check if block data match or not
    //
    if ( memory_compare( (const void *)ui8RdBuf, (const void *)ui8WrBuf, 512 ) != 0 )
    {
       goto _fail;
    }

    return 0;

_fail:
    return -1;
}
