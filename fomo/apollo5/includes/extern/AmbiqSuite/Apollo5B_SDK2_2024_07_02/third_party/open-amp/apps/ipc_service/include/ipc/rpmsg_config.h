/*
 * Copyright (c) 2022, Ambiq Micro, Inc.
 *
 */

#ifndef RPMSG_CONFIG_H__
#define RPMSG_CONFIG_H__
#include "virtio.h"
#include "am_devices_ambt53.h"
/*----------------------------------------------------------------------------------------
|  STATUS  |     RX BUFFER(VQ0)     |     TX BUFFER(VQ0)     |  RX VRING  |  TX VRING  |
----------------------------------------------------------------------------------------*/
// RPMSG SERVICE SHARE MEMORY: STATUS + RX BUFFER + TX BUFFER + RX VRING + TX VRING
// SHARE MEMORY: RX BUFFER + TX BUFFER + RX VRING + TX VRING
// SHARE BUFFER: RX BUFFER + TX BUFFER

/**************************Common parameters for all instance**************************/
/** Number of used VRING buffers. */
#define VRING_COUNT	        (2)
/** VRING alignment. */
#define MEM_ALIGNMENT       (4u)
/* Element number in one VRING buffer */
#define VRING_SIZE          (8u)
/* Size of status region */
#define VDEV_STATUS_SIZE	(4)

#define SHM_DEVICE_NAME     "rpmsg.shm"

#define VIRTQUEUE_ID_HOST	(0)
#define VIRTQUEUE_ID_REMOTE	(1)
/* Role definition for application usage */
#define ROLE_HOST   VIRTIO_DEV_DRIVER
#define ROLE_REMOTE VIRTIO_DEV_DEVICE

/** Number of endpoints one instance supports. */
#define NUM_ENDPOINTS	    (5)
/** Number of instances the application supports. */
#define NUM_INSTANCES	    (4)

// Use vring_size(unsigned int num, unsigned long align) in virtio_ring.h to test specific vring's size
// Size of two vring with 16 elements is 860Bytes(aligned with 4)
// Size of two vring with 8 elements is 444Bytes(aligned with 4)
// Size of one vring with 1 elements is 38Bytes(aligned with 4), ipc_service is using this as unit(check optimal_num_desc)
#define VRING_BUFFER_SIZE   (38u)
//RPMSG_BUFFER_SIZE is defined in virtio.h, default is 512u
#define CONFIG_IPC_SHM_SIZE         (RPMSG_BUFFER_SIZE * VRING_SIZE * VRING_COUNT + VDEV_STATUS_SIZE + VRING_BUFFER_SIZE * VRING_SIZE * VRING_COUNT)

/* Virtual address offset of share memory */
#ifdef MSPI_XIPMM_BASE_ADDRESS
#define RPMSG_SHM_VIRT_ADDR_OFFSET        (MSPI_XIPMM_BASE_ADDRESS)
#else
#define RPMSG_SHM_VIRT_ADDR_OFFSET        (0)
#endif

/* Convert the share memory virtual address to physical address */
#define RPMSG_SHM_ADDR_VIRT_TO_PHYS(addr) ((addr) - RPMSG_SHM_VIRT_ADDR_OFFSET)
/* Convert the share memory physical address to virtual address */
#define RPMSG_SHM_ADDR_PHYS_TO_VIRT(addr) ((addr) + RPMSG_SHM_VIRT_ADDR_OFFSET)

// XIPMM read instruction will get 16 bytes per packet, need to keep the RPMSG 
// buffers 16 bytes aligned to prevent AHB bus alignment issue.
#define ADDR_ALIGN(addr) ((addr) - (((addr) + VDEV_STATUS_SIZE) % 16))

/****************************Instance specific parameters****************************/
///IMPORTANT NOTE: should reserve this SHM area from link script to avoid the SW access it by accident
/* Base virtual address of Share memory */
#define RPMSG_SHM_BASE_VIRT_ADDRESS       (RPMSG_SHM_VIRT_ADDR_OFFSET + SRAM_BANK3_OFFSET + SRAM_BANK3_SIZE - CONFIG_IPC_SHM_SIZE)
/* Instance 0 Share memory Configuration defines */
#define CONFIG_IPC_SHM_BASE_ADDRESS_INST0 (ADDR_ALIGN(RPMSG_SHM_BASE_VIRT_ADDRESS))
/* Instance 1 Share memory Configuration defines */
#define CONFIG_IPC_SHM_BASE_ADDRESS_INST1 (ADDR_ALIGN(CONFIG_IPC_SHM_BASE_ADDRESS_INST0 - CONFIG_IPC_SHM_SIZE))

/*Mailbox interrupt data for instances, the host and remote should be opposite in one CPU for simulation*/
/*The host and remote kick data can be the same on two CPUs' real system*/
#define HOST_KICK_DATA_INST0          (0xA868)
#define REMOTE_KICK_DATA_INST0        (0xA869)
#define HOST_KICK_DATA_INST1          (0xA86A)
#define REMOTE_KICK_DATA_INST1        (0xA86B)

#endif
