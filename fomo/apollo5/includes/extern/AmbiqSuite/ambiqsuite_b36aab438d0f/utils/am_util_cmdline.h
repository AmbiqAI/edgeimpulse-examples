//*****************************************************************************
//
//! @file am_util_cmdline.h
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
#ifndef AM_UTIL_CMDLINE_H
#define AM_UTIL_CMDLINE_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
//! Standard Command Function Pointer Type
//
//*****************************************************************************
typedef uint32_t (*am_util_cmdline_func_t)(char **args, uint32_t argc);

//*****************************************************************************
//
//! Command Structure.
//
//*****************************************************************************
typedef struct
{
    char *pcCommand;
    am_util_cmdline_func_t pfnCommand;
    char *pcHelpString;
}
am_util_cmdline_command_t;

//*****************************************************************************
//
//! Stucture defining how the cmdline utility may transmit or receive
//! characters.
//
//*****************************************************************************
typedef struct
{
    //
    //! Function used to grab input characters.
    //
    uint32_t (*pfnGetChar)(char *pcChar);

    //
    //! Function used to echo characters back to the output.
    //
    void (*pfnPutChar)(uint32_t ui32Module, char pcChar);

    //
    //! List of supported commands, along with their associated function
    //! pointers and help strings.
    //
    am_util_cmdline_command_t *psCommandList;

    //
    //! Number of supported commands. Usually just:
    //! sizeof(psCommandList) / sizeof(am_util_cmdline_command_t)
    //
    uint32_t ui32NumCommands;

    //
    //! Buffer space to use for incoming commands.
    //
    char *psCommandData;

    //
    //! Size of command buffer. Usually just: sizeof(psCommandData)
    //
    uint32_t ui32CommandDataLen;

    //
    //! Prompt String
    //
    char *pcPromptString;
}
am_util_cmdline_interface_t;

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief Initialize the command line.
//!
//! @param psInterface is a pointer to an interface structure defining key
//! characteristics about the command line interface.
//!
//! This function may be used to initialize a command prompt for user
//! interaction. Please see the documentation on am_util_cmdline_interface_t
//! for more details on command line configuration.
//!
//! @note This function must be the first cmdline function to be called in the
//! final application.
//
//*****************************************************************************
extern void am_util_cmdline_init(am_util_cmdline_interface_t *psInterface);

//*****************************************************************************
//
//! @brief Look for and process any incoming commands.
//!
//! This function should be called periodically to check for commands on the
//! user interface. Each call will read characters from the interface until it
//! either completes an entire command, or the provided pfnGetChar() function
//! returns an error. Echoing characters back to the user interface will be
//! handled by this function unless the pfnPutChar() function was not provided.
//
//*****************************************************************************
extern void am_util_cmdline_process_commands(void);

//*****************************************************************************
//
//! @brief Execute a command by name.
//!
//! @param args is an array of strings that make up the arguments of the
//! command.
//!
//! @param argc is the number of argument strings contained in args
//!
//! This function performs a lookup in the command table to find a function
//! whose command string matches the value of args[0]. If it finds a match, it
//! will run the function, passing along args and argc as its arguments. When
//! the inner function returns, the return code will be passed back up to the
//! caller.
//!
//! @return Returns the same value as the command function that was called, or
//! a -1 if the command could not be found.
//
//*****************************************************************************
extern uint32_t am_util_cmdline_run_command(char **args, uint32_t argc);

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_CMDLINE_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

