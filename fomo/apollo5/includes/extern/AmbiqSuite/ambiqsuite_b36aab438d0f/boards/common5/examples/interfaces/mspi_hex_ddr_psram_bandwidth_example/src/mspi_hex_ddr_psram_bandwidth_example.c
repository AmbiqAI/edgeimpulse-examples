//*****************************************************************************
//
//! @file mspi_hex_ddr_psram_bandwidth_example.c
//!
//! @brief Example of the MSPI bandwidth test with DDR HEX SPI PSRAM.
//!
//! Purpose: This example demonstrates MSPI DDR HEX bandwidth using the DDR HEX
//! SPI PSRAM devices.
//!
//! Starting from apollo5b, the MSPI XIP read bandwidth is boosted by the
//! ARM CM55 cache and the MSPI CPU read combine feature. By default,
//! the CPU read queue is on(CPURQEN). Cache prefetch(RID3) and cache miss(RID2)
//! requests deemed appropriate by MSPI hardware are combined and processed
//! with a 2:1 ratio(GQARB) between general queue and CPU read queue.
//!
//! am_hal_mspi_cpu_read_burst_t default =
//! {
//!     .eGQArbBais                         = AM_HAL_MSPI_GQARB_2_1_BAIS,
//!     .bCombineRID2CacheMissAccess        = true,
//!     .bCombineRID3CachePrefetchAccess    = true,
//!     .bCombineRID4ICacheAccess           = false,
//!     .bCPUReadQueueEnable                = true,
//! }
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//!
//! AM_DEBUG_PRINTF
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
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

#include "mspi_hex_ddr_psram_bandwidth_example.h"

#include <stdlib.h>
#include <string.h>

#include "nema_hal.h"
#include "nema_core.h"
#include "nema_utils.h"
#include "nema_regs.h"
#include "nemagfx_heap_customize.h"

#define MSPI_PSRAM_TIMING_CHECK
// #### INTERNAL BEGIN ####
#if defined (APOLLO5_FPGA)
#undef MSPI_PSRAM_TIMING_CHECK
#endif
// #### INTERNAL END ####

#define MSPI_CPU_READ_QUEUE_ENABLE  1

#define TIMER_NUM                   0
#define TIME_DIVIDER                (16/96.0) //Chose HFRC_DIV16 as timer clk source when HFRC=96M

#define START_INSTANCE_INDEX        0
#define END_INSTANCE_INDEX          2

#define START_SPEED_INDEX           0
#define END_SPEED_INDEX             1

#define BANDWIDTH_TEST_LOOP         3

#define MSPI_BURST_SIZE               32
#define MSPI_SMALL_BLOCK_SIZE         (12*1024)
#define MSPI_LARGE_BLOCK_SIZE         (64*1024)

#define DUT_NUM (END_INSTANCE_INDEX - START_INSTANCE_INDEX)
#define SPEED_NUM (END_SPEED_INDEX - START_SPEED_INDEX)
#define LENGTH_NUM 3
#define FORMAT_NUM 3

#define MSPI_BUFFER_SIZE            (MSPI_LARGE_BLOCK_SIZE*BANDWIDTH_TEST_LOOP)

#define MSPI_PSRAM_HEX_CONFIG       MSPI_PSRAM_HexCE0sMSPIConfig

#define MSPI_TIMEOUT             1000000

// #### INTERNAL BEGIN ####
#if defined (APOLLO5_FPGA)
#undef MSPI_PSRAM_TIMING_CHECK
#undef TIME_DIVIDER
#undef START_INSTANCE_INDEX
#undef END_INSTANCE_INDEX
#undef START_SPEED_INDEX
#undef END_SPEED_INDEX

#define TIME_DIVIDER                (16/(APOLLO5_FPGA*1.0)) //Chose HFRC_DIV16 as timer clk source when HFRC=8M
#define START_INSTANCE_INDEX        0
#define END_INSTANCE_INDEX          1
#define START_SPEED_INDEX           0
#define END_SPEED_INDEX             1
#endif
// #### INTERNAL END ####

AM_SHARED_RW uint32_t           DMANonCachableBuffer[2560] __attribute__((aligned(32)));

#define GPU_CMDQ_BUFFER_ADDR    ((uint32_t)DMANonCachableBuffer)
#define GPU_CMDQ_BUFFER_SIZE    (8*1024)

#define MSPI_CMDQ_BUFFER_ADDR   ((uint32_t)DMANonCachableBuffer + GPU_CMDQ_BUFFER_SIZE)
#define MSPI_CMDQ_BUFFER_SIZE   (2048)

am_hal_mpu_region_config_t sMPUConfig =
{
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)DMANonCachableBuffer,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)DMANonCachableBuffer + sizeof(DMANonCachableBuffer) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
};

am_hal_mpu_attr_t sMPUAttribute =
{
    .ui8AttrIndex = 0,
    .bNormalMem = true,
    .sOuterAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .sInnerAttr = {
                    .bNonTransient = false,
                    .bWriteBack = true,
                    .bReadAllocate = false,
                    .bWriteAllocate = false
                  },
    .eDeviceAttr = 0,
} ;

AM_SHARED_RW uint8_t         g_SSRAMTXBuffer[MSPI_BUFFER_SIZE] __attribute__((aligned(128)));
AM_SHARED_RW uint8_t         g_SSRAMRXBuffer[MSPI_BUFFER_SIZE] __attribute__((aligned(128)));

uint8_t         g_TCMTXBuffer[MSPI_BUFFER_SIZE];
uint8_t         g_TCMRXBuffer[MSPI_BUFFER_SIZE];

buffer_config_t g_BufferCfg[NUM_MEM] =
{
  //MEM_NONE
  {NULL, NULL},
  //SRAM
  {g_SSRAMTXBuffer, g_SSRAMRXBuffer},
  //MCU_TCM
  {g_TCMTXBuffer, g_TCMRXBuffer},
};

uint32_t g_LengthCfg[LENGTH_NUM] = { MSPI_BURST_SIZE, MSPI_SMALL_BLOCK_SIZE, MSPI_LARGE_BLOCK_SIZE};

mspi_bandwidth_t g_result_table[DUT_NUM][SPEED_NUM][AM_HAL_MSPI_FLASH_MAX][MSPI_ACCESS_MAX][NUM_MEM][LENGTH_NUM];

mspi_speed_t mspi_test_speeds[] =
{
  { AM_HAL_MSPI_CLK_250MHZ,     " 125MHz" },
  { AM_HAL_MSPI_CLK_192MHZ,     " 96MHz" },
  { AM_HAL_MSPI_CLK_125MHZ,     "62.5MHz" },
  { AM_HAL_MSPI_CLK_96MHZ,      " 48MHz" },

  { AM_HAL_MSPI_CLK_62P5MHZ,    "31.3MHz"},
  { AM_HAL_MSPI_CLK_48MHZ,      " 24MHz" },
} ;

mspi_device_mode_str_t mspi_test_device_modes[] =
{
  { AM_HAL_MSPI_FLASH_SERIAL_CE0,       " SERIAL CE0" },
  { AM_HAL_MSPI_FLASH_SERIAL_CE1,       " SERIAL CE1" },
  { AM_HAL_MSPI_FLASH_DUAL_CE0,         " DUAL CE0" },
  { AM_HAL_MSPI_FLASH_DUAL_CE1,         " DUAL CE1" },
  { AM_HAL_MSPI_FLASH_QUAD_CE0,         " QUAD CE0" },
  { AM_HAL_MSPI_FLASH_QUAD_CE1,         " QUAD CE1" },
  { AM_HAL_MSPI_FLASH_OCTAL_CE0,        " OCTAL CE0" },
  { AM_HAL_MSPI_FLASH_OCTAL_CE1,        " OCTAL CE1" },
  { AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,    " OCTAL DDR CE0" },
  { AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,    " OCTAL DDR CE1" },
  { AM_HAL_MSPI_FLASH_HEX_DDR_CE0,      " HEX DDR CE0" },
  { AM_HAL_MSPI_FLASH_HEX_DDR_CE1,      " HEX DDR CE1" },
} ;

mspi_gpu_format_str_t mspi_gpu_format[] =
{
  { NEMA_RGB24,       " RGB888" },
  { NEMA_RGBA8888,    " RGBA8888" },
  { NEMA_RGB565,      " RGB565" },
};

static const char *g_Memstr[] =
{
    "NULL  ",
    "SSRAM ",
    "MCUTCM",
};

uint32_t mspi_instances[] = {0, 3};

void            *g_pDevHandle;
void            *g_pHandle;

am_abstract_mspi_devices_config_t MSPI_PSRAM_HexCE0sMSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_192MHZ,
    .ui32NBTxnBufLength       = MSPI_CMDQ_BUFFER_SIZE / sizeof(uint32_t),
    .pNBTxnBuf                = (uint32_t *)MSPI_CMDQ_BUFFER_ADDR,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

am_abstract_mspi_devices_config_t MSPI_PSRAM_HexCE1sMSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE1,
    .eClockFreq               = AM_HAL_MSPI_CLK_192MHZ,
    .ui32NBTxnBufLength       = MSPI_CMDQ_BUFFER_SIZE / sizeof(uint32_t),
    .pNBTxnBuf                = (uint32_t *)MSPI_CMDQ_BUFFER_ADDR,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
    MSPI3_IRQn,
};

const MSPIBaseAddr_t g_MSPIAddresses[AM_REG_MSPI_NUM_MODULES] =
{
  {MSPI0_XIP_BASEADDR, MSPI0_APERTURE_START_ADDR},
  {MSPI1_XIP_BASEADDR, MSPI1_APERTURE_START_ADDR},
  {MSPI2_APERTURE_START_ADDR, MSPI2_APERTURE_START_ADDR},
  {MSPI3_XIP_BASEADDR, MSPI3_APERTURE_START_ADDR},
};

mspi_device_func_t mspi_device_func =
{
#if defined(MSPI_HEX_DDR_PSRAM_1P8V_DEVICE)
    .devName = "APS25616N",
    .mspi_init = am_devices_mspi_psram_aps25616n_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps25616n_ddr_deinit,
    .mspi_read_id = am_devices_mspi_psram_aps25616n_ddr_id,
    .mspi_read = am_devices_mspi_psram_aps25616n_ddr_read,
    .mspi_read_adv = am_devices_mspi_psram_aps25616n_ddr_read_adv,
    .mspi_write = am_devices_mspi_psram_aps25616n_ddr_write,
    .mspi_write_adv = am_devices_mspi_psram_aps25616n_ddr_write_adv,
    .mspi_xip_enable = am_devices_mspi_psram_aps25616n_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps25616n_ddr_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_psram_aps25616n_ddr_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_psram_aps25616n_ddr_disable_scrambling,
    .mspi_init_timing_check = am_devices_mspi_psram_aps25616n_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_aps25616n_apply_ddr_timing,
#elif defined(MSPI_HEX_DDR_APM_PSRAM_1P2V_DEVICE)
    .devName = "APS25616BA",
    .mspi_init = am_devices_mspi_psram_aps25616ba_ddr_init,
    .mspi_term = am_devices_mspi_psram_aps25616ba_ddr_deinit,
    .mspi_read_id = am_devices_mspi_psram_aps25616ba_ddr_id,
    .mspi_read = am_devices_mspi_psram_aps25616ba_ddr_read,
    .mspi_read_adv = am_devices_mspi_psram_aps25616ba_ddr_read_adv,
    .mspi_write = am_devices_mspi_psram_aps25616ba_ddr_write,
    .mspi_write_adv = am_devices_mspi_psram_aps25616ba_ddr_write_adv,
    .mspi_xip_enable = am_devices_mspi_psram_aps25616ba_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_aps25616ba_ddr_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_psram_aps25616ba_ddr_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_psram_aps25616ba_ddr_disable_scrambling,
    .mspi_init_timing_check = am_devices_mspi_psram_aps25616ba_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_aps25616ba_apply_ddr_timing,
#elif defined(MSPI_HEX_DDR_WINBOND_PSRAM_DEVICE)
    .devName = "W958D6NW",
    .mspi_init = am_devices_mspi_psram_w958d6nw_ddr_init,
    .mspi_term = am_devices_mspi_psram_w958d6nw_ddr_deinit,
    .mspi_read_id = am_devices_mspi_psram_w958d6nw_ddr_id,
    .mspi_read = am_devices_mspi_psram_w958d6nw_ddr_read,
    .mspi_read_adv = am_devices_mspi_psram_w958d6nw_ddr_read_adv,
    .mspi_write = am_devices_mspi_psram_w958d6nw_ddr_write,
    .mspi_write_adv = am_devices_mspi_psram_w958d6nw_ddr_write_adv,
    .mspi_xip_enable = am_devices_mspi_psram_w958d6nw_ddr_enable_xip,
    .mspi_xip_disable = am_devices_mspi_psram_w958d6nw_ddr_disable_xip,
    .mspi_scrambling_enable = am_devices_mspi_psram_w958d6nw_ddr_enable_scrambling,
    .mspi_scrambling_disable = am_devices_mspi_psram_w958d6nw_ddr_disable_scrambling,
    .mspi_init_timing_check = am_devices_mspi_psram_hex_ddr_init_timing_check,
    .mspi_init_timing_apply = am_devices_mspi_psram_apply_hex_ddr_timing,
#endif
};

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************

void
am_mspi0_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pHandle, ui32Status);

}

void
am_mspi3_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_pHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pHandle, ui32Status);

}

static void pfnMSPI_Callback(void *pCallbackCtxt, uint32_t status)
{
    //
    // Set the DMA complete flag.
    //
    *(volatile bool *)pCallbackCtxt = true;
}

//*****************************************************************************
//
// Bandwidth timer initialization function.
//
//*****************************************************************************
void
ctimer_init(void)
{
    am_hal_timer_config_t       TimerConfig;

    //
    // Set up the default configuration.
    //
    am_hal_timer_default_config_set(&TimerConfig);

    //
    // Update the clock in the config.
    //
    TimerConfig.eInputClock             = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    TimerConfig.eFunction               = AM_HAL_TIMER_FN_UPCOUNT;

    //
    // Configure the TIMER.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_timer_config(TIMER_NUM, &TimerConfig) )
    {
        am_util_stdio_printf("Fail to config timer%d\n", TIMER_NUM);
    }

    //
    // Clear the TIMER.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_timer_clear_stop(TIMER_NUM) )
    {
        am_util_stdio_printf("Fail to clear&stop timer%d\n", TIMER_NUM);
    }

    //
    // Enable the TIMER.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_timer_enable(TIMER_NUM) )
    {
        am_util_stdio_printf("Fail to enable timer%d\n", TIMER_NUM);
    }

    //
    // Start the TIMER.
    //
    if ( AM_HAL_STATUS_SUCCESS !=  am_hal_timer_start(TIMER_NUM) )
    {
        am_util_stdio_printf("Fail to start timer%d\n", TIMER_NUM);
    }
}

//*****************************************************************************
//
// Bandwidth utility functions.
//
//*****************************************************************************
uint32_t cal_time(uint32_t ui32prev, uint32_t ui32curr)
{
  if ( ui32prev > ui32curr )
  {
      return 0xFFFFFFFF - ui32prev + ui32curr;
  }
  else
  {
      return ui32curr - ui32prev;
  }
}

bool mspi_check_buffer(uint32_t ui32NumBytes, uint8_t *pRxBuffer, uint8_t *pTxBuffer)
{
  for (uint32_t i = 0; i < ui32NumBytes; i++)
  {
    if (pRxBuffer[i] != pTxBuffer[i])
    {
      am_util_stdio_printf("Comparison failed at Index %2d with Received Value %2.2X while expected value was %2.2X\n", i, pRxBuffer[i], pTxBuffer[i]);
      return false;
    }
  }
  return true;
}

//*****************************************************************************
//
// MSPI DMA Bandwidth function.
//
//*****************************************************************************
bool mspi_dma_write_read_bandwidth(void *pDevHandle, void *pTestCfg)
{

    uint32_t    ui32TimerTickBefore = 0;
    uint32_t    ui32TimerTickAfter = 0;
    float       num_of_tick_write;
    float       num_of_tick_read;

    uint32_t    ui32Loop;

    uint32_t    ui32Status;

    //
    // Create a pointer to the MSPI test configuration.
    //
    mspi_test_config_t        *pTestConfig = (mspi_test_config_t *)pTestCfg;

    volatile bool bDMAComplete;

    // Generate data into the Sector Buffer
    for (uint32_t i = 0; i < pTestConfig->ui32NumBytes; i++)
    {
        pTestConfig->pTxBuffer[pTestConfig->ui32ByteOffset + i] = (i & 0xFF);
    }
    am_hal_cachectrl_dcache_clean(NULL);

    num_of_tick_write = 0;
    for ( ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        bDMAComplete = false;
        ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);
        ui32Status = mspi_device_func.mspi_write_adv(pDevHandle,
                                                    pTestConfig->pTxBuffer + pTestConfig->ui32ByteOffset,
                                                    pTestConfig->ui32SectorAddress + pTestConfig->ui32SectorOffset * ui32Loop,
                                                    pTestConfig->ui32NumBytes,
                                                    0, 0,
                                                    pfnMSPI_Callback, (void *)&bDMAComplete);
        if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to write to target sector.  Status = %d\n", ui32Status);
            break;
        }
        for (uint32_t i = 0; i < MSPI_TIMEOUT; i++)
        {
            if (bDMAComplete)
            {
                ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);
                break;
            }
        }
        if (!bDMAComplete)
        {
            break;
        }
        num_of_tick_write += cal_time(ui32TimerTickBefore, ui32TimerTickAfter);
    }
    if ( ui32Loop == 0 )
    {
        return false;
    }
    else
    {
        if ( ui32Loop != BANDWIDTH_TEST_LOOP )
        {
            am_util_stdio_printf("############# Warning: write ui32Loop = %d #############\n", ui32Loop);
        }
        num_of_tick_write /= ui32Loop;
    }

    num_of_tick_read = 0;
    for ( ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        bDMAComplete = false;
        ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);
        ui32Status = mspi_device_func.mspi_read_adv(pDevHandle,
                                                pTestConfig->pRxBuffer + pTestConfig->ui32ByteOffset + pTestConfig->ui32SectorOffset * ui32Loop,
                                                pTestConfig->ui32SectorAddress + pTestConfig->ui32SectorOffset * ui32Loop,
                                                pTestConfig->ui32NumBytes,
                                                0, 0,
                                                pfnMSPI_Callback, (void *)&bDMAComplete);
        ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);
        if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to read from target sector.  Status = %d\n", ui32Status);
            break;
        }
        for (uint32_t i = 0; i < MSPI_TIMEOUT; i++)
        {
            if (bDMAComplete)
            {
                ui32TimerTickAfter = am_hal_timer_read(TIMER_NUM);
                break;
            }
        }
        if (!bDMAComplete)
        {
            break;
        }
        num_of_tick_read += cal_time(ui32TimerTickBefore, ui32TimerTickAfter);
    }
    if ( ui32Loop == 0 )
    {
        return false;
    }
    else
    {
        if ( ui32Loop != BANDWIDTH_TEST_LOOP )
        {
            am_util_stdio_printf("############# Warning: read ui32Loop = %d #############\n", ui32Loop);
        }
        num_of_tick_read /= ui32Loop;
    }

    am_hal_cachectrl_dcache_invalidate(NULL, true);

    bool bPass = true;
    for ( ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        if ( !mspi_check_buffer(pTestConfig->ui32NumBytes, pTestConfig->pRxBuffer + pTestConfig->ui32ByteOffset + pTestConfig->ui32SectorOffset * ui32Loop, pTestConfig->pTxBuffer + pTestConfig->ui32ByteOffset) )
        {
            am_util_stdio_printf("ui32Loop#%d, Read & Write buffer doesn not match!\n", ui32Loop);
            bPass = false;
        }
    }

    pTestConfig->sbandwidth.f32WriteBandwidth = (float)pTestConfig->ui32NumBytes / num_of_tick_write / TIME_DIVIDER;
    pTestConfig->sbandwidth.f32ReadBandwidth = (float)pTestConfig->ui32NumBytes / num_of_tick_read / TIME_DIVIDER;

    am_util_debug_printf("Write: %d bytes per Loop, Bandwidth is %f MB/s\n", pTestConfig->ui32NumBytes, pTestConfig->sbandwidth.f32WriteBandwidth);
    am_util_debug_printf("Read:  %d bytes per Loop, Bandwidth is %f MB/s\n", pTestConfig->ui32NumBytes, pTestConfig->sbandwidth.f32ReadBandwidth);

    return bPass;
}

//*****************************************************************************
//
// MSPI XIPMM Bandwidth function.
//
//*****************************************************************************
bool
mspi_xipmm_memcpy_bandwidth(void *pDevHandle, void *pTestCfg)
{
    uint32_t    *pu32Ptr;

    uint32_t    ui32TimerTickBeforeWrite = 0;
    uint32_t    ui32TimerTickAfterWrite = 0;
    uint32_t    ui32TimerTickBeforeRead = 0;
    uint32_t    ui32TimerTickAfterRead = 0;
    uint32_t    num_of_tick_write, num_of_tick_read;

    bool bPass = true;

    //
    // Create a pointer to the MSPI test configuration.
    //
    mspi_test_config_t          *pTestConfig = (mspi_test_config_t *)pTestCfg;

    uint32_t ui32Status = mspi_device_func.mspi_xip_enable(pDevHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        return false;
    }

    if ( pTestConfig->pTxBuffer == NULL || pTestConfig->pRxBuffer == NULL )
    {
        return false;
    }

    uint8_t *pu8TxPtr = (uint8_t *)(pTestConfig->pTxBuffer);
    for ( uint32_t i = 0; i < pTestConfig->ui32NumBytes; i++ )
    {
        *pu8TxPtr++ = i;
    }

    num_of_tick_write = 0;
    for ( uint32_t ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        am_hal_cachectrl_dcache_clean(NULL);
        pu32Ptr = (uint32_t *)(pTestConfig->ui32SectorAddress + pTestConfig->ui32ByteOffset * ui32Loop);
        ui32TimerTickBeforeWrite = am_hal_timer_read(TIMER_NUM);
        memcpy(pu32Ptr, pTestConfig->pTxBuffer, pTestConfig->ui32NumBytes);
        ui32TimerTickAfterWrite = am_hal_timer_read(TIMER_NUM);
        num_of_tick_write += cal_time(ui32TimerTickBeforeWrite, ui32TimerTickAfterWrite);
    }
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    num_of_tick_read = 0;
    for ( uint32_t ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        pu32Ptr = (uint32_t *)(pTestConfig->ui32SectorAddress + pTestConfig->ui32ByteOffset * ui32Loop);
        uint8_t *pu8Dest = pTestConfig->pRxBuffer + pTestConfig->ui32ByteOffset * ui32Loop;
        ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
        memcpy(pu8Dest, pu32Ptr, pTestConfig->ui32NumBytes);
        ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
        num_of_tick_read += cal_time(ui32TimerTickBeforeRead, ui32TimerTickAfterRead);
    }

    for ( uint32_t ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        if ( !mspi_check_buffer(pTestConfig->ui32NumBytes, pTestConfig->pRxBuffer + pTestConfig->ui32ByteOffset * ui32Loop, pTestConfig->pTxBuffer) )
        {
            am_util_stdio_printf("ui32Loop#%d, Read & Write buffer doesn't match\n", ui32Loop);
            bPass = false;
        }
    }

    pTestConfig->sbandwidth.f32WriteBandwidth = (float)pTestConfig->ui32NumBytes * BANDWIDTH_TEST_LOOP / num_of_tick_write / TIME_DIVIDER;
    pTestConfig->sbandwidth.f32ReadBandwidth = (float)pTestConfig->ui32NumBytes * BANDWIDTH_TEST_LOOP / num_of_tick_read / TIME_DIVIDER;

    am_util_debug_printf("Write: %d bytes, Bandwidth is %f MB/s\n", pTestConfig->ui32NumBytes, pTestConfig->sbandwidth.f32WriteBandwidth);
    am_util_debug_printf("Read:  %d bytes, Bandwidth is %f MB/s\n", pTestConfig->ui32NumBytes, pTestConfig->sbandwidth.f32ReadBandwidth);

    //
    // Need to make sure all pending XIPMM transactions are flushed
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    ui32Status = mspi_device_func.mspi_xip_disable(pDevHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode.  Status = %d\n", ui32Status);
        return false;
    }

    return bPass;
}

 //*****************************************************************************
//
// MSPI XIPMM Word Read Bandwidth function.
//
//*****************************************************************************
static void read_standalone(register uint32_t *pu32Ptr, register uint32_t *pu32PtrEnd)
{
    uint32_t read;
    while(pu32Ptr < pu32PtrEnd)
    {
        read = *pu32Ptr++;
    }
}

bool
mspi_xipmm_word_read_bandwidth(void *pDevHandle, void *pTestCfg)
{
    register uint32_t    *pu32Ptr;

    uint32_t    ui32TimerTickBeforeRead = 0;
    uint32_t    ui32TimerTickAfterRead = 0;
    uint32_t    num_of_tick_read;

    bool bPass = true;
    //
    // Create a pointer to the MSPI test configuration.
    //
    mspi_test_config_t          *pTestConfig = (mspi_test_config_t *)pTestCfg;

    uint32_t ui32Status = mspi_device_func.mspi_xip_enable(pDevHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        return false;
    }

    num_of_tick_read = 0;
    for ( uint32_t ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        uint32_t *pu32PtrEnd;
        pu32Ptr = (uint32_t *)(pTestConfig->ui32SectorAddress + pTestConfig->ui32ByteOffset * ui32Loop);
        pu32PtrEnd = pu32Ptr + pTestConfig->ui32NumBytes / sizeof(uint32_t);
        ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
        read_standalone(pu32Ptr, pu32PtrEnd);
        ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
        num_of_tick_read += cal_time(ui32TimerTickBeforeRead, ui32TimerTickAfterRead);
    }

    pTestConfig->sbandwidth.f32WriteBandwidth = 0.0;
    pTestConfig->sbandwidth.f32ReadBandwidth = (float)pTestConfig->ui32NumBytes * BANDWIDTH_TEST_LOOP / num_of_tick_read / TIME_DIVIDER;

    am_util_debug_printf("Read:  %d bytes, Bandwidth is %f MB/s\n", pTestConfig->ui32NumBytes, pTestConfig->sbandwidth.f32ReadBandwidth);

    //
    // Need to make sure all pending XIPMM transactions are flushed
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    ui32Status = mspi_device_func.mspi_xip_disable(pDevHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode.  Status = %d\n", ui32Status);
        return false;
    }

    return bPass;
}

//*****************************************************************************
//
// MSPI XIPMM Word Access Bandwidth function.
//
//*****************************************************************************
bool
mspi_xipmm_word_access_bandwidth(void *pDevHandle, void *pTestCfg)
{
    uint32_t    *pu32Ptr;

    uint32_t    ui32TimerTickBeforeWrite = 0;
    uint32_t    ui32TimerTickAfterWrite = 0;
    uint32_t    ui32TimerTickBeforeRead = 0;
    uint32_t    ui32TimerTickAfterRead = 0;
    uint32_t    num_of_tick_write, num_of_tick_read;

    bool bPass = true;

    //
    // Create a pointer to the MSPI test configuration.
    //
    mspi_test_config_t          *pTestConfig = (mspi_test_config_t *)pTestCfg;

    uint32_t ui32Status = mspi_device_func.mspi_xip_enable(pDevHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        return false;
    }

    if ( pTestConfig->pTxBuffer == NULL || pTestConfig->pRxBuffer == NULL )
    {
        return false;
    }

    uint32_t *pu32TxPtr = (uint32_t *)(pTestConfig->pTxBuffer);
    for ( uint32_t i = 0; i < pTestConfig->ui32NumBytes / 4; i++ )
    {
        *pu32TxPtr++ = i;
    }

    num_of_tick_write = 0;
    for ( uint32_t ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        uint32_t *pu32PtrEnd;
        am_hal_cachectrl_dcache_clean(NULL);
        pu32Ptr = (uint32_t *)(pTestConfig->ui32SectorAddress + pTestConfig->ui32ByteOffset * ui32Loop);
        pu32PtrEnd = pu32Ptr + pTestConfig->ui32NumBytes / 4;
        pu32TxPtr = (uint32_t *)(pTestConfig->pTxBuffer);
        ui32TimerTickBeforeWrite = am_hal_timer_read(TIMER_NUM);
        while(pu32Ptr < pu32PtrEnd)
        {
            *pu32Ptr++ = *pu32TxPtr++;
        }
        ui32TimerTickAfterWrite = am_hal_timer_read(TIMER_NUM);
        num_of_tick_write += cal_time(ui32TimerTickBeforeWrite, ui32TimerTickAfterWrite);
    }
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    num_of_tick_read = 0;
    for ( uint32_t ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        uint32_t *pu32DestEnd;
        uint32_t *pu32Dest = (uint32_t *)(pTestConfig->pRxBuffer + pTestConfig->ui32ByteOffset * ui32Loop);
        pu32DestEnd = pu32Dest + pTestConfig->ui32NumBytes / 4;
        pu32Ptr = (uint32_t *)(pTestConfig->ui32SectorAddress + pTestConfig->ui32ByteOffset * ui32Loop);
        ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
        while(pu32Dest < pu32DestEnd)
        {
            *pu32Dest++ = *pu32Ptr++;
        }
        ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
        num_of_tick_read += cal_time(ui32TimerTickBeforeRead, ui32TimerTickAfterRead);
    }

    for ( uint32_t ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        if ( !mspi_check_buffer(pTestConfig->ui32NumBytes, pTestConfig->pRxBuffer + pTestConfig->ui32ByteOffset * ui32Loop, pTestConfig->pTxBuffer) )
        {
            am_util_stdio_printf("ui32Loop#%d, Read & Write buffer doesn't match\n", ui32Loop);
            bPass = false;
        }
    }

    pTestConfig->sbandwidth.f32WriteBandwidth = (float)pTestConfig->ui32NumBytes * BANDWIDTH_TEST_LOOP / num_of_tick_write / TIME_DIVIDER;
    pTestConfig->sbandwidth.f32ReadBandwidth = (float)pTestConfig->ui32NumBytes * BANDWIDTH_TEST_LOOP / num_of_tick_read / TIME_DIVIDER;

    am_util_debug_printf("Write: %d bytes, Bandwidth is %f MB/s\n", pTestConfig->ui32NumBytes, pTestConfig->sbandwidth.f32WriteBandwidth);
    am_util_debug_printf("Read:  %d bytes, Bandwidth is %f MB/s\n", pTestConfig->ui32NumBytes, pTestConfig->sbandwidth.f32ReadBandwidth);

    //
    // Need to make sure all pending XIPMM transactions are flushed
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    ui32Status = mspi_device_func.mspi_xip_disable(pDevHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode.  Status = %d\n", ui32Status);
        return false;
    }

    return bPass;
}

//*****************************************************************************
//
// GPU XIPMM Bandwidth function.
//
//*****************************************************************************
bool
graphic_heap_region_need_flush(uint32_t region_start, uint32_t region_size)
{
#if NON_CACHEABLE_CL_RB_HEAP == 1
    if ( (region_start >= (uint32_t)GPU_CMDQ_BUFFER_ADDR) &&
       (region_start < (uint32_t)GPU_CMDQ_BUFFER_ADDR + GPU_CMDQ_BUFFER_SIZE) )
    {
        return false;
    }
    else
#endif
    {
        return true;
    }
}

bool
mspi_gpu_bandwidth(void *pDevHandle, void *pTestCfg)
{
    img_obj_t TX_obj;
    img_obj_t PSRAM_obj;
    img_obj_t RX_obj;

    //nemagfx_mspi_burst_length_t fb_burst_length = GPU_MSPI_BURST_LENGTH_128;
    //nemagfx_mspi_burst_length_t tex_burst_length = GPU_MSPI_BURST_LENGTH_128;
    //uint32_t    burst_size_reg_val = 0x0UL|(fb_burst_length<<4)|(tex_burst_length);

    uint32_t    ui32Status;
    uint32_t    ui32TimerTickBeforeWrite = 0;
    uint32_t    ui32TimerTickAfterWrite = 0;
    uint32_t    ui32TimerTickBeforeRead = 0;
    uint32_t    ui32TimerTickAfterRead = 0;
    uint32_t    num_of_tick_write, num_of_tick_read;

    bool bPass = true;

    //
    // Create a pointer to the MSPI test configuration.
    //
    mspi_test_config_t  *pTestConfig = (mspi_test_config_t *)pTestCfg;

    ui32Status = mspi_device_func.mspi_xip_enable(pDevHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        return false;
    }

    TX_obj.bo.base_phys = (uintptr_t)pTestConfig->pTxBuffer;
    TX_obj.bo.base_virt = (void*)TX_obj.bo.base_phys;
    TX_obj.bo.size = pTestConfig->ui32NumBytes;
    TX_obj.bo.fd = 0;
    TX_obj.w = pTestConfig->ui32NumBytes / nema_format_size(pTestConfig->ui8Format) / 32;
    TX_obj.h = 32;
    TX_obj.stride = -1;
    TX_obj.color = 0;
    TX_obj.format = pTestConfig->ui8Format;
    TX_obj.sampling_mode = 0;

    PSRAM_obj.bo.base_phys = (uintptr_t)pTestConfig->ui32SectorAddress;
    PSRAM_obj.bo.base_virt = (void*)PSRAM_obj.bo.base_phys;
    PSRAM_obj.bo.size = pTestConfig->ui32NumBytes;
    PSRAM_obj.bo.fd = 0;
    PSRAM_obj.w = pTestConfig->ui32NumBytes / nema_format_size(pTestConfig->ui8Format) / 32;
    PSRAM_obj.h = 32;
    PSRAM_obj.stride = -1;
    PSRAM_obj.color = 0;
    PSRAM_obj.format = pTestConfig->ui8Format;
    PSRAM_obj.sampling_mode = 0;

    RX_obj.bo.base_phys = (uintptr_t)pTestConfig->pRxBuffer;
    RX_obj.bo.base_virt = (void*)RX_obj.bo.base_phys;
    RX_obj.bo.size = pTestConfig->ui32NumBytes;
    RX_obj.bo.fd = 0;
    RX_obj.w = pTestConfig->ui32NumBytes / nema_format_size(pTestConfig->ui8Format) / 32;
    RX_obj.h = 32;
    RX_obj.stride = -1;
    RX_obj.color = 0;
    RX_obj.format = pTestConfig->ui8Format;
    RX_obj.sampling_mode = 0;

    nema_cmdlist_t sCL = nema_cl_create();
    nema_cl_bind(&sCL);

    // Generate data into TX Buffer
    for (uint32_t i = 0; i < pTestConfig->ui32NumBytes * BANDWIDTH_TEST_LOOP; i++)
    {
        pTestConfig->pTxBuffer[i] = (i & 0xFF);
    }
    am_hal_cachectrl_dcache_clean(NULL);

    num_of_tick_write = 0;
    for ( uint32_t ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        //
        // GPU blit to PSRAM
        //
        ui32TimerTickBeforeWrite = am_hal_timer_read(TIMER_NUM);
        nema_cl_rewind(&sCL);
        nema_cl_bind(&sCL);
        nema_bind_dst_tex(PSRAM_obj.bo.base_phys + pTestConfig->ui32ByteOffset * ui32Loop,
                          PSRAM_obj.w,
                          PSRAM_obj.h,
                          PSRAM_obj.format,
                          PSRAM_obj.stride);
        nema_bind_src_tex(TX_obj.bo.base_phys + pTestConfig->ui32ByteOffset * ui32Loop,
                          TX_obj.w,
                          TX_obj.h,
                          TX_obj.format,
                          TX_obj.stride,
                          NEMA_FILTER_PS);
        nema_set_clip(0, 0, PSRAM_obj.w, PSRAM_obj.h);
        //nema_cl_add_cmd(NEMA_BURST_SIZE, burst_size_reg_val);
        //nema_clear(0);
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_blit(0, 0);
        nema_cl_submit(&sCL);
        nema_cl_wait(&sCL);
        ui32TimerTickAfterWrite = am_hal_timer_read(TIMER_NUM);
        num_of_tick_write += cal_time(ui32TimerTickBeforeWrite, ui32TimerTickAfterWrite);
    }

    num_of_tick_read = 0;
    for ( uint32_t ui32Loop = 0; ui32Loop < BANDWIDTH_TEST_LOOP; ui32Loop++ )
    {
        //
        // GPU blit from psram
        //
        ui32TimerTickBeforeRead = am_hal_timer_read(TIMER_NUM);
        nema_cl_rewind(&sCL);
        nema_cl_bind(&sCL);
        nema_bind_dst_tex(RX_obj.bo.base_phys + pTestConfig->ui32ByteOffset * ui32Loop,
                          RX_obj.w,
                          RX_obj.h,
                          RX_obj.format,
                          RX_obj.stride);
        nema_bind_src_tex(PSRAM_obj.bo.base_phys + pTestConfig->ui32ByteOffset * ui32Loop,
                          PSRAM_obj.w,
                          PSRAM_obj.h,
                          PSRAM_obj.format,
                          PSRAM_obj.stride,
                          NEMA_FILTER_PS);
        nema_set_clip(0, 0, RX_obj.w, RX_obj.h);
        //nema_cl_add_cmd(NEMA_BURST_SIZE, burst_size_reg_val);
        //nema_clear(0);
        nema_set_blend_blit(NEMA_BL_SRC);
        nema_blit(0, 0);
        nema_cl_submit(&sCL);
        nema_cl_wait(&sCL);
        ui32TimerTickAfterRead = am_hal_timer_read(TIMER_NUM);
        num_of_tick_read += cal_time(ui32TimerTickBeforeRead, ui32TimerTickAfterRead);
    }
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    nema_cl_destroy(&sCL);

    //
    // Run the memcmp for Tx and Rx buffer
    //
    int cmp_ret = memcmp(RX_obj.bo.base_virt, TX_obj.bo.base_virt, pTestConfig->ui32NumBytes * BANDWIDTH_TEST_LOOP);

    if ( cmp_ret )
    {
        bPass = false;
        am_util_stdio_printf("Frame buffer compare FAILED! \n");
    }
    else
    {
        am_util_stdio_printf("Frame buffer compare PASS \n");
    }

    pTestConfig->sbandwidth.f32WriteBandwidth = (float)(PSRAM_obj.w * PSRAM_obj.h * nema_format_size(pTestConfig->ui8Format)) * BANDWIDTH_TEST_LOOP / num_of_tick_write / TIME_DIVIDER;
    pTestConfig->sbandwidth.f32ReadBandwidth = (float)(PSRAM_obj.w * PSRAM_obj.h * nema_format_size(pTestConfig->ui8Format)) * BANDWIDTH_TEST_LOOP / num_of_tick_read / TIME_DIVIDER;

    am_util_debug_printf("Write: %d bytes, Bandwidth is %f MB/s\n", (PSRAM_obj.w * PSRAM_obj.h * nema_format_size(pTestConfig->ui8Format)), pTestConfig->sbandwidth.f32WriteBandwidth);
    am_util_debug_printf("Read:  %d bytes, Bandwidth is %f MB/s\n", (PSRAM_obj.w * PSRAM_obj.h * nema_format_size(pTestConfig->ui8Format)), pTestConfig->sbandwidth.f32ReadBandwidth);

    //
    // Need to make sure all pending XIPMM transactions are flushed
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    ui32Status = mspi_device_func.mspi_xip_disable(pDevHandle);
    if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode.  Status = %d\n", ui32Status);
        return false;
    }

    return bPass;
}


//*****************************************************************************
//
// MSPI print bandwidth result function.
//
//*****************************************************************************
void mspi_print_bandwidth_result(void)
{
  char wstr[9];
  char rstr[9];

  am_util_stdio_printf("\n################# Bandwidth Summary #################\n");
  for (uint32_t ui32Index = START_INSTANCE_INDEX; ui32Index < END_INSTANCE_INDEX; ui32Index++)
  {
    am_util_stdio_printf("\nDevice %s on MSPI%d%s ", mspi_device_func.devName, mspi_instances[ui32Index], mspi_test_device_modes[MSPI_PSRAM_HEX_CONFIG.eDeviceConfig].string);
#if MSPI_CPU_READ_QUEUE_ENABLE
    am_util_stdio_printf("CPU read queue enabled\n");
#else
    am_util_stdio_printf("CPU read queue disabled\n");
#endif
    am_util_stdio_printf("\n|%-25s|  %7s  |  %-6s  |  %-8s  |  %8s  |  %8s  |\n", "  Access Type", "IO Freq", "Buffer", "NumBytes", "Wr MB/s", "Rd MB/s");
    for (uint32_t ui32SpeedIndex = START_SPEED_INDEX; ui32SpeedIndex < END_SPEED_INDEX; ui32SpeedIndex++)
    {

      for ( am_memory_type_e eMEMType = SSRAM; eMEMType <= SSRAM; eMEMType++ )
      {
        for ( uint32_t ui32lenIndex = 1; ui32lenIndex < LENGTH_NUM; ui32lenIndex++ )
        {
          am_util_stdio_sprintf(wstr, "%8.3f", g_result_table[ui32Index][ui32SpeedIndex][MSPI_PSRAM_HEX_CONFIG.eDeviceConfig][MSPI_DMA_ACCESS][eMEMType][ui32lenIndex].f32WriteBandwidth);
          am_util_stdio_sprintf(rstr, "%8.3f", g_result_table[ui32Index][ui32SpeedIndex][MSPI_PSRAM_HEX_CONFIG.eDeviceConfig][MSPI_DMA_ACCESS][eMEMType][ui32lenIndex].f32ReadBandwidth);
          am_util_stdio_printf("|%-25s|  %7s  |  %6s  |  %8d  |  %8s  |  %8s  |\n", " DMA W/R PSRAM", mspi_test_speeds[ui32SpeedIndex].string, g_Memstr[eMEMType], g_LengthCfg[ui32lenIndex], wstr, rstr);
        }
      }

      for ( am_memory_type_e eMEMType = SSRAM; eMEMType <= MCU_TCM; eMEMType++ )
      {
        for ( uint32_t ui32lenIndex = 1; ui32lenIndex < LENGTH_NUM; ui32lenIndex++ )
        {
          am_util_stdio_sprintf(wstr, "%8.3f", g_result_table[ui32Index][ui32SpeedIndex][MSPI_PSRAM_HEX_CONFIG.eDeviceConfig][MSPI_MEMCPY_XIPMM_ACCESS][eMEMType][ui32lenIndex].f32WriteBandwidth);
          am_util_stdio_sprintf(rstr, "%8.3f", g_result_table[ui32Index][ui32SpeedIndex][MSPI_PSRAM_HEX_CONFIG.eDeviceConfig][MSPI_MEMCPY_XIPMM_ACCESS][eMEMType][ui32lenIndex].f32ReadBandwidth);
          am_util_stdio_printf("|%-25s|  %7s  |  %6s  |  %8d  |  %8s  |  %8s  |\n", " XIPMM memcpy W/R PSRAM", mspi_test_speeds[ui32SpeedIndex].string, g_Memstr[eMEMType], g_LengthCfg[ui32lenIndex], wstr, rstr);
        }
      }

      for ( am_memory_type_e eMEMType = MEM_NONE; eMEMType <= MCU_TCM; eMEMType++ )
      {
        for ( uint32_t ui32lenIndex = 1; ui32lenIndex < LENGTH_NUM; ui32lenIndex++ )
        {
          am_util_stdio_sprintf(wstr, "%8.3f", g_result_table[ui32Index][ui32SpeedIndex][MSPI_PSRAM_HEX_CONFIG.eDeviceConfig][MSPI_XIPMM_ACCESS_32B][eMEMType][ui32lenIndex].f32WriteBandwidth);
          am_util_stdio_sprintf(rstr, "%8.3f", g_result_table[ui32Index][ui32SpeedIndex][MSPI_PSRAM_HEX_CONFIG.eDeviceConfig][MSPI_XIPMM_ACCESS_32B][eMEMType][ui32lenIndex].f32ReadBandwidth);
          am_util_stdio_printf("|%-25s|  %7s  |  %6s  |  %8d  |  %8s  |  %8s  |\n", " XIPMM Word W/R PSRAM", mspi_test_speeds[ui32SpeedIndex].string, g_Memstr[eMEMType], g_LengthCfg[ui32lenIndex], wstr, rstr);
        }
      }

      for ( am_memory_type_e eMEMType = SSRAM; eMEMType <= SSRAM; eMEMType++ )
      {
        for ( uint32_t ui32FormatIndex = 0; ui32FormatIndex < FORMAT_NUM; ui32FormatIndex++ )
        {
          char title[25];
          am_util_stdio_sprintf(title, " GPU%s W/R PSRAM", mspi_gpu_format[ui32FormatIndex].string);
          for ( uint32_t ui32lenIndex = 1; ui32lenIndex < LENGTH_NUM; ui32lenIndex++ )
          {
            am_util_stdio_sprintf(wstr, "%8.3f", g_result_table[ui32Index][ui32SpeedIndex][MSPI_PSRAM_HEX_CONFIG.eDeviceConfig][MSPI_GPU_RGB24 + ui32FormatIndex][eMEMType][ui32lenIndex].f32WriteBandwidth);
            am_util_stdio_sprintf(rstr, "%8.3f", g_result_table[ui32Index][ui32SpeedIndex][MSPI_PSRAM_HEX_CONFIG.eDeviceConfig][MSPI_GPU_RGB24 + ui32FormatIndex][eMEMType][ui32lenIndex].f32ReadBandwidth);
            am_util_stdio_printf("|%-25s|  %7s  |  %6s  |  %8d  |  %8s  |  %8s  |\n", title, mspi_test_speeds[ui32SpeedIndex].string, g_Memstr[eMEMType], g_LengthCfg[ui32lenIndex], wstr, rstr);
          }
        }
      }

    }
  }

}

//*****************************************************************************
//
// MSPI Example Main.
//
//*****************************************************************************
int
main(void)
{
    uint32_t      ui32Status;

    uint32_t      ui32Module;
    bool          bTestPassed;

    am_hal_pwrctrl_sram_memcfg_t  SRAMCfg =
    {
        .eSRAMCfg       = AM_HAL_PWRCTRL_SRAM_3M,
        .eActiveWithMCU = AM_HAL_PWRCTRL_SRAM_3M,
        .eSRAMRetain    = AM_HAL_PWRCTRL_SRAM_3M
    };

    am_abstract_mspi_devices_config_t sMSPIConfig;
    mspi_test_config_t                sTestConfig;

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttribute, 1);
    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();
    //
    // Set up the regions.
    //
    am_hal_mpu_region_configure(&sMPUConfig, 1);
    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);
    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);

#if defined(CPU_HP)
    //
    // Configure CPU HP
    //
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
// #### INTERNAL BEGIN ####
#ifndef APOLLO5_FPGA
// #### INTERNAL END ####
        while(1);
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
    }
#endif

#if defined(GPU_HP)
    //
    // Configure GPU HP
    //
    if ( am_hal_pwrctrl_gpu_mode_select(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
// #### INTERNAL BEGIN ####
#ifndef APOLLO5_FPGA
// #### INTERNAL END ####
        while(1);
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
    }
#endif

    //
    // Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    graphic_heap_init((void*)GPU_CMDQ_BUFFER_ADDR, GPU_CMDQ_BUFFER_SIZE);

    //
    // Initialize NemaGFX
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    if ( nema_init() != 0 )
    {
        while(1);
    }

    //
    // Configure ssram
    //
    am_hal_pwrctrl_sram_config(&SRAMCfg);

    //
    // Initialize ctimer
    //
    ctimer_init();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo5 MSPI HEX DDR PSRAM Bandwidth Example\n\n");

    for (uint32_t ui32Index = START_INSTANCE_INDEX; ui32Index < END_INSTANCE_INDEX; ui32Index++)
    {
        for (uint32_t ui32SpeedIndex = START_SPEED_INDEX; ui32SpeedIndex < END_SPEED_INDEX; ui32SpeedIndex++)
        {
            ui32Module = mspi_instances[ui32Index];
            sMSPIConfig = MSPI_PSRAM_HEX_CONFIG;
            sMSPIConfig.eClockFreq = mspi_test_speeds[ui32SpeedIndex].eFreq;


#ifdef MSPI_PSRAM_TIMING_CHECK
            am_abstract_mspi_devices_timing_config_t sMSPIDdrTimingConfig;
            am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
            if ( AM_ABSTRACT_MSPI_SUCCESS != mspi_device_func.mspi_init_timing_check(ui32Module, &sMSPIConfig, &sMSPIDdrTimingConfig) )
            {
                am_util_stdio_printf("==== Scan Result: Failed, no valid setting.  \n");
                while(1);
            }
#endif
            //
            // Configure the MSPI and PSRAM Device.
            //
            ui32Status = mspi_device_func.mspi_init(ui32Module, &sMSPIConfig, &g_pDevHandle, &g_pHandle);
            if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
            {
                am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
                am_util_stdio_printf("Must ensure Device is Connected and Properly Initialized!\n");
                while(1);
            }

            NVIC_SetPriority(mspi_interrupts[ui32Module], AM_IRQ_PRIORITY_DEFAULT);
            NVIC_EnableIRQ(mspi_interrupts[ui32Module]);

            am_hal_interrupt_master_enable();

#ifdef MSPI_PSRAM_TIMING_CHECK
            //
            //  Set the DDR timing from previous scan.
            //
            mspi_device_func.mspi_init_timing_apply(g_pDevHandle, &sMSPIDdrTimingConfig);
#endif

#if defined(AM_PART_APOLLO5B)
            sTestConfig.sCPURQCFG.eGQArbBais = AM_HAL_MSPI_GQARB_2_1_BAIS;
            sTestConfig.sCPURQCFG.bCombineRID2CacheMissAccess = true;
            sTestConfig.sCPURQCFG.bCombineRID3CachePrefetchAccess = true;
            sTestConfig.sCPURQCFG.bCombineRID4ICacheAccess = false;
            sTestConfig.sCPURQCFG.bCPUReadQueueEnable = MSPI_CPU_READ_QUEUE_ENABLE;
            am_hal_mspi_control(g_pHandle, AM_HAL_MSPI_REQ_CPU_READ_COMBINE, &sTestConfig.sCPURQCFG);
#endif

            am_hal_cachectrl_dcache_invalidate(NULL, true);
            //
            // DMA bandwidth test loop
            //
            for ( am_memory_type_e eMEMType = SSRAM; eMEMType <= SSRAM; eMEMType++ )
            {
                am_util_stdio_printf("\n=============== Starting%s DMA Bandwidth test at Frequency = %s, MSPI #%d ===============\n",
                                     mspi_test_device_modes[sMSPIConfig.eDeviceConfig].string,
                                     mspi_test_speeds[ui32SpeedIndex].string,
                                     ui32Module);
                am_util_stdio_printf("Device: %s\n", mspi_device_func.devName);
                am_util_stdio_printf("Buffer Location: %s\n", g_Memstr[eMEMType]);

                for ( uint32_t ui32LenIndex = 1; ui32LenIndex < LENGTH_NUM; ui32LenIndex++ )
                {
                    sTestConfig.pTxBuffer = g_BufferCfg[eMEMType].pTxBuffer;
                    sTestConfig.pRxBuffer = g_BufferCfg[eMEMType].pRxBuffer;
                    sTestConfig.ui32NumBytes = g_LengthCfg[ui32LenIndex];
                    sTestConfig.ui32SectorOffset = g_LengthCfg[ui32LenIndex];
                    sTestConfig.ui32ByteOffset = 0;
                    sTestConfig.ui32SectorAddress = 0x00000000;

                    bTestPassed = mspi_dma_write_read_bandwidth(g_pDevHandle, &sTestConfig);

                    if (!bTestPassed)
                    {
                        am_util_stdio_printf("%s PSRAM Write Read bandwidth test failed at Frequency = %s, Numbytes = %d, on MSPI #%d\n",
                                              mspi_test_device_modes[sMSPIConfig.eDeviceConfig].string,
                                              sTestConfig.ui32NumBytes,
                                              mspi_test_speeds[ui32SpeedIndex].string,
                                              ui32Module);
                        break;
                    }

                    g_result_table[ui32Index][ui32SpeedIndex][sMSPIConfig.eDeviceConfig][MSPI_DMA_ACCESS][eMEMType][ui32LenIndex] = sTestConfig.sbandwidth;
                }
            }

            am_hal_cachectrl_dcache_invalidate(NULL, true);

            //
            // XIPMM word access bandwidth test loop
            //
            for ( am_memory_type_e eMEMType = SSRAM; eMEMType <= MCU_TCM; eMEMType++ )
            {
                for ( uint32_t ui32LenIndex = 0; ui32LenIndex < LENGTH_NUM; ui32LenIndex++ )
                {
                    sTestConfig.ui32NumBytes = g_LengthCfg[ui32LenIndex];       //ui32NumBytes == ui32ByteOffset for continuous address access
                    sTestConfig.ui32ByteOffset = g_LengthCfg[ui32LenIndex];
                    sTestConfig.pTxBuffer = g_BufferCfg[eMEMType].pTxBuffer;
                    sTestConfig.pRxBuffer = g_BufferCfg[eMEMType].pRxBuffer;
                    sTestConfig.ui32SectorAddress = g_MSPIAddresses[ui32Module].XIPMMBase;

                    am_util_stdio_printf("\n=============== Starting%s XIPMM Word Bandwidth test at Frequency = %s, MSPI #%d ===============\n",
                                        mspi_test_device_modes[sMSPIConfig.eDeviceConfig].string,
                                        mspi_test_speeds[ui32SpeedIndex].string,
                                        ui32Module);
                    am_util_stdio_printf("Device: %s\n", mspi_device_func.devName);
                    am_util_stdio_printf("Buffer Location: %s\n", g_Memstr[eMEMType]);

                    bTestPassed = mspi_xipmm_word_access_bandwidth(g_pDevHandle, &sTestConfig);
                    if (!bTestPassed)
                    {
                        am_util_stdio_printf("%s XIPMM word access failed to collect bandwidth, Frequency = %s, MSPI #%d\n",
                                            mspi_test_device_modes[sMSPIConfig.eDeviceConfig].string,
                                            mspi_test_speeds[ui32SpeedIndex].string,
                                            ui32Module);
                    }
                    g_result_table[ui32Index][ui32SpeedIndex][sMSPIConfig.eDeviceConfig][MSPI_XIPMM_ACCESS_32B][eMEMType][ui32LenIndex] = sTestConfig.sbandwidth;
                }
            }

            am_hal_cachectrl_dcache_invalidate(NULL, true);

            //
            // XIPMM word read bandwidth test loop
            //
            for ( uint32_t ui32LenIndex = 0; ui32LenIndex < LENGTH_NUM; ui32LenIndex++ )
            {
                sTestConfig.ui32NumBytes = g_LengthCfg[ui32LenIndex];       //ui32NumBytes == ui32ByteOffset for continuous address access
                sTestConfig.ui32ByteOffset = g_LengthCfg[ui32LenIndex];
                sTestConfig.pTxBuffer = g_BufferCfg[MEM_NONE].pTxBuffer;
                sTestConfig.pRxBuffer = g_BufferCfg[MEM_NONE].pRxBuffer;
                sTestConfig.ui32SectorAddress = g_MSPIAddresses[ui32Module].XIPMMBase;

                am_util_stdio_printf("\n=============== Starting%s XIPMM Word Read Bandwidth test at Frequency = %s, MSPI #%d ===============\n",
                                    mspi_test_device_modes[sMSPIConfig.eDeviceConfig].string,
                                    mspi_test_speeds[ui32SpeedIndex].string,
                                    ui32Module);
                am_util_stdio_printf("Device: %s\n", mspi_device_func.devName);
                am_util_stdio_printf("Buffer Location: %s\n", g_Memstr[MEM_NONE]);

                bTestPassed = mspi_xipmm_word_read_bandwidth(g_pDevHandle, &sTestConfig);
                if (!bTestPassed)
                {
                    am_util_stdio_printf("%s XIPMM Word Read failed to collect bandwidth, Frequency = %s, MSPI #%d\n",
                                        mspi_test_device_modes[sMSPIConfig.eDeviceConfig].string,
                                        mspi_test_speeds[ui32SpeedIndex].string,
                                        ui32Module);
                }
                g_result_table[ui32Index][ui32SpeedIndex][sMSPIConfig.eDeviceConfig][MSPI_XIPMM_ACCESS_32B][MEM_NONE][ui32LenIndex] = sTestConfig.sbandwidth;
            }

            am_hal_cachectrl_dcache_invalidate(NULL, true);

            //
            // XIPMM memcpy bandwidth test loop
            //
            for ( am_memory_type_e eMEMType = SSRAM; eMEMType <= MCU_TCM; eMEMType++ )
            {
                for ( uint32_t ui32LenIndex = 0; ui32LenIndex < LENGTH_NUM; ui32LenIndex++ )
                {
                    sTestConfig.ui32NumBytes = g_LengthCfg[ui32LenIndex];       //ui32NumBytes == ui32ByteOffset for continuous address access
                    sTestConfig.ui32ByteOffset = g_LengthCfg[ui32LenIndex];
                    sTestConfig.pTxBuffer = g_BufferCfg[eMEMType].pTxBuffer;
                    sTestConfig.pRxBuffer = g_BufferCfg[eMEMType].pRxBuffer;
                    sTestConfig.ui32SectorAddress = g_MSPIAddresses[ui32Module].XIPMMBase;

                    am_util_stdio_printf("\n=============== Starting%s XIPMM memcpy Bandwidth test at Frequency = %s, MSPI #%d ===============\n",
                                        mspi_test_device_modes[sMSPIConfig.eDeviceConfig].string,
                                        mspi_test_speeds[ui32SpeedIndex].string,
                                        ui32Module);
                    am_util_stdio_printf("Device: %s\n", mspi_device_func.devName);
                    am_util_stdio_printf("Buffer Location: %s\n", g_Memstr[eMEMType]);

                    bTestPassed = mspi_xipmm_memcpy_bandwidth(g_pDevHandle, &sTestConfig);
                    if (!bTestPassed)
                    {
                        am_util_stdio_printf("%s XIPMM memcpy failed to collect bandwidth, Frequency = %s, MSPI #%d\n",
                                            mspi_test_device_modes[sMSPIConfig.eDeviceConfig].string,
                                            mspi_test_speeds[ui32SpeedIndex].string,
                                            ui32Module);
                    }
                    g_result_table[ui32Index][ui32SpeedIndex][sMSPIConfig.eDeviceConfig][MSPI_MEMCPY_XIPMM_ACCESS][eMEMType][ui32LenIndex] = sTestConfig.sbandwidth;
                }
            }

            am_hal_cachectrl_dcache_invalidate(NULL, true);

            //
            // GPU bandwidth test loop
            //
            for ( am_memory_type_e eMEMType = SSRAM; eMEMType <= SSRAM; eMEMType++ )
            {
                for ( uint32_t ui32FormatIndex = 0; ui32FormatIndex < FORMAT_NUM; ui32FormatIndex++ )
                {
                    for ( uint32_t ui32LenIndex = 1; ui32LenIndex < LENGTH_NUM; ui32LenIndex++ )
                    {
                        sTestConfig.ui32NumBytes = g_LengthCfg[ui32LenIndex];       //ui32NumBytes == ui32ByteOffset for continuous address access
                        sTestConfig.ui32ByteOffset = g_LengthCfg[ui32LenIndex];
                        sTestConfig.pTxBuffer = g_BufferCfg[eMEMType].pTxBuffer;
                        sTestConfig.pRxBuffer = g_BufferCfg[eMEMType].pRxBuffer;
                        sTestConfig.ui32SectorAddress = g_MSPIAddresses[ui32Module].XIPMMBase;
                        sTestConfig.ui8Format = mspi_gpu_format[ui32FormatIndex].ui8Format;

                        am_util_stdio_printf("\n=============== Starting%s GPU write read%s test at Frequency = %s, MSPI #%d ===============\n",
                                            mspi_test_device_modes[sMSPIConfig.eDeviceConfig].string,
                                            mspi_gpu_format[ui32FormatIndex].string,
                                            mspi_test_speeds[ui32SpeedIndex].string,
                                            ui32Module);
                        am_util_stdio_printf("Device: %s\n", mspi_device_func.devName);
                        am_util_stdio_printf("Buffer Location: %s\n", g_Memstr[eMEMType]);

                        bTestPassed = mspi_gpu_bandwidth(g_pDevHandle, &sTestConfig);
                        if (!bTestPassed)
                        {
                            am_util_stdio_printf("%s GPU write read failed to collect bandwidth, Frequency = %s, MSPI #%d\n",
                                                mspi_test_device_modes[sMSPIConfig.eDeviceConfig].string,
                                                mspi_test_speeds[ui32SpeedIndex].string,
                                                ui32Module);
                        }
                        g_result_table[ui32Index][ui32SpeedIndex][sMSPIConfig.eDeviceConfig][MSPI_GPU_RGB24 + ui32FormatIndex][eMEMType][ui32LenIndex] = sTestConfig.sbandwidth;
                        am_util_delay_ms(10);
                    }
                }
            }

            //
            // Clean up the MSPI.
            //
            am_hal_interrupt_master_disable();
            NVIC_DisableIRQ(mspi_interrupts[ui32Module]);

            //
            // Need to make sure all pending XIPMM transactions are flushed
            //
            am_hal_cachectrl_dcache_invalidate(NULL, true);

            ui32Status = mspi_device_func.mspi_term(g_pDevHandle);
            if (AM_ABSTRACT_MSPI_SUCCESS != ui32Status)
            {
                am_util_stdio_printf("Failed to shutdown the MSPI and Flash Device!\n");
            }
        }
    }

    mspi_print_bandwidth_result();

    //
    //  End banner.
    //
    am_util_stdio_printf("\nApollo5 MSPI HEX DDR PSRAM Bandwidth Example Complete\n");

    while (1);
}
