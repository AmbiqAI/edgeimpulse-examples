#undef EXTERN
#include "gui_tree.h"
#undef EXTERN

#include "ng_globals.h"
#include "fonts.h"
#include "images.h"

gitem_screen_t _1Screen1 = {EVENT(NULL), ng_screen_draw, GESTURES(NULL), FLAGS(0x00000056U), X(0), Y(0), W(384), H(384), ID(1), GITEM_MAIN_SCREEN , COLOR(0xff000000U) , IMAGE(&_image_1_tsc4)};
	gitem_button_t _4Label_Button1 = {EVENT(NULL), ng_button_draw, GESTURES(NULL), FLAGS(0x00000046U), X(152), Y(280), W(80), H(40), ID(4), GITEM_LABEL_BUTTON , COLOR(0xff969600U) ,COLOR(0xffb5b54cU), IMAGE(&_button_released_tsc6a), IMAGE(&_button_pressed_tsc6a), PEN_WIDTH(0)};
		char *_5Label1_texts[1] = {"Screen 1"};
		nema_font_t *_5Label1_fonts[1] = {&NotoSans_Regular12pt8b};
		attr_text_t _5Label1_text = {0, _5Label1_fonts, _5Label1_texts, NEMA_ALIGNX_CENTER | NEMA_ALIGNY_CENTER};
		gitem_label_t _5Label1 = {EVENT(NULL), ng_label_draw, GESTURES(NULL), FLAGS(0x00000080U), X(0), Y(0), W(80), H(40), ID(5), GITEM_LABEL , COLOR(0xffffffffU) , &_5Label1_text, COLOR(0xffffffffU)};

gitem_screen_t _2Screen2 = {EVENT(NULL), ng_screen_draw, GESTURES(NULL), FLAGS(0x00000056U), X(0), Y(0), W(384), H(384), ID(2), GITEM_MAIN_SCREEN , COLOR(0xff000000U) , IMAGE(&_image_2_tsc4)};
	gitem_button_t _6Label_Button2 = {EVENT(NULL), ng_button_draw, GESTURES(NULL), FLAGS(0x00000046U), X(152), Y(280), W(80), H(40), ID(6), GITEM_LABEL_BUTTON , COLOR(0xff969600U) ,COLOR(0xffb5b54cU), IMAGE(&_button_released_tsc6a), IMAGE(&_button_pressed_tsc6a), PEN_WIDTH(0)};
		char *_7Label2_texts[1] = {"Screen 2"};
		nema_font_t *_7Label2_fonts[1] = {&NotoSans_Regular12pt8b};
		attr_text_t _7Label2_text = {0, _7Label2_fonts, _7Label2_texts, NEMA_ALIGNX_CENTER | NEMA_ALIGNY_CENTER};
		gitem_label_t _7Label2 = {EVENT(NULL), ng_label_draw, GESTURES(NULL), FLAGS(0x00000080U), X(0), Y(0), W(80), H(40), ID(7), GITEM_LABEL , COLOR(0xffffffffU) , &_7Label2_text, COLOR(0xffffffffU)};

gitem_screen_t _3Screen3 = {EVENT(NULL), ng_screen_draw, GESTURES(NULL), FLAGS(0x00000056U), X(0), Y(0), W(384), H(384), ID(3), GITEM_MAIN_SCREEN , COLOR(0xff000000U) , IMAGE(&_image_3_tsc4)};
	gitem_button_t _8Label_Button3 = {EVENT(NULL), ng_button_draw, GESTURES(NULL), FLAGS(0x00000046U), X(152), Y(280), W(80), H(40), ID(8), GITEM_LABEL_BUTTON , COLOR(0xff969600U) ,COLOR(0xffb5b54cU), IMAGE(&_button_released_tsc6a), IMAGE(&_button_pressed_tsc6a), PEN_WIDTH(0)};
		char *_9Label3_texts[1] = {"Screen 0"};
		nema_font_t *_9Label3_fonts[1] = {&NotoSans_Regular12pt8b};
		attr_text_t _9Label3_text = {0, _9Label3_fonts, _9Label3_texts, NEMA_ALIGNX_CENTER | NEMA_ALIGNY_CENTER};
		gitem_label_t _9Label3 = {EVENT(NULL), ng_label_draw, GESTURES(NULL), FLAGS(0x00000080U), X(0), Y(0), W(80), H(40), ID(9), GITEM_LABEL , COLOR(0xffffffffU) , &_9Label3_text, COLOR(0xffffffffU)};


//---------------------------------


//Build tree nodes
//---------------------------------
tree_node_t node_1Screen1 = { NG_GITEM(&_1Screen1), PARENT_NODE(NULL), FIRST_CHILD_NODE(&node_4Label_Button1), NEXT_NODE(NULL)};
	tree_node_t node_4Label_Button1 = { NG_GITEM(&_4Label_Button1), PARENT_NODE(&node_1Screen1), FIRST_CHILD_NODE(&node_5Label1), NEXT_NODE(NULL)};
		tree_node_t node_5Label1 = { NG_GITEM(&_5Label1), PARENT_NODE(&node_4Label_Button1), FIRST_CHILD_NODE(NULL), NEXT_NODE(NULL)};

//Build tree nodes
//---------------------------------
tree_node_t node_2Screen2 = { NG_GITEM(&_2Screen2), PARENT_NODE(NULL), FIRST_CHILD_NODE(&node_6Label_Button2), NEXT_NODE(NULL)};
	tree_node_t node_6Label_Button2 = { NG_GITEM(&_6Label_Button2), PARENT_NODE(&node_2Screen2), FIRST_CHILD_NODE(&node_7Label2), NEXT_NODE(NULL)};
		tree_node_t node_7Label2 = { NG_GITEM(&_7Label2), PARENT_NODE(&node_6Label_Button2), FIRST_CHILD_NODE(NULL), NEXT_NODE(NULL)};

//Build tree nodes
//---------------------------------
tree_node_t node_3Screen3 = { NG_GITEM(&_3Screen3), PARENT_NODE(NULL), FIRST_CHILD_NODE(&node_8Label_Button3), NEXT_NODE(NULL)};
	tree_node_t node_8Label_Button3 = { NG_GITEM(&_8Label_Button3), PARENT_NODE(&node_3Screen3), FIRST_CHILD_NODE(&node_9Label3), NEXT_NODE(NULL)};
		tree_node_t node_9Label3 = { NG_GITEM(&_9Label3), PARENT_NODE(&node_8Label_Button3), FIRST_CHILD_NODE(NULL), NEXT_NODE(NULL)};
