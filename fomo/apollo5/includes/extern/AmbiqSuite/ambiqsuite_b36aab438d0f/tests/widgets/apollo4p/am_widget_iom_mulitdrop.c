//*****************************************************************************
//
//! @file am_widget_iom_multidrop.c
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

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_widget_iom_mulitdrop.h"
#include "am_devices_spipsram.h"
#include <string.h>

//
//! allocate global to hold isr variables
//
am_widget_iom_multdrop_vars g_WidgetMultiDropv ;


#if defined APS6404L
#define PSRAM_DEVICE_ID          AM_DEVICES_SPIPSRAM_KGD_PASS
//#define am_iom_test_devices_t    am_devices_spipsram_config_t
#else
#error "Unknown PSRAM Device"
#endif

#define IOM_INTERRUPT1(n)       AM_HAL_INTERRUPT_IOMASTER ## n
#define IOM_INTERRUPT(n)        IOM_INTERRUPT1(n)
#define PSRAM_IOM_IRQn          ((IRQn_Type)(IOMSTR0_IRQn + PSRAM_IOM_MODULE))
#define IOM_TEST_FREQ           AM_HAL_IOM_16MHZ




//
// Buffer for non-blocking transactions
//

//*****************************************************************************
//
//! @brief Take over default IOM channel ISR.
//!
//! Timer Interrupt Service Routine (ISR)
//
//*****************************************************************************
void
am_ctimer_isr(void)
{
    //
    // Clear TimerA0 Interrupt (write to clear).
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
    am_hal_timer_clear(0);
    g_WidgetMultiDropv.ctimer_tick++ ;
    g_WidgetMultiDropv.ctimer_tickCount++ ;

} // am_ctimer_isr()

//
// Take over the interrupt handler for whichever IOM we're using.
//

//*****************************************************************************
//
// psram isr callback
//
//*****************************************************************************
void
psram_isr_cb(void *pCallbackCtxt, uint32_t transactionStatus )
{
    iom_psram_vars_t *pIomPsramv = (iom_psram_vars_t *) pCallbackCtxt ;
    //
    // psram non-blocking io complete
    //
    pIomPsramv->psramCBComplete = true ;
} // psram_isr_cb

//*****************************************************************************
//
// psram read complete callback, called from ISR
//
//*****************************************************************************
void
am_wdg_iom_multidrop_psram_read_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{

    iom_psram_vars_t *pIomPsramv = (iom_psram_vars_t *) pCallbackCtxt ;
    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nPSRAM Read Failed 0x%x\n", transactionStatus);
        pIomPsramv->readCompleteError++;
    }
    else
    {
        //am_util_stdio_printf(".");
        pIomPsramv->bReadWriteComplete = true;
    }
    pIomPsramv->readPsRamInProgress = false;
} // psram_read_complete

//*****************************************************************************
//
// setup psram data pattern in global ram buffer
//
//*****************************************************************************
void
am_wdg_iom_multidrop_init_pattern(uint8_t *pui8Buff, uint32_t ui32BuffSize )
{
    for (uint32_t i = 0; i < ui32BuffSize; i++)
    {
        *pui8Buff++ = (uint8_t ) i ;
    }
} // am_wdg_iom_multidrop_init_pattern

//*****************************************************************************
//
//! @brief psram init
//!
//! @return 0 success, -1 for error
//
//*****************************************************************************
int32_t
am_wdg_iom_multidrop_psram_init(iom_psram_vars_t *p_psRamVars, void *pOtherHandle)
{
    uint32_t  ui32Status ;

    am_devices_spipsram_config_t stPsramConfig;

    stPsramConfig.ui32ClockFreq      = p_psRamVars->ui32SpiClockFreq;
    stPsramConfig.pNBTxnBuf          = p_psRamVars->DMATCBBuffer;
    stPsramConfig.ui32NBTxnBufLength = p_psRamVars->ui32BufferSizeBytes / 4;
    stPsramConfig.ui32ChipSelectNum  = p_psRamVars->ui32ChipSelectNum;

    //
    // note, these are on the same IOM chanel, so they need the same handle
    // at least in this implementation
    //
    p_psRamVars->pIOMHandle = pOtherHandle ;

    ui32Status = p_psRamVars->pdeviceFP->fpDeviceInit(p_psRamVars->ui32IOMNum,
                                          &stPsramConfig,
                                          &p_psRamVars->pIomDevHdl,
                                          &p_psRamVars->pIOMHandle);
    if (ui32Status)
    {
        am_util_stdio_printf("Device init error PSRAM %ld\n", ui32Status);
        return -1;
    }

    am_util_stdio_printf("%s Found\n", p_psRamVars->pdeviceFP->devName);
    //
    // Set up a pattern data in PSRAM memory
    //
    am_util_stdio_printf("Setting up data pattern in PSRAM using nonblocking write\n");
    p_psRamVars->PsramCBData.ePsramState = ePsramInit ;

    am_hal_iom_callback_t pfcb = p_psRamVars->pCallbackFunction ;
    if ( pfcb == NULL )
    {
        pfcb =   psram_isr_cb ;
    }
    return (int32_t) p_psRamVars->pdeviceFP->fpNonBlockingWrite(p_psRamVars->pIomDevHdl,
                                                    &p_psRamVars->PatternBuf.bytes[0],
                                                    0,
                                                    p_psRamVars->ui32BuffSize,
                                                    pfcb,
                                                    p_psRamVars);
} // am_wdg_iom_multidrop_psram_init


//*****************************************************************************
//
//verify psram data
//
//*****************************************************************************
int32_t
am_wdg_iom_multidrop_verify_data(uint8_t *pui8RefBuff, uint8_t *pui8DataBuff, uint32_t ui32NumBytes)
{
    // 
    // Verify Read PSRAM data
	//
    for (uint32_t i = 0; i < ui32NumBytes; i++)
    {
        if (pui8DataBuff[i] != pui8RefBuff[i])
        {
            am_util_stdio_printf("Receive Data Compare failed at offset %d - Expected = 0x%x, Received = 0x%x\n",
                                 i, pui8DataBuff[i], pui8RefBuff[i]);
            return -1;
        }
    }
    return 0;
} // psram_verify_data
//*****************************************************************************
//
//psram write
//!
//*****************************************************************************
uint32_t
am_wdg_iom_multidrop_psram_write( iom_psram_vars_t *piomPsramVars,
             uint8_t *pui8BuffPtr,
             uint32_t ui32NumBytes,
             bool nonBlocking)
{
    uint32_t ui32Status;

    if (nonBlocking)
    {
        ui32Status = piomPsramVars->pdeviceFP->fpNonBlockingWrite(piomPsramVars->pIomDevHdl,
                                                         pui8BuffPtr,
                                                         0,
                                                         ui32NumBytes,
                                                         am_wdg_iom_multidrop_psram_read_complete,
                                                         piomPsramVars);

    }
    else
    {

        ui32Status = piomPsramVars->pdeviceFP->fpBlockingWrite(piomPsramVars->pIomDevHdl,
                                                        pui8BuffPtr,
                                                        0,
                                                        ui32NumBytes );
    }

    return ui32Status ;
} // psram_write

//*****************************************************************************
//
//! psram read
//!
//*****************************************************************************
uint32_t
am_wdg_iom_multidrop_psram_read(iom_psram_vars_t *piomPsramVars,
           uint8_t *pui8BuffPtr,
           uint32_t numToRead,
           bool nonBlocking)
{
    uint32_t ui32Status;

    //
    // Initiate read of a block of data from PSRAM
    //
    piomPsramVars->PsramCBData.ePsramState = ePsramRead;

    memset( pui8BuffPtr, 0, numToRead ) ;

    if (nonBlocking)
    {
        ui32Status = piomPsramVars->pdeviceFP->fpNonBlockingRead(piomPsramVars->pIomDevHdl,
                                                        pui8BuffPtr,
                                                        0,
                                                        numToRead,
                                                        am_wdg_iom_multidrop_psram_read_complete,
                                                        piomPsramVars);

        if (ui32Status == 0)
        {
            //
            // this will get cleared in a callback when the I/O finishes
            //
            piomPsramVars->readPsRamInProgress  = true ;
        }
    }
    else
    {

        ui32Status = piomPsramVars->pdeviceFP->fpBlockingRead(piomPsramVars->pIomDevHdl,
                                                       pui8BuffPtr,
                                                       0,
                                                       numToRead );
    }

    return ui32Status;
} // psram_read

//*****************************************************************************
//
//! @brief deinit psram
//!
//! @return  generic status
//
//*****************************************************************************
uint32_t
am_wdg_iom_multidrop_psram_term(iom_psram_vars_t *pPsramTestVars)
{
    return pPsramTestVars->pdeviceFP->fpDeviceTerminate(pPsramTestVars->pIomDevHdl);
} // fpDeviceTerminate
