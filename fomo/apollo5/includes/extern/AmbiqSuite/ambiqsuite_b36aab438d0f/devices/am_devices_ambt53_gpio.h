//*****************************************************************************
//
//! @file am_devices_ambt53.h
//!
//! @brief The implementation of Apollo interface to AMBT53 GPIO embeded in SCPM.
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

#ifndef AM_DEVICES_AMBT53_GPIO_H
#define AM_DEVICES_AMBT53_GPIO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_util.h"
#include "am_mcu_apollo.h"

#define SET_ATTRIBUTE(raw_val, attr_offset, attr_length, atrr_val) \
        (((uint32_t)raw_val & (~((((uint32_t)1L << (uint32_t)attr_length) - (uint32_t)1L) << (uint32_t)attr_offset))) | ((uint32_t)atrr_val << (uint32_t)attr_offset))

#define GET_ATTRIBUTE(raw_val, attr_offset, attr_length) \
        (((uint32_t)raw_val & (((((uint32_t)1L << (uint32_t)attr_length) - (uint32_t)1L) << (uint32_t)attr_offset))) >> (uint32_t)attr_offset)

// ********************************EXT GPIO*********************************

//! @defgroup EXTGPIO EXT GPIO
//! @{
#define AMBT53_GPIO_TOTAL_GPIOS          (44)
#define AMBT53_GPIO_NUMWORDS            ((AMBT53_GPIO_TOTAL_GPIOS + 31) / 32)

#define AMBT53_GPIO0_IRQn               (0)
#define AMBT53_GPIO1_IRQn               (1)

#define AMBT53_GPIO_BASE_ADDR              (REG_SCPM_BASE_ADDR + 0x100)
#define AMBT53_GPIO_SIZE                   (0x80)
#define AMBT53_GPIO0_REG_MASK              (0xFFFFFFFF)
#define AMBT53_GPIO1_REG_MASK              (0xFFF)

//! Direction of data flow register
#define AMBT53_GPIO_DIR_REG(x)             (AMBT53_GPIO_BASE_ADDR + 0x00 + AMBT53_GPIO_SIZE * x)
//! Control mode select register
#define AMBT53_GPIO_CTRL_REG(x)            (AMBT53_GPIO_BASE_ADDR + 0x04 + AMBT53_GPIO_SIZE * x)
//! Output data set register
#define AMBT53_GPIO_SET_REG(x)             (AMBT53_GPIO_BASE_ADDR + 0x08 + AMBT53_GPIO_SIZE * x)
//! Output data clear register
#define AMBT53_GPIO_CLR_REG(x)             (AMBT53_GPIO_BASE_ADDR + 0x0C + AMBT53_GPIO_SIZE * x)
//! Output data register
#define AMBT53_GPIO_ODATA_REG(x)           (AMBT53_GPIO_BASE_ADDR + 0x10 + AMBT53_GPIO_SIZE * x)
//! Input data register
#define AMBT53_GPIO_IDATA_REG(x)           (AMBT53_GPIO_BASE_ADDR + 0x14 + AMBT53_GPIO_SIZE * x)
//! Interrupt enable register
#define AMBT53_GPIO_IEN_REG(x)             (AMBT53_GPIO_BASE_ADDR + 0x18 + AMBT53_GPIO_SIZE * x)
//! Interrupt sense register
#define AMBT53_GPIO_IS_REG(x)              (AMBT53_GPIO_BASE_ADDR + 0x1C + AMBT53_GPIO_SIZE * x)
//! Interrupt both edge register
#define AMBT53_GPIO_IBE_REG(x)             (AMBT53_GPIO_BASE_ADDR + 0x20 + AMBT53_GPIO_SIZE * x)
//! Interrupt event register
#define AMBT53_GPIO_IEV_REG(x)             (AMBT53_GPIO_BASE_ADDR + 0x24 + AMBT53_GPIO_SIZE * x)
//! Raw interrupt status register
#define AMBT53_GPIO_RIS_REG(x)             (AMBT53_GPIO_BASE_ADDR + 0x28 + AMBT53_GPIO_SIZE * x)
//! Interrupt mask register for gpio_intr
#define AMBT53_GPIO_IM_REG(x)              (AMBT53_GPIO_BASE_ADDR + 0x2C + AMBT53_GPIO_SIZE * x)
//! Masked interrupt status register for gpio_intr
#define AMBT53_GPIO_MIS_REG(x)             (AMBT53_GPIO_BASE_ADDR + 0x30 + AMBT53_GPIO_SIZE * x)
//! Interrupt clear register
#define AMBT53_GPIO_IC_REG(x)              (AMBT53_GPIO_BASE_ADDR + 0x34 + AMBT53_GPIO_SIZE * x)
//! Delete Bounce flag, 1 - enable, 0 - disable
#define AMBT53_GPIO_DB_REG(x)              (AMBT53_GPIO_BASE_ADDR + 0x38 + AMBT53_GPIO_SIZE * x)
//! Define Filter-ed Glitch
//! If the value is 32'h0000_0000, the db_clk, which is used to define the glitch, will be pclk.
//! If the value is 32'hFFFF_FFFF, the db_clk will be x_clk. Otherwise the db_clk will be 2*(gpio_dfg+1).
//! Anyway, it has no effect on de-bounce when gpio_db is 0.
#define AMBT53_GPIO_DFG_REG(x)             (AMBT53_GPIO_BASE_ADDR + 0x3C + AMBT53_GPIO_SIZE * x)
//! Interrupt mask register for gpio_intr1
#define AMBT53_GPIO_IM1_REG(x)             (AMBT53_GPIO_BASE_ADDR + 0x40 + AMBT53_GPIO_SIZE * x)
//! Masked interrupt status register for gpio_intr1
#define AMBT53_GPIO_MIS1_REG(x)            (AMBT53_GPIO_BASE_ADDR + 0x44 + AMBT53_GPIO_SIZE * x)

#define AMBT53_GPIO0_MASK                  (0xFFFFFFFF)
#define AMBT53_GPIO1_MASK                  (0xFFF)

typedef enum
{
    //! edge or level
    AMBT53_GPIO_INTR_EDGE = 0,
    //! edge or level
    AMBT53_GPIO_INTR_LEVEL = 1,
    //! single or both
    AMBT53_GPIO_INTR_SINGLE_EDGE = 0,
    //! single or both
    AMBT53_GPIO_INTR_BOTH_EDGE = 1,
    //! pos_edge or neg_edge
    AMBT53_GPIO_INTR_NEG_EDGE = 0,
    //! pos_edge or neg_edge
    AMBT53_GPIO_INTR_POS_EDGE = 1,
    //! pos_edge or neg_edge
    AMBT53_GPIO_INTR_NEG_LEVEL = 0,
    //! pos_edge or neg_edge
    AMBT53_GPIO_INTR_POS_LEVEL = 1
}am_devices_ambt53_gpio_intr_type_e;

typedef enum
{
    AMBT53_GPIO_PULL_UP   = 0,
    AMBT53_GPIO_PULL_DOWN = 1
}am_devices_ambt53_gpio_pullup_e;

typedef enum
{
    AMBT53_GPIO_DIR_INPUT  = 0,
    AMBT53_GPIO_DIR_OUTPUT = 1
}am_devices_ambt53_gpio_dir_e;

typedef enum
{
    AMBT53_GPIO_SOFT_MODE = 0,
    AMBT53_GPIO_HARD_MODE = 1 //!< Not supported in ambt53, do not set to 1!!!
}am_devices_ambt53_gpio_mode_e;

#define SCPM_GPIO_CSR_BASE_ADDR                        0x03000A00

#define SCPM_IO_CFG_MODE_SEL_ADDR                      (SCPM_GPIO_CSR_BASE_ADDR + 0x00)
#define SCPM_IO_CFG_MODE_SEL_MASK                      ((uint32_t)0x00000007)
#define SCPM_IO_CFG_MODE_SEL_RESET                     0x00000000

#define SCPM_IO_CFG_TOP_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x04)
#define SCPM_IO_CFG_TOP_MASK                           ((uint32_t)0x1F1F1F1F)
#define SCPM_IO_CFG_TOP_RESET                          0x00000000

#define SCPM_IO_CFG_AON0_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x08)
#define SCPM_IO_CFG_AON0_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON0_RESET                          0x00000000

#define SCPM_IO_CFG_AON1_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x0C)
#define SCPM_IO_CFG_AON1_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON1_RESET                          0x00000000

#define SCPM_IO_CFG_AON2_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x10)
#define SCPM_IO_CFG_AON2_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON2_RESET                          0x00000000

#define SCPM_IO_CFG_AON3_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x14)
#define SCPM_IO_CFG_AON3_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON3_RESET                          0x00000000

#define SCPM_IO_CFG_AON4_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x18)
#define SCPM_IO_CFG_AON4_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON4_RESET                          0x00000000

#define SCPM_IO_CFG_AON5_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x1C)
#define SCPM_IO_CFG_AON5_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON5_RESET                          0x00000000

#define SCPM_IO_CFG_AON6_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x20)
#define SCPM_IO_CFG_AON6_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON6_RESET                          0x00000000

#define SCPM_IO_CFG_AON7_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x24)
#define SCPM_IO_CFG_AON7_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON7_RESET                          0x00000000

#define SCPM_IO_CFG_AON8_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x28)
#define SCPM_IO_CFG_AON8_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON8_RESET                          0x00000000

#define SCPM_IO_CFG_AON9_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x2C)
#define SCPM_IO_CFG_AON9_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON9_RESET                          0x00000000

#define SCPM_IO_CFG_AON10_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x30)
#define SCPM_IO_CFG_AON10_MASK                           ((uint32_t)0xE7E7E7E7)
#define SCPM_IO_CFG_AON10_RESET                          0x00000000

#define SCPM_IO_CFG_AON11_ADDR                           (SCPM_GPIO_CSR_BASE_ADDR + 0x34)
#define SCPM_IO_CFG_AON11_MASK                           ((uint32_t)0x0000001F)
#define SCPM_IO_CFG_AON11_RESET                          0x00000000
//! @}  // end of group

//*****************************************************************************
//! @name SCPM extended GPIO configuration.
//! @{
//*****************************************************************************
#define AMBT53_GPIO_PINCFG_DEFAULT                        \
    {                                                     \
        .ePullup          = AMBT53_GPIO_PULL_UP,             \
        .eGPDir           = AMBT53_GPIO_DIR_INPUT,           \
        .eIntTri          = AMBT53_GPIO_INTR_EDGE,           \
        .eIntDir          = AMBT53_GPIO_INTR_SINGLE_EDGE,    \
        .eIntPol          = AMBT53_GPIO_INTR_NEG_EDGE,       \
        .uDBVal           = 0,                            \
        .bDBEn            = false,                        \
    }

#define AMBT53_GPIO_PINCFG_OUTPUT                         \
    {                                                     \
        .ePullup          = AMBT53_GPIO_PULL_UP,             \
        .eGPDir           = AMBT53_GPIO_DIR_OUTPUT,          \
        .eIntTri          = AMBT53_GPIO_INTR_EDGE,           \
        .eIntDir          = AMBT53_GPIO_INTR_SINGLE_EDGE,    \
        .eIntPol          = AMBT53_GPIO_INTR_POS_EDGE,       \
        .uDBVal           = 0,                            \
        .bDBEn            = false,                        \
    }

#define AMBT53_GPIO_PINCFG_INPUT                          \
    {                                                     \
        .ePullup          = AMBT53_GPIO_PULL_UP,             \
        .eGPDir           = AMBT53_GPIO_DIR_INPUT,          \
        .eIntTri          = AMBT53_GPIO_INTR_EDGE,           \
        .eIntDir          = AMBT53_GPIO_INTR_SINGLE_EDGE,    \
        .eIntPol          = AMBT53_GPIO_INTR_POS_EDGE,       \
        .uDBVal           = 0,                            \
        .bDBEn            = false,                        \
    }
//! @}

//*****************************************************************************
//! Structure for defining bitmasks used in the interrupt functions.
//*****************************************************************************
typedef struct
{
    union
    {
        volatile uint32_t Msk[AMBT53_GPIO_NUMWORDS];

        struct
        {
            volatile uint32_t b0 :   1;
            volatile uint32_t b1 :   1;
            volatile uint32_t b2 :   1;
            volatile uint32_t b3 :   1;
            volatile uint32_t b4 :   1;
            volatile uint32_t b5 :   1;
            volatile uint32_t b6 :   1;
            volatile uint32_t b7 :   1;
            volatile uint32_t b8 :   1;
            volatile uint32_t b9 :   1;
            volatile uint32_t b10 :  1;
            volatile uint32_t b11 :  1;
            volatile uint32_t b12 :  1;
            volatile uint32_t b13 :  1;
            volatile uint32_t b14 :  1;
            volatile uint32_t b15 :  1;
            volatile uint32_t b16 :  1;
            volatile uint32_t b17 :  1;
            volatile uint32_t b18 :  1;
            volatile uint32_t b19 :  1;
            volatile uint32_t b20 :  1;
            volatile uint32_t b21 :  1;
            volatile uint32_t b22 :  1;
            volatile uint32_t b23 :  1;
            volatile uint32_t b24 :  1;
            volatile uint32_t b25 :  1;
            volatile uint32_t b26 :  1;
            volatile uint32_t b27 :  1;
            volatile uint32_t b28 :  1;
            volatile uint32_t b29 :  1;
            volatile uint32_t b30 :  1;
            volatile uint32_t b31 :  1;
            volatile uint32_t b32 :  1;
            volatile uint32_t b33 :  1;
            volatile uint32_t b34 :  1;
            volatile uint32_t b35 :  1;
            volatile uint32_t b36 :  1;
            volatile uint32_t b37 :  1;
            volatile uint32_t b38 :  1;
            volatile uint32_t b39 :  1;
            volatile uint32_t b40 :  1;
            volatile uint32_t b41 :  1;
            volatile uint32_t b42 :  1;
            volatile uint32_t b43 :  1;
        } Msk_b;
    }U;
}am_devices_ambt53_gpio_mask_t;

//*****************************************************************************
//! SCPM extended GPIO configuration.
//*****************************************************************************
typedef struct
{
    uint32_t ePullup;
    uint32_t eGPDir;
    uint32_t eIntTri;
    uint32_t eIntDir;
    uint32_t eIntPol;
    uint32_t uDBVal;
    bool bDBEn;
}am_devices_ambt53_gpio_pincfg_t;

//*****************************************************************************
//! Structure where default configurations can be accessed.
//*****************************************************************************
extern const am_devices_ambt53_gpio_pincfg_t am_devices_ambt53_gpio_pincfg_default;
//*****************************************************************************
//! Structure where default configurations can be accessed.
//*****************************************************************************
extern const am_devices_ambt53_gpio_pincfg_t am_devices_ambt53_gpio_pincfg_output;
//*****************************************************************************
//! Structure where default configurations can be accessed.
//*****************************************************************************
extern const am_devices_ambt53_gpio_pincfg_t am_devices_ambt53_gpio_pincfg_input;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
// GPIO operation interface.
//*****************************************************************************
//*****************************************************************************
//
//! @brief Return the current configuration of a pin.
//!
//! @param ui32GpioNum  - GPIO number
//! @param psGpioCfg    - return pinconfig here
//!
//! @return  success
//
//*****************************************************************************
extern uint32_t am_devices_ambt53_gpio_pinconfig_get(uint32_t ui32GpioNum, am_devices_ambt53_gpio_pincfg_t* psGpioCfg);

//*****************************************************************************
//
//! @brief Configure the function of a single pin.
//!
//! @param ui32GpioNum
//! @param sGpioCfg
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_ambt53_gpio_pinconfig(uint32_t ui32GpioNum, const am_devices_ambt53_gpio_pincfg_t sGpioCfg);

//*****************************************************************************
//
//! @brief Read GPIO state values
//!
//! @param ui32GpioNum
//! @param eReadType
//! @param pui32ReadState
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_ambt53_gpio_state_read(uint32_t ui32GpioNum, am_hal_gpio_read_type_e eReadType,
                           uint32_t *pui32ReadState);

//*****************************************************************************
//
//! @brief Write GPIO state values
//!
//! @param ui32GpioNum
//! @param eWriteType
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_ambt53_gpio_state_write(uint32_t ui32GpioNum, am_hal_gpio_write_type_e eWriteType);

//*****************************************************************************
//
//! @brief GPIO Interrupt control.
//!
//! @param eControl
//! @param pGpioIntMaskOrNumber
//!
//! This function performs interrupt enabling, disabling, clear on
//! the various combinations of interrupt priority levels.
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_ambt53_gpio_interrupt_control(am_hal_gpio_int_ctrl_e eControl,
                              void *pGpioIntMaskOrNumber);

//*****************************************************************************
//
//! @brief Read the interrupt status of a given GPIO IRQ.
//!
//! @param ui32GpioIrq
//! @param bEnabledOnly
//! @param pui32IntStatus
//!
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_ambt53_gpio_interrupt_irq_status_get(uint32_t ui32GpioIrq,
                                         bool bEnabledOnly,
                                         uint32_t *pui32IntStatus);
//*****************************************************************************
//
//! @brief Clear the interrupt(s) for the given GPIO IRQ.
//!
//! @param ui32GpioIrq
//! @param ui32GpioIntMaskStatus
//!
//! @return status from am_devices_scpm_status_e
//
//*****************************************************************************
extern uint32_t am_devices_ambt53_gpio_interrupt_irq_clear(uint32_t ui32GpioIrq,
                                    uint32_t ui32GpioIntMaskStatus);
//*****************************************************************************
//
//! @brief Register an interrupt handler for a specific GPIO
//!
//! @param ui32GpioNum
//! @param pfnHandler
//! @param pArg
//!
//! @return status from am_devices_scpm_status_e
//
//*****************************************************************************
extern uint32_t am_devices_ambt53_gpio_interrupt_register(uint32_t ui32GpioNum,
                                   am_hal_gpio_handler_t pfnHandler,
                                   void *pArg);
//*****************************************************************************
//
//! @brief Relay interrupts from the main GPIO module to individual handlers.
//!
//! @param ui32GpioIrq
//! @param ui32GpioIntMaskStatus
//!
//! @note A typical call sequence to the service routine might look like:
//!   - am_hal_gpio_interrupt_irq_status_get(GPIO1_405F_IRQn, true, &ui32IntStatus);
//!   - am_hal_gpio_interrupt_service(GPIO1_405F_IRQn, ui32IntStatus, 0);
//!
//! @return status from am_devices_scpm_status_e
//
//*****************************************************************************
extern uint32_t am_devices_ambt53_gpio_interrupt_service(uint32_t ui32GpioIrq,
                                  uint32_t ui32GpioIntMaskStatus);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_AMBT53_GPIO_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

