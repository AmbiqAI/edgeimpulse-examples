//*****************************************************************************
//!
//! @file hearingaid_1mic_nnse.c
//!
//! @brief Hearing aid pipeline with single microphone.
//!
//! Hardware Setup:
//! - TDK PDM microphone board connect to GPIO50 and GPIO51.
//! - Amplifier connect to I2S0(GPIO5, GPIO6 and GPIO7)
//!
//! GPIO Inforamtion
//!  | PIN Config | GPIO     | Description
//!  | I2S0-BCLK  | GPIO-5   | Speaker. Connect to Amplifier.
//!  | I2S0-LRCLK | GPIO-7   | Speaker. Connect to Amplifier.
//!  | I2S0-DOUT  | GPIO-6   | Speaker. Connect to Amplifier.
//!  | PDM-CLK    | GPIO-50  | Microphone. Connect to TDK.
//!  | PDM-DATA   | GPIO-51  | Microphone. Connect to TDK.
//!
//! Pipeline Configuration:
//! - Sample rate: 16kHz
//! - Channels: 1
//! - Frame length: 8ms
//! - Clock source: hfrc
//!
//! Pipeline Graph:
//!
//! pdm0->reframe->nnse->reframe->mbdrc->gain->mixer->i2s0
//!                                              |
//!                                promptsrc-->--|
//!
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

#include "am_mcu_apollo.h"
#include "am_util_stdio.h"
#include "am_util_debug.h"

#include "aas_common.h"

#include "aust_nnse_element.h"
#include "ai_callback.h"

#define NNSE_AND_MBDRC // Remove this line for NNSE only testing

nnse_ops_t nnse_ops =
{
    .init = nnse_init,
    .process = nnse_process,
    .deinit = nnse_deinit,
};

#ifdef NNSE_AND_MBDRC
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
    pipeline->addElement(reframe_8_10);
    pipeline->addElement(nnse);
    pipeline->addElement(reframe_10_8);
    pipeline->addElement(mbdrc);
    pipeline->addElement(gain);
    pipeline->addElement(mixer);
    pipeline->addElement(adevsink_i2s0);

    pipeline->addElement(promptsrc);

    // Add Sniffer Element into pipeline. Don't link sniffer.
    pipeline->addElement(sniffer);

    // Link Uplink elements
    adevsrc_pdm->linkElement(reframe_8_10);
    reframe_8_10->linkElement(nnse);
    nnse->linkElement(reframe_10_8);
    reframe_10_8->linkElement(mbdrc);
    mbdrc->linkElement(gain);
    gain->linkElement(mixer);
    mixer->linkElement(adevsink_i2s0);

    // Prompt src should be the second that link to mixer.
    promptsrc->linkElement(mixer);

    // Set properties
    pipeline->setProperty("sample-rate", "16000");
    pipeline->setProperty("channels", "1");
    // MBDRC requires 8ms frame size, 256 Bytes is 8ms
    pipeline->setProperty("pcm-frame-size", "256");
    pipeline->setProperty("clksrc", "hfrc");

    promptsrc->setProperty("gain", "0.1");
    promptsrc->setProperty("sine-wave-frequency", "400");

    reframe_8_10->setProperty("submit-empty", "1");
    reframe_8_10->setProperty("out-frame-size", "320");
    reframe_8_10->setProperty("out-frame-number", "4");
    reframe_10_8->setProperty("out-frame-size", "256");
    reframe_10_8->setProperty("out-frame-number", "5");

    gain->setProperty("gain", "3");

    mixer->setProperty("fade-in-frames", "50");
    mixer->setProperty("fade-out-frames", "10");
    mixer->setProperty("pad-0-target-gain", "0.3");
    mixer->setProperty("pad-1-target-gain", "0.9");

    adevsrc_pdm->setProperty("endpoint", "pdm-0");

    adevsink_i2s0->setProperty("endpoint", "i2s-0");
    adevsink_i2s0->setProperty("role", "master");

    ((NnseElement *)nnse)->setOps(&nnse_ops);

    return true;
}

#else

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
    pipeline->addElement(nnse);
    pipeline->addElement(gain);
    pipeline->addElement(mixer);
    pipeline->addElement(adevsink_i2s0);

    pipeline->addElement(promptsrc);

    // Add Sniffer Element into pipeline. Don't link sniffer.
    pipeline->addElement(sniffer);

    // Link Uplink elements
    adevsrc_pdm->linkElement(nnse);
    nnse->linkElement(gain);
    gain->linkElement(mixer);
    mixer->linkElement(adevsink_i2s0);

    // Prompt src should be the second that link to mixer.
    promptsrc->linkElement(mixer);

    // Set properties
    pipeline->setProperty("sample-rate", "16000");
    pipeline->setProperty("channels", "1");
    // NNSE requires 10ms frame size, 320 Bytes is 8ms
    pipeline->setProperty("pcm-frame-size", "320");
    pipeline->setProperty("clksrc", "hfrc");

    promptsrc->setProperty("gain", "0.1");
    promptsrc->setProperty("sine-wave-frequency", "400");

    gain->setProperty("gain", "6");

    mixer->setProperty("fade-in-frames", "50");
    mixer->setProperty("fade-out-frames", "10");
    mixer->setProperty("pad-0-target-gain", "0.3");
    mixer->setProperty("pad-1-target-gain", "0.9");

    adevsrc_pdm->setProperty("endpoint", "pdm-0");

    adevsink_i2s0->setProperty("endpoint", "i2s-0");
    adevsink_i2s0->setProperty("role", "master");

    ((NnseElement *)nnse)->setOps(&nnse_ops);

    return true;
}

#endif
