//*****************************************************************************
//
//! @file coding_standard.h
//!
//! @brief Brief description of the header. No need to get fancy here.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef CODING_STANDARD_H
#define CODING_STANDARD_H

//
// Put additional includes here if necessary.
//
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define GLOBALLY_IMPORTANT      (6 * 7)     //!< Very brief description

//*****************************************************************************
//
//! @brief Example definition of a function-like macro
//!
//! These should have descriptions if you expect them to be used and understood
//! by end-users. If your macro is getting really long or complicated, please
//! consider whether it makes more sense to define an actual function instead.
//
//*****************************************************************************
#define INLINED_FUNCTION(x)                                                   \
    do                                                                        \
    {                                                                         \
        actions(x);                                                           \
    }                                                                         \
    while(0);

//*****************************************************************************
//
//! @brief Example definition of a struct
//!
//! Add a detailed description here if the user will need more background
//! information to use the struct successfully. In some cases, a long
//! description will be overkill. Use your best judgment here.
//
//*****************************************************************************
typedef struct
{
    //! Short description of each member should go here.
    uint32_t ui32Member1;

    //! Short description of the second member.
    uint32_t ui32Member2;
}
CodingStandardStructName_t;


//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern uint32_t g_ui32Stuff;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void coding_standard_function_name(uint32_t ui32InputVariable,
                                          uint32_t ui32Output, char *pcStr,
                                          char *pcStr2);

#ifdef __cplusplus
}
#endif

#endif // CODING_STANDARD_H
