/**
 ****************************************************************************************
 *
 * @file cae.h
 *
 * @brief Header file of Core Audio Engine(CAE).
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _CAE_H_
#define _CAE_H_

////////////////////////////////////////////////////////////////////////////////////////
//
//                              Speech Algorithm
//
// Standalone modules of speech algorithms for acoustic tuning.
// For production software, please use the speech integration module ssp.h for
// less MIPS
#include "agc.h"
#include "drc.h"
#include "fbaec.h"
#include "ns.h"
//
// Speech algorithms integration module, including fbaec, ns, drc and agc
// modules
#include "ssp.h"
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
//                             Playback Algorithm
//
// DRC for downlink(Playback to speaker). Multiple subbank DRC.
#include "mbdrc.h"
//
// Parametric Equalizer(PEQ)
#include "peq.h"
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
//                             Gain Algorithm
//
//
// Fade algorithm for gain changing
#include "fade.h"
// Gain changing without fading
#include "gain.h"
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
//                       Sample rate convert(SRC) Algorithm
//
// Fast resample for speech
#include "resample.h"
// High quality resample for music
#include "speex_resampler.h"
//
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
//
//                              Other Algorithms
//
// Generate sin wave
#include "wavegen.h"
//
// Calculate Total Harmonic Distortion(THD) of audio data for ADC factory test
#include "thd.h"
//
////////////////////////////////////////////////////////////////////////////////////////

#endif // _CAE_H_
