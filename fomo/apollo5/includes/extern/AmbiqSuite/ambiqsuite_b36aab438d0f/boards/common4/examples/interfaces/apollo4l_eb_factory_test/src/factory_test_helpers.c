//*****************************************************************************
//
//! @file factory_test_helpers.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


#include "factory_test_helpers.h"



int  memory_compare(const void* cs, const void* ct, uint32_t count)
{
    const unsigned char *su1, *su2;
    int res = 0;

    for (su1 = cs, su2 = ct; 0 < count; ++su1, ++su2, count--)
    {
        if ((res = *su1 - *su2) != 0)
        {
            break;
        }
    }

    return res;
}

int UnityEnd(struct UNITY_STORAGE_T *pHandle)
{
    struct UNITY_STORAGE_T *pUnity = pHandle;

    UnityPrint("%d Tests %d Failures \r\n", pUnity->NumberOfTests, pUnity->TestFailures);

    if (pUnity->TestFailures == 0U)
    {
        UnityPrint("OK\r\n");
    }
    else
    {
        UnityPrint("FAIL\r\n");
    }
    return 0;
}
