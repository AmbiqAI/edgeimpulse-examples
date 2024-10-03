//*****************************************************************************
//
//! @file am_widget_iom.c
//!
//! @brief Test widget for testing IOM channels by data transfer using SPI Flash.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include <ctype.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_widget_iom_multidrop_i2c.h"

//*****************************************************************************
//
// Widget Config parameters
//
//*****************************************************************************

#if defined(APOLLO4_FPGA)
#define HFRC_BASE (APOLLO4_FPGA*1000000)
#define XT_BASE   16384
#define LFRC_BASE 50 // FIXME - For now it is 500Hz.
#elif defined(AM_PART_APOLLO4)
#define HFRC_BASE 48000000
#define XT_BASE   32768
#define LFRC_BASE 1024
#elif defined(AM_PART_APOLLO4B)
#define HFRC_BASE 96000000
#define XT_BASE   32768
#define LFRC_BASE 1024
#elif defined(AM_PART_APOLLO4P)
#define HFRC_BASE 96000000
#define XT_BASE   32768
#define LFRC_BASE 1024
#endif
struct
{
    uint32_t    lower;
    uint32_t    upper;
} am_widget_timer_1sec_count[AM_HAL_TIMER_CLOCK_RTC_100HZ+1] =
{
    // Lower Bound Pass         Upper Bound Pass
    { HFRC_BASE/4-50,           HFRC_BASE/4 + 50 },
    { HFRC_BASE/16-30,          HFRC_BASE/16+30 },
    { HFRC_BASE/64-10,          HFRC_BASE/64+10 },
    { HFRC_BASE/256-2,          HFRC_BASE/256+2 },
    { HFRC_BASE/1024-1,         HFRC_BASE/1024+1 },
    { HFRC_BASE/4096-1,         HFRC_BASE/4096+1 },
    { LFRC_BASE-10,             LFRC_BASE+10 },
    { LFRC_BASE/2-5,            LFRC_BASE/2+5 },
    { LFRC_BASE/32-1,           LFRC_BASE/32+1 },
    { 0,                        LFRC_BASE/1024+1 },
    { XT_BASE-1,                XT_BASE+1 },
    { XT_BASE/2-1,              XT_BASE/2+1 },
    { XT_BASE/4-1,              XT_BASE/4+1 },
    { XT_BASE/8-1,              XT_BASE/8+1 },
    { XT_BASE/16-1,             XT_BASE/16+1 },
    { XT_BASE/32-1,             XT_BASE/32+1 },
    { XT_BASE/128-1,            XT_BASE/128+1 },
    { 98,                       102 }
};

//*****************************************************************************
//
// Callback functions
//
//*****************************************************************************
uint32_t g_bIOMNonBlockCompleteRd;

void pfnWidget_IOM_Callback_Read(void *pCtxt, uint32_t ui32Status)
{
    //
    // Set the DMA complete flag.
    //
    g_bIOMNonBlockCompleteRd = (ui32Status ? 2 : 1);
}

//*****************************************************************************
//
// RTC ISR
//
//*****************************************************************************
#define ALS_TEST_SECONDS        1
#define TOUCH_TEST_SECONDS      3
#define TOTAL_TEST_TIME         15

static uint32_t g_RTCseconds = 0;

bool als_read_flag = false;
bool touch_read_flag = false;
bool end_test = false;

uint32_t failures = 0;
uint32_t passes = 0;

void
am_rtc_isr(void)
{
    //
    // Clear the RTC alarm interrupt.
    //
    am_hal_rtc_interrupt_clear(AM_HAL_RTC_INT_ALM);

    //
    // Check the desired number of seconds until LED is toggled.
    //
    if(++g_RTCseconds % 1 == 0)
    {
        am_util_stdio_printf("..");
    }    
    if(g_RTCseconds % ALS_TEST_SECONDS == 0)
    {
        als_read_flag = true;
    }
    if(g_RTCseconds % TOUCH_TEST_SECONDS == 0)
    {
        touch_read_flag = true;
    }
    if(g_RTCseconds == TOTAL_TEST_TIME)
    {
        end_test = true;
    }
}

//*****************************************************************************
//
// Internal Data Structures
//
//*****************************************************************************
typedef struct
{
    uint32_t              ui32Module;
    am_hal_iom_config_t   IOMConfig;
    am_hal_iom_mode_e     mode;
    void                  *pHandle;
} am_widget_iom_t;

//*****************************************************************************
//
// Global Variables.
//
//*****************************************************************************
static am_widget_iom_config_t g_IomI2cWidget;
static volatile uint32_t g_IomIsrErr = 0;
static volatile uint32_t g_bComplete = 0;

void            *g_IomDevHdl;
void            *g_pIOMHandle;
volatile uint32_t g_IOMInterruptStatus = 0;

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
//
//! Take over default ISR for IOM 0. (Queue mode service)
//
void am_iom_isr(void)
{
    uint32_t ui32Status;

    am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status);

    if ( ui32Status )
    {
        am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);

        if ( g_IOMInterruptStatus == 0 )
        {
            am_hal_iom_interrupt_service(g_pIOMHandle, ui32Status);
        }
        g_IOMInterruptStatus &= ~ui32Status;
    }
    else
    {
        am_util_stdio_printf("IOM INTSTAT 0\n");
    }
}

void
am_iomaster0_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 1. (Queue mode service)
//
void
am_iomaster1_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 1. (Queue mode service)
//
void
am_iomaster2_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 3. (Queue mode service)
//
void
am_iomaster3_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 4. (Queue mode service)
//
void
am_iomaster4_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 5. (Queue mode service)
//
void
am_iomaster5_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 6. (Queue mode service)
//
void
am_iomaster6_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 7. (Queue mode service)
//
void
am_iomaster7_isr(void)
{
    am_iom_isr();
}

//*****************************************************************************
//
// Widget helper functions.
//
//*****************************************************************************
static void wait_for_nonblock_response(am_hal_sysctrl_power_state_e sleepMode, volatile uint32_t *pAddr);
//*****************************************************************************
//
// Wait for a non-blocking response.
//
//*****************************************************************************
static void
wait_for_nonblock_response(am_hal_sysctrl_power_state_e sleepMode, volatile uint32_t *pAddr)
{
    if (sleepMode == AM_HAL_SYSCTRL_WAKE)
    {
        while (!*pAddr);
    }
    else
    {
        //
        // Loop forever.
        //
        while(1)
        {
            //
            // Disable interrupt while we decide whether we're going to sleep.
            //
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();

            if (!*pAddr)
            {
                if (sleepMode == AM_HAL_SYSCTRL_DEEPSLEEP)
                {
                    // Disable ITM
                    am_bsp_debug_printf_disable();
                }
                am_hal_sysctrl_sleep(sleepMode);
                if (sleepMode == AM_HAL_SYSCTRL_DEEPSLEEP)
                {
                    // Re-enable ITM
                    am_bsp_debug_printf_enable();
                }
                //
                // Enable interrupts
                //
                am_hal_interrupt_master_set(ui32IntStatus);
            }
            else
            {
                //
                // Enable interrupts
                //
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }
        }
    }
}

//*****************************************************************************
//
// Widget setup function.
//
//*****************************************************************************
uint32_t am_widget_iom_test_setup(am_widget_iom_config_t *pTestCfg,
                                  void **ppWidget, char *pErrStr)
{
    uint32_t ui32Status = 0;
    g_IomI2cWidget.IOMConfig      = pTestCfg->IOMConfig;
    g_IomI2cWidget.ui32Module     = pTestCfg->ui32Module;
    g_IomI2cWidget.ui32DeviceType = pTestCfg->ui32DeviceType;
    *ppWidget = &g_IomI2cWidget;

    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return false;
    }
    
    ui32Status = am_devices_tsl2540_init(pTestCfg->ui32Module, &pTestCfg->IOMConfig, &g_IomDevHdl, &g_pIOMHandle);
    ui32Status = (ui32Status == IOM_WIDGET_DEVICE_SUCCESS) ? AM_WIDGET_SUCCESS : AM_WIDGET_ERROR;
    
    if (AM_WIDGET_ERROR == ui32Status)
    {
        return ui32Status;
    }
    
    ui32Status = am_devices_tma525_multidrop_no_iom_init(pTestCfg->ui32Module, &pTestCfg->IOMConfig, &g_IomDevHdl, &g_pIOMHandle);
    ui32Status = (ui32Status == IOM_WIDGET_DEVICE_SUCCESS) ? AM_WIDGET_SUCCESS : AM_WIDGET_ERROR;

    return ui32Status;
}

//*****************************************************************************
//
// Widget test cleanup function.
//
//*****************************************************************************
uint32_t am_widget_iom_test_cleanup(void *pWidget, char *pErrStr)
{
    uint32_t ui32Status = 0;
    
    am_hal_rtc_osc_disable();
    
    ui32Status =     am_devices_tsl2540_term(g_IomDevHdl);
    ui32Status = (ui32Status == IOM_WIDGET_DEVICE_SUCCESS) ? AM_WIDGET_SUCCESS : AM_WIDGET_ERROR;
    
    if (AM_WIDGET_ERROR == ui32Status)
    {
        return ui32Status;
    }
    
    ui32Status =     am_devices_tma525_multidrop_term(g_IomDevHdl);
    ui32Status = (ui32Status == IOM_WIDGET_DEVICE_SUCCESS) ? AM_WIDGET_SUCCESS : AM_WIDGET_ERROR;

    //
    // Return status.
    //
    return ui32Status;
}

//*****************************************************************************
//
// Widget Test Read ID function.
//
//*****************************************************************************
uint32_t
am_widget_iom_test_read_id(void *pWidget, char *pErrStr)
{
    uint32_t                      ui32DeviceId = 0;
    return ((am_devices_tsl2540_read_id(g_IomDevHdl, &ui32DeviceId) || ( ui32DeviceId != AM_DEVICES_TSL2540_ID )) ? AM_WIDGET_ERROR: AM_WIDGET_SUCCESS);
}

//*****************************************************************************
//
// Widget Multidrop Test NonBlocking Read function.
//
//*****************************************************************************
uint32_t
am_widget_iom_test_nonblocking_read(void *pWidget, void *pCfg, char *pErrStr)
{
    am_widget_iom_test_t   *pTestCfg = (am_widget_iom_test_t *)pCfg;
    uint32_t ui32Status;    
    uint32_t data = 0;        

    am_devices_tc_tma525_info_t g_sTouchInfo = {0};
    
    //
    // Reset test variables
    //
    g_RTCseconds = 0;
    als_read_flag = false;
    touch_read_flag = false;
    end_test = false;
    failures = 0;
    passes = 0;
    
    //
    // Enable the RTC.
    //
    am_hal_rtc_osc_enable();
    
    //
    // Set the alarm repeat interval to be every second.
    //
    am_hal_rtc_alarm_interval_set(AM_HAL_RTC_ALM_RPT_SEC);

    //
    // Clear the RTC alarm interrupt.
    //
    am_hal_rtc_interrupt_clear(AM_HAL_RTC_INT_ALM);

    //
    // Enable the RTC alarm interrupt.
    //
    am_hal_rtc_interrupt_enable(AM_HAL_RTC_INT_ALM);

    NVIC_SetPriority(RTC_IRQn, 1);
    NVIC_EnableIRQ(RTC_IRQn);
    
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    while(1)
    {
        //
        // Go to Deep Sleep until wakeup.
        //
        if (touch_read_flag == false && als_read_flag == false && end_test == false)
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        
        if(end_test == true)
        {
            break;
        }
        
        //
        // If Touch Sensor Timer Expired.
        //
        if (touch_read_flag == true)
        {
            touch_read_flag = false;

            //
            // Read the buffer from the TMA525 I2C Flash device.
            //
            
            //
            // change the touch interface for 2024/3/6 update
            //
//            am_devices_tma525_get_point(&x, &y, &touch_released);
            am_devices_tma525_get_point((am_devices_tc_tma525_info_t *)&g_sTouchInfo);

            //am_util_stdio_printf("\nx = %d, y = %d", x, y);
        }
        
        //
        // If Ambient Light Sensor Timer Expired.
        //
        if (als_read_flag == true)
        {
            als_read_flag = false;
            
            //
            // Read the buffers from the TSL2540 I2C Flash device.
            //
            g_bIOMNonBlockCompleteRd = 0;
            am_devices_tsl2540_read_revision_id(g_IomDevHdl, &data);
            am_devices_tsl2540_read_revision_id2(g_IomDevHdl, &data);
            am_devices_tsl2540_read_status(g_IomDevHdl, &data);
            am_devices_tsl2540_read_visual_data_low_byte(g_IomDevHdl, &data);
            am_devices_tsl2540_read_visual_data_high_byte(g_IomDevHdl, &data);
            am_devices_tsl2540_read_is_data_low_byte(g_IomDevHdl, &data);
            am_devices_tsl2540_read_ir_data_high_byte(g_IomDevHdl, &data);
            ui32Status = am_devices_tsl2540_nonblocking_read_id(g_IomDevHdl, &data, pfnWidget_IOM_Callback_Read);
            
            //
            // Wait until the read has completed
            //
            wait_for_nonblock_response(pTestCfg->sleepMode, &g_bIOMNonBlockCompleteRd);
            
            //
            // Check for completion and accuracy of data
            //
            if ((g_bIOMNonBlockCompleteRd != 1) || (AM_DEVICES_TSL2540_ID != data))
            {
                failures++;
            }
            else
            {
                passes++;
            }   
        }
    }
    
    //
    // Disable the RTC so the test fully stops
    //
    am_hal_rtc_osc_disable();
                 
    am_util_stdio_printf("\n\nPASSES = %d, FAILURES = %d\n", passes, failures);
        
    //
    // Compare the receive buffer to the transmit buffer.
    //
    if (ui32Status || (failures > 0))
    {
        return AM_WIDGET_ERROR;
    }
    else
    {
        return AM_WIDGET_SUCCESS;
    }
}

uint32_t g_UnExpReturnCount = 0;
uint32_t g_ExpErrCount = 0;

void
errCallback(void *pCallbackCtxt, uint32_t transactionStatus)
{
    uint32_t ui32ExpectedError = (uint32_t)pCallbackCtxt;
    if (transactionStatus != ui32ExpectedError)
    {
        am_util_stdio_printf("Call back returned - Expected Value = %2X | Actual Value = %2X\n", ui32ExpectedError, transactionStatus);
        g_UnExpReturnCount++;
    }
    else
    {
        g_ExpErrCount++;
    }
}
