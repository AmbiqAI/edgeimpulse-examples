//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "mspi_mspi_display.h"

//*****************************************************************************
//
// Composition Settings
//
//*****************************************************************************
#define BACKGROUND_COLOR          0x80 // 0x00
#define BAND_COLOR                0xfc // white
#define COLOR_MAX                 0xfc
#define BAND_WIDTH                16

//*****************************************************************************
//
// Color Manipulation Macros
//
//*****************************************************************************
// Extract color from a 4p word
#define COL(word, pix)            (((word) >> ((pix) << 3)) & COLOR_MAX)
#define WRAP_COL(num, max)        (((num) > (max)) ? 0 : (num))
// Increment color
#define INC_COL(color, inc)       WRAP_COL((color) + (inc))
// 4 pixel worth of data
#define COLOR_4P(color)           ((color) | ((color) << 8) | ((color) << 16) | ((color) << 24))
// Increment color for each pixel
#define INC_COL_4P(w, inc)        (INC_COL(COL((w), 0), (inc)) | (INC_COL(COL((w), 1), (inc)) << 8) | (INC_COL(COL((w), 2), (inc)) << 16) | (INC_COL(COL((w), 3), (inc)) << 24))

//*****************************************************************************
//
// Composition Fragments
//
//*****************************************************************************
#define COMPOSE_FRAGMENT_SIZE     ((TEMP_BUFFER_SIZE / COLUMN_NUM) * COLUMN_NUM)
#define NUM_ROW_PER_FRAGMENT      (COMPOSE_FRAGMENT_SIZE/COLUMN_NUM)
#define NUM_COMPOSE_FRAGMENTS     ((FRAME_SIZE + COMPOSE_FRAGMENT_SIZE - 1)/COMPOSE_FRAGMENT_SIZE)

#ifdef IMAGE_BMP
// Pre-constructed Image(s)
extern const unsigned char g_ambiq_logo_bmp[];
extern const unsigned char g_ambiq_logo_bmp0[];
extern const unsigned char g_ambiq_logo_bmp1[];
extern const unsigned char g_ambiq_logo_bmp2[];
uint8_t                    *img0 = (uint8_t *)g_ambiq_logo_bmp0;
uint8_t                    *img1 = (uint8_t *)g_ambiq_logo_bmp2;
#endif

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
volatile bool   g_bNewDisplay = false;
volatile bool   g_bNewFB = false;
void            *g_PsramHandle;
void            *g_MSPIPsramHandle;


//*****************************************************************************
//
// Local Variables
//
//*****************************************************************************
static uint32_t         g_RxFrag[2][COMPOSE_FRAGMENT_SIZE / 4];
static uint32_t         g_TxFrag[COMPOSE_FRAGMENT_SIZE / 4];
// Composition States: n is NUM_COMPOSE_FRAGMENTS
// 0: Initial State - Read two fragments
// 1: Compose frag # 0, Read Frag # 1
// 2: Compose frag # 1, Read Frag # 2
// 3: Compose frag # 2, Read Frag # 3
// 4: Compose frag # 3, Read Frag # 4
// n-1: Compose frag # n-1, Read Frag # 0
static uint32_t        g_ComposeState = 0;
static uint32_t        g_numCompose = 0;

// Buffer for non-blocking transactions for Source MSPI - Needs to be big enough to accomodate
// all the transactions
static uint32_t        g_MspiPsramQBuffer[(AM_HAL_MSPI_CQ_ENTRY_SIZE / 4) * (NUM_FRAGMENTS + 1) * 2];
// Memory for hiprio transactions
// Need to use hiprio transactions to insert in between a running sequence
static uint8_t         g_MspiHiPrioBuf[(AM_HAL_MSPI_HIPRIO_ENTRY_SIZE) * 128]; // We write two lines - which could be split into 4 transactions


//*****************************************************************************
//
// PSRAM MSPI Configuration
//
//*****************************************************************************
am_devices_mspi_psram_config_t MSPI_PSRAM_SerialCE0MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_24MHZ,
    .ui32NBTxnBufLength       = sizeof(g_MspiPsramQBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_MspiPsramQBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

am_devices_mspi_psram_config_t MSPI_PSRAM_QuadCE0MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_QUAD_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_24MHZ,
    .ui32NBTxnBufLength       = sizeof(g_MspiPsramQBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_MspiPsramQBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type mspi_psram_interrupts[] =
{
    MSPI0_IRQn,
#if defined(AM_PART_APOLLO3P)
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(PSRAM_MSPI_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
    uint32_t      ui32Status;

    am_hal_mspi_interrupt_status_get(g_MSPIPsramHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPIPsramHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPIPsramHandle, ui32Status);
}

// Initialization
uint32_t
psram_init(void)
{
    uint32_t ui32Status;
    // Set up the MSPI configuraton for the N25Q256A part.
    ui32Status = am_devices_mspi_psram_init(PSRAM_MSPI_MODULE,
                                            &MSPI_PSRAM_QuadCE0MSPIConfig,
                                            &g_PsramHandle,
                                            &g_MSPIPsramHandle);
    NVIC_EnableIRQ(mspi_psram_interrupts[PSRAM_MSPI_MODULE]);
    am_hal_interrupt_master_enable();
    am_hal_mspi_hiprio_cfg_t mspiHiprioCfg;
    mspiHiprioCfg.pBuf = g_MspiHiPrioBuf;
    mspiHiprioCfg.size = sizeof(g_MspiHiPrioBuf);
    am_hal_mspi_control(g_MSPIPsramHandle, AM_HAL_MSPI_REQ_INIT_HIPRIO, &mspiHiprioCfg);
    return ui32Status;
}

uint32_t
psram_data_init(void)
{
    uint32_t      ui32Status;

    DEBUG_PRINT("Writing a known pattern to psram!\n");
#ifdef IMAGE_BMP
    ui32Status = am_devices_mspi_psram_write(g_PsramHandle, img0, PSRAM_ACTFB1_OFFSET, FRAME_SIZE, true);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        DEBUG_PRINT("Failed to write buffer to PSRAM Device!\n");
        return -1;
    }
    ui32Status = am_devices_mspi_psram_write(g_PsramHandle, img1, PSRAM_ACTFB2_OFFSET, FRAME_SIZE, true);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        DEBUG_PRINT("Failed to write buffer to PSRAM Device!\n");
        return -1;
    }
    g_bNewDisplay = true;
#else
    uint32_t i, j;
    //
    // Generate raw color data into PSRAM frame buffer 1
    //
    for (i = 0; i < NUM_ROW_PER_FRAGMENT; i++)
    {
        uint32_t rowStart = i*(COLUMN_NUM / 4);
        for (j = 0; j < (COLUMN_NUM / 4); j++)
        {
            if (j < BAND_WIDTH / 4)
            {
                g_RxFrag[0][rowStart + j] = COLOR_4P(BAND_COLOR);
            }
            else
            {
                g_RxFrag[0][rowStart + j] = COLOR_4P(0xFF);
            }
            if (j >= (COLUMN_NUM - BAND_WIDTH) / 4)
            {
                g_RxFrag[1][rowStart + j] = COLOR_4P(COLOR_MAX ^ BAND_COLOR);
            }
            else
            {
                g_RxFrag[1][rowStart + j] = COLOR_4P(0xFF);
            }
        }
    }

    // Initialize Src Buf 1
    for (uint32_t address = 0; address < FRAME_SIZE; address += COMPOSE_FRAGMENT_SIZE)
    {
        uint32_t size = ((address + COMPOSE_FRAGMENT_SIZE) > FRAME_SIZE) ? (FRAME_SIZE - address) : COMPOSE_FRAGMENT_SIZE;
        //
        // Write the buffer into the target address in PSRAM
        //
        ui32Status = am_devices_mspi_psram_write(g_PsramHandle, (uint8_t *)g_RxFrag[0], PSRAM_SRCFB1_OFFSET + address, size, true);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to write buffer to PSRAM Device!\n");
            return -1;
        }
    }

    // Initialize Src Buf 2
    for (uint32_t address = 0; address < FRAME_SIZE; address += COMPOSE_FRAGMENT_SIZE)
    {
        uint32_t size = ((address + COMPOSE_FRAGMENT_SIZE) > FRAME_SIZE) ? (FRAME_SIZE - address) : COMPOSE_FRAGMENT_SIZE;
        //
        // Write the buffer into the target address in PSRAM
        //
        ui32Status = am_devices_mspi_psram_write(g_PsramHandle, (uint8_t *)g_RxFrag[1], PSRAM_SRCFB2_OFFSET + address, size, true);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to write buffer to PSRAM Device!\n");
            return -1;
        }
    }
    // Initialize ActFB1
    for (uint32_t address = 0; address < FRAME_SIZE; address += COMPOSE_FRAGMENT_SIZE)
    {
        uint32_t size = ((address + COMPOSE_FRAGMENT_SIZE) > FRAME_SIZE) ? (FRAME_SIZE - address) : COMPOSE_FRAGMENT_SIZE;
        //
        // Write the buffer into the target address in PSRAM
        //
        ui32Status = am_devices_mspi_psram_write(g_PsramHandle, (uint8_t *)g_RxFrag[0], PSRAM_ACTFB1_OFFSET + address, size, true);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            DEBUG_PRINT("Failed to write buffer to PSRAM Device!\n");
            return -1;
        }
    }
    g_bNewFB = true;
    g_ComposeState = 1;
#endif
    return 0;
}

// Composition
static void
composeFragment(uint32_t *pSrcBuf1, uint32_t *pSrcBuf2, uint32_t *pDestBuf, uint32_t numRows)
{
    uint32_t i, j;
    // Compute the output FB fragment
    for (i = 0; i < numRows; i++)
    {
        uint32_t rowStart = i*(COLUMN_NUM / 4);
        for (j = 0; j < (COLUMN_NUM / 4); j++)
        {
            if (pSrcBuf2[rowStart + (j + g_numCompose) % (COLUMN_NUM / 4)] != COLOR_4P(0xFF))
            {
                if (pSrcBuf1[rowStart + ((j + (COLUMN_NUM / 4) - g_numCompose % (COLUMN_NUM / 4)) % (COLUMN_NUM / 4))] != COLOR_4P(0xFF))
                {
                    // Overlap region
                    pDestBuf[rowStart + j] = COLOR_4P(COLOR_MAX ^ (g_numCompose % COLOR_MAX));
                }
                else
                {
                    // Only Src2
                    pDestBuf[rowStart + j] = pSrcBuf2[rowStart + (j + g_numCompose) % (COLUMN_NUM / 4)];
                }
            }
            else
            {
                if (pSrcBuf1[rowStart + ((j + (COLUMN_NUM / 4) - g_numCompose % (COLUMN_NUM / 4)) % (COLUMN_NUM / 4))] != COLOR_4P(0xFF))
                {
                    // Only Src1
                    pDestBuf[rowStart + j] = pSrcBuf1[rowStart + ((j + (COLUMN_NUM / 4) - g_numCompose % (COLUMN_NUM / 4)) % (COLUMN_NUM / 4))];
                }
                else
                {
                    // None
                    pDestBuf[rowStart + j] = COLOR_4P(g_numCompose % COLOR_MAX);
                }
            }
        }
    }
}

// Frag complete callback
static void
frag_complete(void *pCallbackCtxt, uint32_t transactionStatus)
{

    if (transactionStatus != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("\nCompose# %d:Fragment %d Failed 0x%x\n", g_numCompose, g_ComposeState, transactionStatus);
    }
    else
    {

        DEBUG_PRINT_SUCCESS("\nCompose# %d:Fragment %d Done 0x%x\n", g_numCompose, g_ComposeState, transactionStatus);
        g_ComposeState++;
    }
}

// Simulate the composition task using a state machine
uint32_t
compose(void)
{
    static uint32_t lastState = 0;
    uint32_t ui32Status;
    uint32_t *pSrcBuf1;
    uint32_t *pSrcBuf2;
    uint32_t *pDestBuf;

    uint32_t actFbOffset = (g_numCompose & 0x1) ? PSRAM_ACTFB2_OFFSET: PSRAM_ACTFB1_OFFSET;


    pSrcBuf1 = g_RxFrag[0];
    pSrcBuf2 = g_RxFrag[1];

    pDestBuf = (uint32_t *)g_TxFrag;

    if ((g_ComposeState != lastState) && g_bNewFB)
    {
        composeFragment(pSrcBuf1, pSrcBuf2, pDestBuf, NUM_ROW_PER_FRAGMENT);

        // Write the fragment to next frame buffer
        ui32Status = am_devices_mspi_psram_write_hiprio(g_PsramHandle, (uint8_t *)g_TxFrag, actFbOffset + lastState*COMPOSE_FRAGMENT_SIZE, COMPOSE_FRAGMENT_SIZE, NULL, 0);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to write buffer to PSRAM Device!\n");
            return ui32Status;
        }

        if (g_ComposeState == NUM_COMPOSE_FRAGMENTS)
        {
            // This FB is ready - now we can start for the next one
            g_ComposeState = 0;
        }
        lastState = g_ComposeState;
        // Read the two base fragments
        ui32Status = am_devices_mspi_psram_read_hiprio(g_PsramHandle, (uint8_t *)pSrcBuf2, PSRAM_SRCFB2_OFFSET + g_ComposeState*COMPOSE_FRAGMENT_SIZE, COMPOSE_FRAGMENT_SIZE, NULL, 0);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
            return ui32Status;
        }
        ui32Status = am_devices_mspi_psram_read_hiprio(g_PsramHandle, (uint8_t *)pSrcBuf1, PSRAM_SRCFB1_OFFSET + g_ComposeState*COMPOSE_FRAGMENT_SIZE, COMPOSE_FRAGMENT_SIZE, frag_complete, 0);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
            return ui32Status;
        }
        if (g_ComposeState == 0)
        {
            DEBUG_PRINT("Done composing ActFB @ 0x%x\n", actFbOffset);
            g_bNewFB = false;
            g_numCompose++;
            // Notify main task that the frame is ready
            g_bNewDisplay = true;
        }
    }
    return 0;
}
