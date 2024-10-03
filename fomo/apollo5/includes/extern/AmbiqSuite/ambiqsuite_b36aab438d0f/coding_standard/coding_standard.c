//*****************************************************************************
//
//! @file coding_standard.c
//!
//! @brief Description of the purpose of the c file.
//!
//! This file is a dummy c file used to demonstrate some important aspects of
//! the Ambiq Micro coding standard.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "coding_standard.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define SOME_MACRO              42          //!< This is the answer

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//!
uint32_t g_ui32StaticGlobalVariable;        //!<
//!
uint32_t g_ui32Stuff;

//*****************************************************************************
//
//! @brief Stub of a function to show our naming convention
//!
//! The parameter list was left out completely here, since there are no
//! parameters.
//!
//! @return None.
//
//*****************************************************************************
void
coding_standard_noun_verb(void)
{
}

//*****************************************************************************
//
//! @brief Short description of coding_standard_function_name()
//!
//! @param ui32InputVariable - Brief description of parameter.
//! @param ui32Output - You only have a few characters, so make it count.
//! @param pcStr - Make sure not to leave any parameters out.
//! @param pcStr2 - Always include at least a few words about each parameter.
//!
//! Here is a longer function description. This can wrap for several lines as
//! well, and probably will in most cases. This should be enough of a
//! description for someone to understand how to use the function in their
//! application. If you reference parameters like \e ui32Output, make sure to
//! emphasize them with the appropriate markdown sequence.
//!
//! \e ui32Output If you need to elaborate on specific parameters (maybe your
//! one-liner isn't enough) do it in new paragraphs down here. Start your
//! paragraph with the emphasized parameter name.
//!
//! @note If there is anything special that you want to highlight about a
//! function, especially important usage concerns, put them in a note here.
//!
//! @return None.
//
//*****************************************************************************
void
coding_standard_function_name(uint32_t ui32InputVariable, uint32_t *ui32Output,
                              char *pcStr, char *pcStr2)
{
    uint8_t ui8Index, ui8LocalVar;

    ui8Index = 2;

    switch (ui8LocalVar)
    {
        case 1:
            do_things();
            do_more_things();
            break;

        case 2:
            do_different_things();
            break;

        default:
            do_default_things();
            break;
    }

    while (1);

    //
    // Check something.
    //
    if (something)
    {
        //
        // If something is true, we do_this()
        //
        do_this();
    }
    else
    {
        //
        // Otherwise, we do_this_other_thing()
        //
        do_this_other_thing();
    }

    do
    {
        //
        // Things
        //
        do_some_stuff();
    }
    while (condition);
}
