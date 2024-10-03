//*****************************************************************************
//!
//! @file watch_nnse_hfp.c
//!
//! @brief AEC pipeline for watch HFP with NNSE.
//!
//! Hardware Setup:
//! - ESP32(ESP32_LyraT v4.3) as BT module connects to I2S-1
//! - PCM5102A as DAC connects to I2S-0
//! - TDK PDM microphone board connects to PDM
//!
//! GPIO Inforamtion
//!  | PIN Config | GPIO     | Description
//!  | MCLK       | GPIO-15  | Clock source. Connect to ESP32.Frequency is 4.096MHz.
//!  | I2S0-BCLK  | GPIO-5   | Speaker. Connect to PCM5102A.
//!  | I2S0-LRCLK | GPIO-7   | Speaker. Connect to PCM5102A.
//!  | I2S0-DOUT  | GPIO-6   | Speaker. Connect to PCM5102A.
//!  | I2S1-BCLK  | GPIO-16  | Audio source. Connect to ESP32.
//!  | I2S1-LRCLK | GPIO-18  | Audio source. Connect to ESP32.
//!  | I2S1-DOUT  | GPIO-17  | Audio source. Connect to ESP32.
//!  | I2S1-DIN   | GPIO-19  | Audio source. Connect to ESP32.
//!  | PDM-CLK    | GPIO-50  | Microphone. Connect to TDK.
//!  | PDM-DATA   | GPIO-51  | Microphone. Connect to TDK.
//!
//! Pipeline Configuration:
//! - Sample rate: 16kHz
//! - Channels: 1
//! - Frame length: 8ms
//! - Clock source: mclk
//!
//! Pipeline Graph:
//! i2s1<-agc<-peq<-drc<-ns<-reframe<-nnse<-reframe<-aec<-pdm0
//!                                                   |
//!                                           |--->---|
//!                                           |
//! i2s1->gain->mbdrc->peq->drc->iir->mixer->split->i2s0
//!                                     |
//!                       promptsrc-->--|
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>

#include "am_util_stdio.h"
#include "am_util_debug.h"

#include "aas_common.h"

#include "aust_nnse_element.h"
#include "ai_callback.h"

nnse_ops_t nnse_ops = {
    .init = nnse_init,
    .process = nnse_process,
    .deinit = nnse_deinit,
};


/**
 * @brief Pipeline configuration
 *
 * @param pipeline Pipeline to config
 * @return true for success, false for fail
 */
bool pipeline_config(AustPipeline *pipeline)
{
    // Add element into pipeline
    pipeline->addElement(adevsrc_pdm);
    pipeline->addElement(aec);
    pipeline->addElement(reframe_8_10);
    pipeline->addElement(nnse);
    pipeline->addElement(reframe_10_8);
    pipeline->addElement(ns);
    pipeline->addElement(drc_ul);
    pipeline->addElement(peq_ul);
    pipeline->addElement(agc);
    pipeline->addElement(adevsink_i2s1);

    pipeline->addElement(adevsrc_i2s1);
    pipeline->addElement(gain);
    pipeline->addElement(mbdrc);
    pipeline->addElement(peq_dl);
    pipeline->addElement(drc_dl);
    pipeline->addElement(iir_filter);
    pipeline->addElement(mixer);
    pipeline->addElement(splitter);
    pipeline->addElement(adevsink_i2s0);

    pipeline->addElement(promptsrc);

    // Add Sniffer Element into pipeline. Don't link sniffer.
    pipeline->addElement(sniffer);

    // Link Uplink elements
    adevsrc_pdm->linkElement(aec);
    aec->linkElement(reframe_8_10);
    reframe_8_10->linkElement(nnse);
    nnse->linkElement(reframe_10_8);
    reframe_10_8->linkElement(ns);
    ns->linkElement(drc_ul);
    drc_ul->linkElement(peq_ul);
    peq_ul->linkElement(agc);
    agc->linkElement(adevsink_i2s1);

    // Link Downlink elements


    adevsrc_i2s1->linkElement(gain);
    gain->linkElement(mbdrc);
    mbdrc->linkElement(peq_dl);
    peq_dl->linkElement(drc_dl);
    drc_dl->linkElement(iir_filter);
    iir_filter->linkElement(mixer);
    mixer->linkElement(splitter);
    splitter->linkElement(aec); // Link to AEC as reference sigle must after Mic link to AEC
    splitter->linkElement(adevsink_i2s0);

    // Prompt src should be the second that link to mixer.
    promptsrc->linkElement(mixer);

    // Set properties
    pipeline->setProperty("sample-rate", "16000");
    pipeline->setProperty("channels", "1");
    // AEC requires 8ms frame size, 256 Bytes is 8ms
    pipeline->setProperty("pcm-frame-size", "256");
    pipeline->setProperty("clksrc", "mclk");

    promptsrc->setProperty("gain", "0.1");
    promptsrc->setProperty("sine-wave-frequency", "400");

    mixer->setProperty("fade-in-frames", "50");
    mixer->setProperty("fade-out-frames", "10");
    mixer->setProperty("pad-0-target-gain", "0.3");
    mixer->setProperty("pad-1-target-gain", "0.9");

    adevsrc_pdm->setProperty("endpoint", "pdm-0");

    peq_ul->setProperty("link", "uplink");
    peq_dl->setProperty("link", "downlink");

    drc_ul->setProperty("link", "uplink");
    drc_dl->setProperty("link", "downlink");

    reframe_8_10->setProperty("submit-empty", "1");
    reframe_8_10->setProperty("out-frame-size", "320");
    reframe_8_10->setProperty("out-frame-number", "4");
    reframe_10_8->setProperty("out-frame-size", "256");
    reframe_10_8->setProperty("out-frame-number", "5");

    adevsrc_i2s1->setProperty("endpoint", "i2s-1");
    adevsrc_i2s1->setProperty("role", "slave");

    adevsink_i2s1->setProperty("endpoint", "i2s-1");
    adevsink_i2s1->setProperty("role", "slave");

    adevsink_i2s0->setProperty("endpoint", "i2s-0");
    adevsink_i2s0->setProperty("role", "master");

    ((NnseElement *)nnse)->setOps(&nnse_ops);

    return true;
}
