//*****************************************************************************
//
//! @file app_task.c
//!
//! @brief A simple CMD8 to get ext csd example.
//!
//! This file contains the kernel application code for interfacing with the MMC
//! driver to Read Ext CSD
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "nex_api.h"

#include "am_bsp.h"
#include "am_mcu_apollo.h"
#include "am_util.h"

#define APP_PRINT(format, ...)  am_util_stdio_printf("[APP] %33s %04d | " format, __func__, __LINE__, ##__VA_ARGS__)

#define LENGTH 512

#define SDHC   0
#define SLOT   0

#define ALIGN(x) __attribute__((aligned(1 << x)))

//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
TaskHandle_t app_task_handle;

static SemaphoreHandle_t comp;

static SD_COMMAND cmd;
static SD_COMMAND_DATA cmd_data;
static NEX_MMC_EXT_CSD ext_csd;

static uint8_t  buffer[LENGTH];
static uint32_t resp[4];

void am_sdio_isr(void)
{
    nex_irqhandler(SDHC, SLOT);
}

//
// PIO, SDMA, ADMA2 mode setting
//

typedef enum
{
    NEX_XFER_PIO,
    NEX_XFER_DMA,
    NEX_XFER_ADMA2,
} xfer_mode_e;

static void set_block_rw_mode(xfer_mode_e mode)
{
    struct sdio_bus_specific_info businfo;

    GetBusSpecificInformation(&businfo, SDHC, SLOT);
    switch (mode)
    {
        case 0:  //PIO
            businfo.dma_enable_disable = 0;  // = 0 for PIO and 1 for DMA and ADMA
            businfo.dma_type = 0;            // = 0 for PIO, 0 for DMA and 1 for ADMA
            break;
        case 1:  //DMA
            businfo.dma_enable_disable = 1;
            businfo.dma_type = 0;
            break;
        case 2:  //ADMA2
            businfo.dma_enable_disable = 1;
            businfo.dma_type = 1;
            break;
        default: //DMA - default
            businfo.dma_enable_disable = 1;
            businfo.dma_type = 0;
            break;
    }
    SetBusSpecificInformation(&businfo, SDHC, SLOT);
}

static void user_function(SD_CALL_BACK_INFO *a)
{
    APP_PRINT("\n");
    APP_PRINT("CMD8 is completed\n");
    APP_PRINT("Card Status : %d\n", a->card_status);

    resp[0] = a->cmd_resp[0];
    resp[1] = a->cmd_resp[1];
    resp[2] = a->cmd_resp[2];
    resp[3] = a->cmd_resp[3];

    APP_PRINT("Card Resp[0]: 0x%x\n", resp[0]);

    xSemaphoreGive(comp);
}

void app_mmc_send_extcsd_task(void *pvParameters)
{
    while (IsCardInitialized(SDHC, SLOT) == SD_FALSE)
    {
        APP_PRINT("Card is not ready ... \n");
        vTaskDelay(pdMS_TO_TICKS(10));
        APP_PRINT("check again\n");
    }

    APP_PRINT("\n");
    APP_PRINT("Card is ready\n");

    memset(&cmd, 0, sizeof(SD_COMMAND));
    memset(&ext_csd, 0, sizeof(NEX_MMC_EXT_CSD));

    memset(buffer, 0, LENGTH);

    cmd.cmd_data = &cmd_data;
    memset(cmd.cmd_data, 0, sizeof(SD_COMMAND_DATA));

    cmd.sdhcNum = SDHC;
    cmd.slotNum = SLOT;
    cmd.command_opcode = MMC_READ_EXT_CSD;
    cmd.command_arg = 0x0;

    cmd.response_type = MMC_RSP_R1 | MMC_CMD_ADTC;
    cmd.cmd_type = SD_CMD_DATA;
    cmd.cmd_data->buffer = buffer;
    cmd.cmd_data->length = LENGTH;
    cmd.cmd_data->no_of_blocks = 1;
    cmd.cmd_data->blocksize = 512;

    cmd.dir_flag = MMC_READ_DATA;

    RegisterISRCallBack(user_function, SDHC, SLOT);
    comp = xSemaphoreCreateBinary();

    APP_PRINT("\n");
    APP_PRINT("Send the CMD8 to get the EXT CSD information\n");

    set_block_rw_mode(NEX_XFER_PIO);
    SendCommand(&cmd);
    xSemaphoreTake(comp, portMAX_DELAY);
    set_block_rw_mode(NEX_XFER_DMA);

    ext_csd.bus_width     = buffer[183];    //bit1 183
    ext_csd.hs_timing     = buffer[185];    //bit1 185
    ext_csd.power_class   = buffer[187];    //bit1 187
    ext_csd.cmd_set_rev   = buffer[189];    //bit1 189
    ext_csd.ext_csd_rev   = buffer[192];    //1bit 192
    ext_csd.csd_structure = buffer[194];    //1bit 194
    ext_csd.pwr_cl_26_360 = buffer[203];    //1bit 203
    ext_csd.pwr_cl_52_360 = buffer[202];    //1bit 202
    ext_csd.pwr_cl_26_195 = buffer[201];    //1bit 201
    ext_csd.pwr_cl_52_195 = buffer[200];    //1bit 200
    ext_csd.card_type     = buffer[196];    //1bit 196

    APP_PRINT("\n");
    APP_PRINT("========== Important information of the CSD ==========\n");
    APP_PRINT("byte [183] is bus width mode        : [0x%x]\n", ext_csd.bus_width);
    APP_PRINT("byte [185] is High Speed            : [0x%x]\n", ext_csd.hs_timing);
    APP_PRINT("byte [187] is power class           : [0x%x]\n", ext_csd.power_class);
    APP_PRINT("byte [191] is Cmd Set               : [0x%x]\n", ext_csd.cmd_set_rev);
    APP_PRINT("byte [192] is Ext CSD Rev           : [0x%x]\n", ext_csd.ext_csd_rev);
    APP_PRINT("byte [194] is CSD Structure         : [0x%x]\n", ext_csd.csd_structure);
    APP_PRINT("byte [196] is Card Type             : [0x%x]\n", ext_csd.card_type);
    APP_PRINT("byte [200] is pwr class 52_195      : [0x%x]\n", ext_csd.pwr_cl_52_195);
    APP_PRINT("byte [201] is pwr class 26_195      : [0x%x]\n", ext_csd.pwr_cl_26_195);
    APP_PRINT("byte [202] is pwr class 52_360      : [0x%x]\n", ext_csd.pwr_cl_52_360);
    APP_PRINT("byte [203] is pwr class 26_360      : [0x%x]\n", ext_csd.pwr_cl_26_360);

    APP_PRINT("\n");
    APP_PRINT("app_mmc_send_extcsd_task is suspended\n");

    vTaskSuspend(NULL);
}



