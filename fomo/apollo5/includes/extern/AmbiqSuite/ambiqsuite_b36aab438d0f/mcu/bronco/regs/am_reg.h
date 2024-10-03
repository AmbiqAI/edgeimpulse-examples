//*****************************************************************************
//
//! @file am_reg.h
//!
//! @brief Bronco register macros
//!
//! @addtogroup reg_macros_5 Bronco Register Macros
//! @ingroup bronco_hal
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_REG_H
#define AM_REG_H

//*****************************************************************************
//
//! @brief APBDMA
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_APBDMA_NUM_MODULES                    1
#define AM_REG_APBDMAn(n) \
    (REG_APBDMA_BASEADDR + 0x00001000 * n)

//*****************************************************************************
//
//! @brief CLKGEN
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_CLKGEN_NUM_MODULES                    1
#define AM_REG_CLKGENn(n) \
    (REG_CLKGEN_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief CRYPTO
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_CRYPTO_NUM_MODULES                    1
#define AM_REG_CRYPTOn(n) \
    (REG_CRYPTO_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief RSTGEN
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_RSTGEN_NUM_MODULES                    1
#define AM_REG_RSTGENn(n) \
    (REG_RSTGEN_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief RTC
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_RTC_NUM_MODULES                       1
#define AM_REG_RTCn(n) \
    (REG_RTC_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief SECURITY
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_SECURITY_NUM_MODULES                  1
#define AM_REG_SECURITYn(n) \
    (REG_SECURITY_BASEADDR + 0x00001000 * n)

//*****************************************************************************
//
//! @brief WDT
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_WDT_NUM_MODULES                       1
#define AM_REG_WDTn(n) \
    (REG_WDT_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief I2S
//! Instance finder. (2 instance(s) available)
//
//*****************************************************************************
#define AM_REG_I2S_NUM_MODULES                       2
#define AM_REG_I2Sn(n) \
    (REG_I2S_BASEADDR + 0x00001000 * n)

//*****************************************************************************
//
//! @brief PDM
//! Instance finder. (4 instance(s) available)
//
//*****************************************************************************
#define AM_REG_PDM_NUM_MODULES                       4
#define AM_REG_PDMn(n) \
    (REG_PDM_BASEADDR + 0x00001000 * n)

//*****************************************************************************
//
//! @brief ADC
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_ADC_NUM_MODULES                       1
#define AM_REG_ADCn(n) \
    (REG_ADC_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief AUDADC
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_AUDADC_NUM_MODULES                    1
#define AM_REG_AUDADCn(n) \
    (REG_AUDADC_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief CPU
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_CPU_NUM_MODULES                       1
#define AM_REG_CPUn(n) \
    (REG_CPU_BASEADDR + 0x00001000 * n)

//*****************************************************************************
//
//! @brief DC
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_DC_NUM_MODULES                        1
#define AM_REG_DCn(n) \
    (REG_DC_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief DSI
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_DSI_NUM_MODULES                       1
#define AM_REG_DSIn(n) \
    (REG_DSI_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief FPIO
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_FPIO_NUM_MODULES                      1
#define AM_REG_FPIOn(n) \
    (REG_FPIO_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief GPIO
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_GPIO_NUM_MODULES                      1
#define AM_REG_GPIOn(n) \
    (REG_GPIO_BASEADDR + 0x00000004 * n)

//*****************************************************************************
//
//! @brief GPU
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_GPU_NUM_MODULES                       1
#define AM_REG_GPUn(n) \
    (REG_GPU_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief IOM
//! Instance finder. (8 instance(s) available)
//
//*****************************************************************************
#define AM_REG_IOM_NUM_MODULES                       8
#define AM_REG_IOMn(n) \
    (REG_IOM_BASEADDR + 0x00001000 * n)

//*****************************************************************************
//
//! @brief IOSLAVE
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_IOSLAVE_NUM_MODULES                   1
#define AM_REG_IOSLAVEn(n) \
    (REG_IOSLAVE_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief MCUCTRL
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_MCUCTRL_NUM_MODULES                   1
#define AM_REG_MCUCTRLn(n) \
    (REG_MCUCTRL_BASEADDR + 0x00000000 * n)

// ##### INTERNAL BEGIN #####
//*****************************************************************************
//
//! @brief MCUCTRLPRIV
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_MCUCTRLPRIV_NUM_MODULES               1
#define AM_REG_MCUCTRLPRIVn(n) \
    (REG_MCUCTRLPRIV_BASEADDR + 0x00000000 * n)
// ##### INTERNAL END #####
//*****************************************************************************
//
//! @brief MSPI
//! Instance finder. (4 instance(s) available)
//
//*****************************************************************************
#define AM_REG_MSPI_NUM_MODULES                      4
#define AM_REG_MSPIn(n) \
    (REG_MSPI_BASEADDR + 0x00001000 * n)

//*****************************************************************************
//
//! @brief PWRCTRL
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_PWRCTRL_NUM_MODULES                   1
#define AM_REG_PWRCTRLn(n) \
    (REG_PWRCTRL_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief SDIO
//! Instance finder. (2 instance(s) available)
//
//*****************************************************************************
#define AM_REG_SDIO_NUM_MODULES                      2
#define AM_REG_SDIOn(n) \
    (REG_SDIO_BASEADDR + 0x00001000 * n)

//*****************************************************************************
//
//! @brief STIMER
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_STIMER_NUM_MODULES                    1
#define AM_REG_STIMERn(n) \
    (REG_STIMER_BASEADDR + 0x00000000 * n)

//*****************************************************************************
//
//! @brief TIMER
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_TIMER_NUM_MODULES                     1
#define AM_REG_TIMERn(n) \
    (REG_TIMER_BASEADDR + 0x00000020 * n)

//*****************************************************************************
//
//! @brief UART
//! Instance finder. (4 instance(s) available)
//
//*****************************************************************************
#define AM_REG_UART_NUM_MODULES                      4
#define AM_REG_UARTn(n) \
    (REG_UART_BASEADDR + 0x00001000 * n)

//*****************************************************************************
//
//! @brief USB
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_USB_NUM_MODULES                       1
#define AM_REG_USBn(n) \
    (REG_USB_BASEADDR + 0x00000004 * n)

//*****************************************************************************
//
//! @brief USBPHY
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_USBPHY_NUM_MODULES                    1
#define AM_REG_USBPHYn(n) \
    (REG_USBPHY_BASEADDR + 0x00000004 * n)

//*****************************************************************************
//
//! @brief VCOMP
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_VCOMP_NUM_MODULES                     1
#define AM_REG_VCOMPn(n) \
    (REG_VCOMP_BASEADDR + 0x00000000 * n)

#endif // AM_REG_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
