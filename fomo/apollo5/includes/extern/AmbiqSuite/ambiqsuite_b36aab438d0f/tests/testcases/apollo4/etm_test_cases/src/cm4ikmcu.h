/*
 *-----------------------------------------------------------------------------
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from ARM Limited.
 *
 *            (C) COPYRIGHT 2009-2010 ARM Limited.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from ARM Limited.
 *
 *      SVN Information
 *
 *
 *      Revision            : $Revision: 142008 $
 *
 *      Release information : cortexm4_r0p1_00rel0
 *-----------------------------------------------------------------------------
 */

#ifndef __CM4IKMCU_H__
#define __CM4IKMCU_H__

/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */

typedef enum IRQn
{
/******  Cortex-M4 Processor Exceptions Numbers ***************************************************/
  NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                             */
  HardFault_IRQn              = -13,    /*!< 3 Cortex-M4 Hard Fault Interrupt                     */
  SVCall_IRQn                 = -5,     /*!< 11 Cortex-M4 SV Call Interrupt                       */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M4 Pend SV Interrupt                       */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M4 System Tick Interrupt                   */

/******  CM4IKMCU Cortex-M4 specific Interrupt Numbers ********************************************/
  GPIO_IRQn                   = 0,      /*!< GPIO Interrupt                                       */
                                        /*!< maximum of 32 Interrupts are possible                */
  CM4IKMCU_IRQ01_IRQn         = 1,
  CM4IKMCU_IRQ02_IRQn         = 2,
  CM4IKMCU_IRQ03_IRQn         = 3,
  CM4IKMCU_IRQ04_IRQn         = 4,
  CM4IKMCU_IRQ05_IRQn         = 5,
  CM4IKMCU_IRQ06_IRQn         = 6,
  CM4IKMCU_IRQ07_IRQn         = 7,
  CM4IKMCU_IRQ08_IRQn         = 8,
  CM4IKMCU_IRQ09_IRQn         = 9,
  CM4IKMCU_IRQ10_IRQn         = 10,
  CM4IKMCU_IRQ11_IRQn         = 11,
  CM4IKMCU_IRQ12_IRQn         = 12,
  CM4IKMCU_IRQ13_IRQn         = 13,
  CM4IKMCU_IRQ14_IRQn         = 14,
  CM4IKMCU_IRQ15_IRQn         = 15,
  CM4IKMCU_IRQ16_IRQn         = 16,
  CM4IKMCU_IRQ17_IRQn         = 17,
  CM4IKMCU_IRQ18_IRQn         = 18,
  CM4IKMCU_IRQ19_IRQn         = 19,
  CM4IKMCU_IRQ20_IRQn         = 20,
  CM4IKMCU_IRQ21_IRQn         = 21,
  CM4IKMCU_IRQ22_IRQn         = 22,
  CM4IKMCU_IRQ23_IRQn         = 23,
  CM4IKMCU_IRQ24_IRQn         = 24,
  CM4IKMCU_IRQ25_IRQn         = 25,
  CM4IKMCU_IRQ26_IRQn         = 26,
  CM4IKMCU_IRQ27_IRQn         = 27,
  CM4IKMCU_IRQ28_IRQn         = 28,
  CM4IKMCU_IRQ29_IRQn         = 29,
  CM4IKMCU_IRQ30_IRQn         = 30,
  CM4IKMCU_IRQ31_IRQn         = 31
} IRQn_Type;


/*
 * ==========================================================================
 * ----------- Processor and Core Peripheral Section ------------------------
 * ==========================================================================
 */

/* Configuration of the Cortex-M4 Processor and Core Peripherals */
#define __CM4_REV                 0x0100U       /*!< CM4 Core Revision                                                         */
#define __NVIC_PRIO_BITS               3        /*!< Number of Bits used for Priority Levels                                   */
#define __Vendor_SysTickConfig         0        /*!< Set to 1 if different SysTick Config is used                              */
#define __MPU_PRESENT                  1        /*!< MPU present                                                               */
#define __FPU_PRESENT                  1        /*!< FPU present                                                               */
#define __Vendor_SysTickConfig  0       /*!< Set to 1 if different SysTick Config is used         */


#include "core_cm4.h"                   /* Cortex-M4 processor and core peripherals               */
#include "system_cm4ikmcu.h"            /* cm4ikmcu System                                        */



/**
 * Initialize the system clock
 *
 * @param  none
 * @return none
 *
 * @brief  Setup the microcontroller system
 *         Initialize the PLL and update the SystemFrequency variable
 */
extern void SystemInit (void);



/******************************************************************************/
/*                Device Specific Peripheral registers structures             */
/******************************************************************************/

/*--------------------- General Purpose Input and Ouptut ---------------------*/
typedef union
{
  __IO uint32_t WORD;
  __IO uint16_t HALFWORD[2];
  __IO uint8_t  BYTE[4];
} GPIO_Data_TypeDef;

typedef struct
{
  GPIO_Data_TypeDef DATA [256];
  GPIO_Data_TypeDef DIR;
  uint32_t RESERVED[3];
  GPIO_Data_TypeDef IE;
} GPIO_TypeDef;


/******************************************************************************/
/*                         Peripheral memory map                              */
/******************************************************************************/
/* Peripheral and SRAM base address */
#define SRAM_BASE             ((     uint32_t)0x20000000)
#define PERIPH_BASE           ((     uint32_t)0x40000000)

/* Peripheral memory map */
#define GPIO_BASE                 PERIPH_BASE

#define GPIO0_BASE                (GPIO_BASE)
#define GPIO1_BASE                (GPIO_BASE       + 0x0800)
#define GPIO2_BASE                (GPIO_BASE       + 0x1000)


/******************************************************************************/
/*                         Peripheral declaration                             */
/******************************************************************************/
#define GPIO0                     ((GPIO_TypeDef *) GPIO0_BASE)
#define GPIO1                     ((GPIO_TypeDef *) GPIO1_BASE)
#define GPIO2                     ((GPIO_TypeDef *) GPIO2_BASE)


#endif



