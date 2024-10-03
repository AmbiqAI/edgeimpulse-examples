//*****************************************************************************
//
//! @file uart_async_utils.c
//!
//! @brief Functions to setup and manage the uart asynchronous driver
//!
//! @addtogroup peripheral_examples Peripheral Examples
//
//! @defgroup uart_async UART Asynchronous Example
//! @ingroup peripheral_examples
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "uart_async_utils.h"

//
//! @brief array of structs that holds data and setups for each uart
//
uart_async_local_vars_t gs_UartVars[eMAX_UARTS];

//
//! used for gpio pin map for each uart
//
typedef struct
{
    uint32_t pinNUmber;
    const am_hal_gpio_pincfg_t *pinCfg;
}
uart_pin_descr_t;

//
//! descriptor for each uart
//
typedef struct
{
    am_hal_gpio_pincfg_t *pinCfg;
    uart_pin_descr_t rxPin;
    uart_pin_descr_t txPin;
    const am_hal_uart_config_t *pUartConfigs;
    uart_id_e uartId;
    uint8_t uartHwIndex;
}
uart_defs_t;

//
//! uart setup data/table
//
static const am_hal_uart_config_t sUartConfig =
{
    //
    //! Standard UART settings: 115200-8-N-1
    //
    .ui32BaudRate    = 115200,
    .ui32DataBits    = AM_HAL_UART_DATA_BITS_8,
    .ui32Parity      = AM_HAL_UART_PARITY_NONE,
    .ui32StopBits    = AM_HAL_UART_ONE_STOP_BIT,
    .ui32FlowControl = AM_HAL_UART_FLOW_CTRL_NONE,

    .ui32FifoLevels =  AM_HAL_UART_TX_FIFO_7_8 | AM_HAL_UART_RX_FIFO_1_2,
};

//
//! Uart pin definitions
//! This defaults with standard BSP pin definitions.
//
static const uart_defs_t gs_UartPinDefs[eMAX_UARTS] =
{
#if NUM_UARTS_USED > 0
        [eUART0] = {
        .uartId = eUART0,
        .rxPin = {AM_BSP_GPIO_COM_UART_RX, &g_AM_BSP_GPIO_COM_UART_RX},
        .txPin = {AM_BSP_GPIO_COM_UART_TX, &g_AM_BSP_GPIO_COM_UART_TX},
        .pUartConfigs = &sUartConfig,
        .uartHwIndex = 0,
    },
#endif
#if NUM_UARTS_USED > 1
     [eUART1] = {
    .uartId = eUART1,
    .rxPin = {AM_BSP_GPIO_UART1_RX, &g_AM_BSP_GPIO_UART1_RX},
    .txPin = {AM_BSP_GPIO_UART1_TX, &g_AM_BSP_GPIO_UART1_TX},
    .pUartConfigs = &sUartConfig,
    .uartHwIndex = 1,
    },
#endif
};

//*****************************************************************************
//
//! @brief called from uart ISR managed data transfer
//! @param pUartDescriptor point to uart descriptor
//
//*****************************************************************************
static void uart_isr_common(uart_async_local_vars_t *pUartDescriptor);

//*****************************************************************************
//
//! @brief uart0 isr
//
//*****************************************************************************
void am_uart_isr(void)
{
#if defined(AM_BSP_GPIO_UART_TX) && defined(AM_BSP_GPIO_UART_RX)
    uart_isr_common(&gs_UartVars[eUART0]);
#endif
}

//*****************************************************************************
//
//! @brief uart1 isr
//
//*****************************************************************************
void am_uart1_isr(void)
{
#if defined(AM_BSP_GPIO_UART1_TX) && defined(AM_BSP_GPIO_UART1_RX)
    am_isr_common(&gs_UartVars[eUART1]);
#endif
}
//*****************************************************************************
//
//! @brief Common uart isr handler, called from the actual uart ISR
//!
//! @param pUartDescriptor
//
//*****************************************************************************
static void uart_isr_common(uart_async_local_vars_t *pUartDescriptor)
{
    pUartDescriptor->e32Status |= am_hal_uart_interrupt_queue_service(pUartDescriptor->pvUART);
    if (pUartDescriptor->e32Status != AM_HAL_UART_STATUS_SUCCESS)
    {
        pUartDescriptor->ui32ISrErrorCount++;
    }
}

//*****************************************************************************
//
//! @brief                - init the uart peripheral and buffers
//! @param uartId         - uart id used,
//!                         this should correspond to uart hardware channel in this code
//! @param ppUartLocalVar - returns the handle/descriptor here, only valid if status is valid
//! @return               - standard hal status
//
//*****************************************************************************
uint32_t init_uart(uart_id_e uartId, uart_async_local_vars_t **ppUartLocalVar)
{

    //
    //! get and check the uart number
    //
    if (uartId >= eMAX_UARTS)
    {
        *ppUartLocalVar = (uart_async_local_vars_t *) 0;
        return AM_HAL_STATUS_INVALID_ARG;
    }

    // save this into passed in argument
    *ppUartLocalVar = &gs_UartVars[uartId];

    uart_async_local_vars_t *p1UartLocalVar = &gs_UartVars[uartId];

    p1UartLocalVar->uartId = uartId;


    //
    //! setup rx and tx pins
    //
    const uart_defs_t *pUartDefs = &gs_UartPinDefs[uartId];
    uint32_t status = am_hal_gpio_pinconfig(pUartDefs->txPin.pinNUmber, *(pUartDefs->txPin.pinCfg));
    status += am_hal_gpio_pinconfig(pUartDefs->rxPin.pinNUmber, *(pUartDefs->rxPin.pinCfg));
    if (status)
    {
        return status;
    }

    //
    //! set up ram buffers and uart module
    //
    p1UartLocalVar->ui32RxBuffSize = sizeof(p1UartLocalVar->pui8UARTRXBuffer);
    p1UartLocalVar->ui32TxBuffSize = sizeof(p1UartLocalVar->pui8UARTTXBuffer);

    //
    //! init the uart, get the driver handle
    //
    status = am_hal_uart_initialize(UART_ID, &p1UartLocalVar->pvUART);
    if (status)
    {
        return status;
    }

    status = am_hal_uart_power_control(p1UartLocalVar->pvUART, AM_HAL_SYSCTRL_WAKE, false);
    if (status)
    {
        return status;
    }

    am_hal_uart_clock_speed_e eUartClockSpeed = eUART_CLK_SPEED_DEFAULT;

    status = am_hal_uart_control(p1UartLocalVar->pvUART, AM_HAL_UART_CONTROL_CLKSEL, &eUartClockSpeed);
    if (status)
    {
        return status;
    }

    status = am_hal_uart_configure(p1UartLocalVar->pvUART, pUartDefs->pUartConfigs);
    if (status)
    {
        return status;
    }

    status = am_hal_uart_buffer_configure(p1UartLocalVar->pvUART,
                                          p1UartLocalVar->pui8UARTTXBuffer, p1UartLocalVar->ui32TxBuffSize,
                                          p1UartLocalVar->pui8UARTRXBuffer, p1UartLocalVar->ui32RxBuffSize);
    if (status)
    {
        return status;
    }

    //
    // Make sure to enable the interrupts for RX, since the HAL doesn't already
    // know we intend to use them., this is done later
    //
    uint32_t uartInstance = pUartDefs->uartHwIndex;
    NVIC_SetPriority((IRQn_Type) (UART0_IRQn + uartInstance), 0x04);
    NVIC_EnableIRQ((IRQn_Type) (UART0_IRQn + uartInstance));

    return AM_HAL_STATUS_SUCCESS;
}
//*****************************************************************************
//! @}
//*****************************************************************************
