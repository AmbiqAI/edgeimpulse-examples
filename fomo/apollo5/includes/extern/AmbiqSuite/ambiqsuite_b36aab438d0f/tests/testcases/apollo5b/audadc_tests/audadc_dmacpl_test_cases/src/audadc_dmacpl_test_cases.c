//*****************************************************************************
//
//! @file audadc_dmacpl_test_cases.c
//!
//! @brief This test DMA complete interrupt.
//!
//! Purpose: This test check if DMA complete interrupt was missing.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
//*****************************************************************************
#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "stdio.h"
#include "audadc_dmacpl_test_cases.h"

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

    //
    // Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Print the banner
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo5a Audio ADC Test Cases\n\n");
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// Defines and global variables
//
//*****************************************************************************

#define MAX_DURATION (0.00534f * 1.5f)
#define TIMEOUT (10.0f)
//
// AUDADC Sample buffer.
//
AM_SHARED_RW uint32_t g_ui32AUDADCSampleBuffer[AUDADC_SAMPLE_BUF_SIZE];
//
// AUDADC Device Handle.
//
static void *g_AUDADCHandle;
//
// AUDADC DMA complete flag.
//
volatile bool g_bAUDADCDMAComplete;
//
// AUDADC DMA error flag.
//
volatile bool g_bAUDADCDMAError;

volatile float g_calculatedTimer = 0;
bool bTestPass = true;
uint32_t g_ui32TrkcycSetting = TRKCYC_SET;
uint32_t g_lpmode_sel = 0;
float g_duration[MAX_TEST_NUM];
uint32_t g_ui32Cnt = 0;

//*****************************************************************************
//
// returns time in seconds
//
//*****************************************************************************
float get_time(void)
{
    //!<use ctimer
	uint32_t ui32TimerCount = am_hal_timer_read(0); //!< This API only supports timer0.
    static uint32_t count_last = 0;
    static float last_sec = 0;
    float full_scale_sec;
    float timer_offset;
    float fTimerInputFreq;
    uint32_t ui32DivideRatio, ui32Timer0Clock;
    ui32Timer0Clock = TIMER->CTRL0_b.TMR0CLK;
    if ((ui32Timer0Clock >= 1) || (ui32Timer0Clock <= 5))
    {
        ui32DivideRatio = (1U << ((ui32Timer0Clock + 1) << 1));
    }
    else
    {
        return 0; //!< This API only supports HFRC as timer input clock.
    }
    fTimerInputFreq = (float)AM_HAL_CLKGEN_FREQ_MAX_HZ / (float)ui32DivideRatio;
    full_scale_sec = (float)0xFFFFFFFF / fTimerInputFreq;
    timer_offset = (float)ui32TimerCount / fTimerInputFreq;
    if (count_last > ui32TimerCount)
    {
        //
        // overflow
        //
        count_last = ui32TimerCount;
        last_sec += full_scale_sec + timer_offset;
        return last_sec;
    }
    else
    {
        count_last = ui32TimerCount;
        return last_sec + timer_offset;
    }
}


//*****************************************************************************
//
// Interrupt handler for the AUDADC.
//
//*****************************************************************************
void
am_audadc0_isr(void)
{
    uint32_t ui32IntMask;
    float fCurTime = 0.0f;
    //
    // Read the interrupt status.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_interrupt_status(g_AUDADCHandle, &ui32IntMask, false))
    {
        am_util_stdio_printf("Error reading AUDADC interrupt status\n");
        bTestPass = false;
    }
    //
    // Clear the AUDADC interrupt.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_interrupt_clear(g_AUDADCHandle, ui32IntMask))
    {
        am_util_stdio_printf("Error clearing AUDADC interrupt status\n");
        bTestPass = false;
    }
    //
    // If we got a DMA complete, set the flag.
    //
    if (ui32IntMask & AM_HAL_AUDADC_INT_DCMP)
    {
        g_bAUDADCDMAComplete = true;
        fCurTime = get_time();
        if(g_calculatedTimer != 0)
        {
            g_duration[g_ui32Cnt++] = fCurTime - g_calculatedTimer;
        }
        g_calculatedTimer = fCurTime;
    }
    //
    // If we got a DMA error, set the flag.
    //
    if ( ui32IntMask & AM_HAL_AUDADC_INT_DERR )
    {
        g_bAUDADCDMAError = true;
    }
}

//*****************************************************************************
//
// Configure the AUDADC.
//
//*****************************************************************************
void
audadc_config_dma(void)
{
    am_hal_audadc_dma_config_t       AUDADCDMAConfig;
    //
    // Configure the AUDADC to use DMA for the sample transfer.
    //
    AUDADCDMAConfig.bDynamicPriority = true;
    AUDADCDMAConfig.ePriority = AM_HAL_AUDADC_PRIOR_SERVICE_IMMED;
    AUDADCDMAConfig.bDMAEnable = true;
    AUDADCDMAConfig.ui32SampleCount = AUDADC_SAMPLE_BUF_SIZE;
    AUDADCDMAConfig.ui32TargetAddress = (uint32_t) g_ui32AUDADCSampleBuffer;
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_dma(g_AUDADCHandle, &AUDADCDMAConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC DMA failed.\n");
        bTestPass = false;
    }
    //
    // Reset the AUDADC DMA flags.
    //
    g_bAUDADCDMAComplete = false;
    g_bAUDADCDMAError = false;
}

//*****************************************************************************
//
// Configure the AUDADC.
//
//*****************************************************************************
void
audadc_config_local(void)
{
    am_hal_audadc_config_t           AUDADCConfig;
    am_hal_audadc_slot_config_t      AUDADCSlotConfig;
    //
    // Initialize the AUDADC and get the handle.
    //
    if ( AM_HAL_STATUS_SUCCESS != am_hal_audadc_initialize(0, &g_AUDADCHandle) )
    {
        am_util_stdio_printf("Error - reservation of the AUDADC instance failed.\n");
        bTestPass = false;
    }
    //
    // Power on the AUDADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_power_control(g_AUDADCHandle,
                                                          AM_HAL_SYSCTRL_WAKE,
                                                          false) )
    {
        am_util_stdio_printf("Error - AUDADC power on failed.\n");
        bTestPass = false;
    }
    //
    // Set up internal repeat trigger timer
    //
    am_hal_audadc_irtt_config_t      AUDADCIrttConfig =
    {
        .bIrttEnable        = true,
        .eClkDiv            = (am_hal_audadc_irtt_clkdiv_e)CLKDIV,
        .ui32IrttCountMax   = TIMERMAX,

    };
    am_hal_audadc_configure_irtt(g_AUDADCHandle, &AUDADCIrttConfig);
    //
    // Set up the AUDADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    //
#if XTALHS_EN
    AUDADCConfig.eClock             = AM_HAL_AUDADC_CLKSEL_XTHS_32MHz;
#else
    am_util_stdio_printf("AM_HAL_AUDADC_CLKSEL_HFRC_48MHz\n");
    AUDADCConfig.eClock             = AM_HAL_AUDADC_CLKSEL_HFRC_48MHz;
#endif
    AUDADCConfig.ePolarity          = AM_HAL_AUDADC_TRIGPOL_RISING;
    AUDADCConfig.eTrigger           = AM_HAL_AUDADC_TRIGSEL_SOFTWARE;
    AUDADCConfig.eClockMode         = AM_HAL_AUDADC_CLKMODE_LOW_LATENCY;
    if (g_lpmode_sel == 0)
    {
      AUDADCConfig.ePowerMode         = AM_HAL_AUDADC_LPMODE0;
    }
    else
    {
      AUDADCConfig.ePowerMode         = AM_HAL_AUDADC_LPMODE1;
    }
    AUDADCConfig.eRepeat            = AM_HAL_AUDADC_REPEATING_SCAN;
    AUDADCConfig.eRepeatTrigger     = AM_HAL_AUDADC_RPTTRIGSEL_INT;
    AUDADCConfig.eSampMode          = AM_HAL_AUDADC_SAMPMODE_MED;

    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure(g_AUDADCHandle, &AUDADCConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC failed.\n");
        bTestPass = false;
    }
    //
    // Overwrite TrkCycMin for testing
    //
    extern uint32_t g_AUDADCSlotTrkCycMin;
    g_AUDADCSlotTrkCycMin = 7;
    //
    // Undo 2's complement offset configured by am_hal_audadc_configure
    //
    AUDADC->DATAOFFSET = ((AUDADC->DATAOFFSET & ~AUDADC_DATAOFFSET_OFFSET_Msk) | _VAL2FLD(AUDADC_DATAOFFSET_OFFSET, 0));
    //
    // Set up an AUDADC slot
    //
    AUDADCSlotConfig.eMeasToAvg      = AM_HAL_AUDADC_SLOT_AVG_1;
    AUDADCSlotConfig.ePrecisionMode  = AM_HAL_AUDADC_SLOT_12BIT;
    AUDADCSlotConfig.eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE0;
    AUDADCSlotConfig.bWindowCompare  = false;
    AUDADCSlotConfig.bEnabled        = true;
    AUDADCSlotConfig.ui32TrkCyc      = g_ui32TrkcycSetting;
    //AUDADCSlotConfig.ui32TrkCyc      = 7 * EXCESS_TRACKING_FACTOR; // need 12 (=5 + 7) cycles in 24 MHz mode, and 24 (=5 + 19) cycles in 48 MHz mode

    uint32_t status = 0;
#if CH_A0_EN
    status = am_hal_audadc_configure_slot(g_AUDADCHandle, 0, &AUDADCSlotConfig);
    if ( status == AM_HAL_STATUS_INVALID_HANDLE )
    {
      am_util_stdio_printf("Invalid Status Handle\n");
      bTestPass = false;
    }
    else if ( status == AM_HAL_STATUS_OUT_OF_RANGE )
    {
      am_util_stdio_printf("ui32SlotNumber 0 greater than AM_HAL_AUDADC_MAX_SLOTS=%d\n", AM_HAL_AUDADC_MAX_SLOTS);
      am_util_stdio_printf("--OR-- ui32TrkCyc %d less than g_AUDADCSlotTrkCycMin=%d\n", AUDADCSlotConfig.ui32TrkCyc, g_AUDADCSlotTrkCycMin);
      bTestPass = false;
    }
    if (AM_HAL_STATUS_SUCCESS != status)
    {
        am_util_stdio_printf("Error - configuring AUDADC Slot 0 failed.\n");
        bTestPass = false;
    }
#endif

#if CH_A1_EN
    AUDADCSlotConfig.eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE1;
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_slot(g_AUDADCHandle, 1, &AUDADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC Slot 1 failed.\n");
        bTestPass = false;
    }
#endif

#if CH_B0_EN
    AUDADCSlotConfig.eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE2;
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_slot(g_AUDADCHandle, 2, &AUDADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC Slot 2 failed.\n");
        bTestPass = false;
    }
#endif

#if CH_B1_EN
    AUDADCSlotConfig.eChannel        = AM_HAL_AUDADC_SLOT_CHSEL_SE3;
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_configure_slot(g_AUDADCHandle, 3, &AUDADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring AUDADC Slot 3 failed.\n");
        bTestPass = false;
    }
#endif
    //
    // Configure the AUDADC to use DMA for the sample transfer.
    //
    audadc_config_dma();
    //
    // For this example, the samples will be coming in slowly. This means we
    // can afford to wake up for every conversion.
    //
    am_hal_audadc_interrupt_enable(g_AUDADCHandle, AM_HAL_AUDADC_INT_DCMP);
    //
    // Enable the AUDADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_enable(g_AUDADCHandle))
    {
        am_util_stdio_printf("Error - enabling AUDADC failed.\n");
        bTestPass = false;
    }
    //
    // Enable internal repeat trigger timer
    //
    am_hal_audadc_irtt_enable(g_AUDADCHandle);
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
bool
audadc_dmacpl_test(void)
{
    am_hal_timer_config_t sTimerConfig;
    uint32_t ui32Status;
    ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize a timer configuration structure with default values!\n");
        bTestPass = false;
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;
    ui32Status = am_hal_timer_config(0, &sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure a timer!\n");
        bTestPass = false;
    }
    ui32Status = am_hal_timer_start(0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to start a timer!\n");
        bTestPass = false;
    }
    uint32_t autocal_disable = 0;
    autocal_disable = (~AUTOCAL_EN & 0x1);
    g_lpmode_sel = LPMODE_SET;
    g_ui32TrkcycSetting = TRKCYC_SET;
    //
    // Start the ITM interface.
    //
    am_bsp_itm_printf_enable();
    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("AUDADC Example with 48 kHz LPMODE=%d\n", g_lpmode_sel);
    am_util_stdio_printf("autocal_disable=%d\n", autocal_disable);

#if HFADJ_EN == 1
    am_util_stdio_printf("Enabling the 32KHz Crystal\n");

    //
    // Wait for 1 second for the 32KHz XTAL to startup and stabilize.
    //
    am_util_delay_ms(1000);
    //
    // Enable HFADJ.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFADJ_ENABLE, 0);
#else
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_HFADJ_DISABLE, 0);
#endif
    //
    // power up PGA reference generator
    //
    am_hal_audadc_refgen_powerup();
    //
    // power up PGA for the enabled channels
    //
    #if (CH_A0_EN)
    {
        am_hal_audadc_pga_powerup(0);
    }
    #endif

    #if (CH_A1_EN)
    {
        am_hal_audadc_pga_powerup(1);
    }
    #endif

    #if (CH_B0_EN)
    {
        am_hal_audadc_pga_powerup(2);
    }
    #endif

    #if (CH_B1_EN)
    {
        am_hal_audadc_pga_powerup(3);
    }
    #endif
    //
    // Enable op amps for full gain range
    //
    MCUCTRL->PGACTRL1 = 0x0df00000;
    #if (CH_B0_EN && CH_B1_EN)
    MCUCTRL->PGACTRL2 = 0x01f00000;
    #endif

    if (autocal_disable == 0)
    {
      MCUCTRL->ADCCAL_b.CALONPWRUP = 1;
    }
    else
    {
      MCUCTRL->ADCCAL_b.CALONPWRUP = 0;
    }
    //
    // power_up_audadc
    //
    PWRCTRL->AUDSSPWREN = ((PWRCTRL->AUDSSPWREN & ~PWRCTRL_AUDSSPWREN_PWRENAUDADC_Msk) | _VAL2FLD(PWRCTRL_AUDSSPWREN_PWRENAUDADC, 1));
    while(_FLD2VAL(PWRCTRL_AUDSSPWRSTATUS_PWRSTAUDADC, PWRCTRL->AUDSSPWRSTATUS)==0);
    //
    // wait a little
    //
    am_util_delay_ms(500);
    //
    // Configure the AUDADC
    //
    audadc_config_local();
    //
    // Enable interrupts.
    //
    NVIC_EnableIRQ(AUDADC0_IRQn);
    am_hal_interrupt_master_enable();
    //
    // Gain Config
    //
    am_hal_audadc_gain_config_t AUDADCGainConfig;

    AUDADCGainConfig.ui32LGA = (uint32_t)((float)CH_A0_GAIN_DB*2 + 12);
    AUDADCGainConfig.ui32HGADELTA = ((uint32_t)((float)CH_A1_GAIN_DB*2 + 12)) - AUDADCGainConfig.ui32LGA;
    #if (CH_B0_EN && CH_B1_EN)
    AUDADCGainConfig.ui32LGB = (uint32_t)((float)CH_B0_GAIN_DB*2 + 12);
    AUDADCGainConfig.ui32HGBDELTA = ((uint32_t)((float)CH_B1_GAIN_DB*2 + 12)) - AUDADCGainConfig.ui32LGB;
    #endif
    AUDADCGainConfig.eUpdateMode = AM_HAL_AUDADC_GAIN_UPDATE_IMME;
    //
    // Set PGA gains
    //
    am_hal_audadc_internal_pga_config(g_AUDADCHandle, &AUDADCGainConfig);
    am_util_delay_ms(250);
    //
    // Trigger the AUDADC sampling for the first time manually.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_sw_trigger(g_AUDADCHandle))
    {
        am_util_stdio_printf("Error - triggering the AUDADC failed.\n");
    }

    while(1)
    {
        //
        // Check for DMA errors.
        //
        if (g_bAUDADCDMAError)
        {
            am_util_stdio_printf("DMA Error occured\n");
            bTestPass = false;
            TEST_ASSERT_TRUE(bTestPass);
            return false;
        }
        //
        // Check if the AUDADC DMA completion interrupt occurred.
        //
        if (g_bAUDADCDMAComplete)
        {
            //
            // Reset the DMA completion and error flags.
            //
            g_bAUDADCDMAComplete = false;
            //
            // Re-configure the AUDADC DMA.
            //
            audadc_config_dma();
            //
            // Clear the AUDADC interrupts.
            //
            if (AM_HAL_STATUS_SUCCESS != am_hal_audadc_interrupt_clear(g_AUDADCHandle, 0xFFFFFFFF))
            {
                am_util_stdio_printf("Error - clearing the AUDADC interrupts failed.\n");
                bTestPass = false;
                TEST_ASSERT_TRUE(bTestPass);
                return false;
            }
            if (g_ui32Cnt >= MAX_TEST_NUM)
            {
                break;
            }
            if ((get_time() > TIMEOUT) && (g_ui32Cnt < MAX_TEST_NUM))
            {
                am_util_stdio_printf("Error - Timeout.\n");
                bTestPass = false;
                TEST_ASSERT_TRUE(bTestPass);
                return false;
            }
        }
    }
    //
    // Check if DMA complete interrupt was missing
    //
    for (g_ui32Cnt = 0; g_ui32Cnt < MAX_TEST_NUM; g_ui32Cnt++)
    {
        am_util_stdio_printf("%d - The time between current interrupt and last interrupt - %f Seconds.\n", g_ui32Cnt, g_duration[g_ui32Cnt]);
        if (g_duration[g_ui32Cnt] > MAX_DURATION)
        {
            am_util_stdio_printf("ERROR - 1 interrupt was missing at least.\n");
            bTestPass = false;
            TEST_ASSERT_TRUE(bTestPass);
            return false;
        }
    }
    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;
}
