//*****************************************************************************
//
//! @file am_util_cmdline.c
//!
//! @brief Functions to implement a simple command line interface.
//!
//! Functions supporting a command-line interface.
//!
//! @addtogroup cmdline CMDLINE - Simple command Line Interface
//! @ingroup utils
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

#include <stdint.h>
#include <stdbool.h>
#include "am_util_cmdline.h"
#include "am_util_string.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//
//! cmdline uart instance
//! @note - the UART Instance should actually be specified by the BSP.
//
#define CMDLINE_UART_INST                   0

//
//! Max number of cmdline args
//
#define MAX_CMDLINE_ARGS                    10

//*****************************************************************************
//
//! Interface parameter structure
//
//*****************************************************************************
am_util_cmdline_interface_t *g_psInterface;

//*****************************************************************************
//
// Character parsing state information.
//
//*****************************************************************************
uint32_t g_ui32BufferIndex = 0;
bool g_bQuoted = 0;
bool g_bEscaped = 0;
bool g_bPromptNeeded = 0;

//*****************************************************************************
//
// Command execution data.
//
//*****************************************************************************
char *g_ppcArgs[MAX_CMDLINE_ARGS];
uint32_t g_ui32Argc = 0;

//*****************************************************************************
//
// Initialize the command line.
//
//*****************************************************************************
void
am_util_cmdline_init(am_util_cmdline_interface_t *psInterface)
{
    g_psInterface = psInterface;
    g_bQuoted = false;
    g_bEscaped = false;
    g_bPromptNeeded = true;
    g_ui32BufferIndex = 0;
    g_ppcArgs[0] = psInterface->psCommandData;
    g_ui32Argc = 0;
}

//*****************************************************************************
//
// Parses characters as they come in through the interface. If the return value
// is true, there is a command to execute.
//
//*****************************************************************************
bool
parse_char(char cChar)
{
    //
    // Check the state variables to figure out the correct interpretation of
    // this character.
    //
    if ( cChar == 0x7F || cChar == 0x08 || cChar == '\f' )
    {
        //
        // First, if the character was a backspace or delete, clear out
        // everything and return.
        //
        g_bQuoted = false;
        g_bEscaped = false;
        g_ui32BufferIndex = 0;
        g_psInterface->psCommandData[0] = 0;
        g_ppcArgs[0] = g_psInterface->psCommandData;
        g_ui32Argc = 0;
    }
    else if ( g_bEscaped )
    {
        //
        // If we're currently in an 'escape' sequence, print whatever character
        // comes next, no matter what.
        //
        g_psInterface->psCommandData[g_ui32BufferIndex] = cChar;
        g_ui32BufferIndex++;
        g_bEscaped = false;
    }
    else if ( g_bQuoted )
    {
        //
        // If we're in a quoted context, look out for end quotes, and
        // backslashes. Everything else is handled as-is.
        //
        if ( cChar == '"' )
        {
            g_bQuoted = false;
        }
        else if ( cChar == '\\' )
        {
            g_bEscaped = true;
        }
        else
        {
            g_psInterface->psCommandData[g_ui32BufferIndex] = cChar;
            g_ui32BufferIndex++;
        }
    }
    else
    {
        //
        // If we're not in any special context, all characters retain their
        // special meanings.
        //
        if ( cChar == '"' )
        {
            g_bQuoted = true;
        }
        else if ( cChar == '\\' )
        {
            g_bEscaped = true;
        }
        else if ( cChar == ' ' )
        {
            //
            // Spaces delimit arguments, so we need to replace them with NULL
            // terminators.
            //
            g_psInterface->psCommandData[g_ui32BufferIndex] = 0;
            g_ui32BufferIndex++;

            //
            // Also adjust the argument lists as appropriate
            //
            g_ui32Argc++;
            g_ppcArgs[g_ui32Argc] = g_psInterface->psCommandData + g_ui32BufferIndex;
        }
        else if ( cChar == '\n' || cChar == '\r' )
        {
            //
            // New lines delimit entire commands, so we need to replace them
            // with NULL terminators.
            //
            g_psInterface->psCommandData[g_ui32BufferIndex] = 0;
            g_ui32BufferIndex++;

            //
            // Also adjust the argument lists as appropriate.
            //
            g_ui32Argc++;

            return true;
        }
        else
        {
            //
            // If none of the other cases caught this character, it should just
            // be copied into the command buffer as is.
            //
            g_psInterface->psCommandData[g_ui32BufferIndex] = cChar;
            g_ui32BufferIndex++;
        }
    }

    //
    // Make sure we're not about to overflow the command buffer. If we are,
    // just end the function here, and report "true" in hopes that the command
    // can be identified.
    //
    if ( g_ui32BufferIndex > g_psInterface->ui32CommandDataLen )
    {
        return true;
    }

    //
    // Also make sure to check the maximum number of arguments to make sure
    // this buffer doesn't overflow either.
    //
    if ( g_ui32Argc >= MAX_CMDLINE_ARGS )
    {
        return true;
    }

    //
    // If we haven't returned by this point, this character can be assumed not
    // to be the last character of a command.
    //
    return false;
}

//*****************************************************************************
//
// Simple function for printing the prompt string.
//
//*****************************************************************************
void
print_prompt(void)
{
    char *pcChar;

    pcChar = g_psInterface->pcPromptString;

    while ( *pcChar )
    {
        g_psInterface->pfnPutChar(CMDLINE_UART_INST, *pcChar);
        pcChar++;
    }
}

//*****************************************************************************
//
// Echoes characters back to the user interface as they are received. Certain
// characters are handled differently.
//
//*****************************************************************************
void
echo_char(char cChar)
{
    //
    // If there isn't an output function, just return.
    //
    if ( !g_psInterface->pfnPutChar )
    {
        return;
    }

    switch ( cChar )
    {
        case '\r':
        case '\n':
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, '\r');
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, '\n');
        break;

        case 0x7F:
        case 0x08:
            //
            // Erase the line.
            //
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, '\033');
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, '[');
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, '2');
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, 'K');
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, '\r');

            //
            // Print the prompt.
            //
            print_prompt();
        break;

        case '\033':
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, '\\');
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, 'e');
        break;

        case '\f':
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, '\f');

            //
            // Print the prompt.
            //
            print_prompt();
        break;

        default:
            g_psInterface->pfnPutChar(CMDLINE_UART_INST, cChar);
    }
}

//*****************************************************************************
//
// Execute a command by name.
//
//*****************************************************************************
uint32_t
am_util_cmdline_run_command(char **args, uint32_t argc)
{
    am_util_cmdline_command_t *psCommands;
    uint32_t ui32Index, ui32NumCommands, ui32CommandDataLen;
    char *pcCommand;

    //
    // Grab a few important parameters from the global structure.
    //
    psCommands = g_psInterface->psCommandList;
    ui32NumCommands = g_psInterface->ui32NumCommands;
    ui32CommandDataLen = g_psInterface->ui32CommandDataLen;

    //
    // Loop over the commands in the global table.
    //
    for ( ui32Index = 0; ui32Index < ui32NumCommands; ui32Index++ )
    {
        //
        // Check the command name against the first argument.
        //
        pcCommand = psCommands[ui32Index].pcCommand;

        if ( !am_util_string_strncmp(pcCommand, args[0], ui32CommandDataLen) )
        {
            //
            // If the command matches the argument, run the command and return.
            //
            return psCommands[ui32Index].pfnCommand(args, argc);
        }
    }

    //
    // Return a negative one to indicate that there was no command found.
    //
    return 0xffffffff;
}

//*****************************************************************************
//
// Look for and process any incoming commands.
//
//*****************************************************************************
void
am_util_cmdline_process_commands(void)
{
  char cChar = {0};

    //
    // If we need to print a prompt, do it now.
    //
    if ( g_bPromptNeeded )
    {
        print_prompt();
        g_bPromptNeeded = false;
    }

    //
    // As long as there are characters to get, keep reading them.
    //
    while ( g_psInterface->pfnGetChar(&cChar) == 0 )
    {
        //
        // Echo the character back to the interface.
        //
        echo_char(cChar);

        //
        // Run the parser to see if this char completed a command.
        //
        if ( parse_char(cChar) )
        {
            //
            // If a command is ready to go, run it now. This function call will
            // return the return code of the command function that it calls. This
            // may be used in later implementations for error-checking and
            // error-reporting.
            //
            am_util_cmdline_run_command(g_ppcArgs, g_ui32Argc);

            //
            // Reset the state variables to prepare for the next command.
            //
            g_bQuoted = false;
            g_bEscaped = false;
            g_bPromptNeeded = true;
            g_ui32BufferIndex = 0;
            g_psInterface->psCommandData[0] = 0;
            g_ppcArgs[0] = g_psInterface->psCommandData;
            g_ui32Argc = 0;

            return;
        }
    }
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

