#ifndef __GUI_TREE_H__   
#define __GUI_TREE_H__   
                         
#include "ng_globals.h"
                         
#ifndef EXTERN
#define EXTERN extern
#define REMEMBER_TO_UNDEFINE_EXTERN
#endif

//GItem Declaration
//---------------------------------
EXTERN gitem_screen_t _1149Gauge;
	EXTERN gitem_gauge_t _1244Gauge1;
		EXTERN gitem_image_t _1250Image1;
		EXTERN gitem_digimeter_t _1251Digital_Meter2;
		EXTERN gitem_label_t _1253Label1;
		EXTERN gitem_needle_t _1245Needle;

//---------------------------------

//Tree Node Declaration 
EXTERN tree_node_t node_1149Gauge;
	EXTERN tree_node_t node_1244Gauge1;
		EXTERN tree_node_t node_1250Image1;
		EXTERN tree_node_t node_1251Digital_Meter2;
		EXTERN tree_node_t node_1253Label1;
		EXTERN tree_node_t node_1245Needle;

//---------------------------------

#ifdef REMEMBER_TO_UNDEFINE_EXTERN
#undef EXTERN
#undef REMEMBER_TO_UNDEFINE_EXTERN
#endif

#endif //__GUI_TREE_H__
