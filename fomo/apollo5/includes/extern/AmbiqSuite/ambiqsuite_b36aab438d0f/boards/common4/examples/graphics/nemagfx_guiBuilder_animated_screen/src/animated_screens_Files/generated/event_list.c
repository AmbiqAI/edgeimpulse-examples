//*****************************************************************************
//
//! @file event_list.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "gui_tree.h"
#include "event_list.h"
#include "ng_screen_trans.h"
#include "ng_globals.h"
#include "ng_timer.h"
#include "ng_display.h"
#include "ng_callbacks.h"
#include "nema_easing.h"
#include "custom_callbacks.h"
#include "images.h"

#define EVENT_LIST_SIZE   8
#define TEMP_ANIMATIONS   5

#define TIMER_PERIOD_MS 16

//Temprary animations - events reserved by the project (do not edit)
//----------------------------------------------------------------------------------------------------------------
static ng_node_effect_direction_t screen_transition_data = {NULL, NEMA_TRANS_LINEAR_H, 0};
static ng_act_ptr dummy_action1;
static ng_act_ptr dummy_action2;
static ng_act_ptr dummy_action3;
static ng_act_ptr dummy_action4;

static ng_transition_t temp_anim_0  = {EV_TRIGGER_NULL, EV_RETRIGGER_IGNORE, NULL, ng_screen_trans_swipable, NULL, NG_CALLBACK_DATA(&screen_transition_data), -1, EV_STATUS_STOPPED, ng_transition_handler, ng_transition_start, ng_transition_stop, ng_transition_pause_toggle, NULL, 0.f, 1.f, 2.f};
static ng_transition_t temp_anim_1  = {EV_TRIGGER_NULL, EV_RETRIGGER_IGNORE, NULL, NULL, &dummy_action1, NULL, -1, EV_STATUS_STOPPED, ng_transition_handler, ng_transition_start, ng_transition_stop, ng_transition_pause_toggle, NULL, 0.f, 1.f, 2.f};
static ng_transition_t temp_anim_2  = {EV_TRIGGER_NULL, EV_RETRIGGER_IGNORE, NULL, NULL, &dummy_action2, NULL, -1, EV_STATUS_STOPPED, ng_transition_handler, ng_transition_start, ng_transition_stop, ng_transition_pause_toggle, NULL, 0.f, 1.f, 2.f};
static ng_transition_t temp_anim_3  = {EV_TRIGGER_NULL, EV_RETRIGGER_IGNORE, NULL, NULL, &dummy_action3, NULL, -1, EV_STATUS_STOPPED, ng_transition_handler, ng_transition_start, ng_transition_stop, ng_transition_pause_toggle, NULL, 0.f, 1.f, 2.f};
static ng_transition_t temp_anim_4  = {EV_TRIGGER_NULL, EV_RETRIGGER_IGNORE, NULL, NULL, &dummy_action4, NULL, -1, EV_STATUS_STOPPED, ng_transition_handler, ng_transition_start, ng_transition_stop, ng_transition_pause_toggle, NULL, 0.f, 1.f, 2.f};
//----------------------------------------------------------------------------------------------------------------

//on_4_Label_Button1_release actions
static ng_node_effect_direction_t data_start_a_screen_transition_0 = { &node_2Screen2, NEMA_TRANS_LINEAR_H, NG_DIRECTION_RIGHT};
static ng_transition_t start_a_screen_transition_0 = {EV_TRIGGER_RELEASE, EV_RETRIGGER_IGNORE, NG_GITEM(&_4Label_Button1), ng_callback_show_screen, NULL, &data_start_a_screen_transition_0, -1, EV_STATUS_STOPPED, ng_transition_handler, ng_transition_start, ng_transition_stop, ng_transition_pause_toggle, NULL, START_TIME(0.f), DURATION(0.3f), PROGRESS(0.f)};

//on_6_Label_Button2_release actions
static ng_node_effect_direction_t data_start_a_screen_transition_1 = { &node_3Screen3, NEMA_TRANS_FADE, NG_DIRECTION_RIGHT};
static ng_transition_t start_a_screen_transition_1 = {EV_TRIGGER_RELEASE, EV_RETRIGGER_IGNORE, NG_GITEM(&_6Label_Button2), ng_callback_show_screen, NULL, &data_start_a_screen_transition_1, -1, EV_STATUS_STOPPED, ng_transition_handler, ng_transition_start, ng_transition_stop, ng_transition_pause_toggle, NULL, START_TIME(0.f), DURATION(0.3f), PROGRESS(0.f)};

//on_8_Label_Button3_release actions
static ng_node_effect_direction_t data_start_a_screen_transition_2 = { &node_1Screen1, NEMA_TRANS_FADE_ZOOM, NG_DIRECTION_RIGHT};
static ng_transition_t start_a_screen_transition_2 = {EV_TRIGGER_RELEASE, EV_RETRIGGER_IGNORE, NG_GITEM(&_8Label_Button3), ng_callback_show_screen, NULL, &data_start_a_screen_transition_2, -1, EV_STATUS_STOPPED, ng_transition_handler, ng_transition_start, ng_transition_stop, ng_transition_pause_toggle, NULL, START_TIME(0.f), DURATION(0.3f), PROGRESS(0.f)};


static ng_event_base_t* event_list[EVENT_LIST_SIZE];

void event_list_init()
{
    event_list[0] = (NG_EVENT(&temp_anim_0));
    event_list[1] = (NG_EVENT(&temp_anim_1));
    event_list[2] = (NG_EVENT(&temp_anim_2));
    event_list[3] = (NG_EVENT(&temp_anim_3));
    event_list[4] = (NG_EVENT(&temp_anim_4));

    event_list[5] = (NG_EVENT(&start_a_screen_transition_0));
    event_list[6] = (NG_EVENT(&start_a_screen_transition_1));
    event_list[7] = (NG_EVENT(&start_a_screen_transition_2));


    ng_globals_register_screen_transition_event(event_list[0]);
    ng_globals_register_event_list(event_list, EVENT_LIST_SIZE, TEMP_ANIMATIONS);
    ng_timer_set_period(TIMER_PERIOD_MS);
}
