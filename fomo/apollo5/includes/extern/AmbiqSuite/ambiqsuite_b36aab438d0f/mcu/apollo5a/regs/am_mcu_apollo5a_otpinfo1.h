// #### INTERNAL BEGIN ####
//
// Generated by infoGen.py/regMacro.py
//
// Block:       OTP_INFO1
// Chip:        Apollo5a
// Summary:     OTP2 INFO1 Registers
// Description: This document details the contents of the Apollo5 OTP2 INFO1 registers.
//
// #### INTERNAL END ####
//*****************************************************************************
//
//  am_mcu_apollo5a_otp_info1.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_REG_OTP_INFO1_H
#define AM_REG_OTP_INFO1_H

#define AM_REG_OTP_INFO1_BASEADDR 0x42006000
#define AM_REG_OTP_INFO1n(n) 0x42006000

#define AM_REG_OTP_INFO1_MAINPTR_O 0x00000810
#define AM_REG_OTP_INFO1_MAINPTR_ADDR 0x42006810
#define AM_REG_OTP_INFO1_SOCID0_O 0x00000820
#define AM_REG_OTP_INFO1_SOCID0_ADDR 0x42006820
#define AM_REG_OTP_INFO1_SOCID1_O 0x00000824
#define AM_REG_OTP_INFO1_SOCID1_ADDR 0x42006824
#define AM_REG_OTP_INFO1_SOCID2_O 0x00000828
#define AM_REG_OTP_INFO1_SOCID2_ADDR 0x42006828
#define AM_REG_OTP_INFO1_SOCID3_O 0x0000082c
#define AM_REG_OTP_INFO1_SOCID3_ADDR 0x4200682c
#define AM_REG_OTP_INFO1_SOCID4_O 0x00000830
#define AM_REG_OTP_INFO1_SOCID4_ADDR 0x42006830
#define AM_REG_OTP_INFO1_SOCID5_O 0x00000834
#define AM_REG_OTP_INFO1_SOCID5_ADDR 0x42006834
#define AM_REG_OTP_INFO1_SOCID6_O 0x00000838
#define AM_REG_OTP_INFO1_SOCID6_ADDR 0x42006838
#define AM_REG_OTP_INFO1_SOCID7_O 0x0000083c
#define AM_REG_OTP_INFO1_SOCID7_ADDR 0x4200683c
#define AM_REG_OTP_INFO1_PATCH_TRACKER0_O 0x00000840
#define AM_REG_OTP_INFO1_PATCH_TRACKER0_ADDR 0x42006840
#define AM_REG_OTP_INFO1_PATCH_TRACKER1_O 0x00000844
#define AM_REG_OTP_INFO1_PATCH_TRACKER1_ADDR 0x42006844
#define AM_REG_OTP_INFO1_PATCH_TRACKER2_O 0x00000848
#define AM_REG_OTP_INFO1_PATCH_TRACKER2_ADDR 0x42006848
#define AM_REG_OTP_INFO1_PATCH_TRACKER3_O 0x0000084c
#define AM_REG_OTP_INFO1_PATCH_TRACKER3_ADDR 0x4200684c
#define AM_REG_OTP_INFO1_SBR_SDCERT_ADDR_O 0x00000850
#define AM_REG_OTP_INFO1_SBR_SDCERT_ADDR_ADDR 0x42006850
#define AM_REG_OTP_INFO1_SBR_IPT_ADDR_O 0x00000858
#define AM_REG_OTP_INFO1_SBR_IPT_ADDR_ADDR 0x42006858
#define AM_REG_OTP_INFO1_SBR_OPT_ADDR_O 0x0000085c
#define AM_REG_OTP_INFO1_SBR_OPT_ADDR_ADDR 0x4200685c
#define AM_REG_OTP_INFO1_TEMP_CAL_ATE_O 0x00000900
#define AM_REG_OTP_INFO1_TEMP_CAL_ATE_ADDR 0x42006900
#define AM_REG_OTP_INFO1_TEMP_CAL_MEASURED_O 0x00000904
#define AM_REG_OTP_INFO1_TEMP_CAL_MEASURED_ADDR 0x42006904
#define AM_REG_OTP_INFO1_TEMP_CAL_ADC_OFFSET_O 0x00000908
#define AM_REG_OTP_INFO1_TEMP_CAL_ADC_OFFSET_ADDR 0x42006908
#define AM_REG_OTP_INFO1_CHIPSUBREV_O 0x0000090c
#define AM_REG_OTP_INFO1_CHIPSUBREV_ADDR 0x4200690c
#define AM_REG_OTP_INFO1_TRIM_REV_O 0x00000910
#define AM_REG_OTP_INFO1_TRIM_REV_ADDR 0x42006910
#define AM_REG_OTP_INFO1_AUDADC_BINNING_O 0x00000924
#define AM_REG_OTP_INFO1_AUDADC_BINNING_ADDR 0x42006924
#define AM_REG_OTP_INFO1_ADC_GAIN_ERR_O 0x00000928
#define AM_REG_OTP_INFO1_ADC_GAIN_ERR_ADDR 0x42006928
#define AM_REG_OTP_INFO1_ADC_OFFSET_ERR_O 0x0000092c
#define AM_REG_OTP_INFO1_ADC_OFFSET_ERR_ADDR 0x4200692c
#define AM_REG_OTP_INFO1_AUDADC_A0_LG_OFFSET_O 0x00000940
#define AM_REG_OTP_INFO1_AUDADC_A0_LG_OFFSET_ADDR 0x42006940
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_SLOPE_O 0x00000944
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_SLOPE_ADDR 0x42006944
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_INTERCEPT_O 0x00000948
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_INTERCEPT_ADDR 0x42006948
#define AM_REG_OTP_INFO1_AUDADC_A1_LG_OFFSET_O 0x0000094c
#define AM_REG_OTP_INFO1_AUDADC_A1_LG_OFFSET_ADDR 0x4200694c
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_SLOPE_O 0x00000950
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_SLOPE_ADDR 0x42006950
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_INTERCEPT_O 0x00000954
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_INTERCEPT_ADDR 0x42006954
#define AM_REG_OTP_INFO1_AUDADC_B0_LG_OFFSET_O 0x00000958
#define AM_REG_OTP_INFO1_AUDADC_B0_LG_OFFSET_ADDR 0x42006958
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_SLOPE_O 0x0000095c
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_SLOPE_ADDR 0x4200695c
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_INTERCEPT_O 0x00000960
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_INTERCEPT_ADDR 0x42006960
#define AM_REG_OTP_INFO1_AUDADC_B1_LG_OFFSET_O 0x00000964
#define AM_REG_OTP_INFO1_AUDADC_B1_LG_OFFSET_ADDR 0x42006964
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_SLOPE_O 0x00000968
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_SLOPE_ADDR 0x42006968
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_INTERCEPT_O 0x0000096c
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_INTERCEPT_ADDR 0x4200696c

// MAINPTR - The location of the vector table of the main program. This value is read-only.
#define AM_REG_OTP_INFO1_MAINPTR_ADDRESS_S 0
#define AM_REG_OTP_INFO1_MAINPTR_ADDRESS_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_MAINPTR_ADDRESS(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_MAINPTR_ADDRESS_Pos 0
#define AM_REG_OTP_INFO1_MAINPTR_ADDRESS_Msk 0xFFFFFFFF

// SOCID0 - SoC_ID is a statistically unique identification for the device required for the creation of debug certificates.
#define AM_REG_OTP_INFO1_SOCID0_ID_S 0
#define AM_REG_OTP_INFO1_SOCID0_ID_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SOCID0_ID(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SOCID0_ID_Pos 0
#define AM_REG_OTP_INFO1_SOCID0_ID_Msk 0xFFFFFFFF

// SOCID1 - SoC_ID is a statistically unique identification for the device required for the creation of debug certificates.
#define AM_REG_OTP_INFO1_SOCID1_ID_S 0
#define AM_REG_OTP_INFO1_SOCID1_ID_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SOCID1_ID(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SOCID1_ID_Pos 0
#define AM_REG_OTP_INFO1_SOCID1_ID_Msk 0xFFFFFFFF

// SOCID2 - SoC_ID is a statistically unique identification for the device required for the creation of debug certificates.
#define AM_REG_OTP_INFO1_SOCID2_ID_S 0
#define AM_REG_OTP_INFO1_SOCID2_ID_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SOCID2_ID(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SOCID2_ID_Pos 0
#define AM_REG_OTP_INFO1_SOCID2_ID_Msk 0xFFFFFFFF

// SOCID3 - SoC_ID is a statistically unique identification for the device required for the creation of debug certificates.
#define AM_REG_OTP_INFO1_SOCID3_ID_S 0
#define AM_REG_OTP_INFO1_SOCID3_ID_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SOCID3_ID(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SOCID3_ID_Pos 0
#define AM_REG_OTP_INFO1_SOCID3_ID_Msk 0xFFFFFFFF

// SOCID4 - SoC_ID is a statistically unique identification for the device required for the creation of debug certificates.
#define AM_REG_OTP_INFO1_SOCID4_ID_S 0
#define AM_REG_OTP_INFO1_SOCID4_ID_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SOCID4_ID(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SOCID4_ID_Pos 0
#define AM_REG_OTP_INFO1_SOCID4_ID_Msk 0xFFFFFFFF

// SOCID5 - SoC_ID is a statistically unique identification for the device required for the creation of debug certificates.
#define AM_REG_OTP_INFO1_SOCID5_ID_S 0
#define AM_REG_OTP_INFO1_SOCID5_ID_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SOCID5_ID(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SOCID5_ID_Pos 0
#define AM_REG_OTP_INFO1_SOCID5_ID_Msk 0xFFFFFFFF

// SOCID6 - SoC_ID is a statistically unique identification for the device required for the creation of debug certificates.
#define AM_REG_OTP_INFO1_SOCID6_ID_S 0
#define AM_REG_OTP_INFO1_SOCID6_ID_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SOCID6_ID(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SOCID6_ID_Pos 0
#define AM_REG_OTP_INFO1_SOCID6_ID_Msk 0xFFFFFFFF

// SOCID7 - SoC_ID is a statistically unique identification for the device required for the creation of debug certificates.
#define AM_REG_OTP_INFO1_SOCID7_ID_S 0
#define AM_REG_OTP_INFO1_SOCID7_ID_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SOCID7_ID(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SOCID7_ID_Pos 0
#define AM_REG_OTP_INFO1_SOCID7_ID_Msk 0xFFFFFFFF

// PATCH_TRACKER0 - Apollo5a SBL patch tracking [31:0]
#define AM_REG_OTP_INFO1_PATCH_TRACKER0_RSVD0_S 0
#define AM_REG_OTP_INFO1_PATCH_TRACKER0_RSVD0_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_PATCH_TRACKER0_RSVD0(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_PATCH_TRACKER0_RSVD0_Pos 0
#define AM_REG_OTP_INFO1_PATCH_TRACKER0_RSVD0_Msk 0xFFFFFFFF

// PATCH_TRACKER1 - Apollo5a SBL patch tracking [63:32]
#define AM_REG_OTP_INFO1_PATCH_TRACKER1_RSVD1_S 0
#define AM_REG_OTP_INFO1_PATCH_TRACKER1_RSVD1_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_PATCH_TRACKER1_RSVD1(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_PATCH_TRACKER1_RSVD1_Pos 0
#define AM_REG_OTP_INFO1_PATCH_TRACKER1_RSVD1_Msk 0xFFFFFFFF

// PATCH_TRACKER2 - Apollo5a SBL patch tracking [95:64]
#define AM_REG_OTP_INFO1_PATCH_TRACKER2_RSVD2_S 0
#define AM_REG_OTP_INFO1_PATCH_TRACKER2_RSVD2_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_PATCH_TRACKER2_RSVD2(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_PATCH_TRACKER2_RSVD2_Pos 0
#define AM_REG_OTP_INFO1_PATCH_TRACKER2_RSVD2_Msk 0xFFFFFFFF

// PATCH_TRACKER3 - Apollo5a SBL patch tracking [127:96]
#define AM_REG_OTP_INFO1_PATCH_TRACKER3_RSVD3_S 0
#define AM_REG_OTP_INFO1_PATCH_TRACKER3_RSVD3_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_PATCH_TRACKER3_RSVD3(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_PATCH_TRACKER3_RSVD3_Pos 0
#define AM_REG_OTP_INFO1_PATCH_TRACKER3_RSVD3_Msk 0xFFFFFFFF

// SBR_SDCERT_ADDR - A pointer to the SD certificate.
#define AM_REG_OTP_INFO1_SBR_SDCERT_ADDR_ICV_S 0
#define AM_REG_OTP_INFO1_SBR_SDCERT_ADDR_ICV_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SBR_SDCERT_ADDR_ICV(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SBR_SDCERT_ADDR_ICV_Pos 0
#define AM_REG_OTP_INFO1_SBR_SDCERT_ADDR_ICV_Msk 0xFFFFFFFF

// SBR_IPT_ADDR - A pointer to the SBR IPT.
#define AM_REG_OTP_INFO1_SBR_IPT_ADDR_ADDRESS_S 0
#define AM_REG_OTP_INFO1_SBR_IPT_ADDR_ADDRESS_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SBR_IPT_ADDR_ADDRESS(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SBR_IPT_ADDR_ADDRESS_Pos 0
#define AM_REG_OTP_INFO1_SBR_IPT_ADDR_ADDRESS_Msk 0xFFFFFFFF

// SBR_OPT_ADDR - A pointer to the SBR OPT.
#define AM_REG_OTP_INFO1_SBR_OPT_ADDR_ADDRESS_S 0
#define AM_REG_OTP_INFO1_SBR_OPT_ADDR_ADDRESS_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_SBR_OPT_ADDR_ADDRESS(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_SBR_OPT_ADDR_ADDRESS_Pos 0
#define AM_REG_OTP_INFO1_SBR_OPT_ADDR_ADDRESS_Msk 0xFFFFFFFF

// TEMP_CAL_ATE - The temperature measured on the ATE test head when the part's temperature sensor was calibrated.
#define AM_REG_OTP_INFO1_TEMP_CAL_ATE_TEMPERATURE_S 0
#define AM_REG_OTP_INFO1_TEMP_CAL_ATE_TEMPERATURE_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_TEMP_CAL_ATE_TEMPERATURE(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_TEMP_CAL_ATE_TEMPERATURE_Pos 0
#define AM_REG_OTP_INFO1_TEMP_CAL_ATE_TEMPERATURE_Msk 0xFFFFFFFF

// TEMP_CAL_MEASURED - The voltage measured on the analog test mux output when the part's temperature sensor was calibrated.
#define AM_REG_OTP_INFO1_TEMP_CAL_MEASURED_VOLTS_S 0
#define AM_REG_OTP_INFO1_TEMP_CAL_MEASURED_VOLTS_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_TEMP_CAL_MEASURED_VOLTS(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_TEMP_CAL_MEASURED_VOLTS_Pos 0
#define AM_REG_OTP_INFO1_TEMP_CAL_MEASURED_VOLTS_Msk 0xFFFFFFFF

// TEMP_CAL_ADC_OFFSET - The offset voltage measured on for the ADC when the part's temperature sensor was calibrated.
#define AM_REG_OTP_INFO1_TEMP_CAL_ADC_OFFSET_VOLTS_S 0
#define AM_REG_OTP_INFO1_TEMP_CAL_ADC_OFFSET_VOLTS_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_TEMP_CAL_ADC_OFFSET_VOLTS(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_TEMP_CAL_ADC_OFFSET_VOLTS_Pos 0
#define AM_REG_OTP_INFO1_TEMP_CAL_ADC_OFFSET_VOLTS_Msk 0xFFFFFFFF

// CHIPSUBREV -
#define AM_REG_OTP_INFO1_CHIPSUBREV_SUBREV_S 0
#define AM_REG_OTP_INFO1_CHIPSUBREV_SUBREV_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_CHIPSUBREV_SUBREV(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_CHIPSUBREV_SUBREV_Pos 0
#define AM_REG_OTP_INFO1_CHIPSUBREV_SUBREV_Msk 0xFFFFFFFF

// TRIM_REV - Contains the trim revision number.
#define AM_REG_OTP_INFO1_TRIM_REV_REVNUM_S 0
#define AM_REG_OTP_INFO1_TRIM_REV_REVNUM_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_TRIM_REV_REVNUM(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_TRIM_REV_REVNUM_Pos 0
#define AM_REG_OTP_INFO1_TRIM_REV_REVNUM_Msk 0xFFFFFFFF

// AUDADC_BINNING - Audio ADC binning value
#define AM_REG_OTP_INFO1_AUDADC_BINNING_STATUS_S 0
#define AM_REG_OTP_INFO1_AUDADC_BINNING_STATUS_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_BINNING_STATUS(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_BINNING_STATUS_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_BINNING_STATUS_Msk 0xFFFFFFFF

// ADC_GAIN_ERR - This float value is the ADC gain error used for correcting the error from ADC measurement.
#define AM_REG_OTP_INFO1_ADC_GAIN_ERR_ERROR_S 0
#define AM_REG_OTP_INFO1_ADC_GAIN_ERR_ERROR_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_ADC_GAIN_ERR_ERROR(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_ADC_GAIN_ERR_ERROR_Pos 0
#define AM_REG_OTP_INFO1_ADC_GAIN_ERR_ERROR_Msk 0xFFFFFFFF

// ADC_OFFSET_ERR - This float value is the ADC offset (volts).
#define AM_REG_OTP_INFO1_ADC_OFFSET_ERR_OFFSET_S 0
#define AM_REG_OTP_INFO1_ADC_OFFSET_ERR_OFFSET_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_ADC_OFFSET_ERR_OFFSET(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_ADC_OFFSET_ERR_OFFSET_Pos 0
#define AM_REG_OTP_INFO1_ADC_OFFSET_ERR_OFFSET_Msk 0xFFFFFFFF

// AUDADC_A0_LG_OFFSET - This is the float value for low gain offset (less than 12dB). A0 maps to Slot 0
#define AM_REG_OTP_INFO1_AUDADC_A0_LG_OFFSET_LGOFFSET_S 0
#define AM_REG_OTP_INFO1_AUDADC_A0_LG_OFFSET_LGOFFSET_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_A0_LG_OFFSET_LGOFFSET(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_A0_LG_OFFSET_LGOFFSET_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_A0_LG_OFFSET_LGOFFSET_Msk 0xFFFFFFFF

// AUDADC_A0_HG_SLOPE - This is the float value for high gain slope (more than 12dB). A0 maps to Slot 0
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_SLOPE_HGSLOPE_S 0
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_SLOPE_HGSLOPE_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_SLOPE_HGSLOPE(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_SLOPE_HGSLOPE_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_SLOPE_HGSLOPE_Msk 0xFFFFFFFF

// AUDADC_A0_HG_INTERCEPT - This is the float value for high gain intercept (more than 12dB). A0 maps to Slot 0
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_INTERCEPT_HGINTERCEPT_S 0
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_INTERCEPT_HGINTERCEPT_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_INTERCEPT_HGINTERCEPT(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_INTERCEPT_HGINTERCEPT_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_A0_HG_INTERCEPT_HGINTERCEPT_Msk 0xFFFFFFFF

// AUDADC_A1_LG_OFFSET - This is the float value for low gain offset (less than 12dB). A1 maps to Slot 1
#define AM_REG_OTP_INFO1_AUDADC_A1_LG_OFFSET_LGOFFSET_S 0
#define AM_REG_OTP_INFO1_AUDADC_A1_LG_OFFSET_LGOFFSET_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_A1_LG_OFFSET_LGOFFSET(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_A1_LG_OFFSET_LGOFFSET_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_A1_LG_OFFSET_LGOFFSET_Msk 0xFFFFFFFF

// AUDADC_A1_HG_SLOPE - This is the float value for high gain slope (more than 12dB). A1 maps to Slot 1
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_SLOPE_HGSLOPE_S 0
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_SLOPE_HGSLOPE_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_SLOPE_HGSLOPE(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_SLOPE_HGSLOPE_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_SLOPE_HGSLOPE_Msk 0xFFFFFFFF

// AUDADC_A1_HG_INTERCEPT - This is the float value for high gain intercept (more than 12dB). A1 maps to Slot 1
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_INTERCEPT_HGINTERCEPT_S 0
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_INTERCEPT_HGINTERCEPT_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_INTERCEPT_HGINTERCEPT(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_INTERCEPT_HGINTERCEPT_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_A1_HG_INTERCEPT_HGINTERCEPT_Msk 0xFFFFFFFF

// AUDADC_B0_LG_OFFSET - This is the float value for low gain offset (less than 12dB). B0 maps to Slot 2
#define AM_REG_OTP_INFO1_AUDADC_B0_LG_OFFSET_LGOFFSET_S 0
#define AM_REG_OTP_INFO1_AUDADC_B0_LG_OFFSET_LGOFFSET_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_B0_LG_OFFSET_LGOFFSET(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_B0_LG_OFFSET_LGOFFSET_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_B0_LG_OFFSET_LGOFFSET_Msk 0xFFFFFFFF

// AUDADC_B0_HG_SLOPE - This is the float value for high gain slope (more than 12dB). B0 maps to Slot 2
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_SLOPE_HGSLOPE_S 0
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_SLOPE_HGSLOPE_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_SLOPE_HGSLOPE(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_SLOPE_HGSLOPE_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_SLOPE_HGSLOPE_Msk 0xFFFFFFFF

// AUDADC_B0_HG_INTERCEPT - This is the float value for high gain intercept (more than 12dB). B0 maps to Slot 2
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_INTERCEPT_HGINTERCEPT_S 0
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_INTERCEPT_HGINTERCEPT_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_INTERCEPT_HGINTERCEPT(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_INTERCEPT_HGINTERCEPT_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_B0_HG_INTERCEPT_HGINTERCEPT_Msk 0xFFFFFFFF

// AUDADC_B1_LG_OFFSET - This is the float value for low gain offset (less than 12dB). B1 maps to Slot 3
#define AM_REG_OTP_INFO1_AUDADC_B1_LG_OFFSET_LGOFFSET_S 0
#define AM_REG_OTP_INFO1_AUDADC_B1_LG_OFFSET_LGOFFSET_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_B1_LG_OFFSET_LGOFFSET(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_B1_LG_OFFSET_LGOFFSET_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_B1_LG_OFFSET_LGOFFSET_Msk 0xFFFFFFFF

// AUDADC_B1_HG_SLOPE - This is the float value for high gain slope (more than 12dB). B1 maps to Slot 3
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_SLOPE_HGSLOPE_S 0
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_SLOPE_HGSLOPE_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_SLOPE_HGSLOPE(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_SLOPE_HGSLOPE_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_SLOPE_HGSLOPE_Msk 0xFFFFFFFF

// AUDADC_B1_HG_INTERCEPT - This is the float value for high gain intercept (more than 12dB). B1 maps to Slot 3
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_INTERCEPT_HGINTERCEPT_S 0
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_INTERCEPT_HGINTERCEPT_M 0xFFFFFFFF
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_INTERCEPT_HGINTERCEPT(n) (((uint32_t)(n) << 0) & 0xFFFFFFFF)
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_INTERCEPT_HGINTERCEPT_Pos 0
#define AM_REG_OTP_INFO1_AUDADC_B1_HG_INTERCEPT_HGINTERCEPT_Msk 0xFFFFFFFF

#endif