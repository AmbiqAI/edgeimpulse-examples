//*****************************************************************************
//
//! @file am_widget_iom.c
//!
//! @brief Test widget for testing IOM channels by data transfer using fram.
//
//! This widget performs a write followed by read operations to an external
//! fram after configuring the IOM using specified configuration.
//! Success or failure is detected based on buffer matching.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include <ctype.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_widget_iom.h"

#ifdef AM_PART_APOLLO3P
#define SRAM_SIZE       (768 * 1024)        // Apollo3P SRAM size
#else
#define SRAM_SIZE       (384 * 1024)        // Apollo3 SRAM size
#endif

#define AM_WIDGET_IOM_SPI_DEVICE 1
#define AM_WIDGET_IOM_I2C_DEVICE 2

//
// Typedef - to encapsulate device driver functions
//
typedef struct
{
    uint8_t  devName[20];
    uint32_t (*iom_device_init)(uint32_t ui32Module, am_widget_iom_devices_t *pDevConfig, void **ppHandle, void **ppIomHandle);
    uint32_t (*iom_device_term)(void *pHandle);

    uint32_t (*iom_device_read_id)(void *pHandle, uint32_t *pDeviceID);

    uint32_t (*iom_device_blocking_write)(void *pHandle, uint8_t *ui8TxBuffer,
                             uint32_t ui32WriteAddress,
                             uint32_t ui32NumBytes);

    uint32_t (*iom_device_nonblocking_write)(void *pHandle, uint8_t *ui8TxBuffer,
                                uint32_t ui32WriteAddress,
                                uint32_t ui32NumBytes,
                                am_hal_iom_callback_t pfnCallback,
                                void *pCallbackCtxt);

    uint32_t (*iom_device_blocking_read)(void *pHandle, uint8_t *pui8RxBuffer,
                            uint32_t ui32ReadAddress,
                            uint32_t ui32NumBytes);

    uint32_t (*iom_device_nonblocking_read)(void *pHandle, uint8_t *pui8RxBuffer,
                                                      uint32_t ui32ReadAddress,
                                                      uint32_t ui32NumBytes,
                                                      am_hal_iom_callback_t pfnCallback,
                                                      void *pCallbackCtxt);
} am_widget_iom_device_func_t;

am_widget_iom_device_func_t iom_device_func =
{
#if MB85RS1MT
    // Fireball installed SPI FRAM device
    .devName = "SPI FRAM MB85RS1MT",
    .iom_device_init = am_devices_mb85rs1mt_init,
    .iom_device_term = am_devices_mb85rs1mt_term,
    .iom_device_read_id = am_devices_mb85rs1mt_read_id,
    .iom_device_blocking_write = am_devices_mb85rs1mt_blocking_write,
    .iom_device_nonblocking_write = am_devices_mb85rs1mt_nonblocking_write,
    .iom_device_blocking_read = am_devices_mb85rs1mt_blocking_read,
    .iom_device_nonblocking_read = am_devices_mb85rs1mt_nonblocking_read,
#elif MB85RQ4ML
    // Fireball installed SPI FRAM device
    .devName = "SPI FRAM MB85RQ4ML",
    .iom_device_init = am_devices_mb85rq4ml_init,
    .iom_device_term = am_devices_mb85rq4ml_term,
    .iom_device_read_id = am_devices_mb85rq4ml_read_id,
    .iom_device_blocking_write = am_devices_mb85rq4ml_blocking_write,
    .iom_device_nonblocking_write = am_devices_mb85rq4ml_nonblocking_write,
    .iom_device_blocking_read = am_devices_mb85rq4ml_blocking_read,
    .iom_device_nonblocking_read = am_devices_mb85rq4ml_nonblocking_read,
#elif MB85RS64V
    .devName = "SPI FRAM MB85RS64V",
    .iom_device_init = am_devices_mb85rs64v_init,
    .iom_device_term = am_devices_mb85rs64v_term,
    .iom_device_read_id = am_devices_mb85rs64v_read_id,
    .iom_device_blocking_write = am_devices_mb85rs64v_blocking_write,
    .iom_device_nonblocking_write = am_devices_mb85rs64v_nonblocking_write,
    .iom_device_blocking_read = am_devices_mb85rs64v_blocking_read,
    .iom_device_nonblocking_read = am_devices_mb85rs64v_nonblocking_read,
#elif MB85RC256V
    .devName = "I2C FRAM MB85RC256V",
    .iom_device_init = am_devices_mb85rc256v_init,
    .iom_device_term = am_devices_mb85rc256v_term,
    .iom_device_read_id = am_devices_mb85rc256v_read_id,
    .iom_device_blocking_write = am_devices_mb85rc256v_blocking_write,
    .iom_device_nonblocking_write = am_devices_mb85rc256v_nonblocking_write,
    .iom_device_blocking_read = am_devices_mb85rc256v_blocking_read,
    .iom_device_nonblocking_read = am_devices_mb85rc256v_nonblocking_read,
#elif MB85RC64TA
    .devName = "I2C FRAM MB85RC64TA",
    .iom_device_init = am_devices_mb85rc256v_init,
    .iom_device_term = am_devices_mb85rc256v_term,
    .iom_device_read_id = am_devices_mb85rc256v_read_id,
    .iom_device_blocking_write = am_devices_mb85rc256v_blocking_write,
    .iom_device_nonblocking_write = am_devices_mb85rc256v_nonblocking_write,
    .iom_device_blocking_read = am_devices_mb85rc256v_blocking_read,
    .iom_device_nonblocking_read = am_devices_mb85rc256v_nonblocking_read,
#elif APS6404L
    .devName = "SPI PSRAM APS6404L",
    .iom_device_init = am_devices_spipsram_init,
    .iom_device_term = am_devices_spipsram_term,
    .iom_device_read_id = am_devices_spipsram_read_id,
    .iom_device_blocking_write = am_devices_spipsram_blocking_write,
    .iom_device_nonblocking_write = am_devices_spipsram_nonblocking_write,
    .iom_device_blocking_read = am_devices_spipsram_blocking_read,
    .iom_device_nonblocking_read = am_devices_spipsram_nonblocking_read,
#elif EM9304
    .devName = "EM9304",
    .iom_device_init = am_devices_em9304_init,
    .iom_device_term = am_devices_em9304_term,
#elif LOOP
    .devName = "LOOP",
    .iom_device_init = am_devices_loop_init,
    .iom_device_term = am_devices_loop_term,
#else
// No devices
#endif
};

//*****************************************************************************
//
// Widget Config parameters
//
//*****************************************************************************
#define AM_TEST_RANDOMIZE

#define AM_TEST_TIMEOUT     10 // 10 Seconds

//! Number of buffer compare errors tolerated before failing.
#define COMPARE_TOLERANCE   (0)
#define XOR_BYTE            (0x0)

//! End of Buffer Tolerance
#define EOB_TOLERANCE       (0)
#define ERROUT_CHAR         (0)

//! Number of entries in IOM Queue.
//! We queue max 10 transactions - 2 Block writes, each taking 3,
//! and 2 block reads, each taking 2
#define IOM_QUEUESIZE       (10)

#define MAX_TRANS_SIZE      (4096 - 4)

// Status flags.
//! Input buffer set to this value when empty.
#define INBUFFER_EMPTY      (0xEE)
//! Output buffer set to this value when empty.
#define OUTBUFFER_EMPTY     (0xED)
//! Value for one byte write/read test (188).
#define WRITE_READ_BYTE     (0xBC)


//*****************************************************************************
//
// Callback functions
//
//*****************************************************************************
uint32_t g_bIOMNonBlockCompleteWr, g_bIOMNonBlockCompleteRd;

void pfnWidget_IOM_Callback_Write(void *pCtxt, uint32_t ui32Status)
{
    //
    // Set the DMA complete flag.
    //
    g_bIOMNonBlockCompleteWr = (ui32Status ? 2 : 1);
}

void pfnWidget_IOM_Callback_Read(void *pCtxt, uint32_t ui32Status)
{
    //
    // Set the DMA complete flag.
    //
    g_bIOMNonBlockCompleteRd = (ui32Status ? 2 : 1);
}

//*****************************************************************************
//
// Internal Data Structures
//
//*****************************************************************************
typedef struct
{
    uint32_t              ui32Module;
    am_hal_iom_config_t   IOMConfig;
    am_hal_iom_mode_e     mode;
    void                  *pHandle;
} am_widget_iom_t;

//*****************************************************************************
//
// Memory for handling IOM transactions.
//
//*****************************************************************************
// Allow 1 extra word for offset testing
#if 0
static uint8_t g_pucInBuffer[(MAX_TRANS_SIZE+4)*2];
static uint8_t g_pucOutBuffer[(MAX_TRANS_SIZE+4)*2];
#else
#if defined(AM_PART_APOLLO4)
AM_SHARED_RW uint8_t g_pucInBuffer[(MAX_TRANS_SIZE+4)*2];
#else
uint8_t g_pucInBuffer[(MAX_TRANS_SIZE+4)*2];
#endif
#if defined(AM_PART_APOLLO4)
AM_SHARED_RW uint8_t g_pucOutBuffer[(MAX_TRANS_SIZE+4)*2];
#else
uint8_t g_pucOutBuffer[(MAX_TRANS_SIZE+4)*2];
#endif
#endif

//*****************************************************************************
//
// Global Variables.
//
//*****************************************************************************
static am_widget_iom_config_t g_IomSpiWidget;
static volatile uint32_t g_IomIsrErr = 0;
static volatile uint32_t g_bComplete = 0;

void            *g_IomDevHdl;
void            *g_pIOMHandle;
volatile uint32_t g_IOMInterruptStatus = 0;
//uint32_t        g_DMACtrlBuffer[128];

#if APS6404L
const struct
{
    const uint32_t MHz;
    const uint32_t MaxSize;
} g_IOM_SpeedMax[] =
{
    {AM_HAL_IOM_48MHZ, AM_DEVICES_SPIPSRAM_48MHZ_MAX_BYTES},
    {AM_HAL_IOM_24MHZ, AM_DEVICES_SPIPSRAM_24MHZ_MAX_BYTES},
    {AM_HAL_IOM_16MHZ, AM_DEVICES_SPIPSRAM_16MHZ_MAX_BYTES},
    {AM_HAL_IOM_12MHZ, AM_DEVICES_SPIPSRAM_12MHZ_MAX_BYTES},
    {AM_HAL_IOM_8MHZ,  AM_DEVICES_SPIPSRAM_8MHZ_MAX_BYTES}  // Leave this in for PSRAM initialization at 8MHz.
};
#endif

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
//
//! Take over default ISR for IOM 0. (Queue mode service)
//
void am_iom_isr(void)
{
    uint32_t ui32Status;

    am_hal_iom_interrupt_status_get(g_pIOMHandle, true, &ui32Status);

    if ( ui32Status )
    {
        am_hal_iom_interrupt_clear(g_pIOMHandle, ui32Status);

        if ( g_IOMInterruptStatus == 0 )
        {
            am_hal_iom_interrupt_service(g_pIOMHandle, ui32Status);
        }
        g_IOMInterruptStatus &= ~ui32Status;
    }
    else
    {
        am_util_stdio_printf("IOM INTSTAT 0\n");
    }
}

void
am_iomaster0_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 1. (Queue mode service)
//
void
am_iomaster1_isr(void)
{
    am_iom_isr();
}

#ifndef AM_PART_APOLLO
//
//! Take over default ISR for IOM 1. (Queue mode service)
//
void
am_iomaster2_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 3. (Queue mode service)
//
void
am_iomaster3_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 4. (Queue mode service)
//
void
am_iomaster4_isr(void)
{
    am_iom_isr();
}

//
//! Take over default ISR for IOM 5. (Queue mode service)
//
void
am_iomaster5_isr(void)
{
    am_iom_isr();
}
#if AM_PART_APOLLO4
//
//! Take over default ISR for IOM 6. (Queue mode service)
//
void
am_iomaster6_isr(void)
{
    am_iom_isr();
}
//
//! Take over default ISR for IOM 7. (Queue mode service)
//
void
am_iomaster7_isr(void)
{
    am_iom_isr();
}
#endif
#endif

void
wait_for_nonblock_response(am_hal_sysctrl_power_state_e sleepMode, volatile uint32_t *pAddr)
{
    if (sleepMode == AM_HAL_SYSCTRL_WAKE)
    {
        while (!*pAddr);
    }
    else
    {
        //
        // Loop forever.
        //
        while(1)
        {
            //
            // Disable interrupt while we decide whether we're going to sleep.
            //
            uint32_t ui32IntStatus = am_hal_interrupt_master_disable();

            if (!*pAddr)
            {
                if (sleepMode == AM_HAL_SYSCTRL_DEEPSLEEP)
                {
                    // Disable ITM
                    am_bsp_debug_printf_disable();
                }
                am_hal_sysctrl_sleep(sleepMode);
                if (sleepMode == AM_HAL_SYSCTRL_DEEPSLEEP)
                {
                    // Re-enable ITM
                    am_bsp_debug_printf_enable();
                }
                //
                // Enable interrupts
                //
                am_hal_interrupt_master_set(ui32IntStatus);
            }
            else
            {
                //
                // Enable interrupts
                //
                am_hal_interrupt_master_set(ui32IntStatus);
                break;
            }
        }
    }
}


//*****************************************************************************
//
// Pseudo random number generator.
//
//*****************************************************************************
#define DEFAULT_VL_M_W 0x12345678
#define DEFAULT_VL_M_Z 0x90abcdef
static          unsigned vl_m_w = DEFAULT_VL_M_W;   // Default seeds
static          unsigned vl_m_z = DEFAULT_VL_M_Z;   //  "
static volatile unsigned prng_seeded = 0;

void vl_SetSeed(unsigned newseed)
{
    if ( newseed != 0 )
    {
        vl_m_w = newseed;
        vl_m_z = (newseed ^ 0xFFFFFFFF) ^ DEFAULT_VL_M_Z;
        prng_seeded = 1;
    }
} // vl_SetSeed()

unsigned vl_RandVal(void)
{
    //
    //  These functions adapted from code found at the following URL on 5/25/14:
    //  http://www.codeproject.com/Articles/25172/Simple-Random-Number-Generation
    //
    vl_m_z = 36969 * (vl_m_z & 65535) + (vl_m_z >> 16);
    vl_m_w = 18000 * (vl_m_w & 65535) + (vl_m_w >> 16);
    return (vl_m_z << 16) + vl_m_w;
} // vl_RandVal()

unsigned vl_RandValRange(unsigned uMin, unsigned uMax)
{
    unsigned uDelta, uVal;

    // Returns a random value between min and max (inclusive).
    uDelta = uMax - uMin;
    if (uDelta == 0)
    {
        if (uMin == 0)
        {
            return vl_RandVal();    // Return a 32-bit random value
        }
        else
        {
            return uMin;            // Return the given value
        }
    }

    //
    // To get a random value within the given range, we'll simply take a random
    //  value and scale it into the range we need.
    //
    uVal = vl_RandVal() % (uDelta + 1);

    return uVal + uMin;
} // vl_RandValRange()


//*****************************************************************************
//
//! Empty the buffers.
//! Load output buffer with repeated text string.
//! Text string without 0 termination.
//
//*****************************************************************************
static int load_buffers(int offset, int num_addr, int repeatCount)
{
    int ix = 0;     // Returns number of bytes loaded
    int iy = 0;
    uint32_t ui32Rand;
    // First initialize random pattern in whole buffer - out and in same
    for ( ix = 0; ix < sizeof(g_pucOutBuffer); ix++ )
    {
        if ( ((ix & 0x3) == 0) )
        {
            ui32Rand = vl_RandValRange(0,0);
        }

        //
        // Store 1 byte to the output buffer, 1 to the input buffer.
        //
        g_pucOutBuffer[ix] = g_pucInBuffer[ix] = ui32Rand & 0xff;
        ui32Rand >>= 8;
    }
    // Initialize the buffer section which will be used - to initialize out and in different
    for ( ix = offset; ix < num_addr + offset; ix++ )
    {
#ifdef AM_TEST_RANDOMIZE
        if ( ((ix & 0x1) == 0) )
        {
            ui32Rand = vl_RandValRange(0,0);
        }

        for (iy = 0; iy < (repeatCount + 1); iy++)
        {
            //
            // Store 1 byte to the output buffer, 1 to the input buffer.
            //
            g_pucOutBuffer[ix + iy*num_addr] = ui32Rand & 0xff;
            g_pucInBuffer[ix + iy*num_addr]  = (ui32Rand >> 8) & 0xff;
        }
        ui32Rand >>= 16;
#else
        g_pucOutBuffer[ix] = (ix - offset) & 0xff;
        g_pucInBuffer[ix]  = ~g_pucOutBuffer[ix];
#endif
    }

    return ix;
}

//! Compare input and output buffers.
//! This checks that the received data matches transmitted data.
//! It also checks that any of the buffer memory beyond what is needed is not corrupted
static bool compare_buffers(uint32_t offset, int num_addr, int repeatCount)
{
#if 0
    for (uint32_t i = offset; i < offset + num_addr; i++)
    {
        for (uint32_t j = 0; j < (repeatCount + 1); j++)
        {
            if (g_pucInBuffer[i + j*num_addr] != g_pucOutBuffer[i + j*num_addr])
            {
                am_util_stdio_printf("Buffer miscompare at location %d\n", i + j*num_addr);
                am_util_stdio_printf("TX Value = %2X | RX Value = %2X\n", g_pucOutBuffer[i + j*num_addr], g_pucInBuffer[i + j*num_addr]);
                am_util_stdio_printf("TX: ");
                for (uint32_t k = 0; k < num_addr; k++)
                {
                  am_util_stdio_printf("%2X ", g_pucOutBuffer[offset + k]);
                }
                am_util_stdio_printf("\n");
                am_util_stdio_printf("RX: ");
                for (uint32_t k = 0; k < num_addr; k++)
                {
                  am_util_stdio_printf("%2X ", g_pucInBuffer[offset + k]);
                }
                am_util_stdio_printf("\n");
                return false;
            }
        }
    }
#else
    // Compare the output buffer with input buffer
    for (uint32_t i = 0; i < sizeof(g_pucOutBuffer); i++)
    {
        if (g_pucInBuffer[i] != g_pucOutBuffer[i])
        {
            am_util_stdio_printf("Buffer miscompare at location %d\n", i);
            am_util_stdio_printf("TX Value = %2X | RX Value = %2X\n", g_pucOutBuffer[i], g_pucInBuffer[i]);
#if 1 // Detailed logging
            am_util_stdio_printf("TX:\n");
            for (uint32_t k = 0; k < num_addr; k++)
            {
              am_util_stdio_printf("%2X ", g_pucOutBuffer[offset + k]);
              if ((k%64) == 0)
              {
                am_util_stdio_printf("\n");
              }
            }
            am_util_stdio_printf("\n");
            am_util_stdio_printf("RX:\n");
            for (uint32_t k = 0; k < num_addr; k++)
            {
              am_util_stdio_printf("%2X ", g_pucInBuffer[offset + k]);
              if ((k%64) == 0)
              {
                am_util_stdio_printf("\n");
              }
            }
            am_util_stdio_printf("\n");
#endif
            return false;
        }
    }
#endif
    return true;
}

//*****************************************************************************
//
// Widget setup function.
//
//*****************************************************************************
uint32_t am_widget_iom_test_setup(am_widget_iom_config_t *pTestCfg,
                                  void **ppWidget, char *pErrStr)
{
    uint32_t ui32Status = 0;
#if defined(AM_PART_APOLLO4)
    am_hal_pwrctrl_sram_memcfg_t  SRAMCfg =
    {
      .eSRAMCfg       = AM_HAL_PWRCTRL_SRAM_1M,
      .eActiveWithMCU = AM_HAL_PWRCTRL_SRAM_1M,
      .eActiveWithDSP = AM_HAL_PWRCTRL_SRAM_NONE,
      .eSRAMRetain    = AM_HAL_PWRCTRL_SRAM_1M
    };
#endif    

    g_IomSpiWidget.IOMConfig      = pTestCfg->IOMConfig;
    g_IomSpiWidget.ui32Module     = pTestCfg->ui32Module;
    g_IomSpiWidget.ui32DeviceType = pTestCfg->ui32DeviceType;
    *ppWidget = &g_IomSpiWidget;

#if defined(AM_PART_APOLLO4)
    ui32Status = am_hal_pwrctrl_sram_config(&SRAMCfg);
#endif
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
      return false;
    }
    
    ui32Status = iom_device_func.iom_device_init(pTestCfg->ui32Module, &pTestCfg->IOMConfig, &g_IomDevHdl, &g_pIOMHandle);
    ui32Status = (ui32Status == IOM_WIDGET_DEVICE_SUCCESS) ? AM_WIDGET_SUCCESS : AM_WIDGET_ERROR;

    return ui32Status;
}

uint32_t am_widget_iom_test_cleanup(void *pWidget, char *pErrStr)
{
    uint32_t ui32Status = 0;
    //am_widget_iom_config_t        *pTestCfg = (am_widget_iom_config_t *)pWidget;

    iom_device_func.iom_device_term(g_IomDevHdl);

    //
    // Return status.
    //
    return ui32Status;
}

uint32_t
am_widget_iom_power_saverestore(void *pWidget, am_hal_sysctrl_power_state_e sleepMode)
{
    uint32_t ui32Status = am_hal_iom_power_ctrl(g_pIOMHandle, sleepMode, true);
    if (AM_HAL_STATUS_SUCCESS == ui32Status)
    {
        return AM_WIDGET_SUCCESS;
    }
    else
    {
        return AM_WIDGET_ERROR;
    }
}

uint32_t
am_widget_iom_test_read_id(void *pWidget, char *pErrStr)
{
#if !defined(EM9304) && !defined(LOOP)
    //am_widget_iom_config_t        *pTestCfg = (am_widget_iom_config_t *)pWidget;
    uint32_t                      ui32DeviceId = 0;
#if APS6404L
    uint32_t ui32Status;
    uint32_t aui32Rawdata[5] = {0,0,0,0,0};
    ui32Status = iom_device_func.iom_device_read_id(g_IomDevHdl, aui32Rawdata);
    ui32DeviceId = ((aui32Rawdata[0] & 0xFF000000) >> 24) | ((aui32Rawdata[1] & 0xFF) << 8);
#if defined(AM_PART_APOLLO4)
    return (( (ui32Status != AM_DEVICES_SPIPSRAM_STATUS_SUCCESS) || ( ui32DeviceId != AM_DEVICES_SPIPSRAM_KGD_PASS )) ? AM_WIDGET_ERROR: AM_WIDGET_SUCCESS);
#else
    return ((ui32Status || ( ui32DeviceId != AM_DEVICES_SPIPSRAM_KGD_PASS )) ? AM_WIDGET_ERROR: AM_WIDGET_SUCCESS);
#endif
#else
    return ((iom_device_func.iom_device_read_id(g_IomDevHdl, &ui32DeviceId)|| ( ui32DeviceId != IOM_WIDGET_DEVICE_ID )) ? AM_WIDGET_ERROR: AM_WIDGET_SUCCESS);
#endif
#else
   return AM_WIDGET_SUCCESS;
#endif
}

uint32_t
am_widget_iom_test_blocking_write_read(void *pWidget, void *pCfg, char *pErrStr)
{
    //am_widget_iom_config_t        *pWidgetCfg = (am_widget_iom_config_t *)pWidget;
    am_widget_iom_test_t          *pTestCfg = (am_widget_iom_test_t *)pCfg;
    uint32_t                      ui32Status;
    uint32_t ui32Offset           = pTestCfg->ui32ByteOffset & 0x3;

    //
    // Create the transmit buffer to write.
    //
    load_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0);

    //
    // Write the buffer to the FRAM device.
    //
    ui32Status = iom_device_func.iom_device_blocking_write(g_IomDevHdl, &g_pucOutBuffer[0] + ui32Offset, 0, pTestCfg->ui32BufferSize);

    if (ui32Status == 0)
    {
        //
        // Read the buffer from the FRAM device.
        //
        ui32Status = iom_device_func.iom_device_blocking_read(g_IomDevHdl, &g_pucInBuffer[0] + ui32Offset, 0, pTestCfg->ui32BufferSize);
    }

    //
    // Compare the receive buffer to the transmit buffer.
    //
    if ( ui32Status || !compare_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0) )
    {
        return AM_WIDGET_ERROR;
    }
    else
    {
        return AM_WIDGET_SUCCESS;
    }
}

uint32_t
am_widget_iom_test_nonblocking_write_read(void *pWidget, void *pCfg, char *pErrStr)
{
    am_widget_iom_test_t   *pTestCfg = (am_widget_iom_test_t *)pCfg;
    uint32_t ui32Offset              = pTestCfg->ui32ByteOffset & 0x3;
    uint32_t ui32Status;

#if APS6404L
    am_widget_iom_config_t *pWidgetCfg = (am_widget_iom_config_t *)pWidget;
    if(pWidgetCfg->IOMConfig.ui32ClockFreq < AM_HAL_IOM_8MHZ)
    {
        return AM_WIDGET_ERROR; // For APS6404L, only meaningful when the frequency is higher than 8MHz
    }

    for(uint8_t i = 0; i < (sizeof(g_IOM_SpeedMax) / sizeof(g_IOM_SpeedMax[0])); i++)
    {
        if(pWidgetCfg->IOMConfig.ui32ClockFreq == g_IOM_SpeedMax[i].MHz)
        {
            uint32_t ui32Limit = 255 * (g_IOM_SpeedMax[i].MaxSize - 3); // AM_HAL_CMDQ_HW_IDX_MAX
            if(pTestCfg->ui32BufferSize > ui32Limit)
            {
                pTestCfg->ui32BufferSize = ui32Limit - (AM_HAL_IOM_MAX_TXNSIZE_SPI - pTestCfg->ui32BufferSize);
            }
        }
    }
#endif

    //
    // Create the transmit buffer to write.
    //
    load_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0);

    //
    // Write the buffer to the FRAM device.
    //
    g_bIOMNonBlockCompleteWr = 0;

    ui32Status = iom_device_func.iom_device_nonblocking_write(g_IomDevHdl, 
                                                              &g_pucOutBuffer[0] + ui32Offset, 
                                                              0, 
                                                              pTestCfg->ui32BufferSize, 
                                                              pfnWidget_IOM_Callback_Write, 
                                                              0);

    if (ui32Status == 0)
    {
        //
        // Wait until the write has completed
        //
        wait_for_nonblock_response(pTestCfg->sleepMode, &g_bIOMNonBlockCompleteWr);


        if (g_bIOMNonBlockCompleteWr != 1)
        {
            return AM_WIDGET_ERROR;
        }

        //
        // Read the buffer from the FRAM device.
        //
        g_bIOMNonBlockCompleteRd = 0;
        ui32Status = iom_device_func.iom_device_nonblocking_read(g_IomDevHdl, &g_pucInBuffer[0] + ui32Offset, 0, pTestCfg->ui32BufferSize, pfnWidget_IOM_Callback_Read, 0);

        if (ui32Status == 0)
        {
            //
            // Wait until the read has completed
            //
            wait_for_nonblock_response(pTestCfg->sleepMode, &g_bIOMNonBlockCompleteRd);

            if (g_bIOMNonBlockCompleteRd != 1)
            {
                return AM_WIDGET_ERROR;
            }
        }
    }

    //
    // Compare the receive buffer to the transmit buffer.
    //
    if (ui32Status || !compare_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0) )
    {
        return AM_WIDGET_ERROR;
    }
    else
    {
        return AM_WIDGET_SUCCESS;
    }
}

uint32_t
am_widget_iom_test_queue_write_read(void *pWidget, void *pCfg, char *pErrStr)
{
    //am_widget_iom_config_t        *pWidgetCfg = (am_widget_iom_config_t *)pWidget;
    am_widget_iom_test_t          *pTestCfg = (am_widget_iom_test_t *)pCfg;
    uint32_t ui32Offset              = pTestCfg->ui32ByteOffset & 0x3;

    // Split into blocks - we need to account for 2 queued entries for
    // each block - one for Tx, one for Rx
    // For SPI case - each Write takes more than one SPI transaction as well
    uint32_t blockSize               = pTestCfg->ui32BufferSize / (pTestCfg->ui32QueueSize / 8) + 1;
    uint32_t ui32Status = 0;

    //
    // Create the transmit buffer to write.
    //
    load_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0);

    uint32_t ui32Bytes = 0;
    while ((0 == ui32Status) && (ui32Bytes < pTestCfg->ui32BufferSize))
    {
        uint32_t size = (blockSize < (pTestCfg->ui32BufferSize - ui32Bytes)) ? blockSize : (pTestCfg->ui32BufferSize - ui32Bytes);
        ui32Status = iom_device_func.iom_device_nonblocking_write(g_IomDevHdl, &g_pucOutBuffer[0] + ui32Offset + ui32Bytes, ui32Bytes, size, 0, 0);
        ui32Bytes += size;
    }

    if (0 == ui32Status)
    {
        //
        // Read the buffer from the FRAM device.
        //
        g_bIOMNonBlockCompleteRd = 0;
        ui32Bytes = 0;
        while ((ui32Status == 0) && (ui32Bytes < pTestCfg->ui32BufferSize))
        {
            uint32_t size = (blockSize < (pTestCfg->ui32BufferSize - ui32Bytes)) ? blockSize : (pTestCfg->ui32BufferSize - ui32Bytes);
            ui32Status = iom_device_func.iom_device_nonblocking_read(g_IomDevHdl, &g_pucInBuffer[0] + ui32Offset + ui32Bytes, ui32Bytes, size,
                                       (size == pTestCfg->ui32BufferSize - ui32Bytes) ? pfnWidget_IOM_Callback_Read : 0, 0);
            ui32Bytes += size;
        }
    }

    if (0 == ui32Status)
    {
        //
        // Wait until the read has completed
        //
        wait_for_nonblock_response(pTestCfg->sleepMode, &g_bIOMNonBlockCompleteRd);

        if (g_bIOMNonBlockCompleteRd != 1)
        {
            return AM_WIDGET_ERROR;
        }
    }

    //
    // Compare the receive buffer to the transmit buffer.
    //
    if (ui32Status || !compare_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0) )
    {
        return AM_WIDGET_ERROR;
    }
    else
    {
        return AM_WIDGET_SUCCESS;
    }
}

#if defined(IOM_SUPPORTS_FULLDUPLEX)
#if 0
uint32_t
am_widget_iom_test_duplex(void *pWidget, void *pCfg, char *pErrStr)
{
    am_widget_iom_test_t          *pTestCfg = (am_widget_iom_test_t *)pCfg;
    uint32_t                      ui32Status;

    //
    // Create the transmit buffer to write.
    //
    load_buffers(6, pTestCfg->ui32BufferSize, 0);

    // TX cmd (0x42): EM_GetMemoryUsage
    g_pucOutBuffer[0] = 0x42;
    g_pucOutBuffer[1] = 0x00;
    g_pucOutBuffer[2] = 0x01;
    g_pucOutBuffer[3] = 0x2C;
    g_pucOutBuffer[4] = 0xFC;
    g_pucOutBuffer[5] = pTestCfg->ui32BufferSize;

    ui32Status = am_devices_em9304_fullduplex(g_IomDevHdl, 0,     // ui32WriteAddress,
                                 g_pucOutBuffer,
                                 g_pucInBuffer,
                                 pTestCfg->ui32BufferSize + 6);


    if (ui32Status != 0)
    {
        return AM_WIDGET_ERROR;
    }
    else
    {
        uint8_t cmpVal = g_pucInBuffer[2];
        for (uint32_t i = 3; i < pTestCfg->ui32BufferSize + 6; i++)
        {
            if (g_pucInBuffer[i] != (cmpVal - 1))
            {
                am_util_stdio_printf("Buffer miscompare at location %d\n", i);
                am_util_stdio_printf("Expected Value = %2X | RX Value = %2X\n", cmpVal - 1, g_pucInBuffer[i]);
                return AM_WIDGET_ERROR;
            }
            cmpVal = g_pucInBuffer[i];
        }
    }
    return AM_WIDGET_SUCCESS;
}
#else
uint32_t
am_widget_iom_test_duplex(void *pWidget, void *pCfg, char *pErrStr)
{
    am_widget_iom_test_t          *pTestCfg = (am_widget_iom_test_t *)pCfg;
    uint32_t                      ui32Status;
    uint32_t ui32Offset           = pTestCfg->ui32ByteOffset & 0x3;
    am_hal_iom_transfer_t         Transaction;
    am_devices_iom_loop_t         *pLoop = (am_devices_iom_loop_t *)g_IomDevHdl;

    //
    // Create the transmit buffer to write.
    //
    load_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0);

    //
    // Write the buffer to the loop device.
    //
    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    Transaction.eDirection      = AM_HAL_IOM_FULLDUPLEX;
    Transaction.ui32InstrLen    = 0;
    Transaction.ui32Instr       = 0;
    Transaction.ui32NumBytes    = pTestCfg->ui32BufferSize;
    Transaction.pui32TxBuffer   = (uint32_t *)(&g_pucOutBuffer[0] + ui32Offset);
    Transaction.pui32RxBuffer   = (uint32_t *)(&g_pucInBuffer[0] + ui32Offset);
    Transaction.uPeerInfo.ui32SpiChipSelect = pLoop->ui32CS;
    Transaction.bContinue       = false;

    //
    // Start the transaction.
    //
    ui32Status = am_hal_iom_spi_blocking_fullduplex(g_pIOMHandle, &Transaction);

    //
    // Compare the receive buffer to the transmit buffer.
    //
    if ( ui32Status || !compare_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0) )
    {
        return AM_WIDGET_ERROR;
    }
    else
    {
        return AM_WIDGET_SUCCESS;
    }
}
#endif
#endif

uint32_t g_ExpErrCount = 0;
uint32_t g_UnExpReturnCount = 0;

void
errCallback(void *pCallbackCtxt, uint32_t transactionStatus)
{
    uint32_t ui32ExpectedError = (uint32_t)pCallbackCtxt;
    if (transactionStatus != ui32ExpectedError)
    {
        am_util_stdio_printf("Call back returned - Expected Value = %2X | Actual Value = %2X\n", ui32ExpectedError, transactionStatus);
        g_UnExpReturnCount++;
    }
    else
    {
        g_ExpErrCount++;
    }
}

uint32_t sendRawTransaction(am_hal_iom_dir_e eDirection,
                          uint32_t devAddr,
                          uint8_t  *pui8Buffer,
                          uint32_t ui32WriteAddress,
                          uint32_t ui32NumBytes,
                          bool     bContinue,
                          bool     bBlocking,
                          am_hal_iom_callback_t pfnCallback,
                          void *pCallbackCtxt)
{
    am_hal_iom_transfer_t         Transaction;

    Transaction.ui8RepeatCount  = 0;
    Transaction.ui32PauseCondition = 0;
    Transaction.ui32StatusSetClr = 0;
    Transaction.ui8Priority     = 1;        // High priority for now.
    Transaction.uPeerInfo.ui32I2CDevAddr = devAddr;
    Transaction.bContinue       = bContinue;

    //
    // Set up the IOM transaction.
    //
    Transaction.eDirection      = eDirection;
    Transaction.ui32InstrLen    = 2;
    Transaction.ui32Instr       = ui32WriteAddress & 0x0000FFFF;
    Transaction.ui32NumBytes    = ui32NumBytes;
    Transaction.pui32TxBuffer   = (uint32_t *)pui8Buffer;
    Transaction.pui32RxBuffer   = (uint32_t *)pui8Buffer;

    if (bBlocking)
    {
        return am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
    }
    else
    {
        return am_hal_iom_nonblocking_transfer(g_pIOMHandle, &Transaction, pfnCallback, pCallbackCtxt);
    }
}

uint32_t
am_widget_iom_test_blocking_write_read_error(void *pWidget, void *pCfg, char *pErrStr, uint32_t errorMask)
{
    am_widget_iom_test_t          *pTestCfg = (am_widget_iom_test_t *)pCfg;
    uint32_t                      ui32Status = AM_WIDGET_SUCCESS;
    uint32_t ui32Offset           = pTestCfg->ui32ByteOffset & 0x3;
    uint32_t ui32DevMode          = IOM_WIDGET_DEVICE_MODE;

    // This function relies on splitting the transaction into two
    // So - skip the size 1 transaction
    if (pTestCfg->ui32BufferSize < 2)
    {
        return AM_WIDGET_SUCCESS;
    }

#if APS6404L
    am_widget_iom_config_t        *pWidgetCfg = (am_widget_iom_config_t *)pWidget;
    if(pWidgetCfg->IOMConfig.ui32ClockFreq < AM_HAL_IOM_8MHZ)
    {
        return AM_WIDGET_ERROR; // For APS6404L, only meaningful when the frequency is higher than 8MHz
    }

    for(uint8_t i = 0; i < (sizeof(g_IOM_SpeedMax) / sizeof(g_IOM_SpeedMax[0])); i++)
    {
        if(pWidgetCfg->IOMConfig.ui32ClockFreq == g_IOM_SpeedMax[i].MHz)
        {
            uint32_t ui32Limit = 255 * (g_IOM_SpeedMax[i].MaxSize - 3); // AM_HAL_CMDQ_HW_IDX_MAX
            if(pTestCfg->ui32BufferSize > ui32Limit)
            {
                pTestCfg->ui32BufferSize = ui32Limit - (AM_HAL_IOM_MAX_TXNSIZE_SPI - pTestCfg->ui32BufferSize);
            }
        }
    }
#endif

    //
    // Create the transmit buffer to write.
    //
    load_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0);

    //
    // Write the buffer to the FRAM device.
    //
    if ( ui32DevMode == AM_WIDGET_IOM_SPI_DEVICE )
    {
        ui32Status = iom_device_func.iom_device_blocking_write(g_IomDevHdl, &g_pucOutBuffer[0] + ui32Offset, 0, pTestCfg->ui32BufferSize);
    }
    else
    {
        ui32Status = iom_device_func.iom_device_blocking_write(g_IomDevHdl, &g_pucOutBuffer[0] + ui32Offset, 0, pTestCfg->ui32BufferSize/2);
        if ( ui32Status )
        {
            return AM_WIDGET_ERROR;
        }
        if (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_DEVADDR) // Send I2C Write to unknown address - will cause NAK at the start of transaction
        {
            uint32_t expectedErr = AM_HAL_IOM_ERR_I2C_NAK;
            // Insert a transaction which will cause a NAK
            ui32Status = sendRawTransaction(AM_HAL_IOM_TX,
                                      AM_DEVICES_MB85RC256V_SLAVE_ID + 2,
                                      &g_pucOutBuffer[0] + ui32Offset,
                                      0,
                                      pTestCfg->ui32BufferSize,
                                      (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? true : false,
                                      true,
                                      0,
                                      0);
            if ( ui32Status != expectedErr )
            {
                return AM_WIDGET_ERROR;
            }
        }
#if 0 // FRAM does not treat this as an error - it just wraps the address
        if (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_OFFSETSTART) // Send I2C Write to unknown offset - will cause NAK at the start of transaction
        {
            uint32_t expectedErr = AM_HAL_IOM_ERR_I2C_NAK;
            // Insert a transaction which will cause a NAK
            ui32Status = sendRawTransaction(AM_HAL_IOM_TX,
                                      AM_DEVICES_MB85RC256V_SLAVE_ID,
                                      &g_pucOutBuffer[0] + ui32Offset,
                                      AM_DEVICES_MB85RC256V_SIZE,
                                      pTestCfg->ui32BufferSize,
                                      (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? true : false,
                                      true,
                                      0,
                                      0);
            if ( ui32Status != expectedErr )
            {
                return AM_WIDGET_ERROR;
            }
        }
#endif
#if 0 // FRAM does not treat this as an error - it just wraps the address
        if (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_OFFSETMID) // Send I2C Write with invalid size - will cause NAK in middle of transaction
        {
            uint32_t expectedErr = AM_HAL_IOM_ERR_I2C_NAK;
            // Insert a transaction which will cause a NAK
            ui32Status = sendRawTransaction(AM_HAL_IOM_TX,
                                      AM_DEVICES_MB85RC256V_SLAVE_ID,
                                      &g_pucOutBuffer[0] + ui32Offset,
                                      AM_DEVICES_MB85RC256V_SIZE - pTestCfg->ui32BufferSize/2,
                                      pTestCfg->ui32BufferSize,
                                      (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? true : false,
                                      true,
                                      0,
                                      0);
            if ( ui32Status != expectedErr )
            {
                return AM_WIDGET_ERROR;
            }
        }
#endif
        ui32Status = iom_device_func.iom_device_blocking_write(g_IomDevHdl, &g_pucOutBuffer[0] + ui32Offset + pTestCfg->ui32BufferSize/2, pTestCfg->ui32BufferSize/2, (pTestCfg->ui32BufferSize - pTestCfg->ui32BufferSize/2));
        if ( ui32Status )
        {
            return AM_WIDGET_ERROR;
        }
    }

    //
    // Read the buffer from the FRAM device.
    //
    if ( ui32DevMode == AM_WIDGET_IOM_SPI_DEVICE )
    {
        ui32Status = iom_device_func.iom_device_blocking_read(g_IomDevHdl, &g_pucInBuffer[0] + ui32Offset, 0, pTestCfg->ui32BufferSize);
    }
    else
    {
        ui32Status = iom_device_func.iom_device_blocking_read(g_IomDevHdl, &g_pucInBuffer[0] + ui32Offset, 0, pTestCfg->ui32BufferSize/2);
        if ( ui32Status )
        {
            return AM_WIDGET_ERROR;
        }
        if (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_DEVADDR) // Send I2C Read to unknown address - will cause NAK at the start of transaction
        {
            uint32_t expectedErr = AM_HAL_IOM_ERR_I2C_NAK;
            // Insert a transaction which will cause a NAK
            ui32Status = sendRawTransaction(AM_HAL_IOM_RX,
                                      AM_DEVICES_MB85RC256V_SLAVE_ID + 1,
                                      &g_pucOutBuffer[0] + ui32Offset,
                                      0,
                                      pTestCfg->ui32BufferSize,
                                      (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? true : false,
                                      true,
                                      0,
                                      0);
            if ( ui32Status != expectedErr )
            {
                return AM_WIDGET_ERROR;
            }
        }
#if 0 // FRAM does not treat this as an error - it just wraps the address
        if (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_OFFSETSTART) // Send I2C Read to unknown offset - will cause NAK at the start of transaction
        {
            uint32_t expectedErr = AM_HAL_IOM_ERR_I2C_NAK;
            // Insert a transaction which will cause a NAK
            ui32Status = sendRawTransaction(AM_HAL_IOM_RX,
                                      AM_DEVICES_MB85RC256V_SLAVE_ID,
                                      &g_pucOutBuffer[0] + ui32Offset,
                                      AM_DEVICES_MB85RC256V_SIZE,
                                      pTestCfg->ui32BufferSize,
                                      (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? true : false,
                                      true,
                                      0,
                                      0);
            if ( ui32Status != expectedErr )
            {
                return AM_WIDGET_ERROR;
            }
        }
        if (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_OFFSETMID) // Send I2C Read with invalid size - will cause NAK in middle of transaction
        {
            uint32_t expectedErr = AM_HAL_IOM_ERR_I2C_NAK;
            // Insert a transaction which will cause a NAK
            ui32Status = sendRawTransaction(AM_HAL_IOM_RX,
                                      AM_DEVICES_MB85RC256V_SLAVE_ID,
                                      &g_pucOutBuffer[0] + ui32Offset,
                                      AM_DEVICES_MB85RC256V_SIZE - pTestCfg->ui32BufferSize/2,
                                      pTestCfg->ui32BufferSize,
                                      (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? true : false,
                                      true,
                                      0,
                                      0);
            if ( ui32Status != expectedErr )
            {
                return AM_WIDGET_ERROR;
            }
        }
#endif
        ui32Status = iom_device_func.iom_device_blocking_read(g_IomDevHdl, &g_pucInBuffer[0] + ui32Offset + pTestCfg->ui32BufferSize/2, pTestCfg->ui32BufferSize/2, (pTestCfg->ui32BufferSize - pTestCfg->ui32BufferSize/2));
        if ( ui32Status )
        {
            return AM_WIDGET_ERROR;
        }
    }

    //
    // Compare the receive buffer to the transmit buffer.
    //
    if ( ui32Status || !compare_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0))
    {
        return AM_WIDGET_ERROR;
    }
    else
    {
        return AM_WIDGET_SUCCESS;
    }
}

uint32_t
am_widget_iom_test_queue_write_read_error(void *pWidget, void *pCfg, char *pErrStr, uint32_t errorMask)
{
    //am_widget_iom_config_t        *pWidgetCfg = (am_widget_iom_config_t *)pWidget;
    am_widget_iom_test_t          *pTestCfg = (am_widget_iom_test_t *)pCfg;
    uint32_t ui32Offset           = pTestCfg->ui32ByteOffset & 0x3;
    uint32_t ui32DevMode          = IOM_WIDGET_DEVICE_MODE;

    // Split into blocks - we need to account for 2 queued entries for
    // each block - one for Tx, one for Rx
    // For SPI case - each Write takes more than one SPI transaction as well
    uint32_t blockSize               = pTestCfg->ui32BufferSize / (pTestCfg->ui32QueueSize / 8) + 1;
    uint32_t ui32Status = 0;
    uint32_t numExpErr = 0;

    g_ExpErrCount = g_UnExpReturnCount = 0;
    //
    // Create the transmit buffer to write.
    //
    load_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0);

    if ( ui32DevMode == AM_WIDGET_IOM_SPI_DEVICE )
    {
        uint32_t ui32Bytes = 0;
        while ((0 == ui32Status) && (ui32Bytes < pTestCfg->ui32BufferSize))
        {
            uint32_t size =
                   (blockSize < (pTestCfg->ui32BufferSize - ui32Bytes)) ? blockSize : (pTestCfg->ui32BufferSize - ui32Bytes);
            if (errorMask & AM_WIDGET_IOM_ERRMASK_DMA_START) // Send SPI Write - with DMA from illegal address
            {
                uint32_t expectedErr = AM_HAL_STATUS_FAIL;
                ui32Status = iom_device_func.iom_device_nonblocking_write(g_IomDevHdl, (uint8_t *)(SRAM_BASEADDR + SRAM_SIZE), ui32Bytes, size, errCallback, (void *)expectedErr);
                if ( ui32Status )
                {
                    return AM_WIDGET_ERROR;
                }
                numExpErr++;
            }
            if ((errorMask & AM_WIDGET_IOM_ERRMASK_DMA_MID) && (size >= 2)) // Send SPI Write - with DMA from end of Flash (should cause DMAERR in the middle)
            {
                uint32_t expectedErr = AM_HAL_STATUS_FAIL;
                ui32Status = iom_device_func.iom_device_nonblocking_write(g_IomDevHdl, (uint8_t *)(SRAM_BASEADDR + SRAM_SIZE - size/2), ui32Bytes, size, errCallback, (void *)expectedErr);
                if ( ui32Status )
                {
                    return AM_WIDGET_ERROR;
                }
                numExpErr++;
            }
            ui32Status = iom_device_func.iom_device_nonblocking_write(g_IomDevHdl, &g_pucOutBuffer[0] + ui32Offset + ui32Bytes, ui32Bytes, size, 0, 0);
            ui32Bytes += size;
        }

        if (0 == ui32Status)
        {
            //
            // Read the buffer from the FRAM device.
            //
            g_bIOMNonBlockCompleteRd = 0;
            ui32Bytes = 0;
            while ((ui32Status == 0) && (ui32Bytes < pTestCfg->ui32BufferSize))
            {
                uint32_t size =
                       (blockSize < (pTestCfg->ui32BufferSize - ui32Bytes)) ? blockSize : (pTestCfg->ui32BufferSize - ui32Bytes);
                if (errorMask & AM_WIDGET_IOM_ERRMASK_DMA_START) // Send SPI Write - with DMA from illegal address
                {
                    uint32_t expectedErr = AM_HAL_STATUS_FAIL;
                    ui32Status = iom_device_func.iom_device_nonblocking_read(g_IomDevHdl, (uint8_t *)(SRAM_BASEADDR + SRAM_SIZE), ui32Bytes, size, errCallback, (void *)expectedErr);
                    if ( ui32Status )
                    {
                        return AM_WIDGET_ERROR;
                    }
                    numExpErr++;
                }
                if ((errorMask & AM_WIDGET_IOM_ERRMASK_DMA_MID) && (size >= 2)) // Send SPI Write - with DMA from end of Flash (should cause DMAERR in the middle)
                {
                    uint32_t expectedErr = AM_HAL_STATUS_FAIL;
                    ui32Status = iom_device_func.iom_device_nonblocking_read(g_IomDevHdl, (uint8_t *)(SRAM_BASEADDR + SRAM_SIZE - size/2), ui32Bytes, size, errCallback, (void *)expectedErr);
                    if ( ui32Status )
                    {
                        return AM_WIDGET_ERROR;
                    }
                    numExpErr++;
                }
                ui32Status = iom_device_func.iom_device_nonblocking_read(g_IomDevHdl, &g_pucInBuffer[0] + ui32Offset + ui32Bytes, ui32Bytes, size,
                                                                    (size == pTestCfg->ui32BufferSize - ui32Bytes) ? pfnWidget_IOM_Callback_Read : 0, 0);
                ui32Bytes += size;
            }
        }
    }
    else
    {
        uint32_t ui32Bytes = 0;
        while ((0 == ui32Status) && (ui32Bytes < pTestCfg->ui32BufferSize))
        {
            uint32_t size =
                   (blockSize < (pTestCfg->ui32BufferSize - ui32Bytes)) ? blockSize : (pTestCfg->ui32BufferSize - ui32Bytes);
            if (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_DEVADDR) // Send I2C Write to unknown address - will cause NAK at the start of transaction
            {
                uint32_t expectedErr = AM_HAL_IOM_ERR_I2C_NAK;
                // Insert a transaction which will cause a NAK
                ui32Status = sendRawTransaction(AM_HAL_IOM_TX,
                                          AM_DEVICES_MB85RC256V_SLAVE_ID + 2,
                                          &g_pucOutBuffer[0] + ui32Offset + ui32Bytes,
                                          ui32Bytes,
                                          size,
                                          (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? true : false,
                                          false,
                                          errCallback,
                                          (void *)expectedErr);
                if ( ui32Status )
                {
                    return AM_WIDGET_ERROR;
                }
                numExpErr++;
            }
            if (errorMask & AM_WIDGET_IOM_ERRMASK_DMA_START) // Send I2C Write - with DMA from illegal address
            {
                uint32_t expectedErr = AM_HAL_STATUS_FAIL;
                ui32Status = sendRawTransaction(AM_HAL_IOM_TX,
                                          AM_DEVICES_MB85RC256V_SLAVE_ID,
                                          (uint8_t *)(SRAM_BASEADDR + SRAM_SIZE),
                                          ui32Bytes,
                                          size,
                                          (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? true : false,
                                          false,
                                          errCallback,
                                          (void *)expectedErr);
                if ( ui32Status )
                {
                    return AM_WIDGET_ERROR;
                }
                numExpErr++;
            }
            if ((errorMask & AM_WIDGET_IOM_ERRMASK_DMA_MID) && (size >= 2)) // Send I2C Write - with DMA from end of Flash (should cause DMAERR in the middle)
            {
                uint32_t expectedErr = AM_HAL_STATUS_FAIL;
                ui32Status = sendRawTransaction(AM_HAL_IOM_TX,
                                          AM_DEVICES_MB85RC256V_SLAVE_ID,
                                          (uint8_t *)(SRAM_BASEADDR + SRAM_SIZE - size/2),
                                          ui32Bytes,
                                          size,
                                          (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? true : false,
                                          false,
                                          errCallback,
                                          (void *)expectedErr);
                if ( ui32Status )
                {
                    return AM_WIDGET_ERROR;
                }
                numExpErr++;
            }
            ui32Status = iom_device_func.iom_device_nonblocking_write(g_IomDevHdl, &g_pucOutBuffer[0] + ui32Offset + ui32Bytes,
                                                ui32Bytes,
                                                size,
                                                0, 0);
            if ( ui32Status )
            {
                return AM_WIDGET_ERROR;
            }
            ui32Bytes += size;
        }

        if (0 == ui32Status)
        {
            //
            // Read the buffer from the FRAM device.
            //
            g_bIOMNonBlockCompleteRd = 0;
            ui32Bytes = 0;
            while ((ui32Status == 0) && (ui32Bytes < pTestCfg->ui32BufferSize))
            {
                uint32_t size =
                       (blockSize < (pTestCfg->ui32BufferSize - ui32Bytes)) ? blockSize : (pTestCfg->ui32BufferSize - ui32Bytes);
                if (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_DEVADDR) // Insert a transaction with wrong device address
                {
                    uint32_t expectedError = AM_HAL_IOM_ERR_I2C_NAK;
                    // Insert a transaction which will cause a NAK
                    ui32Status = sendRawTransaction(AM_HAL_IOM_RX,
                                              AM_DEVICES_MB85RC256V_SLAVE_ID + 2,
                                              &g_pucOutBuffer[0] + ui32Offset + ui32Bytes,
                                              ui32Bytes,
                                              size,
                                              (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? (size & 0x1) : false,
                                              false,
                                              errCallback,
                                              (void *)expectedError);
                    if ( ui32Status )
                    {
                        return AM_WIDGET_ERROR;
                    }
                    numExpErr++;
                }
                if (errorMask & AM_WIDGET_IOM_ERRMASK_DMA_START) // Send I2C Read - with DMA to illegal address
                {
                    uint32_t expectedError = AM_HAL_STATUS_FAIL;
                    ui32Status = sendRawTransaction(AM_HAL_IOM_RX,
                                              AM_DEVICES_MB85RC256V_SLAVE_ID,
                                              (uint8_t *)(SRAM_BASEADDR + SRAM_SIZE),
                                              ui32Bytes,
                                              size,
                                              (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? (size & 0x1) : false,
                                              false,
                                              errCallback,
                                              (void *)expectedError);
                    if ( ui32Status )
                    {
                        return AM_WIDGET_ERROR;
                    }
                    numExpErr++;
                }
                if ((errorMask & AM_WIDGET_IOM_ERRMASK_DMA_MID) && (size >= 2)) // Send I2C Read - with DMA to end of SRAM (should cause DMAERR in the middle)
                {
                    uint32_t expectedError = AM_HAL_STATUS_FAIL;
                    ui32Status = sendRawTransaction(AM_HAL_IOM_RX,
                                              AM_DEVICES_MB85RC256V_SLAVE_ID,
                                              (uint8_t *)(SRAM_BASEADDR + SRAM_SIZE - size/2), // just before end of SRAM
                                              ui32Bytes,
                                              size,
                                              (errorMask & AM_WIDGET_IOM_ERRMASK_NAK_CONT) ? (size & 0x1) : false,
                                              false,
                                              errCallback,
                                              (void *)expectedError);
                    if ( ui32Status )
                    {
                        return AM_WIDGET_ERROR;
                    }
                    numExpErr++;
                }
                ui32Status = iom_device_func.iom_device_nonblocking_read(g_IomDevHdl, &g_pucInBuffer[0] + ui32Offset + ui32Bytes,
                                                   ui32Bytes,
                                                   size,
                                                   (size == pTestCfg->ui32BufferSize - ui32Bytes) ? pfnWidget_IOM_Callback_Read : 0,
                                                   0);
                if ( ui32Status )
                {
                    return AM_WIDGET_ERROR;
                }

                ui32Bytes += size;
            }
        }
    }

    if (0 == ui32Status)
    {
        //
        // Wait until the read has completed
        //
        wait_for_nonblock_response(pTestCfg->sleepMode, &g_bIOMNonBlockCompleteRd);

        if (g_bIOMNonBlockCompleteRd != 1)
        {
            return AM_WIDGET_ERROR;
        }
    }


    //
    // Compare the receive buffer to the transmit buffer.
    //
    if (ui32Status || !compare_buffers(ui32Offset, pTestCfg->ui32BufferSize, 0) || (numExpErr != g_ExpErrCount) || g_UnExpReturnCount )
    {
        return AM_WIDGET_ERROR;
    }
    else
    {
        return AM_WIDGET_SUCCESS;
    }
}

