//*****************************************************************************
//
//! @file host_ipc_instance.h
//!
//! @brief This file describes the host ipc instance API
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef HOST_IPC_INSTANCE_H
#define HOST_IPC_INSTANCE_H

#include <stdint.h>
#include <stdbool.h>
#include "ipc_service.h"
#include "rpc_client_sys_cntl.h"
#include "rpc_client_dbglog.h"
#include "rpc_client_audio.h"
#include "rpc_client_hci.h"
#include "rpc_common.h"
#include "errno.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DEFINES
 ****************************************************************************************
 */


#define IPC_INST0_IDX           (0)
#define IPC_INST1_IDX           (1)

#define IPC_INST0_KICK_BIT      (1 << IPC_INST0_IDX)
#define IPC_INST1_KICK_BIT      (1 << IPC_INST1_IDX)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */



/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */



/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef struct ipc_ept ipc_ept_t;
typedef struct ipc_device ipc_dev_t;
typedef struct ipc_ept_cfg ipc_ept_cfg_t;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 * host_ipc_instance_setup
 *
 * Create host instance and register endpoints.
 *
 * @param - None
 *
 * return - None
 */
void host_ipc_instance_setup(void);

/**
 * host_audio_task_dispatch
 *
 * Audio task event dispatched.  Designed to be called repeatedly from infinite loop.
 *
 * @param - NA
 *
 * return - NA
 */
void host_audio_task_dispatch(void);

/**
 * ipc_task_dispatch
 *
 * Receive task event dispatched.  Designed to be called repeatedly from infinite loop.
 *
 * @param - NA
 *
 * return - NA
 */
void ipc_task_dispatch(void);

/**
 * mailbox_isr
 *
 * ambt53 Mailbox Interrupt handler.
 *
 * @param - NA
 *
 * return - NA
 */
void mailbox_isr(void);



// #### INTERNAL BEGIN ####
#ifdef IPC_STRESS_TEST_ENABLE
/**
 * host_ipc_stress_test_setup
 *
 * Set up IPC stress test.
 *
 * @param - NA
 *
 * return - NA
 */
void host_ipc_stress_test_setup(void);
#endif
// #### INTERNAL END ####

#ifdef __cplusplus
}
#endif

#endif // HOST_IPC_INSTANCE_H
