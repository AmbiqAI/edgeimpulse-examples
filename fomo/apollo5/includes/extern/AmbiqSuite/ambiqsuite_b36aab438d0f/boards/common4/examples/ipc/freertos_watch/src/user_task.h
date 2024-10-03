//*****************************************************************************
//
//! @file user_task.h
//!
//! @brief This file describes the user task API
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef USER_TASK_H
#define USER_TASK_H

#include <stdint.h>
#include <stdbool.h>
#include "ipc_service.h"
#include "rpc_common.h"
#include "errno.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DEFINES
 ****************************************************************************************
 */
// #### INTERNAL BEGIN ####
#define STRESS_TEST_ENABLE          (1)
// #### INTERNAL END ####

#define USER_TASK_QUEUE_NUM         (2)
#define USER_TIMER_MS_MIN           (10000)
#define USER_TIMER_MS_MAX           (40000)

#define BATTERY_LVL_TIMER_MS_MIN    (15000)
#define BATTERY_LVL_TIMER_MS_MAX    (35000)

// Generate random number
#define     random_num(max, min)   ((rand() % (max - min + 1)) + min)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

typedef enum
{
    HANDLE_AUDIO_DATA_EVENT,
    HANDLE_SYS_CNTL_DATA_EVENT,
    HANDLE_USER_START_EVENT,
    HANDLE_USER_CONFIGED_EVENT,
    HANDLE_USER_START_INQUIRY,
    HANDLE_USER_STOP_INQUIRY,
    HANDLE_USER_START_INQ_SCAN,
    HANDLE_USER_DEV_CONNECT,
    HANDLE_USER_HFP_INIT_OUTGOING_CALL,
    HANDLE_USER_ANSWER_INCOMING_CALL,
    HANDLE_USER_REJECT_INCOMING_CALL,
    HANDLE_USER_HF_SET_SPEAKER_VOL,
    HANDLE_USER_AVRCP_SET_VOL,
    HANDLE_USER_AVRCP_PLAY,
    HANDLE_USER_AVRCP_STOP,
    HANDLE_USER_AVRCP_FORWARD,
    HANDLE_USER_AVRCP_BACKWARD,
    HANDLE_USER_UPDATE_BATTERY_LVL,
    HANDLE_USER_A2DP_SRC_PLAY,
    HANDLE_USER_A2DP_SRC_SUSPEND,
    HANDLE_USER_EVENT_INVALID
}eUserEvent;



/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */

// user task queue type
typedef struct
{
    uint32_t  type;
    uint8_t * data;
    uint16_t  len;
}UserQueueElem_t;



/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
extern metal_thread_t user_task_handle;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */




/**
*
* Initialize the user task queue
* Returns true for success.
*/
bool user_task_queue_create();

/**
* Send a message to the SER Queue.
*
* This may block and if it does it blocks forever.
*
* Returns true if message successfully sent.
*/
bool user_task_queue_send(UserQueueElem_t *elem);

/**
* Read a message from the user task Queue.
*
* This may block and if it does it blocks forever.
*
* Returns true if message successfully read.
*/
bool user_task_queue_read(UserQueueElem_t *elem);

/*
 ****************************************************************************************
 * @brief Function to handle the received inquiry report.
 *
 * @param[in] msg      pointer to the received inquiry report
 ****************************************************************************************
 */
void app_inquiry_report_received(uint8_t *msg);

void app_setup_timer0(uint32_t compare_val);
void app_setup_timer1(uint32_t compare_val);
void setup_buttons(void);
// #### INTERNAL BEGIN ####
#if (STRESS_TEST_ENABLE)
void app_start_timer(void);
void app_start_battery_level_timer(void);
void app_stop_battery_level_timer(void);
#endif
// #### INTERNAL END ####

#ifdef __cplusplus
}
#endif

#endif // USER_TASK_H
