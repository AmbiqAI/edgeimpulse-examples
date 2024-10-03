//*****************************************************************************
//
//  am_reg_flashctrl.h
//! @file
//!
//! @brief Register macros for the FLASHCTRL module
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_REG_FLASHCTRL_H
#define AM_REG_FLASHCTRL_H

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
//! @brief FLASHCTRL
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_FLASHCTRL_NUM_MODULES                 1
#define AM_REG_FLASHCTRLn(n) \
    (REG_FLASHCTRL_BASEADDR + 0x00001000 * n)
// #### INTERNAL END ####
//*****************************************************************************
//
// Register offsets.
//
//*****************************************************************************
// #### INTERNAL BEGIN ####
#define AM_REG_FLASHCTRL_STATUS_O                    0x00000000
#define AM_REG_FLASHCTRL_WRADDR_O                    0x00000004
#define AM_REG_FLASHCTRL_WRDATA_O                    0x00000008
#define AM_REG_FLASHCTRL_DATACMD_O                   0x0000000C
#define AM_REG_FLASHCTRL_INFOCMD_O                   0x00000010
#define AM_REG_FLASHCTRL_INFO1ACCESS_O               0x0000001C
#define AM_REG_FLASHCTRL_ACCESS_O                    0x00000014
#define AM_REG_FLASHCTRL_EVENTREGEN_O                0x00000200
#define AM_REG_FLASHCTRL_EVENTREGSTAT_O              0x00000204
#define AM_REG_FLASHCTRL_EVENTREGCLR_O               0x00000208
#define AM_REG_FLASHCTRL_EVENTREGSET_O               0x0000020C
// #### INTERNAL END ####

//*****************************************************************************
//
// Key values.
//
//*****************************************************************************
// #### INTERNAL BEGIN ####
#define AM_REG_FLASHCTRL_ACCESS_KEYVAL               0x000000C3
// #### INTERNAL END ####

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FLASHCTRL_EVENTREGEN - Flash Controller Event Registers: Enable
//
//*****************************************************************************
// This bit is the operation timeout Event notification.
#define AM_REG_FLASHCTRL_EVENTREGEN_TIMEOUT_S        2
#define AM_REG_FLASHCTRL_EVENTREGEN_TIMEOUT_M        0x00000004
#define AM_REG_FLASHCTRL_EVENTREGEN_TIMEOUT(n)       (((uint32_t)(n) << 2) & 0x00000004)

// This bit is the erase operation complete notification.
#define AM_REG_FLASHCTRL_EVENTREGEN_ERASECOMP_S      1
#define AM_REG_FLASHCTRL_EVENTREGEN_ERASECOMP_M      0x00000002
#define AM_REG_FLASHCTRL_EVENTREGEN_ERASECOMP(n)     (((uint32_t)(n) << 1) & 0x00000002)

// This bit is the write operation complete notification.
#define AM_REG_FLASHCTRL_EVENTREGEN_WRITECOMP_S      0
#define AM_REG_FLASHCTRL_EVENTREGEN_WRITECOMP_M      0x00000001
#define AM_REG_FLASHCTRL_EVENTREGEN_WRITECOMP(n)     (((uint32_t)(n) << 0) & 0x00000001)

// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FLASHCTRL_EVENTREGSTAT - Flash Controller Event Registers: Status
//
//*****************************************************************************
// This bit is the operation timeout Event notification.
#define AM_REG_FLASHCTRL_EVENTREGSTAT_TIMEOUT_S      2
#define AM_REG_FLASHCTRL_EVENTREGSTAT_TIMEOUT_M      0x00000004
#define AM_REG_FLASHCTRL_EVENTREGSTAT_TIMEOUT(n)     (((uint32_t)(n) << 2) & 0x00000004)

// This bit is the erase operation complete notification.
#define AM_REG_FLASHCTRL_EVENTREGSTAT_ERASECOMP_S    1
#define AM_REG_FLASHCTRL_EVENTREGSTAT_ERASECOMP_M    0x00000002
#define AM_REG_FLASHCTRL_EVENTREGSTAT_ERASECOMP(n)   (((uint32_t)(n) << 1) & 0x00000002)

// This bit is the write operation complete notification.
#define AM_REG_FLASHCTRL_EVENTREGSTAT_WRITECOMP_S    0
#define AM_REG_FLASHCTRL_EVENTREGSTAT_WRITECOMP_M    0x00000001
#define AM_REG_FLASHCTRL_EVENTREGSTAT_WRITECOMP(n)   (((uint32_t)(n) << 0) & 0x00000001)

// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FLASHCTRL_EVENTREGCLR - Flash Controller Event Registers: Clear
//
//*****************************************************************************
// This bit is the operation timeout Event notification.
#define AM_REG_FLASHCTRL_EVENTREGCLR_TIMEOUT_S       2
#define AM_REG_FLASHCTRL_EVENTREGCLR_TIMEOUT_M       0x00000004
#define AM_REG_FLASHCTRL_EVENTREGCLR_TIMEOUT(n)      (((uint32_t)(n) << 2) & 0x00000004)

// This bit is the erase operation complete notification.
#define AM_REG_FLASHCTRL_EVENTREGCLR_ERASECOMP_S     1
#define AM_REG_FLASHCTRL_EVENTREGCLR_ERASECOMP_M     0x00000002
#define AM_REG_FLASHCTRL_EVENTREGCLR_ERASECOMP(n)    (((uint32_t)(n) << 1) & 0x00000002)

// This bit is the write operation complete notification.
#define AM_REG_FLASHCTRL_EVENTREGCLR_WRITECOMP_S     0
#define AM_REG_FLASHCTRL_EVENTREGCLR_WRITECOMP_M     0x00000001
#define AM_REG_FLASHCTRL_EVENTREGCLR_WRITECOMP(n)    (((uint32_t)(n) << 0) & 0x00000001)

// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FLASHCTRL_EVENTREGSET - Flash Controller Event Registers: Set
//
//*****************************************************************************
// This bit is the operation timeout Event notification.
#define AM_REG_FLASHCTRL_EVENTREGSET_TIMEOUT_S       2
#define AM_REG_FLASHCTRL_EVENTREGSET_TIMEOUT_M       0x00000004
#define AM_REG_FLASHCTRL_EVENTREGSET_TIMEOUT(n)      (((uint32_t)(n) << 2) & 0x00000004)

// This bit is the erase operation complete notification.
#define AM_REG_FLASHCTRL_EVENTREGSET_ERASECOMP_S     1
#define AM_REG_FLASHCTRL_EVENTREGSET_ERASECOMP_M     0x00000002
#define AM_REG_FLASHCTRL_EVENTREGSET_ERASECOMP(n)    (((uint32_t)(n) << 1) & 0x00000002)

// This bit is the write operation complete notification.
#define AM_REG_FLASHCTRL_EVENTREGSET_WRITECOMP_S     0
#define AM_REG_FLASHCTRL_EVENTREGSET_WRITECOMP_M     0x00000001
#define AM_REG_FLASHCTRL_EVENTREGSET_WRITECOMP(n)    (((uint32_t)(n) << 0) & 0x00000001)

// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FLASHCTRL_STATUS - Flash Controller Status Register
//
//*****************************************************************************
// Information Space Command Enable. When set to 1, this bit indicates that
// write operations to the Flash INFO_CMD register are enabled. When set to 0,
// write and program operations are disabled to the Flash Information space.
#define AM_REG_FLASHCTRL_STATUS_INFOCMDEN_S          3
#define AM_REG_FLASHCTRL_STATUS_INFOCMDEN_M          0x00000008
#define AM_REG_FLASHCTRL_STATUS_INFOCMDEN(n)         (((uint32_t)(n) << 3) & 0x00000008)
#define AM_REG_FLASHCTRL_STATUS_INFOCMDEN_DISABLED   0x00000000
#define AM_REG_FLASHCTRL_STATUS_INFOCMDEN_ENABLED    0x00000008

// Write Data Ready. When set to 1, this bit indicates that the Flash Controller
// is ready to accept a new write data via the WR_DATA register. When set to a
// 0, the Flash Controller is not ready for a new write data.
#define AM_REG_FLASHCTRL_STATUS_WRITERDY_S           2
#define AM_REG_FLASHCTRL_STATUS_WRITERDY_M           0x00000004
#define AM_REG_FLASHCTRL_STATUS_WRITERDY(n)          (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_FLASHCTRL_STATUS_WRITERDY_NOTREADY    0x00000000
#define AM_REG_FLASHCTRL_STATUS_WRITERDY_READY       0x00000004

// Write Address Error. When set to 1, this bit indicates that an address has
// been written to the WRADDR register which is outside the range of valid
// addresses for the Flash Memory. This bit will be cleared on a write to the
// WRADDR register with a value that falls in the range of valid addresses.
// Additionally, when a write to the Write Data registers occurs and the Write
// Address is auto-incremented, this bit will be set if the new address
// increments beyond the end of the valid Flash address space.
#define AM_REG_FLASHCTRL_STATUS_ADDRERROR_S          1
#define AM_REG_FLASHCTRL_STATUS_ADDRERROR_M          0x00000002
#define AM_REG_FLASHCTRL_STATUS_ADDRERROR(n)         (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_FLASHCTRL_STATUS_ADDRERROR_VALID      0x00000000
#define AM_REG_FLASHCTRL_STATUS_ADDRERROR_INVALID    0x00000002

// Controll Busy. This bit reflects the status of the most recently requested
// Flash operation. When set to 1, the Flash Controller is still processing the
// last operation. When set to 0, the controller is idle and ready to accept a
// new command request.
#define AM_REG_FLASHCTRL_STATUS_BUSY_S               0
#define AM_REG_FLASHCTRL_STATUS_BUSY_M               0x00000001
#define AM_REG_FLASHCTRL_STATUS_BUSY(n)              (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_FLASHCTRL_STATUS_BUSY_IDLE            0x00000000
#define AM_REG_FLASHCTRL_STATUS_BUSY_FLASHBUSY       0x00000001

// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FLASHCTRL_WRADDR - Flash Controller Write Address Register
//
//*****************************************************************************
// The value written to this field contains the byte address to use for the next
// Erase or Program operation to the Flash Instance. An instance address is
// required (i.e. 0-0x3FFFF).  If an invalid address is written, the Address
// Error bit of the Status register will be set.
#define AM_REG_FLASHCTRL_WRADDR_INST_S               19
#define AM_REG_FLASHCTRL_WRADDR_INST_M               0x00080000
#define AM_REG_FLASHCTRL_WRADDR_INST(n)              (((uint32_t)(n) << 19) & 0x00080000)

// The value written to this field contains the byte address to use for the next
// Erase or Program operation to the Flash Instance. An instance address is
// required (i.e. 0-0x3FFFF).  If an invalid address is written, the Address
// Error bit of the Status register will be set.
#define AM_REG_FLASHCTRL_WRADDR_ADDR_S               0
#define AM_REG_FLASHCTRL_WRADDR_ADDR_M               0x0007FFFF
#define AM_REG_FLASHCTRL_WRADDR_ADDR(n)              (((uint32_t)(n) << 0) & 0x0007FFFF)

// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FLASHCTRL_WRDATA - Flash Controller Write Address Register
//
//*****************************************************************************
// The value written to this field is the next data to use for a Program
// operation to the Flash Instance. A write to this register will initiate the
// Program Word operation, and once complete, will auto-increment the WRADDR to
// allow continuous writes without the need to write the next Address (for Data
// Segment writes ONLY! This auto-increment functionality is not supported for
// writes to the Information space of the Flash).
#define AM_REG_FLASHCTRL_WRDATA_DATA_S               0
#define AM_REG_FLASHCTRL_WRDATA_DATA_M               0xFFFFFFFF
#define AM_REG_FLASHCTRL_WRDATA_DATA(n)              (((uint32_t)(n) << 0) & 0xFFFFFFFF)

// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FLASHCTRL_DATACMD - Flash Controller Data Space Command Register
//
//*****************************************************************************
// Writing a 1 to this bitfield initiates a programming operation to the Flash
// Data space. Writing a 0 to this bitfield terminates the programming
// operation.
#define AM_REG_FLASHCTRL_DATACMD_PROGRAM_S           2
#define AM_REG_FLASHCTRL_DATACMD_PROGRAM_M           0x00000004
#define AM_REG_FLASHCTRL_DATACMD_PROGRAM(n)          (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_FLASHCTRL_DATACMD_PROGRAM_TERMINATE   0x00000000
#define AM_REG_FLASHCTRL_DATACMD_PROGRAM_GO          0x00000004

// Writing a 1 to this bitfield initiates an erase operation of the entire Flash
// Data space of the given instance.  The WRADDR register must be 0 for this
// operation.
#define AM_REG_FLASHCTRL_DATACMD_MASSERASE_S         1
#define AM_REG_FLASHCTRL_DATACMD_MASSERASE_M         0x00000002
#define AM_REG_FLASHCTRL_DATACMD_MASSERASE(n)        (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_FLASHCTRL_DATACMD_MASSERASE_GO        0x00000002

// Writing a 1 to this bitfield initiates a erase operation to the Flash Data
// word line specified by the WRADDR register.
#define AM_REG_FLASHCTRL_DATACMD_PAGEERASE_S         0
#define AM_REG_FLASHCTRL_DATACMD_PAGEERASE_M         0x00000001
#define AM_REG_FLASHCTRL_DATACMD_PAGEERASE(n)        (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_FLASHCTRL_DATACMD_PAGEERASE_GO        0x00000001

// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FLASHCTRL_INFOCMD - Flash Controller Information Space Command Register
//
//*****************************************************************************
// Writing a 1 to this bitfield initiates a programming operation to the Flash
// Information space. Writing a 0 to this bitfield terminates the programming
// operation.
#define AM_REG_FLASHCTRL_INFOCMD_PROGRAM_S           2
#define AM_REG_FLASHCTRL_INFOCMD_PROGRAM_M           0x00000004
#define AM_REG_FLASHCTRL_INFOCMD_PROGRAM(n)          (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_FLASHCTRL_INFOCMD_PROGRAM_GO          0x00000004

// Writing a 1 to this bitfield initiates an erase operation of both the Flash
// Information and Flash Data spaces for the given flash instance.  The WRADDR
// register must be 0 for this operation.
#define AM_REG_FLASHCTRL_INFOCMD_MASSERASE_S         1
#define AM_REG_FLASHCTRL_INFOCMD_MASSERASE_M         0x00000002
#define AM_REG_FLASHCTRL_INFOCMD_MASSERASE(n)        (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_FLASHCTRL_INFOCMD_MASSERASE_GO        0x00000002

// Writing a 1 to this bitfield initiates a erase operation to the Flash
// Information space for the given instance. The WRADDR register is not relevant
// for an Information Space page erase operation.
#define AM_REG_FLASHCTRL_INFOCMD_PAGEERASE_S         0
#define AM_REG_FLASHCTRL_INFOCMD_PAGEERASE_M         0x00000001
#define AM_REG_FLASHCTRL_INFOCMD_PAGEERASE(n)        (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_FLASHCTRL_INFOCMD_PAGEERASE_GO        0x00000001

// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// FLASHCTRL_INFO1ACCESS - Key Register for the Flash Controller Registers
//
//*****************************************************************************
// Special key to unlock programming of INFO1 space.  Both the test mode and key
// must be written at the same time.  This field will return a 0 if the field is
// locked or 1 if the field is unlocked.
#define AM_REG_FLASHCTRL_INFO1ACCESS_KEY_S           8
#define AM_REG_FLASHCTRL_INFO1ACCESS_KEY_M           0xFFFFFF00
#define AM_REG_FLASHCTRL_INFO1ACCESS_KEY(n)          (((uint32_t)(n) << 8) & 0xFFFFFF00)
#define AM_REG_FLASHCTRL_INFO1ACCESS_KEY_DISABLE     0x00000000
#define AM_REG_FLASHCTRL_INFO1ACCESS_KEY_ENABLE      0x4A076200

// This bit indicates when the flash controls are in test mode.  When enabling
// test mode, software must poll on this bit until a 1 is returned before
// initiating an erase/program operation.  When disabling testmode, software
// must poll on this bit to be zero before allowing reads to the device.
#define AM_REG_FLASHCTRL_INFO1ACCESS_TESTMODE_STATUS_S 7
#define AM_REG_FLASHCTRL_INFO1ACCESS_TESTMODE_STATUS_M 0x00000080
#define AM_REG_FLASHCTRL_INFO1ACCESS_TESTMODE_STATUS(n) (((uint32_t)(n) << 7) & 0x00000080)

// Sets flash test mode
#define AM_REG_FLASHCTRL_INFO1ACCESS_TESTMODE_S      0
#define AM_REG_FLASHCTRL_INFO1ACCESS_TESTMODE_M      0x0000003F
#define AM_REG_FLASHCTRL_INFO1ACCESS_TESTMODE(n)     (((uint32_t)(n) << 0) & 0x0000003F)
#define AM_REG_FLASHCTRL_INFO1ACCESS_TESTMODE_DISABLE 0x00000000
#define AM_REG_FLASHCTRL_INFO1ACCESS_TESTMODE_ERASE_REFERENCE 0x0000000A
#define AM_REG_FLASHCTRL_INFO1ACCESS_TESTMODE_Access INFO1 0x0000001B

// #### INTERNAL END ####
#endif // AM_REG_FLASHCTRL_H
