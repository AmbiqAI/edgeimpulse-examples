
#include "am_sbr_test.h"
#include "am_memory_map.h"
//#include "am_sbrConfig.h"

#define BL_NV_HELPERS_DEFINE_STRUCT_AND_CONSTANT_INITIALZERS
//#include "bl_nv_helpers_INTERNAL_ONLY.h"

// ****************************************************************************
//
// Cleared Registers Test
//    verify that the intial register are cleared (as much) as expected
//    for Apollo5a r0 r2, r6, r7, r8 and PSPLIM and MSPLIM are not be cleared
//
// ****************************************************************************

 uint32_t cleared_reg_test(security_t *sec_mode, char *info_buf)
{
#if defined(__ARMCC_VERSION)
    saved_regs_t *saved_regs = (saved_regs_t *) (&__INITIAL_SP+1);
#elif __GNUC__
    // startup_gcc.c reserves 1024 words for the system stack
    saved_regs_t *saved_regs = (saved_regs_t *) ((uint32_t*)AM_MEM_STACK + 1024 + 1);
#else
    return AM_STATUS_FAILURE;
#endif

    uint32_t rtn_status = AM_STATUS_SUCCESS;
    uint32_t read_result = 0;

    // Check that expected regs are 0x00000000 - for all but CM Vanilla mode
    if (sec_mode->mode != BOOTMODE_NONE)
    {
        read_result |= saved_regs->r1;
        //read_result |= saved_regs->r3;    // latest SBL has non-zero values in R3 and R4
        //read_result |= saved_regs->r4;
        read_result |= saved_regs->r5;
        read_result |= saved_regs->r9;
        read_result |= saved_regs->r10;
        read_result |= saved_regs->r11;
        read_result |= saved_regs->r12;
        read_result |= saved_regs->lr;
    }
    else  // SBR exit to CM Vanilla mode
    {
        read_result |= saved_regs->r4;
        read_result |= saved_regs->r5;
        read_result |= saved_regs->r6;
    }

    // the result should still be 0x00000000
    if (read_result != 0x00000000)
        rtn_status = AM_STATUS_FAILURE;

    am_util_string_strcpy(info_buf, (read_result == 0) ? "Expected Registers are cleared" : "Registers not cleared as expected");
    return rtn_status;
}

// ****************************************************************************
//
// Check DTCM Memory
//    the first 64K of DTCM is cleared except the last 4 words in all modes except unprovisioned CM
//
// ****************************************************************************
 uint32_t DTCM_Memory_Test(security_t *sec_mode, char *info_buf)
{
    uint32_t *paddrSrc = (uint32_t *)DTCM_BASEADDR;
    uint32_t read_result = 0;
    uint32_t rtn_status = AM_STATUS_SUCCESS;

    // Check if all of first 64K of DTCM is 0 (except the last 4 words)
    for (uint32_t i = 0; i < (0x10000/4)-4 ; i++)
    {
        read_result |= *paddrSrc++;
    }

    // The whole region should read 0 in every mode except for unprovisioned CM
    if (read_result != 0x00000000 && sec_mode->lcs != BOOTMODE_NONE)
        rtn_status = AM_STATUS_FAILURE;

    am_util_string_strcpy(info_buf, (read_result == 0) ? "DTCM Memory area cleared" : "DTCM Memory not expected to be cleared");
    return rtn_status;
}

// ****************************************************************************
//
// Check Boot Status Code
//
// ****************************************************************************
uint32_t boot_status_code(security_t *sec_mode, char *info_buf)
{
    // get the expected Boot Status code
    uint32_t exp_bootStatus = expected_boot_status_code[sec_mode->mode][sec_mode->lcs];

    // Read the bootstatus code - check for match
    if (SECURITY->SRCADDR == exp_bootStatus)
    {
       am_util_stdio_sprintf(info_buf, "Boot status code: %X", SECURITY->SRCADDR);
       return AM_STATUS_SUCCESS;
    }

    // Return - SBRLock not as expected
    am_util_stdio_sprintf(info_buf, "Unexpected boot status code: %X", SECURITY->SRCADDR);
    return AM_STATUS_FAILURE;
}

// ****************************************************************************
//
// SBR Debug Words
//
// ****************************************************************************
uint32_t sbr_debug_words(security_t *sec_mode, char *info_buf)
{
    uint32_t x;
    uint32_t debugWords[4];

    debug_words_t exp_debug_words = expected_sbr_debug_words[sec_mode->mode][sec_mode->lcs];

    // Read the SBR Debug Words
    debugWords[0] = *(uint32_t *) AM_SBR_DBGSTAT_ADDR;
    debugWords[1] = *(uint32_t *) AM_SBR_CHECKPOINT_ADDR;
    debugWords[2] = *(uint32_t *) AM_SBR_PROV_STATUS_ADDR;
    debugWords[3] = *(uint32_t *) AM_SBR_SDCERT_STATUS_ADDR;

    // create return info string
    am_util_stdio_sprintf(info_buf, "Debug Words: %08X-%08X-%08X-%08X",  debugWords[0],  debugWords[1],  debugWords[2],  debugWords[3]);

    // check that the current DCU Locks match the expected
    for (x = 0; x < 4; x++)
    {
        if (debugWords[x] != exp_debug_words.debug_word[x])
        {
            return AM_STATUS_FAILURE;
        }
    }
    return AM_STATUS_SUCCESS;
}

// ****************************************************************************
//
// Check SBR Lock
//
// ****************************************************************************
uint32_t sbr_lock_test(security_t *sec_mode, char *info_buf)
{
   // get the expected SBR lock status
    bool lockStatus = sbr_lock_results_data[sec_mode->mode][sec_mode->lcs];

    // Read the SBR lock status - check for match
    if (MCUCTRL->BOOTLOADER_b.SBRLOCK == lockStatus)
    {
       am_util_string_strcpy(info_buf, (lockStatus == LOCKED) ? "SBR Locked" : "SBR Unlocked");
       return AM_STATUS_SUCCESS;
    }

    // Return - SBRLock not as expected
    am_util_string_strcpy(info_buf, (lockStatus == UNLOCKED) ? "SBR Locked" : "SBR Unlocked");
    return AM_STATUS_FAILURE;
}

// ****************************************************************************
//
// Check SBL Lock
//
// ****************************************************************************
uint32_t sbl_lock_test(security_t *sec_mode, char *info_buf)
{
    // get the expected SBR lock status
    bool lockStatus = sbl_lock_results_data[sec_mode->mode][sec_mode->lcs];

    // TODO:  verify that

    // Read the SBL lock status - check for match
    if (MCUCTRL->BOOTLOADER_b.SBLLOCK == lockStatus)
    {
       am_util_string_strcpy(info_buf, (lockStatus == LOCKED) ? "SBL Locked" : "SBL Unlocked");
       return AM_STATUS_SUCCESS;
    }

   // Return - SBLLock not as expected
   am_util_string_strcpy(info_buf, (lockStatus == UNLOCKED) ? "SBL Locked" : "SBL Unlocked");
   return AM_STATUS_FAILURE;
}

// ****************************************************************************
//
// Check DCU Lock
//
// ****************************************************************************
uint32_t dcu_lock_test(security_t *mode_lcs, char *info_buf)
{
    uint32_t x;
    uint32_t dcuLock[4];

    dcu_bits_t exp_DCU_Lock_words = expected_dcu_lock_bits[mode_lcs->mode][mode_lcs->lcs];

    // Read the DCU lock status
    dcuLock[0] = CRYPTO->HOSTDCULOCK0;
    dcuLock[1] = CRYPTO->HOSTDCULOCK1;
    dcuLock[2] = CRYPTO->HOSTDCULOCK2;
    dcuLock[3] = CRYPTO->HOSTDCULOCK3;

    // create return info string
    am_util_stdio_sprintf(info_buf, "DCU Locks: %08X-%08X-%08X-%08X",  dcuLock[0],  dcuLock[1],  dcuLock[2],  dcuLock[3]);

    // check that the current DCU Locks match the expected
    for (x = 0; x < 4; x++)
    {
        if (dcuLock[x] != exp_DCU_Lock_words.dcu_word[x])
        {
            return AM_STATUS_FAILURE;
        }
    }
    return AM_STATUS_SUCCESS;
}

// ****************************************************************************
//
// Check DCU Enables
//
// ****************************************************************************
uint32_t dcu_enables_test(security_t *mode_lcs, char *info_buf)
{
    int x;
    uint32_t dcuEnables[4];

    dcu_bits_t exp_DCU_Enable_words = expected_dcu_enable_bits[mode_lcs->mode][mode_lcs->lcs];

    // Read the DCU lock status
    dcuEnables[0] = CRYPTO->HOSTDCUEN0;
    dcuEnables[1] = CRYPTO->HOSTDCUEN1;
    dcuEnables[2] = CRYPTO->HOSTDCUEN2;
    dcuEnables[3] = CRYPTO->HOSTDCUEN3;

    // create return info string
    am_util_stdio_sprintf(info_buf, "DCU Enables: %08X-%08X-%08X-%08X",  dcuEnables[0],  dcuEnables[1],  dcuEnables[2],  dcuEnables[3]);

    // check that the current DCU Locks match the expected
    for (x = 0; x < 4; x++)
    {
        if (dcuEnables[x] != exp_DCU_Enable_words.dcu_word[x])
        {
            return AM_STATUS_FAILURE;
        }
    }
    return AM_STATUS_SUCCESS;
}

// ****************************************************************************
//
// Check the VTOR value
//
// ****************************************************************************

uint32_t vtor_test(security_t *sec_mode, char *info_buf)
{
   // get the expected SBR lock status
    uint32_t currVTOR = vtor_results_data[sec_mode->mode][sec_mode->lcs];

    am_util_stdio_sprintf(info_buf, "App loaded at 0x%08X", SCB->VTOR);

    // Read the SBR lock status
    if (SCB->VTOR == currVTOR)
       return AM_STATUS_SUCCESS;

    return AM_STATUS_FAILURE;
}

// ****************************************************************************
//
// Check BootROM Copy Protection
//     if SBRlock is asserted, then all protected locations in the bootrom must read 0
//     if SBRlock is not asserted, then all locations must not read 0
//
// ****************************************************************************

#define BOOTROM_ADDRESS     0x02000000
#define BOOTROM_PPROT_SIZE  (0x8000/4)    // size in words


uint32_t bootrom_copy_test(security_t *sec_mode, char *info_buf)
{
    uint32_t *paddrSrc = (uint32_t *)BOOTROM_ADDRESS;
    uint32_t read_result = 0;
    uint32_t lockStatus;
    uint32_t rtn_status = AM_STATUS_SUCCESS;

    // Test that all protected bootrom locations and see if they all read 0
    for (uint32_t i = 0; i < BOOTROM_PPROT_SIZE; i++)
    {
        read_result |= *paddrSrc++;
    }

    lockStatus = MCUCTRL->BOOTLOADER_b.SBRLOCK;
    if (lockStatus == 0 && read_result != 0)            // when locked (0) all locations should read 0
        rtn_status = AM_STATUS_FAILURE;
    else if (lockStatus == 1 && read_result == 0)       // when unlocked, all location can not possibly be 0
        rtn_status = AM_STATUS_FAILURE;

    // any other condition is a pass
    if (rtn_status == AM_STATUS_SUCCESS)

    am_util_string_strcpy(info_buf, (read_result == 0) ? "All locations read '0'" : "Bootrom is readable");
    return rtn_status;
}

// ****************************************************************************
//
// Check Helper Function Region (32K-64K) -
//     Check its execute only (no copy - can't read)  all reads return '0'
//
// ****************************************************************************
#define BOOTROM_HELPER_ADDRESS     0x02008000
#define BOOTROM_HELPER_SIZE        (0x8000/4)    // size in words

uint32_t helper_copy_test(security_t *sec_mode, char *info_buf)
{
    uint32_t *paddrSrc = (uint32_t *)BOOTROM_ADDRESS;
    uint32_t read_result = 0;
    uint32_t rtn_status = AM_STATUS_SUCCESS;

    // Test that all protected bootrom locations and see if they all read 0
    for (uint32_t i = 0; i < BOOTROM_PPROT_SIZE; i++)
    {
        read_result |= *paddrSrc++;
    }

    // The whole region should read 0 if SBRlock is asserted ('0')
    if (read_result != 0x00000000 && MCUCTRL->BOOTLOADER_b.SBRLOCK == 0)
            rtn_status = AM_STATUS_FAILURE;

    am_util_string_strcpy(info_buf, (read_result == 0) ? "All locations read '0'" : "Helper area is readable");
    return rtn_status;
}

// ****************************************************************************
//
// Check Helper Region Execution Test
//
// ****************************************************************************
uint32_t helper_execute_test(security_t *sec_mode, char *info_buf)
{
    uint32_t rtn_status = AM_STATUS_FAILURE;

    // get Bootrom Version - it should return 0x21000009 in Apollo5a
    // lower 16-bits is version number, upper 16-bits is release date (days since 1/1/2000)
    //uint32_t br_version = g_nv_helpers.br_util_rom_version_INTERNAL_USE_ONLY();
    uint32_t br_version = ((uint32_t (*)(void))(0x0200ffe0+1))();

    if (br_version == BOOTROM_VERSION_RTN)
        rtn_status = AM_STATUS_SUCCESS;

    am_util_stdio_sprintf(info_buf, "Bootrom Version: 0x%08X", br_version);
    return rtn_status;
}

// ****************************************************************************
//
// Checks for MCUCTRL FLASH Protection register
//
// ****************************************************************************

#define NVM_WRITE_BUFFER_ADDR_OFFSET    0x100
#define NVM_SIZE                        4*1024*1024
#define NVM_BLOCK_SIZE_PER_BIT          16*1024
#define NVM_PROG_CUSTOMER_KEY           0x12344321
#define NVM_BASE_ADDR                   0x00400000

#if 0 // To do Read/Write tests on protected flash segments will generate s precise hardfault -- need to add a handler in the future to enable these
//
// MCUCTRL Write Protection Test
//
static uint32_t mcu_write_prot_test( uint32_t* pwProtReg, char *info_buf )
{
    volatile uint32_t* pui32flashWProt = &(MCUCTRL->FLASHWPROT0);

    uint32_t buf[100];
    uint32_t temp = 0;

    while(temp < 100)
    {
        buf[temp++] = 1;
    }

    for (uint32_t regNum = 0; regNum < 8; regNum++)
    {
        if (regNum == 4)
        {
            // Assign the iterator to the upper 2MB's write protection register's
            // address
            pui32flashWProt = &(MCUCTRL->FLASHWPROT4);
        }

        // Write to the memory corresponding to the bit set in the write protection register
        // using the helper function and check the return value
        for (uint32_t bitNum = 0; bitNum < 32 ; bitNum++)
        {
            // Set a bit in the write protection register corresponding to each block of 16K NV memory
            *pui32flashWProt = (1 << (bitNum));

            // Write data using the helper function in the corresponding memory at an offset of 100 words
            int32_t retVal = g_nv_helpers.nv_program_main(NVM_PROG_CUSTOMER_KEY, buf, (uint32_t*)(NVM_BASE_ADDR + (bitNum*NVM_BLOCK_SIZE_PER_BIT)+NVM_WRITE_BUFFER_ADDR_OFFSET), 100);

            if (retVal == 0)
            {
                am_util_stdio_sprintf(info_buf, "The Flash Write Protection test fails ");
                return AM_STATUS_FAILURE;
            }
        }
        // Increment the flash protection register pointer to point to the next register
        pui32flashWProt++;
    }

    //
    // Restore the default values of the Write Protection Register
    //
    pui32flashWProt = &(MCUCTRL->FLASHWPROT0);
    for(uint32_t i = 0; i < 8; i++ )
    {
        if (i == 4)
        {
            // Assign the iterator to the upper 2MB's write protection register's
            // address
            pui32flashWProt = &(MCUCTRL->FLASHRPROT4);
        }

        // Restore the default value
        *pui32flashWProt++ = *pwProtReg++;
    }


    return AM_STATUS_SUCCESS;
}

//
// MCUCTRL Read Protection Test
//
static uint32_t mcu_read_prot_test( uint32_t* prProtReg, char *info_buf )
{
    //
    // Check the Read Protection Regiters working
    //

    volatile uint32_t* pui32flashRProt = &(MCUCTRL->FLASHRPROT0);
    uint32_t ui32readResult = 0;

    for (uint32_t regNum = 0; regNum < 8; regNum++)
    {
        if (regNum == 4)
        {
            // Assign the iterator to the upper 2MB's write protection register's
            // address
            pui32flashRProt = &(MCUCTRL->FLASHRPROT4);
        }

        // Write to the memory corresponding to the bit set in the write protection register
        // using the helper function and check the return value
        for (uint32_t bitNum = 0; bitNum < 32 ; bitNum++)
        {
            // Set a bit in the read protection register corresponding to each block of 16K NV memory
            //*pui32flashRProt = 1 << (bitNum);

            uint32_t* pui32srcAddr = (uint32_t*)(NVM_BASE_ADDR + NVM_BLOCK_SIZE_PER_BIT * bitNum);

            // Read the corresponding 16K NV memory
            for (uint32_t i = 0; i < ( NVM_BLOCK_SIZE_PER_BIT / 4) ; i++)
            {
                ui32readResult |= *pui32srcAddr++;
            }

            // Check if we have any reigter with data
            if (ui32readResult != 0x00000000)
            {
                am_util_stdio_sprintf(info_buf, "The Flash Read Protection test fails ");
                return AM_STATUS_FAILURE;
            }

        }
        // Restore the flash protection register to default

        // Increment the flash protection register pointer to point to the next register
        pui32flashRProt++;
    }

    //
    // Restore the default values of the Read Protection Register
    //
    pui32flashRProt = &(MCUCTRL->FLASHRPROT0);
    for(uint32_t i = 0; i < 8; i++ )
    {
        if (i == 4)
        {
            // Assign the iterator to the upper 2MB's write protection register's
            // address
            pui32flashRProt = &(MCUCTRL->FLASHRPROT4);
        }

        // Restore the default value
        *pui32flashRProt++ = *prProtReg++;
    }

    return AM_STATUS_SUCCESS;
}

#endif

static uint32_t mcuctrl_protection_test(security_t *sec_mode, char *info_buf)
{
    
    am_status_e e_retStatus = AM_STATUS_SUCCESS; 
    
    // Store the current MCUCTRL Flash Write Proctection Register values in a temporary array
    uint32_t ui32wProtReg[8];
    
    mcu_fprot_bits_t srefWProtVal = mcu_wprot_ref_val[sec_mode->mode][sec_mode->lcs];
    
    ui32wProtReg[0] = MCUCTRL->FLASHWPROT0;
    ui32wProtReg[1] = MCUCTRL->FLASHWPROT1;
    ui32wProtReg[2] = MCUCTRL->FLASHWPROT2;
    ui32wProtReg[3] = MCUCTRL->FLASHWPROT3;
    ui32wProtReg[4] = MCUCTRL->FLASHWPROT4;
    ui32wProtReg[5] = MCUCTRL->FLASHWPROT5;
    ui32wProtReg[6] = MCUCTRL->FLASHWPROT6;
    ui32wProtReg[7] = MCUCTRL->FLASHWPROT7;
    
    for( uint32_t i = 0; i < 8; i++)
    {
        if (ui32wProtReg[i] != srefWProtVal.fprot_word[i] )
        {
            e_retStatus = AM_STATUS_FAILURE;
        }
    }
    
    // Store the current MCUCTRL Flash Read Proctection Register values in a temporary array
    uint32_t ui32rProtReg[8];
    
    mcu_fprot_bits_t srefRProtVal = mcu_rprot_ref_val[sec_mode->mode][sec_mode->lcs];
    
    ui32rProtReg[0] = MCUCTRL->FLASHRPROT0;
    ui32rProtReg[1] = MCUCTRL->FLASHRPROT1;
    ui32rProtReg[2] = MCUCTRL->FLASHRPROT2;
    ui32rProtReg[3] = MCUCTRL->FLASHRPROT3;
    ui32rProtReg[4] = MCUCTRL->FLASHRPROT4;
    ui32rProtReg[5] = MCUCTRL->FLASHRPROT5;
    ui32rProtReg[6] = MCUCTRL->FLASHRPROT6;
    ui32rProtReg[7] = MCUCTRL->FLASHRPROT7;
    
    for( uint32_t i = 0; i < 8; i++)
    {
        if (ui32rProtReg[i] != srefRProtVal.fprot_word[i] )
        {
            e_retStatus = AM_STATUS_FAILURE;
        }
    }

    // create return info string
    am_util_stdio_sprintf(info_buf, "MCUCTRL Flash Read Proctection Register: \n\t(0-2MB)%08X-%08X-%08X-%08X-\n\t(2-4MB)%08X-%08X-%08X-%08X)\
                                    \n\n\tMCUCTRL Flash Write Proctection Register: \n\t(0-2MB)%08X-%08X-%08X-%08X-\n\t(2-4MB)%08X-%08X-%08X-%08X", 
                                    ui32rProtReg[0], ui32rProtReg[1], ui32rProtReg[2], ui32rProtReg[3],
                                    ui32rProtReg[4], ui32rProtReg[5], ui32rProtReg[6], ui32rProtReg[7], 
                                    ui32wProtReg[0], ui32wProtReg[1], ui32wProtReg[2], ui32wProtReg[3],
                                    ui32wProtReg[4], ui32wProtReg[5], ui32wProtReg[6], ui32wProtReg[7]);    
#if 0
    // Test each bit of the Write Protection register.
    if( mcu_write_prot_test( ui32wProtReg, info_buf ) )
    {
        e_retStatus = AM_STATUS_FAILURE;;
    }

    // Test each bit of the Read Protection register.
    if( mcu_read_prot_test( ui32rProtReg, info_buf ) )
    {
        e_retStatus = AM_STATUS_FAILURE;;
    }
#endif

    return e_retStatus;
}

uint32_t run_mcuctrl_protection_test(security_t *sec_mode, char *info_buf)
{
    return mcuctrl_protection_test(sec_mode, info_buf);
}


// ****************************************************************************
//
// Checks for PUBLOCK
//
// ****************************************************************************

static uint32_t publock_test( security_t* sec_mode, char* info_buf )
{
    // get the expected Publock status
    uint32_t lockStatus = publock_results_data[sec_mode->mode][sec_mode->lcs];

    // Read the Publock
    uint32_t publock = (*((uint32_t*)AM_REG_OTP_INFOC_SECURITY_ADDR) & 0x000000E0) >> 5;

    // check for match
    if ( publock == lockStatus)
    {
       am_util_string_strcpy(info_buf, (publock == 0x5) ? "PUBLOCK not asserted" : "PUBLOCK asserted");
       return AM_STATUS_SUCCESS;
    }

    // Return - SBRLock not as expected
    am_util_string_strcpy(info_buf, (publock == 0x0) ? "PUBLOCK asserted" : "PUBLOCK not asserted");
    
    
    // Check if CRYPTO is inaccessible
    if (publock == 0x00)    // publock is asserted
    {
        // if publock is set then the CRYPTO registers will be inaccessible
        if ((CRYPTO->AOAPBFILTERING_b.ONLYPRIVACCESSALLOWLOCK && CRYPTO->AOAPBFILTERING_b.ONLYSECACCESSALLOWLOCK) != 0x00 )
        {
            return AM_STATUS_FAILURE;
        }
    }
    
    return AM_STATUS_SUCCESS;
}

uint32_t run_publock_test(security_t *sec_mode, char *info_buf)
{
    return publock_test(sec_mode, info_buf);
}
// ****************************************************************************
//
// Checks for CRYPTO Host AO Lcok Bits
//
// ****************************************************************************

static uint32_t crypto_hostao_lock_test( security_t* sec_lcs, char* info_buf )
{
    uint32_t ui32retVal = (uint32_t)AM_STATUS_SUCCESS;
    
    // get the expected register value 
    uint32_t ui32refHostAoLockVal = hostaolock_reg_data[sec_lcs->mode][sec_lcs->lcs];
    
    // get the actual register value 
    uint32_t ui32hostaolockReg = CRYPTO->HOSTAOLOCKBITS;
    
    for(uint32_t i = 0; i < 9; i++)
    {
        if ( ui32hostaolockReg != ui32refHostAoLockVal)
        {
            ui32retVal = AM_STATUS_FAILURE;
        }
    }
    
    am_util_stdio_sprintf(info_buf, "The CRYPTO->HOSTAOLOCKBITS register value = %08X", CRYPTO->HOSTAOLOCKBITS);
    
    return ui32retVal;
}    

uint32_t run_crypto_hostao_lock_test(security_t *sec_mode, char *info_buf)
{
    return crypto_hostao_lock_test(sec_mode, info_buf);
}

// ****************************************************************************
//
// Check the otp_oem_flag_test
//
// ****************************************************************************

static uint32_t otp_oem_flag_test( security_t* sec_lcs, char* info_buf )
{
    uint32_t ui32retVal = (uint32_t)AM_STATUS_SUCCESS;
    
    // get the OEM flag expected value
    uint32_t ui32refOemFlags = expected_oem_flags[sec_lcs->mode][sec_lcs->lcs];
    
    // Get the value of the OTP OEMPROGFLAGS register value
    uint32_t* poemFlagsReg = (uint32_t*) AM_REG_OTP_INFOC_OEMPROGFLAGS_ADDR;
    
    //create the return string
    am_util_stdio_sprintf(info_buf, "OEM Flag Register = %08X ", *poemFlagsReg);
    
    // compare the expected value to the read value 
    // TODOAP: Use the OEM Flag register(BIT 0-23 -> 0 if we did not move to Secure LCS) 
    //         to decide whether we came from DM or Secure LCS
    if(sec_lcs->lcs == RMA_LCS)
    {
        if( (*poemFlagsReg & 0xFFFFFF00) != ui32refOemFlags )
        {
            ui32retVal = (uint32_t) AM_STATUS_FAILURE;
        }
        
        // out put the previous state if we are in RMA LCS
        if ( *poemFlagsReg & 0x0FFFFFFF )
        {
            am_util_string_strcat(info_buf, "\n  Transitioned from SECURE LCS");
        }
        else
        {
            am_util_string_strcat(info_buf, "\n  Transitioned from DM LCS");
        }
    }
    else
    {
        if( (*poemFlagsReg ) != ui32refOemFlags )
        {
            ui32retVal = (uint32_t) AM_STATUS_FAILURE;
        }
    }
    
    return ui32retVal;
}

uint32_t run_otp_oem_flag_test(security_t *sec_mode, char *info_buf)
{
    return otp_oem_flag_test(sec_mode, info_buf);
}
// ****************************************************************************
//
// Check the otp_nvcounters_test -- Currently does nothing but outputs the current counters 
//
// ****************************************************************************
#define AM_REG_OTP_INFOC_HBK0MINVER0_ADDR 0x400c2088
static uint32_t otp_nvcounters_test( security_t* sec_lcs, char* info_buf )
{
    uint32_t ui32retVal = AM_STATUS_SUCCESS;
    
    // get the base address of the NV Counter register
    uint32_t* pnvCounterBaseAddr = (uint32_t*)AM_REG_OTP_INFOC_HBK0MINVER0_ADDR; 
    
    //create the return string 
    // TODO:
    am_util_stdio_sprintf(info_buf, "HBK0MINVER0 = %08X \n\t HBK0MINVER1 = %08X \n\t HBK1MINVER0 = %08X \n\t HBK1MINVER1 = %08X \n\t HBK1MINVER2 = %08X", \
                                    *(pnvCounterBaseAddr), *(pnvCounterBaseAddr + 1 ), *(pnvCounterBaseAddr + 2), \
                                    *(pnvCounterBaseAddr + 3 ), *(pnvCounterBaseAddr + 4));
    
    return ui32retVal;
}

uint32_t run_otp_nvcounters_test(security_t *sec_mode, char *info_buf)
{
    return otp_nvcounters_test(sec_mode, info_buf);
}
// ****************************************************************************
//
// Check the OTA Pointer
//
// ****************************************************************************

static uint32_t ota_test( security_t* sec_lcs, char* info_buf )
{
    uint32_t ui32retVal = AM_STATUS_SUCCESS;
    
    // get the expected value of the OTA pointer
    uint32_t ui32otaPointer = MCUCTRL->OTAPOINTER;
    
    
    if ((ui32otaPointer & 0x1) && (ui32otaPointer & 0xFFFFFFFC)>>2)
    {
        // create the return string
        am_util_stdio_sprintf(info_buf, "The OTA pointer is valid \n\t The OTA Pointer Register = %08X" , ui32otaPointer);
    }
    else if ( !(ui32otaPointer & 0x1) )
    {
        // create the return string
        am_util_stdio_sprintf(info_buf, "The OTA pointer not is valid \n\t The OTA Pointer Register = %08X" , ui32otaPointer);
    }
    else
    {
        // create the return string
        am_util_stdio_sprintf(info_buf, "The OTA pointer is bad");
        ui32retVal = AM_STATUS_FAILURE;
    }
    
    return ui32retVal;
}

uint32_t run_ota_test(security_t *sec_mode, char *info_buf)
{
    return ota_test(sec_mode, info_buf);
}
