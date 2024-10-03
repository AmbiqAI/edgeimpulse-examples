
//*****************************************************************************
//
// SBR Test Defines
//
//*****************************************************************************

#ifndef AM_SBR_TEST_GLOBAL_H
#define AM_SBR_TEST_GLOBAL_H
#include <stdint.h>
#include <stdbool.h>
//#include "otp1infoc.h"  // internal registers INFOC, include before am_bsp.h
#include "am_bsp.h"
#include "am_util.h"
#include "am_reg_base_addresses.h"

#define SBR_TEST_VERSIONSTR  "v0.7"

//#define PTE_TEST   // Define for PTE Test build (with GPIO lines signalling)

#define BOOTROM_VERSION_RTN  0x21000009     // expected bootrom version returned from helper fuction

#define SECURITY_WORD_MASK  (AM_REG_OTP_INFOC_SECURITY_AMB_SECBOOT_M  |  AM_REG_OTP_INFOC_SECURITY_AMB_SECBOOT_INST_M |  \
                             AM_REG_OTP_INFOC_SECURITY_CUST_SECBOOT_M |  AM_REG_OTP_INFOC_SECURITY_AMB_CRYPTO_PUBLOCK_M)


// Defines from am_sbr_config.h -- copied here because including it inclues many others, adds 5+ additional search paths and many other (crypto) includes
#define     AM_SBR_RAM_BASE                             DTCM_BASEADDR
#define     AM_SBR_RAM_SIZE                             (0x10000 - 0x10) // 64K. Leave upper 16 bytes to leave some breadcrumbs for SBL or status
#define     AM_SBR_USER_RAM_BASE                        (AM_SBR_RAM_BASE + 64 * 1024) // 64KB is the amount of RAM used by the SBR + SBL.

// Debug Status
#define     AM_SBR_SDCERT_STATUS_ADDR                   (AM_SBR_RAM_BASE + AM_SBR_RAM_SIZE + 0xC)
#define     AM_SBR_PROV_STATUS_ADDR                     (AM_SBR_RAM_BASE + AM_SBR_RAM_SIZE + 0x8)
#define     AM_SBR_CHECKPOINT_ADDR                      (AM_SBR_RAM_BASE + AM_SBR_RAM_SIZE + 0x4)
#define     AM_SBR_DBGSTAT_ADDR                         (AM_SBR_RAM_BASE + AM_SBR_RAM_SIZE + 0x0)

//
// Typedefs
//

// Security mode/lcs structure
typedef struct
{
    uint32_t  mode;
    uint32_t  lcs;
} security_t;

// Success/Fail return type for each test
typedef enum
{
    AM_STATUS_SUCCESS = 0,
    AM_STATUS_FAILURE = 1,
} am_status_e;


// Enum for different security modes - must be numerical order from 0 -- used to index into expected results arrays
   //TODO:  Add Apollo4 mode escape as a separate mode type 
   //TODO:  Add SBL replacement (run @ 0x00402000 with icv content cert) as a mode
typedef enum
{
    BOOTMODE_NONE = 0,            // 'Vanilla' unprovisioned part
    BOOTMODE_AMB_SBL_NONSECURE,   // Ambq SBL - NonSecure
    BOOTMODE_AMB_SBL_SECURE,      // Ambq SBL - Secure
    BOOTMODE_NOSBL_NONSECURE,     // Customer SBL - NonSecure
    BOOTMODE_NOSBL_SECURE,        // Customer SBL - Secure
    BOOTMODE_ERROR,               // Bootmode Error
} am_sbr_bootmode_e;

// Enum for LCS defines - must be numerical order from 0 -- used to index into expected results arrays
typedef enum
{
    CM_LCS = 0,
    DM_LCS,
    SECURE_LCS,
    RMA_LCS,
    LCS_ERROR,
}  lcs_e;

// DCU bits type - an array of 4 words
typedef struct
{
    uint32_t dcu_word[4];
} dcu_bits_t;

// Locked/Unlocked enums
typedef enum
{
    LOCKED = 0,
    UNLOCKED = 1,
} locked_t;

// SBR Debug words type (array of 4 words)
typedef struct
{
    uint32_t debug_word[4];
} debug_words_t;

// Flash protection type
typedef struct
{
    uint32_t fprot_word[8];
} mcu_fprot_bits_t;

typedef struct
{
    uint32_t nvcounter_reg[5];
}nvcounter_reg_t;

// Struct for the saved registers (pushed at start)
typedef struct {    // the order must match the order pushed in save_startup_regs()
    uint32_t  r8;
    uint32_t  r9;
    uint32_t  r10;
    uint32_t  r11;
    uint32_t  r12;
    uint32_t  MSPLIM;
    uint32_t  PSPLIM;
    uint32_t  lr;
    uint32_t  r0;
    uint32_t  r1;
    uint32_t  r2;
    uint32_t  r3;
    uint32_t  r4;
    uint32_t  r5;
    uint32_t  r6;
    uint32_t  r7;
} saved_regs_t;

//
extern security_t g_secModeLCS;
extern uint32_t __INITIAL_SP;

//
// Externs for expected Results Data
//
extern const uint32_t         expected_boot_status_code[5][4];
extern const debug_words_t    expected_sbr_debug_words[5][4];
extern const bool             sbr_lock_results_data[5][4];
extern const bool             sbl_lock_results_data[5][4];
extern const dcu_bits_t       expected_dcu_lock_bits[5][4];
extern const dcu_bits_t       expected_dcu_enable_bits[5][4];
extern const uint32_t         vtor_results_data[5][4];
extern const uint32_t         vtor_results_data[5][4];
extern const mcu_fprot_bits_t mcu_wprot_ref_val[5][4];
extern const mcu_fprot_bits_t mcu_rprot_ref_val[5][4];
extern const uint32_t         publock_results_data[5][4];
extern const uint32_t         hostaolock_reg_data[5][4];
extern const uint32_t         expected_oem_flags[5][4];
//*****************************************************************************
// Function prototypes
//*****************************************************************************
void output_status(char *test_name, uint32_t pf, uint32_t *tests_run, uint32_t *test_passed, char *info_str);

void print_buffer_init( void );
extern uint32_t output_buffer ( char* pcFmt, ... );
void print_buffer ( void );

extern uint32_t cleared_reg_test(security_t *sec_mode, char *info_buf);
extern uint32_t DTCM_Memory_Test(security_t *sec_mode, char *info_buf);
extern uint32_t boot_status_code(security_t *sec_mode, char *info_buf);
extern uint32_t sbr_debug_words(security_t *sec_mode, char *info_buf);
extern uint32_t sbr_lock_test(security_t *sec_mode, char *info_buf);
extern uint32_t sbl_lock_test(security_t *sec_mode, char *info_buf);
extern uint32_t dcu_lock_test(security_t *mode_lcs, char *info_buf);
extern uint32_t dcu_enables_test(security_t *mode_lcs, char *info_buf);
extern uint32_t vtor_test(security_t *sec_mode, char *info_buf);
extern uint32_t bootrom_copy_test(security_t *sec_mode, char *info_buf);
extern uint32_t helper_copy_test(security_t *sec_mode, char *info_buf);
extern uint32_t helper_execute_test(security_t *sec_mode, char *info_buf);
extern uint32_t run_mcuctrl_protection_test(security_t *sec_mode, char *info_buf);
extern uint32_t run_publock_test( security_t* sec_mode, char* info_buf );
extern uint32_t run_crypto_hostao_lock_test( security_t* sec_lcs, char* info_buf );
extern uint32_t run_otp_oem_flag_test( security_t* sec_lcs, char* info_buf );
extern uint32_t run_otp_nvcounters_test( security_t* sec_lcs, char* info_buf );
extern uint32_t run_ota_test( security_t* sec_lcs, char* info_buf );
#endif // AM_SBR_TEST_GLOBAL_H
