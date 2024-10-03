//*****************************************************************************
//
//! @file pwr_cmnErrorDefs.h
//!
//! @brief This defines and error return codes
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup pwr_32mhz 32Mhz XTHS and HFRC2 Power Example
//! @ingroup power_examples
//! @{
//!
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef PWR_CMNERRORDEFS_H
#define PWR_CMNERRORDEFS_H

typedef enum
{
    ePwrCmnNoError          = 0,
    ePwrCntrlCacheConfig    = 0x01000000,
    ePwrCntrlPwrPeriphDis   = 0x02000000,
    ePwrCntrlBleReset       = 0x03000000,
    ePwrCntrlDispMemCfg     = 0x04000000,
    ePwrCntrl

}
pwr_cmn_error_t;

#define ERROR_MACRO( status, errorFlag ) \
    if ( status ){                      \
        status = (status & 0x00FFFFFF) | errorFlag; \
        break;                                    \
    }


#endif //PWR_CMNERRORDEFS_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

