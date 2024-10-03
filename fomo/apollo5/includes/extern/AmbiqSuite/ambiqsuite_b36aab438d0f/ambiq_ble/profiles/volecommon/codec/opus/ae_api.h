//*****************************************************************************
//
//  ae.api.h
//! @file
//!
//! @brief Functions for interfacing with the encoder library
//!
//! @addtogroup
//! @ingroup
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

#ifndef AE_API__H
#define AE_API__H


//*****************************************************************************
//
// External Function Calls
//
//*****************************************************************************

//*****************************************************************************
//
//! @brief Initialization of the library
//!
//! Run the initialization function before any operation with the encoder.
//! @param option - option to select whether to include 8-byte header into output.
//!                 0 for no header.
//!                 other value for with header.
//! @return numBytes - number of bytes of the data output, including header length.
//!
//! @{
//
//*****************************************************************************
int audio_enc_init(int option);

//*****************************************************************************
//
//! @brief Encodes a frame of audio
//!
//! @param p_pcm_buffer - pointer to the input buffer.
//! @param n_pcm_samples - input length in samples.
//! @param p_encoded_buffer - pointer to the output buffer.
//!
//! This is the major function of the encoder, the encoder encodes a fixed length
//! of audio (20ms) in a fixed input format (16KHz, 16bit, mono, 640 bytes in length),
//! with a fixed output format: CBR, 32000bps output rate (8:1 compression),
//! complexity 4.
//! When output header is selected, each output frame will have a 8 byte header in
//! the beginning of the output data: 4 bytes of length (fixed to 80) followed by
//! 4 bytes of range code.
//!
//! @return numBytes - number of bytes of the data output, including header length.
//!
//! @{
//
//*****************************************************************************
int audio_enc_encode_frame(short *p_pcm_buffer, int n_pcm_samples, unsigned char *p_encoded_buffer);

//*****************************************************************************
//
//! @brief Get version information of the libary
//!
//! @return version_string - pointer to the libary version string.
//!
//! @{
//
//*****************************************************************************
char* audio_get_lib_version(void);

//*****************************************************************************
//
//! @brief Get version information of the base package
//!
//! @return version_string - pointer to the base package version string.
//!                          version information is 7 byte long without line break
//! @{
//
//*****************************************************************************
char* audio_get_package_version(void);

#endif // AE_API__H


