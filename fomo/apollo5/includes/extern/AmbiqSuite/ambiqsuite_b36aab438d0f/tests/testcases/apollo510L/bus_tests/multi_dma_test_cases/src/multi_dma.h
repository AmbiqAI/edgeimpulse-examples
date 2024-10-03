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


#define DEBUG_PIN_IOM1  5
#define DEBUG_PIN_SDIO  11
#define DEBUG_PIN_MSPI0 29
#define DEBUG_PIN_MSPI1 30
#define DEBUG_PIN_MSPI2 31


extern bool iom_init(void);
extern bool iom_start(void);
extern bool iom_wait(void);

extern bool flash_init(void);
extern bool flash_start(void);
extern bool flash_wait(void);

extern bool psram_init(void);
extern bool psram_start(void);
extern bool psram_wait(void);

extern bool adc_init(void);
extern bool adc_start(void);
extern bool adc_wait(void);

extern bool sdio_init(void);
extern bool sdio_write(void);
extern bool sdio_read(void);

#ifdef __cplusplus
}
#endif

#endif /*MULTI_DMA_H*/