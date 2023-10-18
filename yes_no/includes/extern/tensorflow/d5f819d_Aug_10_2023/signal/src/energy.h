/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef SIGNAL_ENERGY_H_
#define SIGNAL_ENERGY_H_

#include <stdint.h>

#include "signal/src/complex.h"

namespace tflite {
namespace tflm_signal {
// TODO(b/286250473): remove namespace once de-duped libraries above

// Calculates the power spectrum from a DFT output between start and end indices
//
// * `start_index` and `end_index` must valid indices into `input`
// * `output` must be the same size as `input`. Only the values at indices
//   `start_index` and `end_index` inclusive should be considered valid.
void SpectrumToEnergy(const Complex<int16_t>* input, int start_index,
                      int end_index, uint32_t* output);

}  // namespace tflm_signal
}  // namespace tflite

#endif  // SIGNAL_ENERGY_H_
