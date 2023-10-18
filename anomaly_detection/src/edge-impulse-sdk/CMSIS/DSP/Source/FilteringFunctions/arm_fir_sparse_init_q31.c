/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_fir_sparse_init_q31.c
 * Description:  Q31 sparse FIR filter initialization function
 *
 * $Date:        18. March 2019
 * $Revision:    V1.6.0
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2019 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "edge-impulse-sdk/CMSIS/DSP/Include/dsp/filtering_functions.h"

/**
  @ingroup groupFilters
 */

/**
  @addtogroup FIR_Sparse
  @{
 */

/**
  @brief         Initialization function for the Q31 sparse FIR filter.
  @param[in,out] S          points to an instance of the Q31 sparse FIR structure
  @param[in]     numTaps    number of nonzero coefficients in the filter
  @param[in]     pCoeffs    points to the array of filter coefficients
  @param[in]     pState     points to the state buffer
  @param[in]     pTapDelay  points to the array of offset times
  @param[in]     maxDelay   maximum offset time supported
  @param[in]     blockSize  number of samples that will be processed per block
  @return        none

  @par           Details
                   <code>pCoeffs</code> holds the filter coefficients and has length <code>numTaps</code>.
                   <code>pState</code> holds the filter's state variables and must be of length
                   <code>maxDelay + blockSize</code>, where <code>maxDelay</code>
                   is the maximum number of delay line values.
                   <code>blockSize</code> is the number of words processed by <code>arm_fir_sparse_q31()</code> function.
 */

void arm_fir_sparse_init_q31(
        arm_fir_sparse_instance_q31 * S,
        uint16_t numTaps,
  const q31_t * pCoeffs,
        q31_t * pState,
        int32_t * pTapDelay,
        uint16_t maxDelay,
        uint32_t blockSize)
{
  /* Assign filter taps */
  S->numTaps = numTaps;

  /* Assign coefficient pointer */
  S->pCoeffs = pCoeffs;

  /* Assign TapDelay pointer */
  S->pTapDelay = pTapDelay;

  /* Assign MaxDelay */
  S->maxDelay = maxDelay;

  /* reset the stateIndex to 0 */
  S->stateIndex = 0U;

  /* Clear state buffer and size is always maxDelay + blockSize */
  memset(pState, 0, (maxDelay + blockSize) * sizeof(q31_t));

  /* Assign state pointer */
  S->pState = pState;
}

/**
  @} end of FIR_Sparse group
 */
