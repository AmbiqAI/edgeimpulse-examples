//*****************************************************************************
//!
//! @file pipeline_config.cpp
//!
//! @brief AEC pipeline for USB audio loopback with NNSE.
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
//! i2s1<-convert<-resample<-nnse<--|
//!                                 |
//! i2s1->convert->resample->-------|
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
    pipeline->addElement(adevsrc_i2s1);
    pipeline->addElement(convert_dl);
    pipeline->addElement(resample1);
    pipeline->addElement(nnse);
    pipeline->addElement(resample2);
    pipeline->addElement(convert);
    pipeline->addElement(adevsink_i2s1);

    // Link element
    adevsrc_i2s1->linkElement(convert_dl);
    convert_dl->linkElement(resample1);
    resample1->linkElement(nnse);
    nnse->linkElement(resample2);
    resample2->linkElement(convert);
    convert->linkElement(adevsink_i2s1);

    // Set properties
    pipeline->setProperty("sample-rate", "16000");
    pipeline->setProperty("channels", "1");
    pipeline->setProperty("pcm-frame-size", "320"); // 10ms
    pipeline->setProperty("clksrc", "mclk");

    adevsrc_i2s1->setProperty("sample-rate", "48000");
    adevsrc_i2s1->setProperty("channels", "2");
    adevsrc_i2s1->setProperty("endpoint", "i2s-1");
    adevsrc_i2s1->setProperty("role", "slave");

    convert_dl->setProperty("in-channels", "2");
    convert_dl->setProperty("out-channels", "1");
    convert_dl->setProperty("mode", "left");

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

    ((NnseElement *)nnse)->setOps(&nnse_ops);

    return true;
}
