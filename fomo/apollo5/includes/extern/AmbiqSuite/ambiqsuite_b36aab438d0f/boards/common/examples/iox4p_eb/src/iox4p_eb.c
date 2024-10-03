///////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Name:  iox4p_eb
//
// Author:   Chad Crosby
//
// Description: IO Expander code for the Apollo1 on the Apollo4+ EB
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  05Jan22 - CCrosby:
//      * Changed case statement s/t UART to JLINK comms are always available
//      * Fixed ordering of LEDs so counting is in correct order on board
//      * Changed LED startup blinks from 3 to 2
//      * Removed mux_off() from beginning of every case except default
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
#include "am_mcu_apollo.h"
#include "iox4p_eb.h"
#include <stdio.h>
#include "am_util_delay.h"
//#include "mem_macros.h"
//#include "val_lib.h"
//#include "gpio_lib.h"
#include <string.h>

#define TRIM 0
#define SECURE 1
#define SWDIO 8
#define SWDCK 7

// Prototypes
//int main(void) __attribute__((section("data")));
//int main(void) CODE_SECTION;
void toggle_led (uint32_t led_gpio, uint32_t num_times, uint32_t delay_ms);
void led_indicate (uint32_t state_num);
//void update_amux_sel (uint32_t, uint32_t);
//void fabd_unlock (uint32_t);
//void iox_update_state(uint32_t iox_index);
//void iox_update_state_all(void);
void iox_write_pio(uint32_t pio_num, uint32_t op);
void mux_off (void);

uint32_t g_operational_mode;
//uint32_t g_ioe_state[4] = {0xFFFFFFFF, 0x00FFFFFF, 0xFFFFFFFF, 0x00FFFFFF};

// Main code
int main (void) {
    uint32_t k;
    uint32_t cmd;
    uint32_t pio_num, op;
    uint32_t operational_mode = MODE_DEFAULT;

    g_operational_mode = operational_mode;

    // Enable the keys for all the modules
    AM_REG(CLKGEN, CLKKEY) = AM_REG_CLKGEN_CLKKEY_KEYVAL;

    AM_REG(GPIO, PADKEY) = 0x73;

    //********************************************************
    // SET UP GPIO0-6 (same for all boards)
    //********************************************************

    //--------------------------------------------------------
    // Set up the pin muxing - IOS SPI mode
    //--------------------------------------------------------
    //
    AM_REG(GPIO, PADREGA) = AM_REG_GPIO_PADREGA_PAD3FNCSEL(1) |
                            AM_REG_GPIO_PADREGA_PAD2FNCSEL(1) |
                            AM_REG_GPIO_PADREGA_PAD1FNCSEL(1) |
                            AM_REG_GPIO_PADREGA_PAD0FNCSEL(1) |
                            AM_REG_GPIO_PADREGA_PAD3INPEN(1)|
                            AM_REG_GPIO_PADREGA_PAD2INPEN(1)|
                            AM_REG_GPIO_PADREGA_PAD1INPEN(1)|
                            AM_REG_GPIO_PADREGA_PAD0INPEN(1) ;

    AM_REG(GPIO, CFGA) = AM_REG_GPIO_CFGA_GPIO3OUTCFG(0x3) |
                         AM_REG_GPIO_CFGA_GPIO2OUTCFG(0x3) |
                         AM_REG_GPIO_CFGA_GPIO1OUTCFG(0x3) |
                         AM_REG_GPIO_CFGA_GPIO0OUTCFG(0x3) ;

    //--------------------------------------------------------
    // Set up the pin muxing - IOM I2C mode
    //--------------------------------------------------------
    //
    // DL 4/28/2020: Not needed for EB, no I2C slave devices
    // connected
    //Pads remain same for apollo1 and apollo3
    //AM_REG(GPIO, PADREGB) = AM_REG_GPIO_PADREGB_PAD5FNCSEL(0) |
    //                        AM_REG_GPIO_PADREGB_PAD6FNCSEL(0) |
    //                        AM_REG_GPIO_PADREGB_PAD6PULL(1) |
    //                        AM_REG_GPIO_PADREGB_PAD5PULL(1) |
    //                        AM_REG_GPIO_PADREGB_PAD5INPEN(1) |
    //                        AM_REG_GPIO_PADREGB_PAD6INPEN(1)     ;

    //--------------------------------------------------------
    // Set up the IOM device
    //--------------------------------------------------------

    // Defaults for the device

    //AM_REGVAL(0x50005208) = 0xFFFFFFFF;
    //while ((AM_REGVAL(0x50005118) & 4) != 4 );
    //AM_REGVAL(0x50005108) = 0x40C;    //defaults from Ron
    //AM_REGVAL(0x5000411C) = 0x80000000;     //IOMSTR0 CFG
    //AM_REGVAL(0x5000410C) = 0x170b1100;     //CLK CFG
                        // AM_REG_IOMSTR0_CLKCFG_FSEL(1) |
                         //     AM_REG_IOMSTR0_DIV3(0) |
                          //     AM_REG_IOMSTR0_DIVEN(1) |
                           //      AM_REG_IOMSTR0_LOWPER(b) |
                            //      AM_REG_IOMSTR0_TOTPER(0x17);


    //iox_update_state_all();

    //--------------------------------------------------------
    // Configure IOM pins as tri-state
    //--------------------------------------------------------
    iox_write_pio(5, 2);
    iox_write_pio(6, 2);

    //--------------------------------------------------------
    // Set up the IO SLAVE device
    //--------------------------------------------------------
    // CKPT (CKPT_IOSLAVE,101);          // Word accesses

    // Set up the clocks
    AM_REG(CLKGEN, CCTRL) = 0;

    // There are 3 different modes to access the ioslave. Split up in units of 8byte chunks
    //    1. Direct area from 0 to FIFOBASE, 120 bytes max. Shared access between external/internal. This can be hooked
    //        to the ISR in the different fields.
    //    2. Fifo area for ios to device data, accessed from address 0x7F externally (always this address).
    //    3. RAM area for internal MCU read access only.  The external master cannot get to this.  FIFOMAX to 0xFF
    // Enable the ioslave interface
    AM_REG(IOSLAVE, CFG) = AM_REG_IOSLAVE_CFG_IFCEN(0x1) |
                           AM_REG_IOSLAVE_CFG_LSB(0x0)   |
                           AM_REG_IOSLAVE_CFG_SPOL(0x0)  |
                           AM_REG_IOSLAVE_CFG_IFCSEL(0x1);

    AM_REG(IOSLAVE, FIFOCFG) = AM_REG_IOSLAVE_FIFOCFG_ROBASE(0x04) |
                               AM_REG_IOSLAVE_FIFOCFG_FIFOMAX(0x1F) |
                               AM_REG_IOSLAVE_FIFOCFG_FIFOBASE(0x10);

    AM_REG(IOSLAVE, FIFOPTR) = AM_REG_IOSLAVE_FIFOPTR_FIFOSIZ(0) |
                               AM_REG_IOSLAVE_FIFOPTR_FIFOPTR(0x80);

    AM_REG(IOSLAVE, FIFOCTR) = AM_REG_IOSLAVE_FIFOCTR_FIFOCTR(0x200);

    // Load IOX Signature into address 0x40/0x41
    AM_REGVAL(REG_IOSLAVE_BASEADDR+0x40) = 0x20;
    AM_REGVAL(REG_IOSLAVE_BASEADDR+0x41) = 0xA5;
    //What is this? Needed if no fabd necessary?
    AM_REGVAL(REG_IOSLAVE_BASEADDR+0x42) = 0x01; //new FABD unlock version

    for (k=7; k<50; k++) {
        if (k == 20 || k == 21) {
            continue;
        } else {
            am_hal_gpio_pin_config(k, AM_HAL_PIN_3STATE); // tristate output on every pad after startup
        }
    }

    // DL 4/28/2020: Updated for three LEDs
    // Configure the LEDs and toggle twice
    // All LED should toggle in sequence and then turn off
    am_hal_gpio_pin_config(GPIO_LED_BLUE_2, AM_HAL_GPIO_OUTPUT);
    toggle_led(GPIO_LED_BLUE_2, 2, 500);
    am_hal_gpio_pin_config(GPIO_LED_BLUE_1, AM_HAL_GPIO_OUTPUT);
    toggle_led(GPIO_LED_BLUE_1, 2, 500);
    am_hal_gpio_pin_config(GPIO_LED_BLUE_0, AM_HAL_GPIO_OUTPUT);
    toggle_led(GPIO_LED_BLUE_0, 2, 500);
    am_hal_gpio_out_bit_clear(GPIO_LED_BLUE_2);
    am_hal_gpio_out_bit_clear(GPIO_LED_BLUE_1);
    am_hal_gpio_out_bit_clear(GPIO_LED_BLUE_0);

    //Configure button GPIO and interrupt
    am_hal_gpio_pin_config(GPIO_SW2, AM_HAL_GPIO_INPUT);
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(GPIO_SW2));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(GPIO_SW2));
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);
    am_hal_interrupt_master_enable();

    //Configure mux controls prior to running
    mux_off();
    iox_write_pio(GPIO_UART_OB_PC_EN, 0);
    iox_write_pio(GPIO_UART_OB_PC_SEL, 0);
    

    while(1) {

        if (g_operational_mode != operational_mode)
        {
            operational_mode = g_operational_mode;
            led_indicate(operational_mode);
            switch(operational_mode)
            {
                case 1: // IOMx to 1.8V PSRAM, route all IOMs to their own 1.8V PSRAM
                    //Select Bits (where required)
                    iox_write_pio(GPIO_IOM0127_FRAM_PSRAM_SEL, 0); // Choose PSRAM for IOM0, 1, 2, and 7
                    iox_write_pio(GPIO_IOM12_PC_DISP_SEL, 2);

                    //Enable Bits
                    //IOM0
                    iox_write_pio(GPIO_IOM_LB_EN, 2); // tristate, enable pins have pullups on them
                    iox_write_pio(GPIO_IOM0_FRAM_PSRAM_EN, 0); // LOW, muxes are active low logic
                    //IOM1
                    iox_write_pio(GPIO_IOM1_FRAM_PSRAM_EN, 0);
                    iox_write_pio(GPIO_IOM1_PC_DISP_EN, 2);
                    //IOM2
                    iox_write_pio(GPIO_IOM2_FRAM_PSRAM_EN, 0);
                    iox_write_pio(GPIO_IOM2_PC_DISP_EN, 2);
                    //IOM3
                    iox_write_pio(GPIO_IOM3_PSRAM_EN, 0);
                    iox_write_pio(GPIO_IOM3_PC_FRAM_EN, 2);
                    //IOM4
                    iox_write_pio(GPIO_IOM4_PSRAM_EN, 0);
                    iox_write_pio(GPIO_IOM4_PC_FRAM_EN, 2);
                    //IOM5
                    iox_write_pio(GPIO_IOM5_PSRAM_EN, 0);
                    iox_write_pio(GPIO_IOM5_PC_FRAM_EN, 2);
                    //IOM6
                    iox_write_pio(GPIO_IOM6_PSRAM_EN, 0);
                    iox_write_pio(GPIO_IOM6_PC_FRAM_EN, 2);
                    //IOM7
                    iox_write_pio(GPIO_IOM7_FRAM_PSRAM_EN, 0);
                    iox_write_pio(GPIO_IOM7_PC_AP1IOS_EN, 2);
                    led_indicate(operational_mode);
                    break;
                case 2: // IOMx I2C to FRAM
                    //Select Bits (where required)
                    iox_write_pio(GPIO_IOM0127_FRAM_PSRAM_SEL, 1); // Choose FRAM for IOM0, 1, 2, and 7
                    iox_write_pio(GPIO_IOM3456_PC_FRAM_SEL, 0);    // Choose FRAM for IOM3, 4, 5, and 6
                    iox_write_pio(GPIO_IOM12_PC_DISP_SEL, 2);
                    //Enable Bits
                    //IOM0
                    iox_write_pio(GPIO_IOM_LB_EN, 2); // tristate, enable pins have pullups on them
                    iox_write_pio(GPIO_IOM0_FRAM_PSRAM_EN, 0); // LOW, muxes are active low logic
                    //IOM1
                    iox_write_pio(GPIO_IOM1_FRAM_PSRAM_EN, 0);
                    iox_write_pio(GPIO_IOM1_PC_DISP_EN, 2);
                    //IOM2
                    iox_write_pio(GPIO_IOM2_FRAM_PSRAM_EN, 0);
                    iox_write_pio(GPIO_IOM2_PC_DISP_EN, 2);
                    //IOM3
                    iox_write_pio(GPIO_IOM3_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM3_PC_FRAM_EN, 0);
                    //IOM4
                    iox_write_pio(GPIO_IOM4_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM4_PC_FRAM_EN, 0);
                    //IOM5
                    iox_write_pio(GPIO_IOM5_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM5_PC_FRAM_EN, 0);
                    //IOM6
                    iox_write_pio(GPIO_IOM6_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM6_PC_FRAM_EN, 0);
                    //IOM7
                    iox_write_pio(GPIO_IOM7_FRAM_PSRAM_EN, 0);
                    iox_write_pio(GPIO_IOM7_PC_AP1IOS_EN, 2);
                    led_indicate(operational_mode);
                    break;
                case 3: // IOM0 to IOS Loopback
                    iox_write_pio(GPIO_IOM0_FRAM_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM0_LB_SEL, 0);
                    iox_write_pio(GPIO_IOS_LB_PC_OB_EN, 0);
                    led_indicate(operational_mode);
                    break;
                case 4: // MSPI0/1 to APMemory Hex PSRAM, MSPI2 to Octal PSRAM
                    // Octal flash not populated on Rev 0 of the AP4P, so MSPI0 may not work properly.
                    // Octal flash for MSPI2 is instead routed to Octal PSRAM on board, instead of flash.
                    //        This can be changed by flipping the GPIO_MSPI2_FLASH_PSRAM_SEL bit

                    iox_write_pio(GPIO_MSPI01_FLASH_EN, 2);
                    iox_write_pio(GPIO_MSPI0_PC_OB_SEL, 0);
                    iox_write_pio(GPIO_MSPI0_PC_OB_EN, 0);
                    iox_write_pio(GPIO_MSPI1_PC_OB_SEL, 0);
                    iox_write_pio(GPIO_MSPI1_PC_OB_EN, 0);

                    iox_write_pio(GPIO_MSPI2_FLASH_PSRAM_EN, 0);
                    iox_write_pio(GPIO_MSPI2_FLASH_PSRAM_SEL, 1);
                    iox_write_pio(GPIO_LOWER_EMMC_PC_DISP_EN, 2);
                    iox_write_pio(GPIO_LOWER_EMMC_PC_DISP_SEL, 2);
                    iox_write_pio(GPIO_MSPI16_MUX_EN, 0);
                    iox_write_pio(GPIO_MSPI16_MUX_SEL, 1);
                    iox_write_pio(GPIO_DISP_GP_EN, 2);
                    led_indicate(operational_mode);
                    break;
                case 5: // MSPI0/1 to Winbond Hex PSRAM & MSPI2 to Display
                    //MSPI2 to Display
                    iox_write_pio(GPIO_MSPI2_PC_DISP_EN, 0);
                    iox_write_pio(GPIO_PC_DISP_EMMC_OB_SEL, 0);

                    iox_write_pio(GPIO_MSPI01_FLASH_EN, 2);
                    iox_write_pio(GPIO_MSPI0_PC_OB_SEL, 0);
                    iox_write_pio(GPIO_MSPI0_PC_OB_EN, 0);
                    iox_write_pio(GPIO_MSPI1_PC_OB_SEL, 0);
                    iox_write_pio(GPIO_MSPI1_PC_OB_EN, 0);
                    iox_write_pio(GPIO_MSPI16_MUX_EN, 0);
                    iox_write_pio(GPIO_MSPI16_MUX_SEL, 0);
                    iox_write_pio(GPIO_DISP_GP_EN, 2);
                    led_indicate(operational_mode);
                    break;
                case 6: // MSPI2 to Display, supporting logic to display for display shield.
                    // MSPI2 to display
                    iox_write_pio(GPIO_MSPI2_PC_DISP_EN, 0);
                    iox_write_pio(GPIO_PC_DISP_EMMC_OB_SEL, 0);
                    // IOM1 to display for ALS and Touch I2C, DISP_TE, and interrupt
                    iox_write_pio(GPIO_IOM0127_FRAM_PSRAM_SEL, 2);
                    iox_write_pio(GPIO_IOM12_PC_DISP_SEL, 0);
                    iox_write_pio(GPIO_IOM1_FRAM_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM1_PC_DISP_EN, 0);
                    // DISP_GPxx to display for additional control signals
                    iox_write_pio(GPIO_DISP_GP_EN, 0);
                    iox_write_pio(GPIO_MSPI1_PC_OB_SEL, 0);
                    // Disable MSPI1
                    iox_write_pio(GPIO_MSPI1_PC_OB_EN, 2);

                    led_indicate(operational_mode);
                    break;
                case 7: // Route everything to peripheral card

                    //MSPI0 to PC
                    iox_write_pio(GPIO_MSPI0_PC_OB_SEL, 1);
                    iox_write_pio(GPIO_MSPI0_PC_OB_EN, 0);
                    iox_write_pio(GPIO_MSPI01_FLASH_EN, 2);

                    //MSPI1 to PC
                    iox_write_pio(GPIO_MSPI1_PC_OB_SEL, 1);
                    iox_write_pio(GPIO_MSPI1_PC_OB_EN, 0);

                    //MSPI2 to PC
                    iox_write_pio(GPIO_EMMC_OB_EN, 2);
                    iox_write_pio(GPIO_PC_DISP_EMMC_OB_SEL, 1);
                    iox_write_pio(GPIO_MSPI2_PC_DISP_EN, 0);
                    iox_write_pio(GPIO_MSPI2_FLASH_PSRAM_EN, 2);
                    iox_write_pio(GPIO_MSPI2_FLASH_PSRAM_SEL, 2);

                    //eMMC bits not covered by MSPI2 Muxing to PC
                    iox_write_pio(GPIO_LOWER_EMMC_OB_EN, 2);
                    iox_write_pio(GPIO_LOWER_EMMC_PC_DISP_EN, 0);
                    iox_write_pio(GPIO_LOWER_EMMC_PC_DISP_SEL, 1);

                    //DISP GP pins to PC
                    iox_write_pio(GPIO_DISP_GP_EN, 0);

                    //IOS to PC
                    iox_write_pio(GPIO_IOM0_LB_SEL, 1);
                    iox_write_pio(GPIO_IOS_LB_PC_OB_EN, 0);

                    //JLINK UART pins to PC
                    iox_write_pio(GPIO_UART_OB_PC_EN, 0);
                    iox_write_pio(GPIO_UART_OB_PC_SEL, 1);

                    //IOM to PC
                    //IOM0
                    iox_write_pio(GPIO_IOM_LB_EN, 0);
                    iox_write_pio(GPIO_IOM0_FRAM_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM0127_FRAM_PSRAM_SEL, 2);
                    //IOM1
                    iox_write_pio(GPIO_IOM1_FRAM_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM1_PC_DISP_EN, 0);
                    iox_write_pio(GPIO_IOM12_PC_DISP_SEL, 1);
                    //IOM2
                    iox_write_pio(GPIO_IOM2_FRAM_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM2_PC_DISP_EN, 0);
                    //IOM3
                    iox_write_pio(GPIO_IOM3_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM3_PC_FRAM_EN, 0);
                    iox_write_pio(GPIO_IOM3456_PC_FRAM_SEL, 1);
                    //IOM4
                    iox_write_pio(GPIO_IOM4_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM4_PC_FRAM_EN, 0);
                    //IOM5
                    iox_write_pio(GPIO_IOM5_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM5_PC_FRAM_EN, 0);
                    //IOM6
                    iox_write_pio(GPIO_IOM6_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM6_PC_FRAM_EN, 0);
                    //IOM7
                    iox_write_pio(GPIO_IOM7_FRAM_PSRAM_EN, 2);
                    iox_write_pio(GPIO_IOM7_PC_AP1IOS_SEL, 1);
                    iox_write_pio(GPIO_IOM7_PC_AP1IOS_EN, 0);

                    led_indicate(operational_mode);
                    break;
                // DL 4/28/2020 - Removing ioe_state as not needed, adding off_mux state
                default: // All PIOs HIGH
                    mux_off();
                    
                    //Once all muxes are disabled, enable UART to JLINK mux
                    // Enable UART comms to onboard JLINK by default
                    iox_write_pio(GPIO_UART_OB_PC_EN, 0);
                    iox_write_pio(GPIO_UART_OB_PC_SEL, 0);
                    //g_ioe_state[0] = 0xFFFFFFFF;
                    //g_ioe_state[1] = 0x00FFFFFF;
                    //g_ioe_state[2] = 0xFFFFFFFF;
                    //g_ioe_state[3] = 0x00FFFFFF;
                    break;
            }
            //iox_update_state_all();
        }

        // TODO: Update this code for FPGA SPI writes to control muxes
        if (AM_REG(IOSLAVE, REGACCINTSTAT)) {                   // If interrupt is found, it means a PIO output needs to be changed

            for (uint32_t h = 0; h < 1000; h++) {
                cmd = AM_REGVAL(REG_IOSLAVE_BASEADDR);
            }
            pio_num = cmd & 0xFF;
            op = ((cmd & 0xFF00)>>8);

            // DL 4/28/2020 - FABD not needed
            //if (pio_num == 255 && op == 0) {
            //    fabd_unlock(TRIM);
            //}
            // if (pio_num == 255 && op == 1) {
            //    fabd_unlock(SECURE);
            //}

            if (pio_num == 20 || pio_num == 21 || pio_num < 7) {
                AM_REG(IOSLAVE, REGACCINTCLR) = 0xFFFFFFFF;     // Clear the interrupt
                continue;
            }

            iox_write_pio(pio_num, op);
            //if (pio_num > 49)
            //{
            //    iox_update_state_all();
            //}

            AM_REG(IOSLAVE, REGACCINTCLR) = 0xFFFFFFFF;         // Clear the interrupt
        }

    }

}


// Change the state of a single PIO
// DL 4/28/2020: PIO>50 not needed (no additional IOX on EB)
// Note for PIO >= 50, this only updates the g_ioe_state[]
// variables. It is necessary to call iox_update_state(0)
// and iox_update_state(1) after all pios have been written
void iox_write_pio(uint32_t pio_num, uint32_t op)
{
    //uint32_t offset = 0;
    //uint32_t ioe_state_index = 0;
    //uint32_t ioe_data = 0;
    //uint32_t ioe_mask = 0;

    //if (pio_num < 50) {
    if (op == 0) {
      // MSG(("set level =%d\n", op));
      am_hal_gpio_pin_config(pio_num, AM_HAL_PIN_OUTPUT | AM_HAL_GPIO_HIGH_DRIVE);
      am_hal_gpio_out_enable_bit_set(pio_num);
      am_hal_gpio_out_bit_clear(pio_num);
    }
    else if (op == 1) {
      // MSG(("set level =%d\n", op));
      am_hal_gpio_pin_config(pio_num, AM_HAL_PIN_OUTPUT | AM_HAL_GPIO_HIGH_DRIVE);
      am_hal_gpio_out_enable_bit_set(pio_num);
      am_hal_gpio_out_bit_set(pio_num);
    }
    else if (op == 2) {
      // MSG(("tristate PIO \n"));
      am_hal_gpio_pin_config(pio_num, AM_HAL_PIN_3STATE);
      am_hal_gpio_out_enable_bit_clear(pio_num);
    }

}

// DL 4/28/2020: Updated to add ability to toggle each LED
void toggle_led (uint32_t led_gpio, uint32_t num_times, uint32_t delay_ms) {
    uint32_t k;
    for (k = 0; k < (num_times); k++) {
        am_hal_gpio_out_bit_toggle(led_gpio);
        am_util_delay_ms(delay_ms);
        am_hal_gpio_out_bit_toggle(led_gpio);
        am_util_delay_ms(delay_ms);
    }
}

// DL 4/28/2020: Added function to handle LED illumination
// for different configuration states
void led_indicate (uint32_t state_num) {
    if (state_num == 0) {
        // toggle_led(5, 500);
        iox_write_pio(GPIO_LED_BLUE_2, 0);
        iox_write_pio(GPIO_LED_BLUE_1, 0);
        iox_write_pio(GPIO_LED_BLUE_0, 0);
    }
    else if (state_num == 1){
        iox_write_pio(GPIO_LED_BLUE_2, 1);
        iox_write_pio(GPIO_LED_BLUE_1, 0);
        iox_write_pio(GPIO_LED_BLUE_0, 0);
    }
    else if (state_num == 2){
        iox_write_pio(GPIO_LED_BLUE_2, 0);
        iox_write_pio(GPIO_LED_BLUE_1, 1);
        iox_write_pio(GPIO_LED_BLUE_0, 0);
    }
    else if (state_num == 3){
        iox_write_pio(GPIO_LED_BLUE_2, 1);
        iox_write_pio(GPIO_LED_BLUE_1, 1);
        iox_write_pio(GPIO_LED_BLUE_0, 0);
    }
    else if (state_num == 4){
        iox_write_pio(GPIO_LED_BLUE_2, 0);
        iox_write_pio(GPIO_LED_BLUE_1, 0);
        iox_write_pio(GPIO_LED_BLUE_0, 1);
    }
    else if (state_num == 5){
        iox_write_pio(GPIO_LED_BLUE_2, 1);
        iox_write_pio(GPIO_LED_BLUE_1, 0);
        iox_write_pio(GPIO_LED_BLUE_0, 1);
    }
    else if (state_num == 6){
        iox_write_pio(GPIO_LED_BLUE_2, 0);
        iox_write_pio(GPIO_LED_BLUE_1, 1);
        iox_write_pio(GPIO_LED_BLUE_0, 1);
    }
    else if (state_num == 7){
        iox_write_pio(GPIO_LED_BLUE_2, 1);
        iox_write_pio(GPIO_LED_BLUE_1, 1);
        iox_write_pio(GPIO_LED_BLUE_0, 1);
    }
}

void mux_off (void) {
// Tristate all GPIOs for AP1, all mux EN pins have pull-ups
    iox_write_pio(GPIO_EMMC_OB_EN, 2);
    iox_write_pio(GPIO_PC_DISP_EMMC_OB_SEL, 2);
    iox_write_pio(GPIO_MSPI2_PC_DISP_EN, 2);
    iox_write_pio(GPIO_IOM0_LB_SEL, 2);
    iox_write_pio(GPIO_IOM_LB_EN, 2);
    iox_write_pio(GPIO_IOM1_PC_DISP_EN, 2);
    iox_write_pio(GPIO_IOM12_PC_DISP_SEL, 2);
    iox_write_pio(GPIO_IOM2_PC_DISP_EN, 2);
    iox_write_pio(GPIO_IOM3_PC_FRAM_EN, 2);
    iox_write_pio(GPIO_IOM3456_PC_FRAM_SEL, 2);
    iox_write_pio(GPIO_IOM4_PC_FRAM_EN, 2);
    iox_write_pio(GPIO_IOM5_PC_FRAM_EN, 2);
    iox_write_pio(GPIO_IOM6_PC_FRAM_EN, 2);
    iox_write_pio(GPIO_IOM7_PC_AP1IOS_SEL, 2);
    iox_write_pio(GPIO_IOM7_PC_AP1IOS_EN, 2);
    iox_write_pio(GPIO_MSPI01_FLASH_EN, 2);
    iox_write_pio(GPIO_IOS_LB_PC_OB_EN, 2);
    iox_write_pio(GPIO_LOWER_EMMC_OB_EN, 2);
    iox_write_pio(GPIO_MSPI0_PC_OB_SEL, 2);
    iox_write_pio(GPIO_MSPI0_PC_OB_EN, 2);
    iox_write_pio(GPIO_MSPI1_PC_OB_SEL, 2);
    iox_write_pio(GPIO_MSPI1_PC_OB_EN, 2);
    iox_write_pio(GPIO_UART_OB_PC_EN, 2);
    iox_write_pio(GPIO_UART_OB_PC_SEL, 2);
    iox_write_pio(GPIO_MSPI2_FLASH_PSRAM_EN, 2);
    iox_write_pio(GPIO_LOWER_EMMC_PC_DISP_EN, 2);
    iox_write_pio(GPIO_LOWER_EMMC_PC_DISP_SEL, 2);
    iox_write_pio(GPIO_MSPI16_MUX_EN, 2);
    iox_write_pio(GPIO_DISP_GP_EN, 2);
}

void am_gpio_isr(void)
{
    // Debounce
    am_util_delay_ms(200);

    // Clear interrupt
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(GPIO_SW2));

    g_operational_mode += 1;
    g_operational_mode &= 0x7;
}
