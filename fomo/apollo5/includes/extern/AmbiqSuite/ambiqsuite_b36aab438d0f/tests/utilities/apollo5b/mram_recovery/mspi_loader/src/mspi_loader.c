//*****************************************************************************
//
//! @file mspi_loader.c
//!
//! @brief Loads a buffer preloaded from the debugger into an MSPI device
//! @{
//! Purpose: This example configures an MSPI connected flash device in Octal SDR
//! mode and loads contents preloaded into SSRAM by the debugger into MSPI
//!
//! Before executing, the user loads parameters into SSRAM configuring the
//! MSPI device, the desired flash location of metadata and recovery, the size
//! of the recovery, and the location of the recovery in SSRAM. In addition,
//! a signalture of 0xABCD1234 must be the first parameter to prevent
//! inadvertent execution.
//!
//! The test will erase, write, and read-verify one sector at a time. If a
//! sector already contains the data to be written, it is skipped.
//!
//! Non-compare errors are logged by breakpointing the debugger and returning a
//! code in the metadata and recovery errorcount registers. All errors are
//! greater than 0xF0000000 so they can be distinguished from compare error
//! counts.
//!
//! Printing takes place over the ITM at 1M Baud.
//!
//! See the attached mspi_load.jlink for a corresponding  JLink CommandScript
//! example.
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
#include "mspi_loader.h"

#define MSPI_TARGET_SECTOR      (0)
#define MSPI_BUFFER_SIZE        (1<<AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT)

#define DEFAULT_TIMEOUT         10000

//
// Define the location of the recovery information structure (defined below).
//
#define PRGM_MSPI_DATA_ADDR             0x200FF000

//
// Define the structure of the data located at PRGM_EMMC_DATA_ADDR
//
#define CFGDATA_SIGNATURE       pMspiCfgData[0] // Signature, must be 0xABCD1234
#define CFGDATA_WRV             pMspiCfgData[1] // Non-zero=write/read/verify. 0=read/verify only.
#define CFGDATA_MSPI_DEV        pMspiCfgData[2] // MSPI device number (byte 0) and chip select (byte 1)
#define CFGDATA_META_DATAOFFSET pMspiCfgData[3] // Metadata: data_offset - MSPI offset for the recovery image
#define CFGDATA_META_DATASIZE   pMspiCfgData[4] // Metadata: data_size - size of the recovery image in bytes
#define CFGDATA_META_OFFSET     pMspiCfgData[5] // MSPI offset where the above metadata is stored
#define CFGDATA_RECOVERY_ADDR   pMspiCfgData[6] // Address of the recovery image
#define RESULT_METADATA_ERRCNT  pMspiCfgData[7] // Result of writing the metadata to the MSPI device
#define RESULT_RECOVERY_ERRCNT  pMspiCfgData[8] // Result of writing the recovery image to the eMMC device

#define CFGDATA_META_SIZE 16

//
// Macro for BKPT instruction
//
#if (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6000000)) || defined(__GNUC_STDC_INLINE__)
#define     AM_ASM_BKPT(n)  __asm("    bkpt "#n)
#elif defined(__IAR_SYSTEMS_ICC__)
#define     AM_ASM_BKPT(n)  asm("    bkpt "#n)
#else
#error Compiler unknown, cannot define BKPT instruction.
#endif

AM_SHARED_RW uint32_t       DMATCBBuffer[2560];
AM_SHARED_RW uint8_t        g_SectorBuffer[MSPI_BUFFER_SIZE] __attribute__((aligned(32)));
AM_SHARED_RW uint8_t g_MetadataBuffer[CFGDATA_META_SIZE] __attribute__((aligned(32)));

/* Reserve 2.5MB of SSRAM so that 0x20090000-0x20300000 can
 * be mapped to fixed addresses. That way we don't have to
 * mess with the linker file generator script */
AM_SHARED_RW uint8_t        g_PayloadReservation[1024*512*5];
uint32_t*                   pMspiCfgData = (uint32_t*)(PRGM_MSPI_DATA_ADDR);

#ifdef APOLLO5_FPGA
#define MSPI_TEST_FREQ          AM_HAL_MSPI_CLK_96MHZ
#else
#define MSPI_TEST_FREQ          AM_HAL_MSPI_CLK_48MHZ
#endif

void            *g_FlashHdl;
void            *g_MSPIHdl;
extern am_hal_mspi_xip_config_t gXipConfig[];

const am_hal_mspi_clock_e eClockFreq = MSPI_TEST_FREQ ;

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

am_abstract_mspi_devices_config_t MSPI_Flash_Config =
{
#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0_1_8_8,
#else
    .eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0,
#endif

    .eClockFreq = MSPI_TEST_FREQ,
    .pNBTxnBuf = DMATCBBuffer,
    .ui32NBTxnBufLength = (sizeof(DMATCBBuffer) / sizeof(uint32_t)),
    .ui32ScramblingStartAddr = 0, // Note - these are ignored by the driver! See gXipConfig below
    .ui32ScramblingEndAddr = 0x10000000,
};

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
    MSPI3_IRQn,
};

//*****************************************************************************
//
// MSPI ISRs. Sure, map all of them because we choose 1 at runtime
//
//*****************************************************************************
void am_mspi0_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPIHdl, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPIHdl, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPIHdl, ui32Status);
}

void am_mspi1_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPIHdl, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPIHdl, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPIHdl, ui32Status);
}

void am_mspi2_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPIHdl, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPIHdl, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPIHdl, ui32Status);
}

void am_mspi3_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPIHdl, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPIHdl, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPIHdl, ui32Status);
}

am_abstract_mspi_devices_timing_config_t MSPISdrTimingConfig;

mspi_device_func_t mspi_device_func =
{
#if defined(AM_BSP_MSPI_FLASH_DEVICE_IS25WX064)
    .devName = "MSPI FLASH IS25WX064",
    .mspi_init = am_devices_mspi_is25wx064_init,
    .mspi_init_timing_check = am_devices_mspi_is25wx064_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_is25wx064_apply_ddr_timing,
    .mspi_term = am_devices_mspi_is25wx064_deinit,
    .mspi_read_id = am_devices_mspi_is25wx064_id,
    .mspi_read = am_devices_mspi_is25wx064_read,
    .mspi_read_adv = am_devices_mspi_is25wx064_read_adv,
// ##### INTERNAL BEGIN #####
    .mspi_read_callback = am_devices_mspi_is25wx064_read_callback,
// ##### INTERNAL END #####
    .mspi_write = am_devices_mspi_is25wx064_write,
    .mspi_mass_erase = am_devices_mspi_is25wx064_mass_erase,
    .mspi_sector_erase = am_devices_mspi_is25wx064_sector_erase,
    .mspi_xip_enable = am_devices_mspi_is25wx064_enable_xip,
    .mspi_xip_disable = am_devices_mspi_is25wx064_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_is25wx064_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_is25wx064_disable_scrambling,
#else
    .devName = "MSPI FLASH ATXP032",
    .mspi_init = am_devices_mspi_atxp032_init,
    .mspi_init_timing_check = am_devices_mspi_atxp032_sdr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_atxp032_apply_sdr_timing,
    .mspi_term = am_devices_mspi_atxp032_deinit,
    .mspi_read_id = am_devices_mspi_atxp032_id,
    .mspi_read = am_devices_mspi_atxp032_read,
    .mspi_read_adv = am_devices_mspi_atxp032_read_adv,
// ##### INTERNAL BEGIN #####
    .mspi_read_callback = am_devices_mspi_atxp032_read_callback,
// ##### INTERNAL END #####
    .mspi_write = am_devices_mspi_atxp032_write,
    .mspi_mass_erase = am_devices_mspi_atxp032_mass_erase,
    .mspi_sector_erase = am_devices_mspi_atxp032_sector_erase,
    .mspi_xip_enable = am_devices_mspi_atxp032_enable_xip,
    .mspi_xip_disable = am_devices_mspi_atxp032_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_atxp032_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_atxp032_disable_scrambling,
#endif
};

/*
 * Erase, program, verify MSPI.
 *
 * If the contents of the buffer match what's already in flash, do nothing. If the buffer
 * does not span the entire sector, the remainder of the sector is preserved and written
 * back along with the buffer after erasing.
 *
 * buffer: pointer to a buffer to write to MSPI
 * flash_start: byte offset into the MSPI flash for the buffer contents
 * length: length of buffer in bytes
 * verify_only: If True, only reads the sector and compares against the buffer. No erase or write is performed
 *
 * Returns  0 if the write and verify (or verify if verify_only) are successful
 *          Number of bytes that failed to verify if not
 *         -1 if there was an error communicating with the MSPI device
 */
int
write_verify(uint8_t* buffer, uint32_t flash_start, uint32_t length, bool verify_only)
{
    uint32_t bad_bytes = 0;
    uint32_t sector, sector_start_offset, sector_length;
    uint8_t* local_buffer = buffer;
    uint32_t remaining = length;
    uint32_t flash_index = flash_start;
    uint32_t ui32Status;
    uint32_t compare_error = false;
    uint32_t already_programmed = true;

    while(remaining > 0)
    {
        sector = flash_index >> AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT;
        sector_start_offset = flash_index - (sector << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT);
        sector_length = (remaining > (1<< AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT) - sector_start_offset) ?
            (1<< AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT) - sector_start_offset:
            remaining;

        if (!verify_only)
        {
            // First check that the sector doesn't already contain the contents we expect
            ui32Status = mspi_device_func.mspi_read(g_FlashHdl, g_SectorBuffer, sector << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, 1<<AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, true);
            if ( AM_ABSTRACT_MSPI_SUCCESS != ui32Status )
            {
                am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
                return -1;
            }
            already_programmed = true;
            for (uint32_t i = 0; i < sector_length; i++)
            {
                if (g_SectorBuffer[i + sector_start_offset] != local_buffer[i])
                {
                    already_programmed = false;
                    break;
                }
            }
            if (!already_programmed)
            {
                // sector erase
                ui32Status = mspi_device_func.mspi_sector_erase(g_FlashHdl, sector << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT);
                if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
                {
                    am_util_stdio_printf("Failed to erase Flash Device sector %d!\n",sector);
                    return -1;
                }

                // Union previous sector contents with buffer
                for (uint32_t i = 0; i < sector_length; i++)
                {
                    g_SectorBuffer[i + sector_start_offset] = local_buffer[i];
                }

                // write the sector
                ui32Status = mspi_device_func.mspi_write(g_FlashHdl, g_SectorBuffer, sector << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, 1<<AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, true);
                if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
                {
                    am_util_stdio_printf("Failed to write buffer to Flash Device sector %d!\n", sector);
                    return -1;
                }
            }
        }

        if (!verify_only && already_programmed)
        {
            // don't repeat the read if we didn't write
            am_util_stdio_printf(".");
        }
        else
        {
            // verify
            ui32Status = mspi_device_func.mspi_read(g_FlashHdl, g_SectorBuffer, sector << AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, 1<<AM_DEVICES_MSPI_FLASH_SECTOR_SHIFT, true);
            if ( AM_ABSTRACT_MSPI_SUCCESS != ui32Status )
            {
                am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
                return -1;
            }
            for (uint32_t i = 0; i < sector_length; i++)
            {
                if (g_SectorBuffer[i + sector_start_offset] != local_buffer[i])
                {
                    compare_error = true;
                    bad_bytes++;
                }
            }
            if(compare_error)
            {
                am_util_stdio_printf("E");
            }
            else
            {
                am_util_stdio_printf("#");
            }
        }
        local_buffer += sector_length;
        flash_index += sector_length;
        remaining -= sector_length;
    }
    return bad_bytes;
}

//*****************************************************************************
//
// MSPI Example Main.
//
//*****************************************************************************
int
main(void)
{
    uint32_t        ui32Status;
    uint32_t        mspi_module;
    uint32_t        reset_gpio_num;
    bool            use_scrambling;

    am_hal_cachectrl_range_t sRange;

    RESULT_RECOVERY_ERRCNT = 0;
    RESULT_METADATA_ERRCNT = 0;
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache, D-Cache disable
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

    am_util_stdio_printf("Apollo5 MSPI Loader\n\n");


    //
    // Check the data to be programmed.
    //
    if ( CFGDATA_SIGNATURE != 0xABCD1234 )
    {
#ifdef AM_DEBUG_PRINTF
        am_util_stdio_printf("Error 9: MSPI configuration signature (0x%08X) incorrect, expected 0xABCD1234.\n", CFGDATA_SIGNATURE);
        am_util_stdio_printf("The data consists of 6 words and is expected to be located at 0x%08X.\n", PRGM_MSPI_DATA_ADDR);
        am_util_stdio_printf("  WD0: Signature. Must be 0xABCD1234.\n");
        am_util_stdio_printf("  WD1: 1 to skip writing and verify only\n");
        am_util_stdio_printf("  WD2: Byte 0: MSPI interface#  Byte 1: MSPI CS#  Byte 2: 1 for scrambling on\n");
        am_util_stdio_printf("  WD3: data_size   1st word of the metadata, in bytes.\n");
        am_util_stdio_printf("  WD4: data_offset 2nd word of the metadata, MSPI byte offset.\n");
        am_util_stdio_printf("  WD5: MSPI byte offset of the location of the metadata.\n");
        am_util_stdio_printf("  WD6: Address of the recovery image.\n");
        am_util_stdio_printf("  WD7: Result of writing the metadata to the MSPI device.\n");
        am_util_stdio_printf("  WD8: Result of writing the recovery image to the MSPI device.\n");
#endif // AM_DEBUG_PRINTF
        RESULT_METADATA_ERRCNT = 0xF000009;
        RESULT_RECOVERY_ERRCNT = 0xF000009;
        // NOTE: we don't breakpoint if the signature is bad. BKPT causes
        // the hardfault handler to trigger if no debugger is attached.
        // The only time this would be run without a correct signature is
        // if the user forgets to overwrite this in MRAM and resets the DUT
        while(1);
    }

    // Clear signature so we don't repeat flashing if we hit a reset
    CFGDATA_SIGNATURE = 0x00000000;
    //
    // Configure the MSPI and Flash Device.
    //

    mspi_module = CFGDATA_MSPI_DEV & 0x03;
    use_scrambling = (CFGDATA_MSPI_DEV & 0xFF0000) > 0;

    // Note: this is a workarond because the ATXP032 driver ignores the scrambling
    // settings in the config for Apollo4 and Apollo5 for some reason
    gXipConfig[mspi_module].scramblingStartAddr = 0;
    gXipConfig[mspi_module].scramblingEndAddr = 0x10000000;

    if ((CFGDATA_MSPI_DEV & 0xFF00) == 0)
    {
        MSPI_Flash_Config.eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE0;
        am_util_stdio_printf("  Targeting MSPI%01d CS0\n", mspi_module);
    }
    else
    {
        MSPI_Flash_Config.eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_CE1;
        am_util_stdio_printf("  Targeting MSPI%01d CS1\n", mspi_module);
    }

    //
    // Hard reset MSPI device via GPIO
    //

    switch(mspi_module)
    {
        case 1:
            reset_gpio_num = AM_BSP_GPIO_MSPI1_RST;
            break;
        case 2:
            reset_gpio_num = AM_BSP_GPIO_MSPI2_RST;
            break;
        case 3:
            reset_gpio_num = AM_BSP_GPIO_MSPI3_RST;
            break;
        default:
        case 0:
            reset_gpio_num = AM_BSP_GPIO_MSPI0_RST;
            break;
    }

    am_hal_gpio_state_write(reset_gpio_num, AM_HAL_GPIO_OUTPUT_SET);
    am_hal_gpio_pinconfig(reset_gpio_num, am_hal_gpio_pincfg_output);
    am_hal_gpio_state_write(reset_gpio_num, AM_HAL_GPIO_OUTPUT_SET);
    am_util_delay_ms(100);
    am_hal_gpio_state_write(reset_gpio_num, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_util_delay_ms(100);
    am_hal_gpio_state_write(reset_gpio_num, AM_HAL_GPIO_OUTPUT_SET);

    NVIC_SetPriority(mspi_interrupts[mspi_module], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[mspi_module]);
    am_hal_interrupt_master_enable();

    ui32Status = mspi_device_func.mspi_init(mspi_module, (void*)&MSPI_Flash_Config, &g_FlashHdl, &g_MSPIHdl);

    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
#ifdef AM_DEBUG_PRINTF
        am_util_stdio_printf("Error 1: Could not configure the device.\n");
#endif // AM_DEBUG_PRINTF
        RESULT_METADATA_ERRCNT = 0xF000001;
        RESULT_RECOVERY_ERRCNT = 0xF000001;
        AM_ASM_BKPT(1);
        while(1);
    }

    if (use_scrambling)
    {
        ui32Status = mspi_device_func.mspi_scrambling_enable(g_FlashHdl);
        am_util_stdio_printf("  Scrambling enabled\n");
    }

    // write metadata

    // there are easier ways, but this is explicit about endianness
    g_MetadataBuffer[0] = CFGDATA_META_DATAOFFSET & 0xFF;
    g_MetadataBuffer[1] = (CFGDATA_META_DATAOFFSET >> 8) & 0xFF;
    g_MetadataBuffer[2] = (CFGDATA_META_DATAOFFSET >> 16) & 0xFF;
    g_MetadataBuffer[3] = (CFGDATA_META_DATAOFFSET >> 24) & 0xFF;
    g_MetadataBuffer[4] = CFGDATA_META_DATASIZE & 0xFF;
    g_MetadataBuffer[5] = (CFGDATA_META_DATASIZE >> 8) & 0xFF;
    g_MetadataBuffer[6] = (CFGDATA_META_DATASIZE >> 16) & 0xFF;
    g_MetadataBuffer[7] = (CFGDATA_META_DATASIZE >> 24) & 0xFF;
    g_MetadataBuffer[8] = 0; // TODO: appdata offset and size
    g_MetadataBuffer[9] = 0;
    g_MetadataBuffer[10] = 0;
    g_MetadataBuffer[11] = 0;
    g_MetadataBuffer[12] = 0;
    g_MetadataBuffer[13] = 0;
    g_MetadataBuffer[14] = 0;
    g_MetadataBuffer[15] = 0;

    // Make sure buffer is flushed before writing
    sRange.ui32StartAddr = (uint32_t)(g_MetadataBuffer);
    sRange.ui32Size = CFGDATA_META_SIZE;
    am_hal_cachectrl_dcache_clean(&sRange);

    am_util_stdio_printf("Write metadata @ flash offset 0x%08x\n", CFGDATA_META_OFFSET);
    RESULT_METADATA_ERRCNT = write_verify(g_MetadataBuffer, CFGDATA_META_OFFSET, CFGDATA_META_SIZE, CFGDATA_WRV ? true : false);
    if (RESULT_METADATA_ERRCNT > 0)
    {
        am_util_stdio_printf("\nError 2: Errors writing metadata - %d bytes, %d errors\n", CFGDATA_META_DATASIZE, RESULT_METADATA_ERRCNT);
        AM_ASM_BKPT(2);
        while(1);
    }

    if (RESULT_METADATA_ERRCNT < 0)
    {
        am_util_stdio_printf("\nError 3: MSPI failure when writing recovery metadata\n");
        RESULT_METADATA_ERRCNT = 0xF000003;
        RESULT_RECOVERY_ERRCNT = 0xF000003;
        AM_ASM_BKPT(3);
        while(1);
    }

    // write recovery
    am_util_stdio_printf("\nWrite recovery @ flash offset 0x%08x from 0x%08x\n", CFGDATA_META_DATAOFFSET, CFGDATA_RECOVERY_ADDR);
    RESULT_RECOVERY_ERRCNT = write_verify((uint8_t*)(CFGDATA_RECOVERY_ADDR), CFGDATA_META_DATAOFFSET, CFGDATA_META_DATASIZE, CFGDATA_WRV ? true : false);

    if (RESULT_RECOVERY_ERRCNT > 0)
    {
        am_util_stdio_printf("\nError 4: Errors writing recovery image - %d bytes, %d errors\n", CFGDATA_META_DATASIZE, RESULT_RECOVERY_ERRCNT);
        AM_ASM_BKPT(4);
        while(1);
    }

    if (RESULT_RECOVERY_ERRCNT < 0)
    {
        am_util_stdio_printf("\nError 5: MSPI failure when writing recovery image\n");
        AM_ASM_BKPT(5);
        while(1);
    }

    if (use_scrambling)
    {
        ui32Status = mspi_device_func.mspi_scrambling_disable(g_FlashHdl);
        am_util_stdio_printf("  Scrambling disabled\n");
    }

    am_util_stdio_printf("\nMSPI Loader successful\n");

// instrumentation - view MSPI contents through XIP in the debugger
#ifdef XIP_AFTER_FLASH
    // Enable XIP and breakpoint to allow inspection
    am_util_stdio_printf("  XIP enabled, break for inspection\n");
    ui32Status = mspi_device_func.mspi_xip_enable(g_FlashHdl);
    AM_ASM_BKPT(0);
    ui32Status = mspi_device_func.mspi_xip_disable(g_FlashHdl);
#endif

    am_hal_interrupt_master_disable();
    NVIC_DisableIRQ(mspi_interrupts[mspi_module]);
    //
    // Clean up the MSPI before exit.
    //
    ui32Status = mspi_device_func.mspi_term(g_FlashHdl);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to shutdown the MSPI and Flash Device!\n");
    }

    //
    //  End banner.
    //

    //
    //  Trigger the debugger on completion
    //
    AM_ASM_BKPT(0);

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
