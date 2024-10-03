//*****************************************************************************
//
//! @file am_devices_ambt53_pwrdown.c
//!
//! @brief Handle ambt53 power down operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "am_devices_ambt53_pwrdown.h"
#include "rpc_common.h"
#include "am_devices_ambt53_scpm.h"

//*****************************************************************************
//
// Macro definitions.
//
//*****************************************************************************

#define AMBT53_EVECTOR                 0x20000
//*****************************************************************************
//
// Global variable.
//
//*****************************************************************************
bool bEnablePowerDownMode = false;
bool bIsPowerDown = false;
bool bIsIpcMsgBlocked = false;
bool bIsMcuNeedRebootAmbt53 = false;
static uint8_t ForceAmbt53ActiveReq;
static bool bRebootAmbt53End = true;
bool bIsPwrindIntrEnable = false;
extern am_devices_mspi_ambt53_config_t stRPMsgConfig;
extern void* g_RpmsgDevHdl;
extern void* pvRpmsgMspiHandle;
static void (*pfnBlockedMessageProcessCb)(void) = NULL;

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
static void am_devices_ambt53_recovery_handler(void);
static void am_devices_ambt53_release_external_wakeup(void);
static void am_devices_ambt53_process_blocked_message(void);
//*****************************************************************************
//
// Interrupt handler.
//
//*****************************************************************************

static void PdIndIntService(void)
{
    if ( bIsPwrindIntrEnable == false )
    {
        return;
    }
    dsp_status_reg_t dsp_status;

    dsp_status.value = am_hal_scpm_get_dsp_status();
    if (dsp_status.fields.pwrdown_cxout0 == 0)
    {
        am_util_debug_printf("Exiting powerdown...\r\n");
        bIsPowerDown = false;
        if ( bIsIpcMsgBlocked )
        {
            bIsIpcMsgBlocked = false;
            am_devices_ambt53_process_blocked_message();
        }
        bRebootAmbt53End = true;
    }
}

static void OscEnIntService(void)
{
    dsp_status_reg_t dsp_status;

    dsp_status.value = am_hal_scpm_get_dsp_status();

    if ( dsp_status.fields.osc_en ) //wake up
    {
        am_devices_ambt53_release_external_wakeup();
        am_util_debug_printf("osc_en wake up\r\n");
        if ((bIsPowerDown) && (bRebootAmbt53End == true) && (bIsMcuNeedRebootAmbt53 == true))
        {
             am_util_debug_printf("ambt53_recovery_handler\r\n");
            bIsMcuNeedRebootAmbt53  = false;
            am_devices_ambt53_recovery_handler();
            bRebootAmbt53End = false;
            // re_enable the power down indication interrupt
            am_hal_scpm_intr_enable(SCPM_IRQ_CXOUT0, true);
            bIsPwrindIntrEnable = true;

            //update the OSC_EN interrupt mode to falling edge
            am_hal_scpm_intr_mode_update(SCPM_IRQ_OSCEN, SCPM_IRQ_NEG_EDGE);
        }
        else
        {
            if ( bIsIpcMsgBlocked )
            {
                bIsIpcMsgBlocked = false;
                am_devices_ambt53_process_blocked_message();
            }
        }
    }
    else //btdm entered deep sleep
    {
        am_util_debug_printf("osc_en deep sleep\r\n");
    }
}

void IdleIndIntService(void)
{
    dsp_status_reg_t dsp_status;

    dsp_status.value = am_hal_scpm_get_dsp_status();

    if (dsp_status.fields.btdm_dsp_cevax_psu_dsp_idle_r)
    {
        am_util_debug_printf("Power down mode entered\r\n");
        // check the ambt53 entered the power down ready
        if ((dsp_status.fields.pwrdown_cxout0) && (dsp_status.fields.osc_en == 0))
        {
            // TODO Disable ambt53 clock and power
            bIsMcuNeedRebootAmbt53 = true;
            bIsPowerDown = true;

            // disable the power down indication interrupt to avoid it's triggered by mistake
            am_hal_scpm_intr_enable(SCPM_IRQ_CXOUT0, false);
            bIsPwrindIntrEnable = false;

            // to simulate the mcu disable ambt53 clock and power by reset the ambt53
            am_hal_scpm_set_globrst();
            am_hal_scpm_irq_clr(SCPM_IRQ_CXOUT0);
            am_util_debug_printf("reset the ambt53\r\n");
        }
    }
    //Enable the OSC_EN interrupt mode to rising edge
    am_hal_scpm_intr_mode_update(SCPM_IRQ_OSCEN, SCPM_IRQ_POS_EDGE);
}

//
// AMBT53 Core recovery handler.
//
//*****************************************************************************
static void am_devices_ambt53_recovery_handler(void)
{
    // Need keep global and core reset to low before EVECTOR configuration
    am_hal_scpm_set_cxrst();
    am_hal_scpm_set_globrst();
    am_util_delay_us(10);

    // Delay at least 8 core cycles before releasing global reset
    am_util_delay_us(2);

    am_hal_scpm_release_globrst();
    am_util_delay_ms(1);

    // Need to reinitialize the xSPI remote since it has been reset.
    uint32_t ui32Status;
    ui32Status = am_devices_mspi_ambt53_deinit(g_RpmsgDevHdl);
    if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
    {
        am_util_debug_printf("MSPI Deinit fail %d\n", ui32Status);
        return;
    }

    ui32Status = am_devices_mspi_ambt53_init(MSPI_AMBT53_MODULE, &stRPMsgConfig, &g_RpmsgDevHdl, &pvRpmsgMspiHandle);
    if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
    {
        am_util_debug_printf("MSPI Init fail %d\n", ui32Status);
        return;
    }

    ui32Status = am_devices_mspi_ambt53_enable_xip(g_RpmsgDevHdl);
    if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
    {
        am_util_debug_printf("XIP Enable fail %d\r\n", ui32Status);
        return;
    }

    // am_devices_mspi_ambt53_basic_config_get(g_RpmsgDevHdl);
    ui32Status = am_devices_ambt53_mailbox_init(MAILBOX_D2M_THRESHOLD, MAILBOX_INT_THRESHOLD_Msk | MAILBOX_INT_ERROR_Msk);
    if (AM_DEVICES_AMBT53_STATUS_SUCCESS != ui32Status)
    {
        am_util_debug_printf("Mailbox Enable fail %d\r\n", ui32Status);
        return;
    }

    // Releasing the core reset, the reset should boot up from the configured evector address
    am_hal_scpm_release_cxrst();
}
//*****************************************************************************
//
// Perform initial setup for the power down task.
//
//*****************************************************************************
void am_devices_ambt53_pwrdown_setup(bool power_down_enable)
{
    bEnablePowerDownMode = power_down_enable;
    am_hal_scpm_intr_init();
    am_util_debug_printf("Ambt53 pwrdown enable = %d\r\n", power_down_enable);

    if ( bEnablePowerDownMode == true )
    {
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

        //
        // If not a multiple of 1024 bytes, append a plus sign to the KB.
        //
        ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32MRAMSize % 1024 ) ? '+' : 0;
        am_util_stdio_printf("\tMRAM size:   %7d (%d KB%s)\n",
                            sIdDevice.sMcuCtrlDevice.ui32MRAMSize,
                            sIdDevice.sMcuCtrlDevice.ui32MRAMSize / 1024,
                            &ui32StrBuf);

        ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32DTCMSize % 1024 ) ? '+' : 0;
        am_util_stdio_printf("\tDTCM size:   %7d (%d KB%s)\n",
                            sIdDevice.sMcuCtrlDevice.ui32DTCMSize,
                            sIdDevice.sMcuCtrlDevice.ui32DTCMSize / 1024,
                            &ui32StrBuf);

        ui32StrBuf = ( sIdDevice.sMcuCtrlDevice.ui32SSRAMSize % 1024 ) ? '+' : 0;
        am_util_stdio_printf("\tSSRAM size:  %7d (%d KB%s)\n\n",
                            sIdDevice.sMcuCtrlDevice.ui32SSRAMSize,
                            sIdDevice.sMcuCtrlDevice.ui32SSRAMSize / 1024,
                            &ui32StrBuf);
    #endif // AM_DEBUG_PRINTF
        // #### INTERNAL END ####

        // Register the oscen isr callback into SCPM interrupt handlers.
        am_devices_ambt53_interrupt_register(SCPM_IRQ_OSCEN, SCPM_IRQ_NEG_EDGE, &OscEnIntService);

        // Register the cxout0 callback into SCPM interrupt handlers.
        am_devices_ambt53_interrupt_register(SCPM_IRQ_CXOUT0, SCPM_IRQ_NEG_EDGE, &PdIndIntService);

        // Register the dsp idle isr callback into SCPM interrupt handlers.
        am_devices_ambt53_interrupt_register(SCPM_IRQ_DSP_IDLE, SCPM_IRQ_POS_EDGE, &IdleIndIntService);
        am_hal_scpm_intr_mode_update(SCPM_IRQ_OSCEN, SCPM_IRQ_NEG_EDGE);
        am_hal_scpm_intr_mode_update(SCPM_IRQ_DSP_IDLE, SCPM_IRQ_POS_EDGE);
    }
    // Set the evector to 0x20000 to make the ambt53 to boot from the boot recovery routine.
    am_hal_scpm_set_dsp_evector(AMBT53_EVECTOR);
}

//*****************************************************************************
//
// AMBT53 external wake up the Bluebud and BTDM
//
//*****************************************************************************
void am_devices_ambt53_wakeup_from_pwrdown(void)
{
    if ( bEnablePowerDownMode == true )
    {
        am_hal_scpm_set_ext_wakeup();
    }
}

//*****************************************************************************
//
// Release AMBT53 external wake up request
//
//*****************************************************************************
void am_devices_ambt53_release_external_wakeup(void)
{
    if ( bEnablePowerDownMode == true )
    {
        am_hal_scpm_release_ext_wakeup();
    }
}

//*****************************************************************************
//
// AMBT53 request the ambt53 not enter power down mode
//
//*****************************************************************************
void am_devices_force_ambt53_active_req(force_req ipc_event)
{
    ForceAmbt53ActiveReq |= ipc_event;
    am_hal_scpm_force_active_enable(true);
}

//*****************************************************************************
//
// Release force ambt53 active request
//
//*****************************************************************************
void am_devices_force_ambt53_active_release(force_req ipc_event)
{
    ForceAmbt53ActiveReq  &= (~ipc_event);
    if ( ForceAmbt53ActiveReq == 0 )
    {
        am_hal_scpm_force_active_enable(false);
    }
}

/**
 ****************************************************************************************
 * @brief Register BlockedMessageProcessCb callback function
 *
 ****************************************************************************************
 */
void am_devices_ambt53_register_wakeup_cb(void (*pfn_callback)())
{
    pfnBlockedMessageProcessCb = pfn_callback;
}

//*****************************************************************************
//
// Process the blocked ipc messages
//
//*****************************************************************************
void am_devices_ambt53_process_blocked_message(void)
{
    if ( pfnBlockedMessageProcessCb != NULL )
    {
        pfnBlockedMessageProcessCb();
    }
}

//*****************************************************************************
//
// Check if the ambt53 is active
// return true if the ambt53 is in active;otherwise return false
//*****************************************************************************
bool am_devices_check_ambt53_active_state(void)
{
    dsp_status_reg_t dsp_status;

    dsp_status.value = am_hal_scpm_get_dsp_status();

    if ( (bEnablePowerDownMode == false )
       || ((dsp_status.fields.pwrdown_cxout0 == 0) && (dsp_status.fields.osc_en)) )
    {
        return true;
    }
    return false;
}
