//*****************************************************************************
//
//! @file am_assert.c
//!
//! @brief Perform an assert action.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_assert.h"

/*************************************************************************************************/
/*!
 *  \def    am_assert
 *
 *  \brief  Perform an assert action.
 *
 *  \param  pFile   Name of file originating assert.
 *  \param  line    Line number of assert statement.
 */
/*************************************************************************************************/
void am_assert(const char *pFile, uint16_t line)
{
    (void) pFile;
    (void) line;

    volatile uint8_t escape = 0;

    /* spin forever if fatal error occurred */
    while(true)
    {
        /*
         *  However, you can exit with a debugger by setting variable 'escape'.
         *  Handy to see where the assert happened if you cannot view the call stack.
        */
        if (escape)
        {
            break;
        }
    }
}

