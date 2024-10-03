//*****************************************************************************
//
//! @file patterns.c
//!
//! @brief Memory pattern utilities
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "patterns.h"
#include "unity.h"

//*****************************************************************************
//
//  Fill a buffer with a predefined pattern
//
// *buffer - pattern destination buffer
// len - buffer length in bytes
// *pattern - pattern source buffer of uint32_t dwords
//
// NOTE: changing the fill pattern will affect the expected display controller
// CRC. If this happens (and you trust your HW!) update the CRC in the header
// from the DC_CRC in the SWO logs.
//
//*****************************************************************************
void
fill_pattern(uint8_t* buffer, uint32_t len, const uint8_t* pattern)
{
    memcpy(buffer, pattern, len);
    flush_cache_range((void*)buffer, len);
}

//*****************************************************************************
//
//  Write generative memory pattern from the CPU to a target address
//
// Writes a pattern to memory from the CPU without fetching from another
// memory. This pattern should be identical to PATTERN_MEM in patterns.h
// and any changes to this function should be followed by an update of
// PATTERN_MEM
//
// *buffer: pointer to where the pattern will be written
// len: length of the pattern in bytes
//
//*****************************************************************************
void
write_cm55_mem_pattern(uint8_t *buffer, uint32_t len)
{
    uint32_t pattern_base_word = 0xaa550FF0;
    uint32_t pattern_offset = 0;
    uint32_t *target_index = (uint32_t*)buffer;
    while(pattern_offset < (len / sizeof(uint32_t)))
    {
        *target_index = pattern_base_word ^ pattern_offset;
        pattern_base_word ^= 0xFFFFFFFF; // invert pattern every other write
        pattern_offset++;
        target_index++;
    }
}

//*****************************************************************************
//
//  Verify a filled buffer
//
// *buffer - buffer to verify
// len - buffer length in bytes
// *pattern - pattern source buffer of uint32_t dwords
// pattern_length - length of pattern in uint32_t size dwords
// pattern_mismatch_msg - string to log when the pattern does not match
//
//*****************************************************************************
void
verify_pattern(uint8_t* buffer, uint32_t len, const uint8_t* pattern, char* pattern_mismatch_msg)
{
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(pattern, buffer, len, pattern_mismatch_msg);
}

//*****************************************************************************
//
//  Compute a 32-bit checksum over a buffer
//
// Checksum is computed 32 bits at a time
//
// *buffer - buffer to generate the checksum over
// len - buffer length in bytes
//
// returns: 32-bit checksum
//
//*****************************************************************************
uint32_t
compute_checksum(const void *buffer, uint32_t len)
{
    uint32_t *index = (void*) buffer;
    uint32_t checksum = 0;
    while( (void*)index < (buffer + len))
    {
        checksum = (checksum + *index) & 0xFFFFFFFF;
        index++;
    }
    return checksum;
}