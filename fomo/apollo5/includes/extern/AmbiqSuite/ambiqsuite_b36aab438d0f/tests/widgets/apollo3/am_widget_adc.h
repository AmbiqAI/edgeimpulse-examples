//*****************************************************************************
//
//! @file am_widget_adc.h
//!
//! @brief This widget allows test cases to exercise the Apollo2 ADC using an
//! adafruit MCP4725 12-bit DAC, IOM #0, and CTIMERS A0 and A3.
//!
//!
//! @addtogroup widgets Test Framework Widgets
//! @addtogroup adc ADC
//! @ingroup widgets
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_WIDGET_ADC_H
#define AM_WIDGET_ADC_H

#ifdef __cplusplus
extern "C"
{
#endif

#define SAMPLES         (512)  
#define FFT_SIZE        (SAMPLES/2)
#define ADC_TEST_MODULE 0


typedef enum 
{
  AM_WIDGET_WAVEFORM_SINE,
  AM_WIDGET_WAVEFORM_TRIANGLE,
  AM_WIDGET_WAVEFORM_SQUARE
} am_waveform_t;

typedef struct
{
  // Select the desired test waveform.
  am_waveform_t                 eWaveform;
  
  // Select the desired test waveform frequency.
  uint32_t                      ui32Frequency;

  // Select the IOM to use for waveform generation.
  uint32_t                      ui32IOM;
  
  // Select the I2C Address for the DAC.
  uint32_t                      ui32I2CAddress;

  // ADC Configuration
  am_hal_adc_config_t           ADCConfig;
  
  // Slot number.
  uint32_t                      ui32SlotNumber;
  
  // Slot configuration.
  am_hal_adc_slot_config_t      SlotConfig;
  
  // Window configuration
  am_hal_adc_window_config_t    WindowConfig;
  
  // Pointer to the DMA buffer.
  uint32_t                      *pDMABuffer;
  
  // DMA buffer size.
  uint32_t                      DMABufSize;
  
  // Pointer to the output sample buffer.
  am_hal_adc_sample_t           *pSampleBuf;
  
  // Size of the sample buffer.
  uint32_t                      SampleBufSize;
  
  // Use FIFO
  bool                          useFifo;
  
  // Use DMA
  bool                          useDMA;
  
} am_widget_adc_config_t;

extern uint32_t am_widget_adc_setup(am_widget_adc_config_t *pADCTestConfig, void **ppWidget, char *pErrStr);
extern uint32_t am_widget_adc_test(void *pWidget, char *pErrStr);
extern uint32_t am_widget_adc_dma_test(void *pWidget, char *pErrStr);
extern uint32_t am_widget_adc_cleanup(void *pWidget, char *pErrStr);
extern uint32_t am_widget_adc_analyze_fft(void *pWidget, char *pErrStr);
extern bool am_widget_adc_analyze_edges(void *pWidget, char *pErrStr);
extern void am_widget_adc_analyze_comparator(void *pWidget, char *pErrStr, uint32_t *ExclusionCount, uint32_t *InclusionCount);
extern void am_widget_adc_analyze_fifo(void *pWidget, char *pErrStr, uint32_t *FifoCount);
extern void am_widget_adc_analyze_dma(void *pWidget, char *pErrStr, uint32_t *DMACount);

#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_ADC_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
