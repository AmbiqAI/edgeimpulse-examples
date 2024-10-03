//*****************************************************************************
//
//! @file uart_rpmsg_bridge.h
//!
//! @brief UART driver for uart hci bridge
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef UART_H_
#define UART_H_

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

// External variable definitions
//
//*****************************************************************************
extern volatile uint32_t g_ui32UARTRxIndex;
extern volatile bool g_bRxTimeoutFlag;
extern uint8_t g_pui8UARTRXBuffer[];
//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
void serial_interface_init(void);
void serial_irq_enable(void);
void serial_irq_disable(void);
void serial_data_write(uint8_t* pui8Data, uint32_t ui32Length);
#endif //UART_H_

