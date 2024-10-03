//*****************************************************************************
//
//! @file mspi_nand_flash_example.c
//!
//! @brief Example of the MSPI operation with Quad SPI NAND Flash.
//!
//! Purpose: This example configures an MSPI connected NAND flash device in Quad mode
//! and performs various operations - verifying the correctness of the same
//! Operations include - Erase, Write, Read.
//!
//! Starting from apollo5b, the MSPI XIP read bandwidth is boosted by the
//! ARM CM55 cache and the MSPI CPU read combine feature. By default,
//! the CPU read queue is on(CPURQEN). Cache prefetch(RID3) and cache miss(RID2)
//! requests deemed appropriate by MSPI hardware are combined and processed
//! with a 2:1 ratio(GQARB) between general queue and CPU read queue.
//!
//! am_hal_mspi_cpu_read_burst_t default =
//! {
//!     .eGQArbBais                         = AM_HAL_MSPI_GQARB_2_1_BAIS,
//!     .bCombineRID2CacheMissAccess        = true,
//!     .bCombineRID3CachePrefetchAccess    = true,
//!     .bCombineRID4ICacheAccess           = false,
//!     .bCPUReadQueueEnable                = true,
//! }
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//!
//! AM_DEBUG_PRINTF
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "mspi_nand_flash_example.h"
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

#define MSPI_TEST_MODULE            1    // 0 ~ 3
#define MSPI_TEST_CLKFREQ           AM_HAL_MSPI_CLK_96MHZ
#define NAND_TEST_PAGE              0x100
#define NAND_TEST_FLASH_CFG_MODE    1    // 0:serial ; 1:quad

#define TIMING_SCAN_CONFIG          1
//*****************************************************************************
//
// Variables Definition
//
//*****************************************************************************
void *g_FlashHdl;
void *g_MSPIHdl;

static char *modeinfo[] = {"SERIAL 1-1-1", "QUAD 1-1-4"};

static AM_SHARED_RW uint8_t write_data[AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE];
static AM_SHARED_RW uint8_t read_data[AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE];
static uint8_t write_oob[AM_DEVICES_MSPI_NAND_FLASH_OOB_SIZE];
static uint8_t read_oob[AM_DEVICES_MSPI_NAND_FLASH_OOB_SIZE];
static AM_SHARED_RW uint32_t DMATCBBuffer[512];

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
    MSPI3_IRQn,
};
mspi_device_func_t mspi_device_func =
{
#if defined(AM_BSP_MSPI_FLASH_DEVICE_DS35X1GA)
    .devName = "DS35X1GA",
    .mspi_init = am_devices_mspi_ds35x1ga_init,
    .mspi_init_timing_check = am_devices_mspi_ds35x1ga_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_ds35x1ga_apply_sdr_timing,
    .mspi_term = am_devices_mspi_ds35x1ga_deinit,
    .mspi_read_id = am_devices_mspi_ds35x1ga_id,
    .mspi_read = am_devices_mspi_ds35x1ga_read,
    .mspi_write = am_devices_mspi_ds35x1ga_write,
    .mspi_block_erase = am_devices_mspi_ds35x1ga_block_erase,

#elif defined(AM_BSP_MSPI_FLASH_DEVICE_TC58CYG0)
    .devName = "TC58CYG0",
    .mspi_init = am_devices_mspi_tc58cyg0_init,
    .mspi_init_timing_check = am_devices_mspi_tc58cyg0_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_tc58cyg0_apply_sdr_timing,
    .mspi_term = am_devices_mspi_tc58cyg0_deinit,
    .mspi_read_id = am_devices_mspi_tc58cyg0_id,
    .mspi_read = am_devices_mspi_tc58cyg0_read,
    .mspi_write = am_devices_mspi_tc58cyg0_write,
    .mspi_block_erase = am_devices_mspi_tc58cyg0_block_erase,
#elif defined(AM_BSP_MSPI_FLASH_DEVICE_W25N02KW)
    .devName = "W25N02KW",
    .mspi_init = am_devices_mspi_w25n02kw_init,
    .mspi_init_timing_check = am_devices_mspi_w25n02kw_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_w25n02kw_apply_sdr_timing,
    .mspi_term = am_devices_mspi_w25n02kw_deinit,
    .mspi_read_id = am_devices_mspi_w25n02kw_id,
    .mspi_read = am_devices_mspi_w25n02kw_read,
    .mspi_write = am_devices_mspi_w25n02kw_write,
    .mspi_block_erase = am_devices_mspi_w25n02kw_block_erase,
#else
#error "Unknown MSPI Device"
#endif
};

am_hal_mpu_region_config_t sMPUCfg =
{
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)DMATCBBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)DMATCBBuffer + sizeof(DMATCBBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
};
am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .sInnerAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .eDeviceAttr = 0,
};

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************

#define flash_mspi_isr                                                         \
    am_mspi_isr1(MSPI_TEST_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi##n##_isr

void flash_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPIHdl, &ui32Status, false);
    am_hal_mspi_interrupt_clear(g_MSPIHdl, ui32Status);
    am_hal_mspi_interrupt_service(g_MSPIHdl, ui32Status);
}

const am_abstract_mspi_devices_config_t MSPI_Serial_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .eClockFreq = MSPI_TEST_CLKFREQ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer) / sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};

const am_abstract_mspi_devices_config_t MSPI_Quad_Flash_Config =
{
    .eDeviceConfig = AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4,
    .eClockFreq = MSPI_TEST_CLKFREQ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer) / sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0,
    .ui32ScramblingEndAddr = 0,
};

am_abstract_mspi_devices_config_t const * MSPI_Flash_Configs[2] =
    { &MSPI_Serial_Flash_Config, &MSPI_Quad_Flash_Config};

//*****************************************************************************
//
// Set up MSPI clock for the device.
//
//*****************************************************************************
const char* clkfreq_string(am_hal_mspi_clock_e clk)
{
    switch(clk)
    {
        case AM_HAL_MSPI_CLK_96MHZ:
            return "96";
        case AM_HAL_MSPI_CLK_48MHZ:
            return "48";
        case AM_HAL_MSPI_CLK_32MHZ:
            return "32";
        case AM_HAL_MSPI_CLK_24MHZ:
            return "24";
        case AM_HAL_MSPI_CLK_16MHZ:
            return "16";
        case AM_HAL_MSPI_CLK_12MHZ:
            return "12";
        case AM_HAL_MSPI_CLK_8MHZ:
            return "8";
        case AM_HAL_MSPI_CLK_6MHZ:
            return "6";
        case AM_HAL_MSPI_CLK_4MHZ:
            return "4";
        case AM_HAL_MSPI_CLK_3MHZ:
            return "3";
        default:
            return "NULL";
    }
}
am_abstract_mspi_devices_timing_config_t MSPISdrTimingConfig;
//*****************************************************************************
//
// Main
//
//*****************************************************************************
int main(void)
{
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);
    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttr, 1);
    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();
    //
    // Set up the regions.
    //
    am_hal_mpu_region_configure(&sMPUCfg, 1);
    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);
    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo5 MSPI NAND Flash Driver example @ %s MHz,  Mode: %s !\n\n",
        clkfreq_string(MSPI_Flash_Configs[NAND_TEST_FLASH_CFG_MODE]->eClockFreq),
        modeinfo[NAND_TEST_FLASH_CFG_MODE]);

    //
    // Print the device info.
    //
    am_util_id_t sIdDevice;
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
    am_util_stdio_printf("Device type: %s\n", sIdDevice.pui8DeviceName);
    am_util_stdio_printf("Nand Flash: %s\n", mspi_device_func.devName);

    uint32_t ui32Status;
    uint8_t ui8EccStatus;

    //
    // fill the test page with some data
    //
    for (uint32_t i = 0; i < AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE; i++)
    {
        write_data[i] = i;
    }

    //
    // fill the oob area with zero
    //
    memset(write_oob, 0x0, sizeof(write_oob));

#if TIMING_SCAN_CONFIG
    am_util_stdio_printf("Starting MSPI SDR Timing Scan for MSPI%d: \n", MSPI_TEST_MODULE);
    if ( AM_ABSTRACT_MSPI_SUCCESS == mspi_device_func.mspi_init_timing_check(MSPI_TEST_MODULE, MSPI_Flash_Configs[NAND_TEST_FLASH_CFG_MODE], &MSPISdrTimingConfig) )
    {
        am_util_debug_printf("Scan Result: TXNEG = %d \n", MSPISdrTimingConfig.bTxNeg);
        am_util_debug_printf("             RXNEG = %d \n", MSPISdrTimingConfig.bRxNeg);
        am_util_debug_printf("             RXCAP = %d \n", MSPISdrTimingConfig.bRxCap);
        am_util_debug_printf("             TURNAROUND = %d \n", MSPISdrTimingConfig.ui8Turnaround);
        am_util_debug_printf("             TXDQSDELAY = %d \n", MSPISdrTimingConfig.ui8TxDQSDelay);
        am_util_debug_printf("             RXDQSDELAY = %d \n", MSPISdrTimingConfig.ui8RxDQSDelay)
    }
    else
    {
        am_util_stdio_printf("Scan Result: Failed, no valid setting.  \n");
        return -1;
    }
#endif

    ui32Status = mspi_device_func.mspi_init(MSPI_TEST_MODULE, MSPI_Flash_Configs[NAND_TEST_FLASH_CFG_MODE], &g_FlashHdl, &g_MSPIHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and Flash Device correctly!\n");
        while(1);
    }
    else
    {
        am_util_stdio_printf("Configure the MSPI and Flash Device correctly!\n");
    }

    NVIC_SetPriority(mspi_interrupts[MSPI_TEST_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_TEST_MODULE]);
    am_hal_interrupt_master_enable();

#if TIMING_SCAN_CONFIG
    //
    //  Set the SDR timing from previous scan.
    //
    mspi_device_func.mspi_init_timing_apply(g_FlashHdl, &MSPISdrTimingConfig);
#endif

    //
    //  Read Device ID
    //
    uint32_t ui32DeviceID;
    mspi_device_func.mspi_read_id(g_FlashHdl, &ui32DeviceID);
    am_util_stdio_printf("NAND flash ID is 0x%x!\n", ui32DeviceID);

    //
    //  Erase the designated block
    //
    ui32Status = mspi_device_func.mspi_block_erase(g_FlashHdl, NAND_TEST_PAGE >> 6);
    am_util_stdio_printf("block %d erase status 0x%x!\n", NAND_TEST_PAGE >> 6, ui32Status);

    //
    //  Write a full page
    //
    ui32Status = mspi_device_func.mspi_write(g_FlashHdl, NAND_TEST_PAGE, &write_data[0], AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE,
                                   &write_oob[0], AM_DEVICES_MSPI_NAND_FLASH_OOB_SIZE);
    am_util_stdio_printf("page %d write status 0x%x!\n", NAND_TEST_PAGE, ui32Status);

    //
    //  Read a full page
    //
    memset(read_data, 0x0, AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE);
    ui32Status = mspi_device_func.mspi_read(g_FlashHdl, NAND_TEST_PAGE, &read_data[0], AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE,
                                               &read_oob[0], 16, &ui8EccStatus);
    am_util_stdio_printf("page %d read status 0x%x!\n", NAND_TEST_PAGE, ui32Status);
    am_util_stdio_printf("page %d read ECC status 0x%x!\n", NAND_TEST_PAGE, ui8EccStatus);

    //
    // Compare the buffers
    //
    uint32_t i;
    for (i = 0; i < AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE; i++)
    {
        if (write_data[i] != read_data[i])
        {
            am_util_stdio_printf("%d : data_check wrong, expected: 0x%x, readback: 0x%x!\n", i, write_data[i], read_data[i]);
            break;
        }
    }

    if (i == AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE)
    {
        am_util_stdio_printf("Write and read buffer matched!\n");
    }
    else
    {
        am_util_stdio_printf("first data wrong is at index %d\n", i);
        for (i = 0;  i < AM_DEVICES_MSPI_NAND_FLASH_PAGE_SIZE; i++)
        {
            am_util_stdio_printf("%d : write data: 0x%x, readback: 0x%x!\n", i, write_data[i], read_data[i]);
        }
    }

    //
    //  End banner.
    //
    am_util_stdio_printf("\nApollo5 MSPI Nand Flash Example Complete\n");

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
        //
        // Go to Deep Sleep.
        //
        //am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}
