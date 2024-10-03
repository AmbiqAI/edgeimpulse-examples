//*****************************************************************************
//
//! @file flash.h
//!
//! @brief Functions to read/write flash.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef FLASH_H
#define FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

// It stores the recording file information.
// NOTE: This struct should be aligned with 4 bytes and struct size should be
// multiple of 16 bytes.
typedef struct
{
    uint32_t length __attribute__((aligned(4))); // File length.
    uint32_t format;                             // Audio format, reserved.
    uint16_t sample_rate;                        // Audio sample rate, reserved.
    uint16_t channel_mode;                       // Audio channel mode, reserved.
    uint32_t bit_rate;                           // Audio bit rate, reserved.
} file_header_t;

void flash_write_init(void);

void flash_write(const void *data, size_t len);

void flash_write_flush(void);

void flash_read_init(void);

uint8_t *flash_get(uint32_t need_size, uint32_t *out_size);

#ifdef __cplusplus
}
#endif

#endif // FLASH_H
