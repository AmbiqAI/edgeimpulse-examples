//*****************************************************************************
//
//! @file cross_memory_test_cases.c
//!
//! @brief Exercise the TCM, SSRAM, MRAM and PSRAM across all
//! boundaries, alignments, and operations. Move data using CM4 memcpy, GPU blit
//! and DC read with 1 - 15 bytes offsets. Run across various DAXI and Cache
//! configurations. This testcase includes the following 9 tests.
//!
//! Jumper configurations:
//! a. MUX Switch is 0b00000101.
//! b. Short I2S0 DATA in GIPO4 and GPIO12 for I2S loopback.
//!
//! 1. cm4_memcpy_cross_internal_memory_test
//! Objective: CPU accesses all internal memories and XIPMM.
//! Test Outline:
//! CM4 copy data from source memory (MRAM, TCM, SSRAM0, DSP0_IRAM, DSP0_DRAM,
//! SSRAM1 and PSRAM (XIPMM0)) to destination memory (TCM, SSRAM0, DSP0_IRAM,
//! DSP0_DRAM, SSRAM1 and PSRAM (XIPMM0)). CM4 memory copy runs across all source
//! and destination memory combinations.
//! This test covers 0 - 16 bytes offsets both for source and destination addresses.
//! This test covers byte accessing, half-word accessing and word accessing.
//! This test crosses block boundaries within a memory and boundaries between
//! 2 memories.
//! Source and destination data buffer size for once copy operation is fixed to
//! 32 bytes.
//! This test sweeps various DAXI and cache configurations, need to define macros
//! "DAXI_TEST" and "CACHE_TEST" to enable this test option.
//! Each copy operation runs in both HP and LP modes, need to define macros
//! "HPLP_TEST_EN" to enable this test option.
//! Pass/Fail Criteria: The data in destination location must be same with the
//! data in source location. Run to end without any hardfaults and wasn't stuck.
//!
//! 2. gpu_blit_cross_internal_memory_test
//! Objective: CPU accesses all internal memories and XIPMM.
//! Test Outline:
//! GPU blit data from source memory (SSRAM0, DSP0_IRAM, DSP0_DRAM, SSRAM1 and
//! PSRAM (XIPMM0)) to destination memory (SSRAM0, DSP0_IRAM, DSP0_DRAM, SSRAM1
//! and PSRAM (XIPMM0)). GPU memory copy runs across all source and destination
//! memory combinations.
//! This test covers 0 - 16 bytes offsets both for source and destination addresses.
//! This test crosses block boundaries within a memory and boundaries between
//! 2 memories.
//! Source and destination data buffer sizes for once copy operation include
//! 4K, 8K, 16K and 32K bytes, need to define macro "MULTIPLE_GPU_BUFFER_SIZE" to
//! enable this test option.
//! This test sweeps various DAXI and cache configurations, need to define macros
//! "DAXI_TEST" and "CACHE_TEST" to enable this test option.
//! Pass/Fail Criteria: The data in destination location must be same with the
//! data in source location. Run to end without any hardfaults and wasn't stuck.
//!
//! 3. apbdma_mspi_cross_memory_test
//! Objective: MSPI DMA accesses all internal memories.
//! Test Outline:
//! MSPI DMA writes data to a fixed area in PSRAM with TX buffers in different
//! memories (MRAM, TCM, SSRAM0, ESRAM, SSRAM1), and reads data from that PSRAM
//! area with RX buffers in different memories (MRAM, TCM, SSRAM0, ESRAM
//! SSRAM1). This test covers 0 - 16 bytes offsets both for source and
//! destination addresses. This test crosses block boundaries within a memory
//! and boundaries between different type of memories. Source and destination
//! data buffer size for one data movement is fixed to 4K bytes. This test
//! sweeps various DAXI and cache configurations, need to define macros
//! "DAXI_TEST" and "CACHE_TEST" to enable this test option.
//! Pass/Fail Criteria: The data in destination location must be same with the
//! data in source location. Run to end without any hardfaults and wasn't stuck.
//!
//! 4. apbdma_sdio_cross_memory_test
//! Objective: SDIO DMA accesses all internal memories.
//! Test Outline:
//! SDIO DMA writes data to a fixed area in eMMC with TX buffers in different
//! memories (MRAM, TCM, SSRAM0, ESRAM, SSRAM1, XIPMM0), and reads data from
//! that eMMC area with RX buffers in different memories (MRAM, TCM, SSRAM0,
//! ESRAM, SSRAM1, XIPMM0). This test covers 0 - 16 bytes offsets both for
//! source and destination addresses. This test crosses block boundaries within
//! a memory and boundaries between different type of memories. Source and
//! destination data buffer size for one data movement is fixed to 512 bytes.
//! This test sweeps various DAXI and cache configurations, need to define
//! macros "DAXI_TEST" and "CACHE_TEST" to enable this test option.
//! Pass/Fail Criteria: The data in destination location must be same with the
//! data in source location. Run to end without any hardfaults and wasn't stuck.
//!
//! 5. apbdma_iom_cross_memory_test
//! Objective: IOM DMA accesses all internal memories.
//! Test Outline:
//! IOM DMA writes data to a fixed area in PSRAM with TX buffers in different
//! memories (MRAM, TCM, SSRAM0, ESRAM, SSRAM1, XIPMM0), and reads data from
//! that PSRAM area with RX buffers in different memories (MRAM, TCM, SSRAM0,
//! ESRAM, SSRAM1, XIPMM0). This test covers 0 - 16 bytes offsets both for
//! source and destination addresses. This test crosses block boundaries within
//! a memory and boundaries between different type of memories. Source and
//! destination data buffer size for one data movement is fixed to 4K bytes.
//! This test sweeps various DAXI and cache configurations, need to define
//! macros "DAXI_TEST" and "CACHE_TEST" to enable this test option.
//! Pass/Fail Criteria: The data in destination location must be same with the
//! data in source location. Run to end without any hardfaults and wasn't stuck.
//!
//! 6. apbdma_i2s_cross_memory_test
//! Objective: I2S DMA accesses all internal memories.
//! Test Outline:
//! I2S0 DMA sends data with TX buffers in different memories (MRAM, TCM,
//! SSRAM0, ESRAM, SSRAM1, XIPMM0), and I2S0 receives data
//! with RX buffers in different memories (MRAM, TCM, SSRAM0,
//! ESRAM, SSRAM1, XIPMM0). This test covers 0 - 16 bytes offsets both for
//! source and destination addresses. This test crosses block boundaries within
//! a memory and boundaries between different type of memories. Source and
//! destination data buffer size for one data movement is fixed to 4K bytes.
//! This test sweeps various DAXI and cache configurations, need to define
//! macros "DAXI_TEST" and "CACHE_TEST" to enable this test option.
//! Pass/Fail Criteria: The data in destination location must be same with the
//! data in source location. Run to end without any hardfaults and wasn't stuck.
//!
//! 7. apbdma_crc_cross_memory_test
//! Objective: CRC DMA accesses all internal memories.
//! Test Outline:
//! CRC DMA reads data from different
//! memories (MRAM, TCM, SSRAM0, ESRAM, SSRAM1, XIPMM0), and calculates CRC.
//! This test covers 0 - 16 bytes offsets both for
//! source and destination addresses. This test crosses block boundaries within
//! a memory and boundaries between different type of memories. Destination data
//! buffer size for one data movement is fixed to 4K bytes.
//! This test sweeps various DAXI and cache configurations, need to define
//! macros "DAXI_TEST" and "CACHE_TEST" to enable this test option.
//! Pass/Fail Criteria: The CRCs are all matched. Run to end without any
//! hardfaults and wasn't stuck.
//!
//! 8. apbdma_pdm_cross_memory_test
//! Objective: PDM DMA accesses all internal memories.
//! Test Outline:
//! PDM DMA receives data with RX buffers in different memories (TCM, SSRAM0,
//! ESRAM, SSRAM1, XIPMM0. This test covers 0 - 16 bytes offsets both for
//! source and destination addresses. This test crosses block boundaries within
//! a memory and boundaries between different type of memories.
//! Destination data buffer size for one data movement is fixed to 4K bytes.
//! This test sweeps various DAXI and cache configurations, need to define
//! macros "DAXI_TEST" and "CACHE_TEST" to enable this test option.
//! Pass/Fail Criteria: Run to end without any hardfaults and wasn't stuck.
//!
//! 9. apbdma_adc_cross_memory_test
//! Objective: ADC DMA accesses all internal memories.
//! Test Outline:
//! ADC DMA receives data with RX buffers in different memories (TCM, SSRAM0,
//! ESRAM, SSRAM1, XIPMM0. This test covers 0 - 16 bytes offsets both for
//! source and destination addresses. This test crosses block boundaries within
//! a memory and boundaries between different type of memories.
//! Destination data buffer size for one data movement is fixed to 4K bytes.
//! This test sweeps various DAXI and cache configurations, need to define
//! macros "DAXI_TEST" and "CACHE_TEST" to enable this test option.
//! Pass/Fail Criteria: Run to end without any hardfaults and wasn't stuck.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "string.h"
#include "stdlib.h"
#include "nema_utils.h"
#include "nema_graphics.h"
#include "am_devices_mspi_psram_aps25616n.h"
#include "am_devices_spipsram.h"
#include "daxi_test_common.h"

//*****************************************************************************
//
// Macros for test items.
//
//*****************************************************************************
#define TCM_ALIGNED_ADDR_WITHOUT_BOUNDARY    0x10054000
#define SSRAM0_ALIGNED_ADDR_WITHOUT_BOUNDARY 0x10074000
#define SIMPLIFIED_TEST
#define MSPI_PSRAM_MODULE 0

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
typedef enum {
    MRAM = 0,
    MCU_TCM,
    SSRAM0,
    XIPMM0,
    NUM_MEM
} am_memory_type_e;

typedef struct {
    uint32_t ui32StartAddr;
    uint32_t ui32Size;
    uint32_t ui32BlockSize;
    bool bFollowedByMem;
} am_memory_config_t;

#ifdef SIMPLIFIED_TEST
am_memory_config_t sMemCfg[] = {
    { // sMramCfg
        .ui32StartAddr = 0x0,
        .ui32Size = 0x200000,
        .ui32BlockSize = 0x100000,
        .bFollowedByMem = false
    },
    { // sMcuTcmCfg
        .ui32StartAddr = 0x10000000,
        .ui32Size = 0x60000,
        .ui32BlockSize = 0x20000,//0x8000,
        .bFollowedByMem = true
    },
    { // sSsram0Cfg
        .ui32StartAddr = 0x10060000,
        .ui32Size = 0x100000,
        .ui32BlockSize = 0x40000,//0x8000,
        .bFollowedByMem = false
    },
    { // sXIPMM0Cfg
        .ui32StartAddr = 0x14000000,
        .ui32Size = 0x2000000,
        .ui32BlockSize = 0x800000,//0x800,
        .bFollowedByMem = false
    }
};
#else //full test
am_memory_config_t sMemCfg[] = {
    { // sMramCfg
        .ui32StartAddr = 0x0,
        .ui32Size = 0x200000,
        .ui32BlockSize = 0x80000,
        .bFollowedByMem = false
    },
    { // sMcuTcmCfg
        .ui32StartAddr = 0x10000000,
        .ui32Size = 0x60000,
        .ui32BlockSize = 0x8000, // TODO: skip 0x10008000 boundary tests
        .bFollowedByMem = true
    },
//    { // sMcuTcm0Cfg
//        .ui32StartAddr = 0x10000000,
//        .ui32Size = 0x20000,
//        .ui32BlockSize = 0x1000,
//        .bFollowedByMem = true
//    },
//    { // sMcuTcm1Cfg
//        .ui32StartAddr = 0x10020000,
//        .ui32Size = 0x40000,
//        .ui32BlockSize = 0x8000,
//        .bFollowedByMem = true
//    },
    { // sSsram0Cfg
        .ui32StartAddr = 0x10060000,
        .ui32Size = 0x100000,
        .ui32BlockSize = 0x8000,
        .bFollowedByMem = false
    },
    { // sXIPMM0Cfg
        .ui32StartAddr = 0x14000000,
        .ui32Size = 0x2000000,
        .ui32BlockSize = 0x800,
        .bFollowedByMem = false
    }
};
#endif
typedef enum {
    BYTE = 0,
    HALF_WORD,
    WORD,
    //DOUBLE_WORD,
    NUM_DATA_SIZE
} am_data_size_e;

typedef enum {
    CPU_MEMCPY = 0,
    GPU_BLIT,
    APBDMA_MSPI,
    APBDMA_SDIO,
    APBDMA_IOM,
    APBDMA_I2S,
    APBDMA_CRC,
    APBDMA_PDM,
    APBDMA_ADC
} am_access_master_e;


uint32_t        ui32DMATCBBuffer2[2560];

void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;
bool            g_bDMAFinish;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
#ifdef APS25616N_OCTAL_MODE_EN
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE1,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer2) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer2,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type MspiInterrupts[] =
{
    MSPI0_IRQn,
#if defined(AM_PART_APOLLO3P) || defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L)
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(MSPI_PSRAM_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr
#if defined(__GNUC_STDC_INLINE__)
    #warning Please used the *.ld file in cross_memory_test_cases/ to replace the linker scripts in gcc/ directory.
#endif
#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION < 6000000)
const uint8_t g_ui8ConstData[] __attribute__ ((at(0x7FFF0)))=
#elif defined(__IAR_SYSTEMS_ICC__)
const uint8_t g_ui8ConstData[48] @ 0x7FFF0 =
#elif defined(__GNUC_STDC_INLINE__)
const uint8_t g_ui8ConstData[] __attribute__ ((section(".const"))) =
#endif
{
    0x26, 0x48, 0x56, 0xed, 0x00, 0xfe, 0x6c, 0x7a,
    0xff, 0xb4, 0x12, 0x53, 0x90, 0x01, 0xab, 0xc1,
    0xee, 0x89, 0x59, 0x72, 0x6a, 0x7f, 0x51, 0xa1,
    0x64, 0x77, 0x12, 0x32, 0x34, 0xd4, 0x3e, 0x88,
    0x61, 0x2f, 0x77, 0x8d, 0x89, 0x99, 0xed, 0xda,
    0x33, 0x44, 0x56, 0x9a, 0xda, 0xe1, 0x09, 0xf0
};

#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION < 6000000)
const uint8_t g_ui8ConstData1[] __attribute__ ((at(0xFFFF0)))=
#elif defined(__IAR_SYSTEMS_ICC__)
const uint8_t g_ui8ConstData1[] @ 0xFFFF0 =
#elif defined(__GNUC_STDC_INLINE__)
const uint8_t g_ui8ConstData1[] __attribute__ ((section(".const1"))) =
#endif
{
    0x26, 0x4a, 0x56, 0xee, 0x00, 0xf4, 0x6c, 0x7a,
    0xff, 0xb6, 0x13, 0x53, 0x95, 0x01, 0xa4, 0xc8,
    0xed, 0x86, 0x59, 0x72, 0x6a, 0x76, 0x51, 0xa1,
    0x68, 0x77, 0x17, 0x32, 0x30, 0xd4, 0x5e, 0x88,
    0xa1, 0x2f, 0x97, 0x6d, 0x89, 0x99, 0xfd, 0x4a,
    0x34, 0x48, 0x58, 0x0a, 0xd1, 0xe5, 0x09, 0xf0
};

#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION < 6000000)
const uint8_t g_ui8ConstData2[] __attribute__ ((at(0x17FFF0)))=
#elif defined(__IAR_SYSTEMS_ICC__)
const uint8_t g_ui8ConstData2[] @ 0x17FFF0 =
#elif defined(__GNUC_STDC_INLINE__)
const uint8_t g_ui8ConstData2[] __attribute__ ((section(".const2"))) =
#endif
{
    0x27, 0x49, 0x5d, 0xed, 0x00, 0xf4, 0x5c, 0x2a,
    0xff, 0xc4, 0x52, 0x43, 0x90, 0x05, 0xdb, 0xf1,
    0xee, 0x8d, 0x59, 0x72, 0x6a, 0x7f, 0x54, 0xa1,
    0x64, 0x75, 0x12, 0x36, 0x34, 0xd4, 0x3d, 0x88,
    0x61, 0x24, 0x77, 0x86, 0x89, 0x99, 0xed, 0xda,
    0x33, 0x44, 0x55, 0x9a, 0xda, 0xe4, 0x08, 0x70
};

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
   uint32_t      ui32Status;

   am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);

}

void
MspiTransferCallback(void *pCallbackCtxt, uint32_t status)
{
    g_bDMAFinish = true;
}

//*****************************************************************************
//
// SDHC host setup.
//
//*****************************************************************************
static am_hal_card_host_t *pSdhcCardHost = NULL;
static am_hal_card_t eMMCard;

void
am_sdio_isr(void)
{
    uint32_t ui32IntStatus;

    am_hal_sdhc_intr_status_get(pSdhcCardHost->pHandle, &ui32IntStatus, true);
    am_hal_sdhc_intr_status_clear(pSdhcCardHost->pHandle, ui32IntStatus);
    am_hal_sdhc_interrupt_service(pSdhcCardHost->pHandle, ui32IntStatus);
}

static void emmc_dma_test_init(void)
{
    //
    // Configure SDIO PINs.
    //
    am_bsp_sdio_pins_enable(AM_HAL_HOST_BUS_WIDTH_8);

    //
    // Init the eMMC
    //

    //
    // Get the uderlying SDHC card host instance
    //
    pSdhcCardHost = am_hal_get_card_host(AM_HAL_SDHC_CARD_HOST, true);
    if (pSdhcCardHost == NULL)
    {
        am_util_stdio_printf("No such card host and stop\n");
        TEST_ASSERT_TRUE(false);
    }

//    am_util_stdio_printf("card host is found\n");

    //
    // check if card is present
    //
    if(am_hal_card_host_find_card(pSdhcCardHost, &eMMCard) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("No card is present now\n");
        TEST_ASSERT_TRUE(false);
    }

    if (am_hal_card_init(&eMMCard, AM_HAL_CARD_TYPE_EMMC, NULL, AM_HAL_CARD_PWR_CTRL_SDHC_OFF) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Failed to initialize the card\n");
        TEST_ASSERT_TRUE(false);
    }

    //
    // set the card type to eMMC by using 48MHz and 4-bit mode for read and write
    //
    if (am_hal_card_cfg_set(&eMMCard, AM_HAL_CARD_TYPE_EMMC,
        AM_HAL_HOST_BUS_WIDTH_4, 48000000, AM_HAL_HOST_BUS_VOLTAGE_1_8,
        AM_HAL_HOST_UHS_NONE) != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("Failed to config 48MHz and 4-bit mode\n");
        TEST_ASSERT_TRUE(false);
    }
}

//*****************************************************************************
//
// IOM setup
//
//*****************************************************************************
#define IOM_MOUDLE 3
uint32_t DMATCBBuffer[4*1024];
void            *g_IomDevHdl;
void            *g_pIOMHandle;
bool g_bIOMNonBlockComplete;
uint32_t g_IOMInterruptStatus;

void
pfnIOM_Callback(void *pCtxt, uint32_t ui32Status)
{
    //
    // Set the DMA complete flag.
    //
    g_bIOMNonBlockComplete = (ui32Status ? false : true);
}
//
//! Take over default ISR for IOM 4. (Queue mode service)
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
am_iomaster3_isr(void)
{
    am_iom_isr();
}

static void iom_dma_test_init(void)
{
    uint32_t ui32Status;
    am_devices_spipsram_config_t IOMConfig;
    uint32_t ui32DeviceId = 0;
    uint32_t aui32Rawdata[5] = {0, 0, 0, 0, 0};
    //
    // Enable the interrupt in the NVIC.
    //
    NVIC_ClearPendingIRQ((IRQn_Type)(IOMSTR0_IRQn + IOM_MOUDLE));
    NVIC_EnableIRQ((IRQn_Type)(IOMSTR0_IRQn + IOM_MOUDLE));
    IOMConfig.ui32ClockFreq   = AM_HAL_IOM_24MHZ;
    IOMConfig.pNBTxnBuf = &DMATCBBuffer[0];
    IOMConfig.ui32NBTxnBufLength = sizeof(DMATCBBuffer) / 4;
    IOMConfig.ui32ChipSelectNum = 0;
    ui32Status = am_devices_spipsram_init(IOM_MOUDLE, &IOMConfig, &g_IomDevHdl, &g_pIOMHandle);
    if (ui32Status != AM_DEVICES_SPIPSRAM_STATUS_SUCCESS)
    {
        am_util_stdio_printf("IOM failed to intialize.\n");
        TEST_ASSERT_TRUE(false);
    }
    ui32Status = am_devices_spipsram_read_id(g_IomDevHdl, aui32Rawdata);
    ui32DeviceId = ((aui32Rawdata[0] & 0xFF000000) >> 24) | ((aui32Rawdata[1] & 0xFF) << 8);
    if ((ui32Status != AM_DEVICES_SPIPSRAM_STATUS_SUCCESS) || ( ui32DeviceId != AM_DEVICES_SPIPSRAM_KGD_PASS))
    {
        am_util_stdio_printf("IOM failed to read device ID.\n");
        TEST_ASSERT_TRUE(false);
    }
}

//*****************************************************************************
//
// I2S setup
//
//*****************************************************************************
#define I2S_TEST_TIMEOUT_MS      4000
#define I2S_MODULE               0
static const IRQn_Type i2s_interrupts[] =
{
    I2S0_IRQn
};

uint32_t g_ui32I2SDmaCpl[3] =
{
    0, //Master TX.
    0, //Master RX.
    0  //Success or Fail.
};

//*****************************************************************************
//
// I2S configuration information.
//
//*****************************************************************************
void *I2S0Handle;
static am_hal_i2s_io_signal_t g_sI2SIOConfigMaster =
{
    .eFyncCpol = AM_HAL_I2S_IO_FSYNC_CPOL_HIGH,
    .eTxCpol = AM_HAL_I2S_IO_TX_CPOL_FALLING,
    .eRxCpol = AM_HAL_I2S_IO_RX_CPOL_RISING,
};

static am_hal_i2s_data_format_t g_sI2SDataConfig =
{
    .ePhase = AM_HAL_I2S_DATA_PHASE_SINGLE,
    .ui32ChannelNumbersPhase1 = 2,
    .ui32ChannelNumbersPhase2 = 0,
    .eDataDelay = 0x1,
    .eDataJust = AM_HAL_I2S_DATA_JUSTIFIED_LEFT,
    .eChannelLenPhase1 = AM_HAL_I2S_FRAME_WDLEN_32BITS,
    .eChannelLenPhase2 = AM_HAL_I2S_FRAME_WDLEN_32BITS,
    .eSampleLenPhase1 = AM_HAL_I2S_SAMPLE_LENGTH_32BITS,
    .eSampleLenPhase2 = AM_HAL_I2S_SAMPLE_LENGTH_32BITS
};

static am_hal_i2s_config_t g_sI2S0Config =
{
    .eMode                = AM_HAL_I2S_IO_MODE_MASTER,
    .eXfer                = AM_HAL_I2S_XFER_RXTX,
    .eClock               = eAM_HAL_I2S_CLKSEL_HFRC2_6MHz,
    .eDiv3                = 0,
    .eASRC                = 0,
    .eData                = &g_sI2SDataConfig,
    .eIO                  = &g_sI2SIOConfigMaster,
//    .eTransfer            = &sTransfer0
};

// Transfer setting.
static am_hal_i2s_transfer_t sTransfer0;

static bool check_i2s_data(uint32_t rxtx_size, uint32_t* rx_databuf, uint32_t* tx_databuf)
{
    int i, index_first_word = 0;
    //
    // Rx will delay several samples in fullduplex mode.
    //
    for(i = 0; i < rxtx_size; i++)
    {
        if(rx_databuf[i] == tx_databuf[0])
        {
            index_first_word = i;
            break;
        }
    }

    for(i = 0; i < rxtx_size - index_first_word; i++)
    {
        if(rx_databuf[i + index_first_word] != tx_databuf[i])
        {
            return false;
        }
    }

    return true;
}

//*****************************************************************************
//
// I2S0 interrupt handler.
//
//*****************************************************************************
void
am_dspi2s0_isr()
{
    uint32_t ui32Status;

    am_hal_i2s_interrupt_status_get(I2S0Handle, &ui32Status, true);
    am_hal_i2s_interrupt_clear(I2S0Handle, ui32Status);


    if (ui32Status & AM_HAL_I2S_INT_RXDMACPL)
    {
        uint32_t ui32DmaStat;
        am_hal_i2s_dma_status_get(I2S0Handle, &ui32DmaStat, AM_HAL_I2S_XFER_RX);

        if(ui32DmaStat & AM_HAL_I2S_STAT_DMA_RX_ERR)
        {
          // cleare DMAERR bit
          // ...
          //I2Sn(ui32Module)->RXDMAEN = 0x0;
        }
        g_ui32I2SDmaCpl[1] = 1;
    }

    if (ui32Status & AM_HAL_I2S_INT_TXDMACPL)
    {
        uint32_t ui32DmaStat;
        am_hal_i2s_dma_status_get(I2S0Handle, &ui32DmaStat, AM_HAL_I2S_XFER_TX);

        if(ui32DmaStat & AM_HAL_I2S_STAT_DMA_RX_ERR)
        {
          // cleare DMAERR bit
          // ...
          //I2Sn(ui32Module)->TXDMAEN = 0x0;
        }

        g_ui32I2SDmaCpl[0] = 1;
    }

    if (ui32Status & AM_HAL_I2S_INT_IPB)
    {
      am_hal_i2s_ipb_interrupt_service(I2S0Handle);
    }
}

//
// define I2S SDIN pin here.
//
#define I2S_DATA_IN_GPIO_FUNC    AM_HAL_PIN_4_I2S0_SDIN
#define I2S_DATA_IN_GPIO_PIN     4
#define I2S_DATA_OUT_GPIO_FUNC   AM_HAL_PIN_12_I2S0_SDOUT
#define I2S_DATA_OUT_GPIO_PIN    12
#define I2S_CLK_GPIO_FUNC        AM_HAL_PIN_11_I2S0_CLK
#define I2S_CLK_GPIO_PIN         11
#define I2S_WS_GPIO_FUNC         AM_HAL_PIN_13_I2S0_WS
#define I2S_WS_GPIO_PIN          13

static void  i2s_dma_test_init(void)
{
    //
    // Configure the i2s pins.
    //
    am_hal_gpio_pincfg_t sPinCfg =
    {
        .GP.cfg_b.eGPOutCfg = 1,
        .GP.cfg_b.ePullup   = 0
    };
    sPinCfg.GP.cfg_b.uFuncSel = I2S_DATA_OUT_GPIO_FUNC;
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_gpio_pinconfig(I2S_DATA_OUT_GPIO_PIN, sPinCfg));
    sPinCfg.GP.cfg_b.uFuncSel = I2S_DATA_IN_GPIO_FUNC;
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_gpio_pinconfig(I2S_DATA_IN_GPIO_PIN, sPinCfg));
    sPinCfg.GP.cfg_b.uFuncSel = I2S_CLK_GPIO_FUNC;
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_gpio_pinconfig(I2S_CLK_GPIO_PIN, sPinCfg));
    sPinCfg.GP.cfg_b.uFuncSel = I2S_WS_GPIO_FUNC;
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_gpio_pinconfig(I2S_WS_GPIO_PIN, sPinCfg));
    //
    // enable module power for test
    //
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_i2s_initialize(I2S_MODULE, &I2S0Handle));
    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_i2s_power_control(I2S0Handle, AM_HAL_I2S_POWER_ON, false));

    CLKGEN->MISC |= 0x20; //bit 5.
    am_util_delay_ms(200);

    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_i2s_configure(I2S0Handle, &g_sI2S0Config));
    I2Sn(I2S_MODULE)->CLKCFG_b.MCLKEN = 0x1;
    am_util_delay_ms(100);

}

//*****************************************************************************
//
// PDM setup
//
//*****************************************************************************
#define     PDM_MODULE          0   // PDM module to select
#define     USE_DMA             1   // 0 = use FIFO only, 1 = use DMA

//! PDM interrupts.
static const IRQn_Type pdm_interrupts[] =
{
    PDM0_IRQn
};

//*****************************************************************************
//
// PDM pins
//
//*****************************************************************************
#define PDM_DATA_GPIO_FUNC  AM_HAL_PIN_51_PDM0_DATA
#define PDM_DATA_GPIO_PIN   51
#define PDM_CLK_GPIO_FUNC   AM_HAL_PIN_50_PDM0_CLK
#define PDM_CLK_GPIO_PIN    50

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************
volatile bool g_bPDMDataReady = false;

//*****************************************************************************
//
// PDM configuration information.
//
//*****************************************************************************
void *PDMHandle;

am_hal_pdm_config_t g_sPdmConfig =
{
    //
    // Example setting:
    //  1.5MHz PDM CLK OUT:
    //      AM_HAL_PDM_CLK_HFRC2ADJ_24_576MHZ, AM_HAL_PDM_MCLKDIV_1, AM_HAL_PDM_PDMA_CLKO_DIV7
    //  16.00KHz 24bit Sampling:
    //      DecimationRate = 48
    //
    .ePDMClkSpeed = AM_HAL_PDM_CLK_HFRC2ADJ_24_576MHZ,
    .eClkDivider = AM_HAL_PDM_MCLKDIV_1,
    .ePDMAClkOutDivder = AM_HAL_PDM_PDMA_CLKO_DIV7,
    .ui32DecimationRate = 48,

    .eLeftGain = AM_HAL_PDM_GAIN_P210DB,
    .eRightGain = AM_HAL_PDM_GAIN_P210DB,
    .eStepSize = AM_HAL_PDM_GAIN_STEP_0_13DB,

    .bHighPassEnable = AM_HAL_PDM_HIGH_PASS_ENABLE,
    .ui32HighPassCutoff = 0x3,
    .bDataPacking = 1,
    .ePCMChannels = AM_HAL_PDM_CHANNEL_STEREO,

    .bPDMSampleDelay = AM_HAL_PDM_CLKOUT_PHSDLY_NONE,
    .ui32GainChangeDelay = AM_HAL_PDM_CLKOUT_DELAY_NONE,

    .bSoftMute = 0,
    .bLRSwap = 0,
};

am_hal_pdm_transfer_t sPDMTransfer =
{
    .ui32TargetAddr        = 0x0,
    .ui32TargetAddrReverse = 0x0,
    .ui32TotalCount        = 0x0,
};

//*****************************************************************************
//
// PDM initialization.
//
//*****************************************************************************
static void pdm_dma_test_init(void)
{
    //
    // Initialize, power-up, and configure the PDM.
    //
    am_hal_pdm_initialize(PDM_MODULE, &PDMHandle);
    am_hal_pdm_power_control(PDMHandle, AM_HAL_PDM_POWER_ON, false);

    // use external XTHS, not reference clock
    am_hal_mcuctrl_control_arg_t ctrlArgs = g_amHalMcuctrlArgDefault;
    ctrlArgs.ui32_arg_hfxtal_user_mask  = 1 << (AM_HAL_HCXTAL_PDM_BASE_EN+PDM_MODULE);

    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32M_KICK_START, (void *)&ctrlArgs);

    // enable HFRC2
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFRC2_START, false);
    am_util_delay_us(200);      // wait for FLL to lock

    // set HF2ADJ for 24.576MHz output
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HF2ADJ_ENABLE, false);
    am_util_delay_us(500);      // wait for adj to apply

    am_hal_pdm_configure(PDMHandle, &g_sPdmConfig);

    //
    // Configure the necessary pins.
    //
    am_hal_gpio_pincfg_t sPinCfg = {0};

    sPinCfg.GP.cfg_b.uFuncSel = PDM_DATA_GPIO_FUNC;
    am_hal_gpio_pinconfig(PDM_DATA_GPIO_PIN, sPinCfg);
    sPinCfg.GP.cfg_b.uFuncSel = PDM_CLK_GPIO_FUNC;
    am_hal_gpio_pinconfig(PDM_CLK_GPIO_PIN, sPinCfg);

    //am_hal_pdm_fifo_threshold_setup(PDMHandle, FIFO_THRESHOLD_CNT);

    //
    // Configure and enable PDM interrupts (set up to trigger on DMA
    // completion).
    //
    am_hal_pdm_interrupt_enable(PDMHandle, (AM_HAL_PDM_INT_DERR
                                            | AM_HAL_PDM_INT_DCMP
                                            | AM_HAL_PDM_INT_UNDFL
                                            | AM_HAL_PDM_INT_OVF));

    NVIC_SetPriority(pdm_interrupts[PDM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(pdm_interrupts[PDM_MODULE]);
    am_hal_pdm_enable(PDMHandle);

}
//
// Take over the interrupt handler for whichever PDM we're using.
//
#define example_pdm_isr     am_pdm_isr1(PDM_MODULE)
#define am_pdm_isr1(n)      am_pdm_isr(n)
#define am_pdm_isr(n)       am_pdm ## n ## _isr

//*****************************************************************************
//
// PDM interrupt handler.
//
//*****************************************************************************
void
example_pdm_isr(void)
{
    uint32_t ui32Status;

    //
    // Read the interrupt status.
    //
    am_hal_pdm_interrupt_status_get(PDMHandle, &ui32Status, true);
    am_hal_pdm_interrupt_clear(PDMHandle, ui32Status);

#if USE_DMA
    //
    // DMA transaction
    //
    am_hal_pdm_interrupt_service(PDMHandle, ui32Status, &sPDMTransfer);
    if (ui32Status & AM_HAL_PDM_INT_DCMP)
    {
        g_bPDMDataReady = true;
    }
#else
    if (ui32Status & AM_HAL_PDM_INT_THR)
    {
        g_bPDMDataReady = true;
    }

    if (ui32Status & AM_HAL_PDM_INT_OVF)
    {

        // #### INTERNAL BEGIN ####
        //
        // Overflow handling is not clearing overflow status, need update.
        //
        // #### INTERNAL END ####
        am_hal_pdm_fifo_flush(PDMHandle);
    }
#endif // #if USE_DMA
}

//*****************************************************************************
//
// ADC set up
//
//*****************************************************************************
//
// ADC Global variables.
//
static void                    *g_ADCHandle;
volatile bool                   g_bADCDMAComplete;
volatile bool                   g_bADCDMAError;
volatile bool                   g_bTriggerADC = false;

//
// Define the ADC SE0 pin to be used.
//
const am_hal_gpio_pincfg_t g_AM_PIN_19_ADCSE0 =
{
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_19_ADCSE0,
};

//*****************************************************************************
//
// Interrupt handler for the ADC.
//
//*****************************************************************************
void
am_adc_isr(void)
{
    uint32_t ui32IntMask;
    //
    // Read the interrupt status.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_status(g_ADCHandle, &ui32IntMask, false))
    {
        am_util_stdio_printf("Error reading ADC interrupt status\n");
        TEST_ASSERT_TRUE(false);
    }

    //
    // Clear the ADC interrupt.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_interrupt_clear(g_ADCHandle, ui32IntMask))
    {
        am_util_stdio_printf("Error clearing ADC interrupt status\n");
        TEST_ASSERT_TRUE(false);
    }


    //
    // If we got a DMA complete, set the flag.
    //
    if (ui32IntMask & AM_HAL_ADC_INT_FIFOOVR1)
//        (ui32IntMask & AM_HAL_ADC_INT_CNVCMP)   || (ui32IntMask & AM_HAL_ADC_INT_DCMP) ||
//        (ui32IntMask & AM_HAL_ADC_INT_SCNCMP))
    {
        if ( ADCn(0)->DMASTAT_b.DMACPL )
        {
            g_bADCDMAComplete = true;
        }
    }

    //
    // If we got a DMA error, set the flag.
    //
    if ( ui32IntMask & AM_HAL_ADC_INT_DERR )
    {
        g_bADCDMAError = true;
        am_util_stdio_printf("AUDADC DMA error!\n");
        TEST_ASSERT_TRUE(false);
    }
}

static void adc_dma_test_init(void)
{
    am_hal_adc_config_t           ADCConfig;
    am_hal_adc_slot_config_t      ADCSlotConfig;

    //
    // Set a pin to act as our ADC input
    //
    am_hal_gpio_pinconfig(19, g_AM_PIN_19_ADCSE0);
    //
    // Initialize the ADC and get the handle.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_adc_initialize(0, &g_ADCHandle) )
    {
        am_util_stdio_printf("Error - reservation of the ADC instance failed.\n");
        TEST_ASSERT_TRUE(false);
    }
    //
    // Power on the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
                                                          AM_HAL_SYSCTRL_WAKE,
                                                          false) )
    {
        am_util_stdio_printf("Error - ADC power on failed.\n");
        TEST_ASSERT_TRUE(false);
    }

    //
    // Set up internal repeat trigger timer
    //
    am_hal_adc_irtt_config_t      ADCIrttConfig =
    {
        .bIrttEnable        = true,
        .eClkDiv            = AM_HAL_ADC_RPTT_CLK_DIV2, // 48MHz
        .ui32IrttCountMax   = 20,   // 48MHz / 20 = 2.4MHz
    };

    am_hal_adc_configure_irtt(g_ADCHandle, &ADCIrttConfig);

    //
    // Set up the ADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    //
#if 1 // As of Apr 2022, only 24MHz clock is available for GP ADC on Apollo4
    ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC_24MHZ;
#else
    ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC;
#endif
    ADCConfig.ePolarity          = AM_HAL_ADC_TRIGPOL_RISING;
    ADCConfig.eTrigger           = AM_HAL_ADC_TRIGSEL_SOFTWARE;
    ADCConfig.eClockMode         = AM_HAL_ADC_CLKMODE_LOW_LATENCY;
    ADCConfig.ePowerMode         = AM_HAL_ADC_LPMODE0;
    ADCConfig.eRepeat            = AM_HAL_ADC_REPEATING_SCAN;

    ADCConfig.eRepeatTrigger     = AM_HAL_ADC_RPTTRIGSEL_INT;

    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure(g_ADCHandle, &ADCConfig))
    {
        am_util_stdio_printf("Error - configuring ADC failed.\n");
        TEST_ASSERT_TRUE(false);
    }

    //
    // Set up an ADC slot
    //
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_128;
    ADCSlotConfig.ui32TrkCyc      = AM_HAL_ADC_MIN_TRKCYC;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_12BIT;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE0;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 0, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 0 failed.\n");
        TEST_ASSERT_TRUE(false);
    }

    //
    // Configure the ADC to use DMA for the sample transfer.
    //
    //adc_config_dma();

    //
    // For this example, the samples will be coming in slowly. This means we
    // can afford to wake up for every conversion.
    //
    am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_FIFOOVR1 | AM_HAL_ADC_INT_DERR | AM_HAL_ADC_INT_DCMP); // AM_HAL_ADC_INT_CNVCMP | AM_HAL_ADC_INT_SCNCMP |

    //
    // Enable the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_enable(g_ADCHandle))
    {
        am_util_stdio_printf("Error - enabling ADC failed.\n");
        TEST_ASSERT_TRUE(false);
    }

    //
    // Enable internal repeat trigger timer
    //
    am_hal_adc_irtt_enable(g_ADCHandle);
    //
    // Enable interrupts.
    //
    NVIC_SetPriority(ADC_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(ADC_IRQn);

}
//*****************************************************************************
//
// Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{
}

void
tearDown(void)
{
}

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{
    uint32_t ui32Status;
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();
    //
    // Init DAXI/cache test settings
    //
    daxi_test_cfg_init();
    cache_test_cfg_init();
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    //
    // Print the banner.
    //
    am_util_stdio_printf("\n\nApollo4l cross memory boundaries test cases\n\n");
    //
    // just for test
    //
    if ((g_ui8ConstData[0] != 0x26) || (g_ui8ConstData1[0] != 0x26) || (g_ui8ConstData2[0] != 0x27))
    {
        TEST_FAIL();
        am_util_stdio_printf("Mram const array init failed, current data are 0x%02X 0x%02X 0x%02X\n", g_ui8ConstData[0], g_ui8ConstData1[0], g_ui8ConstData2[0]);
    }
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DISPCLKSEL_HFRC96, NULL);
// #### INTERNAL BEGIN ####
    //
    // FALCSW-727 - No DC in Apollo4 Lite
    //
    //am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_DCCLK_ENABLE, NULL);
// #### INTERNAL END ####
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
//    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }

    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);
    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
//  Function
//  Calculate checksum
//
//*****************************************************************************
uint32_t
checksum_calculate(uint32_t * pui32StartAddr, uint32_t ui32Size)
{
    uint32_t i, ui32CheckSum = 0;
    uint32_t ui32High16Bits, ui32Low16Bits;
    for (i = 0; i < ui32Size; i++)
    {
        ui32High16Bits = (((*pui32StartAddr) & 0xFFFF0000) >> 16);
        ui32Low16Bits = ((*pui32StartAddr) & 0x0000FFFF);
        ui32CheckSum += (ui32High16Bits + ui32Low16Bits);
        pui32StartAddr++;
    }
    do
    {
        ui32High16Bits = ((ui32CheckSum & 0xFFFF0000) >> 16);
        ui32Low16Bits = (ui32CheckSum & 0x0000FFFF);
        ui32CheckSum = ui32High16Bits + ui32Low16Bits;
    } while (ui32CheckSum > 0x0000FFFF);
    ui32CheckSum = ((~ui32CheckSum) & 0x0000FFFF);
    return ui32CheckSum;
}

//*****************************************************************************
//
//  Function
//  memory copy by different masters
//
//*****************************************************************************
bool
memory_copy(am_access_master_e eMaster, am_memory_type_e eSrcMem, am_memory_type_e eDestMem, uint32_t ui32BuffSizeBytes)
{
    uint32_t ui32SrcBoundAddr, ui32SrcOffset, ui32SrcStartAddr;
    uint32_t ui32DestBoundAddr, ui32DestOffset, ui32DestStartAddr;
    uint32_t ui32BuffAddr;
    uint32_t ui32RandNum;
    uint32_t ui32RefCheckSum, ui32SrcCheckSum, ui32DestCheckSum;
    uint32_t ui32RefBufferAddr;
    uint32_t i, ui32Status, ui32msDelay = 0, ui32msMaxDelay = 2000;
    uint32_t ui32PsramAddress = 0x14000000;
    bool bTestPass = true;
    am_data_size_e eDataSize;

    for (ui32SrcBoundAddr = sMemCfg[eSrcMem].ui32StartAddr + sMemCfg[eSrcMem].ui32BlockSize;
         ui32SrcBoundAddr < sMemCfg[eSrcMem].ui32StartAddr + sMemCfg[eSrcMem].ui32Size + sMemCfg[eSrcMem].bFollowedByMem;
         ui32SrcBoundAddr += sMemCfg[eSrcMem].ui32BlockSize)
    {
        for (ui32SrcOffset = 0; ui32SrcOffset <= 16; ui32SrcOffset++)
        {
            ui32SrcStartAddr = ui32SrcBoundAddr - ui32SrcOffset;
            if (eSrcMem != MRAM)
            {
                //
                // Load data to source memory
                //
                ui32RefBufferAddr = TCM_ALIGNED_ADDR_WITHOUT_BOUNDARY;
                for (ui32BuffAddr = ui32SrcStartAddr; ui32BuffAddr < ui32SrcStartAddr + ui32BuffSizeBytes; ui32BuffAddr += 4)
                {
                    ui32RandNum = rand();
                    *(uint32_t *)ui32BuffAddr = ui32RandNum;
                    *(uint32_t *)ui32RefBufferAddr = ui32RandNum;
                    ui32RefBufferAddr += 4;
                }
                //
                // Calculate source buffer checksum
                //
                ui32RefCheckSum = checksum_calculate((uint32_t *)TCM_ALIGNED_ADDR_WITHOUT_BOUNDARY, ui32BuffSizeBytes / 4);
                ui32SrcCheckSum = checksum_calculate((uint32_t *)ui32SrcStartAddr, ui32BuffSizeBytes / 4);
                if(ui32RefCheckSum != ui32SrcCheckSum)
                {
                    am_util_stdio_printf("-----ERROR!----- Source buffer(at address 0x%08X) checksum and reference checksum did not match!\n", ui32SrcStartAddr);
                    bTestPass = false;
                }
            }
            else
            {
                ui32SrcCheckSum = checksum_calculate((uint32_t *)ui32SrcStartAddr, ui32BuffSizeBytes / 4);
            }
            for (ui32DestBoundAddr = sMemCfg[eDestMem].ui32StartAddr + sMemCfg[eDestMem].ui32BlockSize;
                 ui32DestBoundAddr < sMemCfg[eDestMem].ui32StartAddr + sMemCfg[eDestMem].ui32Size + sMemCfg[eDestMem].bFollowedByMem;
                 ui32DestBoundAddr += sMemCfg[eDestMem].ui32BlockSize)
            {
                if (ui32SrcBoundAddr == ui32DestBoundAddr)
                {
                    am_util_stdio_printf("Skipped memcpy between the same regions!\n");
                }
                else
                {
                    for (ui32DestOffset = 0; ui32DestOffset <= 16; ui32DestOffset++)
                    {
                        ui32DestStartAddr = ui32DestBoundAddr - ui32DestOffset;
#ifdef SKIP_BONDARY_BTW_DIFF_MEM
                        if ((eMaster != APBDMA_SDIO) && (eMaster != CPU_MEMCPY) && (eMaster != GPU_BLIT))
                        {
                            if ((ui32DestBoundAddr == 0x10060000) || (ui32SrcBoundAddr == 0x10060000))
                            {
                                am_util_stdio_printf("Skipped boundary between MCU_TCM and SSRAM0.\n");
                                break;
                            }
                        }
#endif
                        if (eMaster == CPU_MEMCPY)
                        {
                            //
                            // Execute memcpy
                            //
                            for (eDataSize = BYTE; eDataSize < NUM_DATA_SIZE; eDataSize++)
                            {
                                switch (eDataSize)
                                {
                                    case BYTE:
                                        for (i = 0; i < ui32BuffSizeBytes; i += 1)
                                        {
                                            *(uint8_t *)(ui32DestStartAddr + i) = *(uint8_t *)(ui32SrcStartAddr + i);
                                        }
                                        break;
                                    case HALF_WORD:
                                        for (i = 0; i < ui32BuffSizeBytes; i += 2)
                                        {
                                            *(uint16_t *)(ui32DestStartAddr + i) = *(uint16_t *)(ui32SrcStartAddr + i);
                                        }
                                        break;
                                    case WORD:
                                        for (i = 0; i < ui32BuffSizeBytes; i += 4)
                                        {
                                            *(uint32_t *)(ui32DestStartAddr + i) = *(uint32_t *)(ui32SrcStartAddr + i);
                                        }
                                        break;
                                    default:
                                        am_util_stdio_printf("-----ERROR!----- Invalid data type!\n");
                                        bTestPass = false;
                                        break;
                                }
                                //
                                // Calculate destination buffer checksum
                                //
                                ui32DestCheckSum = checksum_calculate((uint32_t *)ui32DestStartAddr, ui32BuffSizeBytes / 4);
                                //
                                // Compare checksum
                                //
                                uint32_t ui32DataSize = (1 << (uint32_t)eDataSize);
                                if(ui32DestCheckSum != ui32SrcCheckSum)
                                {
                                    am_util_stdio_printf("-----ERROR!----- Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum did not match! Copied %d byte(s) per step.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32DataSize);
                                    bTestPass = false;
                                }
                                else
                                {
                                    //am_util_stdio_printf("Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum matched! Copied %d byte(s) per step.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32DataSize);
                                }
                            }
                        }
                        else if (eMaster == GPU_BLIT)
                        {
                            nema_memcpy ((void *)ui32DestStartAddr, (void const*)ui32SrcStartAddr, ui32BuffSizeBytes);
                            //
                            // Calculate destination buffer checksum
                            //
                            ui32DestCheckSum = checksum_calculate((uint32_t *)ui32DestStartAddr, ui32BuffSizeBytes / 4);
                            //
                            // Compare checksum
                            //
                            if(ui32DestCheckSum != ui32SrcCheckSum)
                            {
                                am_util_stdio_printf("-----ERROR!----- Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum did not match! Buffer size is %d Bytes.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32BuffSizeBytes);
                                bTestPass = false;
                            }
                            else
                            {
                                //am_util_stdio_printf("Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum matched! Buffer size is %d Bytes.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32BuffSizeBytes);
                            }
                        }
                        else if (eMaster == APBDMA_MSPI)
                        {
                            memset((void *)ui32PsramAddress, 0 , ui32BuffSizeBytes);
                            g_bDMAFinish = false;
                            ui32Status = am_devices_mspi_psram_aps25616n_ddr_nonblocking_write(g_pPsramHandle,
                                                                                               (uint8_t *)ui32SrcStartAddr,
                                                                                               ui32PsramAddress - sMemCfg[XIPMM0].ui32StartAddr,
                                                                                               ui32BuffSizeBytes,
                                                                                               MspiTransferCallback,
                                                                                               NULL);
                            if (AM_HAL_STATUS_SUCCESS != ui32Status)
                            {
                                am_util_stdio_printf("ERROR! MSPI non blocking write failed!\n");
                                bTestPass = false;
                            }
                            ui32msDelay = 0;
                            while(g_bDMAFinish == false)
                            {
                                ui32msDelay++;
                                am_util_delay_ms(1);
                                if(ui32msDelay > ui32msMaxDelay)
                                {
                                    am_util_stdio_printf("ERROR! MSPI non blocking write time out!\n");
                                    bTestPass = false;
                                    break;
                                }
                            }
                            g_bDMAFinish = false;
                            ui32Status = am_devices_mspi_psram_aps25616n_ddr_nonblocking_read(g_pPsramHandle,
                                                                                              (uint8_t *)ui32DestStartAddr,
                                                                                              ui32PsramAddress - sMemCfg[XIPMM0].ui32StartAddr,
                                                                                              ui32BuffSizeBytes,
                                                                                              MspiTransferCallback,
                                                                                              NULL);
                            if (AM_HAL_STATUS_SUCCESS != ui32Status)
                            {
                                am_util_stdio_printf("ERROR! MSPI non blocking read failed!\n");
                                bTestPass = false;
                            }
                            ui32msDelay = 0;
                            while(g_bDMAFinish == false)
                            {
                                ui32msDelay++;
                                am_util_delay_ms(1);
                                if(ui32msDelay > ui32msMaxDelay)
                                {
                                    am_util_stdio_printf("ERROR! MSPI non blocking read time out!\n");
                                    bTestPass = false;
                                    break;
                                }
                            }
                            //
                            // Calculate destination buffer checksum
                            //
                            ui32DestCheckSum = checksum_calculate((uint32_t *)ui32DestStartAddr, ui32BuffSizeBytes / 4);
                            //
                            // Compare checksum
                            //
                            if(ui32DestCheckSum != ui32SrcCheckSum)
                            {
                                am_util_stdio_printf("-----ERROR!----- Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum did not match! Buffer size is %d Bytes.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32BuffSizeBytes);
                                bTestPass = false;
                            }
                            else
                            {
                                //am_util_stdio_printf("Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum matched! Buffer size is %d Bytes.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32BuffSizeBytes);
                            }
                        }
                        else if (eMaster == APBDMA_SDIO)
                        {
                            //
                            // erase emmc card
                            //
                            if ((am_hal_card_block_erase(&eMMCard, 0x0, 1, AM_HAL_ERASE, 100) & 0xFFFF) != AM_HAL_STATUS_SUCCESS)
                            {
                                am_util_stdio_printf("eMMC erase failed\n");
                                bTestPass = false;
                            }
                            if ((am_hal_card_block_write_sync(&eMMCard, 0x0, 1, (uint8_t *)ui32SrcStartAddr) & 0xFFFF) != AM_HAL_STATUS_SUCCESS)
                            {
                                am_util_stdio_printf("eMMC sync block write failed\n");
                                bTestPass = false;
                            }
                            for (i = 0; i < ui32BuffSizeBytes; i += 4)
                            {
                                *(volatile uint32_t *)(ui32DestStartAddr + i) = 0x0;
                            }
                            if ((am_hal_card_block_read_sync(&eMMCard, 0x0, 1, (uint8_t *)ui32DestStartAddr) & 0xFFFF) != AM_HAL_STATUS_SUCCESS)
                            {
                                am_util_stdio_printf("eMMC sync block read failed\n");
                                bTestPass = false;
                            }
                            //
                            // Calculate destination buffer checksum
                            //
                            ui32DestCheckSum = checksum_calculate((uint32_t *)ui32DestStartAddr, ui32BuffSizeBytes / 4);
                            //
                            // Compare checksum
                            //
                            if(ui32DestCheckSum != ui32SrcCheckSum)
                            {
                                am_util_stdio_printf("-----ERROR!----- Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum did not match! Buffer size is %d Bytes.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32BuffSizeBytes);
                                bTestPass = false;
                            }
                            else
                            {
                                //am_util_stdio_printf("Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum matched! Buffer size is %d Bytes.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32BuffSizeBytes);
                            }
                        }
                        else if (eMaster == APBDMA_IOM)
                        {
                            g_bIOMNonBlockComplete = false;
                            if (am_devices_spipsram_nonblocking_write(g_IomDevHdl, (uint8_t *)ui32SrcStartAddr, 0, ui32BuffSizeBytes, pfnIOM_Callback, 0) != AM_DEVICES_SPIPSRAM_STATUS_SUCCESS)
                            {
                                am_util_stdio_printf("IOM PSRAM non-blocking write failed\n");
                                bTestPass = false;
                            }
                            ui32msDelay = 0;
                            while(g_bIOMNonBlockComplete == false)
                            {
                                ui32msDelay++;
                                am_util_delay_ms(1);
                                if(ui32msDelay > ui32msMaxDelay)
                                {
                                    am_util_stdio_printf("ERROR! IOM non blocking write time out!\n");
                                    bTestPass = false;
                                    break;
                                }
                            }
                            g_bIOMNonBlockComplete = false;
                            for (i = 0; i < ui32BuffSizeBytes; i += 4)
                            {
                                *(volatile uint32_t *)(ui32DestStartAddr + i) = 0x0;
                            }
                            if (am_devices_spipsram_nonblocking_read(g_IomDevHdl, (uint8_t *)ui32DestStartAddr, 0, ui32BuffSizeBytes, pfnIOM_Callback, 0) != AM_DEVICES_SPIPSRAM_STATUS_SUCCESS)
                            {
                                am_util_stdio_printf("IOM PSRAM non-blocking read failed\n");
                                bTestPass = false;
                            }
                            ui32msDelay = 0;
                            while(g_bIOMNonBlockComplete == false)
                            {
                                ui32msDelay++;
                                am_util_delay_ms(1);
                                if(ui32msDelay > ui32msMaxDelay)
                                {
                                    am_util_stdio_printf("ERROR! IOM non blocking read time out!\n");
                                    bTestPass = false;
                                    break;
                                }
                            }
                            //
                            // Calculate destination buffer checksum
                            //
                            ui32DestCheckSum = checksum_calculate((uint32_t *)ui32DestStartAddr, ui32BuffSizeBytes / 4);
                            //
                            // Compare checksum
                            //
                            if(ui32DestCheckSum != ui32SrcCheckSum)
                            {
                                am_util_stdio_printf("-----ERROR!----- Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum did not match! Buffer size is %d Bytes.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32BuffSizeBytes);
                                bTestPass = false;
                            }
                            else
                            {
                                //am_util_stdio_printf("Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum matched! Buffer size is %d Bytes.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32BuffSizeBytes);
                            }
                        }
                        else if (eMaster == APBDMA_I2S)
                        {
                            sTransfer0.ui32RxTargetAddr = ui32DestStartAddr;
                            sTransfer0.ui32TxTargetAddr = ui32SrcStartAddr;
                            sTransfer0.ui32RxTotalCount = ui32BuffSizeBytes / 4;
                            sTransfer0.ui32TxTotalCount = ui32BuffSizeBytes / 4;
                            TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_i2s_dma_configure(I2S0Handle, &g_sI2S0Config, &sTransfer0));
                            NVIC_EnableIRQ(i2s_interrupts[I2S_MODULE]);
                            am_hal_interrupt_master_enable();
                            // start transfer
                            TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_i2s_dma_transfer_start(I2S0Handle, &g_sI2S0Config));
                            //
                            // Loop forever while sleeping.
                            //
                            uint32_t timeout_counter_ms = I2S_TEST_TIMEOUT_MS;
                            while (1)
                            {
                                if (g_ui32I2SDmaCpl[1] && g_ui32I2SDmaCpl[0])
                                {
                                    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_i2s_dma_transfer_complete(I2S0Handle));

                                    I2Sn(0)->I2SCTL = I2S0_I2SCTL_TXRST_Msk | I2S0_I2SCTL_RXRST_Msk;

                                    NVIC_DisableIRQ(i2s_interrupts[I2S_MODULE]);
                                    am_util_delay_us(200);
                                    if(check_i2s_data(ui32BuffSizeBytes / 4, (uint32_t *)ui32DestStartAddr, (uint32_t *)ui32SrcStartAddr))
                                    {
                                        g_ui32I2SDmaCpl[2] = 1;
//                                        am_util_stdio_printf("Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum matched! Buffer size is %d Bytes.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32BuffSizeBytes);
//                                        am_util_stdio_printf(" 0x%08X -- 0x%08X checksum matched!\n", ui32SrcStartAddr, ui32DestStartAddr);
                                    }
                                    else
                                    {
                                        am_util_stdio_printf("-----ERROR!----- Source buffer(at address 0x%08X) checksum and destination buffer(at address 0x%08X) checksum did not match! Buffer size is %d Bytes.\n", ui32SrcStartAddr, ui32DestStartAddr, ui32BuffSizeBytes);
                                        TEST_ASSERT_TRUE(false);
                                    }

                                    g_ui32I2SDmaCpl[2] = g_ui32I2SDmaCpl[1] = g_ui32I2SDmaCpl[0] = 0;

                                    //TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_i2s_power_control(I2S0Handle, AM_HAL_I2S_POWER_OFF, false));
                                    break;  // end current test
                                }

                                am_util_delay_us(1000);
                                timeout_counter_ms --;
                                if(timeout_counter_ms == 0)
                                {
                                    I2Sn(0)->I2SCTL = I2S0_I2SCTL_TXRST_Msk | I2S0_I2SCTL_RXRST_Msk;

                                    NVIC_DisableIRQ(i2s_interrupts[I2S_MODULE]);

                                    TEST_ASSERT_TRUE(AM_HAL_STATUS_SUCCESS == am_hal_i2s_power_control(I2S0Handle, AM_HAL_I2S_POWER_OFF, false));

                                    am_util_stdio_printf("i2s dma test FAILED! --> Timeout after %d ms\n", I2S_TEST_TIMEOUT_MS);
                                    TEST_ASSERT_TRUE(false);
                                    break;
                                }
                            }
                        }
                        else
                        {
                            am_util_stdio_printf("-----ERROR!----- Unknown master!\n");
                            bTestPass = false;
                        }
                    }
                }
            }
        }
    }
    return bTestPass;
}

//*****************************************************************************
//
//  Function
//  crc_calculation_by_memory
//
//*****************************************************************************
bool
security_crc_calculation(am_access_master_e eMaster, am_memory_type_e eSrcMem, uint32_t ui32BuffSizeBytes)
{
    uint32_t ui32SrcBoundAddr, ui32SrcOffset, ui32SrcStartAddr;
    uint32_t ui32BuffAddr;
    uint32_t ui32RandNum;
    uint32_t ui32RefCRC, ui32CurCRC, ui32CRC;
    uint32_t ui32RefBufferAddr;
    uint32_t ui32Status, ui32TimeOut;
    bool bTestPass = true;

    for (ui32SrcBoundAddr = sMemCfg[eSrcMem].ui32StartAddr + sMemCfg[eSrcMem].ui32BlockSize;
         ui32SrcBoundAddr < sMemCfg[eSrcMem].ui32StartAddr + sMemCfg[eSrcMem].ui32Size + sMemCfg[eSrcMem].bFollowedByMem;
         ui32SrcBoundAddr += sMemCfg[eSrcMem].ui32BlockSize)
    {
#ifdef SKIP_BONDARY_BTW_DIFF_MEM
        if (ui32SrcBoundAddr == 0x10060000)
        {
            am_util_stdio_printf("Skipped boundary between MCU_TCM and SSRAM0.\n");
            break;
        }
#endif
        for (ui32SrcOffset = 0; ui32SrcOffset <= 16; ui32SrcOffset++)
        {
            ui32SrcStartAddr = ui32SrcBoundAddr - ui32SrcOffset;
            ui32RefBufferAddr = TCM_ALIGNED_ADDR_WITHOUT_BOUNDARY;
            if (eSrcMem != MRAM)
            {
                //
                // Load data to source memory
                //
                for (ui32BuffAddr = ui32SrcStartAddr; ui32BuffAddr < ui32SrcStartAddr + ui32BuffSizeBytes; ui32BuffAddr += 4)
                {
                    ui32RandNum = rand();
                    *(uint32_t *)ui32BuffAddr = ui32RandNum;
                    *(uint32_t *)ui32RefBufferAddr = ui32RandNum;
                    ui32RefBufferAddr += 4;
                }

            }
            else
            {
                //
                // Load data from MRAM memory to aligned reference memory
                //
                for (ui32BuffAddr = ui32SrcStartAddr; ui32BuffAddr < ui32SrcStartAddr + ui32BuffSizeBytes; ui32BuffAddr += 4)
                {
                    *(uint32_t *)ui32RefBufferAddr = *(uint32_t *)ui32BuffAddr;
                    ui32RefBufferAddr += 4;
                }
            }
            ui32RefBufferAddr = TCM_ALIGNED_ADDR_WITHOUT_BOUNDARY;
            //
            // Calculate current CRC
            //
            ui32Status = am_hal_crc32(ui32SrcStartAddr, ui32BuffSizeBytes, &ui32CRC);
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("am_hal_crc32 returned exceptional status.\n");
                bTestPass = false;
            }
            ui32TimeOut = 500;
            while( (ui32TimeOut > 0) && SECURITY->CTRL_b.ENABLE)
            {
                am_hal_delay_us(100);
                ui32TimeOut--;
            }

            if(ui32TimeOut == 0)
            {
                am_util_stdio_printf("CRC timed out\n");
                bTestPass = false;
            }

            if(SECURITY->CTRL & SECURITY_CTRL_CRCERROR_Msk)
            {
                am_util_stdio_printf("ERROR: Saw unexpected CRC engine DMA error!\n");
                bTestPass = false;
            }
            ui32CurCRC = ui32CRC;
            //
            // Calculate reference CRC
            //
            ui32Status = am_hal_crc32(ui32RefBufferAddr, ui32BuffSizeBytes, &ui32CRC);
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("am_hal_crc32 returned exceptional status.\n");
                bTestPass = false;
            }
            ui32TimeOut = 500;
            while( (ui32TimeOut > 0) && SECURITY->CTRL_b.ENABLE)
            {
                am_hal_delay_us(100);
                ui32TimeOut--;
            }

            if(ui32TimeOut == 0)
            {
                am_util_stdio_printf("Reference CRC calculation timed out\n");
                bTestPass = false;
            }

            if(SECURITY->CTRL & SECURITY_CTRL_CRCERROR_Msk)
            {
                am_util_stdio_printf("ERROR: Saw unexpected CRC engine DMA error when calculating reference CRC!\n");
                bTestPass = false;
            }

            ui32RefCRC = ui32CRC;
            if(ui32RefCRC != ui32CurCRC)
            {
                am_util_stdio_printf("-----ERROR!----- Source buffer(at address 0x%08X) CRC and reference CRC did not match!\n", ui32SrcStartAddr);
                bTestPass = false;
            }
            else
            {
                //am_util_stdio_printf("Source buffer(at address 0x%08X) CRC and reference CRC matched!\n", ui32SrcStartAddr);
            }
        }
    }
    return bTestPass;
}

//*****************************************************************************
//
//  Function
//  store data to memories by different masters
//
//*****************************************************************************
bool
dma_store_data_into_memory(am_access_master_e eMaster, am_memory_type_e eDestMem, uint32_t ui32BuffSizeBytes)
{
    uint32_t ui32DestBoundAddr, ui32DestOffset, ui32DestStartAddr;
    uint32_t ui32Status, ui32msDelay = 0, ui32msMaxDelay = 2000;
    am_hal_adc_dma_config_t       ADCDMAConfig;
    bool bTestPass = true;

    for (ui32DestBoundAddr = sMemCfg[eDestMem].ui32StartAddr + sMemCfg[eDestMem].ui32BlockSize;
         ui32DestBoundAddr < sMemCfg[eDestMem].ui32StartAddr + sMemCfg[eDestMem].ui32Size + sMemCfg[eDestMem].bFollowedByMem;
         ui32DestBoundAddr += sMemCfg[eDestMem].ui32BlockSize)
    {
        am_util_stdio_printf("Testing boundary 0x%08X...\n", ui32DestBoundAddr);
        for (ui32DestOffset = 0; ui32DestOffset <= 16; ui32DestOffset++)
        {
            ui32DestStartAddr = ui32DestBoundAddr - ui32DestOffset;
            if (eMaster == APBDMA_PDM)
            {
                sPDMTransfer.ui32TargetAddr = ui32DestStartAddr;
                sPDMTransfer.ui32TargetAddrReverse = ui32DestStartAddr;
                sPDMTransfer.ui32TotalCount = ui32BuffSizeBytes;
                memset((void *)sPDMTransfer.ui32TargetAddr, 0, sPDMTransfer.ui32TotalCount);
                g_bPDMDataReady = false;
                //
                // Start data conversion
                //
                ui32Status = am_hal_pdm_dma_start(PDMHandle, &sPDMTransfer);
                if (AM_HAL_STATUS_SUCCESS != ui32Status)
                {
                    am_util_stdio_printf("ERROR! PDM non blocking transfer failed!\n");
                    bTestPass = false;
                }
                ui32msDelay = 0;
                while (g_bPDMDataReady == false)
                {
                    ui32msDelay++;
                    am_util_delay_ms(1);
                    if(ui32msDelay > ui32msMaxDelay)
                    {
                        am_util_stdio_printf("ERROR! PDM non blocking transfer time out!\n");
                        bTestPass = false;
                        break;
                    }
                }
                PDMn(0)->DMACFG_b.DMAEN = PDM0_DMACFG_DMAEN_DIS;
            }
            else if (eMaster == APBDMA_ADC)
            {
                //
                // Configure the ADC to use DMA for the sample transfer.
                //
                ADCDMAConfig.bDynamicPriority = true;
                ADCDMAConfig.ePriority = AM_HAL_ADC_PRIOR_SERVICE_IMMED;
                ADCDMAConfig.bDMAEnable = true;
                ADCDMAConfig.ui32SampleCount = ui32BuffSizeBytes;
                ADCDMAConfig.ui32TargetAddress = (uint32_t)ui32DestStartAddr;
                if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_dma(g_ADCHandle, &ADCDMAConfig))
                {
                    am_util_stdio_printf("Error - configuring ADC DMA failed.\n");
                }
                if (g_bTriggerADC == false)
                {
                    //
                    // Trigger the ADC sampling for the first time manually.
                    //
                    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_sw_trigger(g_ADCHandle))
                    {
                        am_util_stdio_printf("Error - triggering the ADC failed.\n");
                        bTestPass = false;
                        break;
                    }
                    g_bTriggerADC = true;
                }
                //
                // Reset the ADC DMA flags.
                //
                g_bADCDMAComplete = false;
                g_bADCDMAError = false;
                //
                // Clear the ADC interrupts.
                //
                am_hal_adc_interrupt_clear(g_ADCHandle, 0xFFFFFFFF);
                ui32msDelay = 0;
                while((!g_bADCDMAComplete) && (ui32msDelay < ui32msMaxDelay))
                {
                    am_util_delay_ms(1);
                    ui32msDelay ++;
                }

                if(ui32msDelay >= ui32msMaxDelay)
                {
                    am_util_stdio_printf("ERROR! ADC non blocking transfer time out!\n");
                    bTestPass = false;
                    break;
                }
            }
            else
            {
                am_util_stdio_printf("-----ERROR!----- Unknown master!\n");
                bTestPass = false;
            }
        }
    }
    return bTestPass;
}

//*****************************************************************************
//
//  Test Case
//  cm4_memcpy_cross_memory_test
//
//*****************************************************************************
void
cm4_memcpy_cross_internal_memory_test(void)
{
    uint32_t ui32BuffSizeBytes = 32;
    bool bTestPass = true, bPass = true;
    am_memory_type_e eSrcMem;
    am_memory_type_e eDestMem;
    am_access_master_e eMaster = CPU_MEMCPY;
    uint32_t ui32DaxiCfg, ui32CacheCfg;

    am_util_stdio_printf("cm4_memcpy_cross_memory_test...\n");

#if defined(CACHE_TEST) || defined(DAXI_TEST)
    uint32_t i, j, ui32Status;
#endif
#ifdef CACHE_TEST
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
#if defined(CACHE_TEST) || defined(DAXI_TEST)
            am_util_stdio_printf("\n\nLoop %d - %d...\n", j, i);
#endif
            ui32DaxiCfg = CPU->DAXICFG;
            ui32CacheCfg = CPU->CACHECFG;
            am_util_stdio_printf("Current cache setting is 0x%08X, DAXI setting is 0x%08X.\n", ui32CacheCfg, ui32DaxiCfg);
            for (eSrcMem = MRAM; eSrcMem < NUM_MEM; eSrcMem++)
            {
                for (eDestMem = MCU_TCM; eDestMem < NUM_MEM; eDestMem++)
                {
#ifdef HPLP_TEST_EN
                    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
                    {
                        TEST_FAIL();
                        am_util_stdio_printf("Failed to enter HP mode!\n");
                        break;
                    }
                    else
                    {
                        am_util_stdio_printf("Entered HP mode!\n");
                    }
                    bPass = memory_copy(eMaster, eSrcMem, eDestMem, ui32BuffSizeBytes);
                    if (bPass == false)
                    {
                        am_util_stdio_printf("cm4_memcpy_cross_internal_memory_test failed at HP mode!\n");
                        bTestPass = false;
                    }

                    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER) != AM_HAL_STATUS_SUCCESS )
                    {
                        TEST_FAIL();
                        am_util_stdio_printf("Failed to enter LP mode!\n");
                        break;
                    }
                    else
                    {
                        am_util_stdio_printf("Entered LP mode!\n");
                    }
#endif
                    bPass = memory_copy(eMaster, eSrcMem, eDestMem, ui32BuffSizeBytes);
                    if (bPass == false)
                    {
                        am_util_stdio_printf("cm4_memcpy_cross_internal_memory_test failed at LP mode!\n");
                        bTestPass = false;
                    }
                }
            }
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    TEST_ASSERT_TRUE(bTestPass);

    return;
} // cm4_memcpy_cross_internal_memory_test

//*****************************************************************************
//
//  Test Case
//  gpu_blit_cross_memory_test
//
//*****************************************************************************
void
gpu_blit_cross_internal_memory_test(void)
{
    uint32_t ui32BuffSizeBytes = 4096;
    bool bTestPass = true, bPass = true;
    am_memory_type_e eSrcMem;
    am_memory_type_e eDestMem;
    am_access_master_e eMaster = GPU_BLIT;
    uint32_t ui32DaxiCfg, ui32CacheCfg;

    //
    // Initialize NemaGFX
    //
    if(nema_init() != 0)
    {
        am_util_stdio_printf("GPU init failed!\n");
    }
    
    am_util_stdio_printf("gpu_blit_cross_memory_test...\n");
#if defined(SIMPLIFIED_TEST) && defined(MULTIPLE_GPU_BUFFER_SIZE)
    uint32_t k;
#endif
#if defined(CACHE_TEST) || defined(DAXI_TEST)
    uint32_t i, j, ui32Status;
#endif
#ifdef CACHE_TEST
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
#if defined(CACHE_TEST) || defined(DAXI_TEST)
            am_util_stdio_printf("\n\nLoop %d - %d...\n", j, i);
#endif
            ui32DaxiCfg = CPU->DAXICFG;
            ui32CacheCfg = CPU->CACHECFG;
            am_util_stdio_printf("Current cache setting is 0x%08X, DAXI setting is 0x%08X.\n", ui32CacheCfg, ui32DaxiCfg);
            for (eSrcMem = SSRAM0; eSrcMem < NUM_MEM; eSrcMem++)
            {
                for (eDestMem = SSRAM0; eDestMem < NUM_MEM; eDestMem++)
                {
                    ui32BuffSizeBytes = 4096;
#if defined(SIMPLIFIED_TEST) && defined(MULTIPLE_GPU_BUFFER_SIZE) //!< only simpified test supports larger buffer zise.
                    for (k = 0; k < 4; k++)
                    {
#endif
#ifndef SIMPLIFIED_TEST  //!< APS25616n page size is 2048 bytes. Set buffer size to 2000 Bytes to avoid memory access address overflow.
                        if ((eSrcMem == XIPMM0) || (eDestMem == XIPMM0))
                        {
                            ui32BuffSizeBytes = 2000;
                        }
#endif
                        bPass = memory_copy(eMaster, eSrcMem, eDestMem, ui32BuffSizeBytes);
                        if (bPass == false)
                        {
                            am_util_stdio_printf("gpu_blit_cross_internal_memory_test failed with %d Bytes buffer size!\n", ui32BuffSizeBytes);
                            bTestPass = false;
                        }
#if defined(SIMPLIFIED_TEST) && defined(MULTIPLE_GPU_BUFFER_SIZE)
                        ui32BuffSizeBytes <<= 1;
                    }
#endif
                }
            }
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    TEST_ASSERT_TRUE(bTestPass);

    return;
} // gpu_blit_cross_internal_memory_test

//*****************************************************************************
//
//  Test Case
//  apbdma_mspi_cross_memory_test
//
//*****************************************************************************
void
apbdma_mspi_cross_memory_test(void)
{
    uint32_t ui32BuffSizeBytes = 4096;
    bool bTestPass = true, bPass = true;
    am_memory_type_e eSrcMem;
    am_memory_type_e eDestMem;
    am_access_master_e eMaster = APBDMA_MSPI;
    uint32_t ui32DaxiCfg, ui32CacheCfg;

    am_util_stdio_printf("apbdma_mspi_cross_memory_test...\n");
#if defined(CACHE_TEST) || defined(DAXI_TEST)
    uint32_t i, j, ui32Status;
#endif
#ifdef CACHE_TEST
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
#if defined(CACHE_TEST) || defined(DAXI_TEST)
            am_util_stdio_printf("\n\nLoop %d - %d...\n", j, i);
#endif
            ui32DaxiCfg = CPU->DAXICFG;
            ui32CacheCfg = CPU->CACHECFG;
            am_util_stdio_printf("Current cache setting is 0x%08X, DAXI setting is 0x%08X.\n", ui32CacheCfg, ui32DaxiCfg);
            for (eSrcMem = MRAM; eSrcMem < XIPMM0; eSrcMem++)
            {
                for (eDestMem = MCU_TCM; eDestMem < XIPMM0; eDestMem++)
                {
                    ui32BuffSizeBytes = 4096;
#ifndef SIMPLIFIED_TEST  //!< APS25616n page size is 2048 bytes. Set buffer size to 2000 Bytes to avoid memory access address overflow.
                    if ((eSrcMem == XIPMM0) || (eDestMem == XIPMM0))
                    {
                        ui32BuffSizeBytes = 2000;
                    }
#endif
                    bPass = memory_copy(eMaster, eSrcMem, eDestMem, ui32BuffSizeBytes);
                    if (bPass == false)
                    {
                        am_util_stdio_printf("apbdma_mspi_cross_memory_test failed with %d Bytes buffer size!\n", ui32BuffSizeBytes);
                        bTestPass = false;
                    }
                }
            }
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    TEST_ASSERT_TRUE(bTestPass);

    return;
} // apbdma_mspi_cross_memory_test

//*****************************************************************************
//
//  Test Case
//  apbdma_sdio_cross_memory_test
//
//*****************************************************************************
void
apbdma_sdio_cross_memory_test(void)
{
    uint32_t ui32BuffSizeBytes = 512;
    bool bTestPass = true, bPass = true;
    am_memory_type_e eSrcMem;
    am_memory_type_e eDestMem;
    am_access_master_e eMaster = APBDMA_SDIO;
    uint32_t ui32DaxiCfg, ui32CacheCfg;

    //
    // Initialize sdio
    //
    emmc_dma_test_init();
    
    am_util_stdio_printf("apbdma_sdio_cross_memory_test...\n");
#if defined(CACHE_TEST) || defined(DAXI_TEST)
    uint32_t i, j, ui32Status;
#endif
#ifdef CACHE_TEST
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
#if defined(CACHE_TEST) || defined(DAXI_TEST)
            am_util_stdio_printf("\n\nLoop %d - %d...\n", j, i);
#endif
            ui32DaxiCfg = CPU->DAXICFG;
            ui32CacheCfg = CPU->CACHECFG;
            am_util_stdio_printf("Current cache setting is 0x%08X, DAXI setting is 0x%08X.\n", ui32CacheCfg, ui32DaxiCfg);
            for (eSrcMem = MRAM; eSrcMem < NUM_MEM; eSrcMem++)
            {
                for (eDestMem = MCU_TCM; eDestMem < NUM_MEM; eDestMem++)
                {
                    ui32BuffSizeBytes = 512; // emmmc block size is 512 bytes.
                    bPass = memory_copy(eMaster, eSrcMem, eDestMem, ui32BuffSizeBytes);
                    if (bPass == false)
                    {
                        am_util_stdio_printf("apbdma_sdio_cross_memory_test failed with %d Bytes buffer size!\n", ui32BuffSizeBytes);
                        bTestPass = false;
                    }
                }
            }
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    TEST_ASSERT_TRUE(bTestPass);

    return;
} // apbdma_sdio_cross_memory_test

//*****************************************************************************
//
//  Test Case
//  apbdma_iom_cross_memory_test
//
//*****************************************************************************
void
apbdma_iom_cross_memory_test(void)
{
    uint32_t ui32BuffSizeBytes = 4096;
    bool bTestPass = true, bPass = true;
    am_memory_type_e eSrcMem;
    am_memory_type_e eDestMem;
    am_access_master_e eMaster = APBDMA_IOM;
    uint32_t ui32DaxiCfg, ui32CacheCfg;

    //
    // Initialize IOM
    //
    iom_dma_test_init();
    
    am_util_stdio_printf("apbdma_iom_cross_memory_test...\n");
#if defined(CACHE_TEST) || defined(DAXI_TEST)
    uint32_t i, j, ui32Status;
#endif
#ifdef CACHE_TEST
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
#if defined(CACHE_TEST) || defined(DAXI_TEST)
            am_util_stdio_printf("\n\nLoop %d - %d...\n", j, i);
#endif
            ui32DaxiCfg = CPU->DAXICFG;
            ui32CacheCfg = CPU->CACHECFG;
            am_util_stdio_printf("Current cache setting is 0x%08X, DAXI setting is 0x%08X.\n", ui32CacheCfg, ui32DaxiCfg);
            for (eSrcMem = MRAM; eSrcMem < NUM_MEM; eSrcMem++)
            {
                for (eDestMem = MCU_TCM; eDestMem < NUM_MEM; eDestMem++)
                {
                    ui32BuffSizeBytes = 4096;
#ifndef SIMPLIFIED_TEST  //!< APS25616n page size is 2048 bytes. Set buffer size to 2000 Bytes to avoid memory access address overflow.
                    if ((eSrcMem == XIPMM0) || (eDestMem == XIPMM0))
                    {
                        ui32BuffSizeBytes = 2000;
                    }
#endif
                    bPass = memory_copy(eMaster, eSrcMem, eDestMem, ui32BuffSizeBytes);
                    if (bPass == false)
                    {
                        am_util_stdio_printf("apbdma_iom_cross_memory_test failed with %d Bytes buffer size!\n", ui32BuffSizeBytes);
                        bTestPass = false;
                    }
                }
            }
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    TEST_ASSERT_TRUE(bTestPass);

    return;
} // apbdma_iom_cross_memory_test

//*****************************************************************************
//
//  Test Case
//  apbdma_i2s_cross_memory_test
//
//*****************************************************************************
void
apbdma_i2s_cross_memory_test(void)
{
    uint32_t ui32BuffSizeBytes = 4096;
    bool bTestPass = true, bPass = true;
    am_memory_type_e eSrcMem;
    am_memory_type_e eDestMem;
    am_access_master_e eMaster = APBDMA_I2S;
    uint32_t ui32DaxiCfg, ui32CacheCfg;

    //
    // Initialize I2S
    //
    i2s_dma_test_init();
    
    am_util_stdio_printf("apbdma_i2s_cross_memory_test...\n");
#if defined(CACHE_TEST) || defined(DAXI_TEST)
    uint32_t i, j, ui32Status;
#endif
#ifdef CACHE_TEST
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
#if defined(CACHE_TEST) || defined(DAXI_TEST)
            am_util_stdio_printf("\n\nLoop %d - %d...\n", j, i);
#endif
            ui32DaxiCfg = CPU->DAXICFG;
            ui32CacheCfg = CPU->CACHECFG;
            am_util_stdio_printf("Current cache setting is 0x%08X, DAXI setting is 0x%08X.\n", ui32CacheCfg, ui32DaxiCfg);
            for (eSrcMem = MRAM; eSrcMem < NUM_MEM; eSrcMem++)
            {
                for (eDestMem = MCU_TCM; eDestMem < NUM_MEM; eDestMem++)
                {
                    ui32BuffSizeBytes = 4096;
#ifndef SIMPLIFIED_TEST  //!< APS25616n page size is 2048 bytes. Set buffer size to 2000 Bytes to avoid memory access address overflow.
                    if ((eSrcMem == XIPMM0) || (eDestMem == XIPMM0))
                    {
                        ui32BuffSizeBytes = 2000;
                    }
#endif
                    bPass = memory_copy(eMaster, eSrcMem, eDestMem, ui32BuffSizeBytes);
                    if (bPass == false)
                    {
                        am_util_stdio_printf("apbdma_i2s_cross_memory_test failed with %d Bytes buffer size!\n", ui32BuffSizeBytes);
                        bTestPass = false;
                    }
                }
            }
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    TEST_ASSERT_TRUE(bTestPass);

    return;
} // apbdma_i2s_cross_memory_test

//*****************************************************************************
//
//  Test Case
//  apbdma_crc_cross_memory_test
//
//*****************************************************************************
void
apbdma_crc_cross_memory_test(void)
{
    uint32_t ui32BuffSizeBytes = 4096;
    bool bTestPass = true, bPass = true;
    am_memory_type_e eSrcMem;
    am_access_master_e eMaster = APBDMA_CRC;
    uint32_t ui32DaxiCfg, ui32CacheCfg;

    am_util_stdio_printf("apbdma_crc_cross_memory_test...\n");
#if defined(CACHE_TEST) || defined(DAXI_TEST)
    uint32_t i, j, ui32Status;
#endif
#ifdef CACHE_TEST
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
#if defined(CACHE_TEST) || defined(DAXI_TEST)
            am_util_stdio_printf("\n\nLoop %d - %d...\n", j, i);
#endif
            ui32DaxiCfg = CPU->DAXICFG;
            ui32CacheCfg = CPU->CACHECFG;
            am_util_stdio_printf("Current cache setting is 0x%08X, DAXI setting is 0x%08X.\n", ui32CacheCfg, ui32DaxiCfg);
            for (eSrcMem = MRAM; eSrcMem < NUM_MEM; eSrcMem++)
            {
                ui32BuffSizeBytes = 4096;
#ifndef SIMPLIFIED_TEST  //!< APS25616n page size is 2048 bytes. Set buffer size to 2000 Bytes to avoid memory access address overflow.
                if ((eSrcMem == XIPMM0) || (eDestMem == XIPMM0))
                {
                    ui32BuffSizeBytes = 2000;
                }
#endif
                bPass = security_crc_calculation(eMaster, eSrcMem, ui32BuffSizeBytes);
                if (bPass == false)
                {
                    am_util_stdio_printf("apbdma_crc_cross_memory_test failed!\n");
                    bTestPass = false;
                }
            }
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    TEST_ASSERT_TRUE(bTestPass);

    return;
} // apbdma_crc_cross_memory_test

//*****************************************************************************
//
//  Test Case
//  apbdma_pdm_cross_memory_test
//
//*****************************************************************************
void
apbdma_pdm_cross_memory_test(void)
{
    uint32_t ui32BuffSizeBytes = 4096;
    bool bTestPass = true, bPass = true;
    am_memory_type_e eDestMem;
    am_access_master_e eMaster = APBDMA_PDM;
    uint32_t ui32DaxiCfg, ui32CacheCfg;

    //
    // Initialize PDM
    //
    pdm_dma_test_init();
    
    am_util_stdio_printf("apbdma_pdm_cross_memory_test...\n");
    am_util_stdio_printf("Only tested if PDM DMA hang at boundary, did not check data in RX buffer.\n");
#if defined(CACHE_TEST) || defined(DAXI_TEST)
    uint32_t i, j, ui32Status;
#endif
#ifdef CACHE_TEST
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
#if defined(CACHE_TEST) || defined(DAXI_TEST)
            am_util_stdio_printf("\n\nLoop %d - %d...\n", j, i);
#endif
            ui32DaxiCfg = CPU->DAXICFG;
            ui32CacheCfg = CPU->CACHECFG;
            am_util_stdio_printf("Current cache setting is 0x%08X, DAXI setting is 0x%08X.\n", ui32CacheCfg, ui32DaxiCfg);
            for (eDestMem = MCU_TCM; eDestMem < NUM_MEM; eDestMem++)
            {
                ui32BuffSizeBytes = 4096;
#ifndef SIMPLIFIED_TEST  //!< APS25616n page size is 2048 bytes. Set buffer size to 2000 Bytes to avoid memory access address overflow.
                if ((eSrcMem == XIPMM0) || (eDestMem == XIPMM0))
                {
                    ui32BuffSizeBytes = 2000;
                }
#endif
                bPass = dma_store_data_into_memory(eMaster, eDestMem, ui32BuffSizeBytes);
                if (bPass == false)
                {
                    am_util_stdio_printf("apbdma_pdm_cross_memory_test failed with %d Bytes buffer size!\n", ui32BuffSizeBytes);
                    bTestPass = false;
                }
            }
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    TEST_ASSERT_TRUE(bTestPass);

    return;
} // apbdma_pdm_cross_memory_test

#ifndef APOLLO4_FPGA
//*****************************************************************************
//
//  Test Case
//  apbdma_adc_cross_memory_test
//
//*****************************************************************************
void
apbdma_adc_cross_memory_test(void)
{
    uint32_t ui32BuffSizeBytes = 4096;
    bool bTestPass = true, bPass = true;
    am_memory_type_e eDestMem;
    am_access_master_e eMaster = APBDMA_ADC;
    uint32_t ui32DaxiCfg, ui32CacheCfg;

    //
    // Initialize adc
    //
    adc_dma_test_init();
    
    am_util_stdio_printf("apbdma_adc_cross_memory_test...\n");
#if defined(CACHE_TEST) || defined(DAXI_TEST)
    uint32_t i, j, ui32Status;
#endif
#ifdef CACHE_TEST
    for (j = 0; j < sizeof(g_sCacheCfg) / sizeof(cache_enable_config_t); j++)
    {
        if (g_sCacheCfg[j].bCacheEnable == false)
        {
            ui32Status = am_hal_cachectrl_disable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to disable cache!");
            }
        }
        else
        {
            ui32Status = am_hal_cachectrl_config(&(g_sCacheCfg[j].sCacheCtrl));
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to configure cache!");
            }
            ui32Status = am_hal_cachectrl_enable();
            if (ui32Status != AM_HAL_STATUS_SUCCESS)
            {
                am_util_stdio_printf("Failed to enable cache!");
            }
        }
#endif
#ifdef DAXI_TEST
        for (i = 0; i < sizeof(g_sDaxiCfg) / sizeof(am_hal_daxi_config_t); i++)
        {
            if (g_sDaxiCfg[i].bDaxiPassThrough == true)
            {
                ui32Status = am_hal_daxi_control(AM_HAL_DAXI_CONTROL_DISABLE, NULL);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to disable DAXI!");
                }
            }
            else
            {
                ui32Status = am_hal_daxi_config(&g_sDaxiCfg[i]);
                if (ui32Status != AM_HAL_STATUS_SUCCESS)
                {
                    am_util_stdio_printf("Failed to configure DAXI!");
                }
            }
#endif
#if defined(CACHE_TEST) || defined(DAXI_TEST)
            am_util_stdio_printf("\n\nLoop %d - %d...\n", j, i);
#endif
            ui32DaxiCfg = CPU->DAXICFG;
            ui32CacheCfg = CPU->CACHECFG;
            am_util_stdio_printf("Current cache setting is 0x%08X, DAXI setting is 0x%08X.\n", ui32CacheCfg, ui32DaxiCfg);
            for (eDestMem = MCU_TCM; eDestMem < NUM_MEM; eDestMem++)
            {
                ui32BuffSizeBytes = 4096;
#ifndef SIMPLIFIED_TEST  //!< APS25616n page size is 2048 bytes. Set buffer size to 2000 Bytes to avoid memory access address overflow.
                if ((eSrcMem == XIPMM0) || (eDestMem == XIPMM0))
                {
                    ui32BuffSizeBytes = 2000;
                }
#endif
                bPass = dma_store_data_into_memory(eMaster, eDestMem, ui32BuffSizeBytes);
                if (bPass == false)
                {
                    am_util_stdio_printf("apbdma_adc_cross_memory_test failed with %d Bytes buffer size!\n", ui32BuffSizeBytes);
                    bTestPass = false;
                }
            }
#ifdef DAXI_TEST
        }
#endif
#ifdef CACHE_TEST
    }
#endif

    TEST_ASSERT_TRUE(bTestPass);

    return;
} // apbdma_adc_cross_memory_test
#endif

