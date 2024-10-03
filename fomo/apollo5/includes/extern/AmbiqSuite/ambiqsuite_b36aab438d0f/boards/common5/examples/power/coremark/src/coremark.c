//*****************************************************************************
//
//! @file coremark.c
//!
//! @brief EEMBC COREMARK test.
//!
//! Purpose: This example runs the official EEMBC COREMARK test.
//!
//! The Coremark run begins by first outputing a banner (to the UART)
//! indicating that it has started.  It then does a complete disable
//! and power down of the UART for accurate power measuring during the run.
//!
//! The Coremark implementation performs 2250 ITERATIONS (specified in
//! ambiq_core_config.h), which is plenty of time for correct operation
//! of the benchmark.
//!
//! Once the run has completed, the UART is reenabled and the results printed.
//!
//! Text is output to the UART at 115,200 BAUD, 8 bit, no parity.
//! Please note that text end-of-line is a newline (LF) character only.
//! Therefore, the UART terminal must be set to simulate a CR/LF.
//!
//! For Arm6 compilation of M55, add the following options in the MDK project.
//!  MDK Compiler option in 'Misc Controls': -Omax
//!  MDK Linker   option in 'Misc Controls': -Omax
//!  Non-MDK Compiler option: -Omax
//!  Non-MDK Linker option  : -Omax --cpu=Cortex-M55 --lto
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

