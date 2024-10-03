//*****************************************************************************
//
//! @file ios_fifo_host.c
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "ios_link.h"


#define IOM_MODULE              5
#define HANDSHAKE_PIN           17
#define USE_SPI                 0   // 0 = I2C, 1 = SPI

uint8_t g_ui8SendDataFromJLINK[LINK_FRAME_SIZE] @0x10007100;
#define SEND_DATA_SUCCESS       0
#define SEND_DATA_FAIL          1
//*****************************************************************************
//
// Global message buffer for the IO master.
//
//*****************************************************************************
volatile bool bIosInt = false;
void *g_IOMHandle;

//*****************************************************************************
//
// Configuration structure for the IO Master.
//
//*****************************************************************************
static am_hal_iom_config_t g_sIOMSpiConfig =
{
    .eInterfaceMode = AM_HAL_IOM_SPI_MODE,
//    .ui32ClockFreq = AM_HAL_IOM_96MHZ,
//    .ui32ClockFreq = AM_HAL_IOM_48MHZ,
//    .ui32ClockFreq = AM_HAL_IOM_24MHZ,
//    .ui32ClockFreq = AM_HAL_IOM_16MHZ,
    .ui32ClockFreq = AM_HAL_IOM_500KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_400KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_375KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_250KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_125KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_100KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_50KHZ,
//    .ui32ClockFreq = AM_HAL_IOM_10KHZ,
    .eSpiMode = AM_HAL_IOM_SPI_MODE_0,
};

#define MAX_SPI_SIZE    1023

static am_hal_iom_config_t g_sIOMI2cConfig =
{
    .eInterfaceMode = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq  = AM_HAL_IOM_1MHZ,
};

#define MAX_I2C_SIZE   255

const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_HANDSHAKE =
{
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_4_GPIO,
    .GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA,
    .GP.cfg_b.eIntDir        = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.eGPInput       = AM_HAL_GPIO_PIN_INPUT_ENABLE,
};

// ISR callback for the host IOINT
void hostint_handler(void)
{
    bIosInt = true;
}

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void am_gpio0_001f_isr(void)
{
    //
    // Read and clear the GPIO interrupt status.
    //
    uint32_t    ui32IntStatus;
    AM_CRITICAL_BEGIN
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    AM_CRITICAL_END
    am_hal_gpio_interrupt_service(GPIO0_001F_IRQn, ui32IntStatus);
}



void iom_slave_read(bool bSpi, uint32_t offset, uint32_t *pBuf, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui32InstrLen    = 1;
    Transaction.ui64Instr = offset;
    Transaction.eDirection      = AM_HAL_IOM_RX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32RxBuffer   = pBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    if ( bSpi )
    {
        Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;
    }
    else
    {
        Transaction.uPeerInfo.ui32I2CDevAddr = I2C_ADDR;
    }
    am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
}

void iom_slave_write(bool bSpi, uint32_t offset, uint32_t *pBuf, uint32_t size)
{
    am_hal_iom_transfer_t       Transaction;

    Transaction.ui32InstrLen    = 1;
    Transaction.ui64Instr = offset;
    Transaction.eDirection      = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes    = size;
    Transaction.pui32TxBuffer   = pBuf;
    Transaction.bContinue       = false;
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;

    if ( bSpi )
    {
        Transaction.uPeerInfo.ui32SpiChipSelect = AM_BSP_IOM0_CS_CHNL;
    }
    else
    {
        Transaction.uPeerInfo.ui32I2CDevAddr = I2C_ADDR;
    }
    am_hal_iom_blocking_transfer(g_IOMHandle, &Transaction);
}



uint32_t ios_link_read(uint32_t *pBuf, uint32_t size)
{
    bool bSpi = USE_SPI;
    iom_slave_read(bSpi, 0x7F, pBuf, size);

    return size;
}

uint32_t ios_link_write(const uint32_t *pBuf, uint32_t size)
{
      bool bSpi = USE_SPI;
     iom_slave_write(bSpi, 0x80,  (uint32_t*)pBuf, size);

     return size;
}


//*****************************************************************************
//
// Internal Helper functions
//
//*****************************************************************************

static void iom_set_up(uint32_t iomModule, bool bSpi)
{
    //
    // Initialize the IOM.
    //
    am_hal_iom_initialize(iomModule, &g_IOMHandle);
    am_hal_iom_power_ctrl(g_IOMHandle, AM_HAL_SYSCTRL_WAKE, false);

    if ( bSpi )
    {
        //
        // Set the required configuration settings for the IOM.
        //
        am_hal_iom_configure(g_IOMHandle, &g_sIOMSpiConfig);

        //
        // Configure the IOM pins.
        //
        am_bsp_iom_pins_enable(iomModule, AM_HAL_IOM_SPI_MODE);
    }
    else
    {
        //
        // Set the required configuration settings for the IOM.
        //
        am_hal_iom_configure(g_IOMHandle, &g_sIOMI2cConfig);

        //
        // Configure the IOM pins.
        //
        am_bsp_iom_pins_enable(iomModule, AM_HAL_IOM_I2C_MODE);
    }

    //
    // Enable all the interrupts for the IOM module.
    //
    //am_hal_iom_InterruptEnable(g_IOMHandle, 0xFF);
    //am_hal_interrupt_enable(AM_HAL_INTERRUPT_IOMASTER0);

    //
    // Enable the IOM.
    //
    am_hal_iom_enable(g_IOMHandle);
    am_hal_gpio_pinconfig(HANDSHAKE_PIN, g_AM_BSP_GPIO_HANDSHAKE);

    uint32_t IntNum = HANDSHAKE_PIN;
    am_hal_gpio_state_write(HANDSHAKE_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    // Set up the host IO interrupt
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_INT_CHANNEL_0, (am_hal_gpio_mask_t*)&IntNum);
    // Register handler for IOS => IOM interrupt
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, HANDSHAKE_PIN,
                                    (am_hal_gpio_handler_t)hostint_handler, NULL);
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&IntNum);
    NVIC_SetPriority(GPIO0_001F_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);
}


uint32_t ios_link_mux_mode_set(uint32_t mode)
{
    link_frame_t link_frame;
    uint32_t retry = 1000;

    memset(link_frame.buf, 0 , LINK_FRAME_SIZE);

    link_frame.frame.head = LINK_FRAME_HEAD;
    link_frame.frame.cmd = LINK_CMD_MUX_MODE;
    link_frame.frame.state = 0x00;
    link_frame.frame.len = 1;
    link_frame.frame.data[0] = mode;
    ios_link_write((const uint32_t*)link_frame.buf, LINK_FRAME_SIZE);

    while(retry--)
    {
        am_util_delay_ms(1);
        if ( bIosInt == true )
        {
            break;
        }
    }

    if ( bIosInt == true )
    {
        bIosInt = false;
        ios_link_read((uint32_t *)link_frame.buf, LINK_FRAME_SIZE);
    }

    return 0;
}

uint32_t ios_link_send_data(uint8_t *pBuf)
{
    link_frame_t link_frame;
    uint32_t retry = 1000;

    ios_link_write((const uint32_t*)pBuf, LINK_FRAME_SIZE);

    while(retry--)
    {
        am_util_delay_ms(1);
        if ( bIosInt == true )
        {
            break;
        }
    }

    if ( bIosInt == true )
    {
        bIosInt = false;
        ios_link_read((uint32_t *)link_frame.buf, LINK_FRAME_SIZE);

    }

    if ( link_frame.frame.state == LINK_EOK )
    {
         return SEND_DATA_SUCCESS;
    }

    return SEND_DATA_FAIL;
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int main(void)
{
    uint32_t iom = IOM_MODULE;
    bool bSpi = USE_SPI;

    //
    // Configure the board for low power operation.
    //
//    am_bsp_low_power_init();

    //
    // Enable the ITM print interface.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Enable Interrupts.
    //
    am_hal_interrupt_master_enable();

    //
    // Set up the IOM
    //
#if defined(MIKROE_1032) || defined(MIKROE_2529)
    g_sIOMI2cConfig.ui32ClockFreq = AM_HAL_IOM_400KHZ;
#endif

    iom_set_up(iom, bSpi);

    //
    // Allow time for apollo3 inital.
    //
    //am_util_delay_ms(2000);

    am_util_stdio_printf("[ISO Master] initialization.\n");

//#if defined(TEST)
#if 0
    g_ui8SendDataFromJLINK[0] = 0x3f;
    g_ui8SendDataFromJLINK[1] = LINK_CMD_MUX_MODE;
    g_ui8SendDataFromJLINK[2] = 0;
    g_ui8SendDataFromJLINK[3] = 1;
    g_ui8SendDataFromJLINK[4] = 1;

#endif
    while(1)
    {
      if ( g_ui8SendDataFromJLINK[0] == LINK_FRAME_HEAD )
      {
          am_util_stdio_printf("[ISO Master] Send Data from jlink : %s.", g_ui8SendDataFromJLINK);

          if ( ios_link_send_data(g_ui8SendDataFromJLINK) == SEND_DATA_SUCCESS )
          {
              g_ui8SendDataFromJLINK[0] = LINK_EOK;
          }
          else
          {
              g_ui8SendDataFromJLINK[0] = LINK_ERR;
          }
      }
    }
    //ios_link_mux_mode_set(0x5a);
}



