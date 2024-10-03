//*****************************************************************************
//
//! @file am_devices_ambt53_mailbox.h
//!
//! @brief The implementation of Apollo interface to AMBT53 mailbox.
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

#ifndef AM_DEVICES_AMBT53_MAILBOX_H
#define AM_DEVICES_AMBT53_MAILBOX_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_util.h"
#include "am_mcu_apollo.h"

// ********************************MAILBOX********************************

//! @defgroup MAILBOX MAILBOX
//! @{

#define REG_MAILBOX_BASE_ADDR                          0x03000000

typedef enum
{
    MAILBOX_RPMSG_KICK = 0xA868,
}mailbox_tag_e;

#define MAILBOX_FIFO_HEADER(tag, len)                  ((tag << 16) | len)

#define MAILBOX_FIFO_MAX_DEEPTH                        (32)
#define MAILBOX_MAX_THRESHOLD                          (MAILBOX_FIFO_MAX_DEEPTH - 1)
#define MAILBOX_D2M_THRESHOLD                          (1)

#define MAILBOX_M2D_DATA_ADDR                          (REG_MAILBOX_BASE_ADDR + 0x0)
#define MAILBOX_M2D_DATA_MASK                          ((uint32_t)0xFFFFFFFF)
#define MAILBOX_M2D_DATA_RESET                         0x00000000

#define MAILBOX_D2M_DATA_ADDR                          (REG_MAILBOX_BASE_ADDR + 0x4)
#define MAILBOX_D2M_DATA_MASK                          ((uint32_t)0xFFFFFFFF)
#define MAILBOX_D2M_DATA_RESET                         0x00000000

#define MAILBOX_STATUS_ADDR                            (REG_MAILBOX_BASE_ADDR + 0x8)
#define MAILBOX_STATUS_MASK                            ((uint32_t)0x00070007)
#define MAILBOX_STATUS_RESET                           0x00010001

#define MAILBOX_M2D_ERROR_ADDR                         (REG_MAILBOX_BASE_ADDR + 0xC)
#define MAILBOX_M2D_ERROR_MASK                         ((uint32_t)0x00000003)
#define MAILBOX_M2D_ERROR_RESET                        0x00000000

#define MAILBOX_M2D_IT_ADDR                            (REG_MAILBOX_BASE_ADDR + 0x10)
#define MAILBOX_M2D_IT_MASK                            ((uint32_t)0x0000001F)
#define MAILBOX_M2D_IT_RESET                           0x00000010

#define MAILBOX_M2D_IS_ADDR                            (REG_MAILBOX_BASE_ADDR + 0x14)
#define MAILBOX_M2D_IS_MASK                            ((uint32_t)0x00000003)
#define MAILBOX_M2D_IS_RESET                           0x00000000

#define MAILBOX_M2D_IE_ADDR                            (REG_MAILBOX_BASE_ADDR + 0x18)
#define MAILBOX_M2D_IE_MASK                            ((uint32_t)0x00000003)
#define MAILBOX_M2D_IE_RESET                           0x00000000

#define MAILBOX_D2M_ERROR_ADDR                         (REG_MAILBOX_BASE_ADDR + 0x1C)//DSP No access
#define MAILBOX_D2M_ERROR_MASK                         ((uint32_t)0x00000003)
#define MAILBOX_D2M_ERROR_RESET                        0x00000000

#define MAILBOX_D2M_IT_ADDR                            (REG_MAILBOX_BASE_ADDR + 0x20)
#define MAILBOX_D2M_IT_MASK                            ((uint32_t)0x0000001F)
#define MAILBOX_D2M_IT_RESET                           0x00000010

#define MAILBOX_D2M_IS_ADDR                            (REG_MAILBOX_BASE_ADDR + 0x24)
#define MAILBOX_D2M_IS_MASK                            ((uint32_t)0x00000003)
#define MAILBOX_D2M_IS_RESET                           0x00000000

#define MAILBOX_D2M_IE_ADDR                            (REG_MAILBOX_BASE_ADDR + 0x28)
#define MAILBOX_D2M_IE_MASK                            ((uint32_t)0x00000003)
#define MAILBOX_D2M_IE_RESET                           0x00000000

#define MAILBOX_INT_THRESHOLD_Pos                      (0UL)
#define MAILBOX_INT_THRESHOLD_Msk                      (0x1UL)
#define MAILBOX_INT_ERROR_Pos                          (1UL)
#define MAILBOX_INT_ERROR_Msk                          (0x2UL)

#define MAILBOX_STATUS_D2M_EMPTY_Pos                   (0UL)
#define MAILBOX_STATUS_D2M_EMPTY_Msk                   (0x1UL)
#define MAILBOX_STATUS_D2M_FULL_Pos                    (1UL)
#define MAILBOX_STATUS_D2M_FULL_Msk                    (0x2UL)
#define MAILBOX_STATUS_D2M_THRESHOLD_ACTIVE_Pos        (2UL)
#define MAILBOX_STATUS_D2M_THRESHOLD_ACTIVE_Msk        (0x4UL)
#define MAILBOX_STATUS_M2D_EMPTY_Pos                   (16UL)
#define MAILBOX_STATUS_M2D_EMPTY_Msk                   (0x10000UL)
#define MAILBOX_STATUS_M2D_FULL_Pos                    (17UL)
#define MAILBOX_STATUS_M2D_FULL_Msk                    (0x20000UL)
#define MAILBOX_STATUS_M2D_THRESHOLD_ACTIVE_Pos        (18UL)
#define MAILBOX_STATUS_M2D_THRESHOLD_ACTIVE_Msk        (0x40000UL)

#define MAILBOX_ERROR_EMPTY_Pos                        (0UL)
#define MAILBOX_ERROR_EMPTY_Msk                        (0x1UL)
#define MAILBOX_ERROR_FULL_Pos                         (1UL)
#define MAILBOX_ERROR_FULL_Msk                         (0x2UL)

//! @} group

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
/**
 ****************************************************************************************
 * @brief: Set mailbox DSP to MCU interrupt threshold
 * @param: threshold - threshold to set, valid range is 1~31
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_set_d2m_int_thr(uint32_t threshold);

/**
 ****************************************************************************************
 * @brief: Enable mailbox DSP to MCU interrupt
 * @param: interrupt - supports MAILBOX_INT_THRESHOLD_Pos and MAILBOX_INT_ERROR_Pos
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_enable_d2m_int(uint32_t interrupt);
/**
 ****************************************************************************************
 * @brief: Read certain length mailbox data sent from MCU
 * @param : *buffer: buffer point, buffer length should be greater than
 * MAILBOX_FIFO_MAX_DEEPTH
 * @param : *len: length of data to read, return actual length read from mailbox
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_read_data(uint32_t *buffer, uint32_t *len);

/**
 ****************************************************************************************
 * @brief: Flush mailbox data sent from DSP
 * @param : *buffer: buffer point, buffer length should be greater than
 * MAILBOX_FIFO_MAX_DEEPTH, set it to NULL will drop all mailbox data
 * @param : *len: return actual length read from mailbox, set it to NULL will not return
 * the actual length
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_flush_data(uint32_t *buffer, uint32_t *len);

/**
 ****************************************************************************************
 * @brief: sent data to DSP through mailbox
 * @param : *buffer : buffer point, buffer length should not be greater than
 * MAILBOX_FIFO_MAX_DEEPTH
 * @param : *len: length of data to write return actual length has written into mailbox
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_write_data(uint32_t *buffer, uint32_t *len);

/**
 ****************************************************************************************
 * @brief: Get mailbox's status register
 * @param : *status: return the status
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_get_status(uint32_t *status);

/**
 ****************************************************************************************
 * @brief: Read mailbox d2m error reg
 * @param : *error_bits: return the error bitmap(MAILBOX_ERROR_EMPTY_Msk & MAILBOX_ERROR_FULL_Msk)
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_get_d2m_err(uint32_t *error_bits);

/**
 ****************************************************************************************
 * @brief: Clear mailbox d2m error reg
 * @param : err_to_clear: write 1 to clear the error bit(MAILBOX_ERROR_EMPTY_Msk
 * and MAILBOX_ERROR_FULL_Msk)
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_clear_d2m_err(uint32_t err_to_clear);

/**
 ****************************************************************************************
 * @brief: Read mailbox DSP to MCU interrupt threshold
 * @param : *threshold: return threshold set in mailbox register
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_get_d2m_int_thr(uint32_t *threshold);

/**
 ****************************************************************************************
 * @brief: Read mailbox DSP to MCU interrupt status
 * @param : *int_status: interrupt status, MAILBOX_INT_THRESHOLD_Msk or MAILBOX_INT_ERROR_Msk
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_get_d2m_int_status(uint32_t *int_status);

/**
 ****************************************************************************************
 * @brief: Clear mailbox DSP to MCU interrupt
 * @param : int_to_clear: MAILBOX_INT_THRESHOLD_Msk or MAILBOX_INT_ERROR_Msk or both
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_clear_d2m_int(uint32_t int_to_clear);

/**
 ****************************************************************************************
 * @brief: Disable mailbox DSP to MCU interrupt
 * @param : interrupt: supports MAILBOX_INT_THRESHOLD_Msk and MAILBOX_INT_ERROR_Msk,
 * set corresponding bit to 1 to disable it
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_disable_d2m_int(uint32_t interrupt);

/**
 ****************************************************************************************
 * @brief: Read mailbox DSP to MCU interrupt threshold
 * @param : *threshold: return threshold set in mailbox register
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_get_m2d_int_thr(uint32_t *threshold);

/**
 ****************************************************************************************
 * @brief: Initiate mailbox
 * @param : threshold: DSP to MCU FIFO threshold for triggering interrupt to
 * MCU, supports 1 ~ 31
 * @param : int_msk: DSP to MCU interrupt mask, supports MAILBOX_INT_THRESHOLD_Msk
 * and MAILBOX_INT_ERROR_Msk
 * @return: AM_DEVICES_AMBT53_STATUS_SUCCESS if success, otherwise fail
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_init(uint32_t threshold, uint32_t int_msk);

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_AMBT53_MAILBOX_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

