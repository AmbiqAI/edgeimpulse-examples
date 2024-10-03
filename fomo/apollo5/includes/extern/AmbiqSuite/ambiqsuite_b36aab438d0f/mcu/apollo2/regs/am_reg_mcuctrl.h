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
#define AM_REG_MCUCTRL_VENDORID_O                    0x00000010
#define AM_REG_MCUCTRL_DEBUGGER_O                    0x00000014
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_CCRG_O                        0x00000020
#define AM_REG_MCUCTRL_ACRG_O                        0x00000024
#define AM_REG_MCUCTRL_VREFGEN_O                     0x00000040
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_BUCK_O                        0x00000060
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_BUCK2_O                       0x00000064
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_BUCK3_O                       0x00000068
#define AM_REG_MCUCTRL_LDOREG1_O                     0x00000080
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_LDOREG2_O                     0x00000084
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_LDOREG3_O                     0x00000088
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_UNCALVREF_O                   0x000000A0
#define AM_REG_MCUCTRL_UNCALCREF_O                   0x000000A4
#define AM_REG_MCUCTRL_HFRC_O                        0x000000C0
#define AM_REG_MCUCTRL_LFRC_O                        0x000000E0
#define AM_REG_MCUCTRL_IOCTRL_O                      0x000000E4
#define AM_REG_MCUCTRL_TEMPSCTRL_O                   0x000000FC
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_BODPORCTRL_O                  0x00000100
#define AM_REG_MCUCTRL_ADCPWRDLY_O                   0x00000104
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_ADCPWRCTRL_O                  0x00000108
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_ADCCAL_O                      0x0000010C
#define AM_REG_MCUCTRL_ADCBATTLOAD_O                 0x00000110
#define AM_REG_MCUCTRL_BUCKTRIM_O                    0x00000114
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_XTALCTRL_O                    0x00000120
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_XTALGENCTRL_O                 0x00000124
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_FLASHPWRDIS_O                 0x00000148
#define AM_REG_MCUCTRL_STRSTATUS_O                   0x00000150
#define AM_REG_MCUCTRL_STRCMD_O                      0x00000154
#define AM_REG_MCUCTRL_STRDATA_O                     0x00000158
#define AM_REG_MCUCTRL_EXTCLKSEL_O                   0x00000160
#define AM_REG_MCUCTRL_PWRDOWNOVRD_O                 0x00000174
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_O               0x00000178
#define AM_REG_MCUCTRL_PWRONRSTDLY_O                 0x00000180
#define AM_REG_MCUCTRL_PWRSEQ1ANA_O                  0x00000184
#define AM_REG_MCUCTRL_PWRSEQ2ANA_O                  0x00000188
#define AM_REG_MCUCTRL_PWRSEQ3ANA_O                  0x0000018C
#define AM_REG_MCUCTRL_BODISABLE_O                   0x00000190
#define AM_REG_MCUCTRL_D2ASPARE_O                    0x00000194
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_BOOTLOADERLOW_O               0x000001A0
#define AM_REG_MCUCTRL_SHADOWVALID_O                 0x000001A4
#define AM_REG_MCUCTRL_ICODEFAULTADDR_O              0x000001C0
#define AM_REG_MCUCTRL_DCODEFAULTADDR_O              0x000001C4
#define AM_REG_MCUCTRL_SYSFAULTADDR_O                0x000001C8
#define AM_REG_MCUCTRL_FAULTSTATUS_O                 0x000001CC
#define AM_REG_MCUCTRL_FAULTCAPTUREEN_O              0x000001D0
#define AM_REG_MCUCTRL_DBGR1_O                       0x00000200
#define AM_REG_MCUCTRL_DBGR2_O                       0x00000204
#define AM_REG_MCUCTRL_PMUENABLE_O                   0x00000220
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
#define AM_REG_MCUCTRL_MISCDEVST_O                   0x00000240
#endif // MCU_VALIDATION
// #### INTERNAL END ####
#define AM_REG_MCUCTRL_TPIUCTRL_O                    0x00000250
// #### INTERNAL BEGIN ####
#define AM_REG_MCUCTRL_KEXTCLKSEL_O                  0x00000348
// #### INTERNAL END ####

//*****************************************************************************
//
// Key values.
//
//*****************************************************************************
// #### INTERNAL BEGIN ####
#define AM_REG_MCUCTRL_KEXTCLKSEL_KEYVAL             0x00000053
// #### INTERNAL END ####

//*****************************************************************************
//
// MCUCTRL_CHIP_INFO - Chip Information Register
//
//*****************************************************************************
// BCD part number.
#define AM_REG_MCUCTRL_CHIP_INFO_PARTNUM_S           0
#define AM_REG_MCUCTRL_CHIP_INFO_PARTNUM_M           0xFFFFFFFF
#define AM_REG_MCUCTRL_CHIP_INFO_PARTNUM(n)          (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO2     0x03000000
#define AM_REG_MCUCTRL_CHIP_INFO_PARTNUM_APOLLO      0x01000000
#define AM_REG_MCUCTRL_CHIP_INFO_PARTNUM_PN_M        0xFF000000

//*****************************************************************************
//
// MCUCTRL_CHIPID0 - Unique Chip ID 0
//
//*****************************************************************************
// Unique chip ID 0.
#define AM_REG_MCUCTRL_CHIPID0_VALUE_S               0
#define AM_REG_MCUCTRL_CHIPID0_VALUE_M               0xFFFFFFFF
#define AM_REG_MCUCTRL_CHIPID0_VALUE(n)              (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_MCUCTRL_CHIPID0_VALUE_APOLLO2         0x00000000

//*****************************************************************************
//
// MCUCTRL_CHIPID1 - Unique Chip ID 1
//
//*****************************************************************************
// Unique chip ID 1.
#define AM_REG_MCUCTRL_CHIPID1_VALUE_S               0
#define AM_REG_MCUCTRL_CHIPID1_VALUE_M               0xFFFFFFFF
#define AM_REG_MCUCTRL_CHIPID1_VALUE(n)              (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_MCUCTRL_CHIPID1_VALUE_APOLLO2         0x00000000

//*****************************************************************************
//
// MCUCTRL_CHIPREV - Chip Revision
//
//*****************************************************************************
// Major Revision ID.
#define AM_REG_MCUCTRL_CHIPREV_REVMAJ_S              4
#define AM_REG_MCUCTRL_CHIPREV_REVMAJ_M              0x000000F0
#define AM_REG_MCUCTRL_CHIPREV_REVMAJ(n)             (((uint32_t)(n) << 4) & 0x000000F0)
#define AM_REG_MCUCTRL_CHIPREV_REVMAJ_B              0x00000020
#define AM_REG_MCUCTRL_CHIPREV_REVMAJ_A              0x00000010

// Minor Revision ID.
#define AM_REG_MCUCTRL_CHIPREV_REVMIN_S              0
#define AM_REG_MCUCTRL_CHIPREV_REVMIN_M              0x0000000F
#define AM_REG_MCUCTRL_CHIPREV_REVMIN(n)             (((uint32_t)(n) << 0) & 0x0000000F)
#define AM_REG_MCUCTRL_CHIPREV_REVMIN_REV0           0x00000000
#define AM_REG_MCUCTRL_CHIPREV_REVMIN_REV2           0x00000002

//*****************************************************************************
//
// MCUCTRL_VENDORID - Unique Vendor ID
//
//*****************************************************************************
// Unique Vendor ID
#define AM_REG_MCUCTRL_VENDORID_VALUE_S              0
#define AM_REG_MCUCTRL_VENDORID_VALUE_M              0xFFFFFFFF
#define AM_REG_MCUCTRL_VENDORID_VALUE(n)             (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_MCUCTRL_VENDORID_VALUE_AMBIQ          0x414D4251

//*****************************************************************************
//
// MCUCTRL_DEBUGGER - Debugger Access Control
//
//*****************************************************************************
// Lockout of debugger (SWD).
#define AM_REG_MCUCTRL_DEBUGGER_LOCKOUT_S            0
#define AM_REG_MCUCTRL_DEBUGGER_LOCKOUT_M            0x00000001
#define AM_REG_MCUCTRL_DEBUGGER_LOCKOUT(n)           (((uint32_t)(n) << 0) & 0x00000001)

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_CCRG - Calibrated Current Reference Generator Control
//
//*****************************************************************************
// CCRG coarse trim value. Values are 2'b00:1x, 2'b01:2x 2'b10:4x 2'b11:6x;
// These bits are directly sent to the analog module, and no SWE or override
// value is available.
#define AM_REG_MCUCTRL_CCRG_CCRGTRIMCRS_S            9
#define AM_REG_MCUCTRL_CCRG_CCRGTRIMCRS_M            0x00000600
#define AM_REG_MCUCTRL_CCRG_CCRGTRIMCRS(n)           (((uint32_t)(n) << 9) & 0x00000600)

// CCRG fine trim value. These bits are directly sent to the analog module, and
// no SWE or override value is available.
#define AM_REG_MCUCTRL_CCRG_CCRGTRIM_S               1
#define AM_REG_MCUCTRL_CCRG_CCRGTRIM_M               0x000001FE
#define AM_REG_MCUCTRL_CCRG_CCRGTRIM(n)              (((uint32_t)(n) << 1) & 0x000001FE)

// Power down the CCRG. (1=Power down, 0 = Power up)
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
// MCUCTRL_ACRG - Analog Current Reference Generator Control
//
//*****************************************************************************
// ACRG Trim value
#define AM_REG_MCUCTRL_ACRG_ACRGTRIM_S               3
#define AM_REG_MCUCTRL_ACRG_ACRGTRIM_M               0x000000F8
#define AM_REG_MCUCTRL_ACRG_ACRGTRIM(n)              (((uint32_t)(n) << 3) & 0x000000F8)

// IBIAS selectfor ACRG.  0: From BGT, 1: From CCR. NOTE: This is only actively
// applied when PWRSEQ2SWE=1 (due to bug).
#define AM_REG_MCUCTRL_ACRG_ACRGIBIASSEL_S           2
#define AM_REG_MCUCTRL_ACRG_ACRGIBIASSEL_M           0x00000004
#define AM_REG_MCUCTRL_ACRG_ACRGIBIASSEL(n)          (((uint32_t)(n) << 2) & 0x00000004)

// Power down the ACRG. In order for the value to be propagated, the ACRGSWE bit
// must be 1.
#define AM_REG_MCUCTRL_ACRG_ACRGPWD_S                1
#define AM_REG_MCUCTRL_ACRG_ACRGPWD_M                0x00000002
#define AM_REG_MCUCTRL_ACRG_ACRGPWD(n)               (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_ACRG_ACRGPWD_ACRG_PWR_DN      0x00000002

// Software enablement for ACRG register.  A value of 1 will cause the ACRG
// powerdown signal to come from the ACRGPWD bit in this register.  Otherwise,
// the powerdown control is from the power control module. Note: Setting the
// ACRGSWE bit only controls the ACRGPWD bit enablement.
#define AM_REG_MCUCTRL_ACRG_ACRGSWE_S                0
#define AM_REG_MCUCTRL_ACRG_ACRGSWE_M                0x00000001
#define AM_REG_MCUCTRL_ACRG_ACRGSWE(n)               (((uint32_t)(n) << 0) & 0x00000001)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_VREFGEN - Voltage Reference Generator Control
//
//*****************************************************************************
// Calibrated voltage reference trim. The contents of this register are sent to
// the analog module. No SWE or override is available or needed.
#define AM_REG_MCUCTRL_VREFGEN_CVRGVREFTRIM_S        14
#define AM_REG_MCUCTRL_VREFGEN_CVRGVREFTRIM_M        0x001FC000
#define AM_REG_MCUCTRL_VREFGEN_CVRGVREFTRIM(n)       (((uint32_t)(n) << 14) & 0x001FC000)

// Calibrated voltage reference ibias current trim. The contents of this
// register are sent to the analog module. No SWE or override is available or
// needed.
#define AM_REG_MCUCTRL_VREFGEN_CVRGITRIM_S           13
#define AM_REG_MCUCTRL_VREFGEN_CVRGITRIM_M           0x00002000
#define AM_REG_MCUCTRL_VREFGEN_CVRGITRIM(n)          (((uint32_t)(n) << 13) & 0x00002000)

// Power Down, Calibrated Voltage Reference Generator. The contents of this
// register are sent to the analog module. No SWE or override is available or
// needed.
#define AM_REG_MCUCTRL_VREFGEN_PWDCVREF_S            12
#define AM_REG_MCUCTRL_VREFGEN_PWDCVREF_M            0x00001000
#define AM_REG_MCUCTRL_VREFGEN_PWDCVREF(n)           (((uint32_t)(n) << 12) & 0x00001000)
#define AM_REG_MCUCTRL_VREFGEN_PWDCVREF_PWR_DN       0x00001000

// Currently unused. Previously set as Calibrated Voltage Reference Generator
// VREF high adjust.
#define AM_REG_MCUCTRL_VREFGEN_CVREFHADJ_S           7
#define AM_REG_MCUCTRL_VREFGEN_CVREFHADJ_M           0x00000F80
#define AM_REG_MCUCTRL_VREFGEN_CVREFHADJ(n)          (((uint32_t)(n) << 7) & 0x00000F80)

// Currently unused. Previously calibrated Voltage Reference Generator VREF low
// adjust.
#define AM_REG_MCUCTRL_VREFGEN_CVREFLADJ_S           4
#define AM_REG_MCUCTRL_VREFGEN_CVREFLADJ_M           0x00000070
#define AM_REG_MCUCTRL_VREFGEN_CVREFLADJ(n)          (((uint32_t)(n) << 4) & 0x00000070)

// Calibrated Voltage Reference Generator tempco trim (bottom transistor). The
// contents of this register are sent to the analog module. No SWE or override
// is available or needed.
#define AM_REG_MCUCTRL_VREFGEN_CVREFBTRIM_S          0
#define AM_REG_MCUCTRL_VREFGEN_CVREFBTRIM_M          0x0000000F
#define AM_REG_MCUCTRL_VREFGEN_CVREFBTRIM(n)         (((uint32_t)(n) << 0) & 0x0000000F)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
//*****************************************************************************
//
// MCUCTRL_BUCK - Analog Buck Control
//
//*****************************************************************************
// Reset control override for Mem Buck; 0=enabled, 1=reset; Value is propagated
// only when the BUCKSWE bit is active, otherwise contrl is from the power
// control module.
#define AM_REG_MCUCTRL_BUCK_MEMBUCKRST_S             7
#define AM_REG_MCUCTRL_BUCK_MEMBUCKRST_M             0x00000080
#define AM_REG_MCUCTRL_BUCK_MEMBUCKRST(n)            (((uint32_t)(n) << 7) & 0x00000080)

// Reset control override for Core Buck; 0=enabled, 1=reset; Value is propagated
// only when the BUCKSWE bit is active, otherwise control is from the power
// control module.
#define AM_REG_MCUCTRL_BUCK_COREBUCKRST_S            6
#define AM_REG_MCUCTRL_BUCK_COREBUCKRST_M            0x00000040
#define AM_REG_MCUCTRL_BUCK_COREBUCKRST(n)           (((uint32_t)(n) << 6) & 0x00000040)

// Not used.  Additional control of buck is available in the power control
// module
#define AM_REG_MCUCTRL_BUCK_BYPBUCKMEM_S             5
#define AM_REG_MCUCTRL_BUCK_BYPBUCKMEM_M             0x00000020
#define AM_REG_MCUCTRL_BUCK_BYPBUCKMEM(n)            (((uint32_t)(n) << 5) & 0x00000020)

// Memory buck power down override. 1=Powered Down; 0=Enabled; Value is
// propagated only when the BUCKSWE bit is active, otherwise control is from the
// power control module.
#define AM_REG_MCUCTRL_BUCK_MEMBUCKPWD_S             4
#define AM_REG_MCUCTRL_BUCK_MEMBUCKPWD_M             0x00000010
#define AM_REG_MCUCTRL_BUCK_MEMBUCKPWD(n)            (((uint32_t)(n) << 4) & 0x00000010)
#define AM_REG_MCUCTRL_BUCK_MEMBUCKPWD_EN            0x00000000

// HFRC clkgen bit 0 override. When set, this will override to 0 bit 0 of the
// hfrc_freq_clkgen internal bus (see internal Shelby-1473)
#define AM_REG_MCUCTRL_BUCK_SLEEPBUCKANA_S           3
#define AM_REG_MCUCTRL_BUCK_SLEEPBUCKANA_M           0x00000008
#define AM_REG_MCUCTRL_BUCK_SLEEPBUCKANA(n)          (((uint32_t)(n) << 3) & 0x00000008)

// Core buck power down override. 1=Powered Down; 0=Enabled; Value is propagated
// only when the BUCKSWE bit is active, otherwise control is from the power
// control module.
#define AM_REG_MCUCTRL_BUCK_COREBUCKPWD_S            2
#define AM_REG_MCUCTRL_BUCK_COREBUCKPWD_M            0x00000004
#define AM_REG_MCUCTRL_BUCK_COREBUCKPWD(n)           (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_BUCK_COREBUCKPWD_EN           0x00000000

// Not used.  Additional control of buck is available in the power control
// module
#define AM_REG_MCUCTRL_BUCK_BYPBUCKCORE_S            1
#define AM_REG_MCUCTRL_BUCK_BYPBUCKCORE_M            0x00000002
#define AM_REG_MCUCTRL_BUCK_BYPBUCKCORE(n)           (((uint32_t)(n) << 1) & 0x00000002)

// Buck Register Software Override Enable.  This will enable the override values
// for MEMBUCKPWD, COREBUCKPWD, COREBUCKRST, MEMBUCKRST, all to be propagated to
// the control logic, instead of the normal power control module signal.  Note -
// Must take care to have correct value for ALL the register bits when this SWE
// is enabled.
#define AM_REG_MCUCTRL_BUCK_BUCKSWE_S                0
#define AM_REG_MCUCTRL_BUCK_BUCKSWE_M                0x00000001
#define AM_REG_MCUCTRL_BUCK_BUCKSWE(n)               (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_BUCK_BUCKSWE_OVERRIDE_DIS     0x00000000
#define AM_REG_MCUCTRL_BUCK_BUCKSWE_OVERRIDE_EN      0x00000001

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_BUCK2 - Buck Control Reg2
//
//*****************************************************************************
// Buck clkgen divider trim. 00 = 1.5MHz; 01 = 750kHz; 10 = 375kHz; 11 =
// 187.5kHz
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

// Flash Buck high turn on trim
#define AM_REG_MCUCTRL_BUCK2_BMEMTONSEL_S            4
#define AM_REG_MCUCTRL_BUCK2_BMEMTONSEL_M            0x000000F0
#define AM_REG_MCUCTRL_BUCK2_BMEMTONSEL(n)           (((uint32_t)(n) << 4) & 0x000000F0)

// Core Buck low turn on trim
#define AM_REG_MCUCTRL_BUCK2_BCORETONSEL_S           0
#define AM_REG_MCUCTRL_BUCK2_BCORETONSEL_M           0x0000000F
#define AM_REG_MCUCTRL_BUCK2_BCORETONSEL(n)          (((uint32_t)(n) << 0) & 0x0000000F)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
//*****************************************************************************
//
// MCUCTRL_BUCK3 - Buck control reg 3
//
//*****************************************************************************
// MEM Buck low TON trim value
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKLOTON_S          18
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKLOTON_M          0x003C0000
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKLOTON(n)         (((uint32_t)(n) << 18) & 0x003C0000)

// MEM Buck burst enable 0=disable, 0=disabled, 1=enable.
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKBURSTEN_S        17
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKBURSTEN_M        0x00020000
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKBURSTEN(n)       (((uint32_t)(n) << 17) & 0x00020000)

// Memory buck zero crossing trim value
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKZXTRIM_S         13
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKZXTRIM_M         0x0001E000
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKZXTRIM(n)        (((uint32_t)(n) << 13) & 0x0001E000)

// Hysterisis trim for mem buck
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKHYSTTRIM_S       11
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKHYSTTRIM_M       0x00001800
#define AM_REG_MCUCTRL_BUCK3_MEMBUCKHYSTTRIM(n)      (((uint32_t)(n) << 11) & 0x00001800)

// Core Buck low TON trim value
#define AM_REG_MCUCTRL_BUCK3_COREBUCKLOTON_S         7
#define AM_REG_MCUCTRL_BUCK3_COREBUCKLOTON_M         0x00000780
#define AM_REG_MCUCTRL_BUCK3_COREBUCKLOTON(n)        (((uint32_t)(n) << 7) & 0x00000780)

// Core Buck burst enable. 0=disabled, 1=enabled
#define AM_REG_MCUCTRL_BUCK3_COREBUCKBURSTEN_S       6
#define AM_REG_MCUCTRL_BUCK3_COREBUCKBURSTEN_M       0x00000040
#define AM_REG_MCUCTRL_BUCK3_COREBUCKBURSTEN(n)      (((uint32_t)(n) << 6) & 0x00000040)

// Core buck  zero crossing trim value
#define AM_REG_MCUCTRL_BUCK3_COREBUCKZXTRIM_S        2
#define AM_REG_MCUCTRL_BUCK3_COREBUCKZXTRIM_M        0x0000003C
#define AM_REG_MCUCTRL_BUCK3_COREBUCKZXTRIM(n)       (((uint32_t)(n) << 2) & 0x0000003C)

// Hysterisis trim for core buck
#define AM_REG_MCUCTRL_BUCK3_COREBUCKHYSTTRIM_S      0
#define AM_REG_MCUCTRL_BUCK3_COREBUCKHYSTTRIM_M      0x00000003
#define AM_REG_MCUCTRL_BUCK3_COREBUCKHYSTTRIM(n)     (((uint32_t)(n) << 0) & 0x00000003)

//*****************************************************************************
//
// MCUCTRL_LDOREG1 - Analog LDO Reg 1
//
//*****************************************************************************
// CORE LDO IBIAS Trim
#define AM_REG_MCUCTRL_LDOREG1_CORELDOIBSTRM_S       20
#define AM_REG_MCUCTRL_LDOREG1_CORELDOIBSTRM_M       0x00100000
#define AM_REG_MCUCTRL_LDOREG1_CORELDOIBSTRM(n)      (((uint32_t)(n) << 20) & 0x00100000)

// CORE LDO Low Power Trim
#define AM_REG_MCUCTRL_LDOREG1_CORELDOLPTRIM_S       14
#define AM_REG_MCUCTRL_LDOREG1_CORELDOLPTRIM_M       0x000FC000
#define AM_REG_MCUCTRL_LDOREG1_CORELDOLPTRIM(n)      (((uint32_t)(n) << 14) & 0x000FC000)

// CORE LDO tempco trim (R3).
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR3_S       10
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR3_M       0x00003C00
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR3(n)      (((uint32_t)(n) << 10) & 0x00003C00)

// CORE LDO Active mode ouput trim (R1).
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR1_S       0
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR1_M       0x000003FF
#define AM_REG_MCUCTRL_LDOREG1_TRIMCORELDOR1(n)      (((uint32_t)(n) << 0) & 0x000003FF)

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_LDOREG2 - LDO Control Register 2
//
//*****************************************************************************
// Core LDO output enable.  0=Hi-Z, 1=enable. This value is propagated only when
// LDO2SWE bit is active(1).
#define AM_REG_MCUCTRL_LDOREG2_CORELDOVDDLEN_S       22
#define AM_REG_MCUCTRL_LDOREG2_CORELDOVDDLEN_M       0x00400000
#define AM_REG_MCUCTRL_LDOREG2_CORELDOVDDLEN(n)      (((uint32_t)(n) << 22) & 0x00400000)

// RAM LDO LP Mode. 0=normal mode, 1=low power mode;  This value is propagated
// only when LDO2SWE bit is active(1).
#define AM_REG_MCUCTRL_LDOREG2_RAMLDOLPMODE_S        21
#define AM_REG_MCUCTRL_LDOREG2_RAMLDOLPMODE_M        0x00200000
#define AM_REG_MCUCTRL_LDOREG2_RAMLDOLPMODE(n)       (((uint32_t)(n) << 21) & 0x00200000)
#define AM_REG_MCUCTRL_LDOREG2_RAMLDOLPMODE_LPMODE   0x00200000

// RAM LDO Power Down. 0=powered up, 1=powered down ; This value is propagated
// only when LDO2SWE bit is active(1).
#define AM_REG_MCUCTRL_LDOREG2_PWDRAMLDO_S           20
#define AM_REG_MCUCTRL_LDOREG2_PWDRAMLDO_M           0x00100000
#define AM_REG_MCUCTRL_LDOREG2_PWDRAMLDO(n)          (((uint32_t)(n) << 20) & 0x00100000)
#define AM_REG_MCUCTRL_LDOREG2_PWDRAMLDO_PWR_DN      0x00100000

// Analog LDO Power Down. This value is propagated only when LDO2SWE bit is
// active(1).
#define AM_REG_MCUCTRL_LDOREG2_PWDANALDO_S           19
#define AM_REG_MCUCTRL_LDOREG2_PWDANALDO_M           0x00080000
#define AM_REG_MCUCTRL_LDOREG2_PWDANALDO(n)          (((uint32_t)(n) << 19) & 0x00080000)
#define AM_REG_MCUCTRL_LDOREG2_PWDANALDO_PWR_DN      0x00080000

// MEM LDO Power Down. This value is propagated only when LDO2SWE bit is
// active(1).
#define AM_REG_MCUCTRL_LDOREG2_PWDMEMLDO_S           18
#define AM_REG_MCUCTRL_LDOREG2_PWDMEMLDO_M           0x00040000
#define AM_REG_MCUCTRL_LDOREG2_PWDMEMLDO(n)          (((uint32_t)(n) << 18) & 0x00040000)
#define AM_REG_MCUCTRL_LDOREG2_PWDMEMLDO_PWR_DN      0x00040000

// CORE LDO Power Down. This value is propagated only when LDO2SWE bit is
// active(1).
#define AM_REG_MCUCTRL_LDOREG2_PWDCORELDO_S          17
#define AM_REG_MCUCTRL_LDOREG2_PWDCORELDO_M          0x00020000
#define AM_REG_MCUCTRL_LDOREG2_PWDCORELDO(n)         (((uint32_t)(n) << 17) & 0x00020000)
#define AM_REG_MCUCTRL_LDOREG2_PWDCORELDO_PWR_DN     0x00020000

// Analog LDO Sleep. This value is propagated only when LDO2SWE bit is
// active(1).
#define AM_REG_MCUCTRL_LDOREG2_SLEEPANALDO_S         16
#define AM_REG_MCUCTRL_LDOREG2_SLEEPANALDO_M         0x00010000
#define AM_REG_MCUCTRL_LDOREG2_SLEEPANALDO(n)        (((uint32_t)(n) << 16) & 0x00010000)
#define AM_REG_MCUCTRL_LDOREG2_SLEEPANALDO_SLEEP     0x00010000

// FLASH LDO Sleep. This value is propagated only when LDO2SWE bit is active(1).
#define AM_REG_MCUCTRL_LDOREG2_SLEEPMEMLDO_S         15
#define AM_REG_MCUCTRL_LDOREG2_SLEEPMEMLDO_M         0x00008000
#define AM_REG_MCUCTRL_LDOREG2_SLEEPMEMLDO(n)        (((uint32_t)(n) << 15) & 0x00008000)
#define AM_REG_MCUCTRL_LDOREG2_SLEEPMEMLDO_SLEEP     0x00008000

// CORE LDO Sleep. This value is propagated only when LDO2SWE bit is active(1).
#define AM_REG_MCUCTRL_LDOREG2_SLEEPCORELDO_S        14
#define AM_REG_MCUCTRL_LDOREG2_SLEEPCORELDO_M        0x00004000
#define AM_REG_MCUCTRL_LDOREG2_SLEEPCORELDO(n)       (((uint32_t)(n) << 14) & 0x00004000)
#define AM_REG_MCUCTRL_LDOREG2_SLEEPCORELDO_SLEEP    0x00004000

// CONTROL BIT IS NOT USED. PLEASE TREAT AS RESERVED
#define AM_REG_MCUCTRL_LDOREG2_VREFSELANALDO_S       13
#define AM_REG_MCUCTRL_LDOREG2_VREFSELANALDO_M       0x00002000
#define AM_REG_MCUCTRL_LDOREG2_VREFSELANALDO(n)      (((uint32_t)(n) << 13) & 0x00002000)

// CONTROL BIT IS NOT USED. PLEASE TREAT AS RESERVED
#define AM_REG_MCUCTRL_LDOREG2_VREFSELSRAMLDO_S      12
#define AM_REG_MCUCTRL_LDOREG2_VREFSELSRAMLDO_M      0x00001000
#define AM_REG_MCUCTRL_LDOREG2_VREFSELSRAMLDO(n)     (((uint32_t)(n) << 12) & 0x00001000)

// CONTROL BIT IS NOT USED. PLEASE TREAT AS RESERVED
#define AM_REG_MCUCTRL_LDOREG2_VREFSELFLASHLDO_S     11
#define AM_REG_MCUCTRL_LDOREG2_VREFSELFLASHLDO_M     0x00000800
#define AM_REG_MCUCTRL_LDOREG2_VREFSELFLASHLDO(n)    (((uint32_t)(n) << 11) & 0x00000800)

// CONTROL BIT IS NOT USED. PLEASE TREAT AS RESERVED
#define AM_REG_MCUCTRL_LDOREG2_VREFSELCORELDO_S      10
#define AM_REG_MCUCTRL_LDOREG2_VREFSELCORELDO_M      0x00000400
#define AM_REG_MCUCTRL_LDOREG2_VREFSELCORELDO(n)     (((uint32_t)(n) << 10) & 0x00000400)

// Analog LDO Trim.
#define AM_REG_MCUCTRL_LDOREG2_TRIMANALDO_S          6
#define AM_REG_MCUCTRL_LDOREG2_TRIMANALDO_M          0x000003C0
#define AM_REG_MCUCTRL_LDOREG2_TRIMANALDO(n)         (((uint32_t)(n) << 6) & 0x000003C0)

// RAM LDO TRIM
#define AM_REG_MCUCTRL_LDOREG2_RAMLDOTRIM_S          1
#define AM_REG_MCUCTRL_LDOREG2_RAMLDOTRIM_M          0x0000003E
#define AM_REG_MCUCTRL_LDOREG2_RAMLDOTRIM(n)         (((uint32_t)(n) << 1) & 0x0000003E)

// LDO2 Software Override Enable. If enabled (=1), this will enable the override
// values from this register to be used instead of the normal control signals
// for the following fields: CORELDOVDDLEN, RAMLDOLPMODE, PWDRAMLDO, PWDANALDO,
// PWDMEMLDO,  PWDCORELDO, SLEEPANALDO,  SLEEPMEMLDO, SLEEPCORELDO.
#define AM_REG_MCUCTRL_LDOREG2_LDO2SWE_S             0
#define AM_REG_MCUCTRL_LDOREG2_LDO2SWE_M             0x00000001
#define AM_REG_MCUCTRL_LDOREG2_LDO2SWE(n)            (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_LDOREG2_LDO2SWE_OVERRIDE_DIS  0x00000000
#define AM_REG_MCUCTRL_LDOREG2_LDO2SWE_OVERRIDE_EN   0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
//*****************************************************************************
//
// MCUCTRL_LDOREG3 - LDO Control Register 3
//
//*****************************************************************************
// MEM LDO active mode trim (R1).
#define AM_REG_MCUCTRL_LDOREG3_TRIMMEMLDOR1_S        12
#define AM_REG_MCUCTRL_LDOREG3_TRIMMEMLDOR1_M        0x0003F000
#define AM_REG_MCUCTRL_LDOREG3_TRIMMEMLDOR1(n)       (((uint32_t)(n) << 12) & 0x0003F000)

// MEM LDO TRIM for low power mode with ADC active
#define AM_REG_MCUCTRL_LDOREG3_MEMLDOLPALTTRIM_S     6
#define AM_REG_MCUCTRL_LDOREG3_MEMLDOLPALTTRIM_M     0x00000FC0
#define AM_REG_MCUCTRL_LDOREG3_MEMLDOLPALTTRIM(n)    (((uint32_t)(n) << 6) & 0x00000FC0)

// MEM LDO TRIM for low power mode with ADC inactive
#define AM_REG_MCUCTRL_LDOREG3_MEMLDOLPTRIM_S        0
#define AM_REG_MCUCTRL_LDOREG3_MEMLDOLPTRIM_M        0x0000003F
#define AM_REG_MCUCTRL_LDOREG3_MEMLDOLPTRIM(n)       (((uint32_t)(n) << 0) & 0x0000003F)

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_UNCALVREF - Uncalibrated VREF Control
//
//*****************************************************************************
// Trim for uncalibrated VREF signal
#define AM_REG_MCUCTRL_UNCALVREF_TRIMUNCALVREF_S     1
#define AM_REG_MCUCTRL_UNCALVREF_TRIMUNCALVREF_M     0x00000006
#define AM_REG_MCUCTRL_UNCALVREF_TRIMUNCALVREF(n)    (((uint32_t)(n) << 1) & 0x00000006)

// Uncalibrated vref power down.  0=powered up, 1=powered down;
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
// MCUCTRL_UNCALCREF - Uncalibrated Current reference control
//
//*****************************************************************************
// Trim for uncalibrated current source. 2'b00:1x, 2'b01:2x, 2'b10:4x, 2'b11:6x
#define AM_REG_MCUCTRL_UNCALCREF_TRIMUNCALCREF_S     0
#define AM_REG_MCUCTRL_UNCALCREF_TRIMUNCALCREF_M     0x00000003
#define AM_REG_MCUCTRL_UNCALCREF_TRIMUNCALCREF(n)    (((uint32_t)(n) << 0) & 0x00000003)

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

// Power Down HFRC override. 0=powered up, 1=powered down. This field is only
// active when the HFRCWSE = 1.
#define AM_REG_MCUCTRL_HFRC_DISABLEHFRC_S            4
#define AM_REG_MCUCTRL_HFRC_DISABLEHFRC_M            0x00000010
#define AM_REG_MCUCTRL_HFRC_DISABLEHFRC(n)           (((uint32_t)(n) << 4) & 0x00000010)
#define AM_REG_MCUCTRL_HFRC_DISABLEHFRC_PWRUP        0x00000000
#define AM_REG_MCUCTRL_HFRC_DISABLEHFRC_PWRDN        0x00000010

// Force HFRC Output Low. This field is only active when the HFRCWSE = 1.
#define AM_REG_MCUCTRL_HFRC_OUTLOWHFRC_S             3
#define AM_REG_MCUCTRL_HFRC_OUTLOWHFRC_M             0x00000008
#define AM_REG_MCUCTRL_HFRC_OUTLOWHFRC(n)            (((uint32_t)(n) << 3) & 0x00000008)
#define AM_REG_MCUCTRL_HFRC_OUTLOWHFRC_EN            0x00000000
#define AM_REG_MCUCTRL_HFRC_OUTLOWHFRC_OUTLOW        0x00000008

// HFRC Frequency Tune trim bits
#define AM_REG_MCUCTRL_HFRC_TRIMCOEFFHFRC_S          1
#define AM_REG_MCUCTRL_HFRC_TRIMCOEFFHFRC_M          0x00000006
#define AM_REG_MCUCTRL_HFRC_TRIMCOEFFHFRC(n)         (((uint32_t)(n) << 1) & 0x00000006)

// HFRC Software Override Enable.  If this field is enabled, will propagate the
// values for fields DISABLEHFRC and  OUTLOWHFRC to the logic. Otherwise, the
// signals will be sourced from the normal logic.
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
// LFRC ITAIL trim
#define AM_REG_MCUCTRL_LFRC_LFRCITAILTRIM_S          9
#define AM_REG_MCUCTRL_LFRC_LFRCITAILTRIM_M          0x00000200
#define AM_REG_MCUCTRL_LFRC_LFRCITAILTRIM(n)         (((uint32_t)(n) << 9) & 0x00000200)

// LFRC ICAS trim
#define AM_REG_MCUCTRL_LFRC_LFRCICASTRIM_S           8
#define AM_REG_MCUCTRL_LFRC_LFRCICASTRIM_M           0x00000100
#define AM_REG_MCUCTRL_LFRC_LFRCICASTRIM(n)          (((uint32_t)(n) << 8) & 0x00000100)

// LFRC Reset override. 0=no reset, 1=reset; This field is only active when the
// LFRCSWE=1
#define AM_REG_MCUCTRL_LFRC_RESETLFRC_S              7
#define AM_REG_MCUCTRL_LFRC_RESETLFRC_M              0x00000080
#define AM_REG_MCUCTRL_LFRC_RESETLFRC(n)             (((uint32_t)(n) << 7) & 0x00000080)
#define AM_REG_MCUCTRL_LFRC_RESETLFRC_EN             0x00000000
#define AM_REG_MCUCTRL_LFRC_RESETLFRC_RESET          0x00000080

// Power Down LFRC override. This field is only active when the LFRCSWE=1.
#define AM_REG_MCUCTRL_LFRC_PWDLFRC_S                6
#define AM_REG_MCUCTRL_LFRC_PWDLFRC_M                0x00000040
#define AM_REG_MCUCTRL_LFRC_PWDLFRC(n)               (((uint32_t)(n) << 6) & 0x00000040)
#define AM_REG_MCUCTRL_LFRC_PWDLFRC_PWRUP            0x00000000
#define AM_REG_MCUCTRL_LFRC_PWDLFRC_PWRDN            0x00000040

// LFRC Frequency Tune trim bits
#define AM_REG_MCUCTRL_LFRC_TRIMTUNELFRC_S           1
#define AM_REG_MCUCTRL_LFRC_TRIMTUNELFRC_M           0x0000003E
#define AM_REG_MCUCTRL_LFRC_TRIMTUNELFRC(n)          (((uint32_t)(n) << 1) & 0x0000003E)

// LFRC Software Override Enable. This register will enable the override values
// in fields RESETLFRC and PWDLFRC to be used.  Otherside (=0), the normal
// values from the logic will be used.
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
// MCUCTRL_IOCTRL - IO Master and Slave Control
//
//*****************************************************************************
// IO Master 5 Delay selection.
#define AM_REG_MCUCTRL_IOCTRL_IOM5DEL_S              18
#define AM_REG_MCUCTRL_IOCTRL_IOM5DEL_M              0x001C0000
#define AM_REG_MCUCTRL_IOCTRL_IOM5DEL(n)             (((uint32_t)(n) << 18) & 0x001C0000)

// IO Master 4 Delay Value.
#define AM_REG_MCUCTRL_IOCTRL_IOM4DEL_S              15
#define AM_REG_MCUCTRL_IOCTRL_IOM4DEL_M              0x00038000
#define AM_REG_MCUCTRL_IOCTRL_IOM4DEL(n)             (((uint32_t)(n) << 15) & 0x00038000)

// IO Master 3 Delay Value.
#define AM_REG_MCUCTRL_IOCTRL_IOM3DEL_S              12
#define AM_REG_MCUCTRL_IOCTRL_IOM3DEL_M              0x00007000
#define AM_REG_MCUCTRL_IOCTRL_IOM3DEL(n)             (((uint32_t)(n) << 12) & 0x00007000)

// IO Master 2 Delay Value.
#define AM_REG_MCUCTRL_IOCTRL_IOM2DEL_S              9
#define AM_REG_MCUCTRL_IOCTRL_IOM2DEL_M              0x00000E00
#define AM_REG_MCUCTRL_IOCTRL_IOM2DEL(n)             (((uint32_t)(n) << 9) & 0x00000E00)

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
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_TEMPSCTRL - Temp Sensor Control
//
//*****************************************************************************
// Bandgap bypass selection. 0=1.2v ref; 1=ptat ref.
#define AM_REG_MCUCTRL_TEMPSCTRL_BGTR4BYPASS_S       16
#define AM_REG_MCUCTRL_TEMPSCTRL_BGTR4BYPASS_M       0x00010000
#define AM_REG_MCUCTRL_TEMPSCTRL_BGTR4BYPASS(n)      (((uint32_t)(n) << 16) & 0x00010000)

// BGT 580m trim value
#define AM_REG_MCUCTRL_TEMPSCTRL_BGT580MTRIM_S       12
#define AM_REG_MCUCTRL_TEMPSCTRL_BGT580MTRIM_M       0x0000F000
#define AM_REG_MCUCTRL_TEMPSCTRL_BGT580MTRIM(n)      (((uint32_t)(n) << 12) & 0x0000F000)

// BOD high hysteresis trim
#define AM_REG_MCUCTRL_TEMPSCTRL_BODHHYSTTRIM_S      10
#define AM_REG_MCUCTRL_TEMPSCTRL_BODHHYSTTRIM_M      0x00000C00
#define AM_REG_MCUCTRL_TEMPSCTRL_BODHHYSTTRIM(n)     (((uint32_t)(n) << 10) & 0x00000C00)

// BOD low hysteresis trim
#define AM_REG_MCUCTRL_TEMPSCTRL_BODLHYSTTRIM_S      8
#define AM_REG_MCUCTRL_TEMPSCTRL_BODLHYSTTRIM_M      0x00000300
#define AM_REG_MCUCTRL_TEMPSCTRL_BODLHYSTTRIM(n)     (((uint32_t)(n) << 8) & 0x00000300)

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
//*****************************************************************************
//
// MCUCTRL_BODPORCTRL - BOD and PDR control Register
//
//*****************************************************************************
// BOD External Reference Select.
#define AM_REG_MCUCTRL_BODPORCTRL_BODEXTREFSEL_S     3
#define AM_REG_MCUCTRL_BODPORCTRL_BODEXTREFSEL_M     0x00000008
#define AM_REG_MCUCTRL_BODPORCTRL_BODEXTREFSEL(n)    (((uint32_t)(n) << 3) & 0x00000008)
#define AM_REG_MCUCTRL_BODPORCTRL_BODEXTREFSEL_SELECT 0x00000008

// PDR External Reference Select.
#define AM_REG_MCUCTRL_BODPORCTRL_PDREXTREFSEL_S     2
#define AM_REG_MCUCTRL_BODPORCTRL_PDREXTREFSEL_M     0x00000004
#define AM_REG_MCUCTRL_BODPORCTRL_PDREXTREFSEL(n)    (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_BODPORCTRL_PDREXTREFSEL_SELECT 0x00000004

// BOD Power Down.
#define AM_REG_MCUCTRL_BODPORCTRL_PWDBOD_S           1
#define AM_REG_MCUCTRL_BODPORCTRL_PWDBOD_M           0x00000002
#define AM_REG_MCUCTRL_BODPORCTRL_PWDBOD(n)          (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_BODPORCTRL_PWDBOD_PWR_DN      0x00000002

// PDR Power Down.
#define AM_REG_MCUCTRL_BODPORCTRL_PWDPDR_S           0
#define AM_REG_MCUCTRL_BODPORCTRL_PWDPDR_M           0x00000001
#define AM_REG_MCUCTRL_BODPORCTRL_PWDPDR(n)          (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_BODPORCTRL_PWDPDR_PWR_DN      0x00000001

//*****************************************************************************
//
// MCUCTRL_ADCPWRDLY - ADC Power Up Delay Control
//
//*****************************************************************************
// ADC Reference Keeper enable delay in 16 ADC CLK increments for ADC_CLKSEL =
// 0x1, 8 ADC CLOCK increments for ADC_CLKSEL = 0x2.
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR1_S           8
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR1_M           0x0000FF00
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR1(n)          (((uint32_t)(n) << 8) & 0x0000FF00)

// ADC Reference Buffer Power Enable delay in 64 ADC CLK increments for
// ADC_CLKSEL = 0x1, 32 ADC CLOCK increments for ADC_CLKSEL = 0x2.
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR0_S           0
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR0_M           0x000000FF
#define AM_REG_MCUCTRL_ADCPWRDLY_ADCPWR0(n)          (((uint32_t)(n) << 0) & 0x000000FF)

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_ADCPWRCTRL - ADC Power Control
//
//*****************************************************************************
// ADC reference keeper out enable override. This field is only active when the
// ADCPWRCTRLSWE=1.
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCKEEPOUTEN_S     18
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCKEEPOUTEN_M     0x00040000
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCKEEPOUTEN(n)    (((uint32_t)(n) << 18) & 0x00040000)

// ADC reference buffer slew enable
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCRFBUFSLWEN_S    17
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCRFBUFSLWEN_M    0x00020000
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCRFBUFSLWEN(n)   (((uint32_t)(n) << 17) & 0x00020000)

// ADC Input Buffer Power Enable override. This field is only active when the
// ADCPWRCTRLSWE=1.
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCINBUFEN_S       16
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCINBUFEN_M       0x00010000
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCINBUFEN(n)      (((uint32_t)(n) << 16) & 0x00010000)

// ADC input buffer mux select
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCINBUFSEL_S      14
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCINBUFSEL_M      0x0000C000
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCINBUFSEL(n)     (((uint32_t)(n) << 14) & 0x0000C000)

// ADC VBAT DIV Power Enable override. This field is only active when the
// ADCPWRCTRLSWE=1.
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCVBATDIVEN_S     12
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCVBATDIVEN_M     0x00001000
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCVBATDIVEN(n)    (((uint32_t)(n) << 12) & 0x00001000)

// Select between 2.0V or 1.5V internal reference override. This field is only
// active when the ADCPWRCTRLSWE=1.
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFSEL_S           11
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFSEL_M           0x00000800
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFSEL(n)          (((uint32_t)(n) << 11) & 0x00000800)
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFSEL_2p0         0x00000000
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFSEL_1p5         0x00000800

// RESETN signal for Power Switched SAR and Digital Controller (when global
// power switch is off and if the ADCPWRCTRLSWE bit is set)
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCRESETN_S     10
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCRESETN_M     0x00000400
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCRESETN(n)    (((uint32_t)(n) << 10) & 0x00000400)
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCRESETN_ASSERT 0x00000000
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCRESETN_DEASSERT 0x00000400

// ISOLATE signal for ADC Digital Contoller ( when ADCAPSEN is switched off and
// if the ADCPWRCTRLSWE bit is set)
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCDIGISOLATE_S 9
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCDIGISOLATE_M 0x00000200
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCDIGISOLATE(n) (((uint32_t)(n) << 9) & 0x00000200)
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCDIGISOLATE_DIS 0x00000000
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCDIGISOLATE_EN 0x00000200

// ISOLATE signal for Power Switched SAR ( when ADCBPSEN is switched off )
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCSARISOLATE_S 8
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCSARISOLATE_M 0x00000100
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCSARISOLATE(n) (((uint32_t)(n) << 8) & 0x00000100)
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCSARISOLATE_DIS 0x00000000
#define AM_REG_MCUCTRL_ADCPWRCTRL_VDDADCSARISOLATE_EN 0x00000100

// Reference Buffer Keeper Power Switch Enable override. This field is only
// active when the ADCPWRCTRLSWE=1
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFKEEPPEN_S       5
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFKEEPPEN_M       0x00000020
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFKEEPPEN(n)      (((uint32_t)(n) << 5) & 0x00000020)
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFKEEPPEN_DIS     0x00000000
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFKEEPPEN_EN      0x00000020

// Reference Buffer Power Switch Enable override. This field is only active when
// the ADCPWRCTRLSWE=1
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFBUFPEN_S        4
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFBUFPEN_M        0x00000010
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFBUFPEN(n)       (((uint32_t)(n) << 4) & 0x00000010)
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFBUFPEN_DIS      0x00000000
#define AM_REG_MCUCTRL_ADCPWRCTRL_REFBUFPEN_EN       0x00000010

// Bandgap and Temperature Sensor Power Switch Enable override. This field is
// only active when the ADCPWRCTRLSWE=1
#define AM_REG_MCUCTRL_ADCPWRCTRL_BGTPEN_S           3
#define AM_REG_MCUCTRL_ADCPWRCTRL_BGTPEN_M           0x00000008
#define AM_REG_MCUCTRL_ADCPWRCTRL_BGTPEN(n)          (((uint32_t)(n) << 3) & 0x00000008)
#define AM_REG_MCUCTRL_ADCPWRCTRL_BGTPEN_DIS         0x00000000
#define AM_REG_MCUCTRL_ADCPWRCTRL_BGTPEN_EN          0x00000008

// Enable the Analog, IO and SAR Digital logic Power Switch on when set to 1 if
// the ADCPWRCTRLSWE bit is set.
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCBPSEN_S         2
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCBPSEN_M         0x00000004
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCBPSEN(n)        (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCBPSEN_DIS       0x00000000
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCBPSEN_EN        0x00000004

// Enable the Global ADC Power Switch on when set to 1 if the ADCPWRCTRLSWE bit
// is set.
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCAPSEN_S         1
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCAPSEN_M         0x00000002
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCAPSEN(n)        (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCAPSEN_DIS       0x00000000
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCAPSEN_EN        0x00000002

// ADC Power Control Software Override Enable for most of the fields in this
// register.
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCPWRCTRLSWE_S    0
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCPWRCTRLSWE_M    0x00000001
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCPWRCTRLSWE(n)   (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCPWRCTRLSWE_OVERRIDE_DIS 0x00000000
#define AM_REG_MCUCTRL_ADCPWRCTRL_ADCPWRCTRLSWE_OVERRIDE_EN 0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
//*****************************************************************************
//
// MCUCTRL_ADCCAL - ADC Calibration Control
//
//*****************************************************************************
// Status for ADC Calibration
#define AM_REG_MCUCTRL_ADCCAL_ADCCALIBRATED_S        1
#define AM_REG_MCUCTRL_ADCCAL_ADCCALIBRATED_M        0x00000002
#define AM_REG_MCUCTRL_ADCCAL_ADCCALIBRATED(n)       (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_ADCCAL_ADCCALIBRATED_FALSE    0x00000000
#define AM_REG_MCUCTRL_ADCCAL_ADCCALIBRATED_TRUE     0x00000002

// Run ADC Calibration on initial power up sequence
#define AM_REG_MCUCTRL_ADCCAL_CALONPWRUP_S           0
#define AM_REG_MCUCTRL_ADCCAL_CALONPWRUP_M           0x00000001
#define AM_REG_MCUCTRL_ADCCAL_CALONPWRUP(n)          (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_ADCCAL_CALONPWRUP_DIS         0x00000000
#define AM_REG_MCUCTRL_ADCCAL_CALONPWRUP_EN          0x00000001

//*****************************************************************************
//
// MCUCTRL_ADCBATTLOAD - ADC Battery Load Enable
//
//*****************************************************************************
// Enable the ADC battery load resistor
#define AM_REG_MCUCTRL_ADCBATTLOAD_BATTLOAD_S        0
#define AM_REG_MCUCTRL_ADCBATTLOAD_BATTLOAD_M        0x00000001
#define AM_REG_MCUCTRL_ADCBATTLOAD_BATTLOAD(n)       (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_ADCBATTLOAD_BATTLOAD_DIS      0x00000000
#define AM_REG_MCUCTRL_ADCBATTLOAD_BATTLOAD_EN       0x00000001

//*****************************************************************************
//
// MCUCTRL_BUCKTRIM - Trim settings for Core and Mem buck modules
//
//*****************************************************************************
// RESERVED.
#define AM_REG_MCUCTRL_BUCKTRIM_RSVD2_S              24
#define AM_REG_MCUCTRL_BUCKTRIM_RSVD2_M              0x3F000000
#define AM_REG_MCUCTRL_BUCKTRIM_RSVD2(n)             (((uint32_t)(n) << 24) & 0x3F000000)

// Core Buck voltage output trim bits[9:6]. Concatenate with field COREBUCKR1_LO
// for the full trim value.
#define AM_REG_MCUCTRL_BUCKTRIM_COREBUCKR1_HI_S      16
#define AM_REG_MCUCTRL_BUCKTRIM_COREBUCKR1_HI_M      0x000F0000
#define AM_REG_MCUCTRL_BUCKTRIM_COREBUCKR1_HI(n)     (((uint32_t)(n) << 16) & 0x000F0000)

// Core Buck voltage output trim bits[5:0], Concatenate with field COREBUCKR1_HI
// for the full trim value.
#define AM_REG_MCUCTRL_BUCKTRIM_COREBUCKR1_LO_S      8
#define AM_REG_MCUCTRL_BUCKTRIM_COREBUCKR1_LO_M      0x00003F00
#define AM_REG_MCUCTRL_BUCKTRIM_COREBUCKR1_LO(n)     (((uint32_t)(n) << 8) & 0x00003F00)

// Trim values for BUCK regulator.
#define AM_REG_MCUCTRL_BUCKTRIM_MEMBUCKR1_S          0
#define AM_REG_MCUCTRL_BUCKTRIM_MEMBUCKR1_M          0x0000003F
#define AM_REG_MCUCTRL_BUCKTRIM_MEMBUCKR1(n)         (((uint32_t)(n) << 0) & 0x0000003F)

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_XTALCTRL - XTAL Oscillator Control
//
//*****************************************************************************
// XTAL comparator tail trim.
#define AM_REG_MCUCTRL_XTALCTRL_XTALICOMPTRIM_S      8
#define AM_REG_MCUCTRL_XTALCTRL_XTALICOMPTRIM_M      0x00000300
#define AM_REG_MCUCTRL_XTALCTRL_XTALICOMPTRIM(n)     (((uint32_t)(n) << 8) & 0x00000300)

// XTAL feedback buffer tail current trim.
#define AM_REG_MCUCTRL_XTALCTRL_XTALIBUFTRIM_S       6
#define AM_REG_MCUCTRL_XTALCTRL_XTALIBUFTRIM_M       0x000000C0
#define AM_REG_MCUCTRL_XTALCTRL_XTALIBUFTRIM(n)      (((uint32_t)(n) << 6) & 0x000000C0)

// XTAL Power down on brown out.
#define AM_REG_MCUCTRL_XTALCTRL_PWDBODXTAL_S         5
#define AM_REG_MCUCTRL_XTALCTRL_PWDBODXTAL_M         0x00000020
#define AM_REG_MCUCTRL_XTALCTRL_PWDBODXTAL(n)        (((uint32_t)(n) << 5) & 0x00000020)
#define AM_REG_MCUCTRL_XTALCTRL_PWDBODXTAL_PWRUPBOD  0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_PWDBODXTAL_PWRDNBOD  0x00000020

// XTAL Oscillator Power Down Comparator override. Value is only propagated when
// XTALSWE = 1, otherwise, the power down control is sourced from the normal
// clk_gen and init logic.
#define AM_REG_MCUCTRL_XTALCTRL_PWDCMPRXTAL_S        4
#define AM_REG_MCUCTRL_XTALCTRL_PWDCMPRXTAL_M        0x00000010
#define AM_REG_MCUCTRL_XTALCTRL_PWDCMPRXTAL(n)       (((uint32_t)(n) << 4) & 0x00000010)
#define AM_REG_MCUCTRL_XTALCTRL_PWDCMPRXTAL_PWRUPCOMP 0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_PWDCMPRXTAL_PWRDNCOMP 0x00000010

// XTAL Oscillator Power Down Core override. Value is only propagated when
// XTALSWE = 1, otherwise, the power down control is sourced from the normal
// clk_gen and init logic.
#define AM_REG_MCUCTRL_XTALCTRL_PWDCOREXTAL_S        3
#define AM_REG_MCUCTRL_XTALCTRL_PWDCOREXTAL_M        0x00000008
#define AM_REG_MCUCTRL_XTALCTRL_PWDCOREXTAL(n)       (((uint32_t)(n) << 3) & 0x00000008)
#define AM_REG_MCUCTRL_XTALCTRL_PWDCOREXTAL_PWRUPCORE 0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_PWDCOREXTAL_PWRDNCORE 0x00000008

// XTAL Oscillator Bypass Comparator.
#define AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_S        2
#define AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_M        0x00000004
#define AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL(n)       (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_USECOMP  0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_BYPCMPRXTAL_BYPCOMP  0x00000004

// XTAL Oscillator Disable Feedback.
#define AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_S       1
#define AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_M       0x00000002
#define AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL(n)      (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_EN      0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_FDBKDSBLXTAL_DIS     0x00000002

// XTAL Software Override Enable. Enabling this will allow the fields of
// PWDCMPRXTAL and PWDCOREXTAL to be actively sourced to the analog module,
// instead of the normal logic.
#define AM_REG_MCUCTRL_XTALCTRL_XTALSWE_S            0
#define AM_REG_MCUCTRL_XTALCTRL_XTALSWE_M            0x00000001
#define AM_REG_MCUCTRL_XTALCTRL_XTALSWE(n)           (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_DIS 0x00000000
#define AM_REG_MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_EN  0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
//*****************************************************************************
//
// MCUCTRL_XTALGENCTRL - XTAL Oscillator General Control
//
//*****************************************************************************
// XTAL IBIAS Kick start trim . This trim value is used during the startup
// process to enable a faster lock and is applied when the kickstart signal is
// active.
#define AM_REG_MCUCTRL_XTALGENCTRL_XTALKSBIASTRIM_S  8
#define AM_REG_MCUCTRL_XTALGENCTRL_XTALKSBIASTRIM_M  0x00003F00
#define AM_REG_MCUCTRL_XTALGENCTRL_XTALKSBIASTRIM(n) (((uint32_t)(n) << 8) & 0x00003F00)

// XTAL IBIAS trim
#define AM_REG_MCUCTRL_XTALGENCTRL_XTALBIASTRIM_S    2
#define AM_REG_MCUCTRL_XTALGENCTRL_XTALBIASTRIM_M    0x000000FC
#define AM_REG_MCUCTRL_XTALGENCTRL_XTALBIASTRIM(n)   (((uint32_t)(n) << 2) & 0x000000FC)

// Auto-calibration delay control
#define AM_REG_MCUCTRL_XTALGENCTRL_ACWARMUP_S        0
#define AM_REG_MCUCTRL_XTALGENCTRL_ACWARMUP_M        0x00000003
#define AM_REG_MCUCTRL_XTALGENCTRL_ACWARMUP(n)       (((uint32_t)(n) << 0) & 0x00000003)
#define AM_REG_MCUCTRL_XTALGENCTRL_ACWARMUP_1SEC     0x00000000
#define AM_REG_MCUCTRL_XTALGENCTRL_ACWARMUP_2SEC     0x00000001
#define AM_REG_MCUCTRL_XTALGENCTRL_ACWARMUP_4SEC     0x00000002
#define AM_REG_MCUCTRL_XTALGENCTRL_ACWARMUP_8SEC     0x00000003

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
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

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_STRSTATUS - Status bits for the D2A STR interface
//
//*****************************************************************************
// The number of remainder bits contained in the currently selected STR
#define AM_REG_MCUCTRL_STRSTATUS_STR_SEL_REMAIN_LEN_S 27
#define AM_REG_MCUCTRL_STRSTATUS_STR_SEL_REMAIN_LEN_M 0xF8000000
#define AM_REG_MCUCTRL_STRSTATUS_STR_SEL_REMAIN_LEN(n) (((uint32_t)(n) << 27) & 0xF8000000)

// The number of words contained in the currently selected STR
#define AM_REG_MCUCTRL_STRSTATUS_STR_SEL_WORD_LEN_S  16
#define AM_REG_MCUCTRL_STRSTATUS_STR_SEL_WORD_LEN_M  0x07FF0000
#define AM_REG_MCUCTRL_STRSTATUS_STR_SEL_WORD_LEN(n) (((uint32_t)(n) << 16) & 0x07FF0000)

// The current status of the STR mode
#define AM_REG_MCUCTRL_STRSTATUS_STR_ENABLE_STATUS_S 14
#define AM_REG_MCUCTRL_STRSTATUS_STR_ENABLE_STATUS_M 0x0000C000
#define AM_REG_MCUCTRL_STRSTATUS_STR_ENABLE_STATUS(n) (((uint32_t)(n) << 14) & 0x0000C000)
#define AM_REG_MCUCTRL_STRSTATUS_STR_ENABLE_STATUS_STR_DISABLED 0x00000000
#define AM_REG_MCUCTRL_STRSTATUS_STR_ENABLE_STATUS_STR_FULL_ENABLE 0x00004000
#define AM_REG_MCUCTRL_STRSTATUS_STR_ENABLE_STATUS_STR_MASKED_ENABLE 0x00008000

// Value of words to be processed in the current operation.  This is an async
// reg between domains, so adequete time should be allowed for it to be stable
#define AM_REG_MCUCTRL_STRSTATUS_STR_CNT_STATUS_S    4
#define AM_REG_MCUCTRL_STRSTATUS_STR_CNT_STATUS_M    0x00003FF0
#define AM_REG_MCUCTRL_STRSTATUS_STR_CNT_STATUS(n)   (((uint32_t)(n) << 4) & 0x00003FF0)

// Status related to the current command.  Bit 0: Read Data Available   Bit 1:
// Write Busy   Bit 2: Command In Progress Bit 3: Waiting for write data
#define AM_REG_MCUCTRL_STRSTATUS_STR_CMD_STATUS_S    0
#define AM_REG_MCUCTRL_STRSTATUS_STR_CMD_STATUS_M    0x0000000F
#define AM_REG_MCUCTRL_STRSTATUS_STR_CMD_STATUS(n)   (((uint32_t)(n) << 0) & 0x0000000F)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_STRCMD - Command interface for the STR design, which controls the D2A
// signaling
//
//*****************************************************************************
// 1 will enable the STR clock divider function, 0 will disable this.
#define AM_REG_MCUCTRL_STRCMD_STR_CLK_EN_S           24
#define AM_REG_MCUCTRL_STRCMD_STR_CLK_EN_M           0x01000000
#define AM_REG_MCUCTRL_STRCMD_STR_CLK_EN(n)          (((uint32_t)(n) << 24) & 0x01000000)

// Divide value for each half of the hclk clk period to run the STR at.
#define AM_REG_MCUCTRL_STRCMD_STR_CLK_DIV_CNT_S      16
#define AM_REG_MCUCTRL_STRCMD_STR_CLK_DIV_CNT_M      0x00FF0000
#define AM_REG_MCUCTRL_STRCMD_STR_CLK_DIV_CNT(n)     (((uint32_t)(n) << 16) & 0x00FF0000)

// Enables the STR to override the D2A signals using the values in the update
// register.
#define AM_REG_MCUCTRL_STRCMD_STR_ENABLE_S           14
#define AM_REG_MCUCTRL_STRCMD_STR_ENABLE_M           0x0000C000
#define AM_REG_MCUCTRL_STRCMD_STR_ENABLE(n)          (((uint32_t)(n) << 14) & 0x0000C000)
#define AM_REG_MCUCTRL_STRCMD_STR_ENABLE_STR_DISABLED 0x00000000
#define AM_REG_MCUCTRL_STRCMD_STR_ENABLE_STR_FULL_ENABLE 0x00004000
#define AM_REG_MCUCTRL_STRCMD_STR_ENABLE_STR_MASKED_ENABLE 0x00008000

// Selection of the STR chain for commands
#define AM_REG_MCUCTRL_STRCMD_STR_SELECT_S           4
#define AM_REG_MCUCTRL_STRCMD_STR_SELECT_M           0x00000030
#define AM_REG_MCUCTRL_STRCMD_STR_SELECT(n)          (((uint32_t)(n) << 4) & 0x00000030)

// The command code to the STR system
#define AM_REG_MCUCTRL_STRCMD_STR_CMD_S              0
#define AM_REG_MCUCTRL_STRCMD_STR_CMD_M              0x0000000F
#define AM_REG_MCUCTRL_STRCMD_STR_CMD(n)             (((uint32_t)(n) << 0) & 0x0000000F)
#define AM_REG_MCUCTRL_STRCMD_STR_CMD_IDLE           0x00000000
#define AM_REG_MCUCTRL_STRCMD_STR_CMD_READCAP        0x00000001
#define AM_REG_MCUCTRL_STRCMD_STR_CMD_WRITECAP       0x00000002
#define AM_REG_MCUCTRL_STRCMD_STR_CMD_READMASK       0x00000003
#define AM_REG_MCUCTRL_STRCMD_STR_CMD_WRITEMASK      0x00000004
#define AM_REG_MCUCTRL_STRCMD_STR_CMD_CAPTURE        0x00000005
#define AM_REG_MCUCTRL_STRCMD_STR_CMD_UPDATE         0x00000006

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_STRDATA - DATA interface for the STR design controlling the D2A
// signaling
//
//*****************************************************************************
// Data read or write to the STR subsystem
#define AM_REG_MCUCTRL_STRDATA_STR_DATA_S            0
#define AM_REG_MCUCTRL_STRDATA_STR_DATA_M            0xFFFFFFFF
#define AM_REG_MCUCTRL_STRDATA_STR_DATA(n)           (((uint32_t)(n) << 0) & 0xFFFFFFFF)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_EXTCLKSEL - Source selection of LFRC, HFRC and XTAL clock sources
//
//*****************************************************************************
// HFRC External Clock Source Select.
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_HF_S            2
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_HF_M            0x00000004
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_HF(n)           (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_HF_INT          0x00000000
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_HF_EXT          0x00000004

// LFRC External Clock Source Select.
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_LF_S            1
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_LF_M            0x00000002
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_LF(n)           (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_LF_INT          0x00000000
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_LF_EXT          0x00000002

// XTAL External Clock Source Select.
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_XT_S            0
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_XT_M            0x00000001
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_XT(n)           (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_XT_INT          0x00000000
#define AM_REG_MCUCTRL_EXTCLKSEL_EXT_XT_EXT          0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_PWRDOWNOVRD - Power down override register
//
//*****************************************************************************
// Flash BIST Isolation enable signal. No additional SWE control is needed to
// set or activate the isolation, and this is intended for manufacturing tests
// only.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLBIST_ISO_OR_S 27
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLBIST_ISO_OR_M 0x08000000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLBIST_ISO_OR(n) (((uint32_t)(n) << 27) & 0x08000000)

// Flash BIST power gate signal.  Active low signal (Set to 1 to enable power, 0
// to disable power). No additional SWE control is needed to set/activate the
// power gate. This is intended for use only in manufacturing tests.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLBIST_PG_OR_S 26
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLBIST_PG_OR_M 0x04000000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLBIST_PG_OR(n) (((uint32_t)(n) << 26) & 0x04000000)

// MBIST Isolation enable signal.  No additional SWE control is needed to set or
// activate the isolation, and this is intended for manufacturing tests only.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_MBIST_ISO_OR_S 25
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_MBIST_ISO_OR_M 0x02000000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_MBIST_ISO_OR(n) (((uint32_t)(n) << 25) & 0x02000000)

// MBIST power gate enable signal. Active low signal (Set to 1 to enable power,
// 0 to disable power). No additional SWE control is needed to set/activate the
// power gate. This is intended for use only in manufacturing tests.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_MBIST_PG_OR_S  24
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_MBIST_PG_OR_M  0x01000000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_MBIST_PG_OR(n) (((uint32_t)(n) << 24) & 0x01000000)

// Override for the PDM Isolation enable signal.  0=Not isolated, 1=isolated.
// Only active when PWRDOWN_SWE = 1, otherwise, control for this logic is
// sourced from the normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_PDM_ISO_OR_S   23
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_PDM_ISO_OR_M   0x00800000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_PDM_ISO_OR(n)  (((uint32_t)(n) << 23) & 0x00800000)

// Override for the PDM Power gate signal. 0=Powered on, 1=Powered Off. Only
// active when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced
// from the normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_PDM_PG_OR_S    22
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_PDM_PG_OR_M    0x00400000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_PDM_PG_OR(n)   (((uint32_t)(n) << 22) & 0x00400000)

// Override for the SYS retain signal. 0=No retain, 1=Retain; Only active when
// PWRDOWN_SWE = 1, otherwise, control for this logic is sourced from the normal
// logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_SYSRETAINN_OR_S   21
#define AM_REG_MCUCTRL_PWRDOWNOVRD_SYSRETAINN_OR_M   0x00200000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_SYSRETAINN_OR(n)  (((uint32_t)(n) << 21) & 0x00200000)

// Override for the SYS isolate signal. 0=No Isolation, 1=Isolation; Only active
// when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced from the
// normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_SYSISOLATEN_OR_S  20
#define AM_REG_MCUCTRL_PWRDOWNOVRD_SYSISOLATEN_OR_M  0x00100000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_SYSISOLATEN_OR(n) (((uint32_t)(n) << 20) & 0x00100000)

// Power gate override for MCUL domain. 0=Powered on, 1=Powered Off. Only active
// when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced from the
// normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_MCUL_PG_OR_S   19
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_MCUL_PG_OR_M   0x00080000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_MCUL_PG_OR(n)  (((uint32_t)(n) << 19) & 0x00080000)

// Reset override for HCPC domain. Active low reset. 0=Reset, 1=No Reset; Only
// active when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced
// from the normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_HCPC_RESET_N_OR_S 18
#define AM_REG_MCUCTRL_PWRDOWNOVRD_HCPC_RESET_N_OR_M 0x00040000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_HCPC_RESET_N_OR(n) (((uint32_t)(n) << 18) & 0x00040000)

// Isolation override for power domain HCPC. 0=Not isolated, 1=isolated. Only
// active when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced
// from the normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPC_ISO_OR_S  17
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPC_ISO_OR_M  0x00020000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPC_ISO_OR(n) (((uint32_t)(n) << 17) & 0x00020000)

// Power gate override for power domain HCPC. 0=Powered on, 1=Powered Off. Only
// active when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced
// from the normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPC_PG_OR_S   16
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPC_PG_OR_M   0x00010000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPC_PG_OR(n)  (((uint32_t)(n) << 16) & 0x00010000)

// Reset override for HCBA domain. Active low reset. 0=Reset, 1=No Reset; Only
// active when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced
// from the normal logic
#define AM_REG_MCUCTRL_PWRDOWNOVRD_HCPB_RESET_N_OR_S 15
#define AM_REG_MCUCTRL_PWRDOWNOVRD_HCPB_RESET_N_OR_M 0x00008000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_HCPB_RESET_N_OR(n) (((uint32_t)(n) << 15) & 0x00008000)

// Isolation override for power domain HCPB. 0=Not isolated, 1=isolated. Only
// active when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced
// from the normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPB_ISO_OR_S  14
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPB_ISO_OR_M  0x00004000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPB_ISO_OR(n) (((uint32_t)(n) << 14) & 0x00004000)

// Power gate override for power domain HCPB. 0=Powered on, 1=Powered Off. Only
// active when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced
// from the normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPB_PG_OR_S   13
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPB_PG_OR_M   0x00002000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPB_PG_OR(n)  (((uint32_t)(n) << 13) & 0x00002000)

// Reset override for HCPA domain. Active low reset. 0=Reset, 1=No Reset; Only
// active when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced
// from the normal logic
#define AM_REG_MCUCTRL_PWRDOWNOVRD_HCPA_RESET_N_OR_S 12
#define AM_REG_MCUCTRL_PWRDOWNOVRD_HCPA_RESET_N_OR_M 0x00001000
#define AM_REG_MCUCTRL_PWRDOWNOVRD_HCPA_RESET_N_OR(n) (((uint32_t)(n) << 12) & 0x00001000)

// Isolation override for power domain HCPA. 0=Not isolated, 1=isolated. Only
// active when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced
// from the normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPA_ISO_OR_S  11
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPA_ISO_OR_M  0x00000800
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPA_ISO_OR(n) (((uint32_t)(n) << 11) & 0x00000800)

// Power gate override for power domain HCPA. 0=Powered on, 1=Powered Off. Only
// active when PWRDOWN_SWE = 1, otherwise, control for this logic is sourced
// from the normal logic.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPA_PG_OR_S   10
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPA_PG_OR_M   0x00000400
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_HCPA_PG_OR(n)  (((uint32_t)(n) << 10) & 0x00000400)

// Override control for the PWRRRDY IO control to both flash macros. Active low
// signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM_PWRRDY_IO_OR_S 8
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM_PWRRDY_IO_OR_M 0x00000100
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM_PWRRDY_IO_OR(n) (((uint32_t)(n) << 8) & 0x00000100)

// Override control for the VDIO (Flash power) for both flash macros.  Active
// low signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM_VDIO_PG_OR_S 7
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM_VDIO_PG_OR_M 0x00000080
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM_VDIO_PG_OR(n) (((uint32_t)(n) << 7) & 0x00000080)

// Power
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PDM_IO_FLAM1_OR_S 6
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PDM_IO_FLAM1_OR_M 0x00000040
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PDM_IO_FLAM1_OR(n) (((uint32_t)(n) << 6) & 0x00000040)
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PDM_IO_FLAM1_OR_PWD 0x00000000

// Power
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM1_PG_OR_S  5
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM1_PG_OR_M  0x00000020
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM1_PG_OR(n) (((uint32_t)(n) << 5) & 0x00000020)
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM1_PG_OR_PWD 0x00000000

// Override control for flash instance 1 isoltate signal, norma .  This value is
// only used if the PWRDOWN_SWE bit is active.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM1_ISO_OR_S 4
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM1_ISO_OR_M 0x00000010
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM1_ISO_OR(n) (((uint32_t)(n) << 4) & 0x00000010)
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM1_ISO_OR_PWD 0x00000010

// Power
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PDM_IO_FLAM0_OR_S 3
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PDM_IO_FLAM0_OR_M 0x00000008
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PDM_IO_FLAM0_OR(n) (((uint32_t)(n) << 3) & 0x00000008)
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PDM_IO_FLAM0_OR_PWD 0x00000000

// Power
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM0_PG_OR_S  2
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM0_PG_OR_M  0x00000004
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM0_PG_OR(n) (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM0_PG_OR_PWD 0x00000000

// Override control for flash instance 0 isoltate signal, norma .  This value is
// only used if the PWRDOWN_SWE bit is active.
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM0_ISO_OR_S 1
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM0_ISO_OR_M 0x00000002
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM0_ISO_OR(n) (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PD_FLAM0_ISO_OR_PWD 0x00000002

// PWR Down control override enable
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PWRDOWN_SWE_S     0
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PWRDOWN_SWE_M     0x00000001
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PWRDOWN_SWE(n)    (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_PWRDOWNOVRD_PWRDOWN_SWE_EN    0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_PWRDOWNSROVRD - Power down SRAM override register
//
//*****************************************************************************
// Isolation control for CACHE GROUP B2.  Only valid when PWRDOWNSR_SWE=1.
// Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB2_ISO_OR_S 26
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB2_ISO_OR_M 0x04000000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB2_ISO_OR(n) (((uint32_t)(n) << 26) & 0x04000000)

// Isolation control for CACHE GROUP B0 , SRAM 0.  Only valid when
// PWRDOWNSR_SWE=1.   Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB0_ISO_OR_S 25
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB0_ISO_OR_M 0x02000000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB0_ISO_OR(n) (((uint32_t)(n) << 25) & 0x02000000)

// Isolation control for SRAM GROUP 7 srams.  Only valid when PWRDOWNSR_SWE=1.
// Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP7_ISO_OR_S 24
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP7_ISO_OR_M 0x01000000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP7_ISO_OR(n) (((uint32_t)(n) << 24) & 0x01000000)

// Isolation control for SRAM GROUP 6 srams.  Only valid when PWRDOWNSR_SWE=1.
// Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP6_ISO_OR_S 23
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP6_ISO_OR_M 0x00800000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP6_ISO_OR(n) (((uint32_t)(n) << 23) & 0x00800000)

// Isolation control for SRAM GROUP 5 srams.  Only valid when PWRDOWNSR_SWE=1.
// Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP5_ISO_OR_S 22
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP5_ISO_OR_M 0x00400000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP5_ISO_OR(n) (((uint32_t)(n) << 22) & 0x00400000)

// Isolation control for SRAM GROUP 4 srams.  Only valid when PWRDOWNSR_SWE=1.
// Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP4_ISO_OR_S 21
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP4_ISO_OR_M 0x00200000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP4_ISO_OR(n) (((uint32_t)(n) << 21) & 0x00200000)

// Isolation control for SRAM GROUP 3 srams.  Only valid when PWRDOWNSR_SWE=1.
// Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP3_ISO_OR_S 20
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP3_ISO_OR_M 0x00100000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP3_ISO_OR(n) (((uint32_t)(n) << 20) & 0x00100000)

// Isolation control for SRAM GROUP 2 srams.  Only valid when PWRDOWNSR_SWE=1.
// Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP2_ISO_OR_S 19
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP2_ISO_OR_M 0x00080000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP2_ISO_OR(n) (((uint32_t)(n) << 19) & 0x00080000)

// Isolation control for SRAM GROUP 1 srams.  Only valid when PWRDOWNSR_SWE=1.
// Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP1_ISO_OR_S 18
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP1_ISO_OR_M 0x00040000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP1_ISO_OR(n) (((uint32_t)(n) << 18) & 0x00040000)

// Isolation control for SRAM GROUP 0 , SRAM 3.  Only valid when
// PWRDOWNSR_SWE=1.   Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_3_ISO_OR_S 17
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_3_ISO_OR_M 0x00020000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_3_ISO_OR(n) (((uint32_t)(n) << 17) & 0x00020000)

// Isolation control for SRAM GROUP 0 , SRAM 2.  Only valid when
// PWRDOWNSR_SWE=1.   Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_2_ISO_OR_S 16
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_2_ISO_OR_M 0x00010000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_2_ISO_OR(n) (((uint32_t)(n) << 16) & 0x00010000)

// Isolation control for SRAM GROUP 0 , SRAM 1.  Only valid when
// PWRDOWNSR_SWE=1.   Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_1_ISO_OR_S 15
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_1_ISO_OR_M 0x00008000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_1_ISO_OR(n) (((uint32_t)(n) << 15) & 0x00008000)

// Isolation control for SRAM GROUP 0 , SRAM 0.  Only valid when
// PWRDOWNSR_SWE=1.   Value of 1 will enable isolation (ISO)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_0_ISO_OR_S 14
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_0_ISO_OR_M 0x00004000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_0_ISO_OR(n) (((uint32_t)(n) << 14) & 0x00004000)

// Power gate control for CACHE GROUPB2.  Only valid when PWRDOWNSR_SWE=1.
// Active low signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB2_PG_OR_S 13
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB2_PG_OR_M 0x00002000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB2_PG_OR(n) (((uint32_t)(n) << 13) & 0x00002000)

// Power gate control for CACHE GROUPB0 srams.  Only valid when PWRDOWNSR_SWE=1.
// Active low signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB0_PG_OR_S 12
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB0_PG_OR_M 0x00001000
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_CACHEB0_PG_OR(n) (((uint32_t)(n) << 12) & 0x00001000)

// Power gate control for SRAM GROUP 7 srams.  Only valid when PWRDOWNSR_SWE=1.
// Active low signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP7_PG_OR_S 11
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP7_PG_OR_M 0x00000800
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP7_PG_OR(n) (((uint32_t)(n) << 11) & 0x00000800)

// Power gate control for SRAM GROUP 6 srams.  Only valid when PWRDOWNSR_SWE=1.
// Active low signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP6_PG_OR_S 10
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP6_PG_OR_M 0x00000400
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP6_PG_OR(n) (((uint32_t)(n) << 10) & 0x00000400)

// Power gate control for SRAM GROUP 5 srams.  Only valid when PWRDOWNSR_SWE=1.
// Active low signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP5_PG_OR_S 9
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP5_PG_OR_M 0x00000200
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP5_PG_OR(n) (((uint32_t)(n) << 9) & 0x00000200)

// Power gate control for SRAM GROUP 4 srams.  Only valid when PWRDOWNSR_SWE=1.
// Active low signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP4_PG_OR_S 8
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP4_PG_OR_M 0x00000100
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP4_PG_OR(n) (((uint32_t)(n) << 8) & 0x00000100)

// Power gate control for SRAM GROUP 3 srams.  Only valid when PWRDOWNSR_SWE=1.
// Active low signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP3_PG_OR_S 7
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP3_PG_OR_M 0x00000080
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP3_PG_OR(n) (((uint32_t)(n) << 7) & 0x00000080)

// Power gate control for SRAM GROUP 2 srams.  Only valid when PWRDOWNSR_SWE=1.
// Active low signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP2_PG_OR_S 6
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP2_PG_OR_M 0x00000040
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP2_PG_OR(n) (((uint32_t)(n) << 6) & 0x00000040)

// Power gate control for SRAM GROUP 1 srams.  Only valid when PWRDOWNSR_SWE=1.
// Active low signal (Set to 1 to enable power, 0 to disable power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP1_PG_OR_S 5
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP1_PG_OR_M 0x00000020
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP1_PG_OR(n) (((uint32_t)(n) << 5) & 0x00000020)

// Power gate control for SRAM GROUP 0 , SRAM 3.  Only valid when
// PWRDOWNSR_SWE=1.   iActive low signal (Set to 1 to enable power, 0 to disable
// power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_3_PG_OR_S 4
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_3_PG_OR_M 0x00000010
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_3_PG_OR(n) (((uint32_t)(n) << 4) & 0x00000010)

// Power gate control for SRAM GROUP 0 , SRAM 2.  Only valid when
// PWRDOWNSR_SWE=1.   Active low signal (Set to 1 to enable power, 0 to disable
// power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_2_PG_OR_S 3
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_2_PG_OR_M 0x00000008
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_2_PG_OR(n) (((uint32_t)(n) << 3) & 0x00000008)

// Power gate control for SRAM GROUP 0 , SRAM 1.  Only valid when
// PWRDOWNSR_SWE=1.   Active low signal (Set to 1 to enable power, 0 to disable
// power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_1_PG_OR_S 2
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_1_PG_OR_M 0x00000004
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_1_PG_OR(n) (((uint32_t)(n) << 2) & 0x00000004)

// Power gate control for SRAM GROUP 0 , SRAM 0.  Only valid when
// PWRDOWNSR_SWE=1.  Active low signal (Set to 1 to enable power, 0 to disable
// power)
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_0_PG_OR_S 1
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_0_PG_OR_M 0x00000002
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PD_SRAM_GP0_0_PG_OR(n) (((uint32_t)(n) << 1) & 0x00000002)

// Power down override enable for the SRAM registers
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PWRDOWNSR_SWE_S 0
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PWRDOWNSR_SWE_M 0x00000001
#define AM_REG_MCUCTRL_PWRDOWNSROVRD_PWRDOWNSR_SWE(n) (((uint32_t)(n) << 0) & 0x00000001)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_PWRONRSTDLY - Power On Sequence Delay Register
//
//*****************************************************************************
// Brown-out and POI Delay Sequence Scale Value (divide the 3 POR delays by this
// value)
#define AM_REG_MCUCTRL_PWRONRSTDLY_POI_BO_SCALE_S    24
#define AM_REG_MCUCTRL_PWRONRSTDLY_POI_BO_SCALE_M    0x07000000
#define AM_REG_MCUCTRL_PWRONRSTDLY_POI_BO_SCALE(n)   (((uint32_t)(n) << 24) & 0x07000000)

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
// mode = 1). Only active when PWRSEQ1SWE=1.
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
// reference. Only active when PWRSEQ2SWE=1.
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
// reset power stable comparator power down override.  Only active when
// PWRSEQ3SWE=1.
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
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_BODISABLE - Brownout Disable
//
//*****************************************************************************
// Disable 1.8V Brown-out reset.
#define AM_REG_MCUCTRL_BODISABLE_BODLRDE_S           0
#define AM_REG_MCUCTRL_BODISABLE_BODLRDE_M           0x00000001
#define AM_REG_MCUCTRL_BODISABLE_BODLRDE(n)          (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_BODISABLE_BODLRDE_EN          0x00000000
#define AM_REG_MCUCTRL_BODISABLE_BODLRDE_DIS         0x00000001

#endif // MCU_VALIDATION
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_D2ASPARE - Spare registers to analog module
//
//*****************************************************************************
// D2A Spare signals (non-shadow loaded)(Usage: Not used currently)
#define AM_REG_MCUCTRL_D2ASPARE_D2A_SPARE_NS_S       8
#define AM_REG_MCUCTRL_D2ASPARE_D2A_SPARE_NS_M       0x0000FF00
#define AM_REG_MCUCTRL_D2ASPARE_D2A_SPARE_NS(n)      (((uint32_t)(n) << 8) & 0x0000FF00)

// D2A Spare signals (shadow loaded) (Usage: [7:5]:unused [4] Testmux_vmode_sel
// [3:2] membuck_leackage_trim [1:0]:corebuck_leakage_trim  )
#define AM_REG_MCUCTRL_D2ASPARE_D2A_SPARE_S_S        0
#define AM_REG_MCUCTRL_D2ASPARE_D2A_SPARE_S_M        0x000000FF
#define AM_REG_MCUCTRL_D2ASPARE_D2A_SPARE_S(n)       (((uint32_t)(n) << 0) & 0x000000FF)

#endif // MCU_VALIDATION
// #### INTERNAL END ####
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

//*****************************************************************************
//
// MCUCTRL_SHADOWVALID - Register to indicate whether the shadow registers have
// been successfully loaded from the Flash Information Space.
//
//*****************************************************************************
// Indicates whether the bootloader should sleep or deep sleep if no image
// loaded.
#define AM_REG_MCUCTRL_SHADOWVALID_BL_DSLEEP_S       1
#define AM_REG_MCUCTRL_SHADOWVALID_BL_DSLEEP_M       0x00000002
#define AM_REG_MCUCTRL_SHADOWVALID_BL_DSLEEP(n)      (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_SHADOWVALID_BL_DSLEEP_DEEPSLEEP 0x00000002

// Indicates whether the shadow registers contain valid data from the Flash
// Information Space.
#define AM_REG_MCUCTRL_SHADOWVALID_VALID_S           0
#define AM_REG_MCUCTRL_SHADOWVALID_VALID_M           0x00000001
#define AM_REG_MCUCTRL_SHADOWVALID_VALID(n)          (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_SHADOWVALID_VALID_VALID       0x00000001

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

//*****************************************************************************
//
// MCUCTRL_DBGR1 - Read-only debug register 1
//
//*****************************************************************************
// Read-only register for communication validation
#define AM_REG_MCUCTRL_DBGR1_ONETO8_S                0
#define AM_REG_MCUCTRL_DBGR1_ONETO8_M                0xFFFFFFFF
#define AM_REG_MCUCTRL_DBGR1_ONETO8(n)               (((uint32_t)(n) << 0) & 0xFFFFFFFF)

//*****************************************************************************
//
// MCUCTRL_DBGR2 - Read-only debug register 2
//
//*****************************************************************************
// Read-only register for communication validation
#define AM_REG_MCUCTRL_DBGR2_COOLCODE_S              0
#define AM_REG_MCUCTRL_DBGR2_COOLCODE_M              0xFFFFFFFF
#define AM_REG_MCUCTRL_DBGR2_COOLCODE(n)             (((uint32_t)(n) << 0) & 0xFFFFFFFF)

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

// #### INTERNAL BEGIN ####
#ifdef MCU_VALIDATION
//*****************************************************************************
//
// MCUCTRL_MISCDEVST - Miscellaneous Device State
//
//*****************************************************************************
// Indicates the state of the chip's (Failure Analysis Backdoor) FA_BD control.
#define AM_REG_MCUCTRL_MISCDEVST_FABD_S              2
#define AM_REG_MCUCTRL_MISCDEVST_FABD_M              0x00000004
#define AM_REG_MCUCTRL_MISCDEVST_FABD(n)             (((uint32_t)(n) << 2) & 0x00000004)
#define AM_REG_MCUCTRL_MISCDEVST_FABD_DIS            0x00000000
#define AM_REG_MCUCTRL_MISCDEVST_FABD_EN             0x00000004

// Indicates that the Flash was mass erased when a debugger was attached.
#define AM_REG_MCUCTRL_MISCDEVST_FLASHWIPED_S        1
#define AM_REG_MCUCTRL_MISCDEVST_FLASHWIPED_M        0x00000002
#define AM_REG_MCUCTRL_MISCDEVST_FLASHWIPED(n)       (((uint32_t)(n) << 1) & 0x00000002)
#define AM_REG_MCUCTRL_MISCDEVST_FLASHWIPED_TRUE     0x00000002
#define AM_REG_MCUCTRL_MISCDEVST_FLASHWIPED_FALSE    0x00000000

// Indicates that the SRAM was forced to power-off when a debugger was attached.
#define AM_REG_MCUCTRL_MISCDEVST_SRAMWIPED_S         0
#define AM_REG_MCUCTRL_MISCDEVST_SRAMWIPED_M         0x00000001
#define AM_REG_MCUCTRL_MISCDEVST_SRAMWIPED(n)        (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_MISCDEVST_SRAMWIPED_TRUE      0x00000001
#define AM_REG_MCUCTRL_MISCDEVST_SRAMWIPED_FALSE     0x00000000

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
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_M             0x00000700
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL(n)            (((uint32_t)(n) << 8) & 0x00000700)
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_LOW_PWR       0x00000000
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_0MHz          0x00000000
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_HFRC_DIV_2    0x00000100
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_HFRC_DIV_8    0x00000200
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_HFRC_DIV_16   0x00000300
#define AM_REG_MCUCTRL_TPIUCTRL_CLKSEL_HFRC_DIV_32   0x00000400

// TPIU Enable field. When set, the ARM M4 TPIU is enabled and data can be
// streamed out of the MCU's SWO port using the ARM ITM and TPIU modules.
#define AM_REG_MCUCTRL_TPIUCTRL_ENABLE_S             0
#define AM_REG_MCUCTRL_TPIUCTRL_ENABLE_M             0x00000001
#define AM_REG_MCUCTRL_TPIUCTRL_ENABLE(n)            (((uint32_t)(n) << 0) & 0x00000001)
#define AM_REG_MCUCTRL_TPIUCTRL_ENABLE_DIS           0x00000000
#define AM_REG_MCUCTRL_TPIUCTRL_ENABLE_EN            0x00000001

#endif // AM_REG_MCUCTRL_H
