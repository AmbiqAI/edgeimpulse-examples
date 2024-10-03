/**
 ****************************************************************************************
 *
 * @file wnr_core.h
 *
 * @brief Declaration of Noise Suppression(NS) internal API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _WNR_CORE_H_
#define _WNR_CORE_H_

#ifdef CMSIS_15
#include "arm_math.h"
#include "arm_const_structs.h"
#else
#include "kiss_fft.h"
#include "kiss_fftr.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif

#define WNR_MAXBINNUM                  (33)
#define WNR_DATAWINLEN                 (33 * 8)
#define WNR_DATAWINNUM                 (8)
#define WNR_UPDATELEN                  (21)
#define WNR_SNRMAPCOEFA                (1 - 0.005)
#define WNR_SNRMAPCOEFB                (1 - 0.0025)
#define WNR_ALPHA_COEF                 (0.85)
#define WNR_SPEECHDEC_THRESHOLDA       (7.636)
#define WNR_SPEECHDEC_THRESHOLDB       (2.7722)
#define WNR_SPEECHDEC_MIN_COMPENS      (1.66)
#define WNR_SPEECHPROB_STAT            (2)
#define WNR_SPEECHPROB_SNR_SMTH_COEF   (0.95)
#define WNR_SPEECHPROB_THRESHOLDC      (3.0)
#define WNR_NOISESPECT_ALPHA_COEF      (0.85)
#define WNR_NOISESPECT_ALPHA_COEF_L    (0.99)
#define WNR_APSSMOOTH_ASNR_COEF        (0.7)
#define WNR_PRO_BIN_IDXA               (13)
#define WNR_PRO_BIN_IDXB               (31)
#define WNR_PROABSENCE_BIN_IDXC        (2)
#define WNR_PROABSENCE_SNR_H_LIMIT     (10)
#define WNR_PROABSENCE_SNR_L_LIMIT     (0)
#define WNR_PROABSENCE_ABSENCE_PRO_MAX (0.998)
#define WNR_PSTSNRMAX                  (255)
#define WNR_SMTHSTARTFRM               (14)
#define WNR_SNRFACT_MAX                (15.9424)
#define WNR_NOISESPECT_PROBTHD         (0.99945)
#define WNR_NOISESPECT_NOISECOMPENS    (1.4685)
#define WNR_PRESENCEPROB_TH1           (0.1)
#define WNR_PRESENCEPROB_TH2           (0.3162)
#define WNR_PRESENCEPROB_FRMTH         (120)
#define WNR_PRESENCEPROB_POWTH         (2.5)
#define WNR_PROB_MIN                   (0.005)
#define WNR_ABSENCEPROB_SNRTH          (-10)
#define WNR_ABSENCEPROB_SNRPENALTYA    (5)
#define WNR_ABSENCEPROB_SNRPENALTYB    (10)
#define WNR_ABSENCEPROB_DIVISOR        (5)
#define WNR_GAIN_ABSENCEPROBTH         (0.9)
#define WNR_GAIN_SMOOTHCOEF            (0.2)
#define WNR_PRISNRMAXLIMIT             (255)
#define WNR_MINPOWERTHD                (16384 * 4)

#define VK_TABLE1_LEN   100
#define VK_TABLE2_LEN   46
#define VK_TABLE3_LEN   46
#define VK_TABLE4_LEN   46
#define VK_TABLE1_START 1.000000000000e-05f
#define VK_TABLE1_END   0.00496000000000000f
#define VK_TABLE2_START VK_TABLE1_END
#define VK_TABLE2_END   0.0499600000000000f
#define VK_TABLE3_START VK_TABLE2_END
#define VK_TABLE3_END   0.499960000000000f
#define VK_TABLE4_START VK_TABLE3_END
#define VK_TABLE4_END   4.99996000000000f

    typedef struct
    {
        int   swIdx;
        short shwZfNum;
        short shwIdxSwitch;
        float NoisePow[WNR_MAXBINNUM];
        float NoisePowRud[WNR_MAXBINNUM];
        float PowSpctSmth[WNR_MAXBINNUM];
        float PowSpctCond[WNR_MAXBINNUM];
        float PowSpctMin[WNR_MAXBINNUM];
        float PowSpctMinC[WNR_MAXBINNUM];
        float PowSpctMinCurt[WNR_MAXBINNUM];
        float PowSpctMinCurtC[WNR_MAXBINNUM];
        float SNRSmth[WNR_MAXBINNUM];
        short PresenIndSmth[WNR_MAXBINNUM];
        short shwSubBandNum;
        short shwOpertBandNum;
        short freqnum;
        short dt_flag;
        short wind_flag;
        short spectral_Centroid;
        float old_powspec[128];
        short wind_count;
        float wind_gain[WNR_MAXBINNUM];
        short wind_gain_flag;
    } WNR_COMM_STRU;

    typedef struct
    {
        float PowSpctSmth[WNR_MAXBINNUM];
        float PowSpct[WNR_MAXBINNUM];
    } WNR_TFSMOOTH_STRU;

    typedef struct
    {
        short SpechIdx[WNR_MAXBINNUM];
    } WNR_SPEECHDEC_STRU;

    typedef struct
    {
        float shwSNRMin;
        float PriSNR[WNR_MAXBINNUM];
        float SpchProb[WNR_MAXBINNUM];
    } WNR_SPEECHPROB_STRU;

    typedef struct
    {
        float PowStck[WNR_DATAWINLEN];
        float CondPowStck[WNR_DATAWINLEN];
    } WNR_MINTRACKUPDATE_STRU;

    typedef struct
    {
        float NoisePowLong[WNR_MAXBINNUM];
    } WNR_NOISESPECT_STRU;

    typedef struct
    {
        float SNRSmthGlb[WNR_MAXBINNUM];
        float SNRSmthLcl[WNR_MAXBINNUM];
    } WNR_SNRSMOOTH_STRU;

    typedef struct
    {
        float ProbLcl[WNR_MAXBINNUM];
        float ProbGlb[WNR_MAXBINNUM];
    } WNR_PRESENCEPROB_STRU;

    typedef struct
    {
        float shwMeanPriSNR;
        float shwPriSNR;
        float AbsenProb[WNR_MAXBINNUM];
    } WNR_ABSENCEPROB_STRU;

    typedef struct
    {
        float PriSNRMid[WNR_MAXBINNUM];
        float shwSNRSqrtMin;
        float Gain[WNR_MAXBINNUM];
    } WNR_GAIN_STRU;

    typedef struct
    {
        WNR_COMM_STRU           stCommon;
        WNR_TFSMOOTH_STRU       stTFSmooth;
        WNR_SPEECHDEC_STRU      stSpeechDec;
        WNR_SPEECHPROB_STRU     stSpeechProb;
        WNR_MINTRACKUPDATE_STRU stMinTrackUpdate;
        WNR_NOISESPECT_STRU     stNoiseSpect;
        WNR_SNRSMOOTH_STRU      stSnrSmooth;
        WNR_PRESENCEPROB_STRU   stPresenceProb;
        WNR_ABSENCEPROB_STRU    stAbsenceProb;
        WNR_GAIN_STRU           stGain;
        short                   shwEnable;
        short                   shwReduceDB;
        short                   dt_flag;
    } WNR_STRU;

    int  WNR_MIC_Main(void *pInstance, float *pshwFrmFreq,
                      float *pshwFrmFreqRef);
    void WNR_MIC_SetPara(void *pInstance, short enSampleRate, short ns_level);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // _WNR_CORE_H_
