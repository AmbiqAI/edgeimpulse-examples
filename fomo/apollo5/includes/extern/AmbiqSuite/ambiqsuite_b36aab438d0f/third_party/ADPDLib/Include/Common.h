/*
Copyright (c) 2015 Analog Devices, Inc.

All rights reserved.

*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdint.h>

//#define DEBUG
/* --------------------------- definitions -------------------------------- */
#ifdef DEBUG
#include <stdio.h>
//extern void adi_printf (uint8_t*, ...);
extern uint32_t am_util_stdio_printf(char *pui8Fmt, ...);
extern void am_util_plot_byte(uint32_t ui32Trace, uint8_t ui8Value);
#define BUF_SIZE (1024)
extern char _SBZ[BUF_SIZE];
#define debug(M, ...)  {_SBZ[0] = 0; \
                        snprintf(_SBZ, BUF_SIZE, "" M "", ##__VA_ARGS__); \
                        am_util_stdio_printf("%s", _SBZ);}
#define debug_plot(M)  {am_util_plot_byte(AM_UTIL_PLOT_0, M);}
#else
#define debug(M, ...)
#endif
/* ------------------------- Global Variable ------------------------------ */

typedef enum
{
  ADI_HAL_OK       = 0x00,
  ADI_HAL_ERROR    = 0x01,
  ADI_HAL_BUSY     = 0x02,
  ADI_HAL_TIMEOUT  = 0x03
} ADI_HAL_STATUS_t;

#define ADXLDrv_SUCCESS (0)
#define ADXLDrv_ERROR (-1)

#define BMIDrv_SUCCESS (0)
#define BMIDrv_ERROR (-1)

# ifdef ADXL_ACCEL
extern ADI_HAL_STATUS_t ADXL362_SPI_Transmit(uint8_t *pData, uint16_t Size);
extern ADI_HAL_STATUS_t ADXL362_SPI_Receive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);
void GPIO_IRQ_ADXL362_Enable(void);
void GPIO_IRQ_ADXL362_Disable(void);
extern void AdxlISR(uint16_t pin);
#endif
#ifdef BMI_ACCEL
extern ADI_HAL_STATUS_t BMI160_SPI_Transmit(uint8_t *pData, uint16_t Size);
extern ADI_HAL_STATUS_t BMI160_SPI_Receive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);
extern ADI_HAL_STATUS_t BMI160_SPI_Receive_Burst(uint8_t *pTxData, uint8_t *pRxData, uint32_t Size);
extern int8_t BMI160_SPI_Transmit_Driv(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);
extern int8_t BMI160_SPI_Receive_Driv(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint8_t cnt);
extern int8_t BMI160_SPI_Receive_Burst_Driv(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint32_t cnt);
void GPIO_IRQ_BMI160_Enable(void);
void GPIO_IRQ_BMI160_Disable(void);
extern void BmiISR(uint16_t pin);
#endif
extern void MCU_HAL_Delay(uint32_t delay);
extern uint32_t MCU_HAL_GetTick(void);
void GPIO_DUT_POWER_On(void);
void GPIO_DUT_POWER_Off(void);
void  GPIO_IRQ_ADPD_Disable(void);
void  GPIO_IRQ_ADPD_Enable(void);
extern ADI_HAL_STATUS_t MCU_HAL_I2C_TxRx(uint8_t *pTxData, uint8_t *pRxData, uint16_t RxSize, uint32_t Timeout);
extern ADI_HAL_STATUS_t MCU_HAL_I2C_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout);

// Ambiq additions
extern void AmbiqConfig(void);
extern uint32_t HAL_RCC_GetHCLKFreq(void);
#endif
