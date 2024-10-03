#include "nema_core.h"

#define EXTERN
#include "images.h"
#undef EXTERN

img_obj_t _speedo_needle_a8;
img_obj_t _rpm_scaled_rgba5551;
img_obj_t _fuel2_a8;
#include "assets/images/speedo_needle_a8.h"
#include "assets/images/rpm_scaled_rgba5551.h"
#include "assets/images/fuel2_a8.h"

void load_image_assets(void) {
    _speedo_needle_a8.bo = NG_LOAD_ARRAY(speedo_needle_a8);
    _speedo_needle_a8.w = 60;
    _speedo_needle_a8.h = 20;
    _speedo_needle_a8.format = NEMA_A8;
    _speedo_needle_a8.stride = -1;
    _speedo_needle_a8.sampling_mode = NEMA_FILTER_BL;
    _speedo_needle_a8.color = 0xff0000ff;

    _rpm_scaled_rgba5551.bo = NG_LOAD_ARRAY(rpm_scaled_rgba5551);
    _rpm_scaled_rgba5551.w = 240;
    _rpm_scaled_rgba5551.h = 240;
    _rpm_scaled_rgba5551.format = NEMA_RGBA5551;
    _rpm_scaled_rgba5551.stride = -1;
    _rpm_scaled_rgba5551.sampling_mode = NEMA_FILTER_BL;
    _rpm_scaled_rgba5551.color = 0xff000000;

    _fuel2_a8.bo = NG_LOAD_ARRAY(fuel2_a8);
    _fuel2_a8.w = 79;
    _fuel2_a8.h = 74;
    _fuel2_a8.format = NEMA_A8;
    _fuel2_a8.stride = -1;
    _fuel2_a8.sampling_mode = NEMA_FILTER_BL;
    _fuel2_a8.color = 0xffffffff;

}
