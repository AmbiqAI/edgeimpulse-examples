//*****************************************************************************
//
//! @file am_devices_ambt53_load_fw.c
//!
//! @brief The implementation of Apollo interface to AMBT53 firmware loading.
//!
//! @addtogroup ambt53 AMBT53 Device Driver
//! @ingroup devices
//! @{
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
#include "am_bsp.h"
#include "am_mcu_apollo.h"
#include "am_util_stdio.h"
#include "am_devices_ambt53.h"

#if (LOAD_AMBT53_FIRMWARE == LOAD_HEADER_FILE)
#include "ambt53_fw_image.h"
#elif (LOAD_AMBT53_FIRMWARE == LOAD_ELF)
#include <openamp/elf_loader.h>
#endif

//*****************************************************************************
//
//! @name Macro definitions.
//! @{
//
//*****************************************************************************
#define LOAD_FW_DEBUG_LOG            0
#define LOAD_FW_BOOT_VERIFY          0
#define LOAD_FW_WRITE_CHECK          1
#define LOAD_FW_BUFFER_SIZE         (4 * 1024)
//*****************************************************************************
//! Data buffer for the firmware loading.
//*****************************************************************************
AM_SHARED_RW static uint8_t g_ui8LoadFWBuffer[LOAD_FW_BUFFER_SIZE];

//*****************************************************************************
//! AMBT53 image information.
//*****************************************************************************
static am_devices_ambt53_image_t g_ambt53Image =
{
    .sectorInfo[0] =
    {
        .eMemoryType       = AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_PTCM,
        .pui8SectorData    = NULL,
        .ui32DataSize      = 0,
        .ui32SectorAddress = PTCM_OFFSET,
        .ui32SectorSize    = PTCM_SIZE
    },
    .sectorInfo[1] =
    {
        .eMemoryType       = AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_DTCM,
        .pui8SectorData    = NULL,
        .ui32DataSize      = 0,
        .ui32SectorAddress = DTCM_OFFSET,
        .ui32SectorSize    = DTCM_SIZE
    },
    .sectorInfo[2] =
    {
        .eMemoryType       = AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_CSRAM,
        .pui8SectorData    = NULL,
        .ui32DataSize      = 0,
        .ui32SectorAddress = SRAM_BANK0_OFFSET,
#if (BLUEBUD_RTL_VER >= BLUEBUD_RTL_1_1_1)
        .ui32SectorSize    = (SRAM_BANK0_SIZE + SRAM_BANK1_SIZE + SRAM_BANK2_SIZE)
#else
        .ui32SectorSize    = SRAM_BANK0_SIZE
#endif
    },
    .sectorInfo[3] =
    {
        .eMemoryType       = AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_DSRAM,
        .pui8SectorData    = NULL,
        .ui32DataSize      = 0,
#if (BLUEBUD_RTL_VER >= BLUEBUD_RTL_1_1_1)
        .ui32SectorAddress = SRAM_BANK3_OFFSET,
        .ui32SectorSize    = SRAM_BANK3_SIZE
#else
        .ui32SectorAddress = SRAM_BANK1_OFFSET,
        .ui32SectorSize    = (SRAM_BANK1_SIZE + SRAM_BANK2_SIZE + SRAM_BANK3_SIZE)
#endif
    }
};

//*****************************************************************************
//! Loaded Image.
//*****************************************************************************
#if (LOAD_AMBT53_FIRMWARE == LOAD_HEADER_FILE)
static am_devices_ambt53_image_t g_loadImage =
{
    .sectorInfo[AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_PTCM] =
    {
        .pui8SectorData    = (uint8_t*)ambt53_fw_ptcm_bin,
        .ui32DataSize      = sizeof(ambt53_fw_ptcm_bin),
    },
    .sectorInfo[AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_DTCM] =
    {
        .pui8SectorData    = (uint8_t*)ambt53_fw_dtcm_bin,
        .ui32DataSize      = sizeof(ambt53_fw_dtcm_bin),
    },
    .sectorInfo[AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_CSRAM] =
    {
        .pui8SectorData    = (uint8_t*)ambt53_fw_csram_bin,
        .ui32DataSize      = sizeof(ambt53_fw_csram_bin),
    },
    .sectorInfo[AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_DSRAM] =
    {
        .pui8SectorData    = (uint8_t*)ambt53_fw_dsram_bin,
        .ui32DataSize      = sizeof(ambt53_fw_dsram_bin),
    }
};
#endif
//*****************************************************************************
//
//  Erase the specific length of giving address of ambt53 memory by
//
//*****************************************************************************
uint32_t
am_devices_ambt53_memory_erase(void *pHandle, uint32_t ui32Address, uint32_t ui32Size)
{
    if ((pHandle == NULL) || (ui32Size == 0))
    {
        return AM_DEVICES_AMBT53_STATUS_INVALID_ARG;
    }

    uint32_t ui32Status;
    memset(&g_ui8LoadFWBuffer[0], 0x00, LOAD_FW_BUFFER_SIZE);
    for (uint32_t i = 0; i < (ui32Size / LOAD_FW_BUFFER_SIZE); i++)
    {
        ui32Status = am_devices_mspi_ambt53_dma_write(pHandle, &g_ui8LoadFWBuffer[0], (ui32Address + LOAD_FW_BUFFER_SIZE * i), LOAD_FW_BUFFER_SIZE, true);
        CHECK_DMA_ERASE(ui32Status, (ui32Address + LOAD_FW_BUFFER_SIZE * i));
    }

    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Write the specific length of data to the giving address of ambt53 memory,
//
//*****************************************************************************
uint32_t
am_devices_ambt53_memory_write(void *pHandle, uint32_t ui32Address, uint32_t ui32Size, uint8_t *pData)
{
    if ((pHandle == NULL) || (ui32Size == 0) || (pData == NULL))
    {
        return AM_DEVICES_AMBT53_STATUS_INVALID_ARG;
    }

    uint32_t ui32Status;

    ui32Status = am_devices_mspi_ambt53_dma_write(pHandle, pData, ui32Address, ui32Size, true);
    CHECK_DMA_WRITE(ui32Status, ui32Address);

#if (LOAD_FW_WRITE_CHECK)
    uint32_t ui32ReadAddress;
    uint32_t ui32ReadFrames = (ui32Size / LOAD_FW_BUFFER_SIZE) + ((ui32Size % LOAD_FW_BUFFER_SIZE) ? 1 : 0);
    uint32_t ui32FrameBytes;

    for (uint32_t i = 0; i < ui32ReadFrames; i++)
    {
        if (i == (ui32ReadFrames - 1))
        {
            // read the last frame
            ui32FrameBytes = (ui32Size % LOAD_FW_BUFFER_SIZE) ? (ui32Size % LOAD_FW_BUFFER_SIZE) : LOAD_FW_BUFFER_SIZE;
        }
        else
        {
            ui32FrameBytes = LOAD_FW_BUFFER_SIZE;
        }

        ui32ReadAddress = ui32Address + LOAD_FW_BUFFER_SIZE * i;

        // Read back the written data for comparison, PTCM can only be written
        if (((ui32ReadAddress + LOAD_FW_BUFFER_SIZE) <= PTCM_OFFSET) || (ui32ReadAddress >= (PTCM_OFFSET + PTCM_SIZE)))
        {
            ui32Status = am_devices_mspi_ambt53_dma_read(pHandle, &g_ui8LoadFWBuffer[0], ui32ReadAddress, ui32FrameBytes, true);
            CHECK_DMA_READ(ui32Status, ui32ReadAddress);

            if (memcmp((pData + LOAD_FW_BUFFER_SIZE * i), &g_ui8LoadFWBuffer[0], ui32FrameBytes))
            {
                am_util_stdio_printf("address 0x%08x write/read verification failed!!\n", ui32ReadAddress);
                return AM_DEVICES_AMBT53_STATUS_FAIL;
            }
        }
    }
#endif

    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Get the ambt53 image for the loading.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_get_image(am_devices_ambt53_image_t *pImage)
{
    if (pImage == NULL)
    {
        return AM_DEVICES_AMBT53_STATUS_INVALID_ARG;
    }

    for (uint8_t i = 0; i < AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_NUMS; i++)
    {
        // The firmware binary array may include only 1 byte 0x00, this is added deliberately to clear
        // the zero-length array compiling error in some specific compiling environment. No need to load
        // such data to ambt53 memory.
        if ((pImage->sectorInfo[i].ui32DataSize != 0) || (pImage->sectorInfo[i].ui32DataSize != 1))
        {
            g_ambt53Image.sectorInfo[i].ui32DataSize   = pImage->sectorInfo[i].ui32DataSize;
            g_ambt53Image.sectorInfo[i].pui8SectorData = pImage->sectorInfo[i].pui8SectorData;
        }
    }

    return AM_DEVICES_AMBT53_STATUS_SUCCESS;
}

#if (LOAD_AMBT53_FIRMWARE != LOAD_ELF)
//*****************************************************************************
//
//  Load the whole firmware to the ambt53 core.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_firmware_load(void *pHandle)
{
    uint32_t ui32Status = AM_DEVICES_AMBT53_STATUS_INVALID_OPERATION;

    am_devices_ambt53_core_hold();

    for (uint8_t i = 0; i < 4; i++)
    {
        if (g_ambt53Image.sectorInfo[i].ui32DataSize != 0)
        {
            ui32Status = am_devices_ambt53_memory_erase(pHandle,
                                                        g_ambt53Image.sectorInfo[i].ui32SectorAddress,
                                                        g_ambt53Image.sectorInfo[i].ui32SectorSize);
            if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
            {
                am_util_stdio_printf("Memory type %d erase fail, status: %d\n", g_ambt53Image.sectorInfo[i].eMemoryType, ui32Status);
                return ui32Status;
            }

            ui32Status = am_devices_ambt53_memory_write(pHandle,
                                                        g_ambt53Image.sectorInfo[i].ui32SectorAddress,
                                                        g_ambt53Image.sectorInfo[i].ui32DataSize,
                                                        g_ambt53Image.sectorInfo[i].pui8SectorData);
            if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
            {
                am_util_stdio_printf("Memory type %d program fail, status: %d\n", g_ambt53Image.sectorInfo[i].eMemoryType, ui32Status);
                return ui32Status;
            }
        }
    }

    if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("ambt53 firmware load fail!!\n");
        return ui32Status;
    }
    else
    {
        am_util_stdio_printf("ambt53 firmware load succeed\n");
    }

#if (LOAD_FW_BOOT_VERIFY)
    ui32Status = am_devices_ambt53_firmare_boot_verify(pHandle);
#else
    ui32Status = am_devices_ambt53_core_start();
#endif

    return ui32Status;
}
#else
//*****************************************************************************
//
//  Load the whole firmware to the ambt53 core.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_firmware_load(void *pHandle)
{
    uint8_t *img_data;
    void *ambt53_info = NULL;
    int number, ret;
    size_t offset, fsize, msize;
    uint32_t vaddr, paddr, ui32SectorAddress;
    unsigned int type, flag;
    uint32_t ui32Status = AM_DEVICES_AMBT53_STATUS_INVALID_OPERATION;

    img_data = (uint8_t *)MEM_ELF_BASE;
    if ( elf_identify(img_data, ELF_LEN) != 0)
    {
        am_util_stdio_printf("This is not ELF file.\r\n");
        goto out;
    }

    ret = elf_load_ph_header(img_data, &ambt53_info, &number);
    if (ret < 0)
    {
        am_util_stdio_printf("load program header fail \r\n");
        goto out;
    }

    am_devices_ambt53_core_hold();
    for (int i = 0; i < number; i++)
    {
        ret = elf_parse_ph_header(&ambt53_info, i, &type, &offset, &vaddr, &paddr,
                            &fsize, &msize, &flag, NULL);
        if (ret < 0)
        {
            am_util_stdio_printf("parse program header fail , the segment index number is %d\r\n", i);
            goto out;
        }

        if ((type == PT_LOAD) && (msize != 0))
        {
            if ((msize & 3) != 0)
            {
                ui32Status = AM_DEVICES_AMBT53_STATUS_SEGMENT_SIZE_ERR;
                am_util_stdio_printf("The memory size %d of segment #%d isn't 4 bytes aligned, status: %d\r\n",
                                msize, i, ui32Status);
                goto out;
            }

            if ((vaddr < SRAM_BANK0_OFFSET) && ((flag & PF_X) != 0))
            {
                ui32SectorAddress = PTCM_OFFSET + vaddr;
            }
            else
            {
                ui32SectorAddress = vaddr;
            }

            ui32Status = am_devices_ambt53_memory_erase(pHandle, ui32SectorAddress, msize);
            if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
            {
                am_util_stdio_printf("Memory erase segment #%d fail, status: %d\r\n", i, ui32Status);
                goto out;
            }

            if (fsize != 0)
            {
                if ((fsize & 3) != 0)
                {
                    ui32Status = AM_DEVICES_AMBT53_STATUS_SEGMENT_SIZE_ERR;
                    am_util_stdio_printf("The file size %d of segment #%d isn't 4 bytes aligned, status: %d\r\n",
                                    fsize, i, ui32Status);
                    goto out;
                }

                ui32Status = am_devices_ambt53_memory_write(pHandle, ui32SectorAddress,
                                                    fsize, img_data + offset);
                if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
                {
                    am_util_stdio_printf("Memory write segment #%d fail, status: %d\r\n", i, ui32Status);
                    goto out;
                }
            }
        }
    }

    if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("ambt53 firmware load fail!!\n");
        goto out;
    }
    else
    {
        am_util_stdio_printf("ambt53 firmware load succeed\n");
    }
    ui32Status = am_devices_ambt53_core_start();

    out:
        elf_release(ambt53_info);
        return ui32Status;
}
//*****************************************************************************
//
//! @brief parse ambt53 dsp information.
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_ambt53_elf_parse(void)
{
    int ret;
    uint8_t *img_data;
    void *ambt53_info = NULL;
    uint32_t ui32Status = AM_DEVICES_AMBT53_STATUS_SUCCESS;
    img_data = (uint8_t *)MEM_ELF_BASE;
    if ( elf_identify(img_data, ELF_LEN) != 0 )
    {
        am_util_stdio_printf("This is not ELF file.\r\n");
        ui32Status = AM_DEVICES_AMBT53_STATUS_FAIL;
        goto out;
    }
    ret = elf_parse(img_data, &ambt53_info);
    if (ret < 0)
    {
        am_util_stdio_printf("ambt53 dsp elf parsing failed\r\n");
        ui32Status = AM_DEVICES_AMBT53_STATUS_FAIL;
        goto out;
    }

    out:
        elf_release(ambt53_info);
        return ui32Status;
}
//*****************************************************************************
//
//! @brief The firmware is dynamically loaded by section name
//!
//! @param name - The section name of elf file
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t
am_devices_ambt53_firmware_dynamic_load(const char *name)
{
    uint8_t *img_data;
    void *ambt53_info = NULL , *pHandle = g_RpmsgDevHdl;
    int ret;
    size_t offset, size;
    uint32_t addr, ui32SectorAddress;
    unsigned int flags, type;
    uint32_t ui32Status = AM_DEVICES_AMBT53_STATUS_INVALID_OPERATION;

    if (pHandle == NULL)
    {
        am_util_stdio_printf("Init mspi driver fail.\r\n");
        goto out;
    }

    img_data = (uint8_t *)MEM_ELF_BASE;
    if (elf_identify(img_data, ELF_LEN) != 0)
    {
        am_util_stdio_printf("This is not ELF file.\r\n");
        goto out;
    }

    ret = elf_parse_section_from_name(img_data, &ambt53_info, name,
                                &type, &flags, &addr, &offset, &size, NULL);
    if (ret < 0)
    {
        am_util_stdio_printf("parse section fail \r\n");
        goto out;
    }

    if (((flags & SHF_EXECINSTR) != 0) && (addr < SRAM_BANK0_OFFSET))
    {
        ui32SectorAddress = PTCM_OFFSET + addr;
    }
    else
    {
        ui32SectorAddress = addr;
    }

    if ((size != 0) && (type == SHT_PROGBITS))
    {
        if ((size & 3) != 0)
        {
            ui32Status = AM_DEVICES_AMBT53_STATUS_SEGMENT_SIZE_ERR;
            am_util_stdio_printf("The size #%d of section #\"%s\" isn't 4 bytes aligned, status: %d\r\n",
                            size, name, ui32Status);
            goto out;
        }

        ui32Status = am_devices_ambt53_memory_erase(pHandle, ui32SectorAddress, size);
        if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Memory erase section #\"%s\" fail, status: %d\r\n", name, ui32Status);
            goto out;
        }

        ui32Status = am_devices_ambt53_memory_write(pHandle, ui32SectorAddress,
                                            size, img_data + offset);
        if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Memory write section #\"%s\" fail, status: %d\r\n", name, ui32Status);
            goto out;
        }
        am_util_stdio_printf("ambt53 firmware dynamic load succeed\n");
    }
    else
    {
        am_util_stdio_printf("ambt53 firmware dynamic load fail, the size #%d of section #\"%s\" must be non-zero, "
                        "the type #%d of section must be 1(PROGBITS), status: %d\r\n", size, name, type, ui32Status);
    }

    out:
        elf_release(ambt53_info);
        return ui32Status;
}
#endif

#if (LOAD_FW_BOOT_VERIFY)
//*****************************************************************************
//! am_devices_ambt53_firmare_boot_verify
//!
//! @brief Verify if the loaded ambt53 has boot up successfully.
//!
//! @param pHandle - MSPI device instance
//! @return 32-bit status
//!
//*****************************************************************************
static uint32_t
am_devices_ambt53_firmare_boot_verify(void *pHandle)
{
    // Assuming the application of loaded firmware will do the ADD operation for memory
    // from TEST_DATA_ADDRESS ~ (TEST_DATA_ADDRESS + TEST_DATA_SIZE) by 1.
    // Comparing the data read out before and after releasing the core reset then we can
    // know if the loaded firmware boots up successfully.
    // For other loaded firmware, you can implement other test case (e.g., uart print,
    // GPIO toggle) to verfiy it.

#define TEST_DATA_ADDRESS  (0x21000)
#define TEST_DATA_SIZE     (16)

    uint8_t ui8BeforeBoot[TEST_DATA_SIZE];
    uint8_t ui8AfterBoot[TEST_DATA_SIZE];
    uint32_t ui32Status;

    ui32Status = am_devices_mspi_ambt53_dma_read(pHandle, &ui8BeforeBoot[0], TEST_DATA_ADDRESS, TEST_DATA_SIZE, true);
    CHECK_DMA_READ(ui32Status, TEST_DATA_ADDRESS);

#if (LOAD_FW_DEBUG_LOG)
    am_util_stdio_printf("Read out data of addr 0x%x before executing ambt53 firmware:\n", TEST_DATA_ADDRESS);
    for (uint32_t i = 0; i < TEST_DATA_SIZE; i++)
    {
        am_util_stdio_printf("%02x ", ui8BeforeBoot[i]);
    }
    am_util_stdio_printf("\n");
#endif

    // relase the reset and ambt53 firmware starts to run
    am_devices_ambt53_core_start();

    ui32Status = am_devices_mspi_ambt53_dma_read(pHandle, &ui8AfterBoot[0], TEST_DATA_ADDRESS, TEST_DATA_SIZE, true);
    if ( AM_DEVICES_AMBT53_STATUS_SUCCESS == ui32Status )
    {
        CHECK_DMA_READ(ui32Status, TEST_DATA_ADDRESS);
    }

#if (LOAD_FW_DEBUG_LOG)
    am_util_stdio_printf("Read out data of addr 0x%x after executing ambt53 firmware:\n", TEST_DATA_ADDRESS);
    for (uint32_t i = 0; i < TEST_DATA_SIZE; i++)
    {
        am_util_stdio_printf("%02x ", ui8AfterBoot[i]);
    }
    am_util_stdio_printf("\n");
#endif

    for (uint32_t i = 0; i < TEST_DATA_SIZE; i++)
    {
        if (ui8AfterBoot[i] != (ui8BeforeBoot[i] + 1))
        {
            am_util_stdio_printf("Loaded firmware boot up verify fail!!!\n");
            return AM_DEVICES_AMBT53_STATUS_FAIL;
        }
    }

    am_util_stdio_printf("Loaded firmware boot up verify succeed\n");

    return ui32Status;
}
#endif

//! @}
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

