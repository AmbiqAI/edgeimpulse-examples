//*****************************************************************************
//
//! @file flash.c
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

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "am_util_debug.h"
#include "am_util_stdio.h"
#include "am_util_bootloader.h"

#include "flash.h"

#define AM_FLASH_PAGE_SIZE             (2*1024)
#define AM_FLASH_DATA_SIZE             (200*1024)
#define AM_FLASH_WRITE_START_ADDRESS   (0x00100000)
#define AM_FLASH_WRITE_END_ADDRESS     (AM_FLASH_WRITE_START_ADDRESS + AM_FLASH_DATA_SIZE)
#define AM_FLASH_READ_DATA_SIZE        (136512)//(200*1024)
#define AM_FLASH_READ_START_ADDRESS    (0x00100000)
#define AM_FLASH_READ_END_ADDRESS      (AM_FLASH_READ_START_ADDRESS + AM_FLASH_READ_DATA_SIZE)

typedef struct
{
    uint8_t     writeBuffer[AM_FLASH_PAGE_SIZE]   __attribute__((aligned(4)));   // needs to be 32-bit word aligned.
    uint16_t    bufferIndex;
} flashOp_t;

void flash_flush_buffer(void);
void flash_flush_header(void);

flashOp_t flash = {
    .bufferIndex = 0,
};

static uint16_t g_recvIndex = 0;
static uint32_t g_writeAddr = AM_FLASH_WRITE_START_ADDRESS;
static uint32_t g_readAddr  = AM_FLASH_READ_START_ADDRESS;
static uint32_t g_readIndex = 0;
static file_header_t file_header;

void flash_write_init(void)
{
    g_writeAddr = AM_FLASH_WRITE_START_ADDRESS;
    while (g_writeAddr < AM_FLASH_WRITE_END_ADDRESS)
    {
        am_util_bootloader_erase_flash_page(g_writeAddr);
        g_writeAddr += AM_FLASH_PAGE_SIZE;
    }
    g_writeAddr = AM_FLASH_WRITE_START_ADDRESS;
    flash.bufferIndex = sizeof(file_header_t);

    memset(&file_header, 0, sizeof(file_header_t));
}

void flash_write(const void *data, size_t len)
{
    uint8_t *p_data = (uint8_t*)data;

    (void)g_recvIndex;
    // am_util_debug_printf("idx %d, len %d\n", ++g_recvIndex, len);

    if (len > AM_FLASH_PAGE_SIZE)
    {
        am_util_debug_printf("flash_write size %d is larger than page size %d\n", len, AM_FLASH_PAGE_SIZE);
        return;
    }

    if (g_writeAddr >= AM_FLASH_WRITE_END_ADDRESS)
    {
        return;
    }

    if ((g_writeAddr + flash.bufferIndex + len) > AM_FLASH_WRITE_END_ADDRESS)
    {
        len = AM_FLASH_PAGE_SIZE - flash.bufferIndex;
    }

    if (flash.bufferIndex + len < AM_FLASH_PAGE_SIZE)
    {
        memcpy(&flash.writeBuffer[flash.bufferIndex], p_data, len);
        flash.bufferIndex += len;
    }
    else
    {
        uint16_t remainingBytes = flash.bufferIndex + len - AM_FLASH_PAGE_SIZE;
        memcpy(&flash.writeBuffer[flash.bufferIndex], p_data, (AM_FLASH_PAGE_SIZE - flash.bufferIndex));

        // Write the received data to MRAM by page
        am_util_bootloader_program_flash_page(g_writeAddr, (uint32_t*)&flash.writeBuffer[0], AM_FLASH_PAGE_SIZE);
        g_writeAddr += AM_FLASH_PAGE_SIZE;
        flash.bufferIndex = 0;
        memset(&flash.writeBuffer[0], 0xff, AM_FLASH_PAGE_SIZE);
        if (remainingBytes > 0)
        {
            memcpy(&flash.writeBuffer[0], (p_data + len - remainingBytes), remainingBytes);
            flash.bufferIndex += remainingBytes;
        }
        if (g_writeAddr >= AM_FLASH_WRITE_END_ADDRESS)
        {
            am_util_debug_printf("flash_write full, size %d\n", g_writeAddr - AM_FLASH_WRITE_START_ADDRESS);
        }
    }
}

void flash_flush_buffer(void)
{
    if (flash.bufferIndex)
    {
        // As the length of written words MUST be multiple of 4, so align it.
        uint32_t word_aligned_len = (flash.bufferIndex + 15) / 16 * 4;
        am_util_bootloader_write_flash_within_page(g_writeAddr, (uint32_t*)&flash.writeBuffer[0], word_aligned_len);
    }
}

void flash_flush_header(void)
{
    file_header_t header;
    memcpy(&header, (const void *)AM_FLASH_WRITE_START_ADDRESS, sizeof(file_header_t));
    header.length = g_writeAddr + flash.bufferIndex - AM_FLASH_WRITE_START_ADDRESS - sizeof(file_header_t);
    am_util_bootloader_write_flash_within_page(AM_FLASH_WRITE_START_ADDRESS, (uint32_t *)&header, sizeof(file_header_t) / 4);
}

void flash_write_flush(void)
{
    flash_flush_buffer();
    flash_flush_header();
}

void flash_read_init(void)
{
    memcpy(&file_header, (const void *)AM_FLASH_READ_START_ADDRESS, sizeof(file_header_t));
    g_readIndex = 0;
    // am_util_debug_printf("flash_read_init, file length %d\n", file_header.length);
}

uint8_t *flash_get(uint32_t need_size, uint32_t *out_size)
{
    uint32_t current_index;
    uint32_t size;
    if (g_readIndex >= file_header.length)
    {
        // g_readIndex = 0;
        *out_size = 0;
        return NULL;
    }
    current_index = g_readIndex;
    if (current_index + need_size > file_header.length)
    {
        size = file_header.length - current_index;
    }
    else
    {
        size = need_size;
    }
    g_readIndex += size;
    *out_size = size;

    return (uint8_t *)(g_readAddr + sizeof(file_header_t) + current_index);
}
