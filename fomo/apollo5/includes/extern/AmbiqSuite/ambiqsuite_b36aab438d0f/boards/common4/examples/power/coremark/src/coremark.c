//*****************************************************************************
//
//! @file coremark.c
//!
//! @brief EEMBC COREMARK test.
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup ble_freertos_fit_lp BLE FreeRTOS Fit Lowpower Example
//! @ingroup power_examples
//! @{
//!
//! Purpose: This example runs the official EEMBC COREMARK test.
//!
//! EEMBC’s CoreMark® is a benchmark that measures the performance of
//! microcontrollers (MCUs) and central processing units (CPUs) used in
//! embedded systems. Replacing the antiquated Dhrystone benchmark, Coremark
//! contains implementations of the following algorithms: list processing
//! (find and sort), matrix manipulation (common matrix operations), state
//! machine (determine if an input stream contains valid numbers), and CRC
//! (cyclic redundancy check). It is designed to run on devices from 8-bit
//! microcontrollers to 64-bit microprocessors.
//!
//! The CRC algorithm serves a dual function; it provides a workload commonly
//! seen in embedded applications and ensures correct operation of the CoreMark
//! benchmark, essentially providing a self-checking mechanism. Specifically,
//! to verify correct operation, a 16-bit CRC is performed on the data
//! contained in elements of the linked-list.
//!
//! To ensure compilers cannot pre-compute the results at compile time, every
//! operation in the benchmark derives a value that is not available at compile
//! time. Furthermore, all code used within the timed portion of the benchmark
//! is part of the benchmark itself (no library calls).
//!
//! More info may be found at the [EEMBC CoreMark website](https://www.eembc.org/coremark/).
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//!
//! AM_DEBUG_PRINTF
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//!
//! Note that when these macros are defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

