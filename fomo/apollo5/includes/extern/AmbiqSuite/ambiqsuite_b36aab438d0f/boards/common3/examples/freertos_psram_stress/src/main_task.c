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
#include "freertos_psram_stress.h"

// Position independent prime function
#define SZ_PRIME_MPI    98
const unsigned char Kc_PRIME_MPI[SZ_PRIME_MPI] =
{
    0x70, 0xB4, 0x04, 0x46, 0x00, 0x20, 0x89, 0x1C, 0x8C, 0x42, 0x28, 0xDB, 0x45, 0x1C, 0x02, 0x26,
    0x8E, 0x42, 0x20, 0xDA, 0x91, 0xFB, 0xF6, 0xF3, 0x06, 0xFB, 0x13, 0x13, 0xD3, 0xB1, 0x76, 0x1C,
    0x8E, 0x42, 0x18, 0xDA, 0x91, 0xFB, 0xF6, 0xF3, 0x06, 0xFB, 0x13, 0x13, 0x93, 0xB1, 0x76, 0x1C,
    0x8E, 0x42, 0x10, 0xDA, 0x91, 0xFB, 0xF6, 0xF3, 0x06, 0xFB, 0x13, 0x13, 0x53, 0xB1, 0x76, 0x1C,
    0x8E, 0x42, 0x08, 0xDA, 0x91, 0xFB, 0xF6, 0xF3, 0x06, 0xFB, 0x13, 0x13, 0x00, 0x2B, 0x18, 0xBF,
    0x76, 0x1C, 0xDD, 0xD1, 0x05, 0x46, 0x51, 0x18, 0x8C, 0x42, 0x28, 0x46, 0xD6, 0xDA, 0x70, 0xBC,
    0x70, 0x47,
};

#define MSPI_TEST_MODULE              0

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

TimerHandle_t xTimerXIP;
TimerHandle_t xTimerXIPMM;


// Globals
// Buffer for non-blocking transactions for IOM - Needs to be big enough to accomodate
// all the transactions
uint32_t        g_IomQBuffer[3072];
// Buffer for non-blocking transactions for MSPI - Needs to be big enough to accomodate
// all the transactions
uint32_t        g_MspiQBuffer[2560];

// DMA transaction buffer
uint8_t         gHiPrioDMABuf[32][24];

// Temp Buffers
uint8_t         g_TXBuffer[FB_SIZE];
uint8_t         g_RXBuffer[FB_SIZE];

void            *g_MSPIDevHdl;
void            *g_MSPIHdl;
void            *g_IomDevHdl;
void            *g_IOMHandle;

uint32_t        g_numIter = 0;

am_devices_mspi_psram_config_t MSPI_PSRAM_SerialCE0MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_SERIAL_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_24MHZ,
    .ui32NBTxnBufLength       = sizeof(g_MspiQBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_MspiQBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = PSRAM_XIP_SIZE / (64 * 1024), // Enable scrambling for the XIP region
};

am_devices_mspi_psram_config_t MSPI_PSRAM_QuadCE0MSPIConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_QUAD_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_24MHZ,
    .ui32NBTxnBufLength       = sizeof(g_MspiQBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_MspiQBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = PSRAM_XIP_SIZE / (64 * 1024), // Enable scrambling for the XIP region
};

fram_device_func_t fram_func =
{
#if (FRAM_DEVICE_MB85RS1MT == 1)
    // Fireball installed SPI FRAM device
    .devName = "SPI FRAM MB85RS1MT",
    .fram_init = am_devices_mb85rs1mt_init,
    .fram_term = am_devices_mb85rs1mt_term,
    .fram_read_id = am_devices_mb85rs1mt_read_id,
    .fram_blocking_write = am_devices_mb85rs1mt_blocking_write,
    .fram_nonblocking_write = am_devices_mb85rs1mt_nonblocking_write,
    .fram_nonblocking_write_adv = am_devices_mb85rs1mt_nonblocking_write_adv,
    .fram_blocking_read = am_devices_mb85rs1mt_blocking_read,
    .fram_nonblocking_read = am_devices_mb85rs1mt_nonblocking_read,
    .fram_command_send = am_devices_mb85rs1mt_command_send,
#if FIREBALL_CARD
    .fram_fireball_control = AM_DEVICES_FIREBALL_STATE_SPI_FRAM,
#else
    .fram_fireball_control = 0,
#endif
#elif (FRAM_DEVICE_MB85RC256V == 1)
    .devName = "I2C FRAM MB85RC256V",
    .fram_init = am_devices_mb85rc256v_init,
    .fram_term = am_devices_mb85rc256v_term,
    .fram_read_id = am_devices_mb85rc256v_read_id,
    .fram_blocking_write = am_devices_mb85rc256v_blocking_write,
    .fram_nonblocking_write = am_devices_mb85rc256v_nonblocking_write,
    .fram_blocking_read = am_devices_mb85rc256v_blocking_read,
    .fram_nonblocking_read = am_devices_mb85rc256v_nonblocking_read,
    .fram_fireball_control = 0,
#elif (FRAM_DEVICE_MB85RS64V == 1)
    .devName = "SPI FRAM MB85RS64V",
    .fram_init = am_devices_mb85rs64v_init,
    .fram_term = am_devices_mb85rs64v_term,
    .fram_read_id = am_devices_mb85rs64v_read_id,
    .fram_blocking_write = am_devices_mb85rs64v_blocking_write,
    .fram_nonblocking_write = am_devices_mb85rs64v_nonblocking_write,
    .fram_nonblocking_write_adv = am_devices_mb85rs64v_nonblocking_write_adv,
    .fram_blocking_read = am_devices_mb85rs64v_blocking_read,
    .fram_nonblocking_read = am_devices_mb85rs64v_nonblocking_read,
    .fram_command_send = am_devices_mb85rs64v_command_send,
    .fram_fireball_control = 0,
#elif (FRAM_DEVICE_MB85RC64TA == 1)
    // Fireball installed I2C FRAM device
    .devName = "I2C FRAM MB85RC64TA",
    .fram_init = am_devices_mb85rc256v_init,
    .fram_term = am_devices_mb85rc256v_term,
    .fram_read_id = am_devices_mb85rc256v_read_id,
    .fram_blocking_write = am_devices_mb85rc256v_blocking_write,
    .fram_nonblocking_write = am_devices_mb85rc256v_nonblocking_write,
    .fram_blocking_read = am_devices_mb85rc256v_blocking_read,
    .fram_nonblocking_read = am_devices_mb85rc256v_nonblocking_read,
#if FIREBALL_CARD
    .fram_fireball_control = AM_DEVICES_FIREBALL_STATE_I2C_FRAM,
#else
    .fram_fireball_control = 0,
#endif
#else
#error "Unknown FRAM Device"
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
// Function prototypes
//
//*****************************************************************************


bool
run_mspi_xip(uint32_t numFunc)
{
    mspi_xip_test_function_t test_function = (mspi_xip_test_function_t)((PSRAM_XIP_BASE + numFunc*SZ_PRIME_MPI) | 0x00000001);
    //
    // Execute a call to the test function in the sector.
    //
    DEBUG_PRINT_SUCCESS("Jumping to function in External Flash\n");
    // Test the function
    return (test_function(100, 0, 1) == 25);
}

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
        if ((*pAddr1++ != (i & 0xFF)) || (*pAddr2 != (i & 0xFF) ^ 0xFF))
        {
            return false;
        }
    }
    return true;
}

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
//
//! Take over default ISR. (Queue mode service)
//
void fram_iom_isr(void)
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



int
fram_init(void)
{
    uint32_t ui32Status;
    uint32_t ui32DeviceId;

    // Set up IOM
    // Initialize the Device

#if FIREBALL_CARD
    uint32_t ui32Ret, ui32ID;

#if 1
    //
    // Get Fireball ID and Rev info.
    //
    ui32Ret = am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_ID_GET, &ui32ID);
    if ( ui32Ret != 0 )
    {
        DEBUG_PRINT("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                             AM_DEVICES_FIREBALL_STATE_ID_GET, ui32Ret);
        return -1;
    }
    else if ( ui32ID == FIREBALL_ID )
    {
        DEBUG_PRINT_SUCCESS("Fireball found, ID is 0x%X.\n", ui32ID);
    }
    else
    {
        DEBUG_PRINT("Unknown device returned ID as 0x%X.\n", ui32ID);
    }

    ui32Ret = am_devices_fireball_control(AM_DEVICES_FIREBALL_STATE_VER_GET, &ui32ID);
    if ( ui32Ret != 0 )
    {
        DEBUG_PRINT("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                             AM_DEVICES_FIREBALL_STATE_VER_GET, ui32Ret);
        return -1;
    }
    else
    {
        DEBUG_PRINT_SUCCESS("Fireball Version is 0x%X.\n", ui32ID);
    }
#endif

    if ( fram_func.fram_fireball_control != 0 )
    {
        ui32Ret = am_devices_fireball_control(fram_func.fram_fireball_control, 0);
        if ( ui32Ret != 0 )
        {
            DEBUG_PRINT("FAIL: am_devices_fireball_control(%d) returned 0x%X.\n",
                                 fram_func.fram_fireball_control, ui32Ret);
            return -1;
        }
    }
#endif // FIREBALL_CARD

    am_iom_test_devices_t stFramConfig;
    stFramConfig.ui32ClockFreq = IOM_TEST_FREQ;
    stFramConfig.pNBTxnBuf = g_IomQBuffer;
    stFramConfig.ui32NBTxnBufLength = sizeof(g_IomQBuffer) / 4;

    ui32Status = fram_func.fram_init(FRAM_IOM_MODULE, &stFramConfig, &g_IomDevHdl, &g_IOMHandle);
    if (0 == ui32Status)
    {
        ui32Status = fram_func.fram_read_id(g_IomDevHdl, &ui32DeviceId);

        if ((ui32Status  != 0) || (ui32DeviceId != FRAM_DEVICE_ID))
        {
            return -1;
        }
        DEBUG_PRINT_SUCCESS("%s Found\n", fram_func.devName);
    }
    else
    {
        return -1;
    }
    //
    // Enable the IOM interrupt in the NVIC.
    //
    NVIC_EnableIRQ(FRAM_IOM_IRQn);

    return 0;
}

int
fram_deinit(void)
{
    uint32_t ui32Status;

    //
    // Disable the IOM interrupt in the NVIC.
    //
    NVIC_DisableIRQ(FRAM_IOM_IRQn);

    // Set up IOM
    // Initialize the Device

    ui32Status = fram_func.fram_term(g_IomDevHdl);
    if (0 != ui32Status)
    {
        DEBUG_PRINT("Failed to terminate FRAM device\n");
        return -1;
    }
    return 0;
}


int
mspi_psram_init(am_devices_mspi_psram_config_t *mspiConfig)
{
    //
    // Configure the MSPI and PSRAM Device.
    //
    am_hal_mspi_hiprio_cfg_t hpCfg;

    uint32_t ui32Status = am_devices_mspi_psram_init(MSPI_TEST_MODULE, mspiConfig, &g_MSPIDevHdl, &g_MSPIHdl);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
        return ui32Status;
    }
    NVIC_EnableIRQ(mspi_interrupts[MSPI_TEST_MODULE]);

    am_hal_interrupt_master_enable();
    //
    // Make sure we aren't in XIP mode.
    //
    ui32Status = am_devices_mspi_psram_disable_xip(g_MSPIDevHdl);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
        return ui32Status;
    }

    // Set up for hi prioirty transactions
    hpCfg.pBuf = (uint8_t *)gHiPrioDMABuf;
    hpCfg.size = sizeof(gHiPrioDMABuf);

    ui32Status = am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_INIT_HIPRIO, &hpCfg);
    if (ui32Status)
    {
        DEBUG_PRINT("Unable to initialize MSPI Hi Prio Buffer\n");
        return ui32Status;
    }


    // PSRAM space is divided into following sections for this experiment
    // XIP - This is where we'll load the library code
    // XIPMM - this is where we'll allocate R/W variables
    // FB_Src - this is where we'll initialize Source Frame Buffer Images - Base1 & Base2
    // FB - Active Frame buffers - FB1 & FB2

    //
    // Generate data into the Source Buffers
    //
    for (uint32_t j = 0; j < 2; j++)
    {
        for (uint32_t i = 0; i < FB_SIZE; i++)
        {
           g_TXBuffer[i] = (i & 0xFF);
           if (j)
           {
                g_TXBuffer[i] ^= 0xFF;
           }
        }

        //
        // Write the TX buffer into the target sector.
        //
        am_util_stdio_printf("Writing %d Bytes to Address 0x%x\n", FB_SIZE, PSRAM_SRCFB_BASE + j*FB_SIZE);
        ui32Status = am_devices_mspi_psram_write(g_MSPIDevHdl, g_TXBuffer, (PSRAM_SRCFB_OFFSET + j*FB_SIZE), FB_SIZE, true);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to write buffer to Flash Device!\n");
            return ui32Status;
        }

        //
        // Write the TX buffer into the target sector.
        //
        am_util_stdio_printf("Writing %d Bytes to Address 0x%x\n", FB_SIZE, PSRAM_ACTFB_BASE + j*FB_SIZE);
        ui32Status = am_devices_mspi_psram_write(g_MSPIDevHdl, g_TXBuffer, (PSRAM_ACTFB_OFFSET + j*FB_SIZE), FB_SIZE, true);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to write buffer to Flash Device!\n");
            return ui32Status;
        }

        //
        // Read the data back into the RX buffer.
        //
        am_util_stdio_printf("Read %d Bytes from address %d\n", FB_SIZE, PSRAM_SRCFB_BASE + j*FB_SIZE);
        ui32Status = am_devices_mspi_psram_read(g_MSPIDevHdl, g_RXBuffer, (PSRAM_SRCFB_OFFSET + j*FB_SIZE), FB_SIZE, true);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to read buffer to Flash Device!\n");
            return ui32Status;
        }

        //
        // Compare the buffers
        //
        am_util_stdio_printf("Comparing the TX and RX Buffers\n");
        for (uint32_t i = 0; i < FB_SIZE; i++)
        {
            if (g_RXBuffer[i] != g_TXBuffer[i])
            {
                am_util_stdio_printf("TX and RX buffers failed to compare!\n");
                return AM_HAL_STATUS_FAIL;
            }
        }
    }

    // Gernerate Data into XIP Area

    //
    // Write the executable function into the target sector.
    //
    am_util_stdio_printf("Writing Executable function of %d Bytes to address %d\n", SZ_PRIME_MPI, PSRAM_XIP_BASE);
    for (uint32_t numFunc = 0; numFunc < XIP_ITERATIONS; numFunc++)
    {
        ui32Status = am_devices_mspi_psram_write(g_MSPIDevHdl, (uint8_t *)Kc_PRIME_MPI, PSRAM_XIP_OFFSET + numFunc*SZ_PRIME_MPI, SZ_PRIME_MPI, true);
        if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
        {
            am_util_stdio_printf("Failed to write executable function to Flash Device!\n");
            return ui32Status;
        }
    }

    //
    // Set up for XIP operation.
    //
    am_util_stdio_printf("Putting the MSPI and External PSRAM into XIP mode\n");
    ui32Status = am_devices_mspi_psram_enable_xip(g_MSPIDevHdl);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to put the MSPI into XIP mode!\n");
        return ui32Status;
    }


#ifdef ENABLE_MSPI_SCRAMBLING
    //
    // Turn on scrambling operation.
    //
    am_util_stdio_printf("Putting the MSPI into Scrambling mode\n");
    ui32Status = am_devices_mspi_psram_enable_scrambling(g_MSPIDevHdl);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable MSPI scrambling!\n");
        return ui32Status;
    }
#endif

    if (!run_mspi_xip(0))
    {
        return AM_HAL_STATUS_FAIL;
    }
    return ui32Status;
}


#if 1
int
init_fram_data(void)
{
    uint32_t      ui32Status;
#if 0
    // Set the FRAM in write mode
    // Send the WRITE ENABLE command to enable writing.
    //
    if (am_devices_mb85rs1mt_command_send(AM_DEVICES_MB85RS1MT_WRITE_ENABLE))
    {
        return AM_DEVICES_MB85RS1MT_STATUS_ERROR;
    }
#endif

    // Verify FRAM data
    for (uint32_t address = 0; address < (FB_SIZE); address += SPI_TXN_SIZE)
    {
        uint32_t numBytes = (((address + SPI_TXN_SIZE) >= FB_SIZE) ? (FB_SIZE - address) : SPI_TXN_SIZE);
        //
        // Generate data into the Sector Buffer
        //
        for (uint32_t i = 0; i < numBytes / 4; i++)
        {
            g_TXBuffer[i] = 0xFF; //address + i*4;
        }

        //
        // Write the TX buffer into the target sector.
        //
        DEBUG_PRINT_SUCCESS("Writing %d Bytes to Address 0x%x\n", address);
        ui32Status = fram_func.fram_blocking_write(g_IomDevHdl, (uint8_t *)g_TXBuffer, address, numBytes);
        if (ui32Status)
        {
            DEBUG_PRINT("Failed to write FRAM!\n");
             return -1;
        }
    }
    // Wait for writes to finish
    for (uint32_t address = 0; address < (FB_SIZE); address += SPI_TXN_SIZE)
    {
        uint32_t numBytes = (((address + SPI_TXN_SIZE) >= FB_SIZE) ? (FB_SIZE - address) : SPI_TXN_SIZE);
        //
        // Read the data back into the RX buffer.
        //
        DEBUG_PRINT_SUCCESS("Read %d Bytes from Address 0x%x\n", numBytes, address);
        // Initiate read of a block of data from FRAM
        ui32Status = fram_func.fram_blocking_read(g_IomDevHdl, (uint8_t *)&g_RXBuffer, address, numBytes);

        if (0 != ui32Status)
        {
            DEBUG_PRINT("Failed to read FRAM!\n");
            return -1;
        }

        //
        // Compare the buffers
        //
        DEBUG_PRINT_SUCCESS("Comparing the TX and RX Buffers\n");
        for (uint32_t i = 0; i < numBytes / 4; i++)
        {
            if (g_TXBuffer[i] != g_RXBuffer[i])
            {
                DEBUG_PRINT("TX and RX buffers failed to compare!\n");
                return -1;
            }
        }
    }
    return 0;
}
#endif


#ifdef VERIFY_DATA
int
verify_fram_data(void)
{
    uint32_t      ui32Status;

    // Verify FRAM data
    //
    // Read the data back into the RX buffer.
    //
    for (uint32_t address = 0; address < (FB_SIZE); address += SPI_TXN_SIZE)
    {
        uint32_t numBytes = (((address + SPI_TXN_SIZE) >= FB_SIZE) ? (FB_SIZE - address) : SPI_TXN_SIZE);
        // Initiate read of a block of data from FRAM
        ui32Status = fram_func.fram_blocking_read(g_IomDevHdl, (uint8_t *)&g_RXBuffer[address], address, numBytes);

        if (0 != ui32Status)
        {
            DEBUG_PRINT("Failed to read FRAM!\n");
            return -1;
        }
    }
    //
    // Compare the buffers
    //
    for (uint32_t address = 0; address < (FB_SIZE); address ++)
    {
        if ((g_numIter - 1) & 0x1)
        {
#if 0
            if (g_RXBuffer[address] != ((((address^0xFF)&0xFF) + g_numIter - 1) & 0xFF))
#else
            if (g_RXBuffer[address] != ((address^0xFF) & 0xFF))
#endif
            {
                DEBUG_PRINT("TX and RX buffers failed to compare!\n");
                return -1;
            }
        }
        else
        {
#if 0
            if (g_RXBuffer[address] != ((address + g_numIter - 1) & 0xFF))
#else
            if (g_RXBuffer[address] != (address & 0xFF))
#endif
            {
                DEBUG_PRINT("TX and RX buffers failed to compare!\n");
                return -1;
            }
        }
    }
    DEBUG_PRINT_SUCCESS("FRAM data matches the expectation\n");
    return 0;
}
#endif

void vTimerCallback( TimerHandle_t xTimer )
{
    switch((uint32_t)pvTimerGetTimerID( xTimer ))
    {
        case 0:
          TEST_GPIO_HIGH(TEST_GPIO1);
          if (XIP_ITERATIONS)
          {
              for (uint32_t numFunc = 0; numFunc < XIP_ITERATIONS; numFunc++)
              {
                  if (!run_mspi_xip(numFunc))
                  {
                      DEBUG_PRINT("Unable to run XIP successfully\n");
                      while(1);
                  }
              }
          }
          TEST_GPIO_LOW(TEST_GPIO1);
          break;
        case 1:
          TEST_GPIO_HIGH(TEST_GPIO);
#ifdef ENABLE_XIPMM
          // XIPMM
          if (!run_mspi_xipmm(numFunc))
          {
              DEBUG_PRINT("Unable to run XIP successfully\n");
              while(1);
          }
#else
          am_util_delay_us(5);
#endif
          TEST_GPIO_LOW(TEST_GPIO);
          break;
        default:
        ;
    }
}

//*****************************************************************************
//
// Perform initial setup for the radio task.
//
//*****************************************************************************
void
MainTaskSetup(void)
{
    int iRet;
    uint32_t timerId = 0;
    am_devices_mspi_psram_config_t *mspiFlashCfg =
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

    NVIC_SetPriority(FRAM_IOM_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_SetPriority(MSPI0_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);

    // Initialize the IOM FRAM
    iRet = fram_init();
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize FRAM\n");
        while(1);
    }

    // Initialize FRAM Data
    iRet = init_fram_data();
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize FRAM data\n");
        while(1);
    }

// #### INTERNAL BEGIN ####
//    if (*((uint32_t *)AM_REG_INFO0_SIGNATURE0_ADDR) == 0x48EAAD88)
//    {
//        while(1);
//    }
// #### INTERNAL END ####
    // Initialize the MSPI Flash
    iRet = mspi_psram_init(mspiFlashCfg);
    if (iRet)
    {
        DEBUG_PRINT("Unable to initialize MSPI Flash\n");
        while(1);
    }
    // Notify main task that the frame is ready
    xEventGroupSetBits(xMainEventHandle, MAIN_EVENT_NEW_FRAME_READY);
    // Configure a Timer to trigger XIP & XIPMM
    xTimerXIP = xTimerCreate("XIPTimer",
                             100,    /* The timer period in ticks. */
                             pdTRUE,         /* The timers will auto-reload themselves when they expire. */
                             ( void * ) timerId,   /* Assign each timer a unique id equal to its array index. */
                             vTimerCallback  /* Each timer calls the same callback when it expires. */
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
    timerId = 1;
    xTimerXIPMM = xTimerCreate("XIPMMTimer",
                             100,    /* The timer period in ticks. */
                             pdTRUE,         /* The timers will auto-reload themselves when they expire. */
                             ( void * ) timerId,   /* Assign each timer a unique id equal to its array index. */
                             vTimerCallback  /* Each timer calls the same callback when it expires. */
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
}

//*****************************************************************************
//
// Short Description.
//
//*****************************************************************************
void
MainTask(void *pvParameters)
{
    uint32_t eventMask;
    bool     bDisplayDone = true;
    bool     bFBReady = false;
    uint32_t      ui32Status;
    am_devices_mspi_psram_config_t *mspiFlashCfg =
#ifdef MSPI_FLASH_SERIAL
            &MSPI_PSRAM_SerialCE0MSPIConfig;
#else
            &MSPI_PSRAM_QuadCE0MSPIConfig;
#endif

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
                bFBReady = true;
            }
            if (eventMask & MAIN_EVENT_DISPLAY_DONE)
            {
#ifdef VERIFY_DATA
                if (verify_fram_data())
                {
                    DEBUG_PRINT("Verify data failed!\n");
                    while(1);
                }
#endif
                bDisplayDone = true;
            }
            if (bFBReady && bDisplayDone)
            {
                bFBReady = bDisplayDone = false;
                if (++g_numIter == NUM_ITERATIONS)
                {
                    break;
                }
                // Initiate new Display Render
                xEventGroupSetBits(xRenderEventHandle, RENDER_EVENT_START_NEW_FRAME);
                // Initiate new frame composition
                xEventGroupSetBits(xComposeEventHandle, COMPOSE_EVENT_START_NEW_FRAME);
            }
        }
    }
#ifdef SEQLOOP
    // Set in Sequence mode
    bool bBool = false;
    am_hal_mspi_control(g_MSPIHdl, AM_HAL_MSPI_REQ_SET_SEQMODE, &bBool);
    am_hal_iom_control(g_IOMHandle, AM_HAL_IOM_REQ_SET_SEQMODE, &bBool);
#endif

#if 0
    //
    // Clean up the FRAM before exit.
    //
    iRet = fram_deinit();
    if (iRet)
    {
        DEBUG_PRINT("Unable to terminate FRAM\n");
    }
#endif
    //
    // Clean up the MSPI before exit.
    //
    am_hal_interrupt_master_disable();
    NVIC_DisableIRQ(mspi_interrupts[MSPI_TEST_MODULE]);
    ui32Status = am_devices_mspi_psram_deinit(g_MSPIDevHdl);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        DEBUG_PRINT("Failed to shutdown the MSPI and PSRAM Device!\n");
    }

    //
    //  End banner.
    //
    DEBUG_PRINT("Apollo3 MSPI-IOM Transfer Example Complete\n");

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }

}
