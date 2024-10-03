//*****************************************************************************
//
//! @file am_widget_iom_multidrop.h
//!
//! @brief iom multidrop utilities
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_WIDGET_IOM_MULITDROP_H
#define AM_WIDGET_IOM_MULITDROP_H

#include "am_devices_spipsram.h"

#define PSRAM_PATTERN_BUF_SIZE        128
//
// ! the 100msec test should run about 9.6 seconds
//
#define NUM_ITERATIONS          96

//
//! define widget operating states
//
typedef enum
{
    //
    //! default state
    //
    ePsramZero,
    //
    //! init state
    //
    ePsramInit,
    //
    //! reading
    //
    ePsramRead,
    //
    //! writing
    //
    ePsramWrite,
    //
    //! forces enum to use 4 bytes
    //
    ePsramX32 = 0x80000000ul,

}
psram_test_cb_states_e ;


//
//!
//
typedef struct
{
    //
    //!
    //
    psram_test_cb_states_e  ePsramState ;
}
psram_cb_params_t ;

//
//! typedef for non-blocking functions
//
typedef uint32_t (*fpNonBlockingt) (void *pHandle,
                                    uint8_t *ui8TxBuffer,
                                    uint32_t ui32Address,
                                    uint32_t ui32NumBytes,
                                    am_hal_iom_callback_t pfnCallback,
                                    void *pCallbackCtxt) ;

//
//! typedef for blocking functions
//
typedef uint32_t (*fpBlockingt)     (void *pHandle,
                                     uint8_t *ui8TxBuffer,
                                     uint32_t ui32Address,
                                     uint32_t ui32NumBytes ) ;

//
//! typedef for device init function
//
typedef uint32_t  (*pfDevInit) (uint32_t ui32Module,
                                   am_devices_spipsram_config_t *pDevConfig,
                                   void **ppHandle,
                                   void **ppIomHandle);


//
//! array of pointers for device setup and I/O
//
typedef struct
{
    //
    //! device init function
    //
    pfDevInit    fpDeviceInit ;

    //
    //! device shutdown function
    //
    uint32_t (*fpDeviceTerminate)(void *pHandle);

    //
    //! read device ID function
    //
    uint32_t (*fpReadDevId)(void *pHandle, uint32_t *pDeviceID);

    //
    //! nonblocking write function
    //
    fpNonBlockingt fpNonBlockingWrite ;
    //
    //! nonblocking read function
    //
    fpNonBlockingt fpNonBlockingRead ;
    //
    //! blocking write function
    //
    fpBlockingt    fpBlockingWrite ;
    //
    //! blocking read function
    //
    fpBlockingt    fpBlockingRead ;
    //
    //! device name string
    //
    uint8_t        devName[20];
}
psram_device_func_t;


#define DMATCBBufferBUFF_SIZE 4096

typedef struct
{
    //
    //! handle for device driver
    //
    void                                    *pIomDevHdl;
    //
    //! handle for iom driver
    //
    void                                    *pIOMHandle;
    //
    //! read failure count
    //
    uint32_t                                readCompleteError ;
    //
    //! number of times processor was put to sleep
    //
    uint32_t                                ui32SleepCount ;
    //
    //! current operating state for the widget
    //! not used
    //
    psram_cb_params_t                       PsramCBData ;
    //
    //! size in bytes of data moved between device and processor
    //
    uint32_t                                ui32BuffSize;
    //
    //! iom number for device used
    //
    uint32_t                                ui32IOMNum;
    //
    //! chip-select number for device
    //
    uint32_t                                ui32ChipSelectNum;
    //
    //! spi frequency for device
    //
    uint32_t                                ui32SpiClockFreq;
    //
    //! command buffer size in bytes
    //
    uint32_t                                ui32BufferSizeBytes ;
    //
    //! callback function used for read or write function
    //
    am_hal_iom_callback_t pCallbackFunction;
    //
    //! set when read or write is complete (read is complete)
    //
    volatile bool         bReadWriteComplete;
    //
    //! is set when read or write operation is ongoing
    //
    volatile bool         readPsRamInProgress;
    //
    //! This is unused, used in generic callback
    //
    volatile bool         psramCBComplete;
    //
    //! keep data 32 bit aligned
    //
    uint8_t                                   res[1] ; // data alignment
    //
    //! pointer to array of I/O and init functions
    //
    const psram_device_func_t                 *pdeviceFP ;
    //
    //! copy of data written to device, will be used to verify
    //! read data
    //
    am_hal_iom_buffer(PSRAM_PATTERN_BUF_SIZE) PatternBuf;
    //
    //! memory location for read data
    //
    am_hal_iom_buffer(PSRAM_PATTERN_BUF_SIZE) RxBuf;
    //
    //! used in IOM driver
    //
    uint32_t                                  DMATCBBuffer[DMATCBBufferBUFF_SIZE];
}
iom_psram_vars_t;


//
//! global variables used in this module
//
typedef struct
{
    //
    //! modified in timer isr, alerts background that interrupt happened
    //
    volatile uint32_t ctimer_tick;
    volatile uint32_t ctimer_tickCount ;


} am_widget_iom_multdrop_vars;



//*****************************************************************************
//
//! @brief psram isr callback
//!
//! @param pCallbackCtxt        - argument passed to callback
//! @param transactionStatus    - transaction status
//
//*****************************************************************************
void psram_isr_cb(void *pCallbackCtxt, uint32_t transactionStatus );
//*****************************************************************************
//
//! @brief psram read complete callback, called from ISR
//!
//! @param pCallbackCtxt        - argument passed to callback
//! @param transactionStatus    - transaction status
//
//*****************************************************************************
void am_wdg_iom_multidrop_psram_read_complete(void *pCallbackCtxt, 
                                              uint32_t transactionStatus);
//*****************************************************************************
//
//! @brief setup psram data pattern in global ram buffer
//
//*****************************************************************************


void am_wdg_iom_multidrop_init_pattern(uint8_t *pui8Buff, uint32_t ui32BuffSize ) ;

//*****************************************************************************
//
//! @brief psram init  inits the psram and IOM
//!
//! @param p_psRamVars     - pointer to pram vars stuct
//! @param pOtherHandle      - pointer to handle for existing IOM device
//! @param ui32SpiClockFreq  - spi freq for this device
//! @param ui32IomModuleNum  - iom module for this device
//! @param ui32ChipSelectNum - chipselect for this device
//! @return status           - generic device status
//
//*****************************************************************************
int32_t am_wdg_iom_multidrop_psram_init(iom_psram_vars_t *p_psRamVars, void *pOtherHandle);
//*****************************************************************************
//! @return 0 success, -1 for error
//
//
//! @brief verify psram data
//!
//! @note ensure psram data maches the expected pattern
//!
//! @return 0 success, -1 for error
//
//*****************************************************************************
int32_t am_wdg_iom_multidrop_verify_data(uint8_t *pui8RefBuff,
                                         uint8_t *pui8DataBuff,
                                         uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief psram write
//!
//! @param ui32NumBytes
//! @param nonBlocking
//!
//! @return
//!
//*****************************************************************************
uint32_t
am_wdg_iom_multidrop_psram_write(iom_psram_vars_t *piomPsramVars,
                                 uint8_t *pui8BuffPtr,
                                 uint32_t ui32NumBytes,
                                 bool nonBlocking);
//*****************************************************************************
//
//! @brief psram read
//!
//! @param ui32NumBytes
//! @param nonBlocking
//!
//! @return
//!
//*****************************************************************************
uint32_t
am_wdg_iom_multidrop_psram_read(iom_psram_vars_t *piomPsramVars,
                                uint8_t *pui8BuffPtr,
                                uint32_t numToRead,
                                bool nonBlocking);

//*****************************************************************************
//
//! @brief deinit psram
//!
//! @return  generic status
//
//*****************************************************************************
uint32_t am_wdg_iom_multidrop_psram_term(iom_psram_vars_t *pPsramTestVars) ;

#endif //AM_WIDGET_IOM_MULITDROP_H
