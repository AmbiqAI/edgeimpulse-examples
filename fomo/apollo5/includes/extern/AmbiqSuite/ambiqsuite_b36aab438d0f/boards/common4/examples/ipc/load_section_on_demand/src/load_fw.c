//*****************************************************************************
//
//! @file load_fw.c
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

#include <openamp/remoteproc.h>
#include <openamp/remoteproc_loader.h>
#include "../../../../../../third_party/open-amp/libmetal/lib/alloc.h"

#include "common.h"

extern const struct image_store_ops mem_image_store_ops;
extern const struct remoteproc_ops ambt53_rproc_ops;

struct mem_file
{
    const void *base;
};
static struct mem_file image =
{
    //! start address of ELF in flash
    .base = (void *)0X100000,
};

//*****************************************************************************
//
//! @brief Load the whole ambt53 firmware from apollo flash to ambt53
//!
//! @return 32-bit status
//
//*****************************************************************************

int load_ambt53_fw(void)
{
    struct remoteproc *rproc = NULL;
    struct remoteproc *rproc_inst = NULL;
    void *store = &image;
    int ret = 0;
    unsigned int image_type = AMBT53_FW;

    rproc_inst = metal_allocate_memory(sizeof(*rproc_inst));
    if (!rproc_inst)
    {
        ret = -EINVAL;
        goto fail;
    }

    rproc = remoteproc_init(rproc_inst, &ambt53_rproc_ops, &image_type);
    if (!rproc)
    {
        ret = -EINVAL;
        goto fail;
    }
    rproc = rproc_inst;

    /* Configure remoteproc to get ready to load executable */
    remoteproc_config(rproc, NULL);

    /* Load remoteproc executable */
    am_util_debug_printf("Start to load firmware with remoteproc_load() \r\n");
    ret = remoteproc_load(rproc, NULL, store, &mem_image_store_ops, NULL);
    if (ret)
    {
        am_util_debug_printf("failed to load firmware\r\n");
        goto fail;
    }
    /* Start the processor */
    ret = remoteproc_start(rproc);
    if (ret)
    {
        am_util_debug_printf("failed to start processor\r\n");
        goto fail;
    }
    am_util_debug_printf("successfully started the processor\r\n");

fail:
    if (rproc)
    {
        remoteproc_stop(rproc);
        remoteproc_shutdown(rproc);
        // Must stop and shutdown firstly, then remove
        remoteproc_remove(rproc);
    }

    if (rproc_inst)
    {
        metal_free_memory(rproc_inst);
        rproc_inst = NULL;
    }
    return ret;
}

//*****************************************************************************
//
//! @brief Lode the section of ambt53 from apollo flash to ambt53
//!
//! @param name - section name
//!
//! @return 32-bit status
//
//*****************************************************************************

int load_ambt53_section(const char *name)
{
    struct remoteproc *rproc = NULL;
    struct remoteproc *rproc_inst = NULL;
    void *store = &image;
    int ret = 0;
    unsigned int image_type = AMBT53_CODEC;

    rproc_inst = metal_allocate_memory(sizeof(*rproc_inst));
    if (!rproc_inst)
    {
        ret = -EINVAL;
        goto fail;
    }

    rproc = remoteproc_init(rproc_inst, &ambt53_rproc_ops, &image_type);
    if (!rproc)
    {
        ret = -EINVAL;
        goto fail;
    }
    rproc = rproc_inst;

    /* Configure remoteproc to get ready to load executable */
    remoteproc_config(rproc, NULL);

    /* Load remoteproc executable */
    am_util_debug_printf("Start to load %s codec with remoteproc_load_section() \r\n", name);
    ret = remoteproc_load_section(rproc, NULL, store, &mem_image_store_ops, NULL, name);
    if (ret)
    {
        am_util_debug_printf("failed to load codec\r\n");
        goto fail;
    }

    /* Start the processor */
    ret = remoteproc_start(rproc);
    if (ret)
    {
        am_util_debug_printf("failed to start processor\r\n");
        goto fail;
    }
    am_util_debug_printf("successfully started the processor\r\n");

fail:
    if (rproc)
    {
        remoteproc_stop(rproc);
        remoteproc_shutdown(rproc);
        // Must stop and shutdown firstly, then remove
        remoteproc_remove(rproc);
    }

    if (rproc_inst)
    {
        metal_free_memory(rproc_inst);
        rproc_inst = NULL;
    }
    return ret;
}