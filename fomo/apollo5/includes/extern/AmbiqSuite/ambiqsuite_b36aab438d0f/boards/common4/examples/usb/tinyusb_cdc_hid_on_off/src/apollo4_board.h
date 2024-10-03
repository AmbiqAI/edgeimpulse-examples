//*****************************************************************************
//
//! @file board.h
//!
//! @brief contains prototypes  for the apollo4_board.c file
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

//*****************************************************************************
//
//! @brief preform a init on common software components
//!
//! @param pcStartupString   string identifying this particular board
//
//*****************************************************************************
extern void board_init(char * pcStartupString);

//*****************************************************************************
//
//! @brief  This function is unused
//! @return
//
//*****************************************************************************
extern uint32_t board_millis(void);

//*****************************************************************************
//
//! @brief This will change the state of the LED
//! @param bLedState leds tate
//
//*****************************************************************************
extern void board_led_write(bool bLedState);

#ifdef __cplusplus
}
#endif

#endif
