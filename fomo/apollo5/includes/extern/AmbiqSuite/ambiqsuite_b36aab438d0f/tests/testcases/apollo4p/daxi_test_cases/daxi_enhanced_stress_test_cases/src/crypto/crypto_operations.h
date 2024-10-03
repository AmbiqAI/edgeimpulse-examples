//*****************************************************************************
//
//! @file crypto_operations.h
//!
//! @brief Cryptographic operations for the stress test.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef CRYPTO_OPERATIONS_H
#define CRYPTO_OPERATIONS_H

#ifdef __cplusplus
extern "C"
{
#endif


extern void crypto_power_up(void);
extern void crypto_power_down(void);
extern void crypto_test_init(void);
extern void crypto_test_operation(void);
extern void crypto_rsa_sign_verify(void);
extern void otp_read_loop(void);
extern void otp_write_loop(void);

#ifdef __cplusplus
}
#endif

#endif // CRYPTO_OPERATIONS_H
