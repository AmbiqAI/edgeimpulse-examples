//*****************************************************************************
//
//! @file common.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

/*
 * Copyright(c) 2019 Xilinx Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>

#include <openamp/remoteproc.h>
#include <openamp/remoteproc_loader.h>

#define AMBT53_FW       1
#define AMBT53_CODEC    2

struct rproc_priv
{
    struct remoteproc *rproc;
    int image_type;
};

#endif /* COMMON_H_ */
