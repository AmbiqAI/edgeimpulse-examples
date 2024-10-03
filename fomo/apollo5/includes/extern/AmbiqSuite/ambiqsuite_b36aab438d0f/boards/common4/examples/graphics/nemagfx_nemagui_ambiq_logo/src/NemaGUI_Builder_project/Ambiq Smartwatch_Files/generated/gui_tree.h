#ifndef __GUI_TREE_H__   
#define __GUI_TREE_H__   
                         
#include "ng_globals.h"
                         
#ifndef EXTERN
#define EXTERN extern
#define REMEMBER_TO_UNDEFINE_EXTERN
#endif

//GItem Declaration
//---------------------------------
EXTERN gitem_screen_t _1MainScreen;
	EXTERN gitem_image_t _7MainWatchFace;
EXTERN gitem_screen_t _3AnalogWatchFaceScreen;
	EXTERN gitem_watchface_t _4AnalogWatchFace;
		EXTERN gitem_needle_t _20HoursHand;
		EXTERN gitem_needle_t _19MinutesHand;
		EXTERN gitem_needle_t _18SecondsHand;
EXTERN gitem_screen_t _5DigitalWatchFaceScreen;
	EXTERN gitem_image_t _10DigitalWatchFace;
	EXTERN gitem_digital_clock_t _17DigitalClock;
EXTERN gitem_screen_t _6ContactScreen;
	EXTERN gitem_image_t _9ContactWatchFace;

//---------------------------------

//Tree Node Declaration 
EXTERN tree_node_t node_1MainScreen;
	EXTERN tree_node_t node_7MainWatchFace;
EXTERN tree_node_t node_3AnalogWatchFaceScreen;
	EXTERN tree_node_t node_4AnalogWatchFace;
		EXTERN tree_node_t node_20HoursHand;
		EXTERN tree_node_t node_19MinutesHand;
		EXTERN tree_node_t node_18SecondsHand;
EXTERN tree_node_t node_5DigitalWatchFaceScreen;
	EXTERN tree_node_t node_10DigitalWatchFace;
	EXTERN tree_node_t node_17DigitalClock;
EXTERN tree_node_t node_6ContactScreen;
	EXTERN tree_node_t node_9ContactWatchFace;

//---------------------------------

#ifdef REMEMBER_TO_UNDEFINE_EXTERN
#undef EXTERN
#undef REMEMBER_TO_UNDEFINE_EXTERN
#endif

#endif //__GUI_TREE_H__
