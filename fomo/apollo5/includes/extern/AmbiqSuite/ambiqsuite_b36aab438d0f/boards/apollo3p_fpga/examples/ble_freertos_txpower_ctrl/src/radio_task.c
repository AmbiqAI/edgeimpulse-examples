//*****************************************************************************
//
//! @file radio_task.c
//!
//! @brief Task to handle radio operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "ble_freertos_txpower_ctrl.h"

//*****************************************************************************
//
// WSF standard includes.
//
//*****************************************************************************
#include "wsf_types.h"
#include "wsf_trace.h"
#include "wsf_buf.h"
#include "wsf_timer.h"

//*****************************************************************************
//
// Includes for operating the ExactLE stack.
//
//*****************************************************************************
#include "hci_handler.h"
#include "dm_handler.h"
#include "l2c_handler.h"
#include "att_handler.h"
#include "smp_handler.h"
#include "l2c_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "app_api.h"
#include "hci_core.h"
#include "hci_drv.h"
#include "hci_drv_apollo.h"
#include "hci_drv_apollo3.h"
#include "hci_apollo_config.h"

#include "wsf_msg.h"

//*****************************************************************************
//
//
//*****************************************************************************
#include "app_ui.h"

//*****************************************************************************
//
// Radio task handle.
//
//*****************************************************************************
TaskHandle_t radio_task_handle;

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
void exactle_stack_init(void);
void button_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg);
void setup_buttons(void);
//*****************************************************************************
//
// Timer for buttons.
//
//*****************************************************************************
wsfHandlerId_t ButtonHandlerId;
wsfTimer_t ButtonTimer;

txPowerLevel_t  tx_power_level = TX_POWER_LEVEL_PLUS_3P0_dBm;
uint32_t        dtm_in_progress = false;

//*****************************************************************************
//
// WSF buffer pools.
//
//*****************************************************************************
#define WSF_BUF_POOLS               4

// Important note: the size of g_pui32BufMem should includes both overhead of internal
// buffer management structure, wsfBufPool_t (up to 16 bytes for each pool), and pool
// description (e.g. g_psPoolDescriptors below).

// Memory for the buffer pool
// extra AMOTA_PACKET_SIZE bytes for OTA handling
static uint32_t g_pui32BufMem[
        (WSF_BUF_POOLS*16
         + 16*8 + 32*4 + 64*6 + 280*8) / sizeof(uint32_t)];

// Default pool descriptor.
static wsfBufPoolDesc_t g_psPoolDescriptors[WSF_BUF_POOLS] =
{
    {  16,  8 },
    {  32,  4 },
    {  64,  6 },
    { 280,  8 }
};

//*****************************************************************************
//
// Tracking variable for the scheduler timer.
//
//*****************************************************************************

void radio_timer_handler(void);



//*****************************************************************************
//
// Poll the buttons.
//
//*****************************************************************************
void
button_handler(wsfEventMask_t event, wsfMsgHdr_t *pMsg)
{
    //
    // Restart the button timer.
    //
    WsfTimerStartMs(&ButtonTimer, 10);

    //
    // Every time we get a button timer tick, check all of our buttons.
    //
    am_devices_button_array_tick(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);

    //
    // If we got a a press, do something with it.
    //
    if ( am_devices_button_released(am_bsp_psButtons[0]) )
    {
        AppUiBtnTest(APP_UI_BTN_1_SHORT);

        HciVsA3_SetRfPowerLevelEx(tx_power_level);

        switch ( tx_power_level )
        {
            case TX_POWER_LEVEL_MINUS_10P0_dBm:
                am_util_debug_printf("Current Tx Power is -10.0 dBm\n");
                tx_power_level = TX_POWER_LEVEL_0P0_dBm;
                break;
            case TX_POWER_LEVEL_0P0_dBm:
                am_util_debug_printf("Current Tx Power is 0.0 dBm\n");
                tx_power_level = TX_POWER_LEVEL_PLUS_3P0_dBm;
                break;
            case TX_POWER_LEVEL_PLUS_3P0_dBm:
                am_util_debug_printf("Current Tx Power is +3.0 dBm\n");
                tx_power_level = TX_POWER_LEVEL_MINUS_10P0_dBm;
                break;
            default:
                am_util_debug_printf("Invalid Tx power level\n");
                break;
        }
    }

    if ( am_devices_button_released(am_bsp_psButtons[1]) )
    {
        AppUiBtnTest(APP_UI_BTN_2_SHORT);

        if ( dtm_in_progress )
        {
            HciLeTestEndCmd();
            dtm_in_progress = false;
            am_util_debug_printf("Transmitter test ended\n");
        }
        else
        {
            // Resetting controller first.
            HciResetCmd();

            // HciLeTransmitterTestCmd
            HciLeTransmitterTestCmd(0, 255, 7);

            dtm_in_progress = true;
            am_util_debug_printf("Transmitter test started\n");
        }
    }
}

//*****************************************************************************
//
// Sets up a button interface.
//
//*****************************************************************************
void
setup_buttons(void)
{
    //
    // Enable the buttons for user interaction.
    //
    am_devices_button_array_init(am_bsp_psButtons, AM_BSP_NUM_BUTTONS);

    //
    // Start a timer.
    //
    ButtonTimer.handlerId = ButtonHandlerId;
    WsfTimerStartSec(&ButtonTimer, 2);
}


//*****************************************************************************
//
// Initialization for the ExactLE stack.
//
//*****************************************************************************
void
exactle_stack_init(void)
{
    wsfHandlerId_t handlerId;
    uint16_t       wsfBufMemLen;
    //
    // Set up timers for the WSF scheduler.
    //
    WsfOsInit();
    WsfTimerInit();

    //
    // Initialize a buffer pool for WSF dynamic memory needs.
    //
    wsfBufMemLen = WsfBufInit(sizeof(g_pui32BufMem), (uint8_t *)g_pui32BufMem, WSF_BUF_POOLS,
               g_psPoolDescriptors);

    if (wsfBufMemLen > sizeof(g_pui32BufMem))
    {
        am_util_debug_printf("Memory pool is too small by %d\r\n",
                             wsfBufMemLen - sizeof(g_pui32BufMem));
    }

    //
    // Initialize the WSF security service.
    //
    SecInit();
    SecAesInit();
    SecCmacInit();
    SecEccInit();

    //
    // Set up callback functions for the various layers of the ExactLE stack.
    //
    handlerId = WsfOsSetNextHandler(HciHandler);
    HciHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(DmHandler);
    DmDevVsInit(0);
    DmAdvInit();
    DmConnInit();
    DmConnSlaveInit();
    DmSecInit();
    DmSecLescInit();
    DmPrivInit();
    DmHandlerInit(handlerId);

    handlerId = WsfOsSetNextHandler(L2cSlaveHandler);
    L2cSlaveHandlerInit(handlerId);
    L2cInit();
    L2cSlaveInit();

    handlerId = WsfOsSetNextHandler(AttHandler);
    AttHandlerInit(handlerId);
    AttsInit();
    AttsIndInit();
    AttcInit();

    handlerId = WsfOsSetNextHandler(SmpHandler);
    SmpHandlerInit(handlerId);
    SmprInit();
    SmprScInit();
    HciSetMaxRxAclLen(251);

    handlerId = WsfOsSetNextHandler(AppHandler);
    AppHandlerInit(handlerId);


    ButtonHandlerId = WsfOsSetNextHandler(button_handler);

    handlerId = WsfOsSetNextHandler(HciDrvHandler);
    HciDrvHandlerInit(handlerId);
}

//*****************************************************************************
//
// UART interrupt handler.
//
//*****************************************************************************
void
am_uart_isr(void)
{
    uint32_t ui32Status;

    //
    // Read and save the interrupt status, but clear out the status register.
    //
    ui32Status = UARTn(0)->MIS;
    UARTn(0)->IEC = ui32Status;

    //
    // Allow the HCI driver to respond to the interrupt.
    //
    //HciDrvUartISR(ui32Status);

    // Signal radio task to run

    WsfTaskSetReady(0, 0);
}

//*****************************************************************************
//
// Interrupt handler for BLE
//
//*****************************************************************************
void
am_ble_isr(void)
{

    HciDrvIntService();

    // Signal radio task to run

    WsfTaskSetReady(0, 0);
}

/*************************************************************************************************/
/*!
 *  \fn     TxPowerCtrlDmCback
 *
 *  \brief  Application DM callback.
 *
 *  \param  pDmEvt  DM callback event
 *
 *  \return None.
 */
/*************************************************************************************************/
static void TxPowerCtrlDmCback(dmEvt_t *pDmEvt)
{
  // do nothing
}

//*****************************************************************************
//
// Perform initial setup for the radio task.
//
//*****************************************************************************
void
RadioTaskSetup(void)
{
    am_util_debug_printf("RadioTask: setup\r\n");
    // #### INTERNAL BEGIN ####
#ifdef AM_DEBUG_PRINTF
    //
    // Print some device information.
    //
    am_util_id_t sIdDevice;
    uint32_t ui32StrBuf;
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
    am_util_stdio_printf("Device type: %s\n",
         sIdDevice.pui8DeviceName);

    am_util_stdio_printf("Device Info:\n"
                         "\tPart number: 0x%08X\n"
                         "\tChip ID0:    0x%08X\n"
                         "\tChip ID1:    0x%08X\n"
                         "\tRevision:    0x%08X (Rev%c%c)\n",
                         sIdDevice.sMcuCtrlDevice.ui32ChipPN,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID0,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID1,
                         sIdDevice.sMcuCtrlDevice.ui32ChipRev,
                         sIdDevice.ui8ChipRevMaj, sIdDevice.ui8ChipRevMin );

#if 1
    //
    // Turn off unneeded Flash & SRAM
    //
#if defined(AM_PART_APOLLO3P)
    // TCM + STACK uses 76KBytes
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_SRAM_192K);
#else
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_SRAM_96K);
#endif
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEM_FLASH_MIN);
#endif

    //
    // If not a multiple of 1024 bytes, append a plus sign to the KB.
    //
    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32FlashSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tFlash size:  %7d (%d KB%s)\n",
                         sIdDevice.sMcuCtrlDevice.ui32FlashSize,
                         sIdDevice.sMcuCtrlDevice.ui32FlashSize / 1024,
                         &ui32StrBuf);

    ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32SRAMSize % 1024 ) ? '+' : 0;
    am_util_stdio_printf("\tSRAM size:   %7d (%d KB%s)\n\n",
                         sIdDevice.sMcuCtrlDevice.ui32SRAMSize,
                         sIdDevice.sMcuCtrlDevice.ui32SRAMSize / 1024,
                         &ui32StrBuf);
#endif // AM_DEBUG_PRINTF
    // #### INTERNAL END ####


    NVIC_SetPriority(BLE_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);

    //
    // Boot the radio.
    //
    HciDrvRadioBoot(1);
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
RadioTask(void *pvParameters)
{
#if WSF_TRACE_ENABLED == TRUE
    //
    // Enable ITM
    //
    am_util_debug_printf("Starting wicentric trace:\n\n");
#endif

    //
    // Initialize the main ExactLE stack.
    //
    exactle_stack_init();

    //
    // Prep the buttons for use
    //

    setup_buttons();

    //
    // Need to register DM callback for Vendor specific command's event
    //

    DmRegister(TxPowerCtrlDmCback);

    am_util_debug_printf("Usage as below:\n");
    am_util_debug_printf("Short press Button 2 to enter/exit DTM tx test\n");
    am_util_debug_printf("Short press Button 1 to change tx power level from high to low.\n");


    while (1)
    {

        //
        // Calculate the elapsed time from our free-running timer, and update
        // the software timers in the WSF scheduler.
        //
        wsfOsDispatcher();

    }
}