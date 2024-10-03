/**
    ******************************************************************************
    * @file     ADPDDrv.h
    * @author   ADI
    * @version  V1.0.0
    * @date     14-May-2014
    * @brief    Include file for the ADPD device driver
    ******************************************************************************
    * @attention
    ******************************************************************************
    */
/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2015 Analog Devices Inc.                                      *
* All rights reserved.                                                        *
*                                                                             *
* This source code is intended for the recipient only under the guidelines of *
* the non-disclosure agreement with Analog Devices Inc.                       *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
*                                                                             *
* This software is intended for use with the ADPD and derivative parts     *
* only                                                                        *
*                                                                             *
******************************************************************************/

#ifndef __ADPDDrv__H
#define __ADPDDrv__H

#include <stdint.h>

/* Exported types ---------------------------------------------------------- */

typedef enum {
    ADPDDrv_MODE_IDLE = 0,
    ADPDDrv_MODE_PAUSE,
    ADPDDrv_MODE_PWR_OFF,
    ADPDDrv_MODE_SAMPLE
} ADPDDrv_OPERATION_MODE;

typedef enum {
    ADPDDrv_SLOT_A = 0,
    ADPDDrv_SLOT_B,
    ADPDDrv_SLOT_AB
} ADPDDrv_OPERATION_SLOT;

/* Exported constants ------------------------------------------------------ */

/*  ADPD REGISTERS */
#define REG_INT_STATUS              0x00
#define REG_INT_MASK                0x01
#define REG_PAD_IO_CTRL             0x02
#define REG_I2CS_CTL_MATCH          0x06
#define REG_CHIP_ID                 0x08
#define REG_SW_RESET		    0x0F
#define REG_OP_MODE                 0x10
#define REG_OP_MODE_CFG             0x11
#define REG_SAMPLING_FREQ           0x12
#define REG_PD_SELECT               0x14
#define REG_DEC_MODE                0x15
#define REG_CH1_OFFSET_A            0x18
#define REG_CH2_OFFSET_A            0x19
#define REG_CH3_OFFSET_A            0x1A
#define REG_CH4_OFFSET_A            0x1B
#define REG_CH1_OFFSET_B            0x1E
#define REG_CH2_OFFSET_B            0x1F
#define REG_CH3_OFFSET_B            0x20
#define REG_CH4_OFFSET_B            0x21
#define REG_LED1_DRV                0x23
#define REG_LED2_DRV                0x24
#define REG_LED_TRIM                0x25
#define REG_PULSE_OFFSET_A          0x30
#define REG_PULSE_PERIOD_A          0x31
#define REG_PULSE_MASK              0x34
#define REG_PULSE_OFFSET_B          0x35
#define REG_PULSE_PERIOD_B          0x36
#define REG_AFE_CTRL_A              0x39
#define REG_AFE_CTRL_B              0x3B
#define REG_AFE_TRIM_A              0x42
#define REG_AFE_TEST_A              0x43
#define REG_AFE_TRIM_B              0x44
#define REG_AFE_TEST_B              0x45
#define REG_OSC32K                  0x4B
#define REG_TEST_PD                 0x52
#define REG_EFUSE_CTRL              0x57
#define REG_FIFO_CLK                0x5f
#define REG_DATA_BUFFER             0x60

#define REG_EFUSE_STATUS0           0x67

#define REG_X1_A                    0x64
#define REG_X2_A                    0x65
#define REG_Y1_A                    0x66
#define REG_Y2_A                    0x67
#define REG_X1_B                    0x68
#define REG_X2_B                    0x69
#define REG_Y1_B                    0x6A
#define REG_Y2_B                    0x6B
#define REG_READ_X1L_A              0x70
#define REG_READ_X2L_A              0x71
#define REG_READ_Y1L_A              0x72
#define REG_READ_Y2L_A              0x73
#define REG_READ_X1H_A              0x74
#define REG_READ_X2H_A              0x75
#define REG_READ_Y1H_A              0x76
#define REG_READ_Y2H_A              0x77
#define REG_READ_X1L_B              0x78
#define REG_READ_X2L_B              0x79
#define REG_READ_Y1L_B              0x7A
#define REG_READ_Y2L_B              0x7B
#define REG_READ_X1H_B              0x7C
#define REG_READ_X2H_B              0x7D
#define REG_READ_Y1H_B              0x7E
#define REG_READ_Y2H_B              0x7F

/*  REGISTER Values */
#define OP_IDLE_MODE            0
#define OP_PAUSE_MODE           1
#define OP_RUN_MODE             2

#define FIFO_CLK_EN             0x0001
#define FIFO_CLK_DIS            0x0000

// IRQ related
#define FIFO_CLR                0x8000
#define IRQ_CLR_ALL             0x00FF
#define IRQ_CLR_FIFO            0x8000
#define IRQ_MASK_RAW            0x0060
#define IRQ_RAW_AB_EN           0xC19F
#define IRQ_RAW_A_EN            0xC1DF
#define IRQ_RAW_B_EN            0xC1BF

// Slot related
#define SLOT_A_DATA_SIZE        0x8
#define SLOT_B_DATA_SIZE        0x8
#define SLOT_AB_DATA_SIZE       0x10
#define SLOT_MASK               0xF800
#define SLOT_A_MODE             0x0011
#define SLOT_B_MODE             0x0120
#define SLOT_AB_MODE            0x0131

#define ADPDDrv_SUCCESS (0)
#define ADPDDrv_ERROR (-1)

/* Exported global variable ------------------------------------------------ */

//extern I2C_HandleTypeDef I2cHandle;

/* Exported functions ------------------------------------------------------ */
extern int16_t ADPDDrv_OpenDriver(void);
extern int16_t ADPDDrv_CloseDriver(void);
extern void AdpdISR(uint16_t);
extern int16_t ADPDDrv_RegRead(uint16_t addr, uint16_t *data);
extern int16_t ADPDDrv_RegWrite(uint16_t addr, uint16_t data);
extern int16_t ADPDDrv_ReadData(uint8_t, uint16_t *data, uint32_t *time);
extern int16_t ADPDDrv_SetOperationMode(uint8_t opmode);
extern int16_t ADPDDrv_SelectSlot(uint8_t slot);
extern int16_t ADPDDrv_EfuseRead(uint16_t *reg_values, uint8_t size);
extern uint32_t ADPDDrv_GetRefTime(void);
extern uint8_t* ADPDDrv_EfuseModuleNameRead(void);
extern int16_t ADPDDrv_SoftReset(void);
extern int16_t ADPDDrv_EfuseModuleTypeRead(uint16_t *moduletype);

/* Macros ------------------------------------------------ */
#ifdef LOW_RATE_DRIVER
#define LOW_RATE_DRIVER_ENABLE(void)    if (state == ADPDLIB_STAGE_HEART_RATE) \
                                    ADPDDrv_SetIsrReadDataFromFIFO(1)
#define LOW_RATE_DRIVER_DISABLE(void)   ADPDDrv_SetIsrReadDataFromFIFO(0)
#define GPIO_IRQ_ADPD_ENABLE(void)      GPIO_IRQ_ADPD_Enable(void)
#define GPIO_IRQ_ADPD_DISABLE(void)     GPIO_IRQ_ADPD_Disable(void)
#else
#define LOW_RATE_DRIVER_ENABLE(void)
#define LOW_RATE_DRIVER_DISABLE(void)
#define GPIO_IRQ_ADPD_ENABLE(void)
#define GPIO_IRQ_ADPD_DISABLE(void)
#endif
#endif
