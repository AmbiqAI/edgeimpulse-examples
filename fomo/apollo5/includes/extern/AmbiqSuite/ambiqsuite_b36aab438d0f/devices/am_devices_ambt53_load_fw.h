//*****************************************************************************
//
//! @file am_devices_ambt53_load_fw.h
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

#ifndef AM_DEVICES_AMBT53_LOAD_FW_H
#define AM_DEVICES_AMBT53_LOAD_FW_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_util.h"
#include "am_mcu_apollo.h"

//! "LOAD_HEADER_FILE" Will load the ambt53 firmware stored in the ambt53_fw_image.h to the ambt53
#define LOAD_HEADER_FILE       (1)
//! "LOAD_ELF" Will load the ambt53 firmware stored in the apollo's Flash(.elf file) to the ambt53
//! core if this macro is enabled.
#define LOAD_ELF               (2)
#define LOAD_AMBT53_FIRMWARE   (0)

//! start address of ELF in flash
#define MEM_ELF_BASE 0X100000
//! ELF file's length, standard ELF file, the length is greater than 4
#define ELF_LEN 0X19000
#define ALIGN_UP(size, align) (((size) + (align) -1) & ~((align) - 1))

//*****************************************************************************
//! AMBT53 firmware load definition.
//*****************************************************************************
typedef enum
{
    AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_PTCM,
    AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_DTCM,
    AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_CSRAM,
    AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_DSRAM,
    AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_NUMS
} am_devices_ambt53_program_memory_type_e;

//
//!
//
typedef struct
{
    am_devices_ambt53_program_memory_type_e eMemoryType;
    uint8_t *pui8SectorData;
    uint32_t ui32DataSize;
    uint32_t ui32SectorAddress;
    uint32_t ui32SectorSize;
} am_devices_ambt53_firmare_sector_info_t;

//
//!
//
typedef struct
{
    am_devices_ambt53_firmare_sector_info_t sectorInfo[AM_DEVICES_AMBT53_PROGRAM_MEMORY_TYPE_NUMS];
} am_devices_ambt53_image_t;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Erase the specific length of giving address of ambt53 memory by
//! writing all 0x00.
//!
//! @param pHandle - MSPI device instance
//! @param ui32Address - Start address to erase in the ambt53 memory
//! @param ui32Size - Size of bytes to erase the ambt53 memory
//!
//! @return 32-bit status
//!
//*****************************************************************************
uint32_t am_devices_ambt53_memory_erase(void *pHandle, uint32_t ui32Address, uint32_t ui32Size);
//*****************************************************************************
//
//! @brief Write the specific length of data to the giving address of ambt53 memory,
//! then read back the written data for comparison to verify if the write succeeded.
//!
//! @param pHandle - MSPI device instance
//! @param ui32Address - Start address to write to in the ambt53 memory
//! @param ui32Size - Size of bytes to write to the ambt53 memory
//! @param pData - Data to write the ambt53 memory
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t am_devices_ambt53_memory_write(void *pHandle, uint32_t ui32Address, uint32_t ui32Size, uint8_t *pData);
//*****************************************************************************
//
//! @brief Get the ambt53 image for the loading.
//!
//! @param pImage - image for loading
//!
//! @return 32-bit status
//!
//*****************************************************************************
uint32_t am_devices_ambt53_get_image(am_devices_ambt53_image_t *pImage);
//*****************************************************************************
//
//! @brief Load the whole firmware to the ambt53 core.
//!
//! @param pHandle - MSPI device instance
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t am_devices_ambt53_firmware_load(void *pHandle);
//*****************************************************************************
//
//! @brief parse ambt53 dsp information.
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t am_devices_ambt53_elf_parse(void);
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
am_devices_ambt53_firmware_dynamic_load(const char *name);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_AMBT53_LOAD_FW_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

