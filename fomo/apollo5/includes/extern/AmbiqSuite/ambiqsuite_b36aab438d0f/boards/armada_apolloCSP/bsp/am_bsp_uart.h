//*****************************************************************************
//
//! @file am_bsp_uart.h
//!
//! @brief Functions for configuring the UART for the board.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_BSP_UART_H
#define AM_BSP_UART_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// UART definitions.
//
//*****************************************************************************
//
// Apollo has a single UART instance.
//
#define AM_BSP_UART_PRINT_INST          0
#define AM_BSP_UART_BTLE_INST           0

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern int am_bsp_uart_init(void);

#ifdef __cplusplus
}
#endif

#endif // AM_BSP_UART_H
