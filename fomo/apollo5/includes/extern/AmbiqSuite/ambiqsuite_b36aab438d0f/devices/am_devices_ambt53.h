//*****************************************************************************
//
//! @file am_devices_ambt53.h
//!
//! @brief The implementation of Apollo interface to AMBT53, including the general
//!        MSPI driver, and System Controller and Power Management (SCPM) module
//!        I2C driver. Also includes the AMBT53 firmware loading and booting functions.
//!
//! @addtogroup ambt53 AMBT53 Device Driver
//! @ingroup devices
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

#ifndef AM_DEVICES_AMBT53_H
#define AM_DEVICES_AMBT53_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_util.h"
#include "am_mcu_apollo.h"
#include "am_devices_ambt53_gpio.h"
#include "am_devices_ambt53_load_fw.h"
#include "am_devices_ambt53_mailbox.h"
#include "am_devices_ambt53_mspi.h"
#include "am_devices_ambt53_otp.h"
#include "am_devices_ambt53_scpm.h"

//*****************************************************************************
//
// Macro definitions.
//
//*****************************************************************************
#define BLUEBUD_RTL_1_0_1 101
#define BLUEBUD_RTL_1_1_1 111
#define BLUEBUD_RTL_VER BLUEBUD_RTL_1_1_1

//*****************************************************************************
//! @name AMBT53 Memory Definitions.
//! @{
//*****************************************************************************
#define DSP_AHBS_OFFSET                  (0)
#define DSP_AHBS_SIZE                    (0x1000000)
#define DTCM_OFFSET                      (DSP_AHBS_OFFSET)
#define DTCM_SIZE                        (0x80000)
#define PTCM_OFFSET                      (0x200000)
#define PTCM_SIZE                        (0x40000)
#if (BLUEBUD_RTL_VER >= BLUEBUD_RTL_1_1_1)
#define SRAM_BANK0_OFFSET                (DSP_AHBS_OFFSET + DSP_AHBS_SIZE)
#define SRAM_BANK0_SIZE                  (0x80000)
#define SRAM_BANK1_OFFSET                (SRAM_BANK0_OFFSET + SRAM_BANK0_SIZE)
#define SRAM_BANK1_SIZE                  (0x80000)
#define SRAM_BANK2_OFFSET                (SRAM_BANK1_OFFSET + SRAM_BANK1_SIZE)
#define SRAM_BANK2_SIZE                  (0x80000)
#define SRAM_BANK3_OFFSET                (SRAM_BANK2_OFFSET + SRAM_BANK2_SIZE)
#define SRAM_BANK3_SIZE                  (0x40000)
#else
#define SRAM_BANK0_OFFSET                (DSP_AHBS_OFFSET + DSP_AHBS_SIZE)
#define SRAM_BANK0_SIZE                  (0x200000)
#define SRAM_BANK1_OFFSET                (0x2000000)
#define SRAM_BANK1_SIZE                  (0x80000)
#define SRAM_BANK2_OFFSET                (SRAM_BANK1_OFFSET + SRAM_BANK1_SIZE)
#define SRAM_BANK2_SIZE                  (0x80000)
#define SRAM_BANK3_OFFSET                (SRAM_BANK2_OFFSET + SRAM_BANK2_SIZE)
#define SRAM_BANK3_SIZE                  (0x80000)
#endif

//! @}
//*****************************************************************************
//
//! @name Share memory map
//! @{
//
//*****************************************************************************
#if (MSPI_AMBT53_MODULE == 0)
#define MSPI_XIPMM_BASE_ADDRESS          0x14000000
#elif (MSPI_AMBT53_MODULE == 1)
#define MSPI_XIPMM_BASE_ADDRESS          0x18000000
#elif (MSPI_AMBT53_MODULE == 2)
#define MSPI_XIPMM_BASE_ADDRESS          0x1C000000
#endif // MSPI_PSRAM_MODULE == 0

#define AMBT53_ADDR(offset)               (MSPI_XIPMM_BASE_ADDRESS + (offset))

#define AMBT53_DATA8(addr)                (*(volatile uint8_t *) (addr))
#define AMBT53_DATA16(addr)               (*(volatile uint16_t *)(addr))
#define AMBT53_DATA32(addr)               (*(volatile uint32_t *)(addr))

#define DSP_AHBS_ADDR(offset)            (AMBT53_ADDR(DSP_AHBS_OFFSET + (offset)))
#define SRAM_BANK0_ADDR(offset)          (AMBT53_ADDR(SRAM_BANK0_OFFSET + (offset)))
#define SRAM_BANK1_ADDR(offset)          (AMBT53_ADDR(SRAM_BANK1_OFFSET + (offset)))
#define SRAM_BANK2_ADDR(offset)          (AMBT53_ADDR(SRAM_BANK2_OFFSET + (offset)))
#define SRAM_BANK3_ADDR(offset)          (AMBT53_ADDR(SRAM_BANK3_OFFSET + (offset)))

#define BIG_LITTLE_SWAP16(n)             ((((uint16_t)(n) & 0xff00) >> 8) | (((uint16_t)(n) & 0x00ff) << 8))
#define BIG_LITTLE_SWAP32(n)             ((((uint32_t)(n) & 0xff000000) >> 24) | (((uint32_t)(n) & 0x00ff0000) >> 8) | (((uint32_t)(n) & 0x0000ff00) << 8) | (((uint32_t)(n) & 0x000000ff) << 24))
//! @}
//*****************************************************************************
//! @name AMBT53 register definitions.
//! @{
//*****************************************************************************

//! Register properties
#define REG_PROP_RO                      (0x01)
#define REG_PROP_RW                      (0x02)
#define REG_PROP_WO                      (0x03)
#define REG_PROP_RWC                     (0x04)
#define REG_PROP_RSVD                    (0x05)
#define REG_PROP_WC                      (0x06)
//! @}

// ********************************BTDMP********************************
//Address

//! @defgroup BTDMPBTDMP BTDMP
//! @ingroup ambt53
//! @{

//!Base address of the TDM-TR0
#define TDM_RX0_BASE_ADDR       (0x410000) // VLSI_PL
//!Base address of the TDM-TX0
#define TDM_TX0_BASE_ADDR       (0x418000) // VLSI_PL

//! The next start address of TDM-TXx is in 0x1000
#define OFFSET_BETWEEN_TDM      (0x2000) //(0x1000)

// **************** TX *****************************************
//! @name TX ADDRESSES
//! @{
// *************************************************************
#define TDMx_BASE_ADDRESS(x)    (TDM_TX0_BASE_ADDR + OFFSET_BETWEEN_TDM * (x))
//x is the number of the TDM
#define TDM_CFG_ADDR_TX(x)      (TDMx_BASE_ADDRESS(x) + 0x0)
#define TDM_ENFLSH_ADDR_TX(x)   (TDMx_BASE_ADDRESS(x) + 0x4)
#define TDM_PH12_ADDR_TX(x)     (TDMx_BASE_ADDRESS(x) + 0x8)
#define TDM_FRCFG_ADDR_TX(x)    (TDMx_BASE_ADDRESS(x) + 0xC)
#define TDM_CDIV_ADDR_TX(x)     (TDMx_BASE_ADDRESS(x) + 0x10)
#define TDM_MCEE_ADDR_TX(x)     (TDMx_BASE_ADDRESS(x) + 0x14)
#define TDM_STAT_ADDR_TX(x)     (TDMx_BASE_ADDRESS(x) + 0x18)
//y is the offset 0 = first,1=second... the offset is multiplied by 2.
#define TDM_TDP_ADDR_TX(x, y)   (TDMx_BASE_ADDRESS(x) + 0x80 + 4 * (y))

//! @}

#define CSAFE_BIT               0x10000

// ****************************************************************************
//! @name TDM TX Regs
//! @{
// ****************************************************************************
#define TDM_CFG_REG_TX(x)       *((volatile unsigned long *)TDM_CFG_ADDR_TX(x))
#define TDM_ENFLSH_REG_TX(x)    *((volatile unsigned long *)TDM_ENFLSH_ADDR_TX(x))
#define TDM_PH12_REG_TX(x)      *((volatile unsigned long *)TDM_PH12_ADDR_TX(x))
#define TDM_FRCFG_REG_TX(x)     *((volatile unsigned long *)TDM_FRCFG_ADDR_TX(x))
#define TDM_CDIV_REG_TX(x)      *((volatile unsigned long *)TDM_CDIV_ADDR_TX(x))
#define TDM_MCEE_REG_TX(x)      *((volatile unsigned long *)TDM_MCEE_ADDR_TX(x))
#define TDM_STAT_REG_TX(x)      *((volatile unsigned long *)TDM_STAT_ADDR_TX(x))
#define TDM_TDP_REG_TX(x, y)    *((volatile unsigned long *)TDM_TDP_ADDR_TX((x), (y)))

#define TDMx_BASE_ADDRESS_RX(x) (TDM_RX0_BASE_ADDR + OFFSET_BETWEEN_TDM * (x))
//x is the number of the TDM
#define TDM_CFG_ADDR_RX(x)      (TDMx_BASE_ADDRESS_RX(x) + 0x0)
#define TDM_ENFLSH_ADDR_RX(x)   (TDMx_BASE_ADDRESS_RX(x) + 0x4)
#define TDM_PH12_ADDR_RX(x)     (TDMx_BASE_ADDRESS_RX(x) + 0x8)
#define TDM_FRCFG_ADDR_RX(x)    (TDMx_BASE_ADDRESS_RX(x) + 0xC)
#define TDM_CDIV_ADDR_RX(x)     (TDMx_BASE_ADDRESS_RX(x) + 0x10)
#define TDM_MCEE_ADDR_RX(x)     (TDMx_BASE_ADDRESS_RX(x) + 0x14)
#define TDM_STAT_ADDR_RX(x)     (TDMx_BASE_ADDRESS_RX(x) + 0x18)
//y is the offset 0 = first,1=second... the offset is multiplied by 2.
#define TDM_RDP_ADDR_RX(x, y)   (TDMx_BASE_ADDRESS_RX(x) + 0x80 + 4 * (y))

//! @}

// ****************************************************************************
//! @name TDM RX Regs Definitions
//! @{
// ****************************************************************************
#define TDM_CFG_REG_RX(x)       *((volatile unsigned long *)TDM_CFG_ADDR_RX(x))
#define TDM_ENFLSH_REG_RX(x)    *((volatile unsigned long *)TDM_ENFLSH_ADDR_RX(x))
#define TDM_PH12_REG_RX(x)      *((volatile unsigned long *)TDM_PH12_ADDR_RX(x))
#define TDM_FRCFG_REG_RX(x)     *((volatile unsigned long *)TDM_FRCFG_ADDR_RX(x))
#define TDM_CDIV_REG_RX(x)      *((volatile unsigned long *)TDM_CDIV_ADDR_RX(x))
#define TDM_MCEE_REG_RX(x)      *((volatile unsigned long *)TDM_MCEE_ADDR_RX(x))
#define TDM_STAT_REG_RX(x)      *((volatile unsigned long *)TDM_STAT_ADDR_RX(x))
#define TDM_RDP_REG_RX(x, y)    *((volatile unsigned long *)TDM_RDP_ADDR_RX((x), (y)))
//! @}

// ****************************************************************************
//! @name RX Definitions
//! @{
// ****************************************************************************
#define DM_CFG_RX_MASK                               ((uint32_t)0xC0FF41FF)
#define DM_CFG_RX_RESET                              0x00000005

#define DM_ENFLSH_RX_RW_MASK                         ((uint32_t)0x00000001)
#define DM_ENFLSH_RX_RESET                           0x00000000

#define DM_PH12_RX_MASK                              ((uint32_t)0x0FE73FE7)
#define DM_PH12_RX_RESET                             0x00000000

#define DM_FRCFG_RX_MASK                             ((uint32_t)0x3FFF1FFF)
#define DM_FRCFG_RX_RESET                            0x00000000

#define DM_CDIV_RX_MASK                              ((uint32_t)0x0000FFFF)
#define DM_CDIV_RX_RESET                             0x00000000

#define DM_MCEE_RX_MASK                              ((uint32_t)0xFFFFFFFF)
#define DM_MCEE_RX_RESET                             0x00000000

#define DM_STAT_RX_MASK                              ((uint32_t)0x000101FF)
#define DM_STAT_RX_RESET                             0x00010000

#define DM_CFG_TX_MASK                               ((uint32_t)0xC0FF41FF)
#define DM_CFG_TX_RESET                              0x00000005

#define DM_ENFLSH_TX_RW_MASK                         ((uint32_t)0x00000001)
#define DM_ENFLSH_TX_RESET                           0x00000000

#define DM_PH12_TX_MASK                              ((uint32_t)0x0FE73FE7)
#define DM_PH12_TX_RESET                             0x00000000

#define DM_FRCFG_TX_MASK                             ((uint32_t)0x3FFF1FFF)
#define DM_FRCFG_TX_RESET                            0x00000000

#define DM_CDIV_TX_MASK                              ((uint32_t)0x0000FFFF)
#define DM_CDIV_TX_RESET                             0x00000000

#define DM_MCEE_TX_MASK                              ((uint32_t)0xFFFFFFFF)
#define DM_MCEE_TX_RESET                             0x00000000

#define DM_STAT_TX_MASK                              ((uint32_t)0x000101FF)
#define DM_STAT_TX_RESET                             0x00010113
//! @}

//! @}  // BTDM

// ****************************************************************************
//! @defgroup INTCNTL INTCNTL
//! @ingroup ambt53
//! @{
//! @name AHB_BT_ICU REGISTER DEFINITION
//! @{
// ****************************************************************************
//================AHB_BT_ICU REGISTER DEFINITION================
#define INTCNTL_IRQ_STATUS_MASK                    ((uint32_t)0xFFFFFFFF)

#define INTCNTL_IRQ_STATUS1_MASK                   ((uint32_t)0xFFFFFFFF)

#define INTCNTL_IRQ_RAW_STATUS_MASK                ((uint32_t)0xFFFFFFFF)

#define INTCNTL_IRQ_RAW_STATUS1_MASK               ((uint32_t)0xFFFFFFFF)

#define INTCNTL_IRQ_UNMASK_SET_MASK                ((uint32_t)0xFFFFFFFF)

#define INTCNTL_IRQ_UNMASK_SET1_MASK               ((uint32_t)0xFFFFFFFF)

#define INTCNTL_IRQ_UNMASK_CLEAR_MASK              ((uint32_t)0xFFFFFFFF)

#define INTCNTL_IRQ_UNMASK_CLEAR1_MASK             ((uint32_t)0xFFFFFFFF)

#define INTCNTL_IRQ_POLARITY_MASK                  ((uint32_t)0xFFFFFFFF)

#define INTCNTL_IRQ_POLARITY1_MASK                 ((uint32_t)0xFFFFFFFF)

#define INTCNTL_IRQ_INDEX_MASK                     ((uint32_t)0xFFFFFFFF)

//! @}

// ****************************************************************************

//! @name AHB_BT_ICU REGISTER DEFINITION
//! @{
#define REG_INTCNTL_DECODING_MASK 0x0000007F
//! @}

// ****************************************************************************
//! @name IRQ_STATUS register definition
//! @{
// ****************************************************************************
#define INTCNTL_IRQ_STATUS_ADDR   0x00C00000
#define INTCNTL_IRQ_STATUS_OFFSET 0x00000000
#define INTCNTL_IRQ_STATUS_INDEX  0x00000000
#define INTCNTL_IRQ_STATUS_RESET  0x00000000
#define INTCNTL_IRQ_STATUS_COUNT  2
//! @}

// ****************************************************************************
//! @name IRQ_RAW_STATUS register definition
//! @{
// ****************************************************************************
#define INTCNTL_IRQ_RAW_STATUS_ADDR   0x00C00008
#define INTCNTL_IRQ_RAW_STATUS_OFFSET 0x00000008
#define INTCNTL_IRQ_RAW_STATUS_INDEX  0x00000002
#define INTCNTL_IRQ_RAW_STATUS_RESET  0x00000000
#define INTCNTL_IRQ_RAW_STATUS_COUNT  2
//! @}

// ****************************************************************************
//! @name IRQ_UNMASK_SET register definition
//! @{
// ****************************************************************************
#define INTCNTL_IRQ_UNMASK_SET_ADDR   0x00C00010
#define INTCNTL_IRQ_UNMASK_SET_OFFSET 0x00000010
#define INTCNTL_IRQ_UNMASK_SET_INDEX  0x00000004
#define INTCNTL_IRQ_UNMASK_SET_RESET  0x00000000
#define INTCNTL_IRQ_UNMASK_SET_COUNT  2
//! @}

// ****************************************************************************
//! @name IRQ_UNMASK_CLEAR register definition
//! @{
// ****************************************************************************
#define INTCNTL_IRQ_UNMASK_CLEAR_ADDR   0x00C00018
#define INTCNTL_IRQ_UNMASK_CLEAR_OFFSET 0x00000018
#define INTCNTL_IRQ_UNMASK_CLEAR_INDEX  0x00000006
#define INTCNTL_IRQ_UNMASK_CLEAR_RESET  0x00000000
#define INTCNTL_IRQ_UNMASK_CLEAR_COUNT  2
//! @}

// ****************************************************************************
//! @name IRQ_POLARITY register definition
//! @{
// ****************************************************************************
#define INTCNTL_IRQ_POLARITY_ADDR   0x00C00020
#define INTCNTL_IRQ_POLARITY_OFFSET 0x00000020
#define INTCNTL_IRQ_POLARITY_INDEX  0x00000008
#define INTCNTL_IRQ_POLARITY_RESET  0xFFFFFFFF
#define INTCNTL_IRQ_POLARITY_COUNT  2
//! @}

// ****************************************************************************
//! @name IRQ_INDEX register definition
//! @{
// ****************************************************************************
#define INTCNTL_IRQ_INDEX_ADDR   0x00C00040
#define INTCNTL_IRQ_INDEX_OFFSET 0x00000040
#define INTCNTL_IRQ_INDEX_INDEX  0x00000010
#define INTCNTL_IRQ_INDEX_RESET  0x00000000
//! @}

//! @} // INTCNTL

// ****************************************************************************
//! @defgroup UART UART
//! @ingroup ambt53
//! @{
// ****************************************************************************

#define REG_UART_DECODING_MASK 0x0000003F

//! @name GENERAL_CONFIG register definition
//! @{
//!  <pre>
//!    Bits           Field Name   Reset Value
//!   -----   ------------------   -----------
//!      06          BAUD_CLK_EN   0
//!   05:04          PARITY_TYPE   0x0
//!      03            PARITY_EN   0
//!      02             STOP_LEN   0
//!   01:00             WORD_LEN   0x0
//!  </pre>
#define UART_GENERAL_CONFIG_ADDR   0x00C01000
#define UART_GENERAL_CONFIG_OFFSET 0x00000000
#define UART_GENERAL_CONFIG_INDEX  0x00000000
#define UART_GENERAL_CONFIG_RESET  0x00000000
//! @}  // GENERAL_CONFIG

//! @name GENERAL_CONFIG Field Definitions
//! @{

#define UART_BAUD_CLK_EN_BIT    ((uint32_t)0x00000040)
#define UART_BAUD_CLK_EN_POS    6
#define UART_PARITY_TYPE_MASK   ((uint32_t)0x00000030)
#define UART_PARITY_TYPE_LSB    4
#define UART_PARITY_TYPE_WIDTH  ((uint32_t)0x00000002)
#define UART_PARITY_EN_BIT      ((uint32_t)0x00000008)
#define UART_PARITY_EN_POS      3
#define UART_STOP_LEN_BIT       ((uint32_t)0x00000004)
#define UART_STOP_LEN_POS       2
#define UART_WORD_LEN_MASK      ((uint32_t)0x00000003)
#define UART_WORD_LEN_LSB       0
#define UART_WORD_LEN_WIDTH     ((uint32_t)0x00000002)

#define UART_BAUD_CLK_EN_RST    0x0
#define UART_PARITY_TYPE_RST    0x0
#define UART_PARITY_EN_RST      0x0
#define UART_STOP_LEN_RST       0x0
#define UART_WORD_LEN_RST       0x0
//! @}  // field

//! @name INT_DIV_LSB register definition
//! @{
//! * <pre>
//! *   Bits           Field Name   Reset Value
//! *  -----   ------------------   -----------
//! *  07:00          INT_DIV_LSB   0x2
//! * </pre>

#define UART_INT_DIV_LSB_ADDR   0x00C01004
#define UART_INT_DIV_LSB_OFFSET 0x00000004
#define UART_INT_DIV_LSB_INDEX  0x00000001
#define UART_INT_DIV_LSB_RESET  0x00000002
//! @}  // INT_DIV_LSB

//! @name INT_DIV_LSB Field Definitions
//! @{
#define UART_INT_DIV_LSB_MASK   ((uint32_t)0x000000FF)
#define UART_INT_DIV_LSB_LSB    0
#define UART_INT_DIV_LSB_WIDTH  ((uint32_t)0x00000008)
#define UART_INT_DIV_LSB_RST    0x2
//! @}  // field

//! @name INT_DIV_MSB register definition
//! @{
//!  <pre>
//!    Bits           Field Name   Reset Value
//!   -----   ------------------   -----------
//!   04:00          INT_DIV_MSB   0x0
//!  </pre>
//!
#define UART_INT_DIV_MSB_ADDR   0x00C01008
#define UART_INT_DIV_MSB_OFFSET 0x00000008
#define UART_INT_DIV_MSB_INDEX  0x00000002
#define UART_INT_DIV_MSB_RESET  0x00000000
//! @}  // INT_DIV_MSB

//! @name INT_DIV_MSB Field Definitions
//! @{
#define UART_INT_DIV_MSB_MASK   ((uint32_t)0x0000001F)
#define UART_INT_DIV_MSB_LSB    0
#define UART_INT_DIV_MSB_WIDTH  ((uint32_t)0x00000005)
#define UART_INT_DIV_MSB_RST    0x0
//! @}  // field

//! @name FRACT_DIV register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  02:00            FRACT_DIV   0x0
//! </pre>

#define UART_FRACT_DIV_ADDR   0x00C0100C
#define UART_FRACT_DIV_OFFSET 0x0000000C
#define UART_FRACT_DIV_INDEX  0x00000003
#define UART_FRACT_DIV_RESET  0x00000000
//! @}  // FRACT_DIV

//! @name FRACT_DIV Field Definitions
//! @{
// field definitions
#define UART_FRACT_DIV_MASK   ((uint32_t)0x00000007)
#define UART_FRACT_DIV_LSB    0
#define UART_FRACT_DIV_WIDTH  ((uint32_t)0x00000003)
#define UART_FRACT_DIV_RST    0x0
//! @}  // field

//! @name FLOW_CTRL register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     04        RX_FIFO_FLUSH   0
//!     03        TX_FIFO_FLUSH   0
//!     02           RTS_FORCED   0
//!     01             AUTO_RTS   0
//!     00               CTS_EN   0
//! </pre>
//
#define UART_FLOW_CTRL_ADDR   0x00C01010
#define UART_FLOW_CTRL_OFFSET 0x00000010
#define UART_FLOW_CTRL_INDEX  0x00000004
#define UART_FLOW_CTRL_RESET  0x00000000
//! @}  // FLOW_CTRL

//! @name FLOW_CTRL Field Definitions
//! @{
#define UART_RX_FIFO_FLUSH_BIT    ((uint32_t)0x00000010)
#define UART_RX_FIFO_FLUSH_POS    4
#define UART_TX_FIFO_FLUSH_BIT    ((uint32_t)0x00000008)
#define UART_TX_FIFO_FLUSH_POS    3
#define UART_RTS_FORCED_BIT       ((uint32_t)0x00000004)
#define UART_RTS_FORCED_POS       2
#define UART_AUTO_RTS_BIT         ((uint32_t)0x00000002)
#define UART_AUTO_RTS_POS         1
#define UART_CTS_EN_BIT           ((uint32_t)0x00000001)
#define UART_CTS_EN_POS           0

#define UART_RX_FIFO_FLUSH_RST    0x0
#define UART_TX_FIFO_FLUSH_RST    0x0
#define UART_RTS_FORCED_RST       0x0
#define UART_AUTO_RTS_RST         0x0
#define UART_CTS_EN_RST           0x0

//! @}  // field

//! @name RX_FIFO_THRESHOLD register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00          RX_FIFO_THR   0x0
//! </pre>
//!
#define UART_RX_FIFO_THRESHOLD_ADDR   0x00C01014
#define UART_RX_FIFO_THRESHOLD_OFFSET 0x00000014
#define UART_RX_FIFO_THRESHOLD_INDEX  0x00000005
#define UART_RX_FIFO_THRESHOLD_RESET  0x00000000
//! @}  // RX_FIFO_THRESHOLD

//! @name RX_FIFO_THRESHOLD Field Definitions
//! @{
#define UART_RX_FIFO_THR_MASK   ((uint32_t)0x000000FF)
#define UART_RX_FIFO_THR_LSB    0
#define UART_RX_FIFO_THR_WIDTH  ((uint32_t)0x00000008)
#define UART_RX_FIFO_THR_RST    0x0
//! @}  // field

//! @name TX_FIFO_THRESHOLD register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00          TX_FIFO_THR   0x0
//! </pre>
//
#define UART_TX_FIFO_THRESHOLD_ADDR   0x00C01018
#define UART_TX_FIFO_THRESHOLD_OFFSET 0x00000018
#define UART_TX_FIFO_THRESHOLD_INDEX  0x00000006
#define UART_TX_FIFO_THRESHOLD_RESET  0x00000000
//! @}  // TX_FIFO_THRESHOLD

//! @name TX_FIFO_THRESHOLD Field Definitions
//! @{
#define UART_TX_FIFO_THR_MASK   ((uint32_t)0x000000FF)
#define UART_TX_FIFO_THR_LSB    0
#define UART_TX_FIFO_THR_WIDTH  ((uint32_t)0x00000008)
#define UART_TX_FIFO_THR_RST    0x0
//! @}  // field

//! @name RX_TIMEOUT register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00           RX_TIMEOUT   0x0
//! </pre>
//!
#define UART_RX_TIMEOUT_ADDR   0x00C0101C
#define UART_RX_TIMEOUT_OFFSET 0x0000001C
#define UART_RX_TIMEOUT_INDEX  0x00000007
#define UART_RX_TIMEOUT_RESET  0x00000000
//! @}  // RX_TIMEOUT

//! @name RX_TIMEOUT Field Definitions
//! @{
#define UART_RX_TIMEOUT_MASK   ((uint32_t)0x000000FF)
#define UART_RX_TIMEOUT_LSB    0
#define UART_RX_TIMEOUT_WIDTH  ((uint32_t)0x00000008)
#define UART_RX_TIMEOUT_RST    0x0
//! @}  // RX_TIMEOUT field

//
//! @name RX_DATA register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00              RX_DATA   0x0
//! </pre>
//
#define UART_RX_DATA_ADDR   0x00C01020
#define UART_RX_DATA_OFFSET 0x00000020
#define UART_RX_DATA_INDEX  0x00000008
#define UART_RX_DATA_RESET  0x00000000
//! @}  // RX_DATA

//! @name RX_DATA Field Definitions
//! @{
#define UART_RX_DATA_MASK   ((uint32_t)0x000000FF)
#define UART_RX_DATA_LSB    0
#define UART_RX_DATA_WIDTH  ((uint32_t)0x00000008)
#define UART_RX_DATA_RST    0x0
//! @}  // field

//
//! @name RX_STATUS register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     07           RX_TIMEOUT   0
//!     06             RX_BREAK   0
//!     05         RX_FRAME_ERR   0
//!     04        RX_PARITY_ERR   0
//!     03      RX_FIFO_OVERRUN   0
//!     02         RX_FIFO_FULL   0
//!     01   RX_FIFO_ALMOST_FULL   0
//!     00    RX_FIFO_NOT_EMPTY   0
//! </pre>
//
#define UART_RX_STATUS_ADDR   0x00C01024
#define UART_RX_STATUS_OFFSET 0x00000024
#define UART_RX_STATUS_INDEX  0x00000009
#define UART_RX_STATUS_RESET  0x00000000
//! @}  // RX_STATUS

//! @name RX_STATUS Field Definitions
//! @{
#define UART_RX_TIMEOUT_BIT             ((uint32_t)0x00000080)
#define UART_RX_TIMEOUT_POS             7
#define UART_RX_BREAK_BIT               ((uint32_t)0x00000040)
#define UART_RX_BREAK_POS               6
#define UART_RX_FRAME_ERR_BIT           ((uint32_t)0x00000020)
#define UART_RX_FRAME_ERR_POS           5
#define UART_RX_PARITY_ERR_BIT          ((uint32_t)0x00000010)
#define UART_RX_PARITY_ERR_POS          4
#define UART_RX_FIFO_OVERRUN_BIT        ((uint32_t)0x00000008)
#define UART_RX_FIFO_OVERRUN_POS        3
#define UART_RX_FIFO_FULL_BIT           ((uint32_t)0x00000004)
#define UART_RX_FIFO_FULL_POS           2
#define UART_RX_FIFO_ALMOST_FULL_BIT    ((uint32_t)0x00000002)
#define UART_RX_FIFO_ALMOST_FULL_POS    1
#define UART_RX_FIFO_NOT_EMPTY_BIT      ((uint32_t)0x00000001)
#define UART_RX_FIFO_NOT_EMPTY_POS      0

#define UART_RX_TIMEOUT_RST             0x0
#define UART_RX_BREAK_RST               0x0
#define UART_RX_FRAME_ERR_RST           0x0
#define UART_RX_PARITY_ERR_RST          0x0
#define UART_RX_FIFO_OVERRUN_RST        0x0
#define UART_RX_FIFO_FULL_RST           0x0
#define UART_RX_FIFO_ALMOST_FULL_RST    0x0
#define UART_RX_FIFO_NOT_EMPTY_RST      0x0

//! @}  // field

//
//! @name RX_MASK register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     07      RX_TIMEOUT_MASK   0
//!     06        RX_BREAK_MASK   0
//!     05    RX_FRAME_ERR_MASK   0
//!     04   RX_PARITY_ERR_MASK   0
//!     03   RX_FIFO_OVERRUN_MASK   0
//!     02    RX_FIFO_FULL_MASK   0
//!     01   RX_FIFO_THRESHOLD_MASK   0
//!     00   RX_FIFO_NOT_EMPTY_MASK   0
//! </pre>
//
#define UART_RX_MASK_ADDR   0x00C01028
#define UART_RX_MASK_OFFSET 0x00000028
#define UART_RX_MASK_INDEX  0x0000000A
#define UART_RX_MASK_RESET  0x00000000
//! @}  // RX_MASK

//! @name RX_MASK Field Definitions
//! @{
#define UART_RX_TIMEOUT_MASK_BIT           ((uint32_t)0x00000080)
#define UART_RX_TIMEOUT_MASK_POS           7
#define UART_RX_BREAK_MASK_BIT             ((uint32_t)0x00000040)
#define UART_RX_BREAK_MASK_POS             6
#define UART_RX_FRAME_ERR_MASK_BIT         ((uint32_t)0x00000020)
#define UART_RX_FRAME_ERR_MASK_POS         5
#define UART_RX_PARITY_ERR_MASK_BIT        ((uint32_t)0x00000010)
#define UART_RX_PARITY_ERR_MASK_POS        4
#define UART_RX_FIFO_OVERRUN_MASK_BIT      ((uint32_t)0x00000008)
#define UART_RX_FIFO_OVERRUN_MASK_POS      3
#define UART_RX_FIFO_FULL_MASK_BIT         ((uint32_t)0x00000004)
#define UART_RX_FIFO_FULL_MASK_POS         2
#define UART_RX_FIFO_THRESHOLD_MASK_BIT    ((uint32_t)0x00000002)
#define UART_RX_FIFO_THRESHOLD_MASK_POS    1
#define UART_RX_FIFO_NOT_EMPTY_MASK_BIT    ((uint32_t)0x00000001)
#define UART_RX_FIFO_NOT_EMPTY_MASK_POS    0

#define UART_RX_TIMEOUT_MASK_RST           0x0
#define UART_RX_BREAK_MASK_RST             0x0
#define UART_RX_FRAME_ERR_MASK_RST         0x0
#define UART_RX_PARITY_ERR_MASK_RST        0x0
#define UART_RX_FIFO_OVERRUN_MASK_RST      0x0
#define UART_RX_FIFO_FULL_MASK_RST         0x0
#define UART_RX_FIFO_THRESHOLD_MASK_RST    0x0
#define UART_RX_FIFO_NOT_EMPTY_MASK_RST    0x0
//! @}  //  RX_MASK field

//!
//! @name TX_DATA register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00              TX_DATA   0x0
//! </pre>
//!
#define UART_TX_DATA_ADDR   0x00C0102C
#define UART_TX_DATA_OFFSET 0x0000002C
#define UART_TX_DATA_INDEX  0x0000000B
#define UART_TX_DATA_RESET  0x00000000
//! @}  //

//! @name TX_DATA Field Definitions
//! @{
#define UART_TX_DATA_MASK   ((uint32_t)0x000000FF)
#define UART_TX_DATA_LSB    0
#define UART_TX_DATA_WIDTH  ((uint32_t)0x00000008)
#define UART_TX_DATA_RST    0x0
//! @}  // field

//!
//! @name TX_STATUS register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     04                  CTS   0
//!     03          TX_SR_EMPTY   0
//!     02        TX_FIFO_EMPTY   0
//!     01   TX_FIO_ALMOST_EMPTY   0
//!     00     TX_FIFO_NOT_FULL   0
//! </pre>
//!
#define UART_TX_STATUS_ADDR   0x00C01030
#define UART_TX_STATUS_OFFSET 0x00000030
#define UART_TX_STATUS_INDEX  0x0000000C
#define UART_TX_STATUS_RESET  0x00000000
//! @}  //

//! @name TX_STATUS Field Definitions
//! @{
#define UART_CTS_BIT                    ((uint32_t)0x00000010)
#define UART_CTS_POS                    4
#define UART_TX_SR_EMPTY_BIT            ((uint32_t)0x00000008)
#define UART_TX_SR_EMPTY_POS            3
#define UART_TX_FIFO_EMPTY_BIT          ((uint32_t)0x00000004)
#define UART_TX_FIFO_EMPTY_POS          2
#define UART_TX_FIO_ALMOST_EMPTY_BIT    ((uint32_t)0x00000002)
#define UART_TX_FIO_ALMOST_EMPTY_POS    1
#define UART_TX_FIFO_NOT_FULL_BIT       ((uint32_t)0x00000001)
#define UART_TX_FIFO_NOT_FULL_POS       0

#define UART_CTS_RST                    0x0
#define UART_TX_SR_EMPTY_RST            0x0
#define UART_TX_FIFO_EMPTY_RST          0x0
#define UART_TX_FIO_ALMOST_EMPTY_RST    0x0
#define UART_TX_FIFO_NOT_FULL_RST       0x0
//! @}  // field

//! @name TX_MASK register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     04             CTS_MASK   0
//!     03     TX_SR_EMPTY_MASK   0
//!     02   TX_FIFO_EMPTY_MASK   0
//!     01   TX_FIO_ALMOST_EMPTY_MASK   0
//!     00   TX_FIFO_NOT_FULL_MASK   0
//! </pre>
//
#define UART_TX_MASK_ADDR   0x00C01034
#define UART_TX_MASK_OFFSET 0x00000034
#define UART_TX_MASK_INDEX  0x0000000D
#define UART_TX_MASK_RESET  0x00000000
//! @}  // TX_MASK

//! @name TX_MASK Field Definitions
//! @{
#define UART_CTS_MASK_BIT                    ((uint32_t)0x00000010)
#define UART_CTS_MASK_POS                    4
#define UART_TX_SR_EMPTY_MASK_BIT            ((uint32_t)0x00000008)
#define UART_TX_SR_EMPTY_MASK_POS            3
#define UART_TX_FIFO_EMPTY_MASK_BIT          ((uint32_t)0x00000004)
#define UART_TX_FIFO_EMPTY_MASK_POS          2
#define UART_TX_FIO_ALMOST_EMPTY_MASK_BIT    ((uint32_t)0x00000002)
#define UART_TX_FIO_ALMOST_EMPTY_MASK_POS    1
#define UART_TX_FIFO_NOT_FULL_MASK_BIT       ((uint32_t)0x00000001)
#define UART_TX_FIFO_NOT_FULL_MASK_POS       0

#define UART_CTS_MASK_RST                    0x0
#define UART_TX_SR_EMPTY_MASK_RST            0x0
#define UART_TX_FIFO_EMPTY_MASK_RST          0x0
#define UART_TX_FIO_ALMOST_EMPTY_MASK_RST    0x0
#define UART_TX_FIFO_NOT_FULL_MASK_RST       0x0
//! @}  // field

//! @} // UART group

//! @defgroup Counter Counter
//! @ingroup ambt53
//! @{
#define REG_COUNTER_DECODING_MASK 0x0000000F

//
//! @name COUNTER_VALUE register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00                VALUE   0x0
//! </pre>
//!
#define COUNTER_COUNTER_VALUE_ADDR   0x00C04000//0x00404000
#define COUNTER_COUNTER_VALUE_OFFSET 0x00000000
#define COUNTER_COUNTER_VALUE_INDEX  0x00000000
#define COUNTER_COUNTER_VALUE_RESET  0x00000000
//! @}  //

//! @name COUNTER_VALUE Field Definitions
//! @{
#define COUNTER_VALUE_MASK   ((uint32_t)0xFFFFFFFF)
#define COUNTER_VALUE_LSB    0
#define COUNTER_VALUE_WIDTH  ((uint32_t)0x00000020)
#define COUNTER_VALUE_RST    0x0
//! @}  // COUNTER_VALUE field

//
//! @name COUNTER_RELOAD_VALUE register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00               RELOAD   0x0
//! </pre>
//
#define COUNTER_COUNTER_RELOAD_VALUE_ADDR   0x00C04004//0x00404004
#define COUNTER_COUNTER_RELOAD_VALUE_OFFSET 0x00000004
#define COUNTER_COUNTER_RELOAD_VALUE_INDEX  0x00000001
#define COUNTER_COUNTER_RELOAD_VALUE_RESET  0x00000000
//! @}  //

//! @name COUNTER_RELOAD_VALUE Field Definitions
//! @{
#define COUNTER_RELOAD_MASK   ((uint32_t)0xFFFFFFFF)
#define COUNTER_RELOAD_LSB    0
#define COUNTER_RELOAD_WIDTH  ((uint32_t)0x00000020)
#define COUNTER_RELOAD_RST    0x0
//! @}  //  COUNTER_RELOAD_VALUEfield

//! @name COUNTER_EXPIRED_FLAG register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     00              EXPIRED   0
//! </pre>

#define COUNTER_COUNTER_EXPIRED_FLAG_ADDR   0x00C04008//0x00404008
#define COUNTER_COUNTER_EXPIRED_FLAG_OFFSET 0x00000008
#define COUNTER_COUNTER_EXPIRED_FLAG_INDEX  0x00000002
#define COUNTER_COUNTER_EXPIRED_FLAG_RESET  0x00000000
//! @}  //

//! @name COUNTER_EXPIRED_FLAG Field Definitions
//! @{
#define COUNTER_EXPIRED_BIT    ((uint32_t)0x00000001)
#define COUNTER_EXPIRED_POS    0
#define COUNTER_EXPIRED_RST    0x0
//! @}  // field

//
//! @name COUNTER_INT_MASK register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     00                 MASK   0
//! </pre>
//
#define COUNTER_COUNTER_INT_MASK_ADDR   0x00C0400C//0x0040400C
#define COUNTER_COUNTER_INT_MASK_OFFSET 0x0000000C
#define COUNTER_COUNTER_INT_MASK_INDEX  0x00000003
#define COUNTER_COUNTER_INT_MASK_RESET  0x00000000
//! @}  //

//! @name COUNTER_INT_MASK Field Definitions
//! @{
#define COUNTER_MASK_BIT    ((uint32_t)0x00000001)
#define COUNTER_MASK_POS    0
#define COUNTER_MASK_RST    0x0
//! @}  // field
//! @}  // COUNTER group

// ********************************BT_GPIO********************************
//! @defgroup BT_GPIO BT_GPIO
//! @ingroup ambt53
//! @{

#define REG_GPIO_DECODING_MASK 0x0000003F

//
//! @name GPIO_OUT register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00             GPIO_OUT   0x0
//! </pre>
//
#define GPIO_GPIO_OUT_ADDR   0x00C05000
#define GPIO_GPIO_OUT_OFFSET 0x00000000
#define GPIO_GPIO_OUT_INDEX  0x00000000
#define GPIO_GPIO_OUT_RESET  0x00000000
//! @}  //

//! @name GPIO_OUT Field Definitions
//! @{
#define GPIO_GPIO_OUT_MASK   ((uint32_t)0xFFFFFFFF)
#define GPIO_GPIO_OUT_LSB    0
#define GPIO_GPIO_OUT_WIDTH  ((uint32_t)0x00000020)
#define GPIO_GPIO_OUT_RST    0x0
//! @}  // field

//!
//! @name GPIO_IN register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00              GPIO_IN   0x0
//! </pre>
//!
#define GPIO_GPIO_IN_ADDR   0x00C05004
#define GPIO_GPIO_IN_OFFSET 0x00000004
#define GPIO_GPIO_IN_INDEX  0x00000001
#define GPIO_GPIO_IN_RESET  0x00000000
//! @}  // GPIO_IN

//! @name  GPIO_IN Field Definitions
//! @{
#define GPIO_GPIO_IN_MASK   ((uint32_t)0xFFFFFFFF)
#define GPIO_GPIO_IN_LSB    0
#define GPIO_GPIO_IN_WIDTH  ((uint32_t)0x00000020)
#define GPIO_GPIO_IN_RST    0x0
//! @}  // GPIO_INfield

//!
//! @name GPIO_DIR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00             GPIO_DIR   0x0
//! </pre>
//!
#define GPIO_GPIO_DIR_ADDR   0x00C05008
#define GPIO_GPIO_DIR_OFFSET 0x00000008
#define GPIO_GPIO_DIR_INDEX  0x00000002
#define GPIO_GPIO_DIR_RESET  0x00000000
//! @}  // GPIO_DIR

//! @name  GPIO_DIR Field Definitions
//! @{
#define GPIO_GPIO_DIR_MASK   ((uint32_t)0xFFFFFFFF)
#define GPIO_GPIO_DIR_LSB    0
#define GPIO_GPIO_DIR_WIDTH  ((uint32_t)0x00000020)
#define GPIO_GPIO_DIR_RST    0x0
//! @}  // GPIO_DIR field

//!
//! @name GPIO_INTMODE register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00         GPIO_INTMODE   0x0
//! </pre>
//!
#define GPIO_GPIO_INTMODE_ADDR   0x00C0500C
#define GPIO_GPIO_INTMODE_OFFSET 0x0000000C
#define GPIO_GPIO_INTMODE_INDEX  0x00000003
#define GPIO_GPIO_INTMODE_RESET  0x00000000
//! @}  // GPIO_INTMODE

//! @name GPIO_INTMODE Field Definitions
//! @{
#define GPIO_GPIO_INTMODE_MASK   ((uint32_t)0xFFFFFFFF)
#define GPIO_GPIO_INTMODE_LSB    0
#define GPIO_GPIO_INTMODE_WIDTH  ((uint32_t)0x00000020)
#define GPIO_GPIO_INTMODE_RST    0x0
//! @}  // field

//!
//! @name GPIO_INTPOL register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00          GPIO_INTPOL   0x0
//! </pre>
//!
#define GPIO_GPIO_INTPOL_ADDR   0x00C05010
#define GPIO_GPIO_INTPOL_OFFSET 0x00000010
#define GPIO_GPIO_INTPOL_INDEX  0x00000004
#define GPIO_GPIO_INTPOL_RESET  0x00000000
//! @}  //

//! @name GPIO_INTPOL Field Definitions
//! @{
#define GPIO_GPIO_INTPOL_MASK   ((uint32_t)0xFFFFFFFF)
#define GPIO_GPIO_INTPOL_LSB    0
#define GPIO_GPIO_INTPOL_WIDTH  ((uint32_t)0x00000020)
#define GPIO_GPIO_INTPOL_RST    0x0
//! @}  // field

//!
//! @name GPIO_INTEN register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00           GPIO_INTEN   0x0
//! </pre>
//!
#define GPIO_GPIO_INTEN_ADDR   0x00C05014
#define GPIO_GPIO_INTEN_OFFSET 0x00000014
#define GPIO_GPIO_INTEN_INDEX  0x00000005
#define GPIO_GPIO_INTEN_RESET  0x00000000
//! @}  //

//! @name GPIO_INTEN Field Definitions
//! @{
#define GPIO_GPIO_INTEN_MASK   ((uint32_t)0xFFFFFFFF)
#define GPIO_GPIO_INTEN_LSB    0
#define GPIO_GPIO_INTEN_WIDTH  ((uint32_t)0x00000020)
#define GPIO_GPIO_INTEN_RST    0x0
//! @}  // field

//!
//! @name GPIO_INTSTATRAW register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00      GPIO_INTSTATRAW   0x0
//! </pre>
//!
#define GPIO_GPIO_INTSTATRAW_ADDR   0x00C05018
#define GPIO_GPIO_INTSTATRAW_OFFSET 0x00000018
#define GPIO_GPIO_INTSTATRAW_INDEX  0x00000006
#define GPIO_GPIO_INTSTATRAW_RESET  0x00000000
//! @}  //

//! @name GPIO_INTSTATRAW Field Definitions
//! @{
#define GPIO_GPIO_INTSTATRAW_MASK   ((uint32_t)0xFFFFFFFF)
#define GPIO_GPIO_INTSTATRAW_LSB    0
#define GPIO_GPIO_INTSTATRAW_WIDTH  ((uint32_t)0x00000020)
#define GPIO_GPIO_INTSTATRAW_RST    0x0
//! @}  // field

//!
//! @name GPIO_INTSTATMASKED register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00   GPIO_INTSTATMASKED   0x0
//! </pre>
//!
#define GPIO_GPIO_INTSTATMASKED_ADDR   0x00C0501C
#define GPIO_GPIO_INTSTATMASKED_OFFSET 0x0000001C
#define GPIO_GPIO_INTSTATMASKED_INDEX  0x00000007
#define GPIO_GPIO_INTSTATMASKED_RESET  0x00000000
//! @}  //

//! @name GPIO_INTSTATMASKED Field Definitions
//! @{
#define GPIO_GPIO_INTSTATMASKED_MASK   ((uint32_t)0xFFFFFFFF)
#define GPIO_GPIO_INTSTATMASKED_LSB    0
#define GPIO_GPIO_INTSTATMASKED_WIDTH  ((uint32_t)0x00000020)
#define GPIO_GPIO_INTSTATMASKED_RST    0x0
//! @}  // field

//!
//! @name GPIO_INTACK register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00          GPIO_INTACK   0x0
//! </pre>
//!
#define GPIO_GPIO_INTACK_ADDR   0x00C05020
#define GPIO_GPIO_INTACK_OFFSET 0x00000020
#define GPIO_GPIO_INTACK_INDEX  0x00000008
#define GPIO_GPIO_INTACK_RESET  0x00000000
//! @}  //

//! @name GPIO_INTACK Field Definitions
//! @{
#define GPIO_GPIO_INTACK_MASK   ((uint32_t)0xFFFFFFFF)
#define GPIO_GPIO_INTACK_LSB    0
#define GPIO_GPIO_INTACK_WIDTH  ((uint32_t)0x00000020)
#define GPIO_GPIO_INTACK_RST    0x0
//! @}  // field

//! @}  // group

// ********************************SPIM********************************
//! @defgroup SPIM SPIM
//! @ingroup ambt53
//! @{
#define REG_SPI_DECODING_MASK 0x0000001F

//! @name SPI_CONFIG register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     00           SPI_ENABLE   0
//! </pre>
#define SPI_SPI_CONFIG_ADDR   0x00C07000
#define SPI_SPI_CONFIG_OFFSET 0x00000000
#define SPI_SPI_CONFIG_INDEX  0x00000000
#define SPI_SPI_CONFIG_RESET  0x00000000
//! @}  //

//! @name  SPI_CONFIG Field Definitions
//! @{
#define SPI_SPI_ENABLE_BIT    ((uint32_t)0x00000001)
#define SPI_SPI_ENABLE_POS    0
#define SPI_SPI_ENABLE_RST    0x0
//! @}  // field

//! @name SPI_CLK_DIV register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00              CLK_DIV   0x0
//! </pre>
#define SPI_SPI_CLK_DIV_ADDR   0x00C07004
#define SPI_SPI_CLK_DIV_OFFSET 0x00000004
#define SPI_SPI_CLK_DIV_INDEX  0x00000001
#define SPI_SPI_CLK_DIV_RESET  0x00000000
//! @}  //

//! @name SPI_CLK_DIV Field Definitions
//! @{
#define SPI_CLK_DIV_MASK   ((uint32_t)0x000000FF)
#define SPI_CLK_DIV_LSB    0
#define SPI_CLK_DIV_WIDTH  ((uint32_t)0x00000008)
#define SPI_CLK_DIV_RST    0x0
//! @}  // field

//!
//! @name SPI_TX_DATA register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00              TX_DATA   0x0
//! </pre>
//!
#define SPI_SPI_TX_DATA_ADDR   0x00C07008
#define SPI_SPI_TX_DATA_OFFSET 0x00000008
#define SPI_SPI_TX_DATA_INDEX  0x00000002
#define SPI_SPI_TX_DATA_RESET  0x00000000
//! @}  //

//! @name SPI_TX_DATA Field Definitions
//! @{
#define SPI_TX_DATA_MASK   ((uint32_t)0x000000FF)
#define SPI_TX_DATA_LSB    0
#define SPI_TX_DATA_WIDTH  ((uint32_t)0x00000008)
#define SPI_TX_DATA_RST    0x0
//! @}  // field

//!
//! @name SPI_RX_DATA register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00              RX_DATA   0x0
//! </pre>
//!
#define SPI_SPI_RX_DATA_ADDR   0x00C0700C
#define SPI_SPI_RX_DATA_OFFSET 0x0000000C
#define SPI_SPI_RX_DATA_INDEX  0x00000003
#define SPI_SPI_RX_DATA_RESET  0x00000000
//! @}  //

//! @name SPI_RX_DATA Field Definitions
//! @{
#define SPI_RX_DATA_MASK   ((uint32_t)0x000000FF)
#define SPI_RX_DATA_LSB    0
#define SPI_RX_DATA_WIDTH  ((uint32_t)0x00000008)
#define SPI_RX_DATA_RST    0x0
//! @}  // field

//!
//! @name SPI_STATUS register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     06      RX_FIFO_OVERRUN   0
//!     05    RX_FIFO_HALF_FULL   0
//!     04    RX_FIFO_NOT_EMPTY   0
//!     02        TX_FIFO_EMPTY   0
//!     01   TX_FIFO_HALF_EMPTY   0
//!     00     TX_FIFO_NOT_FULL   0
//! </pre>
//!
#define SPI_SPI_STATUS_ADDR   0x00C07010
#define SPI_SPI_STATUS_OFFSET 0x00000010
#define SPI_SPI_STATUS_INDEX  0x00000004
#define SPI_SPI_STATUS_RESET  0x00000007
//! @}  //

//! @name SPI_STATUS Field Definitions
//! @{
#define SPI_RX_FIFO_OVERRUN_BIT       ((uint32_t)0x00000040)
#define SPI_RX_FIFO_OVERRUN_POS       6
#define SPI_RX_FIFO_HALF_FULL_BIT     ((uint32_t)0x00000020)
#define SPI_RX_FIFO_HALF_FULL_POS     5
#define SPI_RX_FIFO_NOT_EMPTY_BIT     ((uint32_t)0x00000010)
#define SPI_RX_FIFO_NOT_EMPTY_POS     4
#define SPI_TX_FIFO_EMPTY_BIT         ((uint32_t)0x00000004)
#define SPI_TX_FIFO_EMPTY_POS         2
#define SPI_TX_FIFO_HALF_EMPTY_BIT    ((uint32_t)0x00000002)
#define SPI_TX_FIFO_HALF_EMPTY_POS    1
#define SPI_TX_FIFO_NOT_FULL_BIT      ((uint32_t)0x00000001)
#define SPI_TX_FIFO_NOT_FULL_POS      0

#define SPI_RX_FIFO_OVERRUN_RST       0x0
#define SPI_RX_FIFO_HALF_FULL_RST     0x0
#define SPI_RX_FIFO_NOT_EMPTY_RST     0x0
#define SPI_TX_FIFO_EMPTY_RST         0x0
#define SPI_TX_FIFO_HALF_EMPTY_RST    0x0
#define SPI_TX_FIFO_NOT_FULL_RST      0x0
//! @}  // field

//!
//! @name SPI_MASK register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     06   RX_FIFO_OVERRUN_MASK   0
//!     05   RX_FIFO_HALF_FULL_MASK   0
//!     04   RX_FIFO_NOT_EMPTY_MASK   0
//!     02   TX_FIFO_EMPTY_MASK   0
//!     01   TX_FIFO_HALF_EMPTY_MASK   0
//!     00   TX_FIFO_NOT_FULL_MASK   0
//! </pre>
//!
#define SPI_SPI_MASK_ADDR   0x00C07014
#define SPI_SPI_MASK_OFFSET 0x00000014
#define SPI_SPI_MASK_INDEX  0x00000005
#define SPI_SPI_MASK_RESET  0x00000000
//! @}

//! @name SPI_MASK Field Definitions
//! @{
#define SPI_RX_FIFO_OVERRUN_MASK_BIT       ((uint32_t)0x00000040)
#define SPI_RX_FIFO_OVERRUN_MASK_POS       6
#define SPI_RX_FIFO_HALF_FULL_MASK_BIT     ((uint32_t)0x00000020)
#define SPI_RX_FIFO_HALF_FULL_MASK_POS     5
#define SPI_RX_FIFO_NOT_EMPTY_MASK_BIT     ((uint32_t)0x00000010)
#define SPI_RX_FIFO_NOT_EMPTY_MASK_POS     4
#define SPI_TX_FIFO_EMPTY_MASK_BIT         ((uint32_t)0x00000004)
#define SPI_TX_FIFO_EMPTY_MASK_POS         2
#define SPI_TX_FIFO_HALF_EMPTY_MASK_BIT    ((uint32_t)0x00000002)
#define SPI_TX_FIFO_HALF_EMPTY_MASK_POS    1
#define SPI_TX_FIFO_NOT_FULL_MASK_BIT      ((uint32_t)0x00000001)
#define SPI_TX_FIFO_NOT_FULL_MASK_POS      0

#define SPI_RX_FIFO_OVERRUN_MASK_RST       0x0
#define SPI_RX_FIFO_HALF_FULL_MASK_RST     0x0
#define SPI_RX_FIFO_NOT_EMPTY_MASK_RST     0x0
#define SPI_TX_FIFO_EMPTY_MASK_RST         0x0
#define SPI_TX_FIFO_HALF_EMPTY_MASK_RST    0x0
#define SPI_TX_FIFO_NOT_FULL_MASK_RST      0x0
//! @}  // field
//! @}  // group

// ********************************DMAC********************************
//! @defgroup DMAC DMAC
//! @ingroup ambt53
//! @{

//! @name  DMAC REGISTER DEFINITION
//! @{
#define DMAC_FFSR_MASK                             ((uint32_t)0x0000FFFF)
#define DMAC_ISR_MASK                              ((uint32_t)0xFFFFFFFF)
#define DMAC_RISR_MASK                             ((uint32_t)0xFFFFFFFF)
#define DMAC_ICLR_MASK                             ((uint32_t)0xFFFFFFFF)
#define DMAC_CCFGR1_MASK                           ((uint32_t)0x0007FFFF)
#define DMAC_CCFGR2_MASK                           ((uint32_t)0x000003FF)

//! @}  // DMAC REGISTER DEFINITION
//================
#define REG_DMAC_COMMON_DECODING_MASK 0x0000000F

//!
//! @name FFSR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     15              FFFULL7   0
//!     14             FFEMPTY7   0
//!     13              FFFULL6   0
//!     12             FFEMPTY6   0
//!     11              FFFULL5   0
//!     10             FFEMPTY5   0
//!     09              FFFULL4   0
//!     08             FFEMPTY4   0
//!     07              FFFULL3   0
//!     06             FFEMPTY3   0
//!     05              FFFULL2   0
//!     04             FFEMPTY2   0
//!     03              FFFULL1   0
//!     02             FFEMPTY1   0
//!     01              FFFULL0   0
//!     00             FFEMPTY0   0
//! </pre>
//!
#define DMAC_FFSR_ADDR   0x00C08000
#define DMAC_FFSR_OFFSET 0x00000000
#define DMAC_FFSR_INDEX  0x00000000
#define DMAC_FFSR_RESET  0x00000005
//! @}  //

//! @name FFSR Field Definitions
//! @{
#define DMAC_FFFULL7_BIT     ((uint32_t)0x00008000)
#define DMAC_FFFULL7_POS     15
#define DMAC_FFEMPTY7_BIT    ((uint32_t)0x00004000)
#define DMAC_FFEMPTY7_POS    14
#define DMAC_FFFULL6_BIT     ((uint32_t)0x00002000)
#define DMAC_FFFULL6_POS     13
#define DMAC_FFEMPTY6_BIT    ((uint32_t)0x00001000)
#define DMAC_FFEMPTY6_POS    12
#define DMAC_FFFULL5_BIT     ((uint32_t)0x00000800)
#define DMAC_FFFULL5_POS     11
#define DMAC_FFEMPTY5_BIT    ((uint32_t)0x00000400)
#define DMAC_FFEMPTY5_POS    10
#define DMAC_FFFULL4_BIT     ((uint32_t)0x00000200)
#define DMAC_FFFULL4_POS     9
#define DMAC_FFEMPTY4_BIT    ((uint32_t)0x00000100)
#define DMAC_FFEMPTY4_POS    8
#define DMAC_FFFULL3_BIT     ((uint32_t)0x00000080)
#define DMAC_FFFULL3_POS     7
#define DMAC_FFEMPTY3_BIT    ((uint32_t)0x00000040)
#define DMAC_FFEMPTY3_POS    6
#define DMAC_FFFULL2_BIT     ((uint32_t)0x00000020)
#define DMAC_FFFULL2_POS     5
#define DMAC_FFEMPTY2_BIT    ((uint32_t)0x00000010)
#define DMAC_FFEMPTY2_POS    4
#define DMAC_FFFULL1_BIT     ((uint32_t)0x00000008)
#define DMAC_FFFULL1_POS     3
#define DMAC_FFEMPTY1_BIT    ((uint32_t)0x00000004)
#define DMAC_FFEMPTY1_POS    2
#define DMAC_FFFULL0_BIT     ((uint32_t)0x00000002)
#define DMAC_FFFULL0_POS     1
#define DMAC_FFEMPTY0_BIT    ((uint32_t)0x00000001)
#define DMAC_FFEMPTY0_POS    0

#define DMAC_FFFULL7_RST     0x0
#define DMAC_FFEMPTY7_RST    0x0
#define DMAC_FFFULL6_RST     0x0
#define DMAC_FFEMPTY6_RST    0x0
#define DMAC_FFFULL5_RST     0x0
#define DMAC_FFEMPTY5_RST    0x0
#define DMAC_FFFULL4_RST     0x0
#define DMAC_FFEMPTY4_RST    0x0
#define DMAC_FFFULL3_RST     0x0
#define DMAC_FFEMPTY3_RST    0x0
#define DMAC_FFFULL2_RST     0x0
#define DMAC_FFEMPTY2_RST    0x0
#define DMAC_FFFULL1_RST     0x0
#define DMAC_FFEMPTY1_RST    0x0
#define DMAC_FFFULL0_RST     0x0
#define DMAC_FFEMPTY0_RST    0x0

//! @}  // field

//!
//! @name ISR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:24             DEINTLLI   0x0
//!  23:16             DEINTSRC   0x0
//!  15:08             DEINTDST   0x0
//!  07:00                TCINT   0x0
//! </pre>
//!
#define DMAC_ISR_ADDR   0x00C08004
#define DMAC_ISR_OFFSET 0x00000004
#define DMAC_ISR_INDEX  0x00000001
#define DMAC_ISR_RESET  0x00000000
//! @}  //

//! @name ISR Field Definitions
//! @{
#define DMAC_DEINTLLI_MASK   ((uint32_t)0xFF000000)
#define DMAC_DEINTLLI_LSB    24
#define DMAC_DEINTLLI_WIDTH  ((uint32_t)0x00000008)
#define DMAC_DEINTSRC_MASK   ((uint32_t)0x00FF0000)
#define DMAC_DEINTSRC_LSB    16
#define DMAC_DEINTSRC_WIDTH  ((uint32_t)0x00000008)
#define DMAC_DEINTDST_MASK   ((uint32_t)0x0000FF00)
#define DMAC_DEINTDST_LSB    8
#define DMAC_DEINTDST_WIDTH  ((uint32_t)0x00000008)
#define DMAC_TCINT_MASK      ((uint32_t)0x000000FF)
#define DMAC_TCINT_LSB       0
#define DMAC_TCINT_WIDTH     ((uint32_t)0x00000008)

#define DMAC_DEINTLLI_RST    0x0
#define DMAC_DEINTSRC_RST    0x0
#define DMAC_DEINTDST_RST    0x0
#define DMAC_TCINT_RST       0x0
//! @}  // field

//!
//! @name RISR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:24          RAWDEINTLLI   0x0
//!  23:16          RAWDEINTSRC   0x0
//!  15:08          RAWDEINTDST   0x0
//!  07:00             RAWTCINT   0x0
//! </pre>
//!
#define DMAC_RISR_ADDR   0x00C08008
#define DMAC_RISR_OFFSET 0x00000008
#define DMAC_RISR_INDEX  0x00000002
#define DMAC_RISR_RESET  0x00000000
//! @}  //

//! @name RISR Field Definitions
//! @{
#define DMAC_RAWDEINTLLI_MASK   ((uint32_t)0xFF000000)
#define DMAC_RAWDEINTLLI_LSB    24
#define DMAC_RAWDEINTLLI_WIDTH  ((uint32_t)0x00000008)
#define DMAC_RAWDEINTSRC_MASK   ((uint32_t)0x00FF0000)
#define DMAC_RAWDEINTSRC_LSB    16
#define DMAC_RAWDEINTSRC_WIDTH  ((uint32_t)0x00000008)
#define DMAC_RAWDEINTDST_MASK   ((uint32_t)0x0000FF00)
#define DMAC_RAWDEINTDST_LSB    8
#define DMAC_RAWDEINTDST_WIDTH  ((uint32_t)0x00000008)
#define DMAC_RAWTCINT_MASK      ((uint32_t)0x000000FF)
#define DMAC_RAWTCINT_LSB       0
#define DMAC_RAWTCINT_WIDTH     ((uint32_t)0x00000008)

#define DMAC_RAWDEINTLLI_RST    0x0
#define DMAC_RAWDEINTSRC_RST    0x0
#define DMAC_RAWDEINTDST_RST    0x0
#define DMAC_RAWTCINT_RST       0x0
//! @}  //

//!
//! @name ICLR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:24          CLRDEINTLLI   0x0
//!  23:16          CLRDEINTSRC   0x0
//!  15:08          CLRDEINTDST   0x0
//!  07:00             CLRTCINT   0x0
//! </pre>
//!
#define DMAC_ICLR_ADDR   0x00C0800C
#define DMAC_ICLR_OFFSET 0x0000000C
#define DMAC_ICLR_INDEX  0x00000003
#define DMAC_ICLR_RESET  0x00000000
//! @}  //

//! @name ICLR Field Definitions
//! @{
#define DMAC_CLRDEINTLLI_MASK   ((uint32_t)0xFF000000)
#define DMAC_CLRDEINTLLI_LSB    24
#define DMAC_CLRDEINTLLI_WIDTH  ((uint32_t)0x00000008)
#define DMAC_CLRDEINTSRC_MASK   ((uint32_t)0x00FF0000)
#define DMAC_CLRDEINTSRC_LSB    16
#define DMAC_CLRDEINTSRC_WIDTH  ((uint32_t)0x00000008)
#define DMAC_CLRDEINTDST_MASK   ((uint32_t)0x0000FF00)
#define DMAC_CLRDEINTDST_LSB    8
#define DMAC_CLRDEINTDST_WIDTH  ((uint32_t)0x00000008)
#define DMAC_CLRTCINT_MASK      ((uint32_t)0x000000FF)
#define DMAC_CLRTCINT_LSB       0
#define DMAC_CLRTCINT_WIDTH     ((uint32_t)0x00000008)

#define DMAC_CLRDEINTLLI_RST    0x0
#define DMAC_CLRDEINTSRC_RST    0x0
#define DMAC_CLRDEINTDST_RST    0x0
#define DMAC_CLRTCINT_RST       0x0

#define REG_DMAC_CHANNEL_DECODING_MASK 0x0000001F
//! @}  // field

//!
//! @name SAR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00                  SAR   0x0
//! </pre>
//!
#define DMAC_SAR_ADDR   0x00C08100
#define DMAC_SAR_OFFSET 0x00000000
#define DMAC_SAR_INDEX  0x00000000
#define DMAC_SAR_RESET  0x00000000
//! @}  //

//! @name SAR Field Definitions
//! @{
#define DMAC_SAR_MASK   ((uint32_t)0xFFFFFFFF)
#define DMAC_SAR_LSB    0
#define DMAC_SAR_WIDTH  ((uint32_t)0x00000020)
#define DMAC_SAR_RST    0x0
//! @}  // field

//!
//! @name DAR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00                  DAR   0x0
//! </pre>
//!
#define DMAC_DAR_ADDR   0x00C08104
#define DMAC_DAR_OFFSET 0x00000004
#define DMAC_DAR_INDEX  0x00000001
#define DMAC_DAR_RESET  0x00000000
//! @}  //

//! @name DAR Field Definitions
//! @{
#define DMAC_DAR_MASK   ((uint32_t)0xFFFFFFFF)
#define DMAC_DAR_LSB    0
#define DMAC_DAR_WIDTH  ((uint32_t)0x00000020)

#define DMAC_DAR_RST    0x0
//! @}  //

//! @name CCFGR1 register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:19          BLKXFERSIZE   0x0
//!     18            BUSLCKSRC   0
//!     17            BUSLCKDST   0
//!     16                CHLCK   0
//!  15:13          BRSTSIZESRC   0x0
//!  12:10          BRSTSIZEDST   0x0
//!  09:08         SRCXFERWIDTH   0x0
//!  07:06         DSTXFERWIDTH   0x0
//!     05           MSTRSELSRC   0
//!     04           MSTRSELDST   0
//!     03            ADDRRISRC   0
//!     02            ADDRRIDST   0
//!     01           ADDRINCSRC   0
//!     00           ADDRINCDST   0
//! </pre>
#define DMAC_CCFGR1_ADDR   0x00C08108
#define DMAC_CCFGR1_OFFSET 0x00000008
#define DMAC_CCFGR1_INDEX  0x00000002
#define DMAC_CCFGR1_RESET  0x00000000
//! @}  //

//! @name CCFGR1 Field Definitions
//! @{
#define DMAC_BLKXFERSIZE_MASK    ((uint32_t)0xFFF80000)
#define DMAC_BLKXFERSIZE_LSB     19
#define DMAC_BLKXFERSIZE_WIDTH   ((uint32_t)0x0000000D)
#define DMAC_BUSLCKSRC_BIT       ((uint32_t)0x00040000)
#define DMAC_BUSLCKSRC_POS       18
#define DMAC_BUSLCKDST_BIT       ((uint32_t)0x00020000)
#define DMAC_BUSLCKDST_POS       17
#define DMAC_CHLCK_BIT           ((uint32_t)0x00010000)
#define DMAC_CHLCK_POS           16
#define DMAC_BRSTSIZESRC_MASK    ((uint32_t)0x0000E000)
#define DMAC_BRSTSIZESRC_LSB     13
#define DMAC_BRSTSIZESRC_WIDTH   ((uint32_t)0x00000003)
#define DMAC_BRSTSIZEDST_MASK    ((uint32_t)0x00001C00)
#define DMAC_BRSTSIZEDST_LSB     10
#define DMAC_BRSTSIZEDST_WIDTH   ((uint32_t)0x00000003)
#define DMAC_SRCXFERWIDTH_MASK   ((uint32_t)0x00000300)
#define DMAC_SRCXFERWIDTH_LSB    8
#define DMAC_SRCXFERWIDTH_WIDTH  ((uint32_t)0x00000002)
#define DMAC_DSTXFERWIDTH_MASK   ((uint32_t)0x000000C0)
#define DMAC_DSTXFERWIDTH_LSB    6
#define DMAC_DSTXFERWIDTH_WIDTH  ((uint32_t)0x00000002)
#define DMAC_MSTRSELSRC_BIT      ((uint32_t)0x00000020)
#define DMAC_MSTRSELSRC_POS      5
#define DMAC_MSTRSELDST_BIT      ((uint32_t)0x00000010)
#define DMAC_MSTRSELDST_POS      4
#define DMAC_ADDRRISRC_BIT       ((uint32_t)0x00000008)
#define DMAC_ADDRRISRC_POS       3
#define DMAC_ADDRRIDST_BIT       ((uint32_t)0x00000004)
#define DMAC_ADDRRIDST_POS       2
#define DMAC_ADDRINCSRC_BIT      ((uint32_t)0x00000002)
#define DMAC_ADDRINCSRC_POS      1
#define DMAC_ADDRINCDST_BIT      ((uint32_t)0x00000001)
#define DMAC_ADDRINCDST_POS      0

#define DMAC_BLKXFERSIZE_RST     0x0
#define DMAC_BUSLCKSRC_RST       0x0
#define DMAC_BUSLCKDST_RST       0x0
#define DMAC_CHLCK_RST           0x0
#define DMAC_BRSTSIZESRC_RST     0x0
#define DMAC_BRSTSIZEDST_RST     0x0
#define DMAC_SRCXFERWIDTH_RST    0x0
#define DMAC_DSTXFERWIDTH_RST    0x0
#define DMAC_MSTRSELSRC_RST      0x0
#define DMAC_MSTRSELDST_RST      0x0
#define DMAC_ADDRRISRC_RST       0x0
#define DMAC_ADDRRIDST_RST       0x0
#define DMAC_ADDRINCSRC_RST      0x0
#define DMAC_ADDRINCDST_RST      0x0
//! @}  // field

//!
//! @name CCFGR2 register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     09           MSTSEL_LLI   0
//!     08               MEMSET   0
//!     07               TCMASK   0
//!     06               DEMASK   0
//!     05         SWBRSTREQSRC   0
//!     04         SWBRSTREQDST   0
//!     03          SWSGLREQSRC   0
//!     02          SWSGLREQDST   0
//!     01           HWREQSRCEN   0
//!     00           HWREQDSTEN   0
//! </pre>
//!
#define DMAC_CCFGR2_ADDR   0x00C0810C
#define DMAC_CCFGR2_OFFSET 0x0000000C
#define DMAC_CCFGR2_INDEX  0x00000003
#define DMAC_CCFGR2_RESET  0x00000000
//! @}  //

//! @name CCFGR2 Field Definitions
//! @{
#define DMAC_MSTSEL_LLI_BIT      ((uint32_t)0x00000200)
#define DMAC_MSTSEL_LLI_POS      9
#define DMAC_MEMSET_BIT          ((uint32_t)0x00000100)
#define DMAC_MEMSET_POS          8
#define DMAC_TCMASK_BIT          ((uint32_t)0x00000080)
#define DMAC_TCMASK_POS          7
#define DMAC_DEMASK_BIT          ((uint32_t)0x00000040)
#define DMAC_DEMASK_POS          6
#define DMAC_SWBRSTREQSRC_BIT    ((uint32_t)0x00000020)
#define DMAC_SWBRSTREQSRC_POS    5
#define DMAC_SWBRSTREQDST_BIT    ((uint32_t)0x00000010)
#define DMAC_SWBRSTREQDST_POS    4
#define DMAC_SWSGLREQSRC_BIT     ((uint32_t)0x00000008)
#define DMAC_SWSGLREQSRC_POS     3
#define DMAC_SWSGLREQDST_BIT     ((uint32_t)0x00000004)
#define DMAC_SWSGLREQDST_POS     2
#define DMAC_HWREQSRCEN_BIT      ((uint32_t)0x00000002)
#define DMAC_HWREQSRCEN_POS      1
#define DMAC_HWREQDSTEN_BIT      ((uint32_t)0x00000001)
#define DMAC_HWREQDSTEN_POS      0

#define DMAC_MSTSEL_LLI_RST      0x0
#define DMAC_MEMSET_RST          0x0
#define DMAC_TCMASK_RST          0x0
#define DMAC_DEMASK_RST          0x0
#define DMAC_SWBRSTREQSRC_RST    0x0
#define DMAC_SWBRSTREQDST_RST    0x0
#define DMAC_SWSGLREQSRC_RST     0x0
#define DMAC_SWSGLREQDST_RST     0x0
#define DMAC_HWREQSRCEN_RST      0x0
#define DMAC_HWREQDSTEN_RST      0x0
//! @}  // field

//!
//! @name LLPTR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:00                LLPTR   0x0
//! </pre>
//!
#define DMAC_LLPTR_ADDR   0x00C08110
#define DMAC_LLPTR_OFFSET 0x00000010
#define DMAC_LLPTR_INDEX  0x00000004
#define DMAC_LLPTR_RESET  0x00000000
//! @}  //

//! @name LLPTR Field Definitions
//! @{
#define DMAC_LLPTR_MASK   ((uint32_t)0xFFFFFFFF)
#define DMAC_LLPTR_LSB    0
#define DMAC_LLPTR_WIDTH  ((uint32_t)0x00000020)
#define DMAC_LLPTR_RST    0x0
//! @}  // field

//!
//! @name FFLVLR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  15:00                FFLVL   0x0
//! </pre>
//!
#define DMAC_FFLVLR_ADDR   0x00C08114
#define DMAC_FFLVLR_OFFSET 0x00000014
#define DMAC_FFLVLR_INDEX  0x00000005
#define DMAC_FFLVLR_RESET  0x00000000
//! @}  //

//! @name FFLVLR Field Definitions
//! @{
#define DMAC_FFLVL_MASK   ((uint32_t)0x0000FFFF)
#define DMAC_FFLVL_LSB    0
#define DMAC_FFLVL_WIDTH  ((uint32_t)0x00000010)
#define DMAC_FFLVL_RST    0x0
//! @}  //

//!
//! @name XFRCNTR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  12:00               XFRCNT   0x0
//! </pre>
//!
#define DMAC_XFRCNTR_ADDR   0x00C08118
#define DMAC_XFRCNTR_OFFSET 0x00000018
#define DMAC_XFRCNTR_INDEX  0x00000006
#define DMAC_XFRCNTR_RESET  0x00000000
//! @}  //

//! @name XFRCNTR Field Definitions
//! @{
#define DMAC_XFRCNT_MASK   ((uint32_t)0x00001FFF)
#define DMAC_XFRCNT_LSB    0
#define DMAC_XFRCNT_WIDTH  ((uint32_t)0x0000000D)
#define DMAC_XFRCNT_RST    0x0
//! @}  // field
//! @}  // group

// ********************************XDMA*******************************
//! @defgroup XDMA XDMA
//! @ingroup ambt53
//! @{
//! Base address of the DMA
#define DMA_BASE_ADDR           (0x400000) // VLSI_PL
//#define DMA_BASE_ADDR         (0x80400000)
//! @name Register Addresses
//! @{
#define GCS_CHSTS_ADDR          (DMA_BASE_ADDR + 0)
#define GCS_CHCTRL_ADDR         (DMA_BASE_ADDR + 4)
#define GCS_SEOBC_ADDR          (DMA_BASE_ADDR + 8)
#define GCS_SEOF_ADDR           (DMA_BASE_ADDR + 0xC)
#define GCS_ERR_ADDR            (DMA_BASE_ADDR + 0x10)
#define GCS_PRIOR0_ADDR         (DMA_BASE_ADDR + 0x14)
#define GCS_PRIOR1_ADDR         (DMA_BASE_ADDR + 0x18)
#define GCS_CFG_ADDR            (DMA_BASE_ADDR + 0x1C)
#define GCS_STS_ADDR            (DMA_BASE_ADDR + 0x20)
#define GCS_DMBP_ADDR           (DMA_BASE_ADDR + 0x24)
#define GCS_SAMBP_ADDR          (DMA_BASE_ADDR + 0x28)
#define GCS_DAMBP_ADDR          (DMA_BASE_ADDR + 0x2C)
#define GCS_DMBA_ADDR           (DMA_BASE_ADDR + 0x30)
#define GCS_GCS_DMC_ADDR        (DMA_BASE_ADDR + 0x34)
#define GCS_DPRC_ADDR           (DMA_BASE_ADDR + 0x38)
//! @}

//! @name Register ch x is the channel number (start from ch - 0 )
//! @{
#define DTC_CTRL_ADDR(x)        (DMA_BASE_ADDR + 0x80 * (x + 1))
#define DTC_SSA_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 4)
#define DTC_DSA_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 8)
#define DTC_CBEN_ADDR(x)        (DMA_BASE_ADDR + 0x80 * (x + 1) + 0xC)
#define DTC_EPM_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x10)
#define DTC_CFG_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x14)
#define DTC_FCN_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x18)
#define DTC_BPM_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x1C)
#define DTC_SCPM_ADDR(x)        (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x20)
#define DTC_DCPM_ADDR(x)        (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x24)
#define DTC_SCA_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x28)
#define DTC_DCA_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x2C)
#define DTC_CEC_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x30)
#define DTC_CCC_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x34)
#define DTC_SET_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x38)
#define DTC_RST_ADDR(x)         (DMA_BASE_ADDR + 0x80 * (x + 1) + 0x3C)
//! @}

#define GCS_CHSTS_MASK                              ((uint32_t)0xFFFFFFFF)
#define GCS_CHSTS_RESET                             0x00000000

#define GCS_CHCTRL_MASK                             ((uint32_t)0x000F000F)
#define GCS_CHCTRL_RESET                            0x00000000

#define GCS_SEOBC_RD_MASK                           ((uint32_t)0x00070007)
#define GCS_SEOBC_RESET                             0x00000000

#define GCS_SEOF_RD_MASK                            ((uint32_t)0x0000FFFF)
#define GCS_SEOF_RESET                              0x00000000

#define GCS_ERR_MASK                                ((uint32_t)0xFFFFFFFF)
#define GCS_ERR_RESET                               0x00000000

#define GCS_PRIOR0_MASK                             ((uint32_t)0x0000FFFF)
#define GCS_PRIOR0_RESET                            0x00003210

#define GCS_PRIOR1_MASK                             ((uint32_t)0xFFFFFFFF)
#define GCS_PRIOR1_RESET                            0x00000000

#define GCS_CFG_MASK                               ((uint32_t)0x00001FFF)
#define GCS_CFG_RESET                              0x00000000

#define GCS_STS_MASK_RD                            ((uint32_t)0x0000000F)
#define GCS_STS_RESET                              0x00000008

#define GCS_DMBP_MASK                              ((uint32_t)0xFFFFFFFF)
#define GCS_DMBP_RESET                             0x00000000

#define GCS_SAMBP_MASK                             ((uint32_t)0xFFFFFFFF)
#define GCS_SAMBP_RESET                            0x00000000

#define GCS_DAMBP_MASK                             ((uint32_t)0xFFFFFFFF)
#define GCS_DAMBP_RESET                            0x00000000

#define GCS_DMBA_MASK                              ((uint32_t)0xFFFFFFFC)
#define GCS_DMBA_RESET                             0x00000000

#define GCS_GCS_DMC_MASK                           ((uint32_t)0x0003FFFF)
#define GCS_GCS_DMC_RESET                          0x00000000

#define GCS_DPRC_RO_MASK                           ((uint32_t)0x00FF00FF)
#define GCS_DPRC_RESET                             0x00000000

#define DTC_CTRL_MASK                              ((uint32_t)0x007FFFFF)
#define DTC_CTRL_RESET                             0x00000008

#define DTC_SSA_MASK                               ((uint32_t)0xFFFFFFFF)
#define DTC_SSA_RESET                              0x00000000

#define DTC_DSA_MASK                               ((uint32_t)0xFFFFFFFF)
#define DTC_DSA_RESET                              0x00000000

#define DTC_CBEN_MASK                              ((uint32_t)0xFFFFFFFF)
#define DTC_CBEN_RESET                             0x00010001

#define DTC_EPM_MASK                               ((uint32_t)0xFFFFFFFF)
#define DTC_EPM_RESET                              0x00000000

#define DTC_CFG_MASK                               ((uint32_t)0x80ED80ED)
#define DTC_CFG_RESET                              0x00000000

#define DTC_FCN_MASK                               ((uint32_t)0xFFFFFFFF)
#define DTC_FCN_RESET                              0x00000001

#define DTC_BPM_MASK                               ((uint32_t)0xFFFFFFFF)
#define DTC_BPM_RESET                              0x00000000

#define DTC_SCPM_MASK                              ((uint32_t)0xFFFFFFFF)
#define DTC_SCPM_RESET                             0x00000000

#define DTC_DCPM_MASK                              ((uint32_t)0xFFFFFFFF)
#define DTC_DCPM_RESET                             0x00000000

#define DTC_SCA_MASK                               ((uint32_t)0xFFFFFFFF)
#define DTC_SCA_RESET                              0x00000000

#define DTC_DCA_MASK                               ((uint32_t)0xFFFFFFFF)
#define DTC_DCA_RESET                              0x00000000

#define DTC_CEC_MASK                               ((uint32_t)0x0000FFFF)
#define DTC_CEC_RESET                              0x00000000

#define DTC_CCC_MASK                               ((uint32_t)0xFFFFFFFF)
#define DTC_CCC_RESET                              0x00000000

#define DTC_SET_MASK                               ((uint32_t)0xFFFFFFFF)
#define DTC_SET_RESET                              0x00000000

#define DTC_RST_MASK                               ((uint32_t)0xFFFFFFFF)
#define DTC_RST_RESET                              0x00000000

//! @} // XDMA group

// *******************************I2C********************************
//! @defgroup I2C I2C
//! @ingroup ambt53
//! @{

//!
//! @name  I2C_DATA register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00             I2C_DATA   0
//! </pre>
//!
#define I2C_DATA_ADDR   0x00C0D000
#define I2C_DATA_OFFSET 0x00000000
#define I2C_DATA_INDEX  0x00000000
#define I2C_DATA_RESET  0x00000000
//! @}  //

//! @name I2C_DATA Field Definitions
//! @{
#define I2C_DATA_MASK   ((uint32_t)0x000000FF)
#define I2C_DATA_LSB    0
#define I2C_DATA_WIDTH  ((uint32_t)0x00000008)

#define I2C_DATA_RST    0x0
//! @}  // field

//!
//! @name I2C_SLAD0 register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00            I2C_SLAD0   0x0
//! </pre>
//!
#define I2C_SLAD0_ADDR   0x00C0D004
#define I2C_SLAD0_OFFSET 0x00000004
#define I2C_SLAD0_INDEX  0x00000001
#define I2C_SLAD0_RESET  0x0000005A
//! @}  //

//! @name I2C_SLAD0 Field Definitions
//! @{
#define I2C_SLAD0_MASK   ((uint32_t)0x000000FE)
#define I2C_SLAD0_LSB    0
#define I2C_SLAD0_WIDTH  ((uint32_t)0x00000007)

#define I2C_SLAD0_RST    0x0
//! @}  // field

//!
//! @name I2C_SLAD1 register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00            I2C_SLAD1   0x0
//! </pre>
//!
#define I2C_SLAD1_ADDR   0x00C0D008
#define I2C_SLAD1_OFFSET 0x00000008
#define I2C_SLAD1_INDEX  0x00000002
#define I2C_SLAD1_RESET  0x00000001
//! @}  //

//! @name I2C_SLAD1 Field Definitions
//! @{
#define I2C_SLAD1_MASK   ((uint32_t)0x00000003)
#define I2C_SLAD1_LSB    0
#define I2C_SLAD1_WIDTH  ((uint32_t)0x00000008)

#define I2C_SLAD1_RST    0x0
//! @}  //

//!
//! @name I2C_CNTRL register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     07              TST_MOD   0
//!     06        REP_START_EN    0
//!     05            REPN_STOP   0
//!     04                 SRST   0
//!     03                  SWE   0
//!     02               AD_MOD   0
//!     01                RDNWR   0
//!     00                  MNS   0
//! </pre>
//!
#define I2C_CNTRL_ADDR   0x00C0D00C
#define I2C_CNTRL_OFFSET 0x0000000C
#define I2C_CNTRL_INDEX  0x00000003
#define I2C_CNTRL_RESET  0x00000010
//! @}  //

//! @name I2C_CNTRL Field Definitions
//! @{
#define I2C_CNTRL_TST_MOD_BIT            ((uint32_t)0x00000080)
#define I2C_CNTRL_TST_MOD_POS             7
#define I2C_CNTRL_REP_START_EN_BIT       ((uint32_t)0x00000040)
#define I2C_CNTRL_REP_START_EN_POS        6
#define I2C_CNTRL_REPN_STOP_BIT          ((uint32_t)0x00000020)
#define I2C_CNTRL_REPN_STOP_POS           5
#define I2C_CNTRL_SRST_BIT               ((uint32_t)0x00000010)
#define I2C_CNTRL_SRST_POS                4
#define I2C_CNTRL_SWE_BIT                ((uint32_t)0x00000008)
#define I2C_CNTRL_SWE_POS                 3
#define I2C_CNTRL_AD_MOD_BIT             ((uint32_t)0x00000004)
#define I2C_CNTRL_AD_MOD_POS              2
#define I2C_CNTRL_RDNWR_BIT              ((uint32_t)0x00000002)
#define I2C_CNTRL_RDNWR_POS               1
#define I2C_CNTRL_MNS_BIT                ((uint32_t)0x00000001)
#define I2C_CNTRL_MNS_POS                 0

#define I2C_CNTRL_TST_MOD_RST             0x0
#define I2C_CNTRL_REP_START_EN_RST        0x0
#define I2C_CNTRL_REPN_STOP_RST           0x0
#define I2C_CNTRL_SRST_RST                0x0
#define I2C_CNTRL_SWE_RST                 0x0
#define I2C_CNTRL_AD_MOD_RST              0x0
#define I2C_CNTRL_RDNWR_RST               0x0
#define I2C_CNTRL_MNS_RST                 0x0
//! @}  // field

//!
//! @name I2C_DAT_CNT register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00            I2CDATCNT    0x0
//! </pre>
//!
#define I2C_DAT_CNT_ADDR     0x00C0D010
#define I2C_DAT_CNT_OFFSET   0x00000010
#define I2C_DAT_CNT_INDEX    0x00000004
#define I2C_DAT_CNT_RESET    0x00000000
//! @}  //

//! @name I2C_DAT_CNT Field Definitions
//! @{
#define I2C_DAT_CNT_MASK   ((uint32_t)0x000000FF)
#define I2C_DAT_CNT_LSB    0
#define I2C_DAT_CNT_WIDTH  ((uint32_t)0x00000008)

#define I2C_DAT_CNT_RST    0x0
//! @}  // field

//!
//! @name I2C_SE_AD0 register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  06:00             I2CSEAD0    0x0
//! </pre>
//!
#define I2C_SE_AD0_ADDR      0x00C0D014
#define I2C_SE_AD0_OFFSET    0x00000014
#define I2C_SE_AD0_INDEX     0x00000005
#define I2C_SE_AD0_RESET     0x00000060
//! @}  //

//! @name I2C_SE_AD0 Field Definitions
//! @{
#define I2C_SE_AD0_MASK   ((uint32_t)0x00000FE)
#define I2C_SE_AD0_LSB    0
#define I2C_SE_AD0_WIDTH  ((uint32_t)0x00000007)
#define I2C_SE_AD0_RST    0x0
//! @}  // field

//!
//! @name I2C_SE_AD1 register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  06:00             I2CSEAD1    0x0
//! </pre>
//!
#define I2C_SE_AD1_ADDR      0x00C0D018
#define I2C_SE_AD1_OFFSET    0x00000018
#define I2C_SE_AD1_INDEX     0x00000006
#define I2C_SE_AD1_RESET     0x00000002
//! @}  //

//! @name I2C_SE_AD1 Field Definitions
//! @{
#define I2C_SE_AD1_MASK   ((uint32_t)0x0000007)
#define I2C_SE_AD1_LSB    0
#define I2C_SE_AD1_WIDTH  ((uint32_t)0x00000007)

#define I2C_SE_AD1_RST    0x2
//! @}  // field

//!
//! @name I2C_PSCR0 register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00             I2CPSCR0    0x0
//! </pre>
//!
#define I2C_PSCR0_ADDR      0x00C0D01C
#define I2C_PSCR0_OFFSET    0x0000001C
#define I2C_PSCR0_INDEX     0x00000007
#define I2C_PSCR0_RESET     0x000000F4
//! @}  //

//! @name I2C_PSCR0 Field Definitions
//! @{
#define I2C_PSCR0_MASK   ((uint32_t)0x00000FF)
#define I2C_PSCR0_LSB    0
#define I2C_PSCR0_WIDTH  ((uint32_t)0x00000008)

#define I2C_PSCR0_RST    0x0
//! @}  // field

//!
//! @name I2C_PSCR1 register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00             I2CPSCR1   0x0
//! </pre>
//!
#define I2C_PSCR1_ADDR      0x00C0D020
#define I2C_PSCR1_OFFSET    0x00000020
#define I2C_PSCR1_INDEX     0x00000008
#define I2C_PSCR1_RESET     0x00000000
//! @}  //

//! @name I2C_PSCR1 Field Definitions
//! @{
#define I2C_PSCR1_MASK   ((uint32_t)0x0000003)
#define I2C_PSCR1_LSB    0
#define I2C_PSCR1_WIDTH  ((uint32_t)0x00000008)

#define I2C_PSCR1_RST    0x0
//! @}  // field

//!
//! @name I2C_IMR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     07          I2CBRKIRQEN   0
//!     06             STXIRQEN   0
//!     05             M2SIRQEN   0
//!     04            RXORIRQEN   0
//!     03            RXFFULLEN   0
//!     02             TXMPTYEN   0
//!     01        RXFNOTEMPTYEN   0
//!     00         TXFNOTFULLEN   0
//! </pre>
//!
#define I2C_IMR_ADDR      0x00C0D024
#define I2C_IMR_OFFSET    0x00000024
#define I2C_IMR_INDEX     0x00000009
#define I2C_IMR_RESET     0x00000000
//! @}  //

//! @name I2C_IMR Field Definitions
//! @{
#define I2C_IMR_I2CBRKIRQEN_BIT            ((uint32_t)0x00000080)
#define I2C_IMR_I2CBRKIRQEN_POS             7
#define I2C_IMR_STXIRQEN_BIT               ((uint32_t)0x00000040)
#define I2C_IMR_STXIRQEN_EN_POS             6
#define I2C_IMR_M2SIRQEN_BIT               ((uint32_t)0x00000020)
#define I2C_IMR_M2SIRQEN_POS                5
#define I2C_IMR_RXORIRQEN_BIT              ((uint32_t)0x00000010)
#define I2C_IMR_RXORIRQEN_POS               4
#define I2C_IMR_RXFFULLEN_BIT              ((uint32_t)0x00000008)
#define I2C_IMR_RXFFULLEN_POS               3
#define I2C_IMR_TXMPTYEN_BIT               ((uint32_t)0x00000004)
#define I2C_IMR_TXMPTYEN_POS                2
#define I2C_IMR_RXFNOTEMPTYEN_BIT          ((uint32_t)0x00000002)
#define I2C_IMR_RXFNOTEMPTYEN_POS           1
#define I2C_IMR_TXFNOTFULLEN_BIT           ((uint32_t)0x00000001)
#define I2C_IMR_TXFNOTFULLEN_POS            0

#define I2C_IMR_I2CBRKIRQEN _RST            0x0
#define I2C_IMR_STXIRQEN_RST                0x0
#define I2C_IMR_M2SIRQEN_RST                0x0
#define I2C_IMR_RXORIRQEN_RST               0x0
#define I2C_IMR_RXFFULLEN_RST               0x0
#define I2C_IMR_TXMPTYEN_RST                0x0
#define I2C_IMR_RXFNOTEMPTYEN_RST           0x0
#define I2C_IMR_TXFNOTFULLEN_RST            0x0
//! @}  // field

//!
//! @name I2C_RIS register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!    07      I2CBRKIRQRAWSTAT   0
//!    06         STXIRQRAWSTAT   0
//!    05         M2SIRQRAWSTAT   0
//!    04         RXORIRQAWSTAT   0
//!    03        RXFFULLRAWSTAT   0
//!    02         TXMPTYRAWSTAT   0
//!    01    RXFNOTEMPTYRAWSTAT   0
//!    00     TXFNOTFULLRAWSTAT   0
//! </pre>
//!
#define I2C_RIS_ADDR      0x00C0D028
#define I2C_RIS_OFFSET    0x00000028
#define I2C_RIS_INDEX     0x0000000A
#define I2C_RIS_RESET     0x00000005
//! @}  //

//! @name I2C_RIS Field Definitions
//! @{
#define I2C_RIS_I2CBRKIRQ_BIT            ((uint32_t)0x00000080)
#define I2C_RIS_I2CBRKIRQ_POS             7
#define I2C_RIS_STXIRQ_BIT               ((uint32_t)0x00000040)
#define I2C_RIS_STXIRQ_POS                6
#define I2C_RIS_M2SIRQ_BIT               ((uint32_t)0x00000020)
#define I2C_RIS_M2SIRQ_POS                5
#define I2C_RIS_RXORIRQ_BIT              ((uint32_t)0x00000010)
#define I2C_RIS_RXORIRQ_POS               4
#define I2C_RIS_RXFFULL_BIT              ((uint32_t)0x00000008)
#define I2C_RIS_RXFFULL_POS               3
#define I2C_RIS_TXMPTY_BIT               ((uint32_t)0x00000004)
#define I2C_RIS_TXMPTY_POS                2
#define I2C_RIS_RXFNOTEMPTY_BIT          ((uint32_t)0x00000002)
#define I2C_RIS_RXFNOTEMPTY_POS           1
#define I2C_RIS_TXFNOTFULL_BIT           ((uint32_t)0x00000001)
#define I2C_RIS_TXFNOTFULL_POS            0

#define I2C_RIS_I2CBRKIRQ_RST             0x0
#define I2C_RIS_STXIRQ_RST                0x0
#define I2C_RIS_M2SIRQ_RST                0x0
#define I2C_RIS_RXORIRQ_RST               0x0
#define I2C_RIS_RXFFULL_RST               0x0
#define I2C_RIS_TXMPTY_RST                0x0
#define I2C_RIS_RXFNOTEMPTY_RST           0x0
#define I2C_RIS_TXFNOTFULL_RST            0x0
//! @}  // field

//!
//! @name I2C_CIS register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!    07      I2CBRKIRQRAWSTAT   0
//!    06         STXIRQRAWSTAT   0
//!    05         M2SIRQRAWSTAT   0
//!    04         RXORIRQAWSTAT   0
//!    03        RXFFULLRAWSTAT   0
//!    02         TXMPTYRAWSTAT   0
//!    01    RXFNOTEMPTYRAWSTAT   0
//!    00     TXFNOTFULLRAWSTAT   0
//! </pre>
//!
#define I2C_CIS_ADDR      0x00C0D02C
#define I2C_CIS_OFFSET    0x0000002C
#define I2C_CIS_INDEX     0x0000000B
#define I2C_CIS_RESET     0x00000000
//! @}

//! @name I2C_CIS Field Definitions
//! @{
#define I2C_CIS_I2CBRKIRQEN_BIT            ((uint32_t)0x00000080)
#define I2C_CIS_I2CBRKIRQEN_POS             7
#define I2C_CIS_STXIRQEN_BIT               ((uint32_t)0x00000040)
#define I2C_CIS_STXIRQEN_EN_POS             6
#define I2C_CIS_M2SIRQEN_BIT               ((uint32_t)0x00000020)
#define I2C_CIS_M2SIRQEN_POS                5
#define I2C_CIS_RXORIRQEN_BIT              ((uint32_t)0x00000010)
#define I2C_CIS_RXORIRQEN_POS               4
#define I2C_CIS_RXFFULLEN_BIT              ((uint32_t)0x00000008)
#define I2C_CIS_RXFFULLEN_POS               3
#define I2C_CIS_TXMPTYEN_BIT               ((uint32_t)0x00000004)
#define I2C_CIS_TXMPTYEN_POS                2
#define I2C_CIS_RXFNOTEMPTYEN_BIT          ((uint32_t)0x00000002)
#define I2C_CIS_RXFNOTEMPTYEN_POS           1
#define I2C_CIS_TXFNOTFULLEN_BIT           ((uint32_t)0x00000001)
#define I2C_CIS_TXFNOTFULLEN_POS            0

#define I2C_CIS_I2CBRKIRQEN _RST            0x0
#define I2C_CIS_STXIRQEN_RST                0x0
#define I2C_CIS_M2SIRQEN_RST                0x0
#define I2C_CIS_RXORIRQEN_RST               0x0
#define I2C_CIS_RXFFULLEN_RST               0x0
#define I2C_CIS_TXMPTYEN_RST                0x0
#define I2C_CIS_RXFNOTEMPTYEN_RST           0x0
#define I2C_CIS_TXFNOTFULLEN_RST            0x0
//! @}  // field

//!
//! @name I2C_CSR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!    02               ACTMAST   0
//!    01               BUSBUSY   0
//!    00               DEVBUSY   0
//! </pre>
//!
#define I2C_CSR_ADDR      0x00C0D030
#define I2C_CSR_OFFSET    0x00000030
#define I2C_CSR_INDEX     0x0000000C
#define I2C_CSR_RESET     0x00000000
//! @}  //

//! @name I2C_CSR Field Definitions
//! @{
#define I2C_CSR_ACTMAST_BIT           ((uint32_t)0x00000004)
#define I2C_CSR_ACTMAST_POS            2
#define I2C_CSR_BUSBUSY_BIT           ((uint32_t)0x00000002)
#define I2C_CSR_BUSBUSY_POS            1
#define I2C_CSR_DEVBUSY_BIT           ((uint32_t)0x00000001)
#define I2C_CSR_DEVBUSY_POS            0

#define I2C_CSR_ACTMAST_RST            0x0
#define I2C_CSR_BUSBUSY_RST            0x0
#define I2C_CSR_DEVBUSY_RST            0x0
//! @}  // field

//!
//! @name  I2C_TXCNT  register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     00           I2C_TXCNT   0
//! </pre>
//!
#define I2C_TXCNT_ADDR   0x00C0D034
#define I2C_TXCNT_OFFSET 0x00000034
#define I2C_TXCNT_INDEX  0x0000000D
#define I2C_TXCNT_RESET  0x00000000
//! @}  //

//! @name I2C_TXCNT Field Definitions
//! @{
#define I2C_TXCNT_MASK   ((uint32_t)0x000000FF)
#define I2C_TXCNT_LSB    0
#define I2C_TXCNT_WIDTH  ((uint32_t)0x00000008)

#define I2C_TXCNT_RST    0x0
//! @}  // field

//!
//! @name  I2C_RXCNT  register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     00           I2C_RXCNT   0
//! </pre>
//!
#define I2C_RXCNT_ADDR   0x00C0D038
#define I2C_RXCNT_OFFSET 0x00000038
#define I2C_RXCNT_INDEX  0x0000000E
#define I2C_RXCNT_RESET  0x00000000
//! @}  //

//! @name I2C_RXCNT Field Definitions
//! @{
#define I2C_RXCNT_MASK   ((uint32_t)0x000000FF)
#define I2C_RXCNT_LSB    0
#define I2C_RXCNT_WIDTH  ((uint32_t)0x00000008)
#define I2C_RXCNT_RST    0x0
//! @}  // field

//!
//! @name  I2C_TXFLUSH  register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     00           I2C_TXFLUSH   0
//! </pre>
//!
#define I2C_TXFLUSH_ADDR   0x00C0D03C
#define I2C_TXFLUSH_OFFSET 0x0000003C
#define I2C_TXFLUSH_INDEX  0x0000000F
#define I2C_TXFLUSH_RESET  0x00000000
//! @}  //

//! @name I2C_TXFLUSH Field Definitions
//! @{
#define I2C_TXFLUSH_MASK   ((uint32_t)0x00000001)
#define I2C_TXFLUSH_LSB    0
#define I2C_TXFLUSH_WIDTH  ((uint32_t)0x00000001)
#define I2C_TXFLUSH_RST    0x0
//! @}  // field

//!
//! @name I2C_INTCLR register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     07             BRKINTCLR   0x0
//!     06             STXIRQCLR   0x0
//!     05             M2SIRQCLR   0x0
//!  04:03            RXORIRQCLR   0x0
//!  02:00               STXWCLR   0x0
//! </pre>
//!
#define I2C_INTCLR_ADDR      0x00C0D040
#define I2C_INTCLR_OFFSET    0x00000040
#define I2C_INTCLR_INDEX     0x00000010
#define I2C_INTCLR_RESET     0x00000000
//! @}  //

//! @name I2C_INTCLR Field Definitions
//! @{
#define I2C_INTCLR_BRKINTCLR_BIT            ((uint32_t)0x00000080)
#define I2C_INTCLR_BRKINTCLR_POS             7
#define I2C_INTCLR_STXIRQCLR_BIT            ((uint32_t)0x00000040)
#define I2C_INTCLR_STXIRQCLR_POS             6
#define I2C_INTCLR_M2SIRQCLR_BIT            ((uint32_t)0x00000020)
#define I2C_INTCLR_M2SIRQCLR_POS             5
#define I2C_INTCLR_RXORIRQCLR_BIT           ((uint32_t)0x00000018)
#define I2C_INTCLR_RXORIRQCLR_LSB            3
#define I2C_INTCLR_RXORIRQCLR_WIDTH         ((uint32_t)0x00000002)
#define I2C_INTCLR_STXWCLR_BIT              ((uint32_t)0x00000007)
#define I2C_INTCLR_STXWCLR_LSB               0
#define I2C_INTCLR_STXWCLR_WIDTH            ((uint32_t)0x00000003)

#define I2C_INTCLR_BRKINTCLR _RST               0x0
#define I2C_INTCLR_STXIRQCLR_RST                0x0
#define I2C_INTCLR_M2SIRQCLR_RST                0x0
#define I2C_INTCLR_RXORIRQCLR_RST               0x0
#define I2C_INTCLR_STXWCLR_RST                  0x0
//! @}  // field

//!
//! @name  I2C_TESTIN  register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     01                SDAIN   1
//!     00                SCLIN   0
//! </pre>
//!
#define I2C_TESTIN_ADDR   0x00C0D050
#define I2C_TESTIN_OFFSET 0x00000050
#define I2C_TESTIN_INDEX  0x00000011
#define I2C_TESTIN_RESET  0x00000003
//! @}  //

//! @name I2C_TESTIN Field Definitions
//! @{
#define I2C_TESTIN_SDAIN_BIT   ((uint32_t)0x00000002)
#define I2C_TESTIN_SDAIN_POS     1
#define I2C_TESTIN_SCLIN_BIT   ((uint32_t)0x00000001)
#define I2C_TESTIN_SCLIN_POS     0

#define I2C_TESTIN_SDAIN_RST    0x2
#define I2C_TESTIN_SCLIN_RST    0x0
//! @}  // field

//!
//! @name  I2C_TESTOUT1  register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  07:00         I2C_TESTOUT1   0
//! </pre>
//!
#define I2C_TESTOUT1_ADDR   0x00C0D054
#define I2C_TESTOUT1_OFFSET 0x00000054
#define I2C_TESTOUT1_INDEX  0x00000012
#define I2C_TESTOUT1_RESET  0x00000000
//! @}  //

//! @name I2C_TESTOUT1 Field Definitions
//! @{
#define I2C_TESTOUT1_MASK   ((uint32_t)0x000000FF)
#define I2C_TESTOUT1_LSB    0
#define I2C_TESTOUT1_WIDTH  ((uint32_t)0x00000008)
#define I2C_TESTOUT1_RST    0x0
//! @}  // field

//!
//! @name  I2C_TESTOUT2  register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     03               SCLOEN   0
//!     02               SCLOUT   0
//!     01               SDAOEN   0
//!     00               SDAOUT   0
//! </pre>
//!
#define I2C_TESTOUT2_ADDR   0x00C0D058
#define I2C_TESTOUT2_OFFSET 0x00C0D058
#define I2C_TESTOUT2_INDEX  0x00000013
#define I2C_TESTOUT2_RESET  0x00000000
//! @}  //

//! @name I2C_TESTOUT2 Field Definitions
//! @{
#define I2C_TESTOUT2_SCLOEN_BIT   ((uint32_t)0x00000008)
#define I2C_TESTOUT2_SCLOEN_POS     3
#define I2C_TESTOUT2_SCLOUT_BIT   ((uint32_t)0x00000004)
#define I2C_TESTOUT2_SCLOUT_POS     2
#define I2C_TESTOUT2_SDAOEN_BIT   ((uint32_t)0x00000002)
#define I2C_TESTOUT2_SDAOEN_POS     1
#define I2C_TESTOUT2_SDAOUT_BIT   ((uint32_t)0x00000001)
#define I2C_TESTOUT2_SDAOUT_POS     0

#define I2C_TESTOUT2_SCLOEN_RST    0x0
#define I2C_TESTOUT2_SCLOUT_RST    0x0
#define I2C_TESTOUT2_SDAOEN_RST    0x0
#define I2C_TESTOUT2_SDAOUT_RST    0x0
//! @}  // field

//!
//! @name  I2C_I2CDEBPARAM  register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  03:00      I2C_I2CDEBPARAM   0
//! </pre>
//!
#define I2C_I2CDEBPARAM_ADDR   0x00C0D05C
#define I2C_I2CDEBPARAM_OFFSET 0x0000005C
#define I2C_I2CDEBPARAM_INDEX  0x00000014
#define I2C_I2CDEBPARAM_RESET  0x00000000
//! @}  //

//! @name  I2C_I2CDEBPARAM Field Definitions
//! @{
#define I2C_I2CDEBPARAM_MASK   ((uint32_t)0x0000000F)
#define I2C_I2CDEBPARAM_LSB    0
#define I2C_I2CDEBPARAM_WIDTH  ((uint32_t)0x00000004)
#define I2C_I2CDEBPARAM_RST    0x0
//! @}  // field

//!
//! @name  I2C_HOLDPARAM  register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  05:02          I2CHOLDPARAM  0x0
//!  01:00             NBHOLDDFF  0x0
//! </pre>
//!
#define I2C_HOLDPARAM_ADDR   0x00C0D060
#define I2C_HOLDPARAM_OFFSET 0x00000060
#define I2C_HOLDPARAM_INDEX  0x00000015
#define I2C_HOLDPARAM_RESET  0x00000031
//! @}

//! @name  I2C_HOLDPARAM Field Definitions
//! @{
#define I2C_HOLDPARAM_I2CHOLDPARAM_MASK    ((uint32_t)0x0000003C)
#define I2C_HOLDPARAM_I2CHOLDPARAM_LSB      2
#define I2C_HOLDPARAM_I2CHOLDPARAME_WIDTH  ((uint32_t)0x00000002)
#define I2C_HOLDPARAM_SCLOUT_MASK          ((uint32_t)0x00000003)
#define I2C_HOLDPARAM_SCLOUT_LSB            0
#define I2C_HOLDPARAM_SCLOUT_WIDTH         ((uint32_t)0x00000002)

#define I2C_HOLDPARAM_I2CHOLDPARAM_RST      0x0
#define I2C_HOLDPARAM_SCLOUT_RST            0x0

//! @}  // field
//! @}  // group

// ********************************PMU********************************
//! @defgroup PMU PMU
//! @ingroup ambt53
//! @{

//! PMU register base using I/O
#define APB_REG_BASE_ADDR   0x430000

//! PMU Registers address map
#define PMU_BASE_ADDRESS(x) (APB_REG_BASE_ADDR + x)

//! @name Registers addresses
//! @{

#define PMU_PLLCFG0_ADDR    PMU_BASE_ADDRESS(0x0)  // PLL configuration0
#define PMU_PLLCFG1_ADDR    PMU_BASE_ADDRESS(0x4)  // PLL configuration1
#if (BLUEBUD_RTL_VER >= BLUEBUD_RTL_1_1_1)
#define PMU_CLK_DIVS        PMU_BASE_ADDRESS(0x8) // Clock Dividers configuration
#else
#define PMU_CXCLK_DIV_ADDR  PMU_BASE_ADDRESS(0x8)  // CXCLK Wait state Configuration Register
#define PMU_XHCLK_DIV       PMU_BASE_ADDRESS(0xC)  // XHCLK Wait state Configuration Register
#define PMU_XPCLK_DIV       PMU_BASE_ADDRESS(0x10) // XPCLK Wait state Configuration Register
#endif // (BLUEBUD_RTL_VER >= BLUEBUD_RTL_1_1_1)
#define PMU_CXPMOD          PMU_BASE_ADDRESS(0x14) // TL41x Power Mode configuration
#define PMU_XHPMOD          PMU_BASE_ADDRESS(0x18) // XAHB subsystem power mode configuration
#define PMU_PMU_VER         PMU_BASE_ADDRESS(0x1C) // CEVA-TLS100 Version Register
#define PMU_XAPBMOD         PMU_BASE_ADDRESS(0x20) // XAPB subsystem power mode configuration
//! @}  //

//! @name PLLCFG0 field definitions
//! @{
#define PMU_PLLCFG0_MASK            ((uint32_t)0xFFFFFFFF)
#define PMU_PLLCFG0_RESET           0x00000000
//! @}  //

//! @name PLLCFG1 field definitions
//! @{
#define PMU_PLLCFG1_MASK            ((uint32_t)0xFFFFFFFF)
#define PMU_PLLCFG1_RESET           0x00000000
//! @}  //

#if (BLUEBUD_RTL_VER >= BLUEBUD_RTL_1_1_1)
//! @name PMU_CLKDIV field definitions
#define PMU_CLKDIV_MASK             ((uint32_t)0x00FFFFFF)
#define PMU_CLKDIV_RESET            0x00010100
#else
//! @name PMU_CXCLK_DIV field definitions
//! @{
#define PMU_CXCLK_DIV_MASK          ((uint32_t)0x0000000F)
#define PMU_CXCLK_DIV_RESET         0x00000000
//! @}  //

//! @name PMU_XHCLK_DIV field definitions
//! @{
#define PMU_XHCLK_DIV_MASK          ((uint32_t)0x0000000F)
#define PMU_XHCLK_DIV_RESET         0x00000001
//! @}  //

//! @name PMU_XPCLK_DIV field definitions
//! @{
#define PMU_XPCLK_DIV_MASK         ((uint32_t)0x0000000F)
#define PMU_XPCLK_DIV_RESET         0x00000003
//! @}  //
#endif // (BLUEBUD_RTL_VER >= BLUEBUD_RTL_1_1_1)

//! @name PMU_CXPMOD field definitions
//! @{
#define PMU_CXPMOD_MASK             ((uint32_t)0x000001FF)
#define PMU_CXPMOD_RESET            0x000000FF
//! @}  //

//! @name PMU_XHPMOD field definitions
//! @{
#define PMU_XHPMOD_MASK             ((uint32_t)0x000003FF)
#define PMU_XHPMOD_RESET            0x00000000
//! @}  //

//! @name PMU_PMU_VER field definitions
//! @{
#define PMU_PMU_VER_MASK            ((uint32_t)0x0FFFFF00)
//TODO: should have different reset value for different RTL version
#define PMU_PMU_VER_RESET           0x01001000
//! @}  //

//! @name PMU_XAPBMOD field definitions
//! @{
#define PMU_XAPBMOD_MASK            ((uint32_t)0x0000FFFF)
#define PMU_XAPBMOD_RESET           0x00000000
//! @}  //

//! @} // PMU group
// ********************************SYS ICU********************************
//! @defgroup SYSICU SYS ICU
//! @ingroup ambt53
//! @{

//! @name SYS ICU Register Addresses
//! @{
#define ICU_BASE_ADDR               (APB_REG_BASE_ADDR + 0x400)
#define ISR_LOW_OFFSET_ADDR         (ICU_BASE_ADDR + 0x0)
#define ISR_HIGH_OFFSET_ADDR        (ICU_BASE_ADDR + 0x4)
#define ICR_LOW_OFFSET_ADDR         (ICU_BASE_ADDR + 0x8)
#define ICR_HIGH_OFFSET_ADDR        (ICU_BASE_ADDR + 0xC)
#define IPR_LOW_OFFSET_ADDR         (ICU_BASE_ADDR + 0x10)
#define IPR_HIGH_OFFSET_ADDR        (ICU_BASE_ADDR + 0x14)
#define IMR0_LOW_OFFSET_ADDR        (ICU_BASE_ADDR + 0x18)
#define IMR0_HIGH_OFFSET_ADDR       (ICU_BASE_ADDR + 0x1C)
#define IMR1_LOW_OFFSET_ADDR        (ICU_BASE_ADDR + 0x20)
#define IMR1_HIGH_OFFSET_ADDR       (ICU_BASE_ADDR + 0x24)
#define IMR2_LOW_OFFSET_ADDR        (ICU_BASE_ADDR + 0x28)
#define IMR2_HIGH_OFFSET_ADDR       (ICU_BASE_ADDR + 0x2C)
#define VIMR_LOW_OFFSET_ADDR        (ICU_BASE_ADDR + 0x30)
#define VIMR_HIGH_OFFSET_ADDR       (ICU_BASE_ADDR + 0x34)
#define SPCCFG_OFFSET_ADDR          (ICU_BASE_ADDR + 0x38)
#define IGR_LOW_OFFSET_ADDR         (ICU_BASE_ADDR + 0x3C)
#define IGR_HIGH_OFFSET_ADDR        (ICU_BASE_ADDR + 0x40)
#define NMICFG_OFFSET_ADDR          (ICU_BASE_ADDR + 0x44)
#define VEC46_OFFSET_ADDR           (ICU_BASE_ADDR + 0x48)
#define VEC47_OFFSET_ADDR           (ICU_BASE_ADDR + 0x4C)
#define VIBASE0_OFFSET_ADDR         (ICU_BASE_ADDR + 0x50)
#define VIBASE1_OFFSET_ADDR         (ICU_BASE_ADDR + 0x54)
#define VIBASE2_OFFSET_ADDR         (ICU_BASE_ADDR + 0x58)
#define VICS_BOOT_LOW_OFFSET_ADDR   (ICU_BASE_ADDR + 0x5C)
#define VICS_BOOT_HIGH_OFFSET_ADDR  (ICU_BASE_ADDR + 0x60)
#define CXRCVR_OFFSET_ADDR          (ICU_BASE_ADDR + 0x64)

#define ISR_LOW_MASK                ((uint32_t)0xFFFFFFFF)
#define ISR_LOW_RESET               0x00000000

#define ISR_HIGH_MASK               ((uint32_t)0x0000FFFF)
#define ISR_HIGH_RESET              0x00000000

#define ICR_LOW_MASK                ((uint32_t)0xFFFFFFFF)
#define ICR_LOW_RESET               0x00000000

#define ICR_HIGH_MASK               ((uint32_t)0x0000FFFF)
#define ICR_HIGH_RESET              0x00000000

#define IPR_LOW_MASK                ((uint32_t)0xFFFFFFFF)
#define IPR_LOW_RESET               0x00000000

#define IPR_HIGH_MASK               ((uint32_t)0x0000FFFF)
#define IPR_HIGH_RESET              0x00000000

#define IMR0_LOW_MASK               ((uint32_t)0xFFFFFFFF)
#define IMR0_LOW_RESET              0x00000000

#define IMR0_HIGH_MASK              ((uint32_t)0x0000FFFF)
#define IMR0_HIGH_RESET             0x00000000

#define IMR1_LOW_MASK               ((uint32_t)0xFFFFFFFF)
#define IMR1_LOW_RESET              0x00000000

#define IMR1_HIGH_MASK              ((uint32_t)0x0000FFFF)
#define IMR1_HIGH_RESET             0x00000000

#define IMR2_LOW_MASK               ((uint32_t)0xFFFFFFFF)
#define IMR2_LOW_RESET              0x00000000

#define IMR2_HIGH_MASK              ((uint32_t)0x0000FFFF)
#define IMR2_HIGH_RESET             0x00000000

#define VIMR_LOW_MASK               ((uint32_t)0xFFFFFFFF)
#define VIMR_LOW_RESET              0x00000000

#define VIMR_HIGH_MASK              ((uint32_t)0x0000FFFF)
#define VIMR_HIGH_RESET             0x00000000

#define SPCCFG_MASK                 ((uint32_t)0x0003FFF)
#define SPCCFG_RESET                0x00000000

#define IGR_LOW_MASK                ((uint32_t)0xFFFFFFFF)
#define IGR_LOW_RESET               0x00000000

#define IGR_HIGH_MASK               ((uint32_t)0x0000FFFF)
#define IGR_HIGH_RESET              0x00000000

// #### INTERNAL BEGIN ####
//TODO  EXT_NISR WD_NISR RW in spec ,it's should be RO.
// #### INTERNAL END ####
#define NMICFG_MASK                 ((uint32_t)0x000000C0)
#define NMICFG_RESET                0x00000000

#define VEC46_MASK                  ((uint32_t)0xFFFFFFFF)
#define VEC46_RESET                 0x00000000

#define VEC47_MASK                  ((uint32_t)0xFFFFFFFF)
#define VEC47_RESET                 0x00000000

#define VIBASE0_MASK                ((uint32_t)0xFFFFFC00)
#define VIBASE0_RESET               0x00000000

#define VIBASE1_MASK                ((uint32_t)0xFFFFFC00)
#define VIBASE1_RESET               0x00000000

#define VIBASE2_MASK                ((uint32_t)0xFFFFFC00)
#define VIBASE2_RESET               0x00000000

#define VICS_BOOT_LOW_MASK          ((uint32_t)0xFFFFFFFF)
#define VICS_BOOT_LOW_RESET         0x00000000

#define VICS_BOOT_HIGH_MASK         ((uint32_t)0x0001FFFF)
#define VICS_BOOT_HIGH_RESET        0x00010000

#define CXRCVR_MASK                ((uint32_t)0x0000000F)
#define CXRCVR_RESET               0x0000000F
//! @}

//! @} // SYS ICU group

// ********************************TIMER********************************
//! @defgroup TIMER TIMER
//! @ingroup ambt53
//! @{

#define TIMER_BASE_ADDR                    (APB_REG_BASE_ADDR + 0x800)

//!
//! @name  TIMER_CFG register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  08:07                   TM    0
//!     06                   TP    0
//!     05                   CS    0
//!  04:02                   CM    0
//!  01:00                   TS    0
//! </pre>
//!
#define TIMER_CFG_OFFSET_ADDR              (TIMER_BASE_ADDR + 0x0)
#define TIMER_CFG_MASK                     ((uint32_t)0x000001FF)
#define TIMER_CFG_RW_TEST_MASK             ((uint32_t)0x000001EF)
#define TIMER_CFG_RESET                    0x00000000
//! @}

//!
//! @name  TIMER_CFG1 register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     04                 BPEN    1
//!     03                   RU    0
//!     02                   CT    0
//!     01                   PC    0
//!     00                  RES    0
//! </pre>
//!
#define TIMER_CFG1_OFFSET_ADDR             (TIMER_BASE_ADDR + 0x4)
#define TIMER_CFG1_MASK                    ((uint32_t)0x0000001F)
#define TIMER_CFG1_RESET                   0x00000010
//! @}

//!
//! @name  TIMER_EW register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     01           WD_RST_STS   1
//!     00                  EW    0
//! </pre>
//!

#define TIMER_EW_OFFSET_ADDR               (TIMER_BASE_ADDR + 0x8)
#define TIMER_EW_MASK                      ((uint32_t)0x00000003)
#define TIMER_EW_RESET                     0x00000000
//! @}

//!
//! @name  TIMER_SC register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:0                   SC    0
//! </pre>
//!

#define TIMER_SC_OFFSET_ADDR               (TIMER_BASE_ADDR + 0xC)
#define TIMER_SC_MASK                      ((uint32_t)0xFFFFFFFF)
#define TIMER_SC_RESET                     0x00000000
//! @}

//!
//! @name  TIMER_CC register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:0                   CC    0
//! </pre>
//!
#define TIMER_CC_OFFSET_ADDR               (TIMER_BASE_ADDR + 0x10)
#define TIMER_CC_MASK                      ((uint32_t)0xFFFFFFFF)
#define TIMER_CC_RESET                     0x00000000
//! @}

//!
//! @name  TIMER_SPWMC register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!  31:0          TIMER_SPWMC    0
//! </pre>
//!
#define TIMER_SPWMC_OFFSET_ADDR            (TIMER_BASE_ADDR + 0x14)
#define TIMER_SPWMC_MASK                   ((uint32_t)0xFFFFFFFF)
#define TIMER_SPWMC_RESET                  0x00000000

//! @}

//
//! @name  TIMER_CFG0 register definition
//! @{
//! <pre>
//!   Bits           Field Name   Reset Value
//!  -----   ------------------   -----------
//!     02               TRU_EN    0
//!     01             TREST_EN    0
//!     00              CASCADE    0
//! </pre>
//
#define TIMER_CFG0_OFFSET_ADDR             (TIMER_BASE_ADDR + 0x18)
#define TIMER_CFG0_MASK                    ((uint32_t)0x00000004)
#define TIMER_CFG0_RESET                   0x00000000
//! @}
//! @} // TIMER group

// ********************************APB_GPIO********************************
//! @defgroup APBGPIO APB GPIO
//! @ingroup ambt53
//! @{

//! GPIO I/O bus base address
#define GPIO_BASE_ADDR  (APB_REG_BASE_ADDR + 0x1800)//0xC00

//! @name GPIO Registers map
//! @{
#define GPIO_GP_OUT_ADDR (GPIO_BASE_ADDR + 0x0) // GP_OUT register
#define GPIO_GP_IN_ADDR  (GPIO_BASE_ADDR + 0x4) // GP_IN register
#define GPIO_GP_DIR_ADDR (GPIO_BASE_ADDR + 0x8) // GP_DIR register

#define GPIO_GP_OUT_MASK                   ((uint32_t)0xFFFFFFFF)
#define GPIO_GP_OUT_RESET                  0x00000000

#define GPIO_GP_IN_MASK                    ((uint32_t)0xFFFFFFFF)
#define GPIO_GP_IN_RESET                   0x00000000

#define GPIO_GP_DIR_MASK                   ((uint32_t)0xFFFFFFFF)
#define GPIO_GP_DIR_RESET                  0x00000000
//! @}
//! @} // APB_GPIO group

// ********************************TDM_MUX********************************
//! @defgroup TDMMUX TDM MUX
//! @ingroup ambt53
//! @{

#define TDMMUX_BASE_ADDR                               (APB_REG_BASE_ADDR + 0x4800)

#define TDMMUX_I2SCFG_ADDR                             (TDMMUX_BASE_ADDR + 0x00)
#define TDMMUX_I2SCFG_MASK                             ((uint32_t)0x073F3F3F)
#define TDMMUX_I2SCFG_RESET                            0x00000000

#define TDMMUX_TDMCFG_ADDR                             (TDMMUX_BASE_ADDR + 0x04)
#define TDMMUX_TDMCFG_MASK                             ((uint32_t)0x00000107)
#define TDMMUX_TDMCFG_RESET                            0x00000000

#define TDMMUX_I2SEN_ADDR                              (TDMMUX_BASE_ADDR + 0x08)
#define TDMMUX_I2SEN_MASK                              ((uint32_t)0x003F3F3F)
#define TDMMUX_I2SEN_RESET                             0x00000000

// x: 0 - 5
#define TDMMUX_I2SCTL_ADDR(x)                          (TDMMUX_BASE_ADDR + 0x0C + (0x4 * (x))) // x: 0-5
#define TDMMUX_I2SCTL_MASK                             ((uint32_t)0x0000FFFF)
#define TDMMUX_I2SCTL_RESET                            0x0000FFFF

#define TDMMUX_FIFORST_ADDR                            (TDMMUX_BASE_ADDR + 0x24)
#define TDMMUX_FIFORST_MASK                            ((uint32_t)0x0000003F)
#define TDMMUX_FIFORST_RESET                           0x00000000

#define TDMMUX_TDMIE_ADDR                              (TDMMUX_BASE_ADDR + 0x28)
#define TDMMUX_TDMIE_MASK                              ((uint32_t)0x0000003F)
#define TDMMUX_TDMIE_RESET                             0x00000000

// Read-only
#define TDMMUX_TDMIS_ADDR                             (TDMMUX_BASE_ADDR + 0x2C)
#define TDMMUX_TDMIS_MASK                             ((uint32_t)0x0000003F)
#define TDMMUX_TDMIS_RESET                            0x00000000

// Write-only
#define TDMMUX_TDMIC_ADDR                             (TDMMUX_BASE_ADDR + 0x30)
#define TDMMUX_TDMIC_MASK                             ((uint32_t)0x0000003F)
#define TDMMUX_TDMIC_RESET                            0x00000000

#define TDMMUX_TDMIM_ADDR                             (TDMMUX_BASE_ADDR + 0x34)
#define TDMMUX_TDMIM_MASK                             ((uint32_t)0x00000001)
#define TDMMUX_TDMIM_RESET                            0x00000000

#define TDMMUX_TDMBYP_ADDR                            (TDMMUX_BASE_ADDR + 0x38)
#define TDMMUX_TDMBYP_MASK                            ((uint32_t)0x00000001)
#define TDMMUX_TDMBYP_RESET                           0x00000000

// Read-only
#define TDMMUX_I2SACT_ADDR                            (TDMMUX_BASE_ADDR + 0x3C)
#define TDMMUX_I2SACT_MASK                            ((uint32_t)0x00777777)
#define TDMMUX_I2SACT_RESET                           0x00000000

// x: 0 - 3
#define TDMMUX_TDMSCR_ADDR(x)                         (TDMMUX_BASE_ADDR + 0x2F0 + (0x4 * (x))) // x: 0-3
#define TDMMUX_TDMSCR_MASK                            ((uint32_t)0xFFFFFFFF)
#define TDMMUX_TDMSCR_RESET                           0x00000000

//! @} // group

// *******************************SBOX********************************

//! @defgroup SBOX SBOX
//! @ingroup ambt53
//! @{

#define SBOX_BASE_ADDR                              (APB_REG_BASE_ADDR + 0x3800)

#define SBOX_TDM_RX_CNT_ADDR                        (SBOX_BASE_ADDR + 0x00)
#define SBOX_TDM_RX_CNT_MASK                        ((uint32_t)0xFFFFFFFF)
#define SBOX_TDM_RX_CNT_RESET                       0x00000000

#define SBOX_TDM_TX_CNT_ADDR                        (SBOX_BASE_ADDR + 0x04)
#define SBOX_TDM_TX_CNT_MASK                        ((uint32_t)0xFFFFFFFF)
#define SBOX_TDM_TX_CNT_RESET                       0x00000000

#define SBOX_CORE_CNT_ADDR                          (SBOX_BASE_ADDR + 0x08)
#define SBOX_CORE_CNT_MASK                          ((uint32_t)0xFFFFFFFF)
#define SBOX_CORE_CNT_RESET                         0x00000000

#define SBOX_PICO_CNT_ADDR                          (SBOX_BASE_ADDR + 0x0C)
#define SBOX_PICO_CNT_MASK                          ((uint32_t)0x0FFFFFFF)
#define SBOX_PICO_CNT_RESET                         0x00000000

#define SBOX_FINE_CNT_ADDR                          (SBOX_BASE_ADDR + 0x10)
#define SBOX_FINE_CNT_MASK                          ((uint32_t)0x000003FF)
#define SBOX_FINE_CNT_RESET                         0x00000000

#define SBOX_TDMRX_TRG_VAL_ADDR                     (SBOX_BASE_ADDR + 0x14)
#define SBOX_TDMRX_TRG_VAL_MASK                     ((uint32_t)0xFFFFFFFF)
#define SBOX_TDMRX_TRG_VAL_RESET                    0x0000FFFF

#define SBOX_TDMTX_TRG_VAL_ADDR                     (SBOX_BASE_ADDR + 0x18)
#define SBOX_TDMTX_TRG_VAL_MASK                     ((uint32_t)0xFFFFFFFF)
#define SBOX_TDMTX_TRG_VAL_RESET                    0x0000FFFF

#define SBOX_CORE_TRG_VAL_ADDR                      (SBOX_BASE_ADDR + 0x1C)
#define SBOX_CORE_TRG_VAL_MASK                      ((uint32_t)0xFFFFFFFF)
#define SBOX_CORE_TRG_VAL_RESET                     0x0000FFFF

#define SBOX_PICO_TRG_VAL_ADDR                      (SBOX_BASE_ADDR + 0x20)
#define SBOX_PICO_TRG_VAL_MASK                      ((uint32_t)0x0FFFFFFF)
#define SBOX_PICO_TRG_VAL_RESET                     0x0000FFFF

#define SBOX_FINE_TRG_VAL_ADDR                      (SBOX_BASE_ADDR + 0x24)
#define SBOX_FINE_TRG_VAL_MASK                      ((uint32_t)0x000003FF)
#define SBOX_FINE_TRG_VAL_RESET                     0x000000FF

#define SBOX_CTRL_ADDR                              (SBOX_BASE_ADDR + 0x28)
#define SBOX_CTRL_MASK                              ((uint32_t)0x1FFFFFFF)
#define SBOX_CTRL_RW_MASK                           ((uint32_t)0x1DFC01FF)
#define SBOX_CTRL_RESET                             0x00000000

#define SBOX_STATUS_ADDR                            (SBOX_BASE_ADDR + 0x2C)
#define SBOX_STATUS_MASK                            ((uint32_t)0x000007FF)
#define SBOX_STATUS_RESET                           0x00000000

//! @} // group

// ********************************PTA********************************
//! @defgroup PTA
//! @ingroup ambt53
//! @{
#define PTA_BASE_ADDR                               (APB_REG_BASE_ADDR + 0x4B00)

#define COEX_PTA_WLAN_CHANNEL_STORAGE_ADDR          (PTA_BASE_ADDR + 0x4)
#define COEX_PTA_WLAN_CHANNEL_STORAGE_MASK          ((uint32_t)0x0000FFFF)
#define COEX_PTA_WLAN_CHANNEL_STORAGE_RESET         0x00000000

#define COEX_PTA_WLAN_COEX_POL_ADDR                 (PTA_BASE_ADDR + 0x8)
#define COEX_PTA_WLAN_COEX_POL_MASK                 ((uint32_t)0x0000000F)
#define COEX_PTA_WLAN_COEX_POL_RESET                0x00000000

//! @}
//! @} // group

// ********************************BX1********************************
//! @defgroup BX1 BX1
//! @ingroup ambt53
//! @{
#define BX1_BASE_ADDR               0x300000

#define BX1_REG(offset)             (BX1_BASE_ADDR + (offset))

//! @name MultiCoreConfiguration REGISTER DEFINITION
//! @{
#define BX1_COM_REG_ADDR                                 BX1_REG(0x0000)
#define BX1_COM_REG_MASK                                 ((uint32_t)0xFFFFFFFF)
#define BX1_COM_REG_RESET                                0x00000000

#define BX1_COM_STS_ADDR                                 BX1_REG(0x0080)
#define BX1_COM_STS_MASK                                 ((uint32_t)0xFFFFFFFF)
#define BX1_COM_STS_RESET                                0x00000000

#define BX1_COM_INT_EN_ADDR                              BX1_REG(0x0084)
#define BX1_COM_INT_EN_MASK                              ((uint32_t)0x0000000F)
#define BX1_COM_INT_EN_RESET                             0x00000000

#define BX1_START_ADDR_ADDR                              BX1_REG(0x0090)
#define BX1_START_ADDR_MASK                              ((uint32_t)0x001FFFFF)
#define BX1_START_ADDR_RESET                             0x00000000

#define BX1_TOP_ADDR_ADDR                                BX1_REG(0x0094)
#define BX1_TOP_ADDR_MASK                                ((uint32_t)0x001FFFFF)
#define BX1_TOP_ADDR_RESET                               0x00000000

#define BX1_SNOOP_CSR_ADDR                               BX1_REG(0x0098)
#define BX1_SNOOP_CSR_RW_MASK                            ((uint32_t)0x0000000B)
#define BX1_SNOOP_CSR_RESET                              0x00000000
//! @} //MultiCoreConfiguration REGISTER DEFINITION end

//! @name PMSS REGISTER DEFINITION
//! @{
#define BX1_MSS_PCR_ADDR                                 BX1_REG(0x400)
#define BX1_MSS_PCR_MASK                                 ((uint32_t)0x0000134D)
#define BX1_MSS_PCR_RESET                                0x00000008

#define BX1_P_ADD0_START_ADDR                            BX1_REG(0x420)
#define BX1_P_ADD0_START_MASK                            ((uint32_t)0x100FFFFF)
#define BX1_P_ADD0_START_RESET                           0x00000000

#define BX1_P_ADD0_ATT0_ADDR                             BX1_REG(0x424)
#define BX1_P_ADD0_ATT0_MASK                             ((uint32_t)0x00000F33)
#define BX1_P_ADD0_ATT0_RESET                            0x00000000

#define BX1_P_ADDx_START_ADDR                            BX1_REG(0x428)
#define BX1_P_ADDx_START_MASK                            ((uint32_t)0x100FFFFF)
#define BX1_P_ADDx_START_RESET                           0x100FFFFF

#define BX1_P_ADDx_ATT0_ADDR                             BX1_REG(0x42C)
#define BX1_P_ADDx_ATT0_MASK                             ((uint32_t)0x00000F33)
#define BX1_P_ADDx_ATT0_RESET                            0x00000000

#define BX1_P_CCOSAR_ADDR                                BX1_REG(0x520)
#define BX1_P_CCOSAR_MASK                                ((uint32_t)0xFFFFFFC0)
#define BX1_P_CCOSAR_RESET                               0x00000000

#define BX1_P_CCOCR_ADDR                                 BX1_REG(0x524)
#define BX1_P_CCOCR_MASK                                 ((uint32_t)0x00000030)
#define BX1_P_CCOCR_RESET                                0x00000000
//! @} //PMSS REGISTER DEFINITION end

//! @name DMSS REGISTER DEFINITION
//! @{
#define BX1_MSS_HDCFG_ADDR                                BX1_REG(0x600)
#define BX1_MSS_HDCFG_MASK                                ((uint32_t)0x3FFFFFFF)
#define BX1_MSS_HDCFG_RESET                               0x12000049

#define BX1_MSS_MEMCFG_ADDR                               BX1_REG(0x604)
#define BX1_MSS_MEMCFG_MASK                               ((uint32_t)0xFD770773)
#define BX1_MSS_MEMCFG_RESET                              0x50210441

#define BX1_MSS_SDCFG_ADDR                                BX1_REG(0x608)
#define BX1_MSS_SDCFG_MASK                                ((uint32_t)0x000C0000)
#define BX1_MSS_SDCFG_RESET                               0x00000000

#define BX1_MSS_DMBE_ADDR                                 BX1_REG(0x60C)
#define BX1_MSS_DMBE_MASK                                 ((uint32_t)0x000F0DFF)
#define BX1_MSS_DMBE_RESET                                0x000F0DFF

#define BX1_MSS_DDEA_ADDR                                 BX1_REG(0x610)
#define BX1_MSS_DDEA_MASK                                 ((uint32_t)0xFFFFFFFF)
#define BX1_MSS_DDEA_RESET                                0x00000000

#define BX1_MSS_DDIA_ADDR                                 BX1_REG(0x614)
#define BX1_MSS_DDIA_MASK                                 ((uint32_t)0xFFFFFFFF)
#define BX1_MSS_DDIA_RESET                                0x00000000

#define BX1_MSS_DDTC_ADDR                                 BX1_REG(0x618)
#define BX1_MSS_DDTC_MASK                                 ((uint32_t)0xFFFFFFFF)
#define BX1_MSS_DDTC_RESET                                0x0C000000

#define BX1_MSS_DDQS_ADDR                                 BX1_REG(0x630)
#define BX1_MSS_DDQS_MASK                                 ((uint32_t)0x000100FF)
#define BX1_MSS_DDQS_RESET                                0x00000001

#define BX1_MSS_DDRS_ADDR                                 BX1_REG(0x634)
#define BX1_MSS_DDRS_MASK                                 ((uint32_t)0x000703FF)
#define BX1_MSS_DDRS_RESET                                0x00000000

#define BX1_MSS_BARRIER_ADDR                              BX1_REG(0x638)
#define BX1_MSS_BARRIER_MASK                              ((uint32_t)0x00000080)
#define BX1_MSS_BARRIER_RESET                             0x00000000

#define BX1_CCOSAR_ADDR                                   BX1_REG(0x640)
#define BX1_CCOSAR_MASK                                   ((uint32_t)0xFFFFFFC0)
#define BX1_CCOSAR_RESET                                  0x00000000

#define BX1_CCOSLR_ADDR                                   BX1_REG(0x644)
#define BX1_CCOSLR_MASK                                   ((uint32_t)0xFFFFFFFF)
#define BX1_CCOSLR_RESET                                  0x00000000

#define BX1_CCOCR_ADDR                                    BX1_REG(0x648)
#define BX1_CCOCR_RW_MASK                                 ((uint32_t)0xFFFF00BA)
#define BX1_CCOCR_RESET                                   0x00000000

#define BX1_MSS_GPOUT_ADDR                                BX1_REG(0x660)
#define BX1_MSS_GPOUT_MASK                                ((uint32_t)0xFFFFFFFF)
#define BX1_MSS_GPOUT_RESET                               0x00000000

#define BX1_MSS_GPIN_ADDR                                 BX1_REG(0x664)
#define BX1_MSS_GPIN_MASK                                 ((uint32_t)0xFFFFFFFF)
#define BX1_MSS_GPIN_RESET                                0x00000000
//! @} // DMSS REGISTER DEFINITION end

//! @name InterfaceConfiguration REGISTER DEFINITION
//! @{
#define BX1_XCI_COR_ADDR                                  BX1_REG(0x708)
#define BX1_XCI_COR_MASK                                  ((uint32_t)0x0000000F)
#define BX1_XCI_COR_RESET                                 0x0000000F

#define BX1_MSS_DMBA_ADDR                                 BX1_REG(0x710)
#define BX1_MSS_DMBA_MASK                                 ((uint32_t)0xFFFFFFFF)
#define BX1_MSS_DMBA_RESET                                0x00000000

#define BX1_ADD0_START_ADDR                               BX1_REG(0x720)
#define BX1_ADD0_START_MASK                               ((uint32_t)0x00100000)
#define BX1_ADD0_START_RESET                              0x00100000

#define BX1_ADD0_ATT0_ADDR                                BX1_REG(0x724)
#define BX1_ADD0_ATT0_MASK                                ((uint32_t)0xFFFFFFFF)
#define BX1_ADD0_ATT0_RESET                               0x00000000

#define BX1_ADD0_ATT1_ADDR                                BX1_REG(0x728)
#define BX1_ADD0_ATT1_MASK                                ((uint32_t)0x00000FFF)
#define BX1_ADD0_ATT1_RESET                               0x00000C00

#define BX1_ADDx_START_ADDR                               BX1_REG(0x730)
#define BX1_ADDx_START_MASK                               ((uint32_t)0x10FFFFFF)
#define BX1_ADDx_START_RESET                              0x108FFFFF

#define BX1_ADDx_ATT0_ADDR                                BX1_REG(0x734)
#define BX1_ADDx_ATT0_MASK                                ((uint32_t)0x000FFFFB)
#define BX1_ADDx_ATT0_RESET                               0x00000000

#define BX1_ADDx_ATT1_ADDR                                BX1_REG(0x738)
#define BX1_ADDx_ATT1_MASK                                ((uint32_t)0x00000FFE)
#define BX1_ADDx_ATT1_RESET                               0x00000C00
//! @} // InterfaceConfiguration REGISTER DEFINITION end

//! @name AP REGISTER DEFINITION
//! @{
#define BX1_MAPAR_ADDR                                    BX1_REG(0xc80)
#define BX1_MAPAR_MASK                                    ((uint32_t)0xFFFFFFFF)
#define BX1_MAPAR_RESET                                   0x00000000

#define BX1_DTAP_ADDR                                     BX1_REG(0xc88)
#define BX1_DTAP_MASK                                     ((uint32_t)0x00000007)
#define BX1_DTAP_RESET                                    0x00000000
//! @} // AP REGISTER DEFINITION end

//! @name DBG REGISTER DEFINITION
//! @{
#define BX1_DBG_DDAM_ADDR                                 BX1_REG(0xd00)
#define BX1_DBG_DDAM_MASK                                 ((uint32_t)0xFFFFFFFF)
#define BX1_DBG_DDAM_RESET                                0x00000000

#define BX1_DBG_DDS_ADDR                                  BX1_REG(0xd04)
#define BX1_DBG_DDS_MASK                                  ((uint32_t)0x00000007)
#define BX1_DBG_DDS_RESET                                 0x00000000

#define BX1_DBG_DDC_ADDR                                  BX1_REG(0xd08)
#define BX1_DBG_DDC_MASK                                  ((uint32_t)0x00000007)
#define BX1_DBG_DDC_RESET                                 0x00000000

#define BX1_POSCINT_ADDR                                  BX1_REG(0xd0c)
#define BX1_POSCINT_MASK                                  ((uint32_t)0x0000000F)
#define BX1_POSCINT_RESET                                 0x00000000

#define BX1_NENSIRQ_ADDR                                  BX1_REG(0xd14)
#define BX1_NENSIRQ_MASK                                  ((uint32_t)0xFFFFC007)
#define BX1_NENSIRQ_RESET                                 0x00000000

#define BX1_CENSIRQ_ADDR                                  BX1_REG(0xd1c)
#define BX1_CENSIRQ_MASK                                  ((uint32_t)0x005FFCD7)
#define BX1_CENSIRQ_RESET                                 0x00000000

#define BX1_DBG_GEN_ADDR                                  BX1_REG(0xd20)
#define BX1_DBG_GEN_MASK                                  ((uint32_t)0x047E000E)
#define BX1_DBG_GEN_RESET                                 0x00000000

#define BX1_DBG_GEN_MASK_ADDR                             BX1_REG(0xd30)
#define BX1_DBG_GEN_MASK_MASK                             ((uint32_t)0x047E000E)
#define BX1_DBG_GEN_MASK_RESET                            0x00000000

#define BX1_M_NENSIRQ_ADDR                                BX1_REG(0xd44)
#define BX1_M_NENSIRQ_MASK                                ((uint32_t)0x00000006)
#define BX1_M_NENSIRQ_RESET                               0x00000000

#define BX1_M_CENSIRQ_ADDR                                BX1_REG(0xd4c)
#define BX1_M_CENSIRQ_MASK                                ((uint32_t)0x005FC0D6)
#define BX1_M_CENSIRQ_RESET                               0x00000000

#define BX1_DBG_STACK_START_ADDR                          BX1_REG(0xd70)
#define BX1_DBG_STACK_START_MASK                          ((uint32_t)0xFFFFFFFF)
#define BX1_DBG_STACK_START_RESET                         0xFFFFFFFF

#define BX1_DBG_STACK_END_ADDR                            BX1_REG(0xd74)
#define BX1_DBG_STACK_END_MASK                            ((uint32_t)0xFFFFFFFF)
#define BX1_DBG_STACK_END_RESET                           0x00000000

#define BX1_DBG_DUNMPD_ADDR                               BX1_REG(0xd78)
#define BX1_DBG_DUNMPD_MASK                               ((uint32_t)0x000000FF)
#define BX1_DBG_DUNMPD_RESET                              0x00000000

#define BX1_DBG_DUNMPD_MSK_ADDR                           BX1_REG(0xd7c)
#define BX1_DBG_DUNMPD_MSK_MASK                           ((uint32_t)0x000000FF)
#define BX1_DBG_DUNMPD_MSK_RESET                          0x000000FF
//! @} // DBG REGISTER DEFINITION end

//! @name PSU REGISTER DEFINITION
//! @{
#define BX1_PSVM_ADDR                                     BX1_REG(0xe50)
#define BX1_PSVM_MASK                                     ((uint32_t)0x00000007)
#define BX1_PSVM_RESET                                    0x00000005

#define BX1_PGR_ADDR                                      BX1_REG(0xe54)
#define BX1_PGR_MASK                                      ((uint32_t)0x000000C0)
#define BX1_PGR_RESET                                     0x00000000
//! @} // PSU REGISTER DEFINITION end

//! @name First Timer
//! @{
#define TIMER0_CFG_ADDR             BX1_REG(0x2000)
#define TIMER0_CFG_MASK             ((uint32_t)0x001F07FF)
#define TIMER0_CFG_RW_TEST_MASK     ((uint32_t)0x001F07EF)//Watchdog Timer Mode
#define TIMER0_CFG_RESET            0x00100000

#define TIMER0_EW_ADDR              BX1_REG(0x2004)
#define TIMER0_EW_MASK              ((uint32_t)0x00000001)
#define TIMER0_EW_RESET             0x00000000

#define TIMER0_CC_ADDR              BX1_REG(0x2008)
#define TIMER0_CC_MASK              ((uint32_t)0x00000001)
#define TIMER0_CC_RESET             0x00000000

#define TIMER0_SC_ADDR              BX1_REG(0x200c)
#define TIMER0_SC_MASK              ((uint32_t)0xFFFFFFFF)
#define TIMER0_SC_RESET             0x00000000
//! @} // First Timer

//! @name Second Timer
//! @{
#define TIMER1_CFG_ADDR             BX1_REG(0x2010)
#define TIMER1_CFG_MASK             ((uint32_t)0x001F01FF)
#define TIMER1_CFG_RW_TEST_MASK     ((uint32_t)0x001F07EF)
#define TIMER1_CFG_RESET            0x00100000

#define TIMER1_EW_ADDR              BX1_REG(0x2014)
#define TIMER1_EW_MASK              ((uint32_t)0x00000001)
#define TIMER1_EW_RESET             0x00000000

#define TIMER1_CC_ADDR              BX1_REG(0x2018)
#define TIMER1_CC_MASK              ((uint32_t)0xFFFFFFFF)
#define TIMER1_CC_RESET             0x00000000

#define TIMER1_SC_ADDR              BX1_REG(0x201c)
#define TIMER1_SC_MASK              ((uint32_t)0xFFFFFFFF)
#define TIMER1_SC_RESET             0x00000000
//! @} // Second Timer

//! @name Third Timer
//! @{
#define TIMER2_CFG_ADDR              BX1_REG(0x2020)
#define TIMER2_CFG_MASK              ((uint32_t)0x001F07FF)
#define TIMER2_CFG_RW_TEST_MASK      ((uint32_t)0x001F07EF)
#define TIMER2_CFG_RESET             0x00100000

#define TIMER2_EW_ADDR               BX1_REG(0x2024)
#define TIMER2_EW_MASK               ((uint32_t)0x00000001)
#define TIMER2_EW_RESET              0x00000000

#define TIMER2_CC_ADDR               BX1_REG(0x2028)
#define TIMER2_CC_MASK               ((uint32_t)0xFFFFFFFF)
#define TIMER2_CC_RESET              0x00000000

#define TIMER2_SC_ADDR               BX1_REG(0x202c)
#define TIMER2_SC_MASK               ((uint32_t)0xFFFFFFFF)
#define TIMER2_SC_RESET              0x00000000

//! @} // Third Timer end

//! @name Fourth Timer
//! @{
#define TIMER3_CFG_ADDR              BX1_REG(0x2030)
#define TIMER3_CFG_MASK              ((uint32_t)0x001F03FF)
#define TIMER3_CFG_RW_TEST_MASK      ((uint32_t)0x001F07EF)
#define TIMER3_CFG_RESET             0x00100000

#define TIMER3_EW_ADDR               BX1_REG(0x2034)
#define TIMER3_EW_MASK               ((uint32_t)0x00000001)
#define TIMER3_EW_RESET              0x00000000

#define TIMER3_CC_ADDR               BX1_REG(0x2038)
#define TIMER3_CC_MASK               ((uint32_t)0xFFFFFFFF)
#define TIMER3_CC_RESET              0x00000000

#define TIMER3_SC_ADDR               BX1_REG(0x203c)
#define TIMER3_SC_MASK               ((uint32_t)0xFFFFFFFF)
#define TIMER3_SC_RESET              0x00000000

#define TIMER_IRQ_ADDR               BX1_REG(0x2044)
#define TIMER_IRQ_MASK               ((uint32_t)0xFFFFFFFF)
#define TIMER_IRQ_RESET              0x00000000

#define ISR0_ADDR                    BX1_REG(0x2200) // x = {0,..,3}
#define ISR0_MASK                    ((uint32_t)0xFFFFFFFF)
#define ISR0_RESET                   0x00000000

#define ICR0_ADDR                    BX1_REG(0x2220)// x = {0,..,3}
#define ICR0_MASK                    ((uint32_t)0xFFFFFFFF)
#define ICR0_RESET                   0x00000000

#define IGR0_ADDR                    BX1_REG(0x2240)// x = {0,..,3}
#define IGR0_MASK                    ((uint32_t)0xFFFFFFFF)
#define IGR0_RESET                   0x00000000

#define IMR0_ADDR                    BX1_REG(0x2260)// x = {0,..,3}
#define IMR0_MASK                    ((uint32_t)0xFFFFFFDE)
#define IMR0_RESET                   0x00000000

#define IGMR_ADDR                    BX1_REG(0x22e0)
// #### INTERNAL BEGIN ####
//TODO GM4~GM7 in SPEC ,but ceva code hasn't  GM4~GM7
// #### INTERNAL END ####
#define IGMR_MASK                    ((uint32_t)0x8001000F)
#define IGMR_RESET                   0x00000000

#define IVA2_ADDR                    BX1_REG(0x2308)// x = {2,..,3}
#define IVA2_MASK                    ((uint32_t)0xFFFFFF00)
#define IVA2_RESET                   0x00000000

#define IVA3_ADDR                    BX1_REG(0x230C)
#define IVA3_MASK                    ((uint32_t)0xFFFFFF00)
#define IVA3_RESET                   0x00000000

#define IVA4_ADDR                    BX1_REG(0x2310)
#define IVA4_MASK                    ((uint32_t)0xFFFFFF00)
#define IVA4_RESET                   0x00000000

#define IVA5_ADDR                    BX1_REG(0x2314)
#define IVA5_MASK                    ((uint32_t)0xFFFFFF00)
#define IVA5_RESET                   0x00000000

#define IVA6_ADDR                    BX1_REG(0x2318)
#define IVA6_MASK                    ((uint32_t)0xFFFFFF00)
#define IVA6_RESET                   0x00000000

#define IVA7_ADDR                    BX1_REG(0x231C)
#define IVA7_MASK                    ((uint32_t)0xFFFFFF00)
#define IVA7_RESET                   0x00000000

#define ICFG0_ADDR                   BX1_REG(0x2400)// x = {0,..,31}
#define ICFG0_RO_MASK                ((uint32_t)0x000071EF)
#define ICFG0_RW_MASK                ((uint32_t)0x000001EC)
#define ICFG0_RESET                  0x00007000

//! @} //
//! @} // group end

//*****************************************************************************
//
//! @name Global type definitions.
//! @{
//
//*****************************************************************************
typedef enum
{
    AM_DEVICES_AMBT53_STATUS_SUCCESS,
    AM_DEVICES_AMBT53_STATUS_FAIL,
    AM_DEVICES_AMBT53_STATUS_INVALID_HANDLE,
    AM_DEVICES_AMBT53_STATUS_IN_USE,
    AM_DEVICES_AMBT53_STATUS_TIMEOUT,
    AM_DEVICES_AMBT53_STATUS_OUT_OF_RANGE,
    AM_DEVICES_AMBT53_STATUS_INVALID_ARG,
    AM_DEVICES_AMBT53_STATUS_INVALID_OPERATION,
    AM_DEVICES_AMBT53_STATUS_MEM_ERR,
    AM_DEVICES_AMBT53_STATUS_HW_ERR,
    AM_DEVICES_AMBT53_STATUS_SEGMENT_SIZE_ERR
} am_devices_ambt53_status_t;

typedef enum
{
    REG_EQUAL,
    REG_NOT_EQUAL,
    REG_LESS
}ambt53_reg_check_e;

typedef struct
{
    uint32_t   reg_addr;        //!< reg address
    uint32_t   reg_prop;        //!< Read/Write
    uint32_t   reg_field;       //!< Bits/Location
    uint32_t   reg_reset_val;   //!< Reset Value
}ambt53_reg_t;

typedef struct
{
    ambt53_reg_t* reg_list;
    uint32_t      count;
    char *string;
}ambt53_reg_test_t;

//! @}

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
// Check if the register or DMA opereation via MSPI succeed
//*****************************************************************************
#define CHECK_REG_WRITE(status, addr)                                            \
    if ((status) != AM_DEVICES_AMBT53_STATUS_SUCCESS)                            \
    {                                                                            \
        am_util_stdio_printf("ambt53 reg 0x%08x write error\n", (addr));         \
        return (status);                                                         \
    }

#define CHECK_REG_READ(status, addr)                                             \
    if ((status) != AM_DEVICES_AMBT53_STATUS_SUCCESS)                            \
    {                                                                            \
        am_util_stdio_printf("ambt53 reg 0x%08x read error\n", (addr));          \
        return (status);                                                         \
    }

#define CHECK_DMA_WRITE(status, addr)                                            \
    if ((status) != AM_DEVICES_AMBT53_STATUS_SUCCESS)                            \
    {                                                                            \
        am_util_stdio_printf("ambt53 DMA write error, addr: 0x%08x\n", (addr));  \
        return (status);                                                         \
    }

#define CHECK_DMA_READ(status, addr)                                             \
    if ((status) != AM_DEVICES_AMBT53_STATUS_SUCCESS)                            \
    {                                                                            \
        am_util_stdio_printf("ambt53 DMA read error, addr: 0x%08x\n", (addr));   \
        return (status);                                                         \
    }

#define CHECK_DMA_ERASE(status, addr)                                            \
    if ((status) != AM_DEVICES_AMBT53_STATUS_SUCCESS)                            \
    {                                                                            \
        am_util_stdio_printf("ambt53 DMA erase error, addr: 0x%08x\n", (addr));  \
        return (status);                                                         \
    }

//*****************************************************************************
// Firmware loading and ambt53 core booting interface.
//*****************************************************************************
//*****************************************************************************
//
//! @brief Hold the ambt53 core reset signal.
//!
//! @return 32-bit status
//!
//*****************************************************************************
uint32_t am_devices_ambt53_core_hold(void);
//*****************************************************************************
//
//! @brief Release the ambt53 core reset signal to start running the core.
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t am_devices_ambt53_core_start(void);
//*****************************************************************************
//
//! @brief Initialize ambt53 interface and start to boot.
//!
//! @return 32-bit status
//
//*****************************************************************************
uint32_t am_devices_ambt53_boot(void);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_AMBT53_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

