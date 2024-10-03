//*****************************************************************************
//!
//! @file pipeline_config.cpp
//!
//! @brief AEC pipeline for USB audio loopback with NNSE and CAE.
//!
//! Hardware Setup:
//! - USB-Hub(J10 on AP5 EB) connects to I2S-1.
//! - Audio Peripheral Card connect to high-speed port.
//! - TDK PDM microphone board plugs on Audio Peripheral Card.
//! - Transducer connnect to J9 of Audio Peripheral Card.
//! - PC connect to AP5 EB Main USB port.
//! - AP5 EB will be emulated as an audio card on PC called I2S-Bridge.
//!
//! GPIO Inforamtion
//!  | PIN Config | GPIO     | Description
//!  | MCLK       | GPIO-15  | Clock source. Connect to USB-HUB. Frequency is 12.288MHz.
//!  | I2S0-BCLK  | GPIO-5   | Speaker. Connect to Amplifier.
//!  | I2S0-LRCLK | GPIO-7   | Speaker. Connect to Amplifier.
//!  | I2S0-DOUT  | GPIO-6   | Speaker. Connect to Amplifier.
//!  | I2S1-BCLK  | GPIO-16  | Audio source. Connect to USB-HUB.
//!  | I2S1-LRCLK | GPIO-18  | Audio source. Connect to USB-HUB.
//!  | I2S1-DOUT  | GPIO-17  | Audio source. Connect to USB-HUB.
//!  | I2S1-DIN   | GPIO-19  | Audio source. Connect to USB-HUB.
//!  | PDM-CLK    | GPIO-50  | Microphone. Connect to TDK.
//!  | PDM-DATA   | GPIO-51  | Microphone. Connect to TDK.
//!  | IOM7-SCL   | GPIO-22  | Amplifier. Connect to Amplifier.
//!  | IOM7-SDA   | GPIO-23  | Amplifier. Connect to Amplifier.
//!
//! Pipeline Configuration:
//! - Sample rate: 16kHz
//! - Channels: 1
//! - Frame length: 8ms
//! - Clock source: mclk
//!
//! Pipeline Graph:
//! i2s1<-convert<-resample<-agc<-peq<-drc<-ns<-reframe<-nnse<-reframe<-|
//!                                                                     |
//! i2s1->convert->resample->gain->mbdrc->peq->drc->iir-->--->----------|
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

    // Downlink
    pipeline->addElement(adevsrc_i2s1);
    pipeline->addElement(convert_dl);
    pipeline->addElement(resample1);
    pipeline->addElement(gain);
    pipeline->addElement(mbdrc);
    pipeline->addElement(peq_dl);
    pipeline->addElement(drc_dl);
    pipeline->addElement(iir_filter);

    // Uplink
    pipeline->addElement(reframe_8_10);
    pipeline->addElement(nnse);
    pipeline->addElement(reframe_10_8);
    pipeline->addElement(ns);
    pipeline->addElement(drc_ul);
    pipeline->addElement(peq_ul);
    pipeline->addElement(agc);
    pipeline->addElement(resample2);
    pipeline->addElement(convert);
    pipeline->addElement(adevsink_i2s1);

    // Downlink
    adevsrc_i2s1->linkElement(convert_dl);
    convert_dl->linkElement(resample1);
    resample1->linkElement(gain);
    gain->linkElement(mbdrc);
    mbdrc->linkElement(peq_dl);
    peq_dl->linkElement(drc_dl);
    drc_dl->linkElement(iir_filter);

    // Uplink
    iir_filter->linkElement(reframe_8_10); // Downlink loopback to Uplink
    reframe_8_10->linkElement(nnse);  // NNSE is 10ms frame size
    nnse->linkElement(reframe_10_8);
    reframe_10_8->linkElement(ns);
    ns->linkElement(drc_ul);
    drc_ul->linkElement(peq_ul);
    peq_ul->linkElement(agc);
    agc->linkElement(resample2);
    resample2->linkElement(convert);
    convert->linkElement(adevsink_i2s1);

    // Set properties
    pipeline->setProperty("sample-rate", "16000");
    pipeline->setProperty("channels", "1");
    pipeline->setProperty("pcm-frame-size", "256"); // 10ms
    pipeline->setProperty("clksrc", "mclk");

    adevsrc_i2s1->setProperty("sample-rate", "48000");
    adevsrc_i2s1->setProperty("channels", "2");
    adevsrc_i2s1->setProperty("endpoint", "i2s-1");
    adevsrc_i2s1->setProperty("role", "slave");

    convert_dl->setProperty("in-channels", "2");
    convert_dl->setProperty("out-channels", "1");
    convert_dl->setProperty("mode", "left");

    reframe_8_10->setProperty("submit-empty", "1");
    reframe_8_10->setProperty("out-frame-size", "320");
    reframe_8_10->setProperty("out-frame-number", "4");
    reframe_10_8->setProperty("out-frame-size", "256");
    reframe_10_8->setProperty("out-frame-number", "5");

    resample1->setProperty("sr-in", "48000");
    resample1->setProperty("sr-out", "16000");

    resample2->setProperty("sr-in", "16000");
    resample2->setProperty("sr-out", "48000");

    convert->setProperty("in-channels", "1");
    convert->setProperty("out-channels", "2");

    adevsink_i2s1->setProperty("sample-rate", "48000");
    adevsink_i2s1->setProperty("channels", "2");
    adevsink_i2s1->setProperty("endpoint", "i2s-1");
    adevsink_i2s1->setProperty("role", "slave");

    gain->setProperty("gain", "1");

    peq_dl->setProperty("link", "downlink");
    peq_ul->setProperty("link", "uplink");

    drc_dl->setProperty("link", "downlink");
    drc_ul->setProperty("link", "uplink");

    ((NnseElement *)nnse)->setOps(&nnse_ops);

    return true;
}
