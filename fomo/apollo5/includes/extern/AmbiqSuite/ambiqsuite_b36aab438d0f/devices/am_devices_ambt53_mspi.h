//*****************************************************************************
//
//! @file am_devices_ambt53_mspi.h
//!
//! @brief The implementation of Apollo interface to AMBT53 general
//!        MSPI driver.
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

#ifndef AM_DEVICES_AMBT53_MSPI_H
#define AM_DEVICES_AMBT53_MSPI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_util.h"
#include "am_mcu_apollo.h"

//*****************************************************************************
// MSPI configuration.
//! @defgroup MSPICFG MSPI configuration
//! @{
//*****************************************************************************
#if defined(AM_PART_APOLLO4P)

// Set AMBT53_FMC to 1 if use the FMC daughter board high-speed interface for test.
// The FMC board cannot support OSPI testing because of interface connection limitation.
// Without FMC board, we need to have flying wires to the EVB from FMC daugahter board
// instead of the high-speed interface, only support 4MHz or lower SPI clock at this scenario.
#define AMBT53_FMC                                1
#define MSPI_DQS_EN                               0 //Apollo4 plus can support DQS

#if (AMBT53_FMC)
#define MSPI_AMBT53_MODULE                        2
// GPIO83 in FMC daughter board high-speed interfacewas was designed for the MSPI DQS. We start
// to use GPIO83 for the MSPI CS which works more stably than flying wire from FPGA 82101440-RC10.
// Then the MSPI DQS should not be enabled. Before this image we used GPIO52 to fly wire to FMC
// board J40-13 for MSPI CS.
#define AM_AMBT53_GPIO_MSPI_CE                    83
#if (AM_AMBT53_GPIO_MSPI_CE == 83)
#if (MSPI_DQS_EN)
#error "MSPI DQS cannot be enabled since the PIN is used for MSPI CS"
#endif
#endif
#define AM_AMBT53_GPIO_MSPI_SCK                   78
#else // (AMBT53_FMC)
#define MSPI_AMBT53_MODULE                        0
#endif // (AMBT53_FMC)

#else // (AM_PART_APOLLO4P)

#define AMBT53_FMC                                1
#define MSPI_DQS_EN                               0 //RevB does not support SDR DQS

#if (AMBT53_FMC)
#define MSPI_AMBT53_MODULE                        2
#define AM_AMBT53_GPIO_MSPI_CE                    79
#define AM_AMBT53_GPIO_MSPI_SCK                   78
#else // (AMBT53_FMC)
#define MSPI_AMBT53_MODULE                        0
#endif // (AMBT53_FMC)
#endif // (AM_PART_APOLLO4P)

#define AMBT53_MSPI_FREQ                          AM_HAL_MSPI_CLK_4MHZ
#define AMBT53_MSPI_IRQn                          ((IRQn_Type)(MSPI0_IRQn + MSPI_AMBT53_MODULE))
#if (MSPI_DQS_EN)
#define AMBT53_MSPI_DUMMY                         0
#else
/* We got the real MSPI waiting cyles based on the testing in different clock settings with
 * DQS enablement. We may use some low dummcy cycles or enable DQS mode to save the traffic
 * time after full validation.
 *
 * |    AHB Clock     |      16MHz       |       32MHz       |
 * |    MSPI Clock    |  8MHz  |  12MHz  |  8MHz   |  12MHz  |
 * | MSPI Wait Cycles |  ~12   |   ~16   |   ~7    |   ~9    |
 */
#define AMBT53_MSPI_DUMMY                         32
#endif

//*****************************************************************************
//! Maximum MSPI instance number.
//*****************************************************************************
#define AM_DEVICES_MSPI_AMBT53_MAX_DEVICE_NUM     1

//
//! @name Take over the interrupt handler for whichever IOM we're using.
//! @{
//
#define ambt53_mspi_isr                                                       \
    am_mspi_isr1(MSPI_AMBT53_MODULE)
#define am_mspi_isr1(n)                                                       \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                        \
    am_mspi ## n ## _isr
//! @}
// ********************************XSPI********************************

//! @defgroup XSPI XSPI
//! @{

#define REG_XSPI_BASE_ADDR                          0x03000B00

#define XSPI_TRANSFER_STATUS_ADDR                   (REG_XSPI_BASE_ADDR + 0x0)
#define XSPI_TRANSFER_STATUS_RD_MASK                ((uint32_t)0xFFFFBFFF)
#define XSPI_TRANSFER_STATUS_RW_MASK                ((uint32_t)0x000000FC)
#define XSPI_TRANSFER_STATUS_RESET                  0xA5FF0000

#define XSPI_ADDR_REG_ADDR                          (REG_XSPI_BASE_ADDR + 0xC)
#define XSPI_ADDR_REG_RD_MASK                       ((uint32_t)0xFFFFFFFF)
#define XSPI_ADDR_REG_RESET                         0x00000000

#define XSPI_RX_DATA0_ADDR                          (REG_XSPI_BASE_ADDR + 0x10)
#define XSPI_RX_DATA0_RD_MASK                       ((uint32_t)0xFFFFFFFF)
#define XSPI_RX_DATA0_RESET                         0x00000000

#define XSPI_RX_DATA1_ADDR                          (REG_XSPI_BASE_ADDR + 0x14)
#define XSPI_RX_DATA1_RD_MASK                       ((uint32_t)0xFFFFFFFF)
#define XSPI_RX_DATA1_RESET                         0x00000000

#define XSPI_TX_DATA0_ADDR                          (REG_XSPI_BASE_ADDR + 0x20)
#define XSPI_TX_DATA0_RD_MASK                       ((uint32_t)0xFFFFFFFF)
#define XSPI_TX_DATA0_RESET                         0x00000000

#define XSPI_TX_DATA1_ADDR                          (REG_XSPI_BASE_ADDR + 0x24)
#define XSPI_TX_DATA1_RD_MASK                       ((uint32_t)0xFFFFFFFF)
#define XSPI_TX_DATA1_RESET                         0x00000000

//*****************************************************************************
//
//! @name Global definitions for the commands
//! @{
//
//*****************************************************************************
#define AM_DEVICES_MSPI_AMBT53_SOFT_RESET         0x99
#define AM_DEVICES_MSPI_AMBT53_WRITE_1BYTE        0x60
#define AM_DEVICES_MSPI_AMBT53_WRITE_2BYTES       0x61
// 0x60~0x6F used for 1~16 bytes writing
#define AM_DEVICES_MSPI_AMBT53_WRITE_BL1          0x70
#define AM_DEVICES_MSPI_AMBT53_WRITE_BL2          0x71
#define AM_DEVICES_MSPI_AMBT53_WRITE_BL4          0x72
#define AM_DEVICES_MSPI_AMBT53_WRITE_BL8          0x73
#define AM_DEVICES_MSPI_AMBT53_WRITE_BL16         0x74
#define AM_DEVICES_MSPI_AMBT53_WRITE_BL32         0x75
#define AM_DEVICES_MSPI_AMBT53_WRITE_BL64         0x76
#define AM_DEVICES_MSPI_AMBT53_WRITE_BL128        0x77
#define AM_DEVICES_MSPI_AMBT53_WRITE_BL256        0x78
#define AM_DEVICES_MSPI_AMBT53_WRITE_UL           0x7F
#define AM_DEVICES_MSPI_AMBT53_READ_1BYTE         0x80
#define AM_DEVICES_MSPI_AMBT53_READ_2BYTES        0x81
// 0x80~0x8F used for 1~16 bytes reading
#define AM_DEVICES_MSPI_AMBT53_READ_BL1           0xA0
#define AM_DEVICES_MSPI_AMBT53_READ_BL2           0xA1
#define AM_DEVICES_MSPI_AMBT53_READ_BL4           0xA2
#define AM_DEVICES_MSPI_AMBT53_READ_BL8           0xA3
#define AM_DEVICES_MSPI_AMBT53_READ_BL16          0xA4
#define AM_DEVICES_MSPI_AMBT53_READ_BL32          0xA5
#define AM_DEVICES_MSPI_AMBT53_READ_BL64          0xA6
#define AM_DEVICES_MSPI_AMBT53_READ_BL128         0xA7
#define AM_DEVICES_MSPI_AMBT53_READ_BL256         0xA8
#define AM_DEVICES_MSPI_AMBT53_READ_UL            0xC0
#define AM_DEVICES_MSPI_AMBT53_READ_STS           0x10
#define AM_DEVICES_MSPI_AMBT53_READ_ADR           0x11
#define AM_DEVICES_MSPI_AMBT53_READ_RXD           0x12
#define AM_DEVICES_MSPI_AMBT53_READ_TXD           0x13
#define AM_DEVICES_MSPI_AMBT53_READ_CFG           0x15
#define AM_DEVICES_MSPI_AMBT53_READ_VER           0x16
#define AM_DEVICES_MSPI_AMBT53_WRITE_STS          0x14
#define AM_DEVICES_MSPI_AMBT53_SOPM_1_1_1         0x20
#define AM_DEVICES_MSPI_AMBT53_SOPM_1_2_2         0x21
#define AM_DEVICES_MSPI_AMBT53_SOPM_1_4_4         0x22
#define AM_DEVICES_MSPI_AMBT53_SOPM_1_8_8         0x23
#define AM_DEVICES_MSPI_AMBT53_SOPM_2_2_2         0x24
#define AM_DEVICES_MSPI_AMBT53_SOPM_4_4_4         0x25
#define AM_DEVICES_MSPI_AMBT53_SOPM_8_8_8         0x26
#define AM_DEVICES_MSPI_AMBT53_SOPM_1_4_4_DQS     0x27
#define AM_DEVICES_MSPI_AMBT53_SOPM_1_8_8_DQS     0x28
#define AM_DEVICES_MSPI_AMBT53_SOPM_4_4_4_DQS     0x29
#define AM_DEVICES_MSPI_AMBT53_SOPM_8_8_8_DQS     0x2A
#define AM_DEVICES_MSPI_AMBT53_SET_DUMMY          0x40

#define AM_DEVICES_MSPI_AMBT53_MAX_PACKET_SIZE    0x4000000 //64M for UL
//! @}

typedef union
{
    struct
    {
        uint32_t BUSY       : 1;
        uint32_t WRITE      : 1;
        uint32_t INV_CMD    : 1;
        uint32_t INV_LEN    : 1;
        uint32_t BUS_ERR    : 1;
        uint32_t UNDERRUN   : 1;
        uint32_t OVERRUN    : 1;
        uint32_t BURST_ERR  : 1;
        uint32_t DMODE      : 2;
        uint32_t AMODE      : 2;
        uint32_t CMODE      : 2;
        uint32_t RESERVED   : 1;
        uint32_t DQS_EN     : 1;
        uint32_t DUMMY      : 8;
        uint32_t SYNC       : 8;
    }STATUS_b;
    uint32_t STATUS;
}ambt53_status_reg_t;

typedef enum
{
    XSPI_STATUS_MODE_SERIAL   = 0,
    XSPI_STATUS_MODE_DUAL     = 1,
    XSPI_STATUS_MODE_QUAD     = 2,
    XSPI_STATUS_MODE_OCTAL    = 3
} XSPI_STATUS_MODE_Enum;
//! @} // group

typedef struct
{
    am_hal_mspi_device_e eDeviceConfig;
    am_hal_mspi_clock_e eClockFreq;
    uint32_t *pNBTxnBuf;
    uint32_t ui32NBTxnBufLength;
    uint32_t ui32ScramblingStartAddr;
    uint32_t ui32ScramblingEndAddr;
    uint16_t ui16ReadInstr;
    uint16_t ui16WriteInstr;
    bool bChangeInstr;
} am_devices_mspi_ambt53_config_t;

//*****************************************************************************
//! @name MSPI instance definition.
//! @{
//*****************************************************************************
typedef struct
{
    uint32_t                    ui32Module;
    am_hal_mspi_device_e        eDeviceConfig;
    void                        *pMspiHandle;
    uint32_t                    maxTxSize;
    uint32_t                    maxRxSize;
    bool                        bOccupied;
} am_devices_mspi_ambt53_t;

// #### INTERNAL BEGIN ####
typedef struct
{
    uint32_t ui32Turnaround;
    uint32_t ui32Rxneg;
    uint32_t ui32Rxdqsdelay;
} am_devices_mspi_ambt53_remote_timing_config_t;
// #### INTERNAL END ####
//! @}

//*****************************************************************************
//! External AMBT53 module handle.
//*****************************************************************************
extern void* g_RpmsgDevHdl;
extern void* pvRpmsgMspiHandle;

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

//*****************************************************************************
// MSPI operation interface.
//*****************************************************************************

//*****************************************************************************
//! @brief Generic MSPI Command Write function via PIO method.
//!
//! @param pHandle
//! @param ui16Instr
//! @param bSendAddr
//! @param ui32Addr
//! @param pData
//! @param ui32NumBytes
//!
//! @return   status enum : am_devices_ambt53_status_t
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_pio_write(
   void     *pHandle,
   uint16_t ui16Instr,
   bool     bSendAddr,
   uint32_t ui32Addr,
   uint32_t *pData,
   uint32_t ui32NumBytes);

//*****************************************************************************
//! @brief generic MSPI command Read fucntion via PIO method.
//!
//! @param pHandle
//! @param ui16Instr
//! @param bSendAddr
//! @param ui32Addr
//! @param pData
//! @param ui32NumBytes
//!
//! @return   status enum : am_devices_ambt53_status_t
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_pio_read(void *pHandle,
                                uint16_t ui16Instr,
                                bool bSendAddr,
                                uint32_t ui32Addr,
                                uint32_t *pData,
                                uint32_t ui32NumBytes);

//*****************************************************************************
//
//! @brief Get the ambt53 xSPI basic configuration.
//! @param pHandle
//
//*****************************************************************************
extern void am_devices_mspi_ambt53_basic_config_get(void *pHandle);

//*****************************************************************************
//
//! @brief Get the MSPI status.
//! @param pHandle
//! @param pStatus
//
//*****************************************************************************
extern void am_devices_mspi_ambt53_status_get(void *pHandle,
                                ambt53_status_reg_t *pStatus);

//*****************************************************************************
//
//! @brief Clear the MSPI status.
//! @param pHandle
//! @param pStatusMsk
//
//*****************************************************************************
extern void am_devices_mspi_ambt53_status_clear(void *pHandle,
                                ambt53_status_reg_t *pStatusMsk);

//*****************************************************************************
//
//! @brief
//!
//! @param pHandle
//
//*****************************************************************************
extern void am_devices_mspi_ambt53_remote_reset(void *pHandle);

//*****************************************************************************
//
//! @brief Initialize the MSPI remote driver.
//! @param pHandle
//! @param pConfig
//! @param bDQSEn
//! @return
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_remote_init(void *pHandle,
                                am_hal_mspi_dev_config_t *pConfig,
                                bool bDQSEn);

//*****************************************************************************
//
//! @brief Initialize the MSPI local driver.
//!
//! @param ui32Module     - MSPI Module#
//! @param psMSPIConfig  - MSPI device structure.
//! @param ppHandle
//! @param ppMSPIHandle
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_init(uint32_t ui32Module,
                                am_devices_mspi_ambt53_config_t *psMSPIConfig,
                                void **ppHandle,
                                void **ppMSPIHandle);

//*****************************************************************************
//
//! @brief De-Initialize the mspi master driver.
//!
//! @param pHandle     - mspi handler
//!
//! This function reverses the initialization
//!
//! @return Status.
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_deinit(void *pHandle);

//*****************************************************************************
//
//! @brief Sets up the MSPI into XIP mode.
//!
//! @param pHandle  - Pointer to handle
//!
//! This function sets the MSPI into XIP mode.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_enable_xip(void *pHandle);
//*****************************************************************************
//
//! @brief Removes the MSPI from XIP mode.
//!
//! @param pHandle  - Pointer to handle
//!
//! This function removes the MSPI from XIP mode.
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_disable_xip(void *pHandle);

//*****************************************************************************
//
//! @brief Reads the contents of the external SRAM into a buffer.
//!
//! @param pHandle      - Pointer to driver handle
//! @param pui8RxBuffer - Buffer to store the received data from the ambt53
//! @param ui32ReadAddress - Address of desired data in ambt53 SRAM
//! @param ui32NumBytes - Number of bytes to read from ambt53 SRAM
//! @param bWaitForCompletion - Wait for transaction completion before exiting
//!
//! This function reads the ambt53 SRAM at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.  If the bWaitForCompletion is true,
//! then the function will poll for DMA completion indication flag before
//! returning.
//!
//! @return 32-bit status
//
//*****************************************************************************
//! @brief
//! @return
extern uint32_t am_devices_mspi_ambt53_dma_read(void *pHandle,
                                                       uint8_t *pui8RxBuffer,
                                                       uint32_t ui32ReadAddress,
                                                       uint32_t ui32NumBytes,
                                                       bool bWaitForCompletion);
//*****************************************************************************
//
//! @brief Reads the contents of the external ambt53 SRAM into a buffer.
//!
//! @param pHandle          - Pointer to driver handle
//! @param pui8RxBuffer - Buffer to store the received data from the SRAM
//! @param ui32ReadAddress - Address of desired data in external SRAM
//! @param ui32NumBytes - Number of bytes to read from external SRAM
//! @param ui32PauseCondition - Pause condition before transaction is executed
//! @param ui32StatusSetClr - Post-transaction CQ condition
//! @param pfnCallback - Post-transaction callback function
//! @param pCallbackCtxt - Post-transaction callback context
//!
//! This function reads the external SRAM at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.  The Command Queue pre and post
//! transaction conditions and a callback function and context are also
//! provided.
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_dma_read_adv(void *pHandle,
                                                       uint8_t *pui8RxBuffer,
                                                       uint32_t ui32ReadAddress,
                                                       uint32_t ui32NumBytes,
                                                       uint32_t ui32PauseCondition,
                                                       uint32_t ui32StatusSetClr,
                                                       am_hal_mspi_callback_t pfnCallback,
                                                       void *pCallbackCtxt);
//*****************************************************************************
//
//! @brief Reads the contents of the external sram into a buffer.
//!
//! @param pHandle          - Pointer to driver handle
//! @param pui8RxBuffer   - Buffer to store the received data from the sram
//! @param ui32ReadAddress - Address of desired data in external sram
//! @param ui32NumBytes   - Number of bytes to read from external sram
//! @param pfnCallback    - Pointer to callback function
//! @param pCallbackCtxt  - Callback Context (argument for callback)
//!
//! This function reads the external sram at the provided address and stores
//! the received data into the provided buffer location. This function will
//! only store ui32NumBytes worth of data.
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_dma_read_hiprio(void *pHandle,
                                                      uint8_t *pui8RxBuffer,
                                                      uint32_t ui32ReadAddress,
                                                      uint32_t ui32NumBytes,
                                                      am_hal_mspi_callback_t pfnCallback,
                                                      void *pCallbackCtxt);

//*****************************************************************************
//
//! @brief Programs the given range of ambt53 sram addresses.
//!
//! @param pHandle            - Pointer to driver handle
//! @param pui8TxBuffer       - Buffer to write the ambt53 sram data from
//! @param ui32WriteAddress   - Address to write to in the ambt53 sram
//! @param ui32NumBytes       - Number of bytes to write to the ambt53 sram
//! @param bWaitForCompletion - Wait for completion boolean
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the ambt53 sram at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target sram
//! memory or underflow the pui8TxBuffer array
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_dma_write(void *pHandle,
                                                        uint8_t *pui8TxBuffer,
                                                        uint32_t ui32WriteAddress,
                                                        uint32_t ui32NumBytes,
                                                        bool bWaitForCompletion);
//*****************************************************************************
//
//! @brief Programs the given range of ambt53 sram addresses.
//!
//! @param pHandle          - Pointer to driver handle
//! @param puiTxBuffer      - Buffer to write the external sram data from
//! @param ui32WriteAddress - Address to write to in the external sram
//! @param ui32NumBytes     - Number of bytes to write to the external sram
//! @param ui32PauseCondition -
//! @param ui32StatusSetClr -
//! @param pfnCallback      - Pointer to callback function
//! @param pCallbackCtxt    - Passed to callback function
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external sram at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target sram
//! memory or underflow the pui8TxBuffer array
//
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_dma_write_adv(void *pHandle,
                                                        uint8_t *puiTxBuffer,
                                                        uint32_t ui32WriteAddress,
                                                        uint32_t ui32NumBytes,
                                                        uint32_t ui32PauseCondition,
                                                        uint32_t ui32StatusSetClr,
                                                        am_hal_mspi_callback_t pfnCallback,
                                                        void *pCallbackCtxt);
//*****************************************************************************
//
//! @brief Programs the given range of sram addresses.
//!
//! @param pHandle          - Device number of the external sram
//! @param pui8TxBuffer     - Buffer to write the external sram data from
//! @param ui32WriteAddress - Address to write to in the external sram
//! @param ui32NumBytes     - Number of bytes to write to the external sram
//! @param pfnCallback      - Pointer to callback function
//! @param pCallbackCtxt    - Passed to callback function
//!
//! This function uses the data in the provided pui8TxBuffer and copies it to
//! the external sram at the address given by ui32WriteAddress. It will copy
//! exactly ui32NumBytes of data from the original pui8TxBuffer pointer. The
//! user is responsible for ensuring that they do not overflow the target sram
//! memory or underflow the pui8TxBuffer array
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t am_devices_mspi_ambt53_dma_write_hiprio(void *pHandle,
                                                       uint8_t *pui8TxBuffer,
                                                       uint32_t ui32WriteAddress,
                                                       uint32_t ui32NumBytes,
                                                       am_hal_mspi_callback_t pfnCallback,
                                                       void *pCallbackCtxt);

extern am_devices_mspi_ambt53_t gAMBT53[AM_DEVICES_MSPI_AMBT53_MAX_DEVICE_NUM];

//*****************************************************************************
//
//! @brief Write the given value to the specific register
//!
//! @param ui32WriteAddr - Address of register to write to
//! @param ui32Value     - Value of writing to the register
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t AMBT53_REG32_WR(uint32_t ui32WriteAddr, uint32_t ui32Value);

//*****************************************************************************
//
//! @brief Read the value from the specific register
//!
//! @param ui32ReadAddr  - Address of register to read from
//! @param ui32Value     - Value of reading from the register
//!
//! @return 32-bit status
//
//*****************************************************************************
extern uint32_t AMBT53_REG32_RD(uint32_t ui32ReadAddr, uint32_t* pui32Value);

//*****************************************************************************
//
//! @brief Flush all XIP MM write transactions and wait until it done
//!
//! @param block_in_us  - Max blocking time to wait in us
//!
//! @return 0 if successfully flushed the XIP MM transactions, return -1 if timeout
//
//*****************************************************************************
uint32_t am_device_ambt53_wait_mspi_idle(uint32_t block_in_us);

// #### INTERNAL BEGIN ####
//*****************************************************************************
//! @brief  Generic IOM Command Write function.
//! @param pHandle
//! @param ui64Instr
//! @param bSendAddr
//! @param ui32Addr
//! @param bCont
//! @param pData
//! @param ui32NumBytes
//! @return
//*****************************************************************************
extern uint32_t am_device_iom_ambt53_command_write(
   void *pHandle,
   uint64_t ui64Instr,
   bool bSendAddr,
   uint32_t ui32Addr,
   bool bCont,
   uint32_t *pData,
   uint32_t ui32NumBytes);

//*****************************************************************************
//! @brief Generic IOM Command Read function.
//! @param pHandle
//! @param ui64Instr
//! @param bSendAddr
//! @param ui32Addr
//! @param bCont
//! @param pData
//! @param ui32NumBytes
//! @return
//*****************************************************************************
extern uint32_t am_device_iom_ambt53_command_read(
   void *pHandle,
   uint64_t ui64Instr,
   bool bSendAddr,
   uint32_t ui32Addr,
   bool bCont,
   uint32_t *pData,
   uint32_t ui32NumBytes);

//*****************************************************************************
//! @brief Initialize the IOM local driver.
//! @param ui32Module
//! @param pDevConfig
//! @param ppHandle
//! @param ppIomHandle
//!
//! @return status from am_devices_ambt53_status_t
//*****************************************************************************
extern uint32_t am_devices_iom_ambt53_init(
   uint32_t ui32Module,
   am_devices_mspi_ambt53_config_t *pDevConfig,
   void **ppHandle,
   void **ppIomHandle);

//*****************************************************************************
//
//! @brief De-initialize the IOM local driver.
//! @param pHandle
//!
//! @return status from am_devices_ambt53_status_t
//
//*****************************************************************************
extern uint32_t am_devices_iom_ambt53_deinit(void *pHandle);
// #### INTERNAL END ####

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_AMBT53_MSPI_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

