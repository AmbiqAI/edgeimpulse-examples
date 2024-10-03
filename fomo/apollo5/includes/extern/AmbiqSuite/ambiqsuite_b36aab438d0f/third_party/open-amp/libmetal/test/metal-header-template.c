/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
 * Template file used to ensure that each <metal/...> header can be included
 * without independently without prior inclusion of dependencies.
 */
#include <@INCLUDE@>

// Suppress warnings about empty translation unit:
#if defined(__clang__)
#pragma clang diagnostic ignored "-Wempty-translation-unit"
#endif
