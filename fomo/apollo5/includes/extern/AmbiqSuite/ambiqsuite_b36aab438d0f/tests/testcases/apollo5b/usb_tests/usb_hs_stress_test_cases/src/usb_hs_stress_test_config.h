//*****************************************************************************
//
//! @file usb_hs_stress_test_config.h
//!
//! @brief Global includes for the nemagfx_enhanced_stress_test.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef USB_HS_STRESS_TEST_CONFIG_H
#define USB_HS_STRESS_TEST_CONFIG_H

//*****************************************************************************
//
// Task Config.
//
//*****************************************************************************


#define PSRAM_APS25616N
#define NAND_FLASH_DOSILICON_DS35X1GA
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)

// Number of times you want the task the loop before termination. 
#define TASK_LOOP_NUM          900

/*
 * apollo5_eb
 */
#if defined(apollo5_eb) || defined(apollo5a_eb_revb) || defined(apollo5b_eb_revb)
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define USB_TASK_ENABLE        1
#define PDM_TO_I2S_TASK_ENABLE 1

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0)
#define PSRAM_APS25616N
#define AM_BSP_MSPI_FLASH_DEVICE_IS25WX064
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)

/*
 * default
 */
#else

#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define USB_TASK_ENABLE        1
#define PDM_TO_I2S_TASK_ENABLE 1

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0)
#define PSRAM_APS25616N
#define NOR_FLASH_ADESTO_ATXP032
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)

#endif

//*****************************************************************************
//
// PSRAM address.
//
//*****************************************************************************
#define MSPI_PSRAM_MODULE               0
#define MSPI_FLASH_MODULE               1

#if (MSPI_PSRAM_MODULE == 0)
#define PSRAM_XIP_BASE_ADDRESS           (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 1)
#define PSRAM_XIP_BASE_ADDRESS           (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 2)
#define PSRAM_XIP_BASE_ADDRESS           (MSPI2_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 3)
#define PSRAM_XIP_BASE_ADDRESS           (MSPI3_APERTURE_START_ADDR)
#endif // #if (MSPI_PSRAM_MODULE == 0)

#if (MSPI_FLASH_MODULE == 0)
#define FLASH_XIP_BASE_ADDRESS          (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_FLASH_MODULE == 1)
#define FLASH_XIP_BASE_ADDRESS          (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_FLASH_MODULE == 2)
#define FLASH_XIP_BASE_ADDRESS          (MSPI2_APERTURE_START_ADDR)
#elif (MSPI_FLASH_MODULE == 3)
#define FLASH_XIP_BASE_ADDRESS          (MSPI3_APERTURE_START_ADDR)
#endif // #if (MSPI_FLASH_MODULE == 0)

#define MSPI_PSRAM_SIZE                 (0x1000000)

//The 0-0x80_0000 area is used to hold graphics texture
#define PSRAM_TEXTURE_AREA_OFFSET       ( 0x0 )
#define PSRAM_TEXTURE_AREA_SIZE         ( 0x800000 )
//The 0x80_0000 - 0x90_0000 area is used to hold the code.
#define PSRAM_CODE_AREA_OFFSET          ( PSRAM_TEXTURE_AREA_OFFSET + PSRAM_TEXTURE_AREA_SIZE )
#define PSRAM_CODE_AREA_SIZE            ( 0x100000 )
//The 0x90_0000 - 0xF0_0000 area is used to hold the heap.
#define PSRAM_HEAP_AREA_OFFSET          ( PSRAM_CODE_AREA_OFFSET + PSRAM_CODE_AREA_SIZE )
#define PSRAM_HEAP_AREA_SIZE            ( 0x600000 )
//The 0xF0_0000 - 0x100_0000 area is used to extended ram.
#define PSRAM_EXTENDED_RAM_OFFSET       ( PSRAM_HEAP_AREA_OFFSET + PSRAM_HEAP_AREA_SIZE )
#define PSRAM_EXTENDED_RAM_SIZE         ( 0x100000 )

//*****************************************************************************
//
// ISR priority definition.
//
//*****************************************************************************
#define ADC_ISR_PRIORITY                (1)
#define AUDADC_ISR_PRIORITY             (2)
#define DISPLAY_ISR_PRIORITY            (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2)
#define PSRAM_ISR_PRIORITY              (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2)
#define FLASH_ISR_PRIORITY              (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1)
#define IOM_ISR_PRIORITY                (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
#define UART_ISR_PRIORITY               (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
#define USB_ISR_PRIORITY                (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY)
#define PDM_TO_I2S_ISR_PRIORITY         (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2)

//*****************************************************************************
//
// Task priority definition.
//
//*****************************************************************************
#define TASK_PRIORITY_LOW               (1)
#define TASK_PRIORITY_MIDDLE            (TASK_PRIORITY_LOW + 1)
#define TASK_PRIORITY_HIGH              (TASK_PRIORITY_MIDDLE + 1)

#endif // USB_HS_STRESS_TEST_CONFIG_H
