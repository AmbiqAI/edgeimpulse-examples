#include "nema_font.h"
#include "nema_utils.h"
#include "ng_utils.h"

#define NEMA_FONT_IMPLEMENTATION
#include "assets/fonts/NotoSans_Regular70pt8b.h"
#undef NEMA_FONT_IMPLEMENTATION

#include "fonts.h"

void load_font_assets(void) {
#if 0
    NotoSans_Regular70pt8b.bo = nema_load_file("assets/fonts/NotoSans_Regular70pt8b.bin", -1, 0);
#else
    NotoSans_Regular70pt8b.bo = NG_LOAD_ARRAY(NotoSans_Regular70pt8bBitmaps);
#endif
}

