#include "nema_core.h"

#define EXTERN
#include "images.h"
#undef EXTERN

img_obj_t _button_pressed_tsc6a;
img_obj_t _button_released_tsc6a;
img_obj_t _image_1_tsc4;
img_obj_t _image_2_tsc4;
img_obj_t _image_3_tsc4;
#include "assets/images/button_pressed_tsc6a.h"
#include "assets/images/button_released_tsc6a.h"
#include "assets/images/image_1_tsc4.h"
#include "assets/images/image_2_tsc4.h"
#include "assets/images/image_3_tsc4.h"

void load_image_assets(void) {
    _button_pressed_tsc6a.bo = NG_LOAD_ARRAY(button_pressed_tsc6a);
    _button_pressed_tsc6a.w = 80;
    _button_pressed_tsc6a.h = 40;
    _button_pressed_tsc6a.format = NEMA_TSC6A;
    _button_pressed_tsc6a.stride = -1;
    _button_pressed_tsc6a.sampling_mode = NEMA_FILTER_BL;
    _button_pressed_tsc6a.color = 0xff000000;

    _button_released_tsc6a.bo = NG_LOAD_ARRAY(button_released_tsc6a);
    _button_released_tsc6a.w = 80;
    _button_released_tsc6a.h = 40;
    _button_released_tsc6a.format = NEMA_TSC6A;
    _button_released_tsc6a.stride = -1;
    _button_released_tsc6a.sampling_mode = NEMA_FILTER_BL;
    _button_released_tsc6a.color = 0xff000000;

    _image_1_tsc4.bo = NG_LOAD_ARRAY(image_1_tsc4);
    _image_1_tsc4.w = 384;
    _image_1_tsc4.h = 384;
    _image_1_tsc4.format = NEMA_TSC4;
    _image_1_tsc4.stride = -1;
    _image_1_tsc4.sampling_mode = NEMA_FILTER_BL;
    _image_1_tsc4.color = 0xff000000;

    _image_2_tsc4.bo = NG_LOAD_ARRAY(image_2_tsc4);
    _image_2_tsc4.w = 384;
    _image_2_tsc4.h = 384;
    _image_2_tsc4.format = NEMA_TSC4;
    _image_2_tsc4.stride = -1;
    _image_2_tsc4.sampling_mode = NEMA_FILTER_BL;
    _image_2_tsc4.color = 0xff000000;

    _image_3_tsc4.bo = NG_LOAD_ARRAY(image_3_tsc4);
    _image_3_tsc4.w = 384;
    _image_3_tsc4.h = 384;
    _image_3_tsc4.format = NEMA_TSC4;
    _image_3_tsc4.stride = -1;
    _image_3_tsc4.sampling_mode = NEMA_FILTER_BL;
    _image_3_tsc4.color = 0xff000000;

}
