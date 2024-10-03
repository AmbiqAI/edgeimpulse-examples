//*****************************************************************************
//
//! @file user_task.h
//!
//! @brief Functions and variables related to the user ui task.
//!
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

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * DEFINES
 ****************************************************************************************
 */
#define USER_TASK_QUEUE_NUM (2)

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

        HANDLE_USER_EVENT_INVALID
    } eUserEvent;

    /*
     * STRUCTURE DEFINITIONS
     ****************************************************************************************
     */

    // user task queue type
    typedef struct
    {
        uint32_t type;
        uint8_t *data;
        uint16_t len;
    } UserQueueElem_t;

    /**
     *
     * Initialize the user task queue
     * Returns true for success.
     */
    bool user_task_queue_create();

    /**
     * Send a message to the SET Queue.
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

    void user_task_setup(void);

#ifdef __cplusplus
}
#endif

#endif // USER_TASK_H
