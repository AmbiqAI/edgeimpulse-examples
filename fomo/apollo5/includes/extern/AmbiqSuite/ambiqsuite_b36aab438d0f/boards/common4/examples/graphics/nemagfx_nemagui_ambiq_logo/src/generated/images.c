#include "nema_core.h"

#define EXTERN
#include "images.h"
#undef EXTERN

#if (defined(SCENARIO_A) || defined(SCENARIO_B))
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

void load_image_assets(void)
{
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

#elif defined(SCENARIO_C)
    img_obj_t _AmbiqSmartwatch_MainScreen_tsc4;
    img_obj_t _AmbiqSmartwatch_WatchFace_tsc4;
    img_obj_t _AmbiqSmartwatch_ContactScreen_tsc4;
    img_obj_t _SecondsHand_tsc6a;
    img_obj_t _MinutesHand_tsc6a;
    img_obj_t _HoursHand_tsc6a;
    #include "assets/images/AmbiqSmartwatch_MainScreen_tsc4.h"
    #include "assets/images/AmbiqSmartwatch_WatchFace_tsc4.h"
    #include "assets/images/AmbiqSmartwatch_ContactScreen_tsc4.h"
    #include "assets/images/SecondsHand_tsc6a.h"
    #include "assets/images/MinutesHand_tsc6a.h"
    #include "assets/images/HoursHand_tsc6a.h"

    void load_image_assets(void)
    {
        _AmbiqSmartwatch_MainScreen_tsc4.bo = NG_LOAD_ARRAY(AmbiqSmartwatch_MainScreen_tsc4);
        _AmbiqSmartwatch_MainScreen_tsc4.w = 456;
        _AmbiqSmartwatch_MainScreen_tsc4.h = 456;
        _AmbiqSmartwatch_MainScreen_tsc4.format = NEMA_TSC4;
        _AmbiqSmartwatch_MainScreen_tsc4.stride = -1;
        _AmbiqSmartwatch_MainScreen_tsc4.sampling_mode = NEMA_FILTER_BL;
        _AmbiqSmartwatch_MainScreen_tsc4.color = 0xff000000;

        _AmbiqSmartwatch_WatchFace_tsc4.bo = NG_LOAD_ARRAY(AmbiqSmartwatch_WatchFace_tsc4);
        _AmbiqSmartwatch_WatchFace_tsc4.w = 456;
        _AmbiqSmartwatch_WatchFace_tsc4.h = 456;
        _AmbiqSmartwatch_WatchFace_tsc4.format = NEMA_TSC4;
        _AmbiqSmartwatch_WatchFace_tsc4.stride = -1;
        _AmbiqSmartwatch_WatchFace_tsc4.sampling_mode = NEMA_FILTER_BL;
        _AmbiqSmartwatch_WatchFace_tsc4.color = 0xff000000;

        _AmbiqSmartwatch_ContactScreen_tsc4.bo = NG_LOAD_ARRAY(AmbiqSmartwatch_ContactScreen_tsc4);
        _AmbiqSmartwatch_ContactScreen_tsc4.w = 456;
        _AmbiqSmartwatch_ContactScreen_tsc4.h = 456;
        _AmbiqSmartwatch_ContactScreen_tsc4.format = NEMA_TSC4;
        _AmbiqSmartwatch_ContactScreen_tsc4.stride = -1;
        _AmbiqSmartwatch_ContactScreen_tsc4.sampling_mode = NEMA_FILTER_BL;
        _AmbiqSmartwatch_ContactScreen_tsc4.color = 0xff000000;

        _SecondsHand_tsc6a.bo = NG_LOAD_ARRAY(SecondsHand_tsc6a);
        _SecondsHand_tsc6a.w = 44;
        _SecondsHand_tsc6a.h = 192;
        _SecondsHand_tsc6a.format = NEMA_TSC6A;
        _SecondsHand_tsc6a.stride = -1;
        _SecondsHand_tsc6a.sampling_mode = NEMA_FILTER_BL;
        _SecondsHand_tsc6a.color = 0xff000000;

        _MinutesHand_tsc6a.bo = NG_LOAD_ARRAY(MinutesHand_tsc6a);
        _MinutesHand_tsc6a.w = 32;
        _MinutesHand_tsc6a.h = 192;
        _MinutesHand_tsc6a.format = NEMA_TSC6A;
        _MinutesHand_tsc6a.stride = -1;
        _MinutesHand_tsc6a.sampling_mode = NEMA_FILTER_BL;
        _MinutesHand_tsc6a.color = 0xff000000;

        _HoursHand_tsc6a.bo = NG_LOAD_ARRAY(HoursHand_tsc6a);
        _HoursHand_tsc6a.w = 32;
        _HoursHand_tsc6a.h = 164;
        _HoursHand_tsc6a.format = NEMA_TSC6A;
        _HoursHand_tsc6a.stride = -1;
        _HoursHand_tsc6a.sampling_mode = NEMA_FILTER_BL;
        _HoursHand_tsc6a.color = 0xff000000;
    }
#endif