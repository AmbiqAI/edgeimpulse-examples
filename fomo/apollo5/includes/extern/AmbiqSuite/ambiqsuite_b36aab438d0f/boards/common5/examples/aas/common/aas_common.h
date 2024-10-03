//*****************************************************************************
//
//! @file aas_common.h
//!
//! @brief AAS common Elememts and API.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


#ifndef AAS_COMMON_H
#define AAS_COMMON_H

 #include "aust_pipeline.h"

extern AustElement  *adevsrc_pdm;
extern AustElement  *adevsink_i2s0;
extern AustElement  *adevsrc_i2s1;
extern AustElement  *adevsink_i2s1;
extern AustElement  *dummysrc;
extern AustElement  *dummysink;
extern AustElement  *usbdatasrc;
extern AustElement  *promptsrc;

extern AustElement  *resample;
extern AustElement  *resample1;
extern AustElement  *resample2;
extern AustElement  *convert;
extern AustElement  *convert_dl;
extern AustElement  *demux;
extern AustElement  *mux;
extern AustElement  *splitter;
extern AustElement  *splitter1;
extern AustElement  *reframe_8_10; // Reframe 8ms to 10ms
extern AustElement  *reframe_10_8; // Reframe 10ms to 8ms
extern AustElement  *sniffer;

extern AustElement  *asp;
extern AustElement  *aec;
extern AustElement  *ns;
extern AustElement  *wnr;
extern AustElement  *peq_ul;
extern AustElement  *agc;
extern AustElement  *gain;
extern AustElement  *mbdrc;
extern AustElement  *iir_filter;
extern AustElement  *peq_dl;
extern AustElement  *drc_ul;
extern AustElement  *drc_dl;
extern AustElement  *nnse;
extern AustElement  *nnid;
extern AustElement  *vad;
extern AustElement  *mixer;

extern bool pipeline_config(AustPipeline *pipeline);

#endif // AAS_COMMON_H
