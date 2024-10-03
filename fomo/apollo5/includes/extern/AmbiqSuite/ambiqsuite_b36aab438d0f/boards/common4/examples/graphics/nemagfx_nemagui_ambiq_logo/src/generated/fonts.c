#include "nema_font.h"
#include "nema_utils.h"
#include "ng_utils.h"

#define NEMA_FONT_IMPLEMENTATION
    #if defined(SCENARIO_A)
        #include "assets/fonts/NotoSans_Regular70pt8b.h"
    #elif defined(SCENARIO_B)
        #include "assets/fonts/NotoSans_Regular70pt8b.h"
    #elif defined(SCENARIO_C)
        #include "assets/fonts/NotoSans_Regular80pt8b.h"
    #endif
#undef NEMA_FONT_IMPLEMENTATION

#include "fonts.h"

void load_font_assets(void)
{
#if 0
    NotoSans_Regular80pt8b.bo = nema_load_file("assets/fonts/NotoSans_Regular80pt8b.bin", -1, 0);
#else
    #if defined(SCENARIO_A)
        NotoSans_Regular70pt8b.bo = NG_LOAD_ARRAY(NotoSans_Regular70pt8bBitmaps);
    #elif defined(SCENARIO_B)
        NotoSans_Regular70pt8b.bo = NG_LOAD_ARRAY(NotoSans_Regular70pt8bBitmaps);
    #elif defined(SCENARIO_C)
        NotoSans_Regular80pt8b.bo = NG_LOAD_ARRAY(NotoSans_Regular80pt8bBitmaps);
    #endif
#endif
}

