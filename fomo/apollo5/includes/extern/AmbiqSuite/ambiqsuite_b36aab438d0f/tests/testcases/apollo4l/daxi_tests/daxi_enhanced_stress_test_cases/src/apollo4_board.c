//*****************************************************************************
//
//! @file apollo4_board.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_hal_global.h"

//*****************************************************************************
//
// Main
//
//*****************************************************************************
void
board_init(void)
{
    am_util_stdio_printf("USB CDC Example\n\n");
}

uint32_t board_millis(void)
{
    return 0;
}

void board_led_write(bool bLedState)
{
    return;
}


