#include "nema_core.h"
#include "nema_event.h"
#include "nema_utils.h"

#include "nema_dc_hal.h"
#include "nema_dc.h"

#include "ng_globals.h"
#include "ng_main_loop.h"
#include "ng_display.h"
#include "ng_event.h"

#include "framebuffers.h"
#include "images.h"
#include "fonts.h"
#include "event_list.h"
#include "screen_groups.h"

static int init() {
    int ret;
    //Initialize NemaGFX
    ret = nema_init();
    if (ret) return ret;
    //Initialize Nema|dc
    ret = nemadc_init();
    if (ret) return ret;

    //Load framebuffers to initialize NG_DC_LAYER, NG_RESX, NG_RESY
    load_framebuffers();
    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    nemadc_set_layer(0, &NG_DC_LAYER[0]);

    nema_event_init(1, 0, 0, NG_RESX, NG_RESY);

    //Initialize NemaGUI
    load_image_assets();
    load_font_assets();
    init_screen_groups();
    event_list_init();

    ng_event_init();
    ng_display_init();

    //Check if globals (project generated) have been registered properly to the API
    ret = ng_globals_sanity_check();
    if (ret) return ret;

    return 0;
}

int main()
{
    // Init NemaGFX, NemaDC, NemaGUI (GItems, assets, events)
    int ret = init();
    if (ret) return ret;

    //Enter NemaGUI application's main loop
    ng_main_loop(0);

    nema_cl_destroy(&cl);
    nema_cl_destroy(&cl_screen);

    return 0;
}
