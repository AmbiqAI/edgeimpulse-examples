//*****************************************************************************
//
//! @file adc_measure_test_cases.c
//!
//! @brief This testcase uses the CTIMER and IOM #0 in I2C mode to create digital
//! samples on the MCP4725 DAC. DAC output voltage increases from 0mv to a maximum 
//! of 1190mv with a 50mv step, then decreases to 0mv in the opposite way.
//!
//! GPIO  5 - DAC SCL
//! GPIO  6 - DAC SDA
//! GPIO 13 - DAC OUT
//! GPIO 14 - DAC OUT
//! GPIO 15 - DAC OUT
//! GPIO 18 - DAC OUT
//!    3.3v - DAC VCC
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <math.h>
#include "unity.h"
#include "am_mcu_apollo.h"
#include "test_control_array.h"

// #### INTERNAL BEGIN ####
#define TIMING_CHECK
#define CHECK_PIN           (79)
// #### INTERNAL END ####

#define ADC_INPUT_PIN13     (13)
#define ADC_INPUT_PIN14     (14)
#define ADC_INPUT_PIN15     (15)
#define ADC_INPUT_PIN18     (18)

#define ADC_CHANNEL_NUM     (4)
#define SAMPLE_PER_CHANNEL  (16)
#define ADC_TOTAL_SAMPLE    ((ADC_CHANNEL_NUM)*(SAMPLE_PER_CHANNEL))
#define ACCURACY_TOLERANCE  ((0.01f)*(AM_HAL_ADC_VREFMV))

#define NUM_OF_DAC          (10)

typedef struct
{
    int16_t i2cValue;
    float   volt_mv[NUM_OF_DAC];
} dac_step_t;

int g_stepIndex = 0;
int dacIndex = 0;

//
// DAC setting value and corresponding output voltage in mv, 10 pices.
//
dac_step_t g_sDacTables[] = 
{
    {0x000, {   4.36f,    1.16f,    1.15f,    1.56f,    1.11f,    1.12f,    1.11f,    1.08f,    1.13f,    1.07f}},
    {0x040, {  56.47f,   56.47f,   54.55f,   54.82f,   52.40f,   53.63f,   47.41f,   51.57f,   50.47f,   54.39f}},
    {0x080, { 102.40f,   93.43f,  108.44f,  101.33f,   98.62f,  101.59f,  104.38f,   96.01f,   97.27f,  111.31f}},
    {0x0C0, { 158.50f,  157.78f,  157.54f,  159.06f,  156.49f,  155.62f,  149.53f,  153.50f,  153.64f,  156.21f}},
    {0x100, { 205.30f,  196.38f,  212.36f,  205.10f,  203.21f,  203.82f,  207.24f,  198.39f,  200.65f,  214.04f}},
    {0x140, { 261.91f,  261.63f,  260.18f,  261.83f,  260.87f,  257.36f,  253.18f,  256.38f,  256.92f,  259.42f}},
    {0x180, { 309.40f,  298.80f,  314.68f,  307.91f,  307.17f,  307.06f,  309.94f,  300.72f,  303.54f,  317.42f}},
    {0x1C0, { 365.58f,  364.49f,  363.50f,  365.95f,  364.04f,  360.47f,  355.19f,  359.00f,  360.07f,  362.93f}},
    {0x200, { 414.80f,  403.90f,  419.12f,  412.93f,  412.11f,  410.57f,  414.30f,  405.66f,  407.81f,  420.73f}},
    {0x240, { 470.66f,  469.04f,  468.08f,  469.64f,  469.29f,  464.16f,  460.80f,  463.90f,  463.71f,  466.33f}},
    {0x280, { 517.10f,  506.89f,  522.08f,  515.80f,  515.50f,  513.08f,  518.61f,  509.00f,  509.87f,  523.55f}},
    {0x2C0, { 573.68f,  571.66f,  570.38f,  572.27f,  572.42f,  567.00f,  564.97f,  567.19f,  566.29f,  568.95f}},
    {0x300, { 619.33f,  609.77f,  625.11f,  619.15f,  617.82f,  615.25f,  621.79f,  612.43f,  613.33f,  626.81f}},
    {0x340, { 675.98f,  674.10f,  674.58f,  676.41f,  675.08f,  668.51f,  666.84f,  670.70f,  668.62f,  671.84f}},
    {0x380, { 722.04f,  711.94f,  728.97f,  722.95f,  721.05f,  717.17f,  723.72f,  715.78f,  715.28f,  727.81f}},
    {0x3C0, { 778.18f,  776.36f,  776.77f,  781.04f,  778.41f,  769.51f,  770.30f,  773.68f,  771.86f,  772.61f}},
    {0x400, { 826.08f,  815.22f,  832.98f,  829.07f,  826.13f,  819.96f,  830.73f,  820.47f,  820.52f,  831.64f}},
    {0x440, { 882.69f,  879.86f,  882.05f,  885.41f,  884.28f,  873.26f,  876.31f,  878.50f,  876.66f,  876.21f}},
    {0x480, { 929.82f,  918.13f,  935.65f,  931.69f,  930.17f,  921.40f,  933.54f,  923.78f,  922.82f,  934.02f}},
    {0x4C0, { 986.39f,  982.58f,  984.50f,  988.87f,  987.31f,  974.50f,  979.71f,  982.34f,  978.40f,  979.54f}},
    {0x500, {1032.04f, 1020.21f, 1038.67f, 1034.92f, 1033.47f, 1023.51f, 1036.70f, 1027.21f, 1025.32f, 1037.67f}},
    {0x540, {1088.73f, 1084.88f, 1087.79f, 1092.40f, 1089.47f, 1076.69f, 1081.71f, 1083.57f, 1081.70f, 1082.91f}},
    {0x580, {1134.32f, 1121.99f, 1143.42f, 1139.12f, 1135.59f, 1124.72f, 1138.53f, 1128.18f, 1128.37f, 1140.36f}},
    {0x5C0, {1190.54f, 1187.53f, 1191.47f, 1195.72f, 1192.76f, 1177.77f, 1184.17f, 1186.16f, 1184.95f, 1185.86f}},
    {0x580, {1134.36f, 1121.97f, 1143.39f, 1139.09f, 1135.58f, 1124.76f, 1138.52f, 1128.15f, 1128.43f, 1140.35f}},
    {0x540, {1088.73f, 1084.86f, 1087.83f, 1092.35f, 1089.46f, 1076.67f, 1081.69f, 1083.54f, 1081.71f, 1082.87f}},
    {0x500, {1031.99f, 1020.18f, 1038.62f, 1034.85f, 1033.49f, 1023.50f, 1036.66f, 1027.22f, 1025.30f, 1037.65f}},
    {0x4C0, { 986.36f,  982.59f,  984.53f,  988.82f,  987.32f,  974.49f,  979.67f,  982.34f,  978.36f,  979.49f}},
    {0x480, { 929.79f,  918.10f,  935.65f,  931.68f,  930.13f,  921.44f,  933.51f,  923.74f,  922.77f,  934.01f}},
    {0x440, { 882.76f,  879.78f,  882.07f,  885.39f,  884.27f,  873.27f,  876.27f,  878.49f,  876.56f,  876.20f}},
    {0x400, { 826.17f,  815.19f,  833.02f,  829.09f,  826.07f,  819.96f,  830.69f,  820.44f,  820.45f,  831.64f}},
    {0x3C0, { 778.22f,  776.31f,  776.79f,  781.05f,  778.37f,  769.49f,  770.27f,  773.66f,  771.84f,  772.58f}},
    {0x380, { 722.04f,  711.90f,  729.01f,  722.96f,  721.02f,  717.20f,  723.72f,  715.77f,  715.24f,  727.77f}},
    {0x340, { 676.00f,  674.09f,  674.61f,  676.44f,  675.06f,  668.48f,  666.85f,  670.67f,  668.61f,  671.79f}},
    {0x300, { 619.33f,  609.75f,  625.12f,  619.13f,  617.79f,  615.24f,  621.77f,  612.39f,  613.29f,  626.80f}},
    {0x2C0, { 573.72f,  571.62f,  570.39f,  572.27f,  572.42f,  566.98f,  564.97f,  567.15f,  566.31f,  568.93f}},
    {0x280, { 517.11f,  506.91f,  522.09f,  515.80f,  515.52f,  513.10f,  518.61f,  508.97f,  509.83f,  523.56f}},
    {0x240, { 470.68f,  469.03f,  468.07f,  469.63f,  469.30f,  464.16f,  460.83f,  463.83f,  463.70f,  466.34f}},
    {0x200, { 414.78f,  403.88f,  419.15f,  412.94f,  412.08f,  410.58f,  414.29f,  405.65f,  407.80f,  420.70f}},
    {0x1C0, { 365.57f,  364.47f,  363.52f,  365.95f,  364.01f,  360.45f,  355.22f,  359.00f,  360.07f,  362.91f}},
    {0x180, { 309.43f,  298.81f,  314.67f,  307.93f,  307.09f,  307.10f,  309.97f,  300.68f,  303.56f,  317.45f}},
    {0x140, { 261.97f,  261.63f,  260.15f,  261.85f,  260.82f,  257.36f,  253.20f,  256.35f,  256.90f,  259.44f}},
    {0x100, { 205.31f,  196.39f,  212.37f,  205.14f,  203.15f,  203.82f,  207.26f,  198.38f,  200.59f,  214.04f}},
    {0x0C0, { 158.54f,  157.78f,  157.52f,  159.09f,  156.43f,  155.63f,  149.54f,  153.52f,  153.66f,  156.23f}},
    {0x080, { 102.41f,   93.42f,  108.44f,  101.33f,   98.56f,  101.58f,  104.36f,   96.01f,   97.29f,  111.31f}},
    {0x040, {  56.49f,   56.43f,   54.53f,   54.83f,   52.43f,   53.61f,   47.40f,   51.57f,   50.53f,   54.43f}},
    {0x000, {   4.35f,    1.16f,    1.15f,    1.57f,    1.09f,    1.12f,    1.12f,    1.08f,    1.13f,    1.07f}},
};

//
// ADC device handle.
//
static void *g_ADCHandle;

//
// ADC DMA complete flag.
//
int dma_complete = 0;

//
// Timer trigger flag.
//
int timer_trigger = 0;

//
// ADC Sample buffer.
//
uint32_t             g_ui32ADCSampleBuffer[ADC_TOTAL_SAMPLE];
am_hal_adc_sample_t  slotsSample[ADC_TOTAL_SAMPLE];

//
// IOM device handle.
//
static void *g_pIOMHandle;

//
// IOM buffer and queue.
//
am_hal_iom_buffer(16)           g_sRamBuffer;

//
// IOM Configuration.
//
static am_hal_iom_config_t g_sIOMSpiConfig =
{
    .eInterfaceMode     = AM_HAL_IOM_I2C_MODE,
    .ui32ClockFreq      = AM_HAL_IOM_400KHZ,
    .pNBTxnBuf          = NULL,
    .ui32NBTxnBufLength = 0
};

//
// Set up an array to use for retrieving the correction trim values.
//
float g_fTrims[4];

const am_hal_gpio_pincfg_t g_ADC_PIN13_CFG =
{
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_13_ADCSE6,
};

const am_hal_gpio_pincfg_t g_ADC_PIN14_CFG =
{
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_14_ADCSE5,
};

const am_hal_gpio_pincfg_t g_ADC_PIN15_CFG =
{
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_15_ADCSE4,
};

const am_hal_gpio_pincfg_t g_ADC_PIN18_CFG =
{
    .GP.cfg_b.uFuncSel       = AM_HAL_PIN_18_ADCSE1,
};


//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void globalSetUp(void)
{
    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Let host know we are ready.
    //
    sControlArray.g_ui32ControlWord = 1;

    //
    // Start the ITM interface.
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 ADC Measurement Test Cases with DAC MCP4725\n\n");

}

void globalTearDown(void)
{
}


//*****************************************************************************
//
// Interrupt handler for CTimer
//
//*****************************************************************************
void am_ctimer_isr(void)
{
    uint32_t ui32Status;

    am_hal_timer_interrupt_status_get(true, &ui32Status);
    am_hal_timer_interrupt_clear(ui32Status);

    timer_trigger = 1;
    // #### INTERNAL BEGIN ####
    #ifdef TIMING_CHECK
    am_hal_gpio_state_write(CHECK_PIN, AM_HAL_GPIO_OUTPUT_SET);
    #endif
    // #### INTERNAL END ####
}

//*****************************************************************************
//
// ADC Interrupt Service Routine (ISR)
//
//*****************************************************************************
void am_adc_isr(void)
{
    uint32_t ui32IntStatus;

    //
    // Clear the ADC interrupt.
    //
    am_hal_adc_interrupt_status(g_ADCHandle, &ui32IntStatus, true);
    am_hal_adc_interrupt_clear(g_ADCHandle, ui32IntStatus);

    // #### INTERNAL BEGIN ####
    #ifdef TIMING_CHECK
    am_hal_gpio_state_write(CHECK_PIN, AM_HAL_GPIO_OUTPUT_SET);
    #endif
    // #### INTERNAL END ####
    uint32_t ui32NumSamples = ADC_TOTAL_SAMPLE;

    if ((ui32IntStatus & AM_HAL_ADC_INT_FIFOOVR2) || (ui32IntStatus & AM_HAL_ADC_INT_DCMP))
    {
        am_hal_adc_samples_read(g_ADCHandle, false, g_ui32ADCSampleBuffer, &ui32NumSamples, &slotsSample[0]);
    }

    dma_complete = 1;
}

//*****************************************************************************
//
// Send DAC Command
//
//*****************************************************************************
void sendDACCommand(uint32_t ui32Address, uint16_t ui16Value)
{
    am_hal_iom_transfer_t       Transaction;

    //
    // Scale the value into a 2-byte 12-bit format.
    //
    g_sRamBuffer.bytes[0] = (ui16Value & 0x0F00) >> 8;
    g_sRamBuffer.bytes[1] = (ui16Value & 0xFF);

    //
    // Create the transaction and send the data.
    Transaction.uPeerInfo.ui32I2CDevAddr  = ui32Address;
    Transaction.ui32InstrLen              = 0;
#if defined(AM_PART_APOLLO4B) || defined(AM_PART_APOLLO4P) || defined(AM_PART_APOLLO4L)
    Transaction.ui64Instr = 0;
#else
    Transaction.ui32Instr = 0;
#endif
    Transaction.eDirection                = AM_HAL_IOM_TX;
    Transaction.ui32NumBytes              = 2;
    Transaction.pui32TxBuffer             = g_sRamBuffer.words;
    Transaction.bContinue                 = false;
    Transaction.ui8RepeatCount            = 0;
    Transaction.ui32PauseCondition        = 0;
    Transaction.ui32StatusSetClr          = 0;
    am_hal_iom_blocking_transfer(g_pIOMHandle, &Transaction);
}


//*****************************************************************************
//
// IOM Initialize
//
//*****************************************************************************
void initialize_iom(uint32_t ui32Module)
{
    //
    // Initialize the IOM.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_iom_initialize(ui32Module,&g_pIOMHandle))
    {
        am_util_stdio_printf("Error - failed to initialize the IOM instance\n");
    }

    //
    // Turn on the IOM.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_iom_power_ctrl(g_pIOMHandle, AM_HAL_SYSCTRL_WAKE, false))
    {
        am_util_stdio_printf("Error - failed to power on the IOM\n");
    }

    //
    // Configure IOM #0.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_iom_configure(g_pIOMHandle, &g_sIOMSpiConfig))
    {
        am_util_stdio_printf("Error - failed to configure the IOM\n");
    }

    //
    // Set up IOM #0 interrupts.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_iom_interrupt_enable(g_pIOMHandle, AM_HAL_IOM_INT_CMDCMP))
    {
        am_util_stdio_printf("Error - failed to enable the IOM interrupts\n");
    }

    //
    // Enable IOM #0.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_iom_enable(g_pIOMHandle))
    {
        am_util_stdio_printf("Error - failed to enable the IOM\n");
    }

    am_bsp_iom_pins_enable(0, AM_HAL_IOM_I2C_MODE);
}


//*****************************************************************************
//
// Init function for Timer.
//
//*****************************************************************************
void timer_init(void)
{
    am_hal_timer_config_t       TimerConfig;

    //
    // Set up TIMER0
    //
    am_hal_timer_default_config_set(&TimerConfig);
    TimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    TimerConfig.ui32PatternLimit = 0;
    TimerConfig.ui32Compare0 = 600000;  // Default Clock is HFRC/16 or 6MHz.  0.1 sec.
    if (AM_HAL_STATUS_SUCCESS != am_hal_timer_config(0, &TimerConfig))
    {
      am_util_stdio_printf("Failed to configure TIMER0\n");
    }
    am_hal_timer_clear(0);

    //
    // Clear and enable the timer interrupt
    //
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE0));
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE0));

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_EnableIRQ(TIMER_IRQn);
}


//*****************************************************************************
//
// ADC INIT Function
//
//*****************************************************************************
void adc_init(void)
{
    am_hal_adc_config_t           ADCConfig;
    am_hal_adc_slot_config_t      ADCSlotConfig;

    //
    // Initialize the ADC and get the handle.
    //
    if ( am_hal_adc_initialize(0, &g_ADCHandle) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("Error - reservation of the ADC instance failed.\n");
    }

    //
    // Get the ADC correction offset and gain for this DUT.
    // Note that g_fTrims[3] must contain the value -123.456f before calling
    // the function.
    // On return g_fTrims[0] contains the offset, g_fTrims[1] the gain.
    //
    g_fTrims[0] = g_fTrims[1] = g_fTrims[2] = 0.0F;
    g_fTrims[3] = -123.456f;
    am_hal_adc_control(g_ADCHandle, AM_HAL_ADC_REQ_CORRECTION_TRIMS_GET, g_fTrims);

    //
    // Power on the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
                                                          AM_HAL_SYSCTRL_WAKE,
                                                          false) )
    {
        am_util_stdio_printf("Error - ADC power on failed.\n");
    }

    //
    // Set up internal repeat trigger timer
    //
    am_hal_adc_irtt_config_t      ADCIrttConfig =
    {
        .bIrttEnable        = true,
        .eClkDiv            = AM_HAL_ADC_RPTT_CLK_DIV16,
        .ui32IrttCountMax   = 999,
    };

    am_hal_adc_configure_irtt(g_ADCHandle, &ADCIrttConfig);

    //
    // Set up the ADC configuration parameters. These settings are reasonable
    // for accurate measurements at a low sample rate.
    //
    ADCConfig.eClock             = AM_HAL_ADC_CLKSEL_HFRC_24MHZ;
    ADCConfig.ePolarity          = AM_HAL_ADC_TRIGPOL_RISING;
    ADCConfig.eTrigger           = AM_HAL_ADC_TRIGSEL_SOFTWARE;
    ADCConfig.eClockMode         = AM_HAL_ADC_CLKMODE_LOW_LATENCY;
    ADCConfig.ePowerMode         = AM_HAL_ADC_LPMODE0;
    ADCConfig.eRepeat            = AM_HAL_ADC_REPEATING_SCAN;
    ADCConfig.eRepeatTrigger     = AM_HAL_ADC_RPTTRIGSEL_INT;
    if ( am_hal_adc_configure(g_ADCHandle, &ADCConfig) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("Error - configuring ADC failed.\n");
    }

    //
    // Set up an ADC slot
    //
    #ifdef  ADC_INPUT_PIN13
    am_hal_gpio_pinconfig(ADC_INPUT_PIN13, g_ADC_PIN13_CFG);
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;
    ADCSlotConfig.ui32TrkCyc      = AM_HAL_ADC_MIN_TRKCYC;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_12BIT;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE6;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 0, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 0 failed.\n");
    }
    #endif

    #ifdef ADC_INPUT_PIN14
    am_hal_gpio_pinconfig(ADC_INPUT_PIN14, g_ADC_PIN14_CFG);
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;
    ADCSlotConfig.ui32TrkCyc      = AM_HAL_ADC_MIN_TRKCYC;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_12BIT;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE5;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 1, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 1 failed.\n");
    }
    #endif

    #ifdef ADC_INPUT_PIN15
    am_hal_gpio_pinconfig(ADC_INPUT_PIN15, g_ADC_PIN15_CFG);
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;
    ADCSlotConfig.ui32TrkCyc      = AM_HAL_ADC_MIN_TRKCYC;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_12BIT;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE4;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 2, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 2 failed.\n");
    }
    #endif

    #ifdef ADC_INPUT_PIN18
    am_hal_gpio_pinconfig(ADC_INPUT_PIN18, g_ADC_PIN18_CFG);
    ADCSlotConfig.eMeasToAvg      = AM_HAL_ADC_SLOT_AVG_1;
    ADCSlotConfig.ui32TrkCyc      = AM_HAL_ADC_MIN_TRKCYC;
    ADCSlotConfig.ePrecisionMode  = AM_HAL_ADC_SLOT_12BIT;
    ADCSlotConfig.eChannel        = AM_HAL_ADC_SLOT_CHSEL_SE1;
    ADCSlotConfig.bWindowCompare  = false;
    ADCSlotConfig.bEnabled        = true;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_slot(g_ADCHandle, 3, &ADCSlotConfig))
    {
        am_util_stdio_printf("Error - configuring ADC Slot 3 failed.\n");
    }
    #endif

    //
    // Configure the ADC to use DMA for the sample transfer.
    //
    am_hal_adc_dma_config_t ADCDMAConfig;
    ADCDMAConfig.bDynamicPriority = true;
    ADCDMAConfig.ePriority = AM_HAL_ADC_PRIOR_SERVICE_IMMED;
    ADCDMAConfig.bDMAEnable = true;
    ADCDMAConfig.ui32SampleCount = ADC_TOTAL_SAMPLE;
    ADCDMAConfig.ui32TargetAddress = (uint32_t)g_ui32ADCSampleBuffer;
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_configure_dma(g_ADCHandle, &ADCDMAConfig))
    {
        am_util_stdio_printf("Error - configuring ADC DMA failed.\n");
    }

    //
    // Enable FIFO and DMA interrupt.
    // 
    am_hal_adc_interrupt_enable(g_ADCHandle, AM_HAL_ADC_INT_FIFOOVR2 | AM_HAL_ADC_INT_DERR | AM_HAL_ADC_INT_DCMP); // AM_HAL_ADC_INT_CNVCMP | AM_HAL_ADC_INT_SCNCMP |

    //
    // Enable the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_enable(g_ADCHandle))
    {
        am_util_stdio_printf("Error - enabling ADC failed.\n");
    }

    //
    // Enable internal repeat trigger timer
    //
    am_hal_adc_irtt_enable(g_ADCHandle);

    //
    // Enable the ADC interrupt in the NVIC.
    //
    NVIC_EnableIRQ(ADC_IRQn);
}


//*****************************************************************************
//
// ADC DEINIT Function
//
//*****************************************************************************
void adc_deinit()
{
    //
    // Disable the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_disable(g_ADCHandle))
    {
        am_util_stdio_printf("Error - disabling ADC failed.\n");
    }

    //
    // Power on the ADC.
    //
    if (AM_HAL_STATUS_SUCCESS != am_hal_adc_power_control(g_ADCHandle,
                                                          AM_HAL_SYSCTRL_DEEPSLEEP,
                                                          false) )
    {
        am_util_stdio_printf("Error - ADC power off failed.\n");
    }

    NVIC_DisableIRQ(ADC_IRQn);
    am_hal_adc_deinitialize(g_ADCHandle);
}


bool check_data()
{
    for (int i = 0; i < ADC_TOTAL_SAMPLE; i++)
    {
        float sample_value = ((float)(slotsSample[i].ui32Sample)) * AM_HAL_ADC_VREFMV / 0x1000;
        float delta = fabsf(sample_value - g_sDacTables[g_stepIndex].volt_mv[dacIndex]);
        if (delta >= ACCURACY_TOLERANCE)
        {
            am_util_stdio_printf("Step %d, slot %d failed, %.2f - %.2f = %.2f.\n", g_stepIndex, slotsSample[i].ui32Slot, \
                                 sample_value, g_sDacTables[g_stepIndex].volt_mv[dacIndex], delta);
            return false;
        }
    }

    am_util_stdio_printf("Step %d passed.\n", g_stepIndex);
    return true;
}


void adc_measure_test(void)
{
    // #### INTERNAL BEGIN ####
    #ifdef TIMING_CHECK
    am_hal_gpio_pinconfig(CHECK_PIN, am_hal_gpio_pincfg_output);
    am_hal_gpio_state_write(CHECK_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
    #endif
    // #### INTERNAL END ####
    //
    // Waiting for host to tell us the index number of DAC
    //
    while (sControlArray.g_ui32ControlWord == 1) {};

    dacIndex = sControlArray.g_ui32InputData[0];
    if ((dacIndex < 0) || (dacIndex >= NUM_OF_DAC))
    {
        TEST_FAIL_MESSAGE("Error. Wrong DAC Index.");
        return;
    }
    
    am_util_stdio_printf("Start test with DAC: %d\n", dacIndex);

    //
    // Initialize I2C, and reset DAC
    //
    initialize_iom(0);
    sendDACCommand(0x60, 0x0);

    //
    // Set Timer ISR triggers every 100ms.
    //
    timer_init();

    //
    // Enable interrupts.
    //
    am_hal_interrupt_master_enable();

    for (g_stepIndex = 0; g_stepIndex < sizeof(g_sDacTables)/sizeof(dac_step_t); g_stepIndex++)
    {
        //
        // Waiting for the timer trigger.
        //
        while (timer_trigger == 0) {};
        timer_trigger = 0;

        //
        // Send I2C command and set DAC value.
        //
        sendDACCommand(0x60, g_sDacTables[g_stepIndex].i2cValue);

        //
        // Waiting for DAC to stabilize output, then initialize ADC
        //
        am_util_delay_ms(20);
        // #### INTERNAL BEGIN ####
        #ifdef TIMING_CHECK
        am_hal_gpio_state_write(CHECK_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
        #endif
        // #### INTERNAL END ####
        adc_init();

        //
        // Waiting for ADC to get enough samples
        //
        while (dma_complete == 0) {};
        dma_complete = 0;

        //
        // Deinitialize ADC and check ADC samples.
        //
        adc_deinit();
        TEST_ASSERT_TRUE(check_data());
        // #### INTERNAL BEGIN ####
        #ifdef TIMING_CHECK
        am_hal_gpio_state_write(CHECK_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
        #endif
        // #### INTERNAL END ####
    }
}
