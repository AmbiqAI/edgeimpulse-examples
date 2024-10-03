//*****************************************************************************
//
//! @file am_types.h
//!
//! @brief Definition of common data types.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_TYPES_H
#define AM_TYPES_H



#ifdef __cplusplus
extern "C" {
#endif

#if ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) && \
     (!defined(__ICC8051__) || (__ICC8051__ == 0)))
#include <stdint.h>
#else
typedef signed char     int8_t;
typedef unsigned char   uint8_t;
typedef signed short    int16_t;
typedef unsigned short  uint16_t;
typedef signed long     int32_t;
typedef unsigned long   uint32_t;
#endif

#ifndef NULL
#define NULL  0
#endif

#ifndef true
#define true  1
#endif

#ifndef false
#define false 0
#endif

/* Boolean data type */
typedef uint8_t bool_t;



#ifdef __cplusplus
};
#endif

#endif /* AM_TYPES_H */
