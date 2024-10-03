#ifndef __NG_IMAGES_H__
#define __NG_IMAGES_H__

#include "nema_utils.h"

#include "ng_utils.h"

#ifndef EXTERN
#define EXTERN extern
#define REMEMBER_TO_UNDEFINE_EXTERN
#endif

#if (defined(SCENARIO_A) || defined(SCENARIO_B))
    EXTERN img_obj_t _AmbiqSmartwatch_ContactScreen_390x390_tsc4;
    EXTERN img_obj_t _AmbiqSmartwatch_MainScreen_390x390_tsc4;
    EXTERN img_obj_t _AmbiqSmartwatch_WatchFace_390x390_tsc4;
    EXTERN img_obj_t _HoursHand_390x390_tsc6a;
    EXTERN img_obj_t _MinutesHand_390x390_tsc6a;
    EXTERN img_obj_t _SecondsHand_390x390_tsc6a;
#elif defined(SCENARIO_C)
    EXTERN img_obj_t _AmbiqSmartwatch_MainScreen_tsc4;
    EXTERN img_obj_t _AmbiqSmartwatch_WatchFace_tsc4;
    EXTERN img_obj_t _AmbiqSmartwatch_ContactScreen_tsc4;
    EXTERN img_obj_t _SecondsHand_tsc6a;
    EXTERN img_obj_t _MinutesHand_tsc6a;
    EXTERN img_obj_t _HoursHand_tsc6a;
#endif

void load_image_assets(void);

#ifdef REMEMBER_TO_UNDEFINE_EXTERN
#undef EXTERN
#undef REMEMBER_TO_UNDEFINE_EXTERN
#endif

#endif  //__NG_IMAGES_H__
