#include "nema_font.h"
#include "nema_utils.h"
#include "ng_utils.h"

#define NEMA_FONT_IMPLEMENTATION
#include "assets/fonts/arial8pt8b.h"
#include "assets/fonts/arial6pt8b.h"
#include "assets/fonts/arial14pt8b.h"
#undef NEMA_FONT_IMPLEMENTATION

#include "fonts.h"

void load_font_assets(void) {
#if 0
    arial8pt8b.bo = nema_load_file("assets/fonts/arial8pt8b.bin", -1, 0);
    arial6pt8b.bo = nema_load_file("assets/fonts/arial6pt8b.bin", -1, 0);
    arial14pt8b.bo = nema_load_file("assets/fonts/arial14pt8b.bin", -1, 0);
#else
    arial8pt8b.bo = NG_LOAD_ARRAY(arial8pt8bBitmaps);
    arial6pt8b.bo = NG_LOAD_ARRAY(arial6pt8bBitmaps);
    arial14pt8b.bo = NG_LOAD_ARRAY(arial14pt8bBitmaps);
#endif
}

