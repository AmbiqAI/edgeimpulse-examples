//*****************************************************************************
//
//! @file am_widget_uart.h
//!
//! @brief
//!
//! @addtogroup hal Hardware Abstraction Layer (HAL)
//! @addtogroup ctimer Counter/Timer (CTIMER)
//! @ingroup hal
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
#ifndef AM_WIDGET_UART_H
#define AM_WIDGET_UART_H

#ifdef __cplusplus
extern "C"
{
#endif
  
  
  typedef struct
  {
    am_hal_mspi_dev_config_t      MSPIConfig;             // MSPI configuration.
    uint32_t                      ui32DMACtrlBufferSize;  // DMA control buffer size.
    uint32_t                      *pDMACtrlBuffer;        // DMA control buffer.
    uint32_t                      NumBytes;               // Number of bytes to read/write.
    uint32_t                      SectorAddress;          // Sector address.
    uint32_t                      ScramblingRegionStart;  // Address of start of scrambling region.
    uint32_t                      ScramblingRegionEnd;    // Address of end of scrambling region.
    uint32_t                      UnscrambledSector;      // Address of the unscrambled sector.
  } am_widget_mspi_config_t;
  
  typedef struct
  {
    uint32_t                      NumBytes;               // Number of bytes to read/write.
    uint32_t                      ByteOffset;             // Byte offset in the RX/TX buffers.
    uint32_t                      SectorAddress;          // Sector address.
    uint32_t                      SectorOffset;           // Offset into Sector to write/read.
    bool                          TurnOnCaching;          // Turn on D/I caching.
    uint32_t                      UnscrambledSector;      // Address of the unscrambled sector.
  } am_widget_mspi_test_t;
  
#define MSPI_XIP_BASE_ADDRESS 0x04000000
  
  extern uint32_t am_widget_mspi_octal_setup(am_widget_mspi_config_t *pMSPITestConfig, void **ppWidget, char *pErrStr);
  extern uint32_t am_widget_mspi_octal_cleanup(void *pWidget, char *pErrStr);
  extern bool am_widget_mspi_octal_test_get_eflash_id(void *pWidget, char *pErrStr);
  extern bool am_widget_mspi_octal_test_write_read(void *pWidget, void *pTestCfg, char *pErrStr);
  extern bool am_widget_mspi_octal_test_write_queue_read(void *pWidget, void *pTestCfg, char *pErrStr);
  extern bool am_widget_mspi_octal_test_xip_databus(void *pWidget, void *pTestCfg, char *pErrStr);
  extern bool am_widget_mspi_octal_test_xip_instrbus(void *pWidget, void *pTestCfg, char *pErrStr);
  extern bool am_widget_mspi_octal_test_scrambling(void *pWidget, void *pTestCfg, char *pErrStr);
  
#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_UART_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
