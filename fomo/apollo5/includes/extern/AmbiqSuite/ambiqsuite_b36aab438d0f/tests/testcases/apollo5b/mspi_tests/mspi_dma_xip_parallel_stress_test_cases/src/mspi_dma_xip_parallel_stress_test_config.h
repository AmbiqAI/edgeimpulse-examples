//*****************************************************************************
//
//! @file mspi_dma_xip_parallel_stress_test_config.h
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

#ifndef NEMAGFX_ENHANCED_STRESS_TEST_CONFIG_H
#define NEMAGFX_ENHANCED_STRESS_TEST_CONFIG_H

//*****************************************************************************
//
// Task Config.
//
//*****************************************************************************

/*
 * apollo5b_fpga_turbo
 */
#if defined(apollo5b_fpga_turbo)
#define HPLP_TASK_ENABLE       0
#define XIP_TASK_ENABLE        1
#define CRYPTO_TASK_ENABLE     0
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define MSPI_TASK_ENABLE       0
#define IOMPSRAM_TASK_ENABLE   0
#define ADC_TASK_ENABLE        0
#define EMMC_TASK_ENABLE       0
#define USB_TASK_ENABLE        0
#define UART_TASK_ENABLE       0

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0)

#if defined (APS25616BA)
#define PSRAM_APS25616BA_1P2V
#elif
#define PSRAM_APS25616N
#endif

#define NOR_FLASH_ISSI_IS25WX064
#define IOM_INSTANCE_TO_PSRAM           {0,1,2,3,4,5,6,7}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
//#define XIP_IN_FLASH
#endif //(XIP_TASK_ENABLE == 1)

#elif defined(apollo5b_eb_revb)
#define HPLP_TASK_ENABLE       1
#define XIP_TASK_ENABLE        1
#define CRYPTO_TASK_ENABLE     1
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define MSPI_TASK_ENABLE       0
#define IOMPSRAM_TASK_ENABLE   1
#define ADC_TASK_ENABLE        0
#define EMMC_TASK_ENABLE       1
#define USB_TASK_ENABLE        1
#define UART_TASK_ENABLE       0

//
// Enable Mspi Scrambling In Gui and XIP Task
//
// #define MPSI_GUI_SCRAMBLING_ENABLE
#ifdef  MPSI_GUI_SCRAMBLING_ENABLE
//
// Add Scrambling Test Task When Running GUI Task
//
#define MSPI_SCRAMBLING_TASK_ENABLE 1

//
// Enable Mspi Scrambling In XIP task 
// And Running XIPMM test (only word access)
//
#define MSPI_XIP_TASK_SCRAMBLING_ENABLE
#endif


#if defined(MSPI_DMA_XIP_PARALLEL_TEST) || defined(MPSI_GUI_SCRAMBLING_ENABLE)
#define MSPI_XIPMM_TASK_ENABLE 0
#else
#define MSPI_XIPMM_TASK_ENABLE 1
#endif

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0)

#if defined (APS25616BA)
#define PSRAM_APS25616BA_1P2V
#else
#define PSRAM_APS25616N
#endif

#define NOR_FLASH_ISSI_IS25WX064
#define IOM_INSTANCE_TO_PSRAM           {2,3,4,7}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_8)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
//#define XIP_IN_FLASH
#endif //(XIP_TASK_ENABLE == 1)

/*
 * default
 */
#else
#define HPLP_TASK_ENABLE       0
#define XIP_TASK_ENABLE        0
#define CRYPTO_TASK_ENABLE     0
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define MSPI_TASK_ENABLE       0
#define IOMPSRAM_TASK_ENABLE   0
#define ADC_TASK_ENABLE        0
#define EMMC_TASK_ENABLE       0
#define USB_TASK_ENABLE        0
#define UART_TASK_ENABLE       0

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0)
#define PSRAM_APS25616N
#define NOR_FLASH_ISSI_IS25WX064
#define IOM_INSTANCE_TO_PSRAM           {0,1,2,3,4,5,6,7}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
//#define XIP_IN_FLASH
#endif //(XIP_TASK_ENABLE == 1)

#endif

//*****************************************************************************
//
// PSRAM address.
//
//*****************************************************************************
#define MSPI_PSRAM_MODULE               0
#if defined(APOLLO5_FPGA)
#define MSPI_PSRAM_XIPCODE_MODULE       0
#else
#define MSPI_PSRAM_XIPCODE_MODULE       3
#endif

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

#if defined(APOLLO5_FPGA) || defined(MPSI_GUI_SCRAMBLING_ENABLE)
//The DMA operation in psram address
#define PSRAM_DMA_ADDR_OFFSET           ( PSRAM_EXTENDED_RAM_OFFSET + PSRAM_EXTENDED_RAM_SIZE )
//Alloc buffer in PSRAM
#define ENABLE_ALLOC_BUFFER_IN_PSRAM
#else
//The DMA operation in psram address
#define PSRAM_DMA_ADDR_OFFSET           0
#endif

//The XIPMM operation in psram address
#define PSRAM_XIPMM_ADDR_OFFSET         ( PSRAM_DMA_ADDR_OFFSET + 0x80000 )
//The frame buffer address
#define PSRAM_FREAM_BUFFER_ADDR         ( PSRAM_XIP_BASE_ADDRESS + PSRAM_XIPMM_ADDR_OFFSET + 0x11000 )

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

//*****************************************************************************
//
// Task priority definition.
//
//*****************************************************************************
#define TASK_PRIORITY_LOW               (1)
#define TASK_PRIORITY_MIDDLE            (TASK_PRIORITY_LOW + 1)
#define TASK_PRIORITY_HIGH              (TASK_PRIORITY_MIDDLE + 1)

#endif // NEMAGFX_ENHANCED_STRESS_TEST_CONFIG_H
