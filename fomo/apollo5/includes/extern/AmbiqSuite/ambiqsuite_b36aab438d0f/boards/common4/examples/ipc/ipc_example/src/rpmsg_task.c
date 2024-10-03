//*****************************************************************************
//
//! @file rpmsg_task.c
//!
//! @brief Task to handle rpmsg operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "ipc_service.h"
#include "../../../../../../third_party/open-amp/libmetal/lib/errno.h"
#include "ipc_example.h"
#include "am_devices_ambt53_pwrdown.h"
//*****************************************************************************
//
// Macro definitions.
//
//*****************************************************************************

//*****************************************************************************
//
// Global variable.
//
//*****************************************************************************
metal_thread_t rpmsg_proc_task_handle;

EventGroupHandle_t ipc_event  = NULL;

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
//Called from mailbox callback
static bool host_inst0_kick_cb(void* priv)
{
    *((uint32_t*)priv) |= IPC_INST0_KICK_BIT;
    return true;
}

//Define host IPC instance
struct ipc_device host_inst0 =
{
    .name = "host_inst0",
    .config =
    {
        ROLE_HOST,
        CONFIG_IPC_SHM_BASE_ADDRESS_INST0,
        CONFIG_IPC_SHM_SIZE,
        (HOST_KICK_DATA_INST0 << 16),
        (REMOTE_KICK_DATA_INST0 << 16),
        &host_inst0_kick_cb,
    },
    .api = NULL,
    .data = NULL,
};

//*****************************************************************************
//
// ambt53 MSPI Interrupt handler.
//
//*****************************************************************************
void
ambt53_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(pvRpmsgMspiHandle, &ui32Status, false);
    am_hal_mspi_interrupt_clear(pvRpmsgMspiHandle, ui32Status);
    am_hal_mspi_interrupt_service(pvRpmsgMspiHandle, ui32Status);
}

//*****************************************************************************
//
// ambt53 Mailbox Interrupt handler.
//
//*****************************************************************************
static void mailbox_isr(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t xResult;

    if (ipc_event != NULL)
    {
        xResult = xEventGroupSetBitsFromISR(ipc_event, IPC_MBOX_BIT, &xHigherPriorityTaskWoken);
        if ( xResult == pdPASS )
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
//*****************************************************************************
//
// Perform initial setup for the rpmsg task.
//
//*****************************************************************************
void
RPMsgTaskSetup(void)
{
    int status = 0;
    am_util_debug_printf("RPMsgTask: setup\r\n");
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

    // Boot the rpmsg.
    status = am_devices_ambt53_boot();
    if (status != 0)
    {
        am_util_stdio_printf("am_devices_ambt53_boot failed\n");
        return;
    }

    // Register the mailbox callback into SCPM interrupt handlers.
    am_devices_ambt53_interrupt_register(SCPM_IRQ_MAILBOX_THRESHOLD, SCPM_IRQ_POS_EDGE, mailbox_isr);

    // Create IPC event group
    if ((ipc_event = xEventGroupCreate()) == NULL)
    {
        am_util_stdio_printf("Create IPC event group failed");
        return;
    }

    status = ipc_instance_create(&host_inst0);
    if ((status < 0) && (status != -EALREADY))
    {
        am_util_stdio_printf("ipc_instance_create() failed\n");
        return;
    }

    status = ipc_test_setup(&host_inst0);
    if (status != 0)
    {
        am_util_stdio_printf("ipc_test_setup() failed\n");
    }
}

void
RPMsgProcessTask(void *pvParameters)
{
    EventBits_t event_to_handle = 0;
    uint32_t buf_received;
    while(1)
    {
        event_to_handle = xEventGroupWaitBits(ipc_event, IPC_ALL_BITS, pdTRUE, pdFALSE, portMAX_DELAY);
        while (event_to_handle & IPC_MBOX_BIT)
        {
            // inform the ambt53 don't enter power down
            am_devices_force_ambt53_active_req(IPC_READ);
            // wake up the ambt53
            am_devices_ambt53_wakeup_from_pwrdown();
            while(1)
            {
                if ( am_devices_check_ambt53_active_state() )
                {
                    break;
                }
            }
            if ( mailbox_event_handle(&buf_received) == 0 )
            {
                // Mailbox successfully handled, clear mbox event bit
                event_to_handle &= ~IPC_MBOX_BIT;
                if ( buf_received == 0 )
                {
                    am_devices_force_ambt53_active_release(IPC_READ);
                }
            }
        }

        if (buf_received & IPC_INST0_KICK_BIT)
        {
            if ( am_devices_check_ambt53_active_state() )
            {
                buf_received &= ~IPC_INST0_KICK_BIT;
                ipc_receive(&host_inst0);
                am_devices_force_ambt53_active_release(IPC_READ);
            }
        }

        if (event_to_handle & (IPC_EP0_SEND_BIT | IPC_EP1_SEND_BIT))
        {
            // inform the ambt53 don't enter power down
            am_devices_force_ambt53_active_req(IPC_WRITE);
            // wake up the ambt53
            am_devices_ambt53_wakeup_from_pwrdown();

            if ( am_devices_check_ambt53_active_state() )
            {
                ipc_send_data_event_handler(event_to_handle & (IPC_EP0_SEND_BIT | IPC_EP1_SEND_BIT));
                // inform the ambt53 can not enter power down until the tx finish
                am_device_ambt53_wait_mspi_idle(5);
                am_devices_force_ambt53_active_release(IPC_WRITE);
            }
            else
            {
                //block ipc messages tx
                am_hal_interrupt_master_disable();
                bIsIpcMsgBlocked = true;
                am_hal_interrupt_master_enable();
            }
            event_to_handle &= ~(IPC_EP0_SEND_BIT | IPC_EP1_SEND_BIT);
        }

        // Disable the interrupt to make the remote sender busy randomly
        am_hal_interrupt_master_disable();
        am_util_delay_cycles(random_num(20, 0));
        am_hal_interrupt_master_enable();
    }
}

