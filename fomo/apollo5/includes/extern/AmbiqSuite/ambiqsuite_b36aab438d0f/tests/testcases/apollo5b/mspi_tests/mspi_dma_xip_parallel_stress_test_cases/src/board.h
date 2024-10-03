//*****************************************************************************
//
//! @file board.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern void board_init(void);

extern uint32_t board_millis(void);

extern void board_led_write(bool bLedState);

#ifdef __cplusplus
}
#endif

#endif
