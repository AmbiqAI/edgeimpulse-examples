//*****************************************************************************
//
//! @file am_widget_scard.c
//!
//! @brief This widget allows test cases to exercise the Apollo3 SCARD HAL
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

#include "am_mcu_apollo.h"
#include "am_widget_scard.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Look for errors in the HAL.
//
//*****************************************************************************
#if 0

#define CHECK_STATUS(x, expected)                                             \
    {                                                                         \
        g_ui32ErrorStatus = (x);                                              \
        if (g_ui32ErrorStatus != (expected))                                  \
        {                                                                     \
            am_util_stdio_printf("ERROR: status 0x%02x in %s, L%d\n",         \
                                g_ui32ErrorStatus, __FILE__, __LINE__);       \
                                                                              \
            am_util_stdio_printf(" ...expected: 0x%02x\n", (expected));       \
                                                                              \
            return AM_HAL_STATUS_FAIL;                                        \
        }                                                                     \
    }

#else

#define CHECK_STATUS(x, expected)                                             \
    {                                                                         \
        g_ui32ErrorStatus = (x);                                              \
        if (g_ui32ErrorStatus != (expected))                                  \
        {                                                                     \
            return AM_HAL_STATUS_FAIL;                                        \
        }                                                                     \
    }

#endif

#define CHECK_PASS(x)   CHECK_STATUS(x, AM_HAL_STATUS_SUCCESS)

#define AM_HAL_SCARD_INT_ERR    (AM_HAL_SCARD_INT_OVREN | AM_HAL_SCARD_INT_PEEN | AM_HAL_SCARD_INT_FEREN)
#define AM_HAL_SCARD_INT_RX     (AM_HAL_SCARD_INT_FHFEN | AM_HAL_SCARD_INT_FNEEN)

//*****************************************************************************
//
// Tracking variable for error status.
//
//*****************************************************************************
static am_widget_scard_config_t g_ScardWidget;

uint32_t g_ui32ErrorStatus = 0;

//*****************************************************************************
//
// State variables for the SCARD.
//
//*****************************************************************************
void *SCARD_Handle;

#define SCCCLK_PIN         17
#define SCCIO_PIN          32
#define SCCRST_PIN         26

const am_hal_gpio_pincfg_t g_AM_SCARD_GPIO_SCCCLK =
{
    .uFuncSel            = AM_HAL_PIN_17_SCCCLK,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA
};

const am_hal_gpio_pincfg_t g_AM_SCARD_GPIO_SCCIO =
{
    .uFuncSel            = AM_HAL_PIN_32_SCCIO,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA
};

const am_hal_gpio_pincfg_t g_AM_SCARD_GPIO_SCCRST =
{
    .uFuncSel            = AM_HAL_PIN_26_SCCRST,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA
};

//*****************************************************************************
//
// SCARD interrupt handler.
//
//*****************************************************************************
void
am_scard_isr(void)
{
    //
    // Service the FIFOs as necessary, and clear the interrupts.
    //
    uint32_t ui32Status[2], ui32Idle;
    am_hal_scard_interrupt_status_get(SCARD_Handle, 0, &ui32Status[0]);
    am_hal_scard_interrupt_status_get(SCARD_Handle, 1, &ui32Status[1]);
    am_hal_scard_interrupt_clear(SCARD_Handle, 0, ui32Status[0]);
    am_hal_scard_interrupt_clear(SCARD_Handle, 1, ui32Status[1]);
    am_hal_scard_interrupt_service(SCARD_Handle, ui32Status[0], &ui32Idle);
}

uint32_t am_widget_scard_test_setup(am_widget_scard_config_t *pTestCfg,
                                  void **ppWidget, char *pErrStr)
{
    g_ScardWidget.SCARDConfig    = pTestCfg->SCARDConfig;
    g_ScardWidget.ui32Module     = pTestCfg->ui32Module;
    *ppWidget = &g_ScardWidget;

    CHECK_PASS(am_hal_scard_initialize(g_ScardWidget.ui32Module, &SCARD_Handle));
    CHECK_PASS(am_hal_scard_power_control(SCARD_Handle, AM_HAL_SYSCTRL_WAKE, false));
    CHECK_PASS(am_hal_scard_configure(SCARD_Handle, &g_ScardWidget.SCARDConfig));

    am_hal_gpio_pinconfig(SCCCLK_PIN, g_AM_SCARD_GPIO_SCCCLK);
    am_hal_gpio_pinconfig(SCCIO_PIN, g_AM_SCARD_GPIO_SCCIO);
    am_hal_gpio_pinconfig(SCCRST_PIN, g_AM_SCARD_GPIO_SCCRST);

    am_hal_scard_interrupt_clear(SCARD_Handle, 0, AM_HAL_SCARD_INT_ALL);
    am_hal_scard_interrupt_clear(SCARD_Handle, 1, AM_HAL_SCARD_INT_ALL);
    NVIC_EnableIRQ(SCARD_IRQn);
    am_hal_interrupt_master_enable();

    CHECK_PASS(am_hal_scard_control(SCARD_Handle, AM_HAL_SCARD_REQ_CLK_START, NULL));
    CHECK_PASS(am_hal_scard_control(SCARD_Handle, AM_HAL_SCARD_REQ_ACTIVATE, NULL));

    return AM_HAL_STATUS_SUCCESS;
}

uint32_t am_widget_scard_test_cleanup(void *pWidget, char *pErrStr)
{
    CHECK_PASS(am_hal_scard_control(SCARD_Handle, AM_HAL_SCARD_REQ_DEACTIVATE, NULL));
    CHECK_PASS(am_hal_scard_control(SCARD_Handle, AM_HAL_SCARD_REQ_CLK_STOP, NULL));

    am_hal_interrupt_master_disable();

    NVIC_DisableIRQ(SCARD_IRQn);
    am_hal_scard_interrupt_clear(SCARD_Handle, 0, AM_HAL_SCARD_INT_ALL);
    am_hal_scard_interrupt_clear(SCARD_Handle, 1, AM_HAL_SCARD_INT_ALL);
    am_hal_scard_interrupt_disable(SCARD_Handle, 0, AM_HAL_SCARD_INT_ALL);

    CHECK_PASS(am_hal_scard_deinitialize(SCARD_Handle));
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Send APDU header
//
//*****************************************************************************
uint32_t am_widget_scard_send_apdu_header(uint8_t *cmd)
{
    /* cmd: cla ins p1 p2 p3 */
    uint8_t ins;
    am_hal_scard_transfer_t Transaction;
    Transaction.ui32Direction = AM_HAL_SCARD_WRITE;
    Transaction.ui32NumBytes = AM_HAL_SCARD_APDU_HEADER_LENGTH;
    Transaction.ui32TimeoutMs = g_ScardWidget.SCARDConfig.ui32TxTimeout;
    Transaction.pui8Data = cmd;

    CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));

    if(cmd[AM_HAL_SCARD_APDU_LC] != 0x00)
    {
        Transaction.ui32Direction = AM_HAL_SCARD_READ;
        Transaction.ui32NumBytes = 1;
        Transaction.pui8Data = &ins;
        Transaction.ui32TimeoutMs = g_ScardWidget.SCARDConfig.ui32RxTimeout;
        CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));

        if(ins != cmd[AM_HAL_SCARD_APDU_INS])
        {
            return AM_HAL_STATUS_FAIL;
        }
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Send data
//
//*****************************************************************************
uint32_t am_widget_scard_send_data(uint8_t* pui8DataBuf)
{
    uint8_t apdu[AM_HAL_SCARD_APDU_HEADER_LENGTH] = {0x00, 0xD0, 0x00, 0x00, 0xF};
    uint8_t sw[AM_HAL_SCARD_SW_LENGTH];
    am_hal_scard_transfer_t Transaction;

    /* need some delay */
    am_util_delay_ms(10);
    /* send apdu cmd head */
    CHECK_PASS(am_widget_scard_send_apdu_header(apdu));

    /* need some delay */
    am_util_delay_ms(10);
    /* send data, len = apdu[4] */
    Transaction.ui32Direction = AM_HAL_SCARD_WRITE;
    Transaction.ui32NumBytes = apdu[AM_HAL_SCARD_APDU_LC];
    Transaction.ui32TimeoutMs = g_ScardWidget.SCARDConfig.ui32TxTimeout;
    Transaction.pui8Data = pui8DataBuf;
    CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));

    /* Get SCC SW */
    Transaction.ui32Direction = AM_HAL_SCARD_READ;
    Transaction.ui32NumBytes = AM_HAL_SCARD_SW_LENGTH;
    Transaction.ui32TimeoutMs = g_ScardWidget.SCARDConfig.ui32RxTimeout;
    Transaction.pui8Data = sw;
    CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
    am_util_stdio_printf("SW = %02x %02x \r\n",  (sw[0]),  (sw[1]));

    return AM_HAL_STATUS_SUCCESS;
}


//*****************************************************************************
// @brief SCC receive data
// @note     APDU[0] ;
// -# APDU[1] 0X01 FOR DATA SEND, 0X02 FOR DATA RECEIVE;
// -# APDU[2] APDU[3];
// -# APDU[4] Transfer/Send Data Length;
//*****************************************************************************
uint32_t am_widget_scard_recv_data(uint8_t* pui8DataBuf)
{
    uint8_t apdu[AM_HAL_SCARD_APDU_HEADER_LENGTH] = {0x00, 0xB0, 0x00, 0x00, 0xF0};
    uint8_t sw[AM_HAL_SCARD_SW_LENGTH];
    am_hal_scard_transfer_t Transaction;

    am_util_delay_ms(10);

    /* send apdu cmd head */
    CHECK_PASS(am_widget_scard_send_apdu_header(apdu));

    /* Receive data, len = apdu[4] */
    Transaction.ui32Direction = AM_HAL_SCARD_READ;
    Transaction.ui32NumBytes = apdu[AM_HAL_SCARD_APDU_LC];
    Transaction.ui32TimeoutMs = g_ScardWidget.SCARDConfig.ui32RxTimeout;
    Transaction.pui8Data = pui8DataBuf;
    CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));

    /* get scc sw */
    Transaction.ui32NumBytes = AM_HAL_SCARD_SW_LENGTH;
    Transaction.pui8Data = sw;
    CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
    am_util_stdio_printf("SW = %02x %02x \r\n",  (sw[0]),  (sw[1]));

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
// Get ATR after reset
//
//*****************************************************************************
uint32_t am_widget_scard_get_atr(uint8_t *pui8ATR, uint8_t *pui8Len)
{
    uint8_t data, xor = 0;
    am_hal_scard_transfer_t Transaction;
    uint32_t ui32ActReadLen = 0;
    uint8_t ui8ATRLen = 0;

    Transaction.ui32Direction = AM_HAL_SCARD_READ;
    Transaction.ui32NumBytes = 1;
    Transaction.ui32TimeoutMs = g_ScardWidget.SCARDConfig.ui32RxTimeout;
    Transaction.pui8Data = &data;
    Transaction.pui32BytesTransferred = &ui32ActReadLen;

    //TS
    CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
    if((AM_HAL_SCARD_DIR_MSB == data) || (AM_HAL_SCARD_DIR_LSB == data))
    {
        pui8ATR[ui8ATRLen++] = data;
        am_util_stdio_printf("TS: 0x%x\n", data);

        CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
        pui8ATR[ui8ATRLen++] = data; //T0
        am_util_stdio_printf("T0: 0x%x\n", data);
        while(data & 0xF0)
        {
            if(AM_HAL_SCARD_TAiP1_PRESENCE(data))//TAi
            {
                Transaction.pui8Data = &(pui8ATR[ui8ATRLen++]);
                CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
                am_util_stdio_printf("TA: 0x%x\n", *Transaction.pui8Data);
            }

            if(AM_HAL_SCARD_TBiP1_PRESENCE(data))//TBi
            {
                Transaction.pui8Data = &(pui8ATR[ui8ATRLen++]);
                CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
                am_util_stdio_printf("TB: 0x%x\n", *Transaction.pui8Data);
            }
            if(AM_HAL_SCARD_TCiP1_PRESENCE(data))//TCi
            {
                Transaction.pui8Data = &(pui8ATR[ui8ATRLen++]);
                CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
                am_util_stdio_printf("TC: 0x%x\n", *Transaction.pui8Data);
            }
            if(AM_HAL_SCARD_TDiP1_PRESENCE(data))//TDi
            {
                Transaction.pui8Data = &data;
                CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
                am_util_stdio_printf("TD: 0x%x\n", data);
                pui8ATR[ui8ATRLen++] = data;
            }
            else
            {
                data = 0;
            }
        }

        if(ui8ATRLen)
        {
            data = AM_HAL_SCARD_HISTORY_LEN(pui8ATR[1]);
            if(data) //history byte
            {
                Transaction.pui8Data = &(pui8ATR[ui8ATRLen]);
                Transaction.ui32NumBytes = data;
                CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
                am_util_stdio_printf("History bytes:\n");
                for(uint8_t i = 0; i < data; i++)
                {
                    am_util_stdio_printf("T%d: 0x%x\n", i+1, pui8ATR[ui8ATRLen + i]);
                }
                ui8ATRLen += data;
                Transaction.pui8Data = &(pui8ATR[ui8ATRLen++]);
                Transaction.ui32NumBytes = 1; //TCK
                CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
                xor = pui8ATR[1] ;
                for(data = 2; data < ui8ATRLen; data++)
                {
                    xor ^= pui8ATR[data];
                }
                if(xor)
                {
                    am_util_stdio_printf("TCK ERROR!\n");
                    ui8ATRLen = 0;
                    return AM_HAL_STATUS_FAIL;
                }
                else
                {
                    am_util_stdio_printf("TCK: 0x%x\n", *Transaction.pui8Data);
                }
            }
        }
    }
    else
    {
        am_util_stdio_printf("ATR ERROR: %02X\r\n", data);
        return AM_HAL_STATUS_FAIL;
    }

    *pui8Len = ui8ATRLen;

    return AM_HAL_STATUS_SUCCESS;
}


//*****************************************************************************
//
// PPS message exchange
//
//*****************************************************************************
uint32_t am_widget_scard_pps_exchange(am_hal_scard_pps_t stPPS)
{
    uint8_t ppstxbuf[AM_HAL_SCARD_MAX_PPS_LENGTH], len = 0;
    uint8_t ppsrxbuf[AM_HAL_SCARD_MAX_PPS_LENGTH];
    am_hal_scard_transfer_t Transaction;
    uint32_t ui32ActReadLen = 0;
    uint8_t ui8Lrc = 0;
    uint16_t i =0;

    ppstxbuf[len++] = AM_HAL_SCARD_CLA_PPS;
    ppstxbuf[len++] = stPPS.pps0;

    if(AM_HAL_SCARD_PPS1_PRESENCE(stPPS.pps0))
    {
        ppstxbuf[len++] = stPPS.pps1;
    }

    if(AM_HAL_SCARD_PPS2_PRESENCE(stPPS.pps0))
    {
        ppstxbuf[len++] = stPPS.pps2;
    }

    if(AM_HAL_SCARD_PPS3_PRESENCE(stPPS.pps0))
    {
        ppstxbuf[len++] = stPPS.pps3;
    }

    //PCK
    for(i = 0; i < len; i++)
    {
        ui8Lrc ^= *(ppstxbuf + i);
    }
    ppstxbuf[len] = ui8Lrc;
    len++;
    am_util_stdio_printf("PPS sent out:\t");
    for(i = 0; i < len; i++)
    {
        am_util_stdio_printf("0x%x ", ppstxbuf[i]);
    }
    am_util_stdio_printf("\n");

    Transaction.ui32Direction = AM_HAL_SCARD_WRITE;
    Transaction.ui32NumBytes = len;
    Transaction.ui32TimeoutMs = g_ScardWidget.SCARDConfig.ui32TxTimeout;
    Transaction.pui8Data = ppstxbuf;
    Transaction.pui32BytesTransferred = &ui32ActReadLen;
    CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));

    Transaction.ui32Direction = AM_HAL_SCARD_READ;
    Transaction.pui8Data = ppsrxbuf;
    Transaction.ui32TimeoutMs = g_ScardWidget.SCARDConfig.ui32RxTimeout;
    CHECK_PASS(am_hal_scard_transfer(SCARD_Handle, &Transaction));
    am_util_stdio_printf("ACK received:\t");
    for(i = 0; i < len; i++)
    {
        am_util_stdio_printf("0x%x ", ppsrxbuf[i]);
    }
    am_util_stdio_printf("\n");
    for(i = 0; i < len; i++)
    {
        if(ppstxbuf[i] != ppsrxbuf[i])
        {
            am_util_stdio_printf("Buffer Validation failed @i=%d Rcvd 0x%x Expected 0x%x\n",
                i, ppsrxbuf[i], ppstxbuf[i]);
            return AM_HAL_STATUS_FAIL;
        }
    }

    am_util_stdio_printf("PPS negotiation OK\r\n");

    return AM_HAL_STATUS_SUCCESS;
}


