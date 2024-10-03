/**
 *  \file vendor_specific_init.c
 *
 *
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */


/* ----------------------------------------- Header File Inclusion */
#include "vendor_specific_init.h"
#include "BT_hci_api.h"

DECL_CONST UCHAR vs_commands[] =
{
#include "vendor_specific_commands.c"
};

/* ----------------------------------------- External Global Variables */

/* ----------------------------------------- Exported Global Variables */
DECL_STATIC UINT32 vendor_specific_init_complete = V_INIT_FALSE;

/* ----------------------------------------- Static Global Variables */
DECL_STATIC UINT32 vs_command_index;
DECL_STATIC UINT32 vs_max_commands;

DECL_STATIC void (*vs_init_callback)(void);

/* ----------------------------------------- Functions */

void app_vendor_specific_init(void ((*callback) (void)))
{
    API_RESULT      retval;
    UINT16          hci_command;

    if (V_INIT_FALSE == vendor_specific_init_complete)
    {
        VS_INIT_TRC(
        "[VS INIT] Started vendor specific init procedure.\n");

        vs_command_index = 0;
        vs_max_commands = sizeof(vs_commands);

        if (NULL != callback)
        {
            vs_init_callback = callback;
            vendor_specific_init_complete = V_INIT_IN_PROGRESS;
        }
    }

    if (V_INIT_IN_PROGRESS == vendor_specific_init_complete)
    {
        /* Skip command type */
        vs_command_index += 1;

        /* Unpack the command opcode */
        hci_unpack_2_byte_param
        (
            &hci_command,
            &vs_commands[vs_command_index]
        );

        /* Skip command opcode */
        vs_command_index += 2;

        /* Send vendor specific command */
        retval = BT_hci_vendor_specific_command
                 (
                     hci_command,
                     (UCHAR *) &vs_commands[vs_command_index + 1],
                     vs_commands[vs_command_index]
                 );

        if(API_SUCCESS != retval)
        {
            VS_INIT_ERR(
            "[VS INIT] FAILED to send vendor specific command. retval = %02X\n",
            hci_command);
        }

        /* Skip command param and param length */
        vs_command_index += vs_commands[vs_command_index] + 1;

        /* Change the state on last vendor specific init command */
        if (vs_max_commands <= vs_command_index)
        {
            vendor_specific_init_complete = V_INIT_WAIT_FOR_COMPLETION;

            VS_INIT_TRC(
            "[VS INIT] Vendor specific init procedure completed.\n");
        }
    }
    else if (V_INIT_WAIT_FOR_COMPLETION == vendor_specific_init_complete)
    {
        vendor_specific_init_complete = V_INIT_COMPLETE;

        /* Give callback to application */
        if (NULL != vs_init_callback)
        {
            vs_init_callback();
        }
    }

    return;
}

