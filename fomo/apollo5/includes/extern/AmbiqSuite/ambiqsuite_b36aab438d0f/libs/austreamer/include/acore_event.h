/**
 ****************************************************************************************
 *
 * @file acore_event.h
 *
 * @brief This file contains the definition related to audio events.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ****************************************************************************************
 */

#ifndef _ACORE_EVENT_H_
#define _ACORE_EVENT_H_

#if defined __cplusplus
extern "C"
{
#endif

    /*
     * INCLUDE FILES
     ****************************************************************************************
     */

#include <stdint.h> // standard integer definition

    /*
     * CONSTANTS
     ****************************************************************************************
     */

    /// Status of ke_task API functions
    enum AUDIO_EVENT_STATUS
    {
        AUDIO_EVENT_OK = 0,
        AUDIO_EVENT_FAIL,
        AUDIO_EVENT_UNKNOWN,
        AUDIO_EVENT_CAPA_EXCEEDED,
    };

    /*
     * Audio Event Definition. Greater value is higher priority
     ****************************************************************************************
     */
    enum AUDIO_EVENT_TYPE
    {
        // MCPS stat event
        AUDIO_EVENT_MCPS,

        // Austreamer Core Message Event
        AUDIO_EVENT_TOOL_BUS,
        AUDIO_EVENT_CODER_BUS,
        AUDIO_EVENT_DSP_BUS,
        AUDIO_EVENT_NEED_DATA,
        AUDIO_EVENT_RPMSG_RECEIVED,

        // ACore Message Event
        AUDIO_EVENT_QUEUE, /**< This event should be small than EP MSG */
        AUDIO_EVENT_DSP_EP_MSG,
        AUDIO_EVENT_MCU_EP_MSG,
        AUDIO_EVENT_PC_EP_MSG,

        // Audio Device Event
        AUDIO_EVENT_PDM_RX_READY,
        AUDIO_EVENT_I2S0_RX_READY,
        AUDIO_EVENT_I2S0_TX_READY,
        AUDIO_EVENT_I2S1_RX_READY,
        AUDIO_EVENT_I2S1_TX_READY,

        // Control Event, Highest Priority
        AUDIO_EVENT_TEST_STATE,
        AUDIO_EVENT_A2DP_SINK_START,
        AUDIO_EVENT_A2DP_SINK_STOP,
        AUDIO_EVENT_A2DP_SRC_START,
        AUDIO_EVENT_A2DP_SRC_STOP,
        AUDIO_EVENT_HFP_HF_START,
        AUDIO_EVENT_HFP_HF_STOP,
        AUDIO_EVENT_LE_AUDIO_START,
        AUDIO_EVENT_LE_AUDIO_STOP,
        AUDIO_EVENT_RECORD_START,
        AUDIO_EVENT_RECORD_STOP,
        AUDIO_EVENT_PLAYBACK_START,
        AUDIO_EVENT_PLAYBACK_STOP,

        AUDIO_EVENT_MAX
    };
    /*
     * FUNCTION PROTOTYPES
     ****************************************************************************************
     */

    /**
     ****************************************************************************************
     * @brief Initialize Audio event module.
     * @param[in]  notify_cb       Callback function to notify event raised.
     ****************************************************************************************
     */
    void audio_event_init(void (*notify_cb)(void));

    /**
     ****************************************************************************************
     * @brief Register an event callback.
     *
     * @param[in]  event_type     Event type.
     * @param[in]  event_cb       Pointer to event callback function.
     * @param[in]  arg            Parameter will be provided to callback.
     *
     *
     * @return                      Status
     ****************************************************************************************
     */
    uint8_t audio_event_callback_set(uint8_t event_type,
                                     void (*event_cb)(void *arg), void *arg);

    /**
     ****************************************************************************************
     * @brief Unregister an event callback.
     *
     * @param[in]  event_type     Event type.
     *
     * @return                      Status
     ****************************************************************************************
     */
    uint8_t audio_event_callback_unset(uint8_t event_type);

    /**
     ****************************************************************************************
     * @brief Set an event
     *
     * This primitive sets one event. It will trigger the call to the
     *corresponding event handler in the next scheduling call.
     *
     * @param[in]  event_type      Event to be set.
     ****************************************************************************************
     */
    void audio_event_set(uint8_t event_type);

    /**
     ****************************************************************************************
     * @brief Clear an event
     *
     * @param[in]  event_type      Event to be cleared.
     ****************************************************************************************
     */
    void audio_event_clear(uint8_t event_type);

    /**
     ****************************************************************************************
     * @brief Get the status of an event
     *
     * @param[in]  event_type      Event to get.
     *
     * @return                     Event status (0: not set / 1: set)
     ****************************************************************************************
     */
    uint8_t audio_event_get(uint8_t event_type);

    /**
     ****************************************************************************************
     * @brief Event scheduler entry point.
     *
     * This primitive is the entry point of Audio event scheduling.
     ****************************************************************************************
     */
    void audio_event_schedule(void);

#if defined __cplusplus
}
#endif

#endif //_ACORE_EVENT_H_
