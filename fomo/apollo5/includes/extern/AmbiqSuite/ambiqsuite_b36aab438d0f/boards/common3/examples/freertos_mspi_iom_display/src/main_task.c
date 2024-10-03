//*****************************************************************************
//
//! @file main_task.c
//!
//! @brief Task to handle main operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// Global includes for this project.
//
//*****************************************************************************
#include "freertos_mspi_iom_display.h"

// Position independent prime function
#define SZ_PRIME_MPI    98
const unsigned char Kc_PRIME_MPI[SZ_PRIME_MPI] =
{
0x70, 0xB4, 0x04, 0x46,  0x00, 0x20, 0x89, 0x1C,  0x8C, 0x42, 0x28, 0xDB,  0x45, 0x1C, 0x02, 0x26,
0x8E, 0x42, 0x20, 0xDA,  0x91, 0xFB, 0xF6, 0xF3,  0x06, 0xFB, 0x13, 0x13,  0xD3, 0xB1, 0x76, 0x1C,
0x8E, 0x42, 0x18, 0xDA,  0x91, 0xFB, 0xF6, 0xF3,  0x06, 0xFB, 0x13, 0x13,  0x93, 0xB1, 0x76, 0x1C,
0x8E, 0x42, 0x10, 0xDA,  0x91, 0xFB, 0xF6, 0xF3,  0x06, 0xFB, 0x13, 0x13,  0x53, 0xB1, 0x76, 0x1C,
0x8E, 0x42, 0x08, 0xDA,  0x91, 0xFB, 0xF6, 0xF3,  0x06, 0xFB, 0x13, 0x13,  0x00, 0x2B, 0x18, 0xBF,
0x76, 0x1C, 0xDD, 0xD1,  0x05, 0x46, 0x51, 0x18,  0x8C, 0x42, 0x28, 0x46,  0xD6, 0xDA, 0x70, 0xBC,
0x70, 0x47,
};



//*****************************************************************************
//
// Composition task handle.
//
//*****************************************************************************
TaskHandle_t main_task_handle;

//*****************************************************************************
//
// Handle for Compose-related events.
//
//*****************************************************************************
EventGroupHandle_t xMainEventHandle;

#ifdef ENABLE_XIP
TimerHandle_t xTimerXIP;
#endif
#ifdef ENABLE_XIPMM
TimerHandle_t xTimerXIPMM;
#endif

// MSPI HAL can be accessed both from Render and Compose. So we need to use a mutex to ensure HAL is not re-entered
// This is technically only needed if the RTOS is configured for pre-emptive scheduling
SemaphoreHandle_t xMSPIMutex;

#define AM_DISPLAY_GPIO_PWR_ENABLE    33

#define MSPI_TEST_MODULE              0

const am_hal_gpio_pincfg_t g_AM_DISPLAY_GPIO_ENABLE =
{
    .uFuncSel            = AM_HAL_PIN_33_GPIO,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,
    .eGPOutcfg           = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
};

// Globals
#ifndef CQ_RAW
// Buffer for non-blocking transactions for IOM - Needs to be big enough to accomodate
// all the transactions
uint32_t        g_IomQBuffer[(AM_HAL_IOM_CQ_ENTRY_SIZE / 4) * (NUM_FRAGMENTS + 1)];
// Buffer for non-blocking transactions for MSPI - Needs to be big enough to accomodate
// all the transactions
uint32_t        g_MspiQBuffer[(AM_HAL_MSPI_CQ_ENTRY_SIZE / 4) * (NUM_FRAGMENTS + 1)];
#else
// Buffer for non-blocking transactions for IOM - can be much smaller as the CQ is preconstructed in a separare memory
// all the transactions
uint32_t        g_IomQBuffer[(AM_HAL_IOM_CQ_ENTRY_SIZE / 4) * (2 + 1)];
// Buffer for non-blocking transactions for MSPI - can be much smaller as the CQ is preconstructed in a separare memory
// all the transactions
uint32_t        g_MspiQBuffer[(AM_HAL_MSPI_CQ_ENTRY_SIZE / 4) * (4 + 1)];
#endif

void            *g_MSPIDevHdl;
void            *g_MSPIHdl;
void            *g_IomDevHdl;
void            *g_IOMHandle;
uint32_t        g_numIter = 0;

am_devices_mspi_psram_config_t MSPI_PSRAM_SerialCE0MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .eClockFreq               = MSPI_FREQ,
    .ui32NBTxnBufLength       = sizeof(g_MspiQBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_MspiQBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

am_devices_mspi_psram_config_t MSPI_PSRAM_QuadCE0MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_QUAD_CE0,
    .eClockFreq               = MSPI_FREQ,
    .ui32NBTxnBufLength       = sizeof(g_MspiQBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_MspiQBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

display_device_func_t display_func =
{
#if (DISPLAY_DEVICE_ST7301 == 1)
    // Fireball installed SPI Display device
    .devName = "SPI DISPLAY ST7301",
    .display_init = am_devices_st7301_init,
    .display_term = am_devices_st7301_term,
    .display_blocking_write = am_devices_st7301_blocking_write,
    .display_blocking_cmd_write = am_devices_st7301_command_write,
    .display_nonblocking_write = am_devices_st7301_nonblocking_write,
    .display_nonblocking_write_adv = am_devices_st7301_nonblocking_write_adv,
    .display_blocking_read = am_devices_st7301_blocking_read,
    .display_nonblocking_read = am_devices_st7301_nonblocking_read,
    .display_set_transfer_window = am_devices_st7301_set_transfer_window,
#else
#error "Unknown Display Device"
#endif
};

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
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
    am_mspi_isr1(MSPI_TEST_MODULE)
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

    am_hal_mspi_interrupt_status_get(g_MSPIHdl, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_MSPIHdl, ui32Status);

    am_hal_mspi_interrupt_service(g_MSPIHdl, ui32Status);
}

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
//
//! Take over default ISR. (Queue mode service)
//
void display_iom_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_IOMHandle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_IOMHandle, ui32Status);
            am_hal_iom_interrupt_service(g_IOMHandle, ui32Status);
        }
    }
}

static void teInt_handler(void)
{
#if !defined(START_MSPI_IOM_XFER_ASAP) || !defined(SERIALIZE_COMPOSITION_WITH_RENDERING)
    BaseType_t xHigherPriorityTaskWoken, xResult;
    // Signal main task that TE has arrived
    xHigherPriorityTaskWoken = pdFALSE;

    xResult = xEventGroupSetBitsFromISR(xMainEventHandle, MAIN_EVENT_TE,
                                        &xHigherPriorityTaskWoken);

    if (xResult != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
#endif
#if (IOM_PAUSEFLAG != 0)
    AM_HAL_IOM_SETCLR(DISPLAY_IOM_MODULE, AM_HAL_IOM_SC_UNPAUSE(IOM_PAUSEFLAG));
#endif
}

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void am_gpio_isr(void)
{
    //
    // Read and clear the GPIO interrupt status.
    //
#if defined(AM_PART_APOLLO3P)
    AM_HAL_GPIO_MASKCREATE(GpioIntStatusMask);

    am_hal_gpio_interrupt_status_get(false, pGpioIntStatusMask);
    am_hal_gpio_interrupt_clear(pGpioIntStatusMask);
    am_hal_gpio_interrupt_service(pGpioIntStatusMask);
#elif defined(AM_PART_APOLLO3)
    uint64_t ui64Status;

    am_hal_gpio_interrupt_status_get(false, &ui64Status);
    am_hal_gpio_interrupt_clear(ui64Status);
    am_hal_gpio_interrupt_service(ui64Status);
#else
    #error Unknown device.
#endif
}


int
display_init(void)
{
    uint32_t ui32Status;
    //uint32_t ui32DeviceId;

    am_display_devices_t stDisplayConfig;
    stDisplayConfig.ui32ClockFreq = IOM_FREQ;
    stDisplayConfig.pNBTxnBuf = g_IomQBuffer;
    stDisplayConfig.ui32NBTxnBufLength = sizeof(g_IomQBuffer) / 4;

    ui32Status = display_func.display_init(DISPLAY_IOM_MODULE, &stDisplayConfig, &g_IomDevHdl, &g_IOMHandle);
    if (0 != ui32Status)
    {
        DEBUG_PRINT("Failed to init Display device\n");
        return -1;
    }
    //
    // Enable the IOM interrupt in the NVIC.
    //
    NVIC_EnableIRQ(DISPLAY_IOM_IRQn);
    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_interrupt_clear( AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_DISPLAY_TE));
    am_hal_gpio_interrupt_register(AM_BSP_GPIO_DISPLAY_TE, teInt_handler);
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_DISPLAY_TE));
    NVIC_EnableIRQ(GPIO_IRQn);
    return 0;
}

int
display_deinit(void)
{
    uint32_t ui32Status;

    //
    // Disable the IOM interrupt in the NVIC.
    //
    NVIC_DisableIRQ(DISPLAY_IOM_IRQn);

    ui32Status = display_func.display_term(g_IomDevHdl);
    if (0 != ui32Status)
    {
        DEBUG_PRINT("Failed to terminate Display device\n");
        return -1;
    }
    return 0;
}

int
mspi_psram_init(const am_devices_mspi_psram_config_t *mspiPSRAMConfig)
{
    uint32_t      ui32Status;

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_init(MSPI_TEST_MODULE, &MSPI_PSRAM_QuadCE0MSPIConfig, &g_MSPIDevHdl, &g_MSPIHdl);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        DEBUG_PRINT("Failed to configure the MSPI and PSRAM Device correctly!\n");
        return -1;
    }

    NVIC_EnableIRQ(mspi_interrupts[MSPI_TEST_MODULE]);

#ifdef ENABLE_XIP
    // Gernerate Data into XIP Area

    //
    // Write the executable function into the target sector.
    //
    am_util_stdio_printf("Writing Executable function of %d Bytes to address %d\n", SZ_PRIME_MPI, PSRAM_XIP_BASE);
    ui32Status = am_devices_mspi_psram_write(g_MSPIDevHdl, (uint8_t *)Kc_PRIME_MPI, PSRAM_XIP_OFFSET, SZ_PRIME_MPI, true);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to write executable function to Flash Device!\n");
        return -1;
    }
#endif
#if defined(ENABLE_XIP) || defined(ENABLE_XIPMM) || (MODE_SRCBUF_READ != MODE_DMA) || (MODE_DESTBUF_WRITE != MODE_DMA)
    //
    // Set up for XIP operation.
    //
    am_util_stdio_printf("Putting the MSPI and External PSRAM into XIP mode\n");
    ui32Status = am_devices_mspi_psram_enable_xip(g_MSPIDevHdl);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to put the MSPI into XIP mode!\n");
        return -1;
    }
#ifndef XIP_UNCACHED
    ui32Status = am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_FLASH_CACHE_INVALIDATE, 0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to invalidate Cache!\n");
        return -1;
    }
#else
    //  Mark XIP as non-cached - to make sure we see its impact
    am_hal_cachectrl_nc_cfg_t ncCfg;
    ncCfg.bEnable = true;
    ncCfg.eNCRegion = AM_HAL_CACHECTRL_NCR0;
    ncCfg.ui32StartAddr = PSRAM_XIP_BASE;
    ncCfg.ui32EndAddr = PSRAM_XIP_BASE + PSRAM_XIP_SIZE;
    ui32Status = am_hal_cachectrl_control(AM_HAL_CACHECTRL_CONTROL_NC_CFG, &ncCfg);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to mark XIP region as non-cacheable\n");
        return -1;
    }
#endif

#endif
    return 0;
}


//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
#ifdef ENABLE_XIP
bool
run_mspi_xip(void)
{
    mspi_xip_test_function_t test_function = (mspi_xip_test_function_t)((PSRAM_XIP_BASE) | 0x00000001);
    //
    // Execute a call to the test function in the sector.
    //
    DEBUG_PRINT_SUCCESS("Jumping to function in External Flash\n");

    // Test the function
    return (test_function(10, 0, 1) == 4);
}
#endif

#ifdef ENABLE_XIPMM
bool
run_mspi_xipmm(uint32_t block)
{
    uint32_t i;
    uint8_t *pAddr1 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS + PSRAM_XIPMM_OFFSET + block*XIPMM_BLOCK_SIZE);
    uint8_t *pAddr2 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS + PSRAM_XIPMM_OFFSET + block*XIPMM_BLOCK_SIZE + XIPMM_BLOCK_SIZE);
    // Initialize a pattern
    for (i = 0; i < XIPMM_BLOCK_SIZE; i++)
    {
        *pAddr1++ = i & 0xFF;
        *pAddr2++ = (i & 0xFF) ^ 0xFF;
    }
    pAddr1 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS + PSRAM_XIPMM_OFFSET + block*XIPMM_BLOCK_SIZE);
    pAddr2 = (uint8_t *)(MSPI_XIPMM_BASE_ADDRESS + PSRAM_XIPMM_OFFSET + block*XIPMM_BLOCK_SIZE + XIPMM_BLOCK_SIZE);
    // Verify the pattern
    for (i = 0; i < XIPMM_BLOCK_SIZE; i++)
    {
        if ((*pAddr1++ != (i & 0xFF)) || (*pAddr2++ != ((i & 0xFF) ^ 0xFF)))
        {
            return false;
        }
    }
    return true;
}
#endif

void vTimerCallback( TimerHandle_t xTimer )
{
    switch((uint32_t)pvTimerGetTimerID( xTimer ))
    {
#ifdef ENABLE_XIP
        case 0:
          DEBUG_GPIO_HIGH(TEST_GPIO1);
          if (!run_mspi_xip())
          {
              DEBUG_PRINT("Unable to run XIP successfully\n");
              while(1);
          }
          DEBUG_GPIO_LOW(TEST_GPIO1);
          break;
#endif
#ifdef ENABLE_XIPMM
        case 1:
          // XIPMM
          DEBUG_GPIO_HIGH(TEST_GPIO3);
          if (!run_mspi_xipmm(0))
          {
              DEBUG_PRINT("Unable to run XIP successfully\n");
              while(1);
          }
          DEBUG_GPIO_LOW(TEST_GPIO3);
          break;
#endif
        default:
        ;
    }
}


//*****************************************************************************
//
// Perform initial setup for the main task.
//
//*****************************************************************************
void
MainTaskSetup(void)
{
    int iRet;
#if defined(ENABLE_XIPMM) || defined(ENABLE_XIP)
    uint32_t timerId = 0;
#endif
    am_devices_mspi_psram_config_t *mspiPSRAMCfg =
#ifdef MSPI_FLASH_SERIAL
            &MSPI_PSRAM_SerialCE0MSPIConfig;
#else
            &MSPI_PSRAM_QuadCE0MSPIConfig;
#endif

    am_util_debug_printf("MainTask: setup\r\n");
    //
    // Create an event handle for our wake-up events.
    //
    xMainEventHandle = xEventGroupCreate();

    //
    // Make sure we actually allocated space for the events we need.
    //
    while (xMainEventHandle == NULL);

    NVIC_SetPriority(DISPLAY_IOM_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(MSPI0_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(GPIO_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);


    am_hal_gpio_pinconfig(CPU_SLEEP_GPIO, g_AM_HAL_GPIO_OUTPUT_12);
    DEBUG_GPIO_LOW(CPU_SLEEP_GPIO);

    // Enable Power to display
    am_hal_gpio_pinconfig(AM_DISPLAY_GPIO_PWR_ENABLE, g_AM_DISPLAY_GPIO_ENABLE);
    DEBUG_GPIO_HIGH(AM_DISPLAY_GPIO_PWR_ENABLE);

    // Initialize the MSPI PSRAM
    iRet = mspi_psram_init(mspiPSRAMCfg);
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize MSPI psram\n");
        while(1);
    }

    // Initialize the IOM Display

    iRet = display_init();
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize Display\n");
        while(1);
    }

    am_hal_interrupt_master_enable();

// #### INTERNAL BEGIN ####
//    if (*((uint32_t *)AM_REG_INFO0_SIGNATURE0_ADDR) == 0x48EAAD88)
//    {
//        while(1);
//    }
// #### INTERNAL END ####

    // Configure a Timer to trigger XIP & XIPMM
#ifdef ENABLE_XIP
    xTimerXIP = xTimerCreate("XIPTimer",
                             (XIP_FREQ_HZ / configTICK_RATE_HZ),    /* The timer period in ticks. */
                             pdTRUE,                                /* The timers will auto-reload themselves when they expire. */
                             ( void * ) timerId,                    /* Assign each timer a unique id equal to its array index. */
                             vTimerCallback                         /* Each timer calls the same callback when it expires. */
                             );
    if (xTimerXIP == NULL)
    {
        DEBUG_PRINT("Unable to Create XIP Timer\n");
        while(1);
    }
    if ( xTimerStart( xTimerXIP, 0 ) != pdPASS )
    {
        /* The timer could not be set into the Active state. */
        DEBUG_PRINT("Unable to Start XIP Timer\n");
        while(1);
    }
#endif
#ifdef ENABLE_XIPMM
    timerId = 1;
    xTimerXIPMM = xTimerCreate("XIPMMTimer",
                             (XIPMM_FREQ_HZ / configTICK_RATE_HZ),  /* The timer period in ticks. */
                             pdTRUE,                                /* The timers will auto-reload themselves when they expire. */
                             ( void * ) timerId,                    /* Assign each timer a unique id equal to its array index. */
                             vTimerCallback                         /* Each timer calls the same callback when it expires. */
                             );
    if (xTimerXIPMM == NULL)
    {
        DEBUG_PRINT("Unable to Create XIPMM Timer\n");
        while(1);
    }
    if ( xTimerStart( xTimerXIPMM, 0 ) != pdPASS )
    {
        /* The timer could not be set into the Active state. */
        DEBUG_PRINT("Unable to Start XIPMM Timer\n");
        while(1);
    }
#endif
    xMSPIMutex = xSemaphoreCreateMutex();
    if (xMSPIMutex == NULL)
    {
        DEBUG_PRINT("Unable to Create xMspiMutex\n");
        while(1);
    }
}

typedef enum
{
    DISPLAY_IDLE,
    DISPLAY_SCHEDULED,
    DISPLAY_STARTED
} display_state_e;

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
MainTask(void *pvParameters)
{
    uint32_t eventMask;
    display_state_e displayState = DISPLAY_IDLE;
    uint32_t numFBReadyForDisplay = 0;
    // One FB composition is already started outside
    uint32_t numFBAvailForComposition = NUM_FB - 1;
    bool     bCompositionInProgress = true;

    while (1)
    {
        //
        // Wait for an event to be posted to the Radio Event Handle.
        //
        eventMask = xEventGroupWaitBits(xMainEventHandle, 0xF, pdTRUE,
                            pdFALSE, portMAX_DELAY);
        if (eventMask != 0)
        {
            if (eventMask & MAIN_EVENT_NEW_FRAME_READY)
            {
                numFBReadyForDisplay++;
                bCompositionInProgress = false;
            }
            if (eventMask & MAIN_EVENT_DISPLAY_DONE)
            {
                displayState = DISPLAY_IDLE;
#ifdef SERIALIZE_COMPOSITION_WITH_RENDERING
                // Frame is available for composition only once display rendering is done
                numFBAvailForComposition++;
#endif
            }
#if !defined(SERIALIZE_COMPOSITION_WITH_RENDERING) && defined(START_MSPI_IOM_XFER_ASAP)
            // Frame is available for composition as soon as Rendering starts (indicated by TE)
            if ((displayState == DISPLAY_SCHEDULED) && (eventMask & MAIN_EVENT_TE))
            {
                displayState = DISPLAY_STARTED;
                numFBAvailForComposition++;
            }
#endif
#ifndef START_MSPI_IOM_XFER_ASAP
            if ((displayState == DISPLAY_IDLE) && numFBReadyForDisplay && (eventMask & MAIN_EVENT_TE))
            {
                numFBReadyForDisplay--;
#if !defined(SERIALIZE_COMPOSITION_WITH_RENDERING) && !defined(START_MSPI_IOM_XFER_ASAP)
                // Frame is available for composition as soon as Rendering starts (now)
                displayState = DISPLAY_STARTED;
                numFBAvailForComposition++;
#else
                // Frame is not available for composition till Rendering starts (on next TE)
                displayState = DISPLAY_SCHEDULED;
#endif
                // Initiate new frame transfer
                xEventGroupSetBits(xRenderEventHandle, RENDER_EVENT_START_NEW_FRAME);
            }
#else
            if ((displayState == DISPLAY_IDLE) && numFBReadyForDisplay)
            {
                numFBReadyForDisplay--;
#if !defined(SERIALIZE_COMPOSITION_WITH_RENDERING) && !defined(START_MSPI_IOM_XFER_ASAP)
                // Frame is available for composition as soon as Rendering starts (now)
                displayState = DISPLAY_STARTED;
                numFBAvailForComposition++;
#else
                // Frame is not available for composition till Rendering starts (on next TE)
                displayState = DISPLAY_SCHEDULED;
#endif
                // Initiate new frame transfer
                xEventGroupSetBits(xRenderEventHandle, RENDER_EVENT_START_NEW_FRAME);
            }
#endif

            if (numFBAvailForComposition && !bCompositionInProgress)
            {
                numFBAvailForComposition--;
                bCompositionInProgress = true;
                // Initiate new frame composition
                xEventGroupSetBits(xComposeEventHandle, COMPOSE_EVENT_START_NEW_FRAME);
            }
        }
    }
}
