#include "am_bsp.h"
#include "am_util.h"

#include "nema_core.h"
#include "nema_event.h"
#include "nema_utils.h"

#include "ng_globals.h"
#include "ng_main_loop.h"
#include "ng_display.h"
#include "ng_event.h"
#include "ng_callbacks.h"
#include "ng_gestures.h"
#include "ng_timer.h"
#include "ng_screen_trans.h"

#include "images.h"
#include "fonts.h"
#include "framebuffers.h"
#include "event_list.h"
#include "screen_groups.h"

static int ng_init()
{
    int ret;

    load_framebuffers();
    load_image_assets();
    load_font_assets();
    init_screen_groups();
    event_list_init();
    nema_event_init(1, 0, 0, NG_RESX, NG_RESY);
    ng_event_init();

    //Check if globals (project generated) have been forwarded to the API
    ret = ng_globals_sanity_check();
    if (ret)
    {
        return ret;
    }

    //Format        | Pixclock | RESX | FP | SYNC | BP | RESY | FP | SYNC | BP
    //800x600, 60Hz | 40.000   | 800  | 40 | 128  | 88 | 600  | 1  | 4    | 23
    // nemadc_timing(800, 40, 128, 88, 600, 1, 4, 23);
    // nemadc_set_layer(0, &NG_DC_LAYER[0]);

    //Initialize display module
    ng_display_init();

    //Trigger events
    for (int i = 0; i < NG_EVENT_LIST_SIZE; ++i)
    {
        ng_event_base_t *event = NG_EVENT_LIST[i];

        if ( event->trigger == EV_TRIGGER_APP_START )
        {
            event->start(event);
        }
    }
    return 0;
}

int lunch_application()
{
#if defined(BAREMETAL) && defined(APOLLO4_FPGA) && defined(AM_PART_APOLLO4P)
    am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);
#endif
    // Init NemaGFX, NemaDC, GUI environment (GItems, assets, events)
    int ret = ng_init();
    if (ret)
    {
        return ret;
    }


    //Enter application's main loop
    ng_main_loop(0);

    nema_cl_destroy(&cl);
    nema_cl_destroy(&cl_screen);

    return 0;
}

