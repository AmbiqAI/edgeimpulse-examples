//*****************************************************************************
//
//! @file ambt53_rproc_ops.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <openamp/remoteproc.h>
#include <openamp/remoteproc_loader.h>
#include "../../../../../../third_party/open-amp/libmetal/lib/alloc.h"

#include "am_devices_ambt53.h"
#include "elf_loader.h"
#include "common.h"

static struct remoteproc *ambt53_rproc_init(struct remoteproc *rproc,
                    const struct remoteproc_ops *ops, void *arg)
{
    struct rproc_priv *priv;
    unsigned int image_type = *((unsigned int *)arg);

    if (image_type == AMBT53_FW)
    {
        // Hould ambt53.
        am_devices_ambt53_core_hold();
    }

    am_util_stdio_printf("%s:image type : %d\n\r", __func__, image_type);
    priv = metal_allocate_memory(sizeof(*priv));
    if (!priv)
    {
        return NULL;
    }

    memset(priv, 0, sizeof(*priv));
    priv->rproc = rproc;
    priv->image_type = image_type;
    priv->rproc->ops = ops;
    metal_list_init(&priv->rproc->mems);
    priv->rproc->priv = priv;
    rproc->state = RPROC_READY;
    return priv->rproc;
}

static void ambt53_rproc_remove(struct remoteproc *rproc)
{
    struct rproc_priv *priv;

    priv = (struct rproc_priv *)rproc->priv;
    metal_free_memory(priv);
}

static void *ambt53_rproc_mmap(struct remoteproc *rproc,
                               metal_phys_addr_t *pa, metal_phys_addr_t *da,
                               size_t size, unsigned int attribute,
                               struct metal_io_region **io)
{
    struct rproc_priv *priv;

    priv = rproc->priv;

    if (priv->image_type == AMBT53_FW)
    {
        // PTCM and DTCM have the same start address in ambt53 and they are
        // less than the start address of SRAM.
        // The flag of segment is executable, it's run on PTCM.
        if ((*da < SRAM_BANK0_OFFSET) && ((attribute & PF_X) != 0))
        {
            // The ptcm writes data through the AHB, and the address need to be offset
            *pa = PTCM_OFFSET + *da;
        }
        else
        {
            *pa = *da;
        }
    }
    else
    {
        // PTCM and DTCM have the same start address in ambt53 and they are
        // less than the start address of SRAM.
        // The flag of segment is executable, it's run on PTCM.
        if ((*da < SRAM_BANK0_OFFSET) && ((attribute & SHF_EXECINSTR) != 0))
        {
            // The ptcm writes data through the AHB, and the address need to be offset
            *pa = PTCM_OFFSET + *da;
        }
        else
        {
            *pa = *da;
        }
    }

    return NULL;
}

static int ambt53_rproc_start(struct remoteproc *rproc)
{
    struct rproc_priv *priv;

    priv = rproc->priv;

    if (priv->image_type == AMBT53_FW)
    {
        // Ambt53 start to run.
        am_devices_ambt53_core_start();
    }

    return 0;
}

static int ambt53_rproc_stop(struct remoteproc *rproc)
{
    /* It is lacking a stop operation in the libPM */
    (void)rproc;
    return 0;
}

static int ambt53_rproc_shutdown(struct remoteproc *rproc)
{
    return 0;
}

const struct remoteproc_ops ambt53_rproc_ops =
{
    .init = ambt53_rproc_init,
    .remove = ambt53_rproc_remove,
    .start = ambt53_rproc_start,
    .stop = ambt53_rproc_stop,
    .shutdown = ambt53_rproc_shutdown,
    .mmap = ambt53_rproc_mmap,
};
