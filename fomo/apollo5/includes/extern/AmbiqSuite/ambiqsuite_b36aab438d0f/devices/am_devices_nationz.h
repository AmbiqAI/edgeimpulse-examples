//*****************************************************************************
//
//! @file am_devices_nationz.h
//!
//! @brief Support functions for the NationZ.
//!
//! @addtogroup nationz NationZ Driver
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
#ifndef AM_DEVICES_NATIONZ_H
#define AM_DEVICES_NATIONZ_H

//*****************************************************************************
//
// Type definitions.
//
//*****************************************************************************

//! @name  Patch types.
//! @{
#define AM_DEVICES_NATIONZ_PATCH_FTCODE     0xAA
#define AM_DEVICES_NATIONZ_PATCH_RAMCODE    0xBB
#define AM_DEVICES_NATIONZ_PATCH_ROM        0xCC
//! @}

//! Callback for HCI reads.
typedef void (*am_devices_nationz_read_handler_t)(uint8_t *pui8Data, uint32_t ui32Length);

//! @name  Container for patches
//! @{
#define AM_DEVICES_NATIONZ_HW_PATCH         0xCC
#define AM_DEVICES_NATIONZ_RAM_PATCH        0xBB
//! @}

typedef struct
{
    uint8_t ui8PatchType;
    uint32_t ui32PatchId;
    uint32_t ui32Version;
}
am_devices_nationz_patch_t;

//*****************************************************************************
//
// External function declarations.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @brief Set up pins and IOM for use with Nationz device.
//
//*****************************************************************************
extern void am_devices_nationz_setup(void);

//*****************************************************************************
//
//! @brief Runs a state machine to download patches.
//!
//! @param pui8Patch
//! @param ui16PatchLen
//! @param ui8PatchType
//! @param ui16CRC
//
//*****************************************************************************
extern void am_devices_nationz_apply_patch(const uint8_t *pui8Patch, uint16_t ui16PatchLen, uint8_t ui8PatchType, uint16_t ui16CRC);

//*****************************************************************************
//
//! @brief Runs the driver function to apply the default patch.
//
//*****************************************************************************
extern void am_devices_nationz_apply_default_patch(void);

//*****************************************************************************
//
//! @brief Complete the patching process, and move on to
//
//*****************************************************************************
extern void am_devices_nationz_patches_complete(void);

//*****************************************************************************
//
//! @brief Send an HCI message.
//!
//! @param type
//! @param len
//! @param pData
//!
//! @return
//
//*****************************************************************************
extern uint16_t am_devices_nationz_write(uint8_t type, uint16_t len, uint8_t *pData);

//*****************************************************************************
//
//! @brief Send an HCI message without a "type" byte added to the beginning.
//!
//! @param len
//! @param pData
//!
//! @return
//
//*****************************************************************************
extern uint16_t am_devices_nationz_raw_write(uint16_t len, uint8_t *pData);

//*****************************************************************************
//
//! @brief Retrieve an incoming HCI message.
//
//*****************************************************************************
extern void am_devices_nationz_read(void);

//*****************************************************************************
//
//! @brief Choose a function to handle incoming packets from the Nationz device.
//!
//! @param pfnCallback
//
//*****************************************************************************
extern void am_devices_nationz_set_read_callback(am_devices_nationz_read_handler_t pfnCallback);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_NATIONZ_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

