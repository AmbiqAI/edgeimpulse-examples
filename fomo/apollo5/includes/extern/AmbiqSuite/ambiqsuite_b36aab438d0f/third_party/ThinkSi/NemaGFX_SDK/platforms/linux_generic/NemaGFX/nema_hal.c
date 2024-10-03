/* TSI 2023.xmo */
/*******************************************************************************
 * Copyright (c) 2023 Think Silicon Single Member PC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGFX API. THE UNMODIFIED, NORMATIVE VERSIONS OF THINK-SILICON NEMAGFX
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT:
 *   https://think-silicon.com/products/software/nemagfx-api
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon Single Member PC be liable for any claim, damages or other
 *  liability, whether in an action of contract, tort or otherwise, arising
 *  from, out of or in connection with the software or the use or other dealings
 *  in the software.
 ******************************************************************************/

#include "nema_hal.h"
#include "nema_sys_defs.h"
#include "nema_regs.h"
#include "tsi_malloc.h"
#include "nema_ringbuffer.h"
#include "nema_error.h"
#include "nema_error_intern.h"

#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "nema_devmem.h"

#include "nema_incl.h"
static int nema_fd = -1;

// Video Memory
//---------------------------------------
uint32_t vmem_base_phys;
#define vmem_size   NEMA_SHMEM_SIZE

nema_ringbuffer_t *ring_buffer_str;

#if defined(NEMA_MULTI_PROCESS) || defined(NEMA_MULTI_THREAD)
static pthread_mutex_t *mutexes;
#endif

int32_t nema_sys_init(void) {
    // Initialise Graphics Mem Space
    //-------------------------------------------------------------------------------
    void *vmem_base_virt;

    // Open driver
    nema_fd = open("/dev/nema", O_RDWR);
    if (nema_fd < 0) {
        printf("Could not open /dev/nema\n");
        return -1;
    }

    // Map and initialize Graphics Memory
    vmem_base_virt = mmap((void *)NULL, NEMA_SHMEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, nema_fd, 0);
    if ( vmem_base_virt == MAP_FAILED ) {
        printf("Couldn't mmap Graphics Memory\n");
        return -1;
    }

    int ret = ioctl(nema_fd, NEMA_IOCTL_GET_SHMEM_PHYS_ADDR, &vmem_base_phys);

    if (ret != 0) {
        printf("Couldn't get Graphics Memory physical address\n");
        return ret;
    }

    printf("Graphics mem: V:%p, P:0x%08x\n", vmem_base_virt, vmem_base_phys);

    //Set mutexes and ring_buffer structure

#if defined(NEMA_MULTI_PROCESS) || defined(NEMA_MULTI_THREAD)
    mutexes         = (pthread_mutex_t *)vmem_base_virt;
    ring_buffer_str = (nema_ringbuffer_t *)(&mutexes[2]);
#else
    ring_buffer_str = (nema_ringbuffer_t *)vmem_base_virt;
#endif

    //Shared graphics memory looks like:
    //  ___________________
    // |   mutex[0]_(CL)   |
    // |___________________|
    // | mutex[1] (Malloc) |
    // |___________________|
    // |  ring_buffer_str  |
    // |___________________|
    // |  (malloc managed) |
    // |        ...        |
    // |        ...        |
    // |        ...        |
    // |        ...        |
    // |        ...        |
    // |___________________|

    uintptr_t ring_buffer_str_plus_1 = (uintptr_t)ring_buffer_str+1U;
    uintptr_t vmem_offset = ring_buffer_str_plus_1 - (uintptr_t)(char *)vmem_base_virt;
    vmem_offset     = (vmem_offset+0xffU) & ~0xffU;

    void *mem_base_virt = (void *)((char *)vmem_base_virt+vmem_offset);

    // Get attached processes count
    // If 0, we should do the setup
    int attached_processes = 0;
    ioctl(nema_fd, NEMA_IOCTL_PROCESS_ATTACH, &attached_processes);

#if defined(NEMA_MULTI_PROCESS) || defined(NEMA_MULTI_THREAD)
    if (attached_processes > 0) {
        tsi_malloc_init(mem_base_virt, vmem_base_phys+vmem_offset, vmem_size, 0);
        nema_rb_init(ring_buffer_str, 0);
        return 1;
    }

    //Initialize Mutices
    pthread_mutexattr_t mutex_shared_attr;
    if ( pthread_mutexattr_init(&mutex_shared_attr) != 0 ||
         pthread_mutexattr_setpshared(&mutex_shared_attr,PTHREAD_PROCESS_SHARED) != 0 ||
         pthread_mutex_init(&mutexes[MUTEX_MALLOC], &mutex_shared_attr) != 0 ||
         pthread_mutex_init(&mutexes[MUTEX_RB    ], &mutex_shared_attr) != 0
        ) {
        printf("Failed to initialize mutices\n");
        return -2;
    }
#endif

    tsi_malloc_init(mem_base_virt, vmem_base_phys+vmem_offset, vmem_size, 1);

    //allocate ring_buffer memory
    const int RING_SIZE=1024;
    ring_buffer_str->bo = nema_buffer_create(RING_SIZE);
    (void)nema_buffer_map(&ring_buffer_str->bo);

    //Initialize Ring BUffer
    ret = nema_rb_init(ring_buffer_str, 1);

    if (ret < 0) {
        return ret;
    }

    return 0;
}

int nema_wait_irq(void)
{
    return ioctl(nema_fd, NEMA_IOCTL_WAIT_IDLE);
}

int nema_wait_irq_cl(int cl_id)
{
    unsigned long lcdarg[2];

    lcdarg[0] = NEMA_CLID;
    lcdarg[1] = cl_id;

    return ioctl(nema_fd, NEMA_IOCTL_WAIT_REG, lcdarg);

}

uint32_t  nema_reg_read(uint32_t reg) {
    nema_ioctl_readwrite_t ioctl_rw;
    ioctl_rw.reg = reg;
    ioctl(nema_fd, NEMA_IOCTL_REG_READ, &ioctl_rw);

    return ioctl_rw.value;
}

void nema_reg_write(uint32_t reg, uint32_t value) {
    nema_ioctl_readwrite_t ioctl_rw;
    ioctl_rw.reg   = reg;
    ioctl_rw.value = value;
    ioctl(nema_fd, NEMA_IOCTL_REG_WRITE, &ioctl_rw);
}

nema_buffer_t nema_buffer_create(int size) {
    nema_mutex_lock(MUTEX_MALLOC);

    nema_buffer_t bo;
    bo.base_virt = tsi_malloc(size);
    bo.base_phys = (uint32_t) tsi_virt2phys(bo.base_virt);
    bo.size      = size;
    bo.fd        = 0;

    nema_mutex_unlock(MUTEX_MALLOC);
    return bo;
}

nema_buffer_t nema_buffer_create_pool(int pool, int size) {
    //not implemented, just allocate from default pool
    return nema_buffer_create(size);
}

void *nema_buffer_map(nema_buffer_t *bo) {
    if(bo == NULL){
        nema_set_error(NEMA_ERR_INVALID_BO);
        return NULL;
    }
    return bo->base_virt;
}

void nema_buffer_unmap(nema_buffer_t *bo) {

}

void nema_buffer_destroy(nema_buffer_t *bo) {
    if(bo == NULL){
        nema_set_error(NEMA_ERR_INVALID_BO);
        return;
    }
    nema_mutex_lock(MUTEX_MALLOC);
    tsi_free(bo->base_virt);

    bo->base_virt = (void *)0;
    bo->base_phys = 0;
    bo->size      = 0;
    bo->fd        = -1;

    nema_mutex_unlock(MUTEX_MALLOC);
}

uint32_t nema_buffer_phys(nema_buffer_t *bo) {
    if(bo == NULL){
        nema_set_error(NEMA_ERR_INVALID_BO);
        return -1;
    }
    return bo->base_phys;
}

void nema_buffer_flush(nema_buffer_t * bo) {

}

void  nema_host_free     (void *ptr ) {
    free(ptr);
}

void *nema_host_malloc   (unsigned size) {
    return malloc(size);
}

int nema_mutex_lock(int mutex_id) {
#if (defined(NEMA_MULTI_PROCESS) || defined(NEMA_MULTI_THREAD))
    return pthread_mutex_lock(&mutexes[mutex_id]);
#else
    return 0;
#endif
}

int nema_mutex_unlock(int mutex_id) {
#if (defined(NEMA_MULTI_PROCESS) || defined(NEMA_MULTI_THREAD))
    return pthread_mutex_unlock(&mutexes[mutex_id]);
#else
    return 0;
#endif
}
