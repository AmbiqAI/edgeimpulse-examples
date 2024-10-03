//*****************************************************************************
//
//! @file ap3bp_mux_apollo4l_eb.c
//!
//! @brief ap3bp_mux_apollo4l_eb example.
//!
//! Purpose: .
//!
//! Printing takes place over the ITM at 1M Baud.
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
#include "am_bsp.h"
#include "am_util.h"
#include "ap3bp_mux_define.h"

#define USE_IOS
#define PULL_DOWN_BEFORE_READ
#define APOLLO4L_EB_FACTORY_TEST

//DIPSW changed.
volatile uint32_t g_ui32GPIOIntSrv = 0;
 //0:dipsw; 1:I2S
volatile uint32_t g_ui32SwitchMode = 0;

#ifdef USE_IOS

#define TEST_IOS_XCMP_INT   1

#define AM_HAL_IOS_INT_ERR  (AM_HAL_IOS_INT_FOVFL | AM_HAL_IOS_INT_FUNDFL | AM_HAL_IOS_INT_FRDERR)
#define AM_HAL_IOS_XCMP_INT (AM_HAL_IOS_INT_XCMPWR | AM_HAL_IOS_INT_XCMPWF | AM_HAL_IOS_INT_XCMPRR | AM_HAL_IOS_INT_XCMPRF)

#define ROBUFFER_INIT               (0x55)
#define AM_IOS_LRAM_SIZE_MAX        0x78
uint8_t g_pIosSendBuf[AM_IOS_LRAM_SIZE_MAX];
#define AM_IOS_HEADER_SIZE          sizeof(sHeader)
#define AM_IOS_MAX_DATA_SIZE        (AM_IOS_LRAM_SIZE_MAX - AM_IOS_HEADER_SIZE)

static void *g_pIOSHandle;
volatile bool bIomSendComplete = false;

const am_hal_gpio_pincfg_t g_AM_BSP_GPIO_ENABLE =
{
    .uFuncSel            = AM_HAL_PIN_4_SLINT,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA,
    .eCEpol              = AM_HAL_GPIO_PIN_CEPOL_ACTIVEHIGH,
    .eGPOutcfg           = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
};

static am_hal_ios_config_t g_sIOSSpiConfig =
{
    // Configure the IOS in SPI mode.
    .ui32InterfaceSelect = AM_HAL_IOS_USE_SPI,

    // Eliminate the "read-only" section, so an external host can use the
    // entire "direct write" section.
    .ui32ROBase = 0x78,

    // Making the "FIFO" section as big as possible.
    .ui32FIFOBase = 0x80,

    // We don't need any RAM space, so extend the FIFO all the way to the end
    // of the LRAM.
    .ui32RAMBase = 0x100,

    // FIFO Threshold - set to half the size
    .ui32FIFOThreshold = 0x20,
};
#endif

//*****************************************************************************
//
// dipsw read status
//
//*****************************************************************************
uint32_t dipsw_read_status()
{
    uint32_t status = 0;

#ifdef PULL_DOWN_BEFORE_READ
    am_hal_gpio_pinconfig(DIPSW_0, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pinconfig(DIPSW_1, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pinconfig(DIPSW_2, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pinconfig(DIPSW_3, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pinconfig(DIPSW_4, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pinconfig(DIPSW_5, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_state_write(DIPSW_0, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(DIPSW_1, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(DIPSW_2, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(DIPSW_3, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(DIPSW_4, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(DIPSW_5, AM_HAL_GPIO_OUTPUT_CLEAR);

    // Keep low for a while
    am_util_delay_ms(50);
#endif

    am_hal_gpio_pinconfig(DIPSW_0, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_1, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_2, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_3, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_4, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_5, g_AM_HAL_GPIO_INPUT);

    am_util_delay_ms(100);

    uint32_t dipsw0_status = 0;
    uint32_t dipsw1_status = 0;
    uint32_t dipsw2_status = 0;
    uint32_t dipsw3_status = 0;
    uint32_t dipsw4_status = 0;
    uint32_t dipsw5_status = 0;

    am_hal_gpio_state_read(DIPSW_0, AM_HAL_GPIO_INPUT_READ, &dipsw0_status);
    am_hal_gpio_state_read(DIPSW_1, AM_HAL_GPIO_INPUT_READ, &dipsw1_status);
    am_hal_gpio_state_read(DIPSW_2, AM_HAL_GPIO_INPUT_READ, &dipsw2_status);
    am_hal_gpio_state_read(DIPSW_3, AM_HAL_GPIO_INPUT_READ, &dipsw3_status);
    am_hal_gpio_state_read(DIPSW_4, AM_HAL_GPIO_INPUT_READ, &dipsw4_status);
    am_hal_gpio_state_read(DIPSW_5, AM_HAL_GPIO_INPUT_READ, &dipsw5_status);

    status = dipsw0_status * DIPSW_0_BIT +
           dipsw1_status * DIPSW_1_BIT +
           dipsw2_status * DIPSW_2_BIT +
           dipsw3_status * DIPSW_3_BIT +
           dipsw4_status * DIPSW_4_BIT +
           dipsw5_status * DIPSW_5_BIT;

    return status;
}

//*****************************************************************************
//
// dipsw interrupt handler
//  dipsw status changed.
//*****************************************************************************
void dipsw_int_handler()
{
    g_ui32GPIOIntSrv = 1;
}

//*****************************************************************************
//
// led_init
//
//*****************************************************************************
void led_init()
{
  am_hal_gpio_pinconfig(AP3BP_LED, g_AM_HAL_GPIO_OUTPUT);
  am_hal_gpio_state_write(AP3BP_LED, AM_HAL_GPIO_OUTPUT_CLEAR);
}

//*****************************************************************************
//
// dipsw_init
//
//*****************************************************************************
void dipsw_init()
{
    am_hal_gpio_pinconfig(DIPSW_0, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_1, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_2, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_3, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_4, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_5, g_AM_HAL_GPIO_INPUT);

    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_0));
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_1));
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_2));
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_3));
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_4));
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_5));

    //
    // Register handler for dipsw
    //
    am_hal_gpio_interrupt_register(DIPSW_0, dipsw_int_handler);
    am_hal_gpio_interrupt_register(DIPSW_1, dipsw_int_handler);
    am_hal_gpio_interrupt_register(DIPSW_2, dipsw_int_handler);
    am_hal_gpio_interrupt_register(DIPSW_3, dipsw_int_handler);
    am_hal_gpio_interrupt_register(DIPSW_4, dipsw_int_handler);
    am_hal_gpio_interrupt_register(DIPSW_5, dipsw_int_handler);

    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_0));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_1));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_2));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_3));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_4));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, DIPSW_5));

    NVIC_EnableIRQ(GPIO_IRQn);
}

//*****************************************************************************
//
// iomux_init
//
//*****************************************************************************
void iomux_init()
{
    am_hal_gpio_pinconfig(MUX_MSPI0_OE,      g_AM_HAL_GPIO_OUTPUT);    //  4
    am_hal_gpio_pinconfig(MUX_MSPI0_L1_SEL,  g_AM_HAL_GPIO_OUTPUT);     //  5
    am_hal_gpio_pinconfig(MUX_MSPI0_L2_SEL,  g_AM_HAL_GPIO_OUTPUT);    //  6
    am_hal_gpio_pinconfig(MUX_MSPI0_L3_SEL,  g_AM_HAL_GPIO_OUTPUT);    //  7
    am_hal_gpio_pinconfig(MUX_MSPI1_OE,      g_AM_HAL_GPIO_OUTPUT);    //  8
    am_hal_gpio_pinconfig(MUX_MSPI1_L1_SEL,  g_AM_HAL_GPIO_OUTPUT);    //  9
    am_hal_gpio_pinconfig(MUX_MSPI1_L2_SEL,  g_AM_HAL_GPIO_OUTPUT);    // 10
    am_hal_gpio_pinconfig(MUX_MSPI1_L3A_SEL, g_AM_HAL_GPIO_OUTPUT);    // 11
    am_hal_gpio_pinconfig(MUX_MSPI1_L3B_SEL, g_AM_HAL_GPIO_OUTPUT);    // 12
    am_hal_gpio_pinconfig(MUX_MSPI2_OE,      g_AM_HAL_GPIO_OUTPUT);    // 13
    am_hal_gpio_pinconfig(MUX_MSPI2_L1_SEL,  g_AM_HAL_GPIO_OUTPUT);    // 14
    am_hal_gpio_pinconfig(MUX_MSPI2_L2_SEL,  g_AM_HAL_GPIO_OUTPUT);    // 15
    am_hal_gpio_pinconfig(MUX_MSPI2_L3_SEL,  g_AM_HAL_GPIO_OUTPUT);    // 16
    am_hal_gpio_pinconfig(MUX_SDIO_OE,       g_AM_HAL_GPIO_OUTPUT);    // 17
    am_hal_gpio_pinconfig(MUX_SDIO_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 18
    am_hal_gpio_pinconfig(MUX_IOM0_OE,       g_AM_HAL_GPIO_OUTPUT);    // 19
    am_hal_gpio_pinconfig(MUX_IOM0_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 22
    am_hal_gpio_pinconfig(MUX_IOM0_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 23
    am_hal_gpio_pinconfig(MUX_IOM0_L3_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 24
    am_hal_gpio_pinconfig(MUX_IOM1_OE,       g_AM_HAL_GPIO_OUTPUT);    // 25
    am_hal_gpio_pinconfig(MUX_IOM1_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 26
    am_hal_gpio_pinconfig(MUX_IOM1_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 27
    am_hal_gpio_pinconfig(MUX_IOM2_OE,       g_AM_HAL_GPIO_OUTPUT);    // 28
    am_hal_gpio_pinconfig(MUX_IOM2_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 29
    am_hal_gpio_pinconfig(MUX_IOM2_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 30
    am_hal_gpio_pinconfig(MUX_IOM2_L3_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 31
    am_hal_gpio_pinconfig(MUX_IOM3_OE,       g_AM_HAL_GPIO_OUTPUT);    // 32
    am_hal_gpio_pinconfig(MUX_IOM3_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 33
    am_hal_gpio_pinconfig(MUX_IOM3_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 34
    am_hal_gpio_pinconfig(MUX_IOM4_OE,       g_AM_HAL_GPIO_OUTPUT);    // 35
    am_hal_gpio_pinconfig(MUX_IOM4_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 36
    am_hal_gpio_pinconfig(MUX_IOM4_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 37
    am_hal_gpio_pinconfig(MUX_IOM4_L3_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 38
    am_hal_gpio_pinconfig(MUX_IOM5_OE,       g_AM_HAL_GPIO_OUTPUT);    // 39
    am_hal_gpio_pinconfig(MUX_IOM5_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 40
    am_hal_gpio_pinconfig(MUX_IOM5_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 42
    am_hal_gpio_pinconfig(MUX_IOM5_L3_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 43
    am_hal_gpio_pinconfig(MUX_IOM6_OE,       g_AM_HAL_GPIO_OUTPUT);    // 44
    am_hal_gpio_pinconfig(MUX_IOM6_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 45
    am_hal_gpio_pinconfig(MUX_IOM6_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 46
    am_hal_gpio_pinconfig(MUX_IOM7_OE,       g_AM_HAL_GPIO_OUTPUT);    // 47
    am_hal_gpio_pinconfig(MUX_IOM7_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 48
    am_hal_gpio_pinconfig(MUX_IOM7_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 49
    am_hal_gpio_pinconfig(MUX_IOM7_L3_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 50
    am_hal_gpio_pinconfig(MUX_IOS_OE,        g_AM_HAL_GPIO_OUTPUT);    // 51
    am_hal_gpio_pinconfig(MUX_IOS_L1_SEL,    g_AM_HAL_GPIO_OUTPUT);    // 52
    am_hal_gpio_pinconfig(MUX_IOS_L2_SEL,    g_AM_HAL_GPIO_OUTPUT);    // 53
    am_hal_gpio_pinconfig(MUX_UART_OE,       g_AM_HAL_GPIO_OUTPUT);    // 54
    am_hal_gpio_pinconfig(MUX_UART_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 55
    am_hal_gpio_pinconfig(MUX_SWD_OE,        g_AM_HAL_GPIO_OUTPUT);    // 56
    am_hal_gpio_pinconfig(MUX_SWD_L1_SEL,    g_AM_HAL_GPIO_OUTPUT);    // 57
}

//*****************************************************************************
//
// am_gpio_isr
//
//*****************************************************************************
void am_gpio_isr()
{
    // Debounce
    am_util_delay_ms(200);

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
#endif
}

//*****************************************************************************
//
// led_indicate
//
//*****************************************************************************
void led_indicate(uint32_t onoff)
{
    if ( onoff == DIPSW_NONE ) //USE IOS.
    {
        am_hal_gpio_state_write(AP3BP_LED, AM_HAL_GPIO_OUTPUT_CLEAR);
    }
    else
    {
        am_hal_gpio_state_write(AP3BP_LED, AM_HAL_GPIO_OUTPUT_SET);
    }
}

#ifdef USE_IOS
//*****************************************************************************
//
// Configure the SPI slave.
//
//*****************************************************************************
static void ios_set_up()
{
    // Configure SPI interface
    am_bsp_ios_pins_enable(0, AM_HAL_IOS_USE_SPI);
    //
    // Configure the IOS interface and LRAM structure.
    //
    am_hal_ios_initialize(0, &g_pIOSHandle);
    am_hal_ios_power_ctrl(g_pIOSHandle, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_ios_configure(g_pIOSHandle, &g_sIOSSpiConfig);

    //
    // Clear out any IOS register-access interrupts that may be active, and
    // enable interrupts for the registers we're interested in.
    //
    am_hal_ios_interrupt_clear(g_pIOSHandle, AM_HAL_IOS_INT_ALL);
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_INT_ERR | AM_HAL_IOS_INT_FSIZE);
#ifdef TEST_IOINTCTL
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_INT_IOINTW);
#endif
#ifdef TEST_IOS_XCMP_INT
    am_hal_ios_interrupt_enable(g_pIOSHandle, AM_HAL_IOS_XCMP_INT);
#endif

    //
    // Set the bit in the NVIC to accept access interrupts from the IO Slave.
    //
    NVIC_EnableIRQ(IOSLAVE_IRQn);

    // Set up the IOSINT interrupt pin
    //am_hal_gpio_pinconfig(HANDSHAKE_IOS_PIN, g_AM_BSP_GPIO_ENABLE);

    // Initialize RO & AHB-RAM data with pattern
    for (uint8_t i = 0; i < 8; i++)
    {
        am_hal_ios_pui8LRAM[0x78 + i] = ROBUFFER_INIT;
    }
}

//*****************************************************************************
//
// IOS Slave Main ISR.
//
//*****************************************************************************
void am_ioslave_ios_isr(void)
{
    uint32_t ui32Status;

    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //

    am_hal_ios_interrupt_status_get(g_pIOSHandle, false, &ui32Status);

    am_hal_ios_interrupt_clear(g_pIOSHandle, ui32Status);

    if (ui32Status & AM_HAL_IOS_INT_FUNDFL)
    {
        am_util_stdio_printf("Hitting underflow for the requested IOS FIFO transfer\n");
        // We should never hit this case unless the threshold has beeen set
        // incorrect, or we are unable to handle the data rate
        // ERROR!
        am_hal_debug_assert_msg(0,
            "Hitting underflow for the requested IOS FIFO transfer.");
    }

    if (ui32Status & AM_HAL_IOS_INT_ERR)
    {
        // We should never hit this case
        // ERROR!
        am_hal_debug_assert_msg(0,
            "Hitting ERROR case.");
    }

    if (ui32Status & AM_HAL_IOS_INT_FSIZE)
    {
        //
        // Service the I2C slave FIFO if necessary.
        //
        am_hal_ios_interrupt_service(g_pIOSHandle, ui32Status);
    }

    if (ui32Status & AM_HAL_IOS_INT_XCMPWR)
    {
        bIomSendComplete = true;
    }
}

//*****************************************************************************
//
// IOS COMMAND HANDLER.
//
//*****************************************************************************
void ios_cmd_handler(uint8_t* buffer)
{

}
#endif


void mspi0_group_set(uint32_t mode)
{
    switch (mode)
    {
        case MSPI0_GROUP_HZ:
            am_hal_gpio_state_write(MUX_MSPI0_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case MSPI0_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_MSPI0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI0_L1_SEL, MUX_SEL_B);
            break;
        case MSPI0_GROUP_NOR_FLASH:
            am_hal_gpio_state_write(MUX_MSPI0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI0_L2_SEL, MUX_SEL_B);
            break;
        case MSPI0_GROUP_HEX_PSRAM_AP:
            am_hal_gpio_state_write(MUX_MSPI0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI0_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI0_L3_SEL, MUX_SEL_A);
            break;
        case MSPI0_GROUP_HEX_PSRAM_WB:
            am_hal_gpio_state_write(MUX_MSPI0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI0_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI0_L3_SEL, MUX_SEL_B);
            break;
    }
}

void mspi1_group_set(uint32_t mode)
{
    switch (mode)
    {
        case MSPI1_GROUP_HZ:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case MSPI1_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI1_L1_SEL, MUX_SEL_B);
            break;
        case MSPI1_GROUP_OCTAL_PSRAM:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI1_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI1_L3A_SEL, MUX_SEL_A);
            break;
        case MSPI1_GROUP_QSPI_DISP:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI1_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI1_L3A_SEL, MUX_SEL_B);
            break;
        case MSPI1_GROUP_HEX_PSRAM_AP:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI1_L2_SEL, MUX_SEL_B);
            am_hal_gpio_state_write(MUX_MSPI1_L3B_SEL, MUX_SEL_A);
            break;
        case MSPI1_GROUP_HEX_PSRAM_WB:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI1_L2_SEL, MUX_SEL_B);
            am_hal_gpio_state_write(MUX_MSPI1_L3B_SEL, MUX_SEL_B);
            break;
    }
}

void mspi2_group_set(uint32_t mode)
{
    switch (mode)
    {
        case MSPI2_GROUP_HZ:
            am_hal_gpio_state_write(MUX_MSPI2_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case MSPI2_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_MSPI2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI2_L1_SEL, MUX_SEL_B);
            break;
        case MSPI2_GROUP_OCTAL_PSRAM:
            am_hal_gpio_state_write(MUX_MSPI2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI2_L2_SEL, MUX_SEL_B);
            break;
        case MSPI2_GROUP_QUAD_NAND_FLASH:
            am_hal_gpio_state_write(MUX_MSPI2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI2_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI2_L3_SEL, MUX_SEL_A);
            break;
        case MSPI2_GROUP_DISP_COOPER_EMMC_D4_D7:
            am_hal_gpio_state_write(MUX_MSPI2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI2_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI2_L3_SEL, MUX_SEL_B);
            break;
    }
}

void sdio_group_set(uint32_t mode)
{
    switch (mode)
    {
        case SDIO_GROUP_HZ:
            am_hal_gpio_state_write(MUX_SDIO_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case SDIO_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_SDIO_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_SDIO_L1_SEL, MUX_SEL_B);
            break;
        case SDIO_GROUP_EMMC_D0_D3:
            am_hal_gpio_state_write(MUX_SDIO_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_SDIO_L1_SEL, MUX_SEL_A);
            break;
    }
}

void iom0_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM0_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM0_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM0_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM0_L1_SEL, MUX_SEL_B);
            break;
        case IOM0_GROUP_IOM_IOS_LOOP_BACK:
            am_hal_gpio_state_write(MUX_IOM0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM0_L2_SEL, MUX_SEL_B);
            break;
        case IOM0_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM0_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM0_L3_SEL, MUX_SEL_A);
            break;
        case IOM0_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM0_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM0_L3_SEL, MUX_SEL_B);
            break;
    }
}

void iom1_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM1_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM1_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM1_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM1_L1_SEL, MUX_SEL_B);
            break;
        case IOM1_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM1_L2_SEL, MUX_SEL_A);
            break;
        case IOM1_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM1_L2_SEL, MUX_SEL_B);
            break;
    }
}

void iom2_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM2_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM2_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM2_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM2_L1_SEL, MUX_SEL_B);
            break;
        case IOM2_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM2_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM2_L3_SEL, MUX_SEL_A);
            break;
        case IOM2_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM2_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM2_L3_SEL, MUX_SEL_B);
            break;
        case IOM2_GROUP_DISP:
            am_hal_gpio_state_write(MUX_IOM2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM2_L2_SEL, MUX_SEL_B);
            break;
    }
}

void iom3_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM3_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM3_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM3_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM3_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM3_L1_SEL, MUX_SEL_B);
            break;
        case IOM3_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM3_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM3_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM3_L2_SEL, MUX_SEL_A);
            break;
        case IOM3_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM3_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM3_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM3_L2_SEL, MUX_SEL_B);
            break;
    }
}

void iom4_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM4_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM4_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM4_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM4_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM4_L1_SEL, MUX_SEL_B);
            break;
        case IOM4_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM4_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM4_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM4_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM4_L3_SEL, MUX_SEL_A);
            break;
        case IOM4_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM4_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM4_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM4_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM4_L3_SEL, MUX_SEL_B);
            break;
        case IOM4_GROUP_DISP_COOPER:
            am_hal_gpio_state_write(MUX_IOM4_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM4_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM4_L2_SEL, MUX_SEL_B);
            break;
    }
}


void iom5_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM5_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM5_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM5_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM5_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM5_L1_SEL, MUX_SEL_B);
            break;
        case IOM5_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM5_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM5_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM5_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM5_L3_SEL, MUX_SEL_A);
            break;
        case IOM5_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM5_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM5_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM5_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM5_L3_SEL, MUX_SEL_B);
            break;
        case IOM5_GROUP_IOS:
            am_hal_gpio_state_write(MUX_IOM5_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM5_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM5_L2_SEL, MUX_SEL_B);
            break;
    }
}

void iom6_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM6_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM6_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM6_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM6_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM6_L1_SEL, MUX_SEL_B);
            break;
        case IOM6_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM6_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM6_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM6_L2_SEL, MUX_SEL_A);
            break;
        case IOM6_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM6_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM6_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM6_L2_SEL, MUX_SEL_B);
            break;
    }
}

void iom7_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM7_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM7_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM7_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM7_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM7_L1_SEL, MUX_SEL_B);
            break;
        case IOM7_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM7_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM7_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM7_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM7_L3_SEL, MUX_SEL_A);
            break;
        case IOM7_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM7_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM7_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM7_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM7_L3_SEL, MUX_SEL_B);
            break;
        case IOM7_GROUP_JLINK_SWO:
            am_hal_gpio_state_write(MUX_IOM7_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM7_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM7_L2_SEL, MUX_SEL_B);
            break;
    }
}

void ios_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOS_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOS_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOS_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOS_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOS_L1_SEL, MUX_SEL_B);
            break;
        case IOS_GROUP_IOM_IOS_LOOP_BACK:
            am_hal_gpio_state_write(MUX_IOS_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOS_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOS_L2_SEL, MUX_SEL_A);
            break;
        case IOS_GROUP_ETM:
            am_hal_gpio_state_write(MUX_IOS_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOS_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOS_L2_SEL, MUX_SEL_B);
            break;
    }
}

void uart_group_set(uint32_t mode)
{
    switch (mode)
    {
        case UART_GROUP_HZ:
            am_hal_gpio_state_write(MUX_UART_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case UART_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_UART_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_UART_L1_SEL, MUX_SEL_B);
            break;
        case UART_GROUP_JLINK_UART:
            am_hal_gpio_state_write(MUX_UART_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_UART_L1_SEL, MUX_SEL_A);
            break;
    }
}

void swd_group_set(uint32_t mode)
{
    switch (mode)
    {
        case SWD_GROUP_HZ:
            am_hal_gpio_state_write(MUX_SWD_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case SWD_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_SWD_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_SWD_L1_SEL, MUX_SEL_B);
            break;
        case SWD_GROUP_SWD:
            am_hal_gpio_state_write(MUX_SWD_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_SWD_L1_SEL, MUX_SEL_A);
            break;
    }
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    uint32_t preStatus = 0xFF;
    uint32_t status = 0xFF;
    //
    // Set the clock frequency.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

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
    // Initialize the printf interface for ITM output
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("ap3bp_mux_apollo4l_eb!\n\n");

    //
    // We are done printing.
    // Disable debug printf messages on ITM.
    //
    //am_bsp_debug_printf_disable();

    led_init();
    dipsw_init();
    iomux_init();
#ifdef USE_IOS
    ios_set_up();
#endif
    am_hal_interrupt_master_enable();

    while (1)
    {
        status = dipsw_read_status();
        if (status != preStatus)
        {
            am_util_stdio_printf("DIP switch is 0x%x: \n", status);
            preStatus = status;
            switch (status)
            {
                case 0:
                    mspi0_group_set(MSPI0_GROUP_HZ);
                    mspi1_group_set(MSPI1_GROUP_HZ);
                    mspi2_group_set(MSPI2_GROUP_HZ);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_HZ);
                    iom1_group_set(IOM1_GROUP_HZ);
                    iom2_group_set(IOM2_GROUP_HZ);
                    iom3_group_set(IOM3_GROUP_HZ);
                    iom4_group_set(IOM4_GROUP_HZ);
                    iom5_group_set(IOM5_GROUP_HZ);
                    iom6_group_set(IOM6_GROUP_HZ);
                    iom7_group_set(IOM7_GROUP_HZ);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_HZ);
                    break;
                case 1:
                    mspi0_group_set(MSPI0_GROUP_PC_HARNESS);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_PC_HARNESS);
                    sdio_group_set(SDIO_GROUP_PC_HARNESS);
                    iom0_group_set(IOM0_GROUP_PC_HARNESS);
                    iom1_group_set(IOM1_GROUP_PC_HARNESS);
                    iom2_group_set(IOM2_GROUP_PC_HARNESS);
                    iom3_group_set(IOM3_GROUP_PC_HARNESS);
                    iom4_group_set(IOM4_GROUP_PC_HARNESS);
                    iom5_group_set(IOM5_GROUP_PC_HARNESS);
                    iom6_group_set(IOM6_GROUP_PC_HARNESS);
                    iom7_group_set(IOM7_GROUP_PC_HARNESS);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_PC_HARNESS);
                    swd_group_set(SWD_GROUP_PC_HARNESS);
                    break;
                case 2:
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_QSPI_DISP);
                    mspi2_group_set(MSPI2_GROUP_QUAD_NAND_FLASH);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_DISP);
                    iom3_group_set(IOM3_GROUP_PSRAM);
                    iom4_group_set(IOM4_GROUP_PSRAM);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO);
                    ios_group_set(IOS_GROUP_ETM);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 3:
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_OCTAL_PSRAM);
                    mspi2_group_set(MSPI2_GROUP_OCTAL_PSRAM);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_PSRAM);
                    iom4_group_set(IOM4_GROUP_PSRAM);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO);
                    ios_group_set(IOS_GROUP_ETM);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 4:
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_WB);
                    mspi1_group_set(MSPI1_GROUP_OCTAL_PSRAM);
                    mspi2_group_set(MSPI2_GROUP_OCTAL_PSRAM);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_PSRAM);
                    iom4_group_set(IOM4_GROUP_PSRAM);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO);
                    ios_group_set(IOS_GROUP_ETM);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 5:
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_HEX_PSRAM_AP);
                    mspi2_group_set(MSPI2_GROUP_DISP_COOPER_EMMC_D4_D7);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_PSRAM);
                    iom4_group_set(IOM4_GROUP_DISP_COOPER);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO);
                    ios_group_set(IOS_GROUP_ETM);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 6:
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_WB);
                    mspi1_group_set(MSPI1_GROUP_HEX_PSRAM_WB);
                    mspi2_group_set(MSPI2_GROUP_OCTAL_PSRAM);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_IOM_IOS_LOOP_BACK);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_PSRAM);
                    iom4_group_set(IOM4_GROUP_PSRAM);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO);
                    ios_group_set(IOS_GROUP_IOM_IOS_LOOP_BACK);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 7:
                    mspi0_group_set(MSPI0_GROUP_NOR_FLASH);
                    mspi1_group_set(MSPI1_GROUP_OCTAL_PSRAM);
                    mspi2_group_set(MSPI2_GROUP_QUAD_NAND_FLASH);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_FRAM);
                    iom1_group_set(IOM1_GROUP_FRAM);
                    iom2_group_set(IOM2_GROUP_FRAM);
                    iom3_group_set(IOM3_GROUP_FRAM);
                    iom4_group_set(IOM4_GROUP_FRAM);
                    iom5_group_set(IOM5_GROUP_FRAM);
                    iom6_group_set(IOM6_GROUP_FRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO);
                    ios_group_set(IOS_GROUP_ETM);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 8:
                    mspi0_group_set(MSPI0_GROUP_PC_HARNESS);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_PC_HARNESS);
                    sdio_group_set(SDIO_GROUP_PC_HARNESS);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_PSRAM);
                    iom4_group_set(IOM4_GROUP_PSRAM);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_PSRAM);
                    ios_group_set(IOS_GROUP_ETM);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 9:
                    mspi0_group_set(MSPI0_GROUP_PC_HARNESS);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_PC_HARNESS);
                    sdio_group_set(SDIO_GROUP_PC_HARNESS);
                    iom0_group_set(IOM0_GROUP_FRAM);
                    iom1_group_set(IOM1_GROUP_FRAM);
                    iom2_group_set(IOM2_GROUP_FRAM);
                    iom3_group_set(IOM3_GROUP_FRAM);
                    iom4_group_set(IOM4_GROUP_FRAM);
                    iom5_group_set(IOM5_GROUP_FRAM);
                    iom6_group_set(IOM6_GROUP_FRAM);
                    iom7_group_set(IOM7_GROUP_FRAM);
                    ios_group_set(IOS_GROUP_ETM);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 10:
                    mspi0_group_set(MSPI0_GROUP_PC_HARNESS);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_PC_HARNESS);
                    sdio_group_set(SDIO_GROUP_PC_HARNESS);
                    iom0_group_set(IOM0_GROUP_PC_HARNESS);
                    iom1_group_set(IOM1_GROUP_PC_HARNESS);
                    iom2_group_set(IOM2_GROUP_PC_HARNESS);
                    iom3_group_set(IOM3_GROUP_PC_HARNESS);
                    iom4_group_set(IOM4_GROUP_PC_HARNESS);
                    iom5_group_set(IOM5_GROUP_PC_HARNESS);
                    iom6_group_set(IOM6_GROUP_PC_HARNESS);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_PC_HARNESS);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 11:
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_QSPI_DISP);
                    mspi2_group_set(MSPI2_GROUP_QUAD_NAND_FLASH);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_IOM_IOS_LOOP_BACK);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_DISP);
                    iom3_group_set(IOM3_GROUP_PSRAM);
                    iom4_group_set(IOM4_GROUP_PSRAM);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_FRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO);
                    ios_group_set(IOS_GROUP_IOM_IOS_LOOP_BACK);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 12:
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_QSPI_DISP);
                    mspi2_group_set(MSPI2_GROUP_PC_HARNESS);
                    sdio_group_set(SDIO_GROUP_PC_HARNESS);
                    iom0_group_set(IOM0_GROUP_PC_HARNESS);
                    iom1_group_set(IOM1_GROUP_PC_HARNESS);
                    iom2_group_set(IOM2_GROUP_DISP);
                    iom3_group_set(IOM3_GROUP_PC_HARNESS);
                    iom4_group_set(IOM4_GROUP_PC_HARNESS);
                    iom5_group_set(IOM5_GROUP_PC_HARNESS);
                    iom6_group_set(IOM6_GROUP_PC_HARNESS);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                case 13:
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_QSPI_DISP);
                    mspi2_group_set(MSPI2_GROUP_QUAD_NAND_FLASH);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_DISP);
                    iom3_group_set(IOM3_GROUP_PSRAM);
                    iom4_group_set(IOM4_GROUP_DISP_COOPER);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO);
                    ios_group_set(IOS_GROUP_ETM);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                    break;
                default:
                    mspi0_group_set(MSPI0_GROUP_HZ);
                    mspi1_group_set(MSPI1_GROUP_HZ);
                    mspi2_group_set(MSPI2_GROUP_HZ);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_HZ);
                    iom1_group_set(IOM1_GROUP_HZ);
                    iom2_group_set(IOM2_GROUP_HZ);
                    iom3_group_set(IOM3_GROUP_HZ);
                    iom4_group_set(IOM4_GROUP_HZ);
                    iom5_group_set(IOM5_GROUP_HZ);
                    iom6_group_set(IOM6_GROUP_HZ);
                    iom7_group_set(IOM7_GROUP_HZ);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_HZ);
            }
        }
        am_util_delay_ms(500);
    }
}
