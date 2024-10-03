//*****************************************************************************
//
//! @file mspi_hex_ddr_aps25616n_psram_example.c
//!
//! @brief Example of the MSPI operation with DDR HEX SPI PSRAM APS25616N.
//!
//! Purpose: This example demonstrates MSPI DDR HEX operation using the APS25616N
//! PSRAM device.
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//!
//! AM_DEBUG_PRINTF
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//!
//! Note that when these macros are defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.
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
#include "factory_test_helpers.h"

extern int mspi0_hex_ddr_aps256_psram_test(uint32_t);
//int mspi1_qspi_test(void);
extern int mspi2_quad_nand_flash_test(uint32_t );
extern int emmc_test(uint32_t);
extern int iom_psram_test(uint32_t);
extern int iom6_fram_test(uint32_t);
extern int iom_ios_loopback_test(uint32_t);
extern int qspi_display_test(uint32_t);

struct factory_test
{
  uint32_t id;
  char *name;
  char *description;
  int (*handler)(uint32_t );
  uint32_t mode;
} ;


struct factory_test test_list[] =
{
  {1,  "MSPI0 Group" ,          "MSPI0 Group x16 PSRAM",        mspi0_hex_ddr_aps256_psram_test,        NULL},
  {3,  "MSPI2 Group" ,          "MSPI2 Quad NAND Flash",        mspi2_quad_nand_flash_test,             NULL},
  {4,  "SDIO Group",            "SDIO eMMC",                    emmc_test,                              NULL},
  {5,  "IOM0&IOMS Group",       "IOMS LoopBack ",               iom_ios_loopback_test,                  NULL},
  {6,  "IOM1 Group",            "IOM1 PSRAM",                   iom_psram_test,                         1},
  {7,  "IOM3 Group",            "IOM3 PSRAM",                   iom_psram_test,                         3},
  {8,  "IOM4 Group",            "IOM4 PSRAM",                   iom_psram_test,                         4},
  {9,  "IOM5 Group",            "IOM5 PSRAM",                   iom_psram_test,                         5},
  {10, "IOM6 Group",            "IOM6 FRAM",                    iom6_fram_test,                         NULL},
  {2,  "MSPI1&iom2 Group" ,     "QSPI DISP&DISP TP_IF",         qspi_display_test,                      NULL},
};
struct UNITY_STORAGE_T Unity;
//*****************************************************************************
//
// MSPI Example Main.
//
//*****************************************************************************
int
main(void)
{
    uint32_t index;
    am_util_id_t sIdDevice;
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
    // Wait for Apollo4L EB MUX configuration to complete.
    //
    am_util_delay_ms(1000);
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Print the device info.
    //
    am_util_id_device(&sIdDevice);
    am_util_stdio_printf("Vendor Name: %s\n", sIdDevice.pui8VendorName);
    am_util_stdio_printf("Device type: %s\n",
         sIdDevice.pui8DeviceName);

    am_util_stdio_printf("Device Info:\n"
                         "\tPart number: 0x%08X\n"
                         "\tChip ID0:    0x%08X\n"
                         "\tChip ID1:    0x%08X\n"
                         "\tRevision:    0x%08X (Rev%c%c)\n",
                         sIdDevice.sMcuCtrlDevice.ui32ChipPN,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID0,
                         sIdDevice.sMcuCtrlDevice.ui32ChipID1,
                         sIdDevice.sMcuCtrlDevice.ui32ChipRev,
                         sIdDevice.ui8ChipRevMaj, sIdDevice.ui8ChipRevMin );

    Unity.NumberOfTests = 0;
    Unity.TestFailures = 0;
#if 1
    uint32_t IntNum = 77;

    am_hal_gpio_pinconfig(IntNum, am_hal_gpio_pincfg_output);
    am_hal_gpio_state_write(IntNum, AM_HAL_GPIO_OUTPUT_SET);
#endif
    //
    // Loop forever while sleeping.
    //
    while (1)
    {
        am_util_stdio_printf("[==========] Apollo4L EB factory test started\r\n");
        for ( index = 0 ; index < sizeof(test_list) / sizeof(test_list[0]) ; index++ )
        {
            Unity.NumberOfTests++;
            am_util_stdio_printf("[----------] #%d.[ %s ] (%s) started\r\n", test_list[index].id, test_list[index].name , test_list[index].description);
            if (test_list[index].handler(test_list[index].mode) == 0)
            {
              am_util_stdio_printf("[  PASSED  ] #%d.[ %s ] (%s)\r\n", test_list[index].id, test_list[index].name , test_list[index].description);

            }
            else
            {
              Unity.TestFailures++;
              am_util_stdio_printf("[  FAILED  ] #%d.[ %s ] (%s)\r\n", test_list[index].id, test_list[index].name , test_list[index].description);
            }

        }
        am_util_stdio_printf("[==========] Apollo4L EB factory test finished\r\n");
        UnityEnd(&Unity);
        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        while(1);
    }
}

