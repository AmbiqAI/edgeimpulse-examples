//*****************************************************************************
//
//! @file mem_image_store.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <errno.h>

#include <openamp/remoteproc_loader.h>

#include "am_devices_ambt53.h"
struct mem_file
{
    const void *base;
};

extern void* g_RpmsgDevHdl;

int mem_image_open(void *store, const char *path, const void **image_data)
{
    struct mem_file *image = store;
    const void *fw_base = image->base;

    (void)(path);
    if (image_data == NULL)
    {
        am_util_debug_printf("%s: input image_data is NULL\r\n", __func__);
        return -EINVAL;
    }
    *image_data = fw_base;
    /* return an abitrary length, as the whole firmware is in memory */
    return 0x100;
}

void mem_image_close(void *store)
{
    /* The image is in memory, does nothing */
    (void)store;
}

int mem_image_load(void *store, size_t offset, size_t size,
                   const void **data, metal_phys_addr_t pa,
                   struct metal_io_region *io,
                   char is_blocking)
{
    struct mem_file *image = store;
    const void *fw_base = image->base;
    uint32_t ui32Status = AM_DEVICES_AMBT53_STATUS_INVALID_OPERATION;

    (void)is_blocking;

    // am_util_debug_printf("%s: offset=0x%x, size=0x%x\n\r",
        // __func__, offset, size);
    if (pa == METAL_BAD_PHYS)
    {
        if (data == NULL)
        {
            am_util_debug_printf("%s: data is NULL while pa is ANY\r\n",
                __func__);
            return -EINVAL;
        }
        *data = (const void *)((const char *)fw_base + offset);
    }
    else
    {
        // The size must be 4 bytes aligned.
        if ((size & 3) != 0)
        {
            ui32Status = AM_DEVICES_AMBT53_STATUS_SEGMENT_SIZE_ERR;
            am_util_debug_printf("The memory size %d isn't 4 bytes aligned, status: %d\r\n",
                            size, ui32Status);
            goto out;
        }

        ui32Status = am_devices_ambt53_memory_erase(g_RpmsgDevHdl, pa, size);
        if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
        {
            am_util_debug_printf("Memory erase fail, status: %d\r\n", ui32Status);
            goto out;
        }

        if (size != 0)
        {
            if ((size & 3) != 0)
            {
                ui32Status = AM_DEVICES_AMBT53_STATUS_SEGMENT_SIZE_ERR;
                am_util_stdio_printf("The file size %d isn't 4 bytes aligned, status: %d\r\n",
                                size, ui32Status);
                goto out;
            }

            // Write data to ambt53 memory through mspi.
            ui32Status = am_devices_ambt53_memory_write(g_RpmsgDevHdl, pa,
                                                size, (uint8_t *)fw_base + offset);
            if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
            {
                am_util_stdio_printf("Memory write fail, status: %d\r\n", ui32Status);
                goto out;
            }
        }
    }

    return (int)size;

out:
    return -EINVAL;
}

const struct image_store_ops mem_image_store_ops =
{
    .open = mem_image_open,
    .close = mem_image_close,
    .load = mem_image_load,
    .features = SUPPORT_SEEK,
};

