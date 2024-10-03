#ifndef __NG_FONTS_H__
#define __NG_FONTS_H__

#include "nema_font.h"

#if defined(SCENARIO_A)
    #include "assets/fonts/NotoSans_Regular70pt8b.h"
#elif defined(SCENARIO_B)
    #include "assets/fonts/NotoSans_Regular70pt8b.h"
#elif defined(SCENARIO_C)
    #include "assets/fonts/NotoSans_Regular80pt8b.h"
#endif

void load_font_assets(void);

#endif //__NG_FONTS_H__
