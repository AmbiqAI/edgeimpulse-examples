//*****************************************************************************
//
//! @file am_util_crypto.h
//!
//! @brief Utility functions for the crypto module.
//!
//! A few utils to help with crypto functionality
//!
//! @addtogroup crypto Crypto
//! @ingroup utils
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

#ifndef AM_UTIL_CRYPTO_H
#define AM_UTIL_CRYPTO_H

#if !defined(AM_PART_APOLLO4_API) && !defined(AM_PART_APOLLO5_API)
#error "Error: am_util_crypto.h only supports Apollo3 and Apollo4."
#endif

//
// Memory Pools
//
#define AM_UTIL_CRYPTO_VARIABLE_POOL_SIZE      512
#define AM_UTIL_CRYPTO_FIXED_BUFFER_SIZE       20
#define AM_UTIL_CRYPTO_FIXED_BUFFER_COUNT      1024

//
// LibSWD configuration
//
#define AM_UTIL_CRYPTO_WRITE_BLOCK_SIZE        128
#define AM_UTIL_CRYPTO_READ_BLOCK_SIZE         32

//*****************************************************************************
//
//! @brief Malloc implemtation tailored to LibCrypto's needs.
//!
//! @param size - size of buffers to malloc
//!
//! @return pointer to space malloc'ed
//
//*****************************************************************************
extern void * am_util_crypto_malloc(size_t size);

//*****************************************************************************
//
//! @brief Calloc implementation tailored to LibCrypto's needs.
//!
//! @param num - number of buffers to calloc
//! @param size - size of buffers to calloc
//!
//! @return pointer to space calloc'ed
//
//*****************************************************************************
extern void * am_util_crypto_calloc(size_t num, size_t size);

//*****************************************************************************
//
//! @brief Minimal implementation of Crypto Free.
//
//! @note This implementation of free can only free memory from the fixed-size
//! buffer pool. It ignores all other requests.
//!
//! @param ptr - pointer to buffer to free
//
//*****************************************************************************
extern void am_util_crypto_free(void *ptr);

//*****************************************************************************
//
//! @brief Statistics function for malloc
//
//*****************************************************************************
extern void am_util_crypto_print_malloc_stats(void);

//*****************************************************************************
//
//! @brief Crypto Exit
//!
//! @param exitCode - Exit Code
//
//*****************************************************************************
extern void am_util_crypto_exit(uint32_t exitCode);

//*****************************************************************************
//
//! @brief Crypto Abort
//
//*****************************************************************************
extern void am_util_crypto_abort(void);

#endif // AM_UTIL_CRYPTO_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

