#include "nema_core.h"

#define EXTERN
#include "images.h"
#undef EXTERN

img_obj_t _AmbiqSmartwatch_ContactScreen_390x390_tsc4;
img_obj_t _AmbiqSmartwatch_MainScreen_390x390_tsc4;
img_obj_t _AmbiqSmartwatch_WatchFace_390x390_tsc4;
img_obj_t _HoursHand_390x390_tsc6a;
img_obj_t _MinutesHand_390x390_tsc6a;
img_obj_t _SecondsHand_390x390_tsc6a;
#include "assets/images/AmbiqSmartwatch_ContactScreen_390x390_tsc4.h"
#include "assets/images/AmbiqSmartwatch_MainScreen_390x390_tsc4.h"
#include "assets/images/AmbiqSmartwatch_WatchFace_390x390_tsc4.h"
#include "assets/images/HoursHand_390x390_tsc6a.h"
#include "assets/images/MinutesHand_390x390_tsc6a.h"
#include "assets/images/SecondsHand_390x390_tsc6a.h"

void load_image_assets(void) {
    _AmbiqSmartwatch_ContactScreen_390x390_tsc4.bo = NG_LOAD_ARRAY(AmbiqSmartwatch_ContactScreen_390x390_tsc4);
    _AmbiqSmartwatch_ContactScreen_390x390_tsc4.w = 392;
    _AmbiqSmartwatch_ContactScreen_390x390_tsc4.h = 392;
    _AmbiqSmartwatch_ContactScreen_390x390_tsc4.format = NEMA_TSC4;
    _AmbiqSmartwatch_ContactScreen_390x390_tsc4.stride = -1;
    _AmbiqSmartwatch_ContactScreen_390x390_tsc4.sampling_mode = NEMA_FILTER_BL;
    _AmbiqSmartwatch_ContactScreen_390x390_tsc4.color = 0xff000000;

    _AmbiqSmartwatch_MainScreen_390x390_tsc4.bo = NG_LOAD_ARRAY(AmbiqSmartwatch_MainScreen_390x390_tsc4);
    _AmbiqSmartwatch_MainScreen_390x390_tsc4.w = 392;
    _AmbiqSmartwatch_MainScreen_390x390_tsc4.h = 392;
    _AmbiqSmartwatch_MainScreen_390x390_tsc4.format = NEMA_TSC4;
    _AmbiqSmartwatch_MainScreen_390x390_tsc4.stride = -1;
    _AmbiqSmartwatch_MainScreen_390x390_tsc4.sampling_mode = NEMA_FILTER_BL;
    _AmbiqSmartwatch_MainScreen_390x390_tsc4.color = 0xff000000;

    _AmbiqSmartwatch_WatchFace_390x390_tsc4.bo = NG_LOAD_ARRAY(AmbiqSmartwatch_WatchFace_390x390_tsc4);
    _AmbiqSmartwatch_WatchFace_390x390_tsc4.w = 392;
    _AmbiqSmartwatch_WatchFace_390x390_tsc4.h = 392;
    _AmbiqSmartwatch_WatchFace_390x390_tsc4.format = NEMA_TSC4;
    _AmbiqSmartwatch_WatchFace_390x390_tsc4.stride = -1;
    _AmbiqSmartwatch_WatchFace_390x390_tsc4.sampling_mode = NEMA_FILTER_BL;
    _AmbiqSmartwatch_WatchFace_390x390_tsc4.color = 0xff000000;

    _HoursHand_390x390_tsc6a.bo = NG_LOAD_ARRAY(HoursHand_390x390_tsc6a);
    _HoursHand_390x390_tsc6a.w = 28;
    _HoursHand_390x390_tsc6a.h = 140;
    _HoursHand_390x390_tsc6a.format = NEMA_TSC6A;
    _HoursHand_390x390_tsc6a.stride = -1;
    _HoursHand_390x390_tsc6a.sampling_mode = NEMA_FILTER_BL;
    _HoursHand_390x390_tsc6a.color = 0xff000000;

    _MinutesHand_390x390_tsc6a.bo = NG_LOAD_ARRAY(MinutesHand_390x390_tsc6a);
    _MinutesHand_390x390_tsc6a.w = 28;
    _MinutesHand_390x390_tsc6a.h = 164;
    _MinutesHand_390x390_tsc6a.format = NEMA_TSC6A;
    _MinutesHand_390x390_tsc6a.stride = -1;
    _MinutesHand_390x390_tsc6a.sampling_mode = NEMA_FILTER_BL;
    _MinutesHand_390x390_tsc6a.color = 0xff000000;

    _SecondsHand_390x390_tsc6a.bo = NG_LOAD_ARRAY(SecondsHand_390x390_tsc6a);
    _SecondsHand_390x390_tsc6a.w = 40;
    _SecondsHand_390x390_tsc6a.h = 164;
    _SecondsHand_390x390_tsc6a.format = NEMA_TSC6A;
    _SecondsHand_390x390_tsc6a.stride = -1;
    _SecondsHand_390x390_tsc6a.sampling_mode = NEMA_FILTER_BL;
    _SecondsHand_390x390_tsc6a.color = 0xff000000;

}
