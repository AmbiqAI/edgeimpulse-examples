/*
Copyright (c) 2015 Analog Devices, Inc.

All rights reserved.

*/

#ifndef _COMMON_H_
#define _COMMON_H_


#define DEBUG
/* --------------------------- definitions -------------------------------- */
#ifdef DEBUG
#include <stdio.h>
extern void adi_printf (uint8_t*, ...);
#define BUF_SIZE (128)
extern char _SBZ[BUF_SIZE];
#define debug(M, ...)  {_SBZ[0] = 0; \
                        snprintf(_SBZ, BUF_SIZE, "" M "", ##__VA_ARGS__); \
                        adi_printf("%s", _SBZ);}
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

extern ADI_HAL_STATUS_t ADXL362_SPI_Transmit(uint8_t *pData, uint16_t Size);
extern ADI_HAL_STATUS_t ADXL362_SPI_Receive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);
extern void MCU_HAL_Delay(uint32_t delay);
extern uint32_t MCU_HAL_GetTick();
void GPIO_IRQ_ADXL362_Enable(void);
void GPIO_IRQ_ADXL362_Disable(void);
void GPIO_DUT_POWER_On(void);
void GPIO_DUT_POWER_Off(void);
void  GPIO_IRQ_ADPD_Disable(void);
void  GPIO_IRQ_ADPD_Enable(void);
ADI_HAL_STATUS_t MCU_HAL_I2C_TxRx(uint8_t *pTxData, uint8_t *pRxData, uint16_t RxSize, uint32_t Timeout);
ADI_HAL_STATUS_t MCU_HAL_I2C_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout);

#endif
