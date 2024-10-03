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

extern float get_time(void);
extern float g_DispStartTime, g_DispStopTime;
extern float g_PsramStartTime, g_PsramStopTime;
extern float g_SdioStartTime, g_SdioStopTime;
extern float g_FlashStartTime, g_FlashStopTime;
extern float g_AdcStartTime, g_AdcStopTime;
extern float g_IomStartTime[], g_IomStopTime[];
extern volatile bool g_bMspiFlashDMAFinish;
extern volatile bool g_bMspiPsramDMAFinish;
extern volatile bool bAsyncWriteIsDone;
extern volatile bool g_bIomDmaFinish[];
extern volatile bool g_bADCDMAComplete;
extern volatile bool g_bDisplayDone;

#ifdef __cplusplus
}
#endif

#endif /*MULTI_DMA_H*/