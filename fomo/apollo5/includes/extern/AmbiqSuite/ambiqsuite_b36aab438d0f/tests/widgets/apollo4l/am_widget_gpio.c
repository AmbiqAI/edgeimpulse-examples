//*****************************************************************************
//
//! @file am_widget_gpio.c
//!
//! @brief This widget allows test cases to exercise the Apollo4 GPIO HAL.
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

#include "am_mcu_apollo.h"
#include "am_widget_gpio.h"
#include "am_bsp.h"
#include "am_util.h"
#include "unity.h"

//*****************************************************************************
//
// Look for errors in the HAL.
//
//*****************************************************************************


//*****************************************************************************
//
// Tracking variable for error status.
//
//*****************************************************************************
uint32_t g_ui32ErrorStatus = 0;

uint32_t g_ui32ch0_001f = 0;
uint32_t g_ui32ch0_203f = 0;
uint32_t g_ui32ch0_405f = 0;
uint32_t g_ui32ch0_607f = 0;
uint32_t g_ui32ch1_001f = 0;
uint32_t g_ui32ch1_203f = 0;
uint32_t g_ui32ch1_405f = 0;
uint32_t g_ui32ch1_607f = 0;

//*****************************************************************************
//
// Pin configurations.
//
//*****************************************************************************
am_hal_gpio_pincfg_t g_GPIO_INPUT_INT =
{
    .GP.cfg_b.uFuncSel         = 3,
    .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_ENABLE,
    .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_NONE,   // LO2HI, HI2LO, BOTH
    .GP.cfg_b.uSlewRate        = 0,
    .GP.cfg_b.uNCE             = 0,
    .GP.cfg_b.eCEpol           = 0,
    .GP.cfg_b.ePowerSw         = 0,
    .GP.cfg_b.eForceInputEn    = 0,
    .GP.cfg_b.eForceOutputEn   = 0,
    .GP.cfg_b.uRsvd_0          = 0,
    .GP.cfg_b.uRsvd_1          = 0
};


am_hal_gpio_pincfg_t g_GPIO_OUTPUT_READ =
{
        .GP.cfg_b.uFuncSel         = 3,
        .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
        .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
        .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
        .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_NONE,
        .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
        .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_NONE,
        .GP.cfg_b.uSlewRate        = 0,
        .GP.cfg_b.uNCE             = 0,
        .GP.cfg_b.eCEpol           = 0,
        .GP.cfg_b.ePowerSw         = 0,
        .GP.cfg_b.eForceInputEn    = 0,
        .GP.cfg_b.eForceOutputEn   = 0,
        .GP.cfg_b.uRsvd_0          = 0,
        .GP.cfg_b.uRsvd_1          = 0
};

//*****************************************************************************
//
// GPIO interrupt handlers
//
//*****************************************************************************
void
am_gpio0_001f_isr(void)
{
    if(GPIO->MCUN0INT0STAT)
    {
        g_ui32ch0_001f = GPIO->MCUN0INT0STAT;
    }
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT0CLR)),GPIO->MCUN0INT0STAT, REG_EQUAL_MASK);
}

void
am_gpio0_203f_isr(void)
{
    if(GPIO->MCUN0INT1STAT)
    {
        g_ui32ch0_203f = GPIO->MCUN0INT1STAT;
    }
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT1CLR)),GPIO->MCUN0INT1STAT, REG_EQUAL_MASK);
}

void
am_gpio0_405f_isr(void)
{
    if(GPIO->MCUN0INT2STAT)
    {
        g_ui32ch0_405f = GPIO->MCUN0INT2STAT;
    }
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT2CLR)),GPIO->MCUN0INT2STAT, REG_EQUAL_MASK);
}

void
am_gpio0_607f_isr(void)
{
    if(GPIO->MCUN0INT3STAT)
    {
        g_ui32ch0_607f = GPIO->MCUN0INT3STAT;
    }
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT3CLR)),GPIO->MCUN0INT3STAT, REG_EQUAL_MASK);
}

void
am_gpio1_001f_isr(void)
{
    if(GPIO->MCUN1INT0STAT)
    {
        g_ui32ch1_001f = GPIO->MCUN1INT0STAT;
    }
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT0CLR)),GPIO->MCUN1INT0STAT, REG_EQUAL_MASK);
}

void
am_gpio1_203f_isr(void)
{
    if(GPIO->MCUN1INT1STAT)
    {
        g_ui32ch1_203f = GPIO->MCUN1INT1STAT;
    }
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT1CLR)),GPIO->MCUN1INT1STAT, REG_EQUAL_MASK);
}

void
am_gpio1_405f_isr(void)
{
    if(GPIO->MCUN1INT2STAT)
    {
        g_ui32ch1_405f = GPIO->MCUN1INT2STAT;
    }
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT2CLR)),GPIO->MCUN1INT2STAT, REG_EQUAL_MASK);
}

void
am_gpio1_607f_isr(void)
{
    if(GPIO->MCUN1INT3STAT)
    {
        g_ui32ch1_607f = GPIO->MCUN1INT3STAT;
    }
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT3CLR)),GPIO->MCUN1INT3STAT, REG_EQUAL_MASK);
}


static uint32_t
isrflag_get(am_hal_gpio_int_channel_e echl, uint32_t pin)
{
    uint32_t retval = 0;

    if ( pin >= 128 )
    {
        return retval;
    }

    if ( echl == AM_HAL_GPIO_INT_CHANNEL_0 )
    {
        if ( pin < 32 )
        {
            retval = (g_ui32ch0_001f >> pin) & 0x00000001;
        }
        else if ( pin < 64 )
        {
            retval = (g_ui32ch0_203f >> (pin - 32)) & 0x00000001;
        }
        else if ( pin < 96 )
        {
            retval = (g_ui32ch0_405f >> (pin - 64)) & 0x00000001;
        }
        else if ( pin < 128 )
        {
            retval = (g_ui32ch0_607f >> (pin - 96)) & 0x00000001;
        }
    }
    else if ( echl == AM_HAL_GPIO_INT_CHANNEL_1 )
    {
        if ( pin < 32 )
        {
            retval = (g_ui32ch1_001f >> pin) & 0x00000001;
        }
        else if ( pin < 64 )
        {
            retval = (g_ui32ch1_203f >> (pin - 32)) & 0x00000001;
        }
        else if ( pin < 96 )
        {
            retval = (g_ui32ch1_405f >> (pin - 64)) & 0x00000001;
        }
        else if ( pin < 128 )
        {
            retval = (g_ui32ch1_607f >> (pin - 96)) & 0x00000001;
        }
    }
    else
    {
        retval = 0;
    }

    return retval;

} // isrflag_get()


static void
isrflags_initall(uint32_t setval)
{
    g_ui32ch0_001f = g_ui32ch0_203f = g_ui32ch0_405f = g_ui32ch0_607f = setval;
    g_ui32ch1_001f = g_ui32ch1_203f = g_ui32ch1_405f = g_ui32ch1_607f = setval;
} // isrflags_initall()

static bool
isrflags_chkall(am_hal_gpio_int_channel_e echl_excl, uint32_t pin_excl)
{
    uint32_t uexclbit;
    uint32_t ui32Mask = 0;

    //
    // All except one of the flags are expected to be zero.
    // Return false if no other flags are set.
    // Return true if another flag is set.
    //

    if ( g_ui32ch0_001f != 0 )
    {
        ui32Mask |= 0x01;
    }
    if ( g_ui32ch0_203f != 0 )
    {
        ui32Mask |= 0x02;
    }
    if ( g_ui32ch0_405f != 0 )
    {
        ui32Mask |= 0x04;
    }
    if ( g_ui32ch0_607f != 0 )
    {
        ui32Mask |= 0x08;
    }

    if ( g_ui32ch1_001f != 0 )
    {
        ui32Mask |= 0x10;
    }
    if ( g_ui32ch1_203f != 0 )
    {
        ui32Mask |= 0x20;
    }
    if ( g_ui32ch1_405f != 0 )
    {
        ui32Mask |= 0x40;
    }
    if ( g_ui32ch1_607f != 0 )
    {
        ui32Mask |= 0x80;
    }

    uexclbit  = 1 << (pin_excl / 32);
    uexclbit <<= ((echl_excl == AM_HAL_GPIO_INT_CHANNEL_1) ? 4 : 0);

    ui32Mask &= ( ~uexclbit );

    return ui32Mask == 0 ? false : true;
} // isrflags_chkall()


static IRQn_Type
NVICIRQ_get(am_hal_gpio_int_channel_e eChl, uint32_t ui32pin)
{
    uint32_t ui32irq;

    ui32irq = ui32pin / 32;
    ui32irq += (eChl == AM_HAL_GPIO_INT_CHANNEL_0) ? 0 : 4;

    switch ( ui32irq )
    {
        case 0:
            return GPIO0_001F_IRQn;
        case 1:
            return GPIO0_203F_IRQn;
        case 2:
            return GPIO0_405F_IRQn;
        case 3:
            return GPIO0_607F_IRQn;
        case 4:
            return GPIO1_001F_IRQn;
        case 5:
            return GPIO1_203F_IRQn;
        case 6:
            return GPIO1_405F_IRQn;
        case 7:
            return GPIO1_607F_IRQn;
        default:
            return (IRQn_Type)0;
    }
} // NVICIRQ_get()

static void
gpio_clear_allints(void)
{
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT0CLR)),0xFFFFFFFF, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT1CLR)),0xFFFFFFFF, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT2CLR)),0xFFFFFFFF, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT3CLR)),0xFFFFFFFF, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT0CLR)),0xFFFFFFFF, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT1CLR)),0xFFFFFFFF, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT2CLR)),0xFFFFFFFF, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT3CLR)),0xFFFFFFFF, REG_EQUAL_MASK);
} // gpio_clear_allints()

static void
gpio_disable_allints(void)
{
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT0EN)),0, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT1EN)),0, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT2EN)),0, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN0INT3EN)),0, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT0EN)),0, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT1EN)),0, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT2EN)),0, REG_EQUAL_MASK);
    am_hal_gpio_reg_wr((uint32_t) (&(GPIO->MCUN1INT3EN)),0, REG_EQUAL_MASK);
} // gpio_disable_allints()

#if 0
static uint32_t
isrflag_set(am_hal_gpio_int_channel_e echl, uint32_t pin, uint32_t setval)
{
    uint32_t retval = 0;

    if ( pin >= 128 )
    {
        return retval;
    }

    if ( echl == AM_HAL_GPIO_INT_CHANNEL_0 )
    {
        if ( pin < 32 )
        {
            g_ui32ch0_001f = setval;
        }
        else if ( pin < 64 )
        {
            g_ui32ch0_203f = setval;
        }
        else if ( pin < 96 )
        {
            g_ui32ch0_405f = setval;
        }
        else if ( pin < 128 )
        {
            g_ui32ch0_607f = setval;
        }
        retval = setval;
    }
    else if ( echl == AM_HAL_GPIO_INT_CHANNEL_1 )
    {
        if ( pin < 32 )
        {
            g_ui32ch1_001f = setval;
        }
        else if ( pin < 64 )
        {
            g_ui32ch1_203f = setval;
        }
        else if ( pin < 96 )
        {
            g_ui32ch1_405f = setval;
        }
        else if ( pin < 128 )
        {
            g_ui32ch1_607f = setval;
        }
        retval = setval;
    }
    else
    {
        retval = 0;
    }

    return retval;
} // isrflag_set()
#endif

static void
gpio_int_check_fail(uint32_t RetCode, uint32_t DesiredCode)
{
    if(RetCode != DesiredCode)
    {
        am_util_stdio_printf("-- FAIL!!!\n");
        if(RetCode & TINT_LO2HI)
        {
            am_util_stdio_printf("   rising edge triggered\n");
        }
        else
        {
            am_util_stdio_printf("   rising edge not triggered\n");
        }

        if(RetCode & TINT_LO2HI_OTHER_INT)
        {
            am_util_stdio_printf("   other interrupt triggered at rising edge\n");
        }

        if(RetCode & TINT_HI2LO)
        {
            am_util_stdio_printf("   falling edge triggered\n");
        }
        else
        {
            am_util_stdio_printf("   falling edge not triggered\n");
        }

        if(RetCode & TINT_HI2LO_OTHER_INT)
        {
            am_util_stdio_printf("   other interrupt triggered at falling edge\n");
        }
    }
    else
    {
        am_util_stdio_printf("-- PASS!!!\n");
    }
}
//*****************************************************************************
//
// GPIO brief interrupt test.
//
// Arguments:
//  ui32PinTestIntrpt   = Pin (input) to be interrupted.
//  ui32PinDrive        = Pin used to drive the interrupt.
//  eIntType:   One of AM_HAL_GPIO_PIN_INTDIR_NONE/HI2LO/LO2HI/BOTH
//  eChl:       One of AM_HAL_GPIO_INT_CHANNEL_0/1/BOTH
//
// The 2 pins are expected to be physically connected together and
// the pin configured by the caller.  Interrupts should not be enabled
// on entry.
//
//*****************************************************************************
uint32_t
am_widget_gpio_int_check(uint32_t ui32PinTestIntrpt, uint32_t ui32PinDrive,
                         am_hal_gpio_intdir_e eIntType, am_hal_gpio_int_channel_e eChl,
                         uint8_t ui8LoopCount)
{
    uint32_t ui32FuncRet, ui32RetVal;
    uint32_t ui32Code;
    IRQn_Type eIRQnum;

    //
    // Disable all GPIO interrupts
    //
    gpio_disable_allints();

    //
    // Initialize the return value
    //
    ui32RetVal = 0;

    //
    // Make sure the IRQ is disabled.
    //
    eIRQnum = NVICIRQ_get(eChl, ui32PinTestIntrpt);
    NVIC_DisableIRQ(eIRQnum);

    //
    // Configure the pin being tested.
    //
    am_hal_gpio_output_clear(ui32PinDrive);
    g_GPIO_INPUT_INT.GP.cfg_b.eIntDir = eIntType;
    ui32FuncRet = am_hal_gpio_pinconfig(ui32PinTestIntrpt, g_GPIO_INPUT_INT);
    CHKPINCFG(ui32PinTestIntrpt, ui32FuncRet);

    //
    // Init and configure the driving pin.
    //
    am_hal_gpio_output_clear(ui32PinDrive);
    ui32FuncRet = am_hal_gpio_pinconfig(ui32PinDrive, g_GPIO_OUTPUT_READ);
    CHKPINCFG(ui32PinDrive, ui32FuncRet);

    //
    // After pin configuration, clear all interrupts before enabling.
    //
    gpio_clear_allints();

    //
    // Enable the interrupt
    //
    ui32FuncRet = am_hal_gpio_interrupt_control(eChl,
                                                AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                                &ui32PinTestIntrpt);
    CHKGPIOINTCTRL(eChl, ui32PinTestIntrpt, ui32FuncRet);

    am_hal_interrupt_master_enable();

    //
    // Perform the given interrupt test.
    //
    isrflags_initall(0);

    NVIC_EnableIRQ(eIRQnum);
    // Trigger interrupt for given count.
    for(uint32_t ui32Loop = 0; ui32Loop < ui8LoopCount; ++ui32Loop)
    {
        ui32Code = 0;
        //
        // Take the driving signal high.
        //
        am_hal_gpio_output_set(ui32PinDrive);
        am_util_stdio_printf("Set tried: %d\n", g_ui32RetryCnt);
        am_hal_delay_us(100);

        //
        // Verify that if ISR was called.
        //
        if ( isrflag_get(eChl, ui32PinTestIntrpt) )
        {
            ui32Code |= TINT_LO2HI;
        }

        //
        // Verify that no other ISRs were called.
        //
        if ( isrflags_chkall(eChl, ui32PinTestIntrpt) )
        {
            ui32Code |= TINT_LO2HI_OTHER_INT;
        }

        //
        // Clear all interrupt flags.
        //
        isrflags_initall(0);

        //
        // Take the driving signal low
        //
        am_hal_gpio_output_clear(ui32PinDrive);
        am_util_stdio_printf("Clear tried: %d\n", g_ui32RetryCnt);
        am_hal_delay_us(100);

        //
        // Verify that if ISR was called.
        //
        if ( isrflag_get(eChl, ui32PinTestIntrpt) )
        {
            ui32Code |= TINT_HI2LO;
        }

        //
        // Verify that no other ISRs were called.
        //
        if ( isrflags_chkall(eChl, ui32PinTestIntrpt) )
        {
            ui32Code |= TINT_HI2LO_OTHER_INT;
        }

        //
        // Clear all interrupt flags.
        //
        isrflags_initall(0);

        switch(eIntType)
        {
            case AM_HAL_GPIO_PIN_INTDIR_LO2HI:
                TEST_ASSERT_TRUE(ui32Code == TINT_LO2HI);
                gpio_int_check_fail(ui32Code, TINT_LO2HI);
                break;
            case AM_HAL_GPIO_PIN_INTDIR_HI2LO:
                TEST_ASSERT_TRUE(ui32Code == TINT_HI2LO);
                gpio_int_check_fail(ui32Code, TINT_HI2LO);
                break;
            case AM_HAL_GPIO_PIN_INTDIR_BOTH:
                TEST_ASSERT_TRUE(ui32Code == (TINT_LO2HI | TINT_HI2LO));
                gpio_int_check_fail(ui32Code, TINT_LO2HI | TINT_HI2LO);
                break;
            case AM_HAL_GPIO_PIN_INTDIR_NONE:
                TEST_ASSERT_TRUE(ui32Code == 0);
                gpio_int_check_fail(ui32Code, 0);
                break;
        }
    }

    //
    // Disable IRQ and master interrupts
    // NVIC_ClearPendingIRQ() is probably overkill, but it'll help ensure a
    // clean slate for the next text.
    //
    NVIC_DisableIRQ(eIRQnum);
    am_hal_interrupt_master_disable();
    NVIC_ClearPendingIRQ(eIRQnum);

    //
    // Set pins back to default config
    //
    am_hal_gpio_output_clear(ui32PinDrive);
    ui32FuncRet = am_hal_gpio_pinconfig(ui32PinDrive,      am_hal_gpio_pincfg_default);
    ui32FuncRet = am_hal_gpio_pinconfig(ui32PinTestIntrpt, am_hal_gpio_pincfg_default);

    //
    // After pin configuration, disable and clear all interrupts.
    //
    gpio_disable_allints();
    gpio_clear_allints();

    return ui32RetVal;
}
