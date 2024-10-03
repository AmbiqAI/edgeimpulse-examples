#ifndef __GUI_TREE_H__   
#define __GUI_TREE_H__   
                         
#include "ng_globals.h"
                         
#ifndef EXTERN
#define EXTERN extern
#define REMEMBER_TO_UNDEFINE_EXTERN
#endif

//GItem Declaration
//---------------------------------
EXTERN gitem_screen_t _1Screen1;
	EXTERN gitem_button_t _4Label_Button1;
		EXTERN gitem_label_t _5Label1;
EXTERN gitem_screen_t _2Screen2;
	EXTERN gitem_button_t _6Label_Button2;
		EXTERN gitem_label_t _7Label2;
EXTERN gitem_screen_t _3Screen3;
	EXTERN gitem_button_t _8Label_Button3;
		EXTERN gitem_label_t _9Label3;

//---------------------------------

//Tree Node Declaration 
EXTERN tree_node_t node_1Screen1;
	EXTERN tree_node_t node_4Label_Button1;
		EXTERN tree_node_t node_5Label1;
EXTERN tree_node_t node_2Screen2;
	EXTERN tree_node_t node_6Label_Button2;
		EXTERN tree_node_t node_7Label2;
EXTERN tree_node_t node_3Screen3;
	EXTERN tree_node_t node_8Label_Button3;
		EXTERN tree_node_t node_9Label3;

//---------------------------------

#ifdef REMEMBER_TO_UNDEFINE_EXTERN
#undef EXTERN
#undef REMEMBER_TO_UNDEFINE_EXTERN
#endif

#endif //__GUI_TREE_H__
