//*****************************************************************************
//
//! @file nemagfx_enhanced_stress_test_config.h
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
 * apollo4p_fpga_ps
 */
#ifdef apollo4p_fpga_ps
#define HPLP_TASK_ENABLE       0
#define XIP_TASK_ENABLE        1
#define CRYPTO_TASK_ENABLE     1
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define MSPI_TASK_ENABLE       0
#define IOMPSRAM_TASK_ENABLE   1
#define ADC_TASK_ENABLE        0
#define EMMC_TASK_ENABLE       0
#define USB_TASK_ENABLE        0

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_HEX_DDR_CE1)
#define PSRAM_APS25616N
#define NOR_FLASH_ADESTO_ATXP032
#define IOM_INSTANCE_TO_PSRAM           {0,1,4,5,6,7}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1) 
/*
 * apollo4b_eb
 */
#elif defined(apollo4b_eb)
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

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1)
#define PSRAM_APS12808L
#define NOR_FLASH_ADESTO_ATXP032
#define IOM_INSTANCE_TO_PSRAM           {0,1,4,5,6,7}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1) 
/*
 * apollo4b_evb_disp_shield
 */
#elif defined(apollo4b_evb_disp_shield)

#define HPLP_TASK_ENABLE       1
#define XIP_TASK_ENABLE        0
#define CRYPTO_TASK_ENABLE     1
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define MSPI_TASK_ENABLE       1
#define IOMPSRAM_TASK_ENABLE   1
#define ADC_TASK_ENABLE        1
#define EMMC_TASK_ENABLE       1
#define USB_TASK_ENABLE        1

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1)
#define PSRAM_APS12808L
#define NOR_FLASH_ADESTO_ATXP032
#define IOM_INSTANCE_TO_PSRAM           {0,1,4,5,6,7}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1) 
/*
 * apollo4p_val
 */
#elif defined(apollo4p_val)

#define HPLP_TASK_ENABLE       0
#define XIP_TASK_ENABLE        1
#define CRYPTO_TASK_ENABLE     1
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define MSPI_TASK_ENABLE       0
#define IOMPSRAM_TASK_ENABLE   0
#define ADC_TASK_ENABLE        1
#define EMMC_TASK_ENABLE       0
#define USB_TASK_ENABLE        1

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_HEX_DDR_CE1)
#define PSRAM_APS25616N
#define NOR_FLASH_ADESTO_ATXP032
#define IOM_INSTANCE_TO_PSRAM           {0,1,4,5,6,7}    
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1) 
/*
 * apollo4p_bga_sys_test
 */
#elif defined(apollo4p_bga_sys_test)

#define HPLP_TASK_ENABLE       1
#define XIP_TASK_ENABLE        0
#define CRYPTO_TASK_ENABLE     1
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#if (DISP_CTRL_IP == DISP_CTRL_IP_DC)
#define MSPI_TASK_ENABLE       1
#elif (DISP_CTRL_IP == DISP_CTRL_IP_MSPI)
#define MSPI_TASK_ENABLE       0
#endif
#define IOMPSRAM_TASK_ENABLE   1
#define ADC_TASK_ENABLE        1
#define EMMC_TASK_ENABLE       0
#define USB_TASK_ENABLE        1

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_HEX_DDR_CE1)
#define PSRAM_APS25616N
#define NOR_FLASH_ADESTO_ATXP032

#define IOM_INSTANCE_TO_PSRAM           {4}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1) 
/*
 * apollo4l_fpga_sys_test
 */
#elif defined(apollo4l_fpga_sys_test)

#define HPLP_TASK_ENABLE       0
#define XIP_TASK_ENABLE        1
#define CRYPTO_TASK_ENABLE     1
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define MSPI_TASK_ENABLE       0
#define IOMPSRAM_TASK_ENABLE   1
#define ADC_TASK_ENABLE        1
#define EMMC_TASK_ENABLE       1
#define USB_TASK_ENABLE        0

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_HEX_DDR_CE1)
#define PSRAM_APS25616N
#define NOR_FLASH_ADESTO_ATXP032
#define IOM_INSTANCE_TO_PSRAM           {4}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1) 
/*
 * apollo4l_bga_sys_test
 */
#elif defined(apollo4l_bga_sys_test)
#define HPLP_TASK_ENABLE       1
#define XIP_TASK_ENABLE        0
#define CRYPTO_TASK_ENABLE     1
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define MSPI_TASK_ENABLE       1
#define IOMPSRAM_TASK_ENABLE   1
#define ADC_TASK_ENABLE        1
#define EMMC_TASK_ENABLE       1
#define USB_TASK_ENABLE        0

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_HEX_DDR_CE1)
#define PSRAM_APS25616N
#define NOR_FLASH_ADESTO_ATXP032
#define IOM_INSTANCE_TO_PSRAM           {4}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1) 
/*
 * apollo4l_eb
 */
#elif defined(apollo4l_eb) 
#define HPLP_TASK_ENABLE        1
#define XIP_TASK_ENABLE         0
#define CRYPTO_TASK_ENABLE      1
#define GUI_TASK_ENABLE         1
#define RENDER_TASK_ENABLE      1
#define DISPLAY_TASK_ENABLE     1
#define MSPI_TASK_ENABLE        1
#define IOMPSRAM_TASK_ENABLE    1
#define ADC_TASK_ENABLE         1
#define EMMC_TASK_ENABLE        1
#define UART_TASK_ENABLE        1
#define USB_TASK_ENABLE         0

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1)
#define PSRAM_APS25616N
#define NAND_FLASH_DOSILICON_DS35X1GA
#define IOM_INSTANCE_TO_PSRAM           {3,4,5,6}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1)

/*
 * apollo4l_blue_eb
 */
#elif defined(apollo4l_blue_eb) 
#define HPLP_TASK_ENABLE        1
#define XIP_TASK_ENABLE         0
#define CRYPTO_TASK_ENABLE      1
#define GUI_TASK_ENABLE         1
#define RENDER_TASK_ENABLE      1
#define DISPLAY_TASK_ENABLE     1
#define MSPI_TASK_ENABLE        0
#define IOMPSRAM_TASK_ENABLE    1
#define ADC_TASK_ENABLE         1
#define EMMC_TASK_ENABLE        0
#define UART_TASK_ENABLE        0
#define USB_TASK_ENABLE         0

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1)
#define PSRAM_APS25616N
#define NAND_FLASH_DOSILICON_DS35X1GA
#define IOM_INSTANCE_TO_PSRAM           {0,1,2,5,6}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1)

/*
 * apollo4l_blue_eb_slt
 */
#elif defined(apollo4l_blue_eb_slt) 
#define HPLP_TASK_ENABLE        1
#define XIP_TASK_ENABLE         0
#define CRYPTO_TASK_ENABLE      1
#define GUI_TASK_ENABLE         1
#define RENDER_TASK_ENABLE      1
#define DISPLAY_TASK_ENABLE     1
#define MSPI_TASK_ENABLE        1
#define IOMPSRAM_TASK_ENABLE    1
#define ADC_TASK_ENABLE         1
#define EMMC_TASK_ENABLE        0
#define UART_TASK_ENABLE        0
#define USB_TASK_ENABLE         0

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1)
#define PSRAM_APS25616N
#define NAND_FLASH_DOSILICON_DS35X1GA
#define IOM_INSTANCE_TO_PSRAM            {0,1,2,6}

#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1)
/*
 * apollo4l_blue_kbr_eb
 */
#elif defined(apollo4l_blue_kbr_eb)
#define HPLP_TASK_ENABLE        1
#define XIP_TASK_ENABLE         0
#define CRYPTO_TASK_ENABLE      1
#define GUI_TASK_ENABLE         1
#define RENDER_TASK_ENABLE      1
#define DISPLAY_TASK_ENABLE     1
#define MSPI_TASK_ENABLE        0
#define IOMPSRAM_TASK_ENABLE    1
#define ADC_TASK_ENABLE         1
#define EMMC_TASK_ENABLE        0
#define UART_TASK_ENABLE        0
#define USB_TASK_ENABLE         0

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1)
#define PSRAM_APS25616N
#define NOR_FLASH_ADESTO_ATXP032
#define IOM_INSTANCE_TO_PSRAM           {3,4,5,6}
#define SDIO_BUS_WIDTH                  (AM_HAL_HOST_BUS_WIDTH_4)
#if (XIP_TASK_ENABLE == 1)
#define XIP_IN_PSRAM
#endif  //(XIP_TASK_ENABLE == 1)
/*
 * default
 */
#else
#define HPLP_TASK_ENABLE       0
#define XIP_TASK_ENABLE        0
#define CRYPTO_TASK_ENABLE     1
#define GUI_TASK_ENABLE        1
#define RENDER_TASK_ENABLE     1
#define DISPLAY_TASK_ENABLE    1
#define MSPI_TASK_ENABLE       0
#define IOMPSRAM_TASK_ENABLE   1
#define ADC_TASK_ENABLE        1
#define EMMC_TASK_ENABLE       1
#define USB_TASK_ENABLE        0

#define PSRAM_DEVICE_CONFIG             (AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1)
#define PSRAM_APS25616N
#define NOR_FLASH_ADESTO_ATXP032
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
#define MSPI_FLASH_MODULE               AM_BSP_MSPI_FLASH_MODULE

#if (MSPI_PSRAM_MODULE == 0)
#define PSRAM_XIP_BASE_ADDRESS           (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 1)
#define PSRAM_XIP_BASE_ADDRESS           (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_PSRAM_MODULE == 2)
#define PSRAM_XIP_BASE_ADDRESS           (MSPI2_APERTURE_START_ADDR)
#endif // #if (MSPI_PSRAM_MODULE == 0)

#if (MSPI_FLASH_MODULE == 0)
#define FLASH_XIP_BASE_ADDRESS          (MSPI0_APERTURE_START_ADDR)
#elif (MSPI_FLASH_MODULE == 1)
#define FLASH_XIP_BASE_ADDRESS          (MSPI1_APERTURE_START_ADDR)
#elif (MSPI_FLASH_MODULE == 2)
#define FLASH_XIP_BASE_ADDRESS          (MSPI2_APERTURE_START_ADDR)
#endif // #if (MSPI_FLASH_MODULE == 0)

#define MSPI_PSRAM_SIZE                 (0x1000000)

//The 0-0x100000 area is used to hold graphics texture
#define PSRAM_TEXTURE_AREA_OFFSET       ( 0x0 )
#define PSRAM_TEXTURE_AREA_SIZE         ( 0x100000 )
//The 0x100000-0x200000 area is used to hold the code.
#define PSRAM_CODE_AREA_OFFSET          ( PSRAM_TEXTURE_AREA_OFFSET + PSRAM_TEXTURE_AREA_SIZE )
#define PSRAM_CODE_AREA_SIZE            ( 0x100000 )
//The 0x200000-0x800000 area is used to hold the heap.
#define PSRAM_HEAP_AREA_OFFSET          ( PSRAM_CODE_AREA_OFFSET + PSRAM_CODE_AREA_SIZE )
#define PSRAM_HEAP_AREA_SIZE            ( 0x600000 )
//The 0x800000-0x900000 area is used to extended ram.
#define PSRAM_EXTENDED_RAM_OFFSET       ( PSRAM_HEAP_AREA_OFFSET + PSRAM_HEAP_AREA_SIZE )
#define PSRAM_EXTENDED_RAM_SIZE         ( 0x100000 )

//*****************************************************************************
//
// Task priority definition.
//
//*****************************************************************************
#define TASK_PRIORITY_LOW               (4)
#define TASK_PRIORITY_MIDDLE            (TASK_PRIORITY_LOW + 1)
#define TASK_PRIORITY_HIGH              (TASK_PRIORITY_MIDDLE + 1)
#define TASK_PRIORITY_HIGHEST           (TASK_PRIORITY_MIDDLE + 2)

//*****************************************************************************
//
// ISR priority definition.
//
//*****************************************************************************
#define ADC_ISR_PRIORITY                (1)
#define AUDADC_ISR_PRIORITY             (2)
#define DISPLAY_ISR_PRIORITY            (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY + 2)
#define PSRAM_ISR_PRIORITY              (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY + 2)
#define FLASH_ISR_PRIORITY              (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY + 1)
#define IOM_ISR_PRIORITY                (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY)
#define UART_ISR_PRIORITY               (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY)
#define USB_ISR_PRIORITY                (NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY)


#endif // NEMAGFX_ENHANCED_STRESS_TEST_CONFIG_H
