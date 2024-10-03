#ifndef __NG_IMAGES_H__
#define __NG_IMAGES_H__

#include "nema_utils.h"

#include "ng_utils.h"

#ifndef EXTERN
#define EXTERN extern
#define REMEMBER_TO_UNDEFINE_EXTERN
#endif

EXTERN img_obj_t _speedo_needle_a8;
EXTERN img_obj_t _rpm_scaled_rgba5551;
EXTERN img_obj_t _fuel2_a8;

void load_image_assets(void);

#ifdef REMEMBER_TO_UNDEFINE_EXTERN
#undef EXTERN
#undef REMEMBER_TO_UNDEFINE_EXTERN
#endif

#endif  //__NG_IMAGES_H__ 
