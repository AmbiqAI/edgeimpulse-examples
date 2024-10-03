//*****************************************************************************
//
//! @file pipeline.cpp
//!
//! @brief Audio pipeline control.
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

#include "errno.h"

#include "am_util_stdio.h"
#include "am_util_debug.h"

#include "aust_pipeline.h"
#include "aust_pipeline_state.h"
#include "aust_element_factory.h"

#include "pipeline.h"

#include "aust_usb_src_element.h"
#include "aust_usb_sink_element.h"
#include "webusb_controller.h"

#include "aas_common.h"

#define DELETE_ELEMENT(e)                                                      \
    do                                                                         \
    {                                                                          \
        if (e)                                                                 \
        {                                                                      \
            delete e;                                                          \
            e = NULL;                                                          \
        }                                                                      \
    } while (0)

AustElement  *adevsrc_pdm   = NULL;
AustElement  *adevsink_i2s0 = NULL;
AustElement  *adevsrc_i2s1  = NULL;
AustElement  *adevsink_i2s1 = NULL;
AustElement  *dummysrc      = NULL;
AustElement  *dummysink     = NULL;
AustElement  *usbdatasrc    = NULL;
AustElement  *promptsrc     = NULL;

AustElement  *resample      = NULL;
AustElement  *resample1     = NULL;
AustElement  *resample2     = NULL;
AustElement  *convert       = NULL;
AustElement  *convert_dl    = NULL;
AustElement  *demux         = NULL;
AustElement  *mux           = NULL;
AustElement  *splitter      = NULL;
AustElement  *splitter1     = NULL;
AustElement  *mixer         = NULL;
AustElement  *reframe_8_10  = NULL; // Reframe 8ms to 10ms
AustElement  *reframe_10_8  = NULL; // Reframe 10ms to 8ms
AustElement  *sniffer       = NULL;

AustElement  *asp           = NULL;
AustElement  *aec           = NULL;
AustElement  *ns            = NULL;
AustElement  *wnr           = NULL;
AustElement  *peq_ul        = NULL;
AustElement  *agc           = NULL;
AustElement  *gain          = NULL;
AustElement  *mbdrc         = NULL;
AustElement  *iir_filter    = NULL;
AustElement  *peq_dl        = NULL;
AustElement  *drc_ul        = NULL;
AustElement  *drc_dl        = NULL;
AustElement  *nnse          = NULL;
AustElement  *nnid          = NULL;
AustElement  *vad          = NULL;

// DSP pipeline
AustPipeline *pipeline_dsp  = NULL;


// TOOL pipeline
AustPipeline *pipeline_tool = NULL;
AustElement  *usbsrc        = NULL;
AustElement  *toolsink      = NULL;
AustElement  *toolsrc       = NULL;
AustElement  *usbsink       = NULL;

void delete_dsp_elements(void)
{
    DELETE_ELEMENT(adevsrc_pdm);
    DELETE_ELEMENT(adevsink_i2s0);
    DELETE_ELEMENT(adevsrc_i2s1);
    DELETE_ELEMENT(adevsink_i2s1);
    DELETE_ELEMENT(dummysrc);
    DELETE_ELEMENT(dummysink);
    DELETE_ELEMENT(usbdatasrc);
    DELETE_ELEMENT(promptsrc);

    DELETE_ELEMENT(resample);
    DELETE_ELEMENT(resample1);
    DELETE_ELEMENT(resample2);
    DELETE_ELEMENT(convert);
    DELETE_ELEMENT(convert_dl);
    DELETE_ELEMENT(demux);
    DELETE_ELEMENT(mux);
    DELETE_ELEMENT(splitter);
    DELETE_ELEMENT(splitter1);
    DELETE_ELEMENT(mixer);
    DELETE_ELEMENT(reframe_8_10);
    DELETE_ELEMENT(reframe_10_8);
    DELETE_ELEMENT(sniffer);

    DELETE_ELEMENT(asp);
    DELETE_ELEMENT(aec);
    DELETE_ELEMENT(ns);
    DELETE_ELEMENT(wnr);
    DELETE_ELEMENT(peq_ul);
    DELETE_ELEMENT(agc);
    DELETE_ELEMENT(gain);
    DELETE_ELEMENT(mbdrc);
    DELETE_ELEMENT(iir_filter);
    DELETE_ELEMENT(peq_dl);
    DELETE_ELEMENT(drc_ul);
    DELETE_ELEMENT(drc_dl);

    DELETE_ELEMENT(nnse);
    DELETE_ELEMENT(nnid);
    DELETE_ELEMENT(vad);
    DELETE_ELEMENT(pipeline_dsp);
}

bool create_dsp_elements(void)
{
    // Obtain element factory, factory is Singleton Pattern
    ElementFactory &factory = ElementFactory::getInstance();

    //*****************************************************************
    // Create Dsp Pipeline
    //*****************************************************************
    pipeline_dsp = (AustPipeline *)factory.make("pipeline", "DSP");

    // Source and Sink Elements
    adevsrc_pdm   = factory.make("adevsrc", "AdevSrcPdm");
    adevsink_i2s0 = factory.make("adevsink", "AdevSinkI2s0");
    adevsrc_i2s1  = factory.make("adevsrc", "AdevSrcI2s1");
    adevsink_i2s1 = factory.make("adevsink", "AdevSinkI2s1");
    dummysrc      = factory.make("dummysrc", "DummySource");
    dummysink     = factory.make("dummysink", "DummySink");
    usbdatasrc    = factory.make("usbdata", "UsbData");
    promptsrc     = factory.make("promptsrc", "PromptSrc");

    // Util Elements
    splitter      = factory.make("splitter", "Splitter");
    splitter1     = factory.make("splitter", "Splitter1");
    mixer         = factory.make("mixer", "Mixer");
    resample      = factory.make("resample", "Resample");
    resample1     = factory.make("resample", "Resample1");
    resample2     = factory.make("resample", "Resample2");
    convert       = factory.make("convert", "ConvertUl");
    convert_dl    = factory.make("convert", "ConvertDl");
    demux         = factory.make("demux", "Demux");
    mux           = factory.make("mux", "Mux");
    reframe_8_10  = factory.make("reframe", "Reframe-8-10");
    reframe_10_8  = factory.make("reframe", "Reframe-10-8");
    sniffer       = factory.make("sniffer", "Sniffer");

    // Algorithm Elements
    asp           = factory.make("asp", "ASP");
    aec           = factory.make("aec", "AEC");
    ns            = factory.make("ns", "NS");
    wnr           = factory.make("wnr", "WNR");
    peq_ul        = factory.make("peq", "PeqUl");
    agc           = factory.make("agc", "AGC");
    gain          = factory.make("gain", "Gain");
    mbdrc         = factory.make("mbdrc", "MBDRC");
    iir_filter    = factory.make("iirfilter", "IirFilter");
    peq_dl        = factory.make("peq", "PeqDl");
    drc_ul        = factory.make("drc", "DrcUl");
    drc_dl        = factory.make("drc", "DrcDl");
    nnse          = factory.make("nnse", "NNSE");
    nnid          = factory.make("generic", "NNID");
    vad           = factory.make("vad", "VAD");

    if (!pipeline_dsp || !adevsrc_pdm || !adevsink_i2s0 || !adevsrc_i2s1 || !adevsink_i2s1 ||
        !dummysrc || !dummysink || !usbdatasrc || !promptsrc || !splitter || !splitter1 ||
        !mixer || !sniffer || !resample || !resample1 || !resample2 || !convert || !convert_dl ||
        !demux || !mux || !reframe_8_10 || !reframe_10_8 || !asp || !aec || !ns  || !wnr || !peq_ul || !agc ||
        !gain || !mbdrc || !iir_filter || !peq_dl || !drc_ul || !drc_dl || !nnse || !nnid || !vad)
    {
        goto fail;
    }

    am_util_debug_printf("pipeline %s success\n", __func__);

    return true;

fail:
    am_util_debug_printf("Error: pipeline %s fail\n", __func__);

    delete_dsp_elements();

    return false;
}

void destroy_dsp_pipeline(void)
{
    delete_dsp_elements();
}

void destroy_tool_pipeline(void)
{
    DELETE_ELEMENT(usbsrc);
    DELETE_ELEMENT(toolsink);
    DELETE_ELEMENT(toolsrc);
    DELETE_ELEMENT(usbsink);
    DELETE_ELEMENT(pipeline_tool);
}

bool create_dsp_pipeline(void)
{
    if (!create_dsp_elements())
    {
        return false;
    }

    return pipeline_config(pipeline_dsp);
}

bool create_tool_pipeline(void)
{
    if (pipeline_tool)
    {
        am_util_debug_printf("pipeline created already\n");
        return true;
    }

    // Obtain element factory, factory is Singleton Pattern
    ElementFactory &factory = ElementFactory::getInstance();

    //*****************************************************************
    // Create Tool Pipeline
    //*****************************************************************
    pipeline_tool = (AustPipeline *)factory.make("pipeline", "TOOL");

    // Create each element
    usbsrc   = factory.make("usbsrc", "UsbSrc");
    toolsink = factory.make("acoresink", "ToolSink");
    toolsrc  = factory.make("acoresrc", "ToolSrc");
    usbsink  = factory.make("usbsink", "UsbSink");

    if (!pipeline_tool || !usbsrc || !toolsink || !toolsrc || !usbsink)
    {
        goto fail;
    }

    // Add element into pipeline
    pipeline_tool->addElement(usbsrc);
    pipeline_tool->addElement(toolsink);
    pipeline_tool->addElement(toolsrc);
    pipeline_tool->addElement(usbsink);

    // Link element
    usbsrc->linkElement(toolsink);
    toolsrc->linkElement(usbsink);

    // Set properties
    ((UsbSinkElement *)usbsink)->register_process_cb(webusb_send_data_flush);
    webusb_register_msg_cb(UsbSrcElement::handleUsbPacket, usbsrc);
    am_util_debug_printf("tool pipeline created\n");

    return true;

fail:
    pipeline_destroy();

    return false;
}

bool pipeline_create(void)
{
    if (!create_dsp_pipeline() || !create_tool_pipeline())
    {
        pipeline_destroy();
        return false;
    }

    return true;
}

int pipeline_set_state(int state)
{
    int ret = 0;

    if (!pipeline_dsp)
    {
        am_util_debug_printf("create pipeline first\n");
        return -ENODEV;
    }

    switch (state)
    {
        case ST_STOP:
            ret = aust_pipeline_set_state(pipeline_tool, STATE_IDLE);
            if (ret)
            {
                break;
            }
            ret = aust_pipeline_set_state(pipeline_dsp, STATE_IDLE);
            break;

        case ST_PAUSE:
            ret = aust_pipeline_set_state(pipeline_tool, STATE_PAUSE);
            if (ret)
            {
                break;
            }
            ret = aust_pipeline_set_state(pipeline_dsp, STATE_PAUSE);
            break;

        case ST_PLAYING:
            ret = aust_pipeline_set_state(pipeline_tool, STATE_PLAY);
            if (ret)
            {
                break;
            }
            ret = aust_pipeline_set_state(pipeline_dsp, STATE_PLAY);
            break;

        default:
            ret = -EINVAL;
            am_util_debug_printf("pipeline set unknown state\n");
            break;
    }

    return ret;
}

void pipeline_destroy(void)
{
    am_util_debug_printf("delete elements\n");

    destroy_dsp_pipeline();

    destroy_tool_pipeline();

    am_util_debug_printf("pipeline destroyed\n");
}
