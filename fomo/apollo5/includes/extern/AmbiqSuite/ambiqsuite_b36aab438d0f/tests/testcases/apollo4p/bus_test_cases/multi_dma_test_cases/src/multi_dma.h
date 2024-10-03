//*****************************************************************************
//
//! @file multi_dma.h
//!
//! @brief Common include file for multi_dma testcase
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
#ifndef MULTI_DMA_H
#define MULTI_DMA_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Debug pin defines
//
//*****************************************************************************
#ifdef USE_DEBUG_PIN
#define DEBUG_PIN_1 37 //used by iom0
#define DEBUG_PIN_2 38 //used by iom1
#define DEBUG_PIN_3 39 //used by iom2
#define DEBUG_PIN_4 40 //used by reg read
#define DEBUG_PIN_5 41 //used by iom4
#define DEBUG_PIN_6 42 //used by iom5
#define DEBUG_PIN_7 44 //used by iom6
#define DEBUG_PIN_8 45 //used by iom7
#define DEBUG_PIN_9 54 //used by mspi psram
#define DEBUG_PIN_10 56 //used by mspi flash
#define DEBUG_PIN_11 100 //used by mspi psram
#define DEBUG_PIN_12 90 //used by mspi flash
#endif


extern bool iom_init(void);
extern bool iom_start(void);
extern bool iom_wait(void);
extern bool iom_deinit(void);

extern bool flash_init(void);
extern bool flash_start(void);
extern bool flash_wait(void);
extern bool flash_deinit(void);

extern bool psram_init(void);
extern bool psram_start(void);
extern bool psram_wait(void);
extern bool psram_deinit(void);

extern bool adc_init(void);
extern bool adc_start(void);
extern bool adc_wait(void);

extern bool audadc_init(void);
extern bool audadc_start(void);
extern bool audadc_wait(void);

#ifdef __cplusplus
}
#endif

#endif /*MULTI_DMA_H*/