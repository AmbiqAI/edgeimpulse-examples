/*********************************************************
(C) 2014 EEMBC(R) and ULPBench(TM).  All rights reserved.                            

EEMBC ULPBench Software is a product of EEMBC and is provided 
under the terms of the ULPBench License that is distributed 
with the official EEMBC ULPBench Software release.  
The Software is the proprietary intellectual property of EEMBC 
and its members and is protected under all applicable laws, 
including all applicable copyright laws.  
If you received this EEMBC ULPBench Software 
without the accompanying ULPBench License, 
you must discontinue use and download the official release 
from http://www.eembc.org/benchmark/ulp_sl.php.  
**********************************************************/


// TesLists.c    mbutler    Jan 2013
//
// TES List-related data and functions
//=============================================================================

//=============================================================================
#include "TesTPI.h"

//=============================================================================
// Insert Event in list after prev Event
// NOTE: Caller must ensure PLT_INTERRUPTS_OFF !
void tesListInsertAfter( TesListElement * prev, TesListElement * element )
{
    element->next = prev->next;
    prev->next = element;
}

//=============================================================================
// Append Event to the end of the list
void tesQueueAppend( TesQueueHeader * queueHeader, TesListElement * element )
{
    PLT_INTERRUPTS_OFF;     // synchronize with interrupt handlers
    if (queueHeader->first == TES_LIST_END) {
        queueHeader->first = queueHeader->last = element;
        element->next = TES_LIST_END;
    } else {
        tesListInsertAfter(queueHeader->last, element);
        queueHeader->last = element;
    }
    PLT_INTERRUPTS_ON;
}

//=============================================================================
// Initialize an Event list to empty
// NOTE: Caller must ensure PLT_INTERRUPTS_OFF ! (if necessary)
void tesQueueSetEmpty( TesQueueHeader * queueHeader )
{
    queueHeader->first = queueHeader->last = TES_LIST_END;
}
