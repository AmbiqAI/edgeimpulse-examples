#include "am_sbr_test.h"
//#include "am_errorCodes.h"

// globals
security_t g_secModeLCS;

// From am_errorCodes.h, sha 54a1f9 8/21/2023
#define BR_STATUS_CKPT9             0x17C558CB
#define BR_STATUS_CKPT12            0x182FEADC
#define AM_SECBOOT_STATUS_SUCCESS   0x9086b220
#define AM_SBR_NONSBL_NS_BOOT       0x2747241B

//
// expected results per secutiry mode and lcs in [5][4] arrays
//

// Expected Bootstatus Register
const uint32_t expected_boot_status_code[5][4] =  {
     //   CM                                    DM                         Sec                        RMA
     { BR_STATUS_CKPT9,                                   -1,                         -1,                         -1 },  // CM Vanilla (Security word not written)
     { AM_SECBOOT_STATUS_SUCCESS,  AM_SECBOOT_STATUS_SUCCESS,  AM_SECBOOT_STATUS_SUCCESS,  AM_SECBOOT_STATUS_SUCCESS },  // AmbSBL-nonsecure mode
     { AM_SECBOOT_STATUS_SUCCESS,  AM_SECBOOT_STATUS_SUCCESS,  AM_SECBOOT_STATUS_SUCCESS,  AM_SECBOOT_STATUS_SUCCESS },  // AmbSBL-secure mode
     { AM_SBR_NONSBL_NS_BOOT    ,  AM_SBR_NONSBL_NS_BOOT    ,  AM_SBR_NONSBL_NS_BOOT    ,  AM_SBR_NONSBL_NS_BOOT     },  // nonSBL-nonsecure mode
     { AM_SBR_NONSBL_NS_BOOT    ,  AM_SBR_NONSBL_NS_BOOT    ,  AM_SBR_NONSBL_NS_BOOT    ,  AM_SBR_NONSBL_NS_BOOT     },  // nonSBL-secure mode
 };

 // Debug Words   (each element is a 'debug_words_t' type, an array of 4 words)
const debug_words_t expected_sbr_debug_words[5][4] = {
     {   // CM Vanilla (Security word not written)
         {{0x00000000,       BR_STATUS_CKPT9, 0x00000000, 0x00000000}},  // CM
         {{        -1,               -1,         -1,         -1}},  // DM  (not-possible)
         {{        -1,               -1,         -1,         -1}},  // Sec (not-possible)
         {{        -1,               -1,         -1,         -1}},  // RMA (not-possible)
     },
     {   // AmbSBL-nonsecure mode
         {{0x00000000,       0x00000000, 0x00000000, 0x00000000}},  // CM
         {{0x00000000,       0x00000000, 0x00000000, 0x00000000}},  // DM
         {{0x00000000,       0x00000000, 0x00000000, 0x00000000}},  // Sec
         {{0x00000000,       0x00000000, 0x00000000, 0x00000000}},  // RMA
     },
     {   // AmbSBL-secure mode
         {{0x00000000,       0x00000000, 0x00000000, 0x00000000}},  // CM
         {{0x00000000,       0x00000000, 0x00000000, 0x00000000}},  // DM
         {{0x00000000,       0x00000000, 0x00000000, 0x00000000}},  // Sec
         {{0x00000000,       0x00000000, 0x00000000, 0x00000000}},  // RMA
     },
     {   // nonSBL-nonsecure mode
         {{0x00000000,  BR_STATUS_CKPT12, 0x00000000, 0x00000000}},  // CM
         {{0x00000000,  BR_STATUS_CKPT12, 0x00000000, 0x00000000}},  // DM
         {{0x00000000,  BR_STATUS_CKPT12, 0x00000000, 0x00000000}},  // Sec
         {{0x00000000,  BR_STATUS_CKPT12, 0x00000000, 0x00000000}},  // RMA
     },
     {   // nonSBL-secure mode
         {{0x00000000,  BR_STATUS_CKPT12, 0x00000000, 0x00000000}},  // CM
         {{0x00000000,  BR_STATUS_CKPT12, 0x00000000, 0x00000000}},  // DM
         {{0x00000000,  BR_STATUS_CKPT12, 0x0200816F, 0x020000D5}},  // Sec
         {{0x00000000,  BR_STATUS_CKPT12, 0x00000000, 0x00000000}},  // RMA
     },
 };

// SBRlock expected results
const bool sbr_lock_results_data[5][4] =  {
     //  CM         DM      Sec      RMA
     { UNLOCKED,    -1,      -1,      -1 },  // CM Vanilla (Security word not written)
     { LOCKED,  LOCKED,  LOCKED,  LOCKED },  // AmbSBL-nonsecure mode
     { LOCKED,  LOCKED,  LOCKED,  LOCKED },  // AmbSBL-secure mode
     { LOCKED,  LOCKED,  LOCKED,  LOCKED },  // nonSBL-nonsecure mode
     { LOCKED,  LOCKED,  LOCKED,  LOCKED },  // nonSBL-secure mode
 };

 // SBLlock expected results
const bool sbl_lock_results_data[5][4] =  {
     //    CM         DM        Sec        RMA
     { UNLOCKED,        -1,        -1,        -1 },  // CM Vanilla (Security word not written)
     {   LOCKED,    LOCKED,    LOCKED,    LOCKED },  // AmbSBL-nonsecure mode
     {   LOCKED,    LOCKED,    LOCKED,    LOCKED },  // AmbSBL-secure mode
     {   LOCKED,    LOCKED,    LOCKED,    LOCKED },  // nonSBL-nonsecure mode
     {   LOCKED,    LOCKED,    LOCKED,    LOCKED },  // nonSBL-secure mode
 };

// DCU Lock bits   (each element is a 'dcu_bits_t' type, an array of 4 words)
const dcu_bits_t expected_dcu_lock_bits[5][4] = {
     {   // CM Vanilla (Security word not written)
         {{0x00000000, 0x00000000, 0x00000000, 0x00000000}},  // CM
         {{        -1,         -1,         -1,         -1}},  // DM  (not-possible)
         {{        -1,         -1,         -1,         -1}},  // Sec (not-possible)
         {{        -1,         -1,         -1,         -1}},  // RMA (not-possible)
     },
     {   // AmbSBL-nonsecure mode
         {{0x00000000, 0x00000000, 0x00000000, 0x00000000}},  // CM
         {{0x3FFF8E3F, 0x8FFFFFFE, 0x00000000, 0x00000000}},  // DM
         {{0x3FFF8E3F, 0x8FFFFFFE, 0x00000000, 0x00000000}},  // Sec
         {{0x00000000, 0x00000000, 0x00000000, 0x00000000}},  // RMA
     },
     {   // AmbSBL-secure mode
         {{0x00000000, 0x00000000, 0x00000000, 0x00000000}},  // CM
         {{0x3FFF8E3F, 0x8FFFFFFE, 0x00000000, 0x00000000}},  // DM
         {{0x3FFF8E3F, 0x8FFFFFFE, 0x00000000, 0x00000000}},  // Sec
         {{0x00000000, 0x00000000, 0x00000000, 0x00000000}},  // RMA
     },
     {   // nonSBL-nonsecure mode
         {{0x00000000, 0x00000000, 0x00000000, 0x00000000}},  // CM
         {{0x3FFF8E3F, 0x8FFFFFFE, 0x00000000, 0x00000000}},  // DM
         {{0x3FFF8E3F, 0x8FFFFFFE, 0x00000000, 0x00000000}},  // Sec
         {{0x00000000, 0x00000000, 0x00000000, 0x00000000}},  // RMA
     },
     {   // nonSBL-secure mode
         {{0x00000000, 0x00000000, 0x00000000, 0x00000000}},  // CM
         {{0x3FFF8E3F, 0x8FFFFFFE, 0x00000000, 0x00000000}},  // DM
         {{0x3FFF8E3F, 0x8FFFFFFE, 0x00000000, 0x00000000}},  // Sec
         {{0x00000000, 0x00000000, 0x00000000, 0x00000000}},  // RMA
     },
 };

 // DCU Enable bits  (each element is a 'dcu_bits_t' type, an array of 4 words)
const dcu_bits_t expected_dcu_enable_bits[5][4] = {
     {   // CM Vanilla (Security word not written)
         {{0x6DB6DB68, 0x5B6DB6DB, 0x6DB6DB6D, 0x5B6DB6DB}},  // CM
         {{        -1,         -1,         -1,         -1}},  // DM  (not-possible)
         {{        -1,         -1,         -1,         -1}},  // Sec (not-possible)
         {{        -1,         -1,         -1,         -1}},  // RMA (not-possible)
     },
     {   // AmbSBL-nonsecure mode
         {{0x6DB6DB68, 0x5B6DB6DB, 0x6DB6DB6D, 0x5B6DB6DB}},  // CM
         {{0x40005140, 0x00000001, 0x6DB6DB6D, 0x5B6DB6DB}},  // DM
         {{0x40005140, 0x00000001, 0x00000000, 0x00000000}},  // Sec
         {{0x6DB6DB68, 0x5B6DB6DB, 0x6DB6DB6D, 0x5B6DB6DB}},  // RMA
     },
     {   // AmbSBL-secure mode
         {{0x6DB6DB68, 0x5B6DB6DB, 0x6DB6DB6D, 0x5B6DB6DB}},  // CM
         {{0x40005140, 0x00000001, 0x6DB6DB6D, 0x5B6DB6DB}},  // DM
         {{0x40005140, 0x00000001, 0x00000000, 0x00000000}},  // Sec
         {{0x6DB6DB68, 0x5B6DB6DB, 0x6DB6DB6D, 0x5B6DB6DB}},  // RMA
     },
     {   // nonSBL-nonsecure mode
         {{0x6DB6DB68, 0x5B6DB6DB, 0x6DB6DB6D, 0x5B6DB6DB}},  // CM
         {{0x40005140, 0x00000001, 0x6DB6DB6D, 0x5B6DB6DB}},  // DM
         {{0x40005140, 0x00000001, 0x00000000, 0x00000000}},  // Sec
         {{0x6DB6DB68, 0x5B6DB6DB, 0x6DB6DB6D, 0x5B6DB6DB}},  // RMA
     },
     {   // nonSBL-secure mode
         {{0x6DB6DB68, 0x5B6DB6DB, 0x6DB6DB6D, 0x5B6DB6DB}},  // CM
         {{0x40005140, 0x00000001, 0x6DB6DB6D, 0x5B6DB6DB}},  // DM
         {{0x40005140, 0x00000001, 0x00000000, 0x00000000}},  // Sec
         {{0x6DB6DB68, 0x5B6DB6DB, 0x6DB6DB6D, 0x5B6DB6DB}},  // RMA
     },
 };

// Expected VTOR register for each mode (Load Address)
const uint32_t vtor_results_data[5][4] = {
     // CM             DM          Sec         RMA
     { 0x00400000,         -1,         -1,         -1 },  // CM Vanilla (Security word not written)
     { 0x00410000, 0x00410000, 0x00410000, 0x00410000 },  // AmbSBL-nonsecure mode
     { 0x00410000, 0x00410000, 0x00410000, 0x00410000 },  // AmbSBL-secure mode
     { 0x00400000, 0x00400000, 0x00400000, 0x00400000 },  // nonSBL-nonsecure mode
     { 0x00400000, 0x00400000, 0x00400000, 0x00400000 },  // nonSBL-secure mode
 };

// Expected value for MCUCTRL Write Protection Bits
const mcu_fprot_bits_t mcu_wprot_ref_val[5][4] =
{
    {   // CM VANILLA  Mode  (Security word not written)
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // CM LCS
        {{        -1,         -1,         -1,         -1,         -1,         -1,         -1,         -1}},    // DM LCS (Not Possible)
        {{        -1,         -1,         -1,         -1,         -1,         -1,         -1,         -1}},    // Secure LCS (Not Possible)
        {{        -1,         -1,         -1,         -1,         -1,         -1,         -1,         -1}},    // RMA LCS (Not Possible)
    },
    {   // Ambiq SBL Non-Secure Mode
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // CM LCS
        {{0xFFFFFFF0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // DM LCS
        {{0xFFFFFFF0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // Secure LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // RMA LCS
    },
    {   // Ambiq SBL Secure Mode
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // CM LCS
        {{0xFFFFFFF0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // DM LCS
        {{0xFFFFFFF0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // Secure LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // RMA LCS
    },
    {   // Non Ambiq SBL Non-Secure Mode
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // CM LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // DM LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // Secure LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // RMA LCS
    },
    {   // Non Ambiq SBL Secure Mode
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // CM LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // DM LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // Secure LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // RMA LCS
    },
};

// Expected value for MCUCTRL Read Protection Bits
const mcu_fprot_bits_t mcu_rprot_ref_val[5][4] =
{
    {   // CM VANILLA  Mode  (Security word not written)
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // CM LCS
        {{        -1,         -1,         -1,         -1,         -1,         -1,         -1,         -1}},    // DM LCS (Not Possible)
        {{        -1,         -1,         -1,         -1,         -1,         -1,         -1,         -1}},    // Secure LCS (Not Possible)
        {{        -1,         -1,         -1,         -1,         -1,         -1,         -1,         -1}},    // RMA LCS (Not Possible)
    },
    {   // Ambiq SBL Non-Secure Mode
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // CM LCS
        {{0xFFFFFFF0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // DM LCS
        {{0xFFFFFFF0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // Secure LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // RMA LCS
    },
    {   // Ambiq SBL Secure Mode
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // CM LCS
        {{0xFFFFFFF0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // DM LCS
        {{0xFFFFFFF0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // Secure LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // RMA LCS
    },
    {   // Non Ambiq SBL Non-Secure Mode
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // CM LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // DM LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // Secure LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // RMA LCS
    },
    {   // Non Ambiq SBL Secure Mode
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // CM LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // DM LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // Secure LCS
        {{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}},    // RMA LCS
    },
};

// Expected PUBLock results
const uint32_t publock_results_data[5][4] =  {
     //CM        DM        Sec       RMA
     { 0x0,       -1,       -1,       -1 },  // CM Vanilla (Security word not written)
     { 0x5,      0x5,      0x5,      0x5 },  // AmbSBL-nonsecure mode
     { 0x5,      0x5,      0x5,      0x5 },  // AmbSBL-secure mode
     { 0x5,      0x5,      0x5,      0x5 },  // nonSBL-nonsecure mode
     { 0x5,      0x5,      0x5,      0x5 },  // nonSBL-secure mode
 };

 // Expected CRYPTO HOST AO LOCK BITS result
const uint32_t hostaolock_reg_data[5][4] =
{
    // CM             DM          Sec         RMA
     { 0x00000080,         -1,         -1,         -1 },  // CM Vanilla (Security word not written)
     { 0x00000146, 0x00000166, 0x00000126, 0x00000146 },  // AmbSBL-nonsecure mode
     { 0x00000146, 0x00000166, 0x00000126, 0x00000146 },  // AmbSBL-secure mode
     { 0x000000C6, 0x000000E6, 0x000000A6, 0x000000C6 },  // nonSBL-nonsecure mode
     { 0x000000C6, 0x000000E6, 0x000000A6, 0x000000C6 },  // nonSBL-secure mode
};

// Expected OEM Flag Data
const uint32_t expected_oem_flags[5][4] =
{
    // CM          DM          Secure      RMA
    { 0x00000000,         -1,         -1,         -1},     // CM Vanilla (Security word not written)
    { 0x00000000, 0x00000000, 0x0000003D, 0xC0000000 },    // AmbiqSBL Non Secure
    { 0x00000000, 0x00000000, 0x0000003D, 0xC0000000 },    // AmbiqSBL Secure
    { 0x00000000, 0x00000000, 0x0000003D, 0xC0000000 },    // Non AmbiqSBL Non Secure
    { 0x00000000, 0x00000000, 0x0000003D, 0xC0000000 },    // Non AmbiqSBL Secure
};

uint32_t ui32refValue;
uint32_t refvalue[4];

