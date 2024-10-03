//*****************************************************************************
//
//  am_reg_mcuctrl.h
//! @file
//!
//! @brief Register macros for the MCUCTRL module
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_REG_MCUCTRL_H
#define AM_REG_MCUCTRL_H

//*****************************************************************************
//
//! @brief MCUCTRL
//! Instance finder. (1 instance(s) available)
//
//*****************************************************************************
#define AM_REG_MCUCTRL_NUM_MODULES                   1
#define AM_REG_MCUCTRLn(n) \
    (REG_MCUCTRL_BASEADDR + 0x00000000 * n)
//*****************************************************************************
//
// Register offsets.
//
//*****************************************************************************
#define AM_REG_MCUCTRL_CHIP_INFO_O                   0x00000000
#define AM_REG_MCUCTRL_CHIPID0_O                     0x00000004
#define AM_REG_MCUCTRL_CHIPID1_O                     0x00000008
#define AM_REG_MCUCTRL_CHIPREV_O                     0x0000000C
#define AM_REG_MCUCTRL_SUPPLYSRC_O                   0x00000010
#define AM_REG_MCUCTRL_SUPPLYSTATUS_O                0x00000014
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_CCRG_O                        0x00000020
#define AM_REG_MCUCTRL_VREFGEN_O                     0x00000040
#define AM_REG_MCUCTRL_BUCK_O                        0x00000060
#define AM_REG_MCUCTRL_BUCK2_O                       0x00000064
#define AM_REG_MCUCTRL_LDOREG1_O                     0x00000080
#define AM_REG_MCUCTRL_LDOREG2_O                     0x00000084
#define AM_REG_MCUCTRL_LDOREG3_O                     0x00000088
#define AM_REG_MCUCTRL_UNCALVREF_O                   0x000000A0
#define AM_REG_MCUCTRL_HFRC_O                        0x000000C0
#define AM_REG_MCUCTRL_LFRC_O                        0x000000E0
#define AM_REG_MCUCTRL_IOCTRL_O                      0x000000E4
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_BANDGAPEN_O                   0x000000FC
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_TEMPSCTRL_O                   0x00000100
#define AM_REG_MCUCTRL_ADCPWRDLY_O                   0x00000108
#define AM_REG_MCUCTRL_ADCTSBGPWD_O                  0x0000010C
#define AM_REG_MCUCTRL_XTALCTRL_O                    0x00000120
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_O              0x00000140
#define AM_REG_MCUCTRL_SRAMPWRDIS_O                  0x00000144
#define AM_REG_MCUCTRL_FLASHPWRDIS_O                 0x00000148
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_PWRONRSTDLY_O                 0x00000180
#define AM_REG_MCUCTRL_PWRSEQ1ANA_O                  0x00000184
#define AM_REG_MCUCTRL_PWRSEQ2ANA_O                  0x00000188
#define AM_REG_MCUCTRL_PWRSEQ3ANA_O                  0x0000018C
#endif // MCU_VALIDATION
#define AM_REG_MCUCTRL_BOOTLOADERLOW_O               0x000001A0
#define AM_REG_MCUCTRL_SHADOWVALID_O                 0x000001A4
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_ICODEFAULTADDR_O              0x000001C0
#define AM_REG_MCUCTRL_DCODEFAULTADDR_O              0x000001C4
#define AM_REG_MCUCTRL_SYSFAULTADDR_O                0x000001C8
#define AM_REG_MCUCTRL_FAULTSTATUS_O                 0x000001CC
#define AM_REG_MCUCTRL_FAULTCAPTUREEN_O              0x000001D0
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_PMUENABLE_O                   0x00000220
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_TPIUCTRL_O                    0x00000250

//*****************************************************************************
//
// MCUCTRL_CHIP_INFO - Chip Information Register
//
//*****************************************************************************
// Device class.
#define AM_REG_MCUCTRL_CHIP_INFO_CLASS_S             24
#define AM_REG_MCUCTRL_CHIP_INFO_CLASS_M             0xFF000000
#define AM_REG_MCUCTRL_CHIP_INFO_CLASS(n)            (((uint32_t)(n) << 24) & 0xFF000000)
#define AM_REG_MCUCTRL_CHIP_INFO_CLASS_APOLLO        0x01000000

// Device flash size.
#define AM_REG_MCUCTRL_CHIP_INFO_FLASH_S             20
#define AM_REG_MCUCTRL_CHIP_INFO_FLASH_M             0x00F00000
#define AM_REG_MCUCTRL_CHIP_INFO_FLASH(n)            (((uint32_t)(n) << 20) & 0x00F00000)
#define AM_REG_MCUCTRL_CHIP_INFO_FLASH_256K          0x00300000
#define AM_REG_MCUCTRL_CHIP_INFO_FLASH_512K          0x00400000

// Device RAM size.
#define AM_REG_MCUCTRL_CHIP_INFO_RAM_S               16
#define AM_REG_MCUCTRL_CHIP_INFO_RAM_M               0x000F0000
#define AM_REG_MCUCTRL_CHIP_INFO_RAM(n)              (((uint32_t)(n) << 16) & 0x000F0000)
#define AM_REG_MCUCTRL_CHIP_INFO_RAM_32K             0x00000000
#define AM_REG_MCUCTRL_CHIP_INFO_RAM_64K             0x00010000

// Major device revision number.
#define AM_REG_MCUCTRL_CHIP_INFO_MAJORREV_S          12
#define AM_REG_MCUCTRL_CHIP_INFO_MAJORREV_M          0x0000F000
#define AM_REG_MCUCTRL_CHIP_INFO_MAJORREV(n)         (((uint32_t)(n) << 12) & 0x0000F000)

// Minor device revision number.
#define AM_REG_MCUCTRL_CHIP_INFO_MINORREV_S          8
#define AM_REG_MCUCTRL_CHIP_INFO_MINORREV_M          0x00000F00
#define AM_REG_MCUCTRL_CHIP_INFO_MINORREV(n)         (((uint32_t)(n) << 8) & 0x00000F00)

// Device package type.
#define AM_REG_MCUCTRL_CHIP_INFO_PKG_S               6
#define AM_REG_MCUCTRL_CHIP_INFO_PKG_M               0x000000C0
#define AM_REG_MCUCTRL_CHIP_INFO_PKG(n)              (((uint32_t)(n) << 6) & 0x000000C0)
#define AM_REG_MCUCTRL_CHIP_INFO_PKG_BGA             0x00000080
#define AM_REG_MCUCTRL_CHIP_INFO_PKG_CSP             0x000000C0

// Number of pins.
#define AM_REG_MCUCTRL_CHIP_INFO_PINS_S              3
#define AM_REG_MCUCTRL_CHIP_INFO_PINS_M              0x00000038
#define AM_REG_MCUCTRL_CHIP_INFO_PINS(n)             (((uint32_t)(n) << 3) & 0x00000038)
#define AM_REG_MCUCTRL_CHIP_INFO_PINS_41PINS         0x00000008
#define AM_REG_MCUCTRL_CHIP_INFO_PINS_64PINS         0x00000008

// Device temperature range.
#define AM_REG_MCUCTRL_CHIP_INFO_TEMP_S              1
#define AM_REG_MCUCTRL_CHIP_INFO_TEMP_M              0x00000006
#define AM_REG_MCUCTRL_CHIP_INFO_TEMP(n)             (((uint32_t)(n) << 1) & 0x00000006)
#define AM_REG_MCUCTRL_CHIP_INFO_TEMP_COMMERCIAL     0x00000000

// Device qualified.
#define AM_REG_MCUCTRL_CHIP_INFO_QUAL_S              0
#define AM_REG_MCUCTRL_CHIP_INFO_QUAL_M              0x00000001
#define AM_REG_MCUCTRL_CHIP_INFO_QUAL(n)             (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_CHIP_INFO_QUAL_PROTOTYPE      0x00000000
#define AM_REG_MCUCTRL_CHIP_INFO_QUAL_QUALIFIED      0x00000001

//*****************************************************************************
//
// MCUCTRL_CHIPID0 - Unique Chip ID 0
//
//*****************************************************************************
// Unique chip ID 0.
#define AM_REG_MCUCTRL_CHIPID0_VALUE_S               0
#define AM_REG_MCUCTRL_CHIPID0_VALUE_M               0xFFFFFFFF
#define AM_REG_MCUCTRL_CHIPID0_VALUE(n)              (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_MCUCTRL_CHIPID0_VALUE_APOLLO          0x00000000

//*****************************************************************************
//
// MCUCTRL_CHIPID1 - Unique Chip ID 1
//
//*****************************************************************************
// Unique chip ID 1.
#define AM_REG_MCUCTRL_CHIPID1_VALUE_S               0
#define AM_REG_MCUCTRL_CHIPID1_VALUE_M               0xFFFFFFFF
#define AM_REG_MCUCTRL_CHIPID1_VALUE(n)              (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_MCUCTRL_CHIPID1_VALUE_APOLLO          0x00000000

//*****************************************************************************
//
// MCUCTRL_CHIPREV - Chip Revision
//
//*****************************************************************************
// Chip Revision Number.
#define AM_REG_MCUCTRL_CHIPREV_REVISION_S            0
#define AM_REG_MCUCTRL_CHIPREV_REVISION_M            0x000000FF
#define AM_REG_MCUCTRL_CHIPREV_REVISION(n)           (((uint32_t)(n) << 0) & 0x000000FF)
#define AM_REG_MCUCTRL_CHIPREV_REVISION_APOLLO       0x00000000

//*****************************************************************************
//
// MCUCTRL_SUPPLYSRC - Memory and Core Voltage Supply Source Select Register
//
//*****************************************************************************
// Enables and Selects the Core Buck as the supply for the low-voltage power
// domain.
#define AM_REG_MCUCTRL_SUPPLYSRC_COREBUCKEN_S        1
#define AM_REG_MCUCTRL_SUPPLYSRC_COREBUCKEN_M        0x00000002
#define AM_REG_MCUCTRL_SUPPLYSRC_COREBUCKEN(n)       (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_SUPPLYSRC_COREBUCKEN_EN       0x00000002

// Enables and select the Memory Buck as the supply for the Flash and SRAM power
// domain.
#define AM_REG_MCUCTRL_SUPPLYSRC_MEMBUCKEN_S         0
#define AM_REG_MCUCTRL_SUPPLYSRC_MEMBUCKEN_M         0x00000001
#define AM_REG_MCUCTRL_SUPPLYSRC_MEMBUCKEN(n)        (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_SUPPLYSRC_MEMBUCKEN_EN        0x00000001

//*****************************************************************************
//
// MCUCTRL_SUPPLYSTATUS - Memory and Core Voltage Supply Source Status Register
//
//*****************************************************************************
// Indicates whether the Core low-voltage domain is supplied from the LDO or the
// Buck.
#define AM_REG_MCUCTRL_SUPPLYSTATUS_COREBUCKON_S     1
#define AM_REG_MCUCTRL_SUPPLYSTATUS_COREBUCKON_M     0x00000002
#define AM_REG_MCUCTRL_SUPPLYSTATUS_COREBUCKON(n)    (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_SUPPLYSTATUS_COREBUCKON_LDO   0x00000000
#define AM_REG_MCUCTRL_SUPPLYSTATUS_COREBUCKON_BUCK  0x00000002

// Indicate whether the Memory power domain is supplied from the LDO or the
// Buck.
#define AM_REG_MCUCTRL_SUPPLYSTATUS_MEMBUCKON_S      0
#define AM_REG_MCUCTRL_SUPPLYSTATUS_MEMBUCKON_M      0x00000001
#define AM_REG_MCUCTRL_SUPPLYSTATUS_MEMBUCKON(n)     (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_SUPPLYSTATUS_MEMBUCKON_LDO    0x00000000
#define AM_REG_MCUCTRL_SUPPLYSTATUS_MEMBUCKON_BUCK   0x00000001

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_CCRG - Calibrated Current Reference Generator Control
//
//*****************************************************************************
// Set the CCRG trim.
#define AM_REG_MCUCTRL_CCRG_CCRGTRIM_S               1
#define AM_REG_MCUCTRL_CCRG_CCRGTRIM_M               0x000001FE
#define AM_REG_MCUCTRL_CCRG_CCRGTRIM(n)              (((uint32_t)(n) << 1) & 0x000001FE)
#define AM_REG_MCUCTRL_CCRG_CCRGTRIM_TRIM_EN         0x00000002

// Power down the CCRG.
#define AM_REG_MCUCTRL_CCRG_CCRGPWD_S                0
#define AM_REG_MCUCTRL_CCRG_CCRGPWD_M                0x00000001
#define AM_REG_MCUCTRL_CCRG_CCRGPWD(n)               (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_CCRG_CCRGPWD_PWR_DN           0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_VREFGEN - Voltage Reference Generator Control
//
//*****************************************************************************
// Power Down, Calibrated Voltage Reference Generator.
#define AM_REG_MCUCTRL_VREFGEN_PWDCVREF_S            12
#define AM_REG_MCUCTRL_VREFGEN_PWDCVREF_M            0x00001000
#define AM_REG_MCUCTRL_VREFGEN_PWDCVREF(n)           (((uint32_t)(n) << 12) & 0x00001000)
#define AM_REG_MCUCTRL_VREFGEN_PWDCVREF_PWR_DN       0x00001000

// Calibrated Voltage Reference Generator VREF high adjust.
#define AM_REG_MCUCTRL_VREFGEN_CVREFHADJ_S           7
#define AM_REG_MCUCTRL_VREFGEN_CVREFHADJ_M           0x00000F80
#define AM_REG_MCUCTRL_VREFGEN_CVREFHADJ(n)          (((uint32_t)(n) << 7) & 0x00000F80)

// Calibrated Voltage Reference Generator VREF low adjust.
#define AM_REG_MCUCTRL_VREFGEN_CVREFLADJ_S           4
#define AM_REG_MCUCTRL_VREFGEN_CVREFLADJ_M           0x00000070
#define AM_REG_MCUCTRL_VREFGEN_CVREFLADJ(n)          (((uint32_t)(n) << 4) & 0x00000070)

// Calibrated Voltage Reference Generator tc trim (bottom transistor)
#define AM_REG_MCUCTRL_VREFGEN_CVREFBTRIM_S          0
#define AM_REG_MCUCTRL_VREFGEN_CVREFBTRIM_M          0x0000000F
#define AM_REG_MCUCTRL_VREFGEN_CVREFBTRIM(n)         (((uint32_t)(n) << 0) & 0x0000000F)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_BUCK - Analog Buck Control
//
//*****************************************************************************
// Memory Buck Bypass.
#define AM_REG_MCUCTRL_BUCK_BYPBUCKMEM_S             5
#define AM_REG_MCUCTRL_BUCK_BYPBUCKMEM_M             0x00000020
#define AM_REG_MCUCTRL_BUCK_BYPBUCKMEM(n)            (((uint32_t)(n) << 5) & 0x00000020)
#define AM_REG_MCUCTRL_BUCK_BYPBUCKMEM_BYPASS        0x00000020

// Memory buck Enable.
#define AM_REG_MCUCTRL_BUCK_ENBUCKMEM_S              4
#define AM_REG_MCUCTRL_BUCK_ENBUCKMEM_M              0x00000010
#define AM_REG_MCUCTRL_BUCK_ENBUCKMEM(n)             (((uint32_t)(n) << 4) & 0x00000010)
#define AM_REG_MCUCTRL_BUCK_ENBUCKMEM_EN             0x00000010

// Analog buck sleep.
#define AM_REG_MCUCTRL_BUCK_SLEEPBUCKANA_S           3
#define AM_REG_MCUCTRL_BUCK_SLEEPBUCKANA_M           0x00000008
#define AM_REG_MCUCTRL_BUCK_SLEEPBUCKANA(n)          (((uint32_t)(n) << 3) & 0x00000008)
#define AM_REG_MCUCTRL_BUCK_SLEEPBUCKANA_SLEEP       0x00000008

// Core buck enable.
#define AM_REG_MCUCTRL_BUCK_ENBUCKCORE_S             2
#define AM_REG_MCUCTRL_BUCK_ENBUCKCORE_M             0x00000004
#define AM_REG_MCUCTRL_BUCK_ENBUCKCORE(n)            (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_BUCK_ENBUCKCORE_EN            0x00000004

// Core buck bypass.
#define AM_REG_MCUCTRL_BUCK_BYPBUCKCORE_S            1
#define AM_REG_MCUCTRL_BUCK_BYPBUCKCORE_M            0x00000002
#define AM_REG_MCUCTRL_BUCK_BYPBUCKCORE(n)           (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_BUCK_BYPBUCKCORE_BYPASS       0x00000002

// Buck Register Software Override Enable.
#define AM_REG_MCUCTRL_BUCK_BUCKSWE_S                0
#define AM_REG_MCUCTRL_BUCK_BUCKSWE_M                0x00000001
#define AM_REG_MCUCTRL_BUCK_BUCKSWE(n)               (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_BUCK_BUCKSWE_OVERRIDE_DIS     0x00000000
#define AM_REG_MCUCTRL_BUCK_BUCKSWE_OVERRIDE_EN      0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_BUCK2 - Buck Control Reg2
//
//*****************************************************************************
// Enable/disable hysteresis on core buck converters internal comparators.
#define AM_REG_MCUCTRL_BUCK2_BUCKLFCLKSEL_S          10
#define AM_REG_MCUCTRL_BUCK2_BUCKLFCLKSEL_M          0x00000C00
#define AM_REG_MCUCTRL_BUCK2_BUCKLFCLKSEL(n)         (((uint32_t)(n) << 10) & 0x00000C00)

// Enable/disable hysteresis on core buck converters internal comparators.
#define AM_REG_MCUCTRL_BUCK2_HYSTBUCKCORE_S          9
#define AM_REG_MCUCTRL_BUCK2_HYSTBUCKCORE_M          0x00000200
#define AM_REG_MCUCTRL_BUCK2_HYSTBUCKCORE(n)         (((uint32_t)(n) << 9) & 0x00000200)
#define AM_REG_MCUCTRL_BUCK2_HYSTBUCKCORE_DIS        0x00000000
#define AM_REG_MCUCTRL_BUCK2_HYSTBUCKCORE_EN         0x00000200

// Enable/disable hysteresis on memory buck converters internal comparators.
#define AM_REG_MCUCTRL_BUCK2_HYSTBUCKMEM_S           8
#define AM_REG_MCUCTRL_BUCK2_HYSTBUCKMEM_M           0x00000100
#define AM_REG_MCUCTRL_BUCK2_HYSTBUCKMEM(n)          (((uint32_t)(n) << 8) & 0x00000100)
#define AM_REG_MCUCTRL_BUCK2_HYSTBUCKMEM_DIS         0x00000000
#define AM_REG_MCUCTRL_BUCK2_HYSTBUCKMEM_EN          0x00000100

// Flash Buck Time On Select
#define AM_REG_MCUCTRL_BUCK2_BMEMTONSEL_S            4
#define AM_REG_MCUCTRL_BUCK2_BMEMTONSEL_M            0x000000F0
#define AM_REG_MCUCTRL_BUCK2_BMEMTONSEL(n)           (((uint32_t)(n) << 4) & 0x000000F0)

// Core Buck Time On Select
#define AM_REG_MCUCTRL_BUCK2_BCORETONSEL_S           0
#define AM_REG_MCUCTRL_BUCK2_BCORETONSEL_M           0x0000000F
#define AM_REG_MCUCTRL_BUCK2_BCORETONSEL(n)          (((uint32_t)(n) << 0) & 0x0000000F)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_LDOREG1 - Analog LDO Reg 1
//
//*****************************************************************************
// CORE LDO Trim R3.
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR3_S       19
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR3_M       0x03F80000
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR3(n)      (((uint32_t)(n) << 19) & 0x03F80000)

// CORE LDO Trim R2.
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR2_S       9
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR2_M       0x0007FE00
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR2(n)      (((uint32_t)(n) << 9) & 0x0007FE00)

// CORE LDO Trim R1.
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR1_S       0
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR1_M       0x000001FF
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR1(n)      (((uint32_t)(n) << 0) & 0x000001FF)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_LDOREG2 - LDO Control Register 2
//
//*****************************************************************************
// Analog LDO Power Down.
#define AM_REG_MCUCTRL_LDOREG2_PWDANALDO_S           22
#define AM_REG_MCUCTRL_LDOREG2_PWDANALDO_M           0x00400000
#define AM_REG_MCUCTRL_LDOREG2_PWDANALDO(n)          (((uint32_t)(n) << 22) & 0x00400000)
#define AM_REG_MCUCTRL_LDOREG2_PWDANALDO_PWR_DN      0x00400000

// FLASH LDO Power Down.
#define AM_REG_MCUCTRL_LDOREG2_PWDMEMLDO_S           21
#define AM_REG_MCUCTRL_LDOREG2_PWDMEMLDO_M           0x00200000
#define AM_REG_MCUCTRL_LDOREG2_PWDMEMLDO(n)          (((uint32_t)(n) << 21) & 0x00200000)
#define AM_REG_MCUCTRL_LDOREG2_PWDMEMLDO_PWR_DN      0x00200000

// CORE LDO Power Down.
#define AM_REG_MCUCTRL_LDOREG2_PWDCORELDO_S          20
#define AM_REG_MCUCTRL_LDOREG2_PWDCORELDO_M          0x00100000
#define AM_REG_MCUCTRL_LDOREG2_PWDCORELDO(n)         (((uint32_t)(n) << 20) & 0x00100000)
#define AM_REG_MCUCTRL_LDOREG2_PWDCORELDO_PWR_DN     0x00100000

// Analog LDO Sleep.
#define AM_REG_MCUCTRL_LDOREG2_SLEEPANALDO_S         19
#define AM_REG_MCUCTRL_LDOREG2_SLEEPANALDO_M         0x00080000
#define AM_REG_MCUCTRL_LDOREG2_SLEEPANALDO(n)        (((uint32_t)(n) << 19) & 0x00080000)
#define AM_REG_MCUCTRL_LDOREG2_SLEEPANALDO_SLEEP     0x00080000

// FLASH LDO Sleep.
#define AM_REG_MCUCTRL_LDOREG2_SLEEPMEMLDO_S         18
#define AM_REG_MCUCTRL_LDOREG2_SLEEPMEMLDO_M         0x00040000
#define AM_REG_MCUCTRL_LDOREG2_SLEEPMEMLDO(n)        (((uint32_t)(n) << 18) & 0x00040000)
#define AM_REG_MCUCTRL_LDOREG2_SLEEPMEMLDO_SLEEP     0x00040000

// CORE LDO Sleep.
#define AM_REG_MCUCTRL_LDOREG2_SLEEPCORELDO_S        17
#define AM_REG_MCUCTRL_LDOREG2_SLEEPCORELDO_M        0x00020000
#define AM_REG_MCUCTRL_LDOREG2_SLEEPCORELDO(n)       (((uint32_t)(n) << 17) & 0x00020000)
#define AM_REG_MCUCTRL_LDOREG2_SLEEPCORELDO_SLEEP    0x00020000

// Analog LDO VREF Select.
#define AM_REG_MCUCTRL_LDOREG2_VREFSELANALDO_S       16
#define AM_REG_MCUCTRL_LDOREG2_VREFSELANALDO_M       0x00010000
#define AM_REG_MCUCTRL_LDOREG2_VREFSELANALDO(n)      (((uint32_t)(n) << 16) & 0x00010000)
#define AM_REG_MCUCTRL_LDOREG2_VREFSELANALDO_SELECT  0x00010000

// SRAM LDO VREF Select.
#define AM_REG_MCUCTRL_LDOREG2_VREFSELSRAMLDO_S      15
#define AM_REG_MCUCTRL_LDOREG2_VREFSELSRAMLDO_M      0x00008000
#define AM_REG_MCUCTRL_LDOREG2_VREFSELSRAMLDO(n)     (((uint32_t)(n) << 15) & 0x00008000)
#define AM_REG_MCUCTRL_LDOREG2_VREFSELSRAMLDO_SELECT 0x00008000

// Flash LDO VREF Select.
#define AM_REG_MCUCTRL_LDOREG2_VREFSELFLASHLDO_S     14
#define AM_REG_MCUCTRL_LDOREG2_VREFSELFLASHLDO_M     0x00004000
#define AM_REG_MCUCTRL_LDOREG2_VREFSELFLASHLDO(n)    (((uint32_t)(n) << 14) & 0x00004000)
#define AM_REG_MCUCTRL_LDOREG2_VREFSELFLASHLDO_SELECT 0x00004000

// Core LDO VREF Select.
#define AM_REG_MCUCTRL_LDOREG2_VREFSELCORELDO_S      13
#define AM_REG_MCUCTRL_LDOREG2_VREFSELCORELDO_M      0x00002000
#define AM_REG_MCUCTRL_LDOREG2_VREFSELCORELDO(n)     (((uint32_t)(n) << 13) & 0x00002000)
#define AM_REG_MCUCTRL_LDOREG2_VREFSELCORELDO_SELECT 0x00002000

// Analog LDO Trim.
#define AM_REG_MCUCTRL_LDOREG2_TRIMANALDO_S          9
#define AM_REG_MCUCTRL_LDOREG2_TRIMANALDO_M          0x00001E00
#define AM_REG_MCUCTRL_LDOREG2_TRIMANALDO(n)         (((uint32_t)(n) << 9) & 0x00001E00)

// SRAM LDO Trim.
#define AM_REG_MCUCTRL_LDOREG2_TRIMSRAMLDO_S         5
#define AM_REG_MCUCTRL_LDOREG2_TRIMSRAMLDO_M         0x000001E0
#define AM_REG_MCUCTRL_LDOREG2_TRIMSRAMLDO(n)        (((uint32_t)(n) << 5) & 0x000001E0)

// Mem LDO Trim.
#define AM_REG_MCUCTRL_LDOREG2_TRIMLDOMEM_S          1
#define AM_REG_MCUCTRL_LDOREG2_TRIMLDOMEM_M          0x0000001E
#define AM_REG_MCUCTRL_LDOREG2_TRIMLDOMEM(n)         (((uint32_t)(n) << 1) & 0x0000001E)

// LDO2 Software Override Enable.
#define AM_REG_MCUCTRL_LDOREG2_LDO2SWE_S             0
#define AM_REG_MCUCTRL_LDOREG2_LDO2SWE_M             0x00000001
#define AM_REG_MCUCTRL_LDOREG2_LDO2SWE(n)            (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_LDOREG2_LDO2SWE_OVERRIDE_DIS  0x00000000
#define AM_REG_MCUCTRL_LDOREG2_LDO2SWE_OVERRIDE_EN   0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_LDOREG3 - LDO Control Register 3
//
//*****************************************************************************
// SRAM LDO Power Down.
#define AM_REG_MCUCTRL_LDOREG3_PWDSRAMLDO_S          2
#define AM_REG_MCUCTRL_LDOREG3_PWDSRAMLDO_M          0x00000004
#define AM_REG_MCUCTRL_LDOREG3_PWDSRAMLDO(n)         (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_LDOREG3_PWDSRAMLDO_PWR_DN     0x00000004

// Bypass SRAM LDO.
#define AM_REG_MCUCTRL_LDOREG3_SLEEPSRAMLDO_S        1
#define AM_REG_MCUCTRL_LDOREG3_SLEEPSRAMLDO_M        0x00000002
#define AM_REG_MCUCTRL_LDOREG3_SLEEPSRAMLDO(n)       (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_LDOREG3_SLEEPSRAMLDO_BYPASS   0x00000002

// LDO3 Software Override Enable.
#define AM_REG_MCUCTRL_LDOREG3_LDO3SWE_S             0
#define AM_REG_MCUCTRL_LDOREG3_LDO3SWE_M             0x00000001
#define AM_REG_MCUCTRL_LDOREG3_LDO3SWE(n)            (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_LDOREG3_LDO3SWE_OVERRIDE_DIS  0x00000000
#define AM_REG_MCUCTRL_LDOREG3_LDO3SWE_OVERRIDE_EN   0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_UNCALVREF - Uncalibrated VREF Control
//
//*****************************************************************************
// Uncalibrated vref power down
#define AM_REG_MCUCTRL_UNCALVREF_PWDUNCALVREF_S      0
#define AM_REG_MCUCTRL_UNCALVREF_PWDUNCALVREF_M      0x00000001
#define AM_REG_MCUCTRL_UNCALVREF_PWDUNCALVREF(n)     (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_UNCALVREF_PWDUNCALVREF_PWR_DN 0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_HFRC - HFRC Control
//
//*****************************************************************************
// Default HFRC frequency tune value
#define AM_REG_MCUCTRL_HFRC_HFTUNE_S                 10
#define AM_REG_MCUCTRL_HFRC_HFTUNE_M                 0x001FFC00
#define AM_REG_MCUCTRL_HFRC_HFTUNE(n)                (((uint32_t)(n) << 10) & 0x001FFC00)

// Power Up Delay before enabling clocks.
#define AM_REG_MCUCTRL_HFRC_HFRCDEL_S                5
#define AM_REG_MCUCTRL_HFRC_HFRCDEL_M                0x000003E0
#define AM_REG_MCUCTRL_HFRC_HFRCDEL(n)               (((uint32_t)(n) << 5) & 0x000003E0)

// Power Down HFRC.
#define AM_REG_MCUCTRL_HFRC_DISABLEHFRC_S            4
#define AM_REG_MCUCTRL_HFRC_DISABLEHFRC_M            0x00000010
#define AM_REG_MCUCTRL_HFRC_DISABLEHFRC(n)           (((uint32_t)(n) << 4) & 0x00000010)
#define AM_REG_MCUCTRL_HFRC_DISABLEHFRC_PWRUP        0x00000000
#define AM_REG_MCUCTRL_HFRC_DISABLEHFRC_PWRDN        0x00000010

// Force HFRC Output Low.
#define AM_REG_MCUCTRL_HFRC_OUTLOWHFRC_S             3
#define AM_REG_MCUCTRL_HFRC_OUTLOWHFRC_M             0x00000008
#define AM_REG_MCUCTRL_HFRC_OUTLOWHFRC(n)            (((uint32_t)(n) << 3) & 0x00000008)
#define AM_REG_MCUCTRL_HFRC_OUTLOWHFRC_EN            0x00000000
#define AM_REG_MCUCTRL_HFRC_OUTLOWHFRC_OUTLOW        0x00000008

// HFRC Frequency Tune trim bits
#define AM_REG_MCUCTRL_HFRC_TRIMCOEFFHFRC_S          1
#define AM_REG_MCUCTRL_HFRC_TRIMCOEFFHFRC_M          0x00000006
#define AM_REG_MCUCTRL_HFRC_TRIMCOEFFHFRC(n)         (((uint32_t)(n) << 1) & 0x00000006)

// HFRC Software Override Enable.
#define AM_REG_MCUCTRL_HFRC_HFRCSWE_S                0
#define AM_REG_MCUCTRL_HFRC_HFRCSWE_M                0x00000001
#define AM_REG_MCUCTRL_HFRC_HFRCSWE(n)               (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_HFRC_HFRCSWE_OVERRIDE_DIS     0x00000000
#define AM_REG_MCUCTRL_HFRC_HFRCSWE_OVERRIDE_EN      0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_LFRC - LFRC Control
//
//*****************************************************************************
// LFRC Reset.
#define AM_REG_MCUCTRL_LFRC_RESETLFRC_S              7
#define AM_REG_MCUCTRL_LFRC_RESETLFRC_M              0x00000080
#define AM_REG_MCUCTRL_LFRC_RESETLFRC(n)             (((uint32_t)(n) << 7) & 0x00000080)
#define AM_REG_MCUCTRL_LFRC_RESETLFRC_EN             0x00000000
#define AM_REG_MCUCTRL_LFRC_RESETLFRC_RESET          0x00000080

// Power Down LFRC.
#define AM_REG_MCUCTRL_LFRC_PWDLFRC_S                6
#define AM_REG_MCUCTRL_LFRC_PWDLFRC_M                0x00000040
#define AM_REG_MCUCTRL_LFRC_PWDLFRC(n)               (((uint32_t)(n) << 6) & 0x00000040)
#define AM_REG_MCUCTRL_LFRC_PWDLFRC_PWRUP            0x00000000
#define AM_REG_MCUCTRL_LFRC_PWDLFRC_PWRDN            0x00000040

// LFRC Frequency Tune trim bits
#define AM_REG_MCUCTRL_LFRC_TRIMTUNELFRC_S           1
#define AM_REG_MCUCTRL_LFRC_TRIMTUNELFRC_M           0x0000003E
#define AM_REG_MCUCTRL_LFRC_TRIMTUNELFRC(n)          (((uint32_t)(n) << 1) & 0x0000003E)

// LFRC Software Override Enable.
#define AM_REG_MCUCTRL_LFRC_LFRCSWE_S                0
#define AM_REG_MCUCTRL_LFRC_LFRCSWE_M                0x00000001
#define AM_REG_MCUCTRL_LFRC_LFRCSWE(n)               (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_LFRC_LFRCSWE_OVERRIDE_DIS     0x00000000
#define AM_REG_MCUCTRL_LFRC_LFRCSWE_OVERRIDE_EN      0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_IOCTRL - IO Master and Slave Control (NOT CURRENTLY USED)
//
//*****************************************************************************
// IO Master 1 Delay Value.
#define AM_REG_MCUCTRL_IOCTRL_IOM1DEL_S              6
#define AM_REG_MCUCTRL_IOCTRL_IOM1DEL_M              0x000001C0
#define AM_REG_MCUCTRL_IOCTRL_IOM1DEL(n)             (((uint32_t)(n) << 6) & 0x000001C0)

// IO Master 0 Delay Value.
#define AM_REG_MCUCTRL_IOCTRL_IOM0DEL_S              3
#define AM_REG_MCUCTRL_IOCTRL_IOM0DEL_M              0x00000038
#define AM_REG_MCUCTRL_IOCTRL_IOM0DEL(n)             (((uint32_t)(n) << 3) & 0x00000038)

// IO Slave Delay Value.
#define AM_REG_MCUCTRL_IOCTRL_IOSDEL_S               0
#define AM_REG_MCUCTRL_IOCTRL_IOSDEL_M               0x00000007
#define AM_REG_MCUCTRL_IOCTRL_IOSDEL(n)              (((uint32_t)(n) << 0) & 0x00000007)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
//*****************************************************************************
//
// MCUCTRL_BANDGAPEN - Band Gap Enable
//
//*****************************************************************************
// Bandgap Enable
#define AM_REG_MCUCTRL_BANDGAPEN_BGPEN_S             0
#define AM_REG_MCUCTRL_BANDGAPEN_BGPEN_M             0x00000001
#define AM_REG_MCUCTRL_BANDGAPEN_BGPEN(n)            (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_BANDGAPEN_BGPEN_DIS           0x00000000
#define AM_REG_MCUCTRL_BANDGAPEN_BGPEN_EN            0x00000001

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_TEMPSCTRL - Temp Sensor Control
//
//*****************************************************************************
// Bandgap trim bits.
#define AM_REG_MCUCTRL_TEMPSCTRL_BGTRIM_S            4
#define AM_REG_MCUCTRL_TEMPSCTRL_BGTRIM_M            0x000000F0
#define AM_REG_MCUCTRL_TEMPSCTRL_BGTRIM(n)           (((uint32_t)(n) << 4) & 0x000000F0)

// Temperature sensor trim bits.
#define AM_REG_MCUCTRL_TEMPSCTRL_TSTRIM_S            0
#define AM_REG_MCUCTRL_TEMPSCTRL_TSTRIM_M            0x0000000F
#define AM_REG_MCUCTRL_TEMPSCTRL_TSTRIM(n)           (((uint32_t)(n) << 0) & 0x0000000F)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_ADCPWRDLY - ADC Power Up Delay Control
//
//*****************************************************************************
// ADC IP Reference Voltage Trim.
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCREFTRIM_S        16
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCREFTRIM_M        0x001F0000
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCREFTRIM(n)       (((uint32_t)(n) << 16) & 0x001F0000)

// ADC IP enable delay in HFRC/16 increments.
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR1_S           8
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR1_M           0x0000FF00
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR1(n)          (((uint32_t)(n) << 8) & 0x0000FF00)

// ADC Power Switch delay in HFRC/16 increments.
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR0_S           0
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR0_M           0x000000FF
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR0(n)          (((uint32_t)(n) << 0) & 0x000000FF)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_ADCTSBGPWD - ADC Temp Sensor Bandgap Power Control
//
//*****************************************************************************
// Enable the Temp Sensor Power when set to 1 if the ADCTSBGSWE bit is set.
#define AM_REG_MCUCTRL_ADCTSBGPWD_TSPEN_S            2
#define AM_REG_MCUCTRL_ADCTSBGPWD_TSPEN_M            0x00000004
#define AM_REG_MCUCTRL_ADCTSBGPWD_TSPEN(n)           (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_ADCTSBGPWD_TSPEN_DIS          0x00000000
#define AM_REG_MCUCTRL_ADCTSBGPWD_TSPEN_EN           0x00000004

// Enable the ADC Power Switch on when set to 1 if the ADCTSBGSWE bit is set.
#define AM_REG_MCUCTRL_ADCTSBGPWD_ADCPSEN_S          1
#define AM_REG_MCUCTRL_ADCTSBGPWD_ADCPSEN_M          0x00000002
#define AM_REG_MCUCTRL_ADCTSBGPWD_ADCPSEN(n)         (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_ADCTSBGPWD_ADCPSEN_DIS        0x00000000
#define AM_REG_MCUCTRL_ADCTSBGPWD_ADCPSEN_EN         0x00000002

// ADC Temperature Sensor Bandgap Software Override Enable
#define AM_REG_MCUCTRL_ADCTSBGPWD_ADCTSBGSWE_S       0
#define AM_REG_MCUCTRL_ADCTSBGPWD_ADCTSBGSWE_M       0x00000001
#define AM_REG_MCUCTRL_ADCTSBGPWD_ADCTSBGSWE(n)      (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_ADCTSBGPWD_ADCTSBGSWE_OVERRIDE_DIS 0x00000000
#define AM_REG_MCUCTRL_ADCTSBGPWD_ADCTSBGSWE_OVERRIDE_EN 0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_XTALCTRL - XTAL Oscillator Control
//
//*****************************************************************************
// XTAL Oscillator Power Down Comparator.
#define AM_REG_MCUCTRL_XTALCTRL_PWDCMPRXTAL_S        11
#define AM_REG_MCUCTRL_XTALCTRL_PWDCMPRXTAL_M        0x00000800
#define AM_REG_MCUCTRL_XTALCTRL_PWDCMPRXTAL(n)       (((uint32_t)(n) << 11) & 0x00000800)
#define AM_REG_MCUCTRL_XTALCTRL_PWDCMPRXTAL_PWRUPCOMP 0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_PWDCMPRXTAL_PWRDNCOMP 0x00000800

// XTAL Oscillator Power Down Core.
#define AM_REG_MCUCTRL_XTALCTRL_PWDCOREXTAL_S        10
#define AM_REG_MCUCTRL_XTALCTRL_PWDCOREXTAL_M        0x00000400
#define AM_REG_MCUCTRL_XTALCTRL_PWDCOREXTAL(n)       (((uint32_t)(n) << 10) & 0x00000400)
#define AM_REG_MCUCTRL_XTALCTRL_PWDCOREXTAL_PWRUPCORE 0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_PWDCOREXTAL_PWRDNCORE 0x00000400

// Auto-calibration delay control
#define AM_REG_MCUCTRL_XTALCTRL_ACWARMUP_S           8
#define AM_REG_MCUCTRL_XTALCTRL_ACWARMUP_M           0x00000300
#define AM_REG_MCUCTRL_XTALCTRL_ACWARMUP(n)          (((uint32_t)(n) << 8) & 0x00000300)
#define AM_REG_MCUCTRL_XTALCTRL_ACWARMUP_1SEC        0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_ACWARMUP_2SEC        0x00000100
#define AM_REG_MCUCTRL_XTALCTRL_ACWARMUP_4SEC        0x00000200
#define AM_REG_MCUCTRL_XTALCTRL_ACWARMUP_8SEC        0x00000300

// XTAL Oscillator Bypass Comparator.
#define AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_S        7
#define AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_M        0x00000080
#define AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL(n)       (((uint32_t)(n) << 7) & 0x00000080)
#define AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_USECOMP  0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_BYPCOMP  0x00000080

// XTAL Oscillator Disable Feedback.
#define AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_S       6
#define AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_M       0x00000040
#define AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL(n)      (((uint32_t)(n) << 6) & 0x00000040)
#define AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_EN      0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_DIS     0x00000040

// XTAL Oscillator Core bias trim bits
#define AM_REG_MCUCTRL_XTALCTRL_TRIMCOREBXTAL_S      1
#define AM_REG_MCUCTRL_XTALCTRL_TRIMCOREBXTAL_M      0x0000003E
#define AM_REG_MCUCTRL_XTALCTRL_TRIMCOREBXTAL(n)     (((uint32_t)(n) << 1) & 0x0000003E)

// XTAL Software Override Enable.
#define AM_REG_MCUCTRL_XTALCTRL_XTALSWE_S            0
#define AM_REG_MCUCTRL_XTALCTRL_XTALSWE_M            0x00000001
#define AM_REG_MCUCTRL_XTALCTRL_XTALSWE(n)           (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_DIS 0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_EN  0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
//*****************************************************************************
//
// MCUCTRL_SRAMPWDINSLEEP - Powerdown an SRAM Bank in Deep Sleep mode
//
//*****************************************************************************
// Force SRAM Bank 7 to powerdown in deep sleep mode, causing the contents of
// the bank to be lost.
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK7_S        7
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK7_M        0x00000080
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK7(n)       (((uint32_t)(n) << 7) & 0x00000080)
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK7_NORMAL   0x00000000
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK7_PWRDN_IN_DEEPSLEEP 0x00000080

// Force SRAM Bank 6 to powerdown in deep sleep mode, causing the contents of
// the bank to be lost.
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK6_S        6
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK6_M        0x00000040
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK6(n)       (((uint32_t)(n) << 6) & 0x00000040)
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK6_NORMAL   0x00000000
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK6_PWRDN_IN_DEEPSLEEP 0x00000040

// Force SRAM Bank 5 to powerdown in deep sleep mode, causing the contents of
// the bank to be lost.
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK5_S        5
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK5_M        0x00000020
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK5(n)       (((uint32_t)(n) << 5) & 0x00000020)
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK5_NORMAL   0x00000000
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK5_PWRDN_IN_DEEPSLEEP 0x00000020

// Force SRAM Bank 4 to powerdown in deep sleep mode, causing the contents of
// the bank to be lost.
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK4_S        4
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK4_M        0x00000010
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK4(n)       (((uint32_t)(n) << 4) & 0x00000010)
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK4_NORMAL   0x00000000
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK4_PWRDN_IN_DEEPSLEEP 0x00000010

// Force SRAM Bank 3 to powerdown in deep sleep mode, causing the contents of
// the bank to be lost.
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK3_S        3
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK3_M        0x00000008
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK3(n)       (((uint32_t)(n) << 3) & 0x00000008)
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK3_NORMAL   0x00000000
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK3_PWRDN_IN_DEEPSLEEP 0x00000008

// Force SRAM Bank 2 to powerdown in deep sleep mode, causing the contents of
// the bank to be lost.
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK2_S        2
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK2_M        0x00000004
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK2(n)       (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK2_NORMAL   0x00000000
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK2_PWRDN_IN_DEEPSLEEP 0x00000004

// Force SRAM Bank 1 to powerdown in deep sleep mode, causing the contents of
// the bank to be lost.
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK1_S        1
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK1_M        0x00000002
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK1(n)       (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK1_NORMAL   0x00000000
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK1_PWRDN_IN_DEEPSLEEP 0x00000002

// Force SRAM Bank 0 to powerdown in deep sleep mode, causing the contents of
// the bank to be lost.
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK0_S        0
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK0_M        0x00000001
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK0(n)       (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK0_NORMAL   0x00000000
#define AM_REG_MCUCTRL_SRAMPWDINSLEEP_BANK0_PWRDN_IN_DEEPSLEEP 0x00000001

//*****************************************************************************
//
// MCUCTRL_SRAMPWRDIS - Disables individual banks of the SRAM array
//
//*****************************************************************************
// Remove power from SRAM Bank 7 which will cause an access to its address space
// to generate a Hard Fault.
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK7_S            7
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK7_M            0x00000080
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK7(n)           (((uint32_t)(n) << 7) & 0x00000080)
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK7_DIS          0x00000080

// Remove power from SRAM Bank 6 which will cause an access to its address space
// to generate a Hard Fault.
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK6_S            6
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK6_M            0x00000040
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK6(n)           (((uint32_t)(n) << 6) & 0x00000040)
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK6_DIS          0x00000040

// Remove power from SRAM Bank 5 which will cause an access to its address space
// to generate a Hard Fault.
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK5_S            5
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK5_M            0x00000020
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK5(n)           (((uint32_t)(n) << 5) & 0x00000020)
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK5_DIS          0x00000020

// Remove power from SRAM Bank 4 which will cause an access to its address space
// to generate a Hard Fault.
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK4_S            4
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK4_M            0x00000010
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK4(n)           (((uint32_t)(n) << 4) & 0x00000010)
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK4_DIS          0x00000010

// Remove power from SRAM Bank 3 which will cause an access to its address space
// to generate a Hard Fault.
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK3_S            3
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK3_M            0x00000008
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK3(n)           (((uint32_t)(n) << 3) & 0x00000008)
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK3_DIS          0x00000008

// Remove power from SRAM Bank 2 which will cause an access to its address space
// to generate a Hard Fault.
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK2_S            2
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK2_M            0x00000004
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK2(n)           (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK2_DIS          0x00000004

// Remove power from SRAM Bank 1 which will cause an access to its address space
// to generate a Hard Fault.
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK1_S            1
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK1_M            0x00000002
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK1(n)           (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK1_DIS          0x00000002

// Remove power from SRAM Bank 0 which will cause an access to its address space
// to generate a Hard Fault.
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK0_S            0
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK0_M            0x00000001
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK0(n)           (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_SRAMPWRDIS_BANK0_DIS          0x00000001

//*****************************************************************************
//
// MCUCTRL_FLASHPWRDIS - Disables individual banks of the Flash array
//
//*****************************************************************************
// Remove power from Flash Bank 1 which will cause an access to its address
// space to generate a Hard Fault.
#define AM_REG_MCUCTRL_FLASHPWRDIS_BANK1_S           1
#define AM_REG_MCUCTRL_FLASHPWRDIS_BANK1_M           0x00000002
#define AM_REG_MCUCTRL_FLASHPWRDIS_BANK1(n)          (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_FLASHPWRDIS_BANK1_DIS         0x00000002

// Remove power from Flash Bank 0 which will cause an access to its address
// space to generate a Hard Fault.
#define AM_REG_MCUCTRL_FLASHPWRDIS_BANK0_S           0
#define AM_REG_MCUCTRL_FLASHPWRDIS_BANK0_M           0x00000001
#define AM_REG_MCUCTRL_FLASHPWRDIS_BANK0(n)          (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_FLASHPWRDIS_BANK0_DIS         0x00000001

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_PWRONRSTDLY - Power On Sequence Delay Register
//
//*****************************************************************************
// Power On Sequence Delay 3. Aanalog stabalizing after ldo_vref_sel and
// pwd_cmp_1p8hp assertion. Increments of 32768/HFRC ns.
#define AM_REG_MCUCTRL_PWRONRSTDLY_PORDLY3_S         16
#define AM_REG_MCUCTRL_PWRONRSTDLY_PORDLY3_M         0x00FF0000
#define AM_REG_MCUCTRL_PWRONRSTDLY_PORDLY3(n)        (((uint32_t)(n) << 16) & 0x00FF0000)

// Power On Sequence Delay 2. Delay before ldo_vref_sel and pwd_cmp_1p8hp
// assertion. Increments of 32768/HFRC ns.
#define AM_REG_MCUCTRL_PWRONRSTDLY_PORDLY2_S         8
#define AM_REG_MCUCTRL_PWRONRSTDLY_PORDLY2_M         0x0000FF00
#define AM_REG_MCUCTRL_PWRONRSTDLY_PORDLY2(n)        (((uint32_t)(n) << 8) & 0x0000FF00)

// Power On Sequence Delay 1. Delay before cvrg_hs_n de-assertion. Increments of
// 32768/HFRC ns.
#define AM_REG_MCUCTRL_PWRONRSTDLY_PORDLY1_S         0
#define AM_REG_MCUCTRL_PWRONRSTDLY_PORDLY1_M         0x000000FF
#define AM_REG_MCUCTRL_PWRONRSTDLY_PORDLY1(n)        (((uint32_t)(n) << 0) & 0x000000FF)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_PWRSEQ1ANA - Power on analog sequence1
//
//*****************************************************************************
// Calibrated Voltage Reference Generator speed mode (high speed = 0, low speed
// mode = 1)
#define AM_REG_MCUCTRL_PWRSEQ1ANA_CVRGHSN_S          1
#define AM_REG_MCUCTRL_PWRSEQ1ANA_CVRGHSN_M          0x00000002
#define AM_REG_MCUCTRL_PWRSEQ1ANA_CVRGHSN(n)         (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_PWRSEQ1ANA_CVRGHSN_HIGH       0x00000000
#define AM_REG_MCUCTRL_PWRSEQ1ANA_CVRGHSN_LOW        0x00000002

// Analog Power sequence signals Software Override Enable.
#define AM_REG_MCUCTRL_PWRSEQ1ANA_PWRSEQ1SWE_S       0
#define AM_REG_MCUCTRL_PWRSEQ1ANA_PWRSEQ1SWE_M       0x00000001
#define AM_REG_MCUCTRL_PWRSEQ1ANA_PWRSEQ1SWE(n)      (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_PWRSEQ1ANA_PWRSEQ1SWE_OVERRIDE_DIS 0x00000000
#define AM_REG_MCUCTRL_PWRSEQ1ANA_PWRSEQ1SWE_OVERRIDE_EN 0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_PWRSEQ2ANA - Power on analog sequence2
//
//*****************************************************************************
// Select to the ALDO and CLDO to use uncalibrated (0) or calibrated (1) voltage
// reference.
#define AM_REG_MCUCTRL_PWRSEQ2ANA_LDOVREFSEL_S       1
#define AM_REG_MCUCTRL_PWRSEQ2ANA_LDOVREFSEL_M       0x00000002
#define AM_REG_MCUCTRL_PWRSEQ2ANA_LDOVREFSEL(n)      (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_PWRSEQ2ANA_LDOVREFSEL_UNCALIBRATED 0x00000000
#define AM_REG_MCUCTRL_PWRSEQ2ANA_LDOVREFSEL_CALIBRATED 0x00000002

// Analog Power sequence signals Software Override Enable.
#define AM_REG_MCUCTRL_PWRSEQ2ANA_PWRSEQ2SWE_S       0
#define AM_REG_MCUCTRL_PWRSEQ2ANA_PWRSEQ2SWE_M       0x00000001
#define AM_REG_MCUCTRL_PWRSEQ2ANA_PWRSEQ2SWE(n)      (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_PWRSEQ2ANA_PWRSEQ2SWE_OVERRIDE_DIS 0x00000000
#define AM_REG_MCUCTRL_PWRSEQ2ANA_PWRSEQ2SWE_OVERRIDE_EN 0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_PWRSEQ3ANA - Power on analog sequence3
//
//*****************************************************************************
// reset power stable comparator power down.
#define AM_REG_MCUCTRL_PWRSEQ3ANA_PWDCMPP18HP_S      1
#define AM_REG_MCUCTRL_PWRSEQ3ANA_PWDCMPP18HP_M      0x00000002
#define AM_REG_MCUCTRL_PWRSEQ3ANA_PWDCMPP18HP(n)     (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_PWRSEQ3ANA_PWDCMPP18HP_PWR_DN 0x00000002

// Analog Power sequence signals Software Override Enable.
#define AM_REG_MCUCTRL_PWRSEQ3ANA_PWRSEQ3SWE_S       0
#define AM_REG_MCUCTRL_PWRSEQ3ANA_PWRSEQ3SWE_M       0x00000001
#define AM_REG_MCUCTRL_PWRSEQ3ANA_PWRSEQ3SWE(n)      (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_PWRSEQ3ANA_PWRSEQ3SWE_OVERRIDE_DIS 0x00000000
#define AM_REG_MCUCTRL_PWRSEQ3ANA_PWRSEQ3SWE_OVERRIDE_EN 0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// MCUCTRL_BOOTLOADERLOW - Determines whether the bootloader code is visible at
// address 0x00000000
//
//*****************************************************************************
// Determines whether the bootloader code is visible at address 0x00000000 or
// not.
#define AM_REG_MCUCTRL_BOOTLOADERLOW_VALUE_S         0
#define AM_REG_MCUCTRL_BOOTLOADERLOW_VALUE_M         0x00000001
#define AM_REG_MCUCTRL_BOOTLOADERLOW_VALUE(n)        (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_BOOTLOADERLOW_VALUE_ADDR0     0x00000001

// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// MCUCTRL_SHADOWVALID - Register to indicate whether the shadow registers have
// been successfully loaded from the Flash Information Space.
//
//*****************************************************************************
// Indicates whether the shadow registers contain valid data from the Flash
// Information Space.
#define AM_REG_MCUCTRL_SHADOWVALID_VALID_S           0
#define AM_REG_MCUCTRL_SHADOWVALID_VALID_M           0x00000001
#define AM_REG_MCUCTRL_SHADOWVALID_VALID(n)          (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_SHADOWVALID_VALID_VALID       0x00000001

// #### INTERNAL END ####
//*****************************************************************************
//
// MCUCTRL_ICODEFAULTADDR - ICODE bus address which was present when a bus fault
// occurred.
//
//*****************************************************************************
// The ICODE bus address observed when a Bus Fault occurred. Once an address is
// captured in this field, it is held until the corresponding Fault Observed bit
// is cleared in the FAULTSTATUS register.
#define AM_REG_MCUCTRL_ICODEFAULTADDR_ADDR_S         0
#define AM_REG_MCUCTRL_ICODEFAULTADDR_ADDR_M         0xFFFFFFFF
#define AM_REG_MCUCTRL_ICODEFAULTADDR_ADDR(n)        (((uint32_t)(n) << 0) & 0xFFFFFFFF)

//*****************************************************************************
//
// MCUCTRL_DCODEFAULTADDR - DCODE bus address which was present when a bus fault
// occurred.
//
//*****************************************************************************
// The DCODE bus address observed when a Bus Fault occurred. Once an address is
// captured in this field, it is held until the corresponding Fault Observed bit
// is cleared in the FAULTSTATUS register.
#define AM_REG_MCUCTRL_DCODEFAULTADDR_ADDR_S         0
#define AM_REG_MCUCTRL_DCODEFAULTADDR_ADDR_M         0xFFFFFFFF
#define AM_REG_MCUCTRL_DCODEFAULTADDR_ADDR(n)        (((uint32_t)(n) << 0) & 0xFFFFFFFF)

//*****************************************************************************
//
// MCUCTRL_SYSFAULTADDR - System bus address which was present when a bus fault
// occurred.
//
//*****************************************************************************
// SYS bus address observed when a Bus Fault occurred. Once an address is
// captured in this field, it is held until the corresponding Fault Observed bit
// is cleared in the FAULTSTATUS register.
#define AM_REG_MCUCTRL_SYSFAULTADDR_ADDR_S           0
#define AM_REG_MCUCTRL_SYSFAULTADDR_ADDR_M           0xFFFFFFFF
#define AM_REG_MCUCTRL_SYSFAULTADDR_ADDR(n)          (((uint32_t)(n) << 0) & 0xFFFFFFFF)

//*****************************************************************************
//
// MCUCTRL_FAULTSTATUS - Reflects the status of the bus decoders' fault
// detection. Any write to this register will clear all of the status bits
// within the register.
//
//*****************************************************************************
// SYS Bus Decoder Fault Detected bit. When set, a fault has been detected, and
// the SYSFAULTADDR register will contain the bus address which generated the
// fault.
#define AM_REG_MCUCTRL_FAULTSTATUS_SYS_S             2
#define AM_REG_MCUCTRL_FAULTSTATUS_SYS_M             0x00000004
#define AM_REG_MCUCTRL_FAULTSTATUS_SYS(n)            (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_FAULTSTATUS_SYS_NOFAULT       0x00000000
#define AM_REG_MCUCTRL_FAULTSTATUS_SYS_FAULT         0x00000004

// DCODE Bus Decoder Fault Detected bit. When set, a fault has been detected,
// and the DCODEFAULTADDR register will contain the bus address which generated
// the fault.
#define AM_REG_MCUCTRL_FAULTSTATUS_DCODE_S           1
#define AM_REG_MCUCTRL_FAULTSTATUS_DCODE_M           0x00000002
#define AM_REG_MCUCTRL_FAULTSTATUS_DCODE(n)          (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_FAULTSTATUS_DCODE_NOFAULT     0x00000000
#define AM_REG_MCUCTRL_FAULTSTATUS_DCODE_FAULT       0x00000002

// The ICODE Bus Decoder Fault Detected bit. When set, a fault has been
// detected, and the ICODEFAULTADDR register will contain the bus address which
// generated the fault.
#define AM_REG_MCUCTRL_FAULTSTATUS_ICODE_S           0
#define AM_REG_MCUCTRL_FAULTSTATUS_ICODE_M           0x00000001
#define AM_REG_MCUCTRL_FAULTSTATUS_ICODE(n)          (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_FAULTSTATUS_ICODE_NOFAULT     0x00000000
#define AM_REG_MCUCTRL_FAULTSTATUS_ICODE_FAULT       0x00000001

//*****************************************************************************
//
// MCUCTRL_FAULTCAPTUREEN - Enable the fault capture registers
//
//*****************************************************************************
// Fault Capture Enable field. When set, the Fault Capture monitors are enabled
// and addresses which generate a hard fault are captured into the FAULTADDR
// registers.
#define AM_REG_MCUCTRL_FAULTCAPTUREEN_ENABLE_S       0
#define AM_REG_MCUCTRL_FAULTCAPTUREEN_ENABLE_M       0x00000001
#define AM_REG_MCUCTRL_FAULTCAPTUREEN_ENABLE(n)      (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_FAULTCAPTUREEN_ENABLE_DIS     0x00000000
#define AM_REG_MCUCTRL_FAULTCAPTUREEN_ENABLE_EN      0x00000001

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_PMUENABLE - Control bit to enable/disable the PMU
//
//*****************************************************************************
// PMU Enable Control bit. When set, the MCU's PMU will place the MCU into the
// lowest power consuming Deep Sleep mode upon execution of a WFI instruction
// (dependent on the setting of the SLEEPDEEP bit in the ARM SCR register). When
// cleared, regardless of the requested sleep mode, the PMU will not enter the
// lowest power Deep Sleep mode, instead entering the Sleep mode.
#define AM_REG_MCUCTRL_PMUENABLE_ENABLE_S            0
#define AM_REG_MCUCTRL_PMUENABLE_ENABLE_M            0x00000001
#define AM_REG_MCUCTRL_PMUENABLE_ENABLE(n)           (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_PMUENABLE_ENABLE_DIS          0x00000000
#define AM_REG_MCUCTRL_PMUENABLE_ENABLE_EN           0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
//*****************************************************************************
//
// MCUCTRL_TPIUCTRL - TPIU Control Register. Determines the clock enable and
// frequency for the M4's TPIU interface.
//
//*****************************************************************************
// This field selects the frequency of the ARM M4 TPIU port.
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_S             8
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_M             0x00000300
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL(n)            (((uint32_t)(n) << 8) & 0x00000300)
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_LOW_PWR       0x00000000
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_0MHz          0x00000000
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_6MHZ          0x00000100
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_3MHZ          0x00000200
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_1_5MHZ        0x00000300

// TPIU Enable field. When set, the ARM M4 TPIU is enabled and data can be
// streamed out of the MCU's SWO port using the ARM ITM and TPIU modules.
#define AM_REG_MCUCTRL_TPIUCTRL_ENABLE_S             0
#define AM_REG_MCUCTRL_TPIUCTRL_ENABLE_M             0x00000001
#define AM_REG_MCUCTRL_TPIUCTRL_ENABLE(n)            (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_TPIUCTRL_ENABLE_DIS           0x00000000
#define AM_REG_MCUCTRL_TPIUCTRL_ENABLE_EN            0x00000001

#endif // AM_REG_MCUCTRL_H
