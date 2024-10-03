//////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Module:  iox4
// Author:   Xiaoping Yang
// Date: 11/26/2021
// Description: Updated Apollo4p MCU Card Apollo IO expander code to support rev.C silicon
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "am_mcu_apollo.h"
#include "iox4.h"
#include <stdio.h>
#include "am_util_delay.h"
#include <string.h>

#define TRIM 0
#define SECURE 1 // Apollo3/3c/4a
#define SECURE_AP4B 2 // Apollo4b
#define SWDIO 8
#define SWDCK 7

// Prototypes
//int main(void) __attribute__((section("data")));
//int main(void) CODE_SECTION;
void toggle_led (uint32_t, uint32_t);
void update_amux_sel (uint32_t, uint32_t);
void fabd_unlock (uint32_t);
void iox_update_state(uint32_t iox_index);
void iox_update_state_all(void);
void iox_write_pio(uint32_t pio_num, uint32_t op);
void iox_set_default_states(void);

uint32_t g_operational_mode = MODE_DEFAULT;
uint32_t g_ioe_state[4] = {0xFFFFFFFF, 0x00FFFFFF, 0xFFFFFFFF, 0x00FFFFFF};

// Main code
int main (void) {
    uint32_t k;
    uint32_t cmd;
    uint32_t pio_num, op;
    uint32_t operational_mode = 0xFFFFFFFF;

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

    //Pads remain same for apollo1 and apollo3
    AM_REG(GPIO, PADREGB) = AM_REG_GPIO_PADREGB_PAD5FNCSEL(0) |
                            AM_REG_GPIO_PADREGB_PAD6FNCSEL(0) |
                            AM_REG_GPIO_PADREGB_PAD6PULL(1) |
                            AM_REG_GPIO_PADREGB_PAD5PULL(1) |
                            AM_REG_GPIO_PADREGB_PAD5INPEN(1) |
                            AM_REG_GPIO_PADREGB_PAD6INPEN(1)     ;

    //--------------------------------------------------------
    // Set up the IOM device
    //--------------------------------------------------------

    // Defaults for the device

    AM_REGVAL(0x50005208) = 0xFFFFFFFF;
    while ((AM_REGVAL(0x50005118) & 4) != 4 );
    AM_REGVAL(0x50005108) = 0x40C;    //defaults from Ron
    AM_REGVAL(0x5000411C) = 0x80000000;     //IOMSTR0 CFG
    AM_REGVAL(0x5000410C) = 0x170b1100;     //CLK CFG
                        // AM_REG_IOMSTR0_CLKCFG_FSEL(1) |
                         //     AM_REG_IOMSTR0_DIV3(0) |
                          //     AM_REG_IOMSTR0_DIVEN(1) |
                           //      AM_REG_IOMSTR0_LOWPER(b) |
                            //      AM_REG_IOMSTR0_TOTPER(0x17);


//    iox_update_state_all();

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
#if defined(BOARD_FALCON_PS_CARD)
    AM_REGVAL(REG_IOSLAVE_BASEADDR+0x40) = 0x21;
#elif defined(BOARD_FALCON_SIP_MCU_CARD_REVA)
    AM_REGVAL(REG_IOSLAVE_BASEADDR+0x40) = 0x22;
#elif defined(BOARD_FALCON_BGA_MCU_CARD_REVA)
    AM_REGVAL(REG_IOSLAVE_BASEADDR+0x40) = 0x23;
#elif defined(BOARD_FALCON_SIP_MCU_CARD_REVB)
    AM_REGVAL(REG_IOSLAVE_BASEADDR+0x40) = 0x24;
#elif defined(BOARD_FALCON_BGA_MCU_CARD_REVB)
    AM_REGVAL(REG_IOSLAVE_BASEADDR+0x40) = 0x25;
#endif
    AM_REGVAL(REG_IOSLAVE_BASEADDR+0x41) = 0xA5;
    //AM_REGVAL(REG_IOSLAVE_BASEADDR+0x42) = 0x0D; //version 13. Rev.B silicon FABD and Rev.B MCU card support
    AM_REGVAL(REG_IOSLAVE_BASEADDR+0x42) = 0x0E; //version 14. Rev.B power mux inversion

    for (k=7; k<50; k++) {
        if (k == 20 || k == 21) {
            continue;
        } else {
            am_hal_gpio_pin_config(k, AM_HAL_PIN_3STATE); // tristate output on every pad after startup
        }
    }

    // Configure the LED and toggle twice
    am_hal_gpio_pin_config(GPIO_LED_BLUE, AM_HAL_GPIO_OUTPUT);
    toggle_led(3,300);
    am_hal_gpio_out_bit_set(GPIO_LED_BLUE);

    am_hal_gpio_pin_config(GPIO_SW2, AM_HAL_GPIO_INPUT);
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(GPIO_SW2));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(GPIO_SW2));
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);
    am_hal_interrupt_master_enable();

//    iox_set_default_states();
//    iox_update_state_all();

    while(1)
    {
        if(g_operational_mode != operational_mode)
        {
            operational_mode = g_operational_mode;
            toggle_led(operational_mode, 200);
            switch(operational_mode)
            {
                case 0: // Default mode. MSPI2 QSPI Display & MSPI0 to 1.8V PSRAM
                    iox_set_default_states();
                    //MSPI2 Muxes
                    iox_write_pio(GPIO_MSPI2_SEL3, 0);
                    iox_write_pio(GPIO_MSPI2_SEL2, 1);
                    iox_write_pio(GPIO_MSPI2_SEL1, 0);
                    iox_write_pio(GPIO_MSPI2_SEL0, 0);
                    //MSPI0 Muxes
                    iox_write_pio(GPIO_MSPI0_SEL3, 0);
                    iox_write_pio(GPIO_MSPI0_SEL2, 0);
                    iox_write_pio(GPIO_MSPI0_SEL1, 1);
                    iox_write_pio(GPIO_MSPI0_SEL0, 0);
                    //Get GP52 for CEn to FPGA Header
                    iox_write_pio(GPIO_MSPI02_PSRAM_CE_SEL1, 1);
                    iox_write_pio(GPIO_MSPI02_PSRAM_CE_SEL0, 0);
                    //Get MSPI0 and new MSPI1 pins to FPGA Header
                    iox_write_pio(GPIO_UPPER_IO_CTRL, 0);
                    iox_write_pio(GPIO_PC_SEL, 0);
                    iox_write_pio(GPIO_AT_UART_SEL, 1);
                    //IOM Muxes
                    iox_write_pio(GPIO_IOM0_LB_SEL, 1);
                    iox_write_pio(GPIO_IOM_PC_SEL, 0);
                    iox_write_pio(GPIO_IOM_ANA_SEL, 0);
                    iox_write_pio(GPIO_IOM_VOLT_SEL, 0);
                    iox_write_pio(GPIO_IOM1_ANA_SEL, 1);
                    iox_write_pio(GPIO_IOM_MEM_SEL, 1);
                    //break;
                case 1: // IOMx SPI to 1.8V PSRAM
                    iox_write_pio(GPIO_IOM0_LB_SEL, 0);
                    iox_write_pio(GPIO_IOM1_ANA_SEL, 0);
                    iox_write_pio(GPIO_IOM_PC_SEL, 0);
                    iox_write_pio(GPIO_IOM_ANA_SEL, 0);
                    iox_write_pio(GPIO_IOM_VOLT_SEL, 1);
                    iox_write_pio(GPIO_IOM_MEM_SEL, 0);
                    break;
                case 2: // IOMx I2C to FRAM
                    iox_write_pio(GPIO_IOM_ANA_SEL, 1);
                    iox_write_pio(GPIO_IOM_VOLT_SEL, 1);
                    iox_write_pio(GPIO_IOM0_LB_SEL, 0);
                    iox_write_pio(GPIO_IOM_PC_SEL, 0);
                    iox_write_pio(GPIO_IOM_MEM_SEL, 0);
                    iox_write_pio(GPIO_IOM1_ANA_SEL, 0);
                    break;
                case 3: // IOM0 to IOS Loopback
                    //IOM0 Muxes
                    iox_write_pio(GPIO_IOM0_LB_SEL, 1);
                    iox_write_pio(GPIO_IOM_PC_SEL, 1);
                    iox_write_pio(GPIO_IOM_ANA_SEL, 0);
					iox_write_pio(GPIO_IOM_VOLT_SEL, 0);
                    iox_write_pio(GPIO_IOM1_ANA_SEL, 1);
                    iox_write_pio(GPIO_IOM_MEM_SEL, 0);
					//IOS Muxes
                    iox_write_pio(GPIO_IOS_LB_SEL, 1);
					iox_write_pio(GPIO_AT_UART_SEL, 0);
					iox_write_pio(GPIO_PC_SEL, 0);
					// High Z the MSPI0 & MSPI1 Muxes
					iox_write_pio(GPIO_MSPI2_SEL3, 0);
                    iox_write_pio(GPIO_MSPI2_SEL2, 0);
                    iox_write_pio(GPIO_MSPI2_SEL1, 0);
                    iox_write_pio(GPIO_MSPI2_SEL0, 0);
                    iox_write_pio(GPIO_MSPI0_SEL3, 0);
                    iox_write_pio(GPIO_MSPI0_SEL2, 0);
                    iox_write_pio(GPIO_MSPI0_SEL1, 0);
                    iox_write_pio(GPIO_MSPI0_SEL0, 0);
                    break;
                case 4: // MSPI0&MSPI2 to 1.8V Quad PSRAM + MSPI0 Octal SPI Flash
                    //MSPI2 Muxes
                    iox_write_pio(GPIO_MSPI2_SEL3, 0);
                    iox_write_pio(GPIO_MSPI2_SEL2, 1);
                    iox_write_pio(GPIO_MSPI2_SEL1, 0);//modified
                    iox_write_pio(GPIO_MSPI2_SEL0, 0);//modified
                    //MSPI0 Muxes
                    iox_write_pio(GPIO_MSPI0_SEL3, 0);
                    iox_write_pio(GPIO_MSPI0_SEL2, 1);
                    iox_write_pio(GPIO_MSPI0_SEL1, 0);//modified
                    iox_write_pio(GPIO_MSPI0_SEL0, 0);//modified
                    //IOS Muxes and Additional I/O Muxes(MSPI0 & MSPI2 CE Muxes)
                    iox_write_pio(GPIO_MSPI02_PSRAM_CE_SEL1, 1);
                    iox_write_pio(GPIO_MSPI02_PSRAM_CE_SEL0, 1);
                    iox_write_pio(GPIO_IOS_LB_SEL, 0);//modified
					iox_write_pio(GPIO_AT_UART_SEL, 0);
                    iox_write_pio(GPIO_PC_SEL, 0);
                    iox_write_pio(GPIO_UPPER_IO_CTRL, 0);
                    //IOM Muxes
                    iox_write_pio(GPIO_IOM0_LB_SEL, 0);//modified
                    iox_write_pio(GPIO_IOM_PC_SEL, 0);
                    iox_write_pio(GPIO_IOM_ANA_SEL, 0);
                    iox_write_pio(GPIO_IOM_VOLT_SEL, 0);
                    iox_write_pio(GPIO_IOM1_ANA_SEL, 0);
                    iox_write_pio(GPIO_IOM_MEM_SEL, 0);
                    break;
                case 5: // MSPI0 to Display
                    //MSPI0 Muxes
                    iox_write_pio(GPIO_MSPI0_SEL3, 1);
                    iox_write_pio(GPIO_MSPI0_SEL2, 1);
                    iox_write_pio(GPIO_MSPI0_SEL1, 0);
                    iox_write_pio(GPIO_MSPI0_SEL0, 0);
                    //MSPI2 Muxes
                    iox_write_pio(GPIO_MSPI2_SEL3, 0);
                    iox_write_pio(GPIO_MSPI2_SEL2, 0);
                    iox_write_pio(GPIO_MSPI2_SEL1, 0);
                    iox_write_pio(GPIO_MSPI2_SEL0, 0);
                    //IOM Muxes
                    iox_write_pio(GPIO_IOM0_LB_SEL, 0);
                    iox_write_pio(GPIO_IOM_PC_SEL, 0);
                    iox_write_pio(GPIO_IOM_ANA_SEL, 0);
                    iox_write_pio(GPIO_IOM_VOLT_SEL, 0);
                    iox_write_pio(GPIO_IOM1_ANA_SEL, 0);
                    iox_write_pio(GPIO_IOM_MEM_SEL, 0);
                    break;
                case 6: // DC QSPI Display & MSPI0 to 1.8V PSRAM
                    //MSPI2 Muxes
                    iox_write_pio(GPIO_MSPI2_SEL3, 1);
                    iox_write_pio(GPIO_MSPI2_SEL2, 1);
                    iox_write_pio(GPIO_MSPI2_SEL1, 0);
                    iox_write_pio(GPIO_MSPI2_SEL0, 0);
                    //MSPI0 Muxes
                    iox_write_pio(GPIO_MSPI0_SEL3, 0);
                    iox_write_pio(GPIO_MSPI0_SEL2, 0);
                    iox_write_pio(GPIO_MSPI0_SEL1, 1);
                    iox_write_pio(GPIO_MSPI0_SEL0, 0);
                    //Get GP52 for CEn to FPGA Header
                    iox_write_pio(GPIO_MSPI02_PSRAM_CE_SEL1, 1);
                    iox_write_pio(GPIO_MSPI02_PSRAM_CE_SEL0, 0);
                    //Get MSPI0 and new MSPI1 pins to FPGA Header
                    iox_write_pio(GPIO_UPPER_IO_CTRL, 0);
                    iox_write_pio(GPIO_PC_SEL, 0);
                    iox_write_pio(GPIO_AT_UART_SEL, 1);
                    //IOM Muxes
                    iox_write_pio(GPIO_IOM0_LB_SEL, 0);
                    iox_write_pio(GPIO_IOM_PC_SEL, 0);
                    iox_write_pio(GPIO_IOM_ANA_SEL, 0);
                    iox_write_pio(GPIO_IOM_VOLT_SEL, 0);
                    iox_write_pio(GPIO_IOM1_ANA_SEL, 0);
                    iox_write_pio(GPIO_IOM_MEM_SEL, 0);
                    break;
                case 7: // Peripheral Card Connections(what is this?)
                    iox_write_pio(GPIO_PC_SEL, 1);
                    iox_write_pio(GPIO_UPPER_IO_CTRL, 0);
                    iox_write_pio(GPIO_IOM_PC_SEL, 1);
                    iox_write_pio(GPIO_IOM_MEM_SEL, 0);
#ifdef BOARD_FALCON_EB
                    iox_write_pio(GPIO_DMUX_NC_SEL, 0);
                    iox_write_pio(GPIO_IOM_NC_SEL, 0);
                    iox_write_pio(GPIO_IOM1_DISP_SEL, 0);
                    iox_write_pio(GPIO_IOM7_NC_SEL, 0);
#endif
                    iox_write_pio(GPIO_MSPI02_PSRAM_CE_SEL1, 0);
                    iox_write_pio(GPIO_MSPI02_PSRAM_CE_SEL0, 1);
                    iox_write_pio(GPIO_MSPI2_SEL1, 0);
                    iox_write_pio(GPIO_MSPI2_SEL0, 1);
                    iox_write_pio(GPIO_MSPI0_SEL1, 0);
                    iox_write_pio(GPIO_MSPI0_SEL0, 1);
                    break;
                default: // All PIOs HIGH
                    g_ioe_state[0] = 0xFFFFFFFF;
                    g_ioe_state[1] = 0x00FFFFFF;
                    g_ioe_state[2] = 0xFFFFFFFF;
                    g_ioe_state[3] = 0x00FFFFFF;
                    break;
            }
            iox_update_state_all();
        }
        // TODO: Update this code for FPGA SPI writes to control muxes
        if(AM_REG(IOSLAVE, REGACCINTSTAT))
        {                   // If interrupt is found, it means a PIO output needs to be changed
            for (uint32_t h = 0; h < 1000; h++)
            {
                cmd = AM_REGVAL(REG_IOSLAVE_BASEADDR);
            }
            pio_num = cmd & 0xFF;
            op = ((cmd & 0xFF00)>>8);
            if (pio_num == 255 && op == 0)
            {
                fabd_unlock(TRIM);
            }
            if (pio_num == 255 && op == 1)
            {
                fabd_unlock(SECURE);
            }
            if (pio_num == 254 && op == 1)
            {
                fabd_unlock(SECURE_AP4B);
            }
            if (pio_num == 20 || pio_num == 21 || pio_num < 7)
            {
                AM_REG(IOSLAVE, REGACCINTCLR) = 0xFFFFFFFF;     // Clear the interrupt
                continue;
            }
            iox_write_pio(pio_num, op);
            if (pio_num > 49)
            {
                iox_update_state_all();
            }
            AM_REG(IOSLAVE, REGACCINTCLR) = 0xFFFFFFFF;         // Clear the interrupt
        }
    }
}

// Write latest state to the IOX
void iox_update_state(uint32_t iox_index)
{
    if (iox_index == 0)
    {
        // Load the FIFO
        AM_REGVAL(0x50004000) = g_ioe_state[0];
        AM_REGVAL(0x50004000) = g_ioe_state[1];

        // Wait for FIFO load complete
        while ((AM_REGVAL(0x50004100) & 0x3F) < 8);

        // Execute Write command
        AM_REGVAL(0x50004110) = 0x40200006;

    }
    else if (iox_index == 1)
    {
        // Load the FIFO
        AM_REGVAL(0x50004000) = g_ioe_state[2];
        AM_REGVAL(0x50004000) = g_ioe_state[3];

        // Wait for FIFO load complete
        while ((AM_REGVAL(0x50004100) & 0x3F) < 8);

        // Execute Write command
        AM_REGVAL(0x50004110) = 0x40210006;
    }

    while (AM_REGVAL(0x50004118) != 4);

}

// Update all of the IOXs
void iox_update_state_all(void)
{
    iox_update_state(0);
    iox_update_state(1);
}

// void iox_write_pio(uint32_t pio_num, uint32_t op)
//
// Change the state of a single PIO
//
// Note for PIO >= 50, this only updates the g_ioe_state[]
// variables. It is necessary to call iox_update_state(0)
// and iox_update_state(1) after all pios have been written
void iox_write_pio(uint32_t pio_num, uint32_t op)
{
    uint32_t offset = 0;
    uint32_t ioe_state_index = 0;
    uint32_t ioe_data = 0;
    uint32_t ioe_mask = 0;
    if ( (pio_num > 12 && pio_num < 17) || (pio_num == 18) || (pio_num > 50 && pio_num < 56) || (pio_num == 61) ||
        (pio_num == 64) || (pio_num == 74) || (pio_num > 97 && pio_num < 118) )
    {
        if ( (op == 0) || (op == 1) ) {
            op = 1 - op;
        }
    }
    if (pio_num < 50) {
      if (op == 0) {
        // MSG(("set level =%d\n", op));
        am_hal_gpio_pin_config(pio_num, AM_HAL_PIN_OUTPUT | AM_HAL_GPIO_HIGH_DRIVE);
        am_hal_gpio_out_enable_bit_set(pio_num);
        am_hal_gpio_out_bit_clear(pio_num);
      }
      if (op == 1) {
        // MSG(("set level =%d\n", op));
        am_hal_gpio_pin_config(pio_num, AM_HAL_PIN_OUTPUT | AM_HAL_GPIO_HIGH_DRIVE);
        am_hal_gpio_out_enable_bit_set(pio_num);
        am_hal_gpio_out_bit_set(pio_num);
      }
      if (op == 2) {
        // MSG(("tristate PIO \n"));
        am_hal_gpio_pin_config(pio_num, AM_HAL_PIN_3STATE);
        am_hal_gpio_out_enable_bit_clear(pio_num);
      }
    }
    else
    {
        // IOX pio
        if (pio_num < 82)
        {
            offset = 50;
            ioe_state_index = 0;
        }
        else if (pio_num < 98)
        {
            offset = 82;
            ioe_state_index = 1;
        }
        else if (pio_num < 130)
        {
            offset = 98;
            ioe_state_index = 2;
        }
        else if (pio_num < 146)
        {
            offset = 130;
            ioe_state_index = 3;
        }
        ioe_mask = (1 << (pio_num - offset));
        ioe_data = g_ioe_state[ioe_state_index];
        if (op == 0) {
           if ((g_ioe_state[ioe_state_index] & ioe_mask) == ioe_mask) {
              g_ioe_state[ioe_state_index] = ioe_data - ioe_mask;
           }
        } else if (op == 1) {
           g_ioe_state[ioe_state_index] = g_ioe_state[ioe_state_index] | ioe_mask;
        } else {
         //do nothing
        }
    }


}

void toggle_led (uint32_t num_times, uint32_t delay_ms) {
    uint32_t k;
    for (k = 0; k < (num_times); k++) {
        am_hal_gpio_out_bit_toggle(GPIO_LED_BLUE);
        am_util_delay_ms(delay_ms);
        am_hal_gpio_out_bit_toggle(GPIO_LED_BLUE);
        am_util_delay_ms(delay_ms);
    }
}

void fabd_unlock (uint32_t type) {
    uint32_t i,j;
    //uint32_t trim1, trim2, trim3;
    uint32_t unlock_trim_seq[3] = {0x00FFFFFF, 0xA8FBFFFF, 0x77331303};
    uint32_t unlock_secure_seq[3] = {0x0D5C0CEF, 0x00780D5C, 0x67670357};
    uint32_t unlock_secure_seq_ap4b[11] = {0xFFFFFFFC, 0x000FFFFF, 0xE4FFFF30, 0x4FF86F3A, 0x80E4E1C7, 0x069D2A7C, 0xDACED7B6, 0x3AA2F66D, 0x255CAF5D, 0x6176EBDE, 0x00014F23};
    //FABD unlock trim sequence
    //trim1 = 0x00FFFFFF;
    //trim2 = 0xA8FBFFFF;
    //trim3 = 0x77331303;

    //Drive SWDCK low
    am_hal_gpio_pin_config(SWDCK, AM_HAL_PIN_OUTPUT | AM_HAL_GPIO_HIGH_DRIVE);
    am_hal_gpio_out_enable_bit_set(SWDCK);
    am_hal_gpio_out_bit_clear(SWDCK);

    //Drive SWDIO low
    am_hal_gpio_pin_config(SWDIO, AM_HAL_PIN_OUTPUT | AM_HAL_GPIO_HIGH_DRIVE);
    am_hal_gpio_out_enable_bit_set(SWDIO);
    am_hal_gpio_out_bit_clear(SWDIO);

    //Switch debugger connection
    am_hal_gpio_pin_config(9, AM_HAL_PIN_OUTPUT | AM_HAL_GPIO_HIGH_DRIVE);
    am_hal_gpio_pin_config(10, AM_HAL_PIN_OUTPUT | AM_HAL_GPIO_HIGH_DRIVE);  //jh added
    am_hal_gpio_pin_config(11, AM_HAL_PIN_OUTPUT | AM_HAL_GPIO_HIGH_DRIVE);  //jh added
    am_hal_gpio_out_enable_bit_set(9);
    am_hal_gpio_out_enable_bit_set(10); //jh added
    am_hal_gpio_out_enable_bit_set(11); //jh added
    for (i = 0; i < 1000; i++) {
        am_hal_gpio_out_bit_set(9); //jh changed
        am_hal_gpio_out_bit_set(10); //jh added
        am_hal_gpio_out_bit_clear(11); //jh added
    }
    //Send FABD unlock trim sequence on SWDCK and SWDIO
    if (type == TRIM) {
        for (i = 0; i < 24; i++) {
            //Read SWDIO sequence and change state
            if ( ((unlock_trim_seq[0]>>i)&1) == 1 ) {
                am_hal_gpio_out_bit_set(SWDIO);
            } else {
                am_hal_gpio_out_bit_clear(SWDIO);
            }
            //clock SWDCK
            am_hal_gpio_out_bit_set(SWDCK);
            am_hal_gpio_out_bit_clear(SWDCK);
        }
        for (j = 1; j < 3; j++) {
            for (i = 0; i < 32; i++) {
                //Read SWDIO sequence and change state
                if ( ((unlock_trim_seq[j]>>i)&1) == 1 ) {
                    am_hal_gpio_out_bit_set(SWDIO);
                } else {
                    am_hal_gpio_out_bit_clear(SWDIO);
                }
                //clock SWDCK
                am_hal_gpio_out_bit_set(SWDCK);
                am_hal_gpio_out_bit_clear(SWDCK);
            }
        }
    }
    //Send FABD unlock secure sequence on SWDCK and SWDIO for Apollo3/3c/4a
    if (type == SECURE) {
        for (j = 0; j < 3; j++) {
            for (i = 0; i < 32; i++) {
                //Read SWDIO sequence and change state
                if ( ((unlock_secure_seq[j]>>i)&1) == 1 ) {
                    am_hal_gpio_out_bit_set(SWDIO);
                } else {
                    am_hal_gpio_out_bit_clear(SWDIO);
                }
                //clock SWDCK
                am_hal_gpio_out_bit_set(SWDCK);
                am_hal_gpio_out_bit_clear(SWDCK);
            }
        }
    }

    //Send FABD unlock secure sequence on SWDCK and SWDIO for Apollo4b
    if (type == SECURE_AP4B) {
        for (j = 0; j < 11; j++) {
            for (i = 0; i < 32; i++) {
                //Read SWDIO sequence and change state
                if ( ((unlock_secure_seq_ap4b[j]>>i)&1) == 1 ) {
                    am_hal_gpio_out_bit_set(SWDIO);
                } else {
                    am_hal_gpio_out_bit_clear(SWDIO);
                }
                //clock SWDCK
                am_hal_gpio_out_bit_set(SWDCK);
                am_hal_gpio_out_bit_clear(SWDCK);
            }
        }
    }

    if (type != SECURE_AP4B) {
        //Send bus idle and reset
        am_hal_gpio_out_bit_clear(SWDIO);
        for (i = 0; i < 14; i++) {
            am_hal_gpio_out_bit_set(SWDCK);
            am_hal_gpio_out_bit_clear(SWDCK);
        }
        am_hal_gpio_out_bit_set(SWDCK);
        for (i = 0; i < 50; i++) {
            am_hal_gpio_out_bit_set(SWDCK);
            am_hal_gpio_out_bit_clear(SWDCK);
        }
    }

    for (i = 0; i < 1000; i++) {
        am_hal_gpio_out_bit_clear(9);
    }
    //Switch debugger connection
    am_hal_gpio_pin_config(9, AM_HAL_PIN_3STATE); //jh updated
    am_hal_gpio_pin_config(10, AM_HAL_PIN_3STATE); //jh added
    //am_hal_gpio_pin_config(11, AM_HAL_PIN_3STATE); //jh added
    //am_hal_gpio_out_bit_clear(9); //jh changed
    //am_hal_gpio_out_bit_set(10); //jh added
    am_hal_gpio_out_bit_set(11); //jh added
    //Tristate SWDIO and SWDCK
    am_hal_gpio_pin_config(SWDIO, AM_HAL_PIN_3STATE);
    am_hal_gpio_pin_config(SWDCK, AM_HAL_PIN_3STATE);
}

void iox_set_default_states(void)
{
    // Power Muxes
    iox_write_pio(GPIO_CTRL_RAIL_PSU, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDC, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDC_LV, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDF, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDS, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDBH, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDC_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDC_LV_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDF_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDS_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDSBH_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_PER, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_PER_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDUSB0P9, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDUSB0P9_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDUSB33, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDUSB33_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDAUDA, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDAUDA_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDD18, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDD18_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDAUDD, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDB, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDH2, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDP, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDH, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDA, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDAUDD_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDB_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDH2_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDP_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDH_A, POWER_MUX_STATE);
    iox_write_pio(GPIO_CTRL_VDDA_A, POWER_MUX_STATE);

    // Other muxes
    iox_write_pio(GPIO_USB_PWR_EN, 0);
    iox_write_pio(GPIO_SHELBY_ADC_REF_LOAD_EN, 0);
    iox_write_pio(GPIO_CTRL_VCCIO_FPGA, 0);
    iox_write_pio(GPIO_VDDC_ISINK_EN, 0);
    iox_write_pio(GPIO_VDDC_ISINK_RANGE_SEL_A0, 0);
    iox_write_pio(GPIO_VDDC_ISINK_RANGE_SEL_A1, 0);
    iox_write_pio(GPIO_VDDM_ISINK_EN, 0);
    iox_write_pio(GPIO_VDDM_ISINK_RANGE_SEL_A0, 0);
    iox_write_pio(GPIO_VDDM_ISINK_RANGE_SEL_A1, 0);
    iox_write_pio(GPIO_PAD_TIA_RSEL_A0, 0);
    iox_write_pio(GPIO_PAD_TIA_RSEL_A1, 0);
    iox_write_pio(GPIO_SHELBY_IBIAS_SEL, 0);
    iox_write_pio(GPIO_XTAL_CL_SEL_A0, 0);
    iox_write_pio(GPIO_XTAL_CL_SEL_A1, 0);
    iox_write_pio(GPIO_DAC_AD5791_CLRb, 1);
    iox_write_pio(GPIO_AMUX_11_SEL1, 0);
    iox_write_pio(GPIO_AMUX_11_SEL0, 1);
    iox_write_pio(GPIO_AMUX_12_13_SEL1, 0);
    iox_write_pio(GPIO_AMUX_12_13_SEL0, 1);
    iox_write_pio(GPIO_AMUX_14_15_SEL1, 0);
    iox_write_pio(GPIO_AMUX_14_15_SEL0, 1);
    iox_write_pio(GPIO_DAQ_ADC5_SEL0, 0);
    iox_write_pio(GPIO_DAQ_ADC5_SEL1, 0);
    iox_write_pio(GPIO_DAQB_ADC67_SEL1, 1);
    iox_write_pio(GPIO_DAQB_ADC67_SEL0, 1);
    iox_write_pio(GPIO_DAQ_ADC67_SEL0, 0);
    iox_write_pio(GPIO_DAQ_ADC67_SEL1, 1);
    iox_write_pio(GPIO_AMUX_18_19_SEL0, 1);
    iox_write_pio(GPIO_AMUX_18_19_SEL1, 0);
    iox_write_pio(GPIO_AMUX_16_17_SEL0, 1);
    iox_write_pio(GPIO_AMUX_16_17_SEL1, 0);
    iox_write_pio(GPIO_DAC20_FILT_SEL0, 1);
    iox_write_pio(GPIO_LPADC_D0_OE, 1);
    iox_write_pio(GPIO_LPADC_D0_SEL, 0);
    iox_write_pio(GPIO_XTAL_XI_GND_EN, 0);
    iox_write_pio(GPIO_CTRL_VDDC_ISINK1, 0);
    iox_write_pio(GPIO_CTRL_VDDM_ISINK2, 0);
    iox_write_pio(GPIO_VDDC_ISINK_DISABLE, 0);
    iox_write_pio(GPIO_VDDM_ISINK_DISABLE, 0);
    iox_write_pio(GPIO_ABUF_REF_SEL1, 1);
    iox_write_pio(GPIO_ABUF_REF_SEL0, 1);
    iox_write_pio(GPIO_DAC20_FILT_SEL1, 1);
    iox_write_pio(GPIO_MIPI_SEL, 1);
    iox_write_pio(GPIO_MIPI_CTRL, 1);
    iox_write_pio(GPIO_LPADC_D1_OE, 1);
    iox_write_pio(GPIO_LPADC_D1_SEL, 0);
    iox_write_pio(GPIO_UPPER_IO_CTRL, 0);
    iox_write_pio(GPIO_IOM_PC_SEL, 0);
    iox_write_pio(GPIO_IOM_MEM_SEL, 1);					//Changed
    iox_write_pio(GPIO_IOM_VOLT_SEL, 0);				//Changed
	iox_write_pio(GPIO_IOM1_ANA_SEL, 0);				//Added
    iox_write_pio(GPIO_IOM_ANA_SEL, 1);
    iox_write_pio(GPIO_IOM0_LB_SEL, 1);					//Changed
    iox_write_pio(GPIO_IOS_LB_SEL, 1);					//Changed
    iox_write_pio(GPIO_PC_SEL, 1);						//Changed
    iox_write_pio(GPIO_DAC_PERF_MUX_SEL2, 1);
    iox_write_pio(GPIO_DAC_PERF_MUX_SEL1, 1);
    iox_write_pio(GPIO_DAC_PERF_MUX_SEL0, 1);
    iox_write_pio(GPIO_DAQ_DAC7_SEL0, 1);
    iox_write_pio(GPIO_DAQ_DAC7_SEL1, 1);
    iox_write_pio(GPIO_MSPI_nRST_IOX, 1);
    iox_write_pio(GPIO_AT_UART_SEL, 0);
    iox_write_pio(GPIO_GP19_ATST1_A0_SEL, 0);
    iox_write_pio(GPIO_GP19_ATST1_A1_SEL, 0);
    iox_write_pio(GPIO_GP19_ATST1_VV_RLOAD_SEL, 0);
    iox_write_pio(GPIO_GP19_ATST1_IV_GAIN_SEL, 0);
    iox_write_pio(GPIO_GP19_ATST1_XO_SEL, 0);
    iox_write_pio(GPIO_GP18_ATST2_A0_SEL, 0);
    iox_write_pio(GPIO_GP18_ATST2_A1_SEL, 0);
    iox_write_pio(GPIO_GP18_ATST2_VV_RLOAD_SEL, 0);
    iox_write_pio(GPIO_RAIL_PSU_RSEL_A0, 1);
    iox_write_pio(GPIO_RAIL_PSU_RSEL_A1, 1);
    iox_write_pio(GPIO_V1_PSU_RSEL_A0, 1);
    iox_write_pio(GPIO_V1_PSU_RSEL_A1, 1);
    iox_write_pio(GPIO_MSPI0_SEL3, 0);					//Changed
    iox_write_pio(GPIO_MSPI0_SEL2, 0);					//Changed
    iox_write_pio(GPIO_MSPI0_SEL1, 1);					//Changed
    iox_write_pio(GPIO_MSPI0_SEL0, 0);
    iox_write_pio(GPIO_MSPI2_SEL3, 0);					//Changed
    iox_write_pio(GPIO_MSPI2_SEL2, 0);					//Changed
    iox_write_pio(GPIO_MSPI2_SEL1, 1);					//Changed
    iox_write_pio(GPIO_MSPI2_SEL0, 0);
    iox_write_pio(GPIO_MSPI02_PSRAM_CE_SEL0, 0);
    iox_write_pio(GPIO_MSPI02_PSRAM_CE_SEL1, 0);
//    iox_write_pio(GPIO_IOM1_ANA_SEL, 0);
#if defined BOARD_FALCON_BGA_MCU_CARD_REVB || defined BOARD_FALCON_BGA_MCU_CARD_REVA
    iox_write_pio(GPIO_MSPI1_SEL3, 1);
    iox_write_pio(GPIO_MSPI1_SEL2, 1);
    iox_write_pio(GPIO_MSPI1_SEL1, 0);
    iox_write_pio(GPIO_MSPI1_SEL0, 0);
#endif
#if defined BOARD_FALCON_BGA_MCU_CARD_REVB || defined BOARD_FALCON_SIP_MCU_CARD_REVB
    iox_write_pio(DISP_CLK_SEL, 0);
    iox_write_pio(SWO_ARMADA_EN, 0);
#endif


}


void am_gpio_isr(void)
{
    // Debounce
    am_util_delay_ms(500);

    g_operational_mode += 1;
    g_operational_mode &= 0x7;

    // Clear interrupt
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(GPIO_SW2));

}
