/**
 ****************************************************************************************
 *
 * @file ns_core.h
 *
 * @brief Declaration of Noise Suppression(NS) internal API.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */

#ifndef _NS_CORE_H_
#define _NS_CORE_H_

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

#define NS_MAXBINNUM                  (33)
#define NS_DATAWINLEN                 (33 * 8)
#define NS_DATAWINNUM                 (8)
#define NS_UPDATELEN                  (21)
#define NS_SNRMAPCOEFA                (1 - 0.005)
#define NS_SNRMAPCOEFB                (1 - 0.0025)
#define NS_ALPHA_COEF                 (0.85)
#define NS_SPEECHDEC_THRESHOLDA       (7.636)
#define NS_SPEECHDEC_THRESHOLDB       (2.7722)
#define NS_SPEECHDEC_MIN_COMPENS      (1.66)
#define NS_SPEECHPROB_STAT            (2)
#define NS_SPEECHPROB_SNR_SMTH_COEF   (0.95)
#define NS_SPEECHPROB_THRESHOLDC      (3.0)
#define NS_NOISESPECT_ALPHA_COEF      (0.85)
#define NS_NOISESPECT_ALPHA_COEF_L    (0.99)
#define NS_APSSMOOTH_ASNR_COEF        (0.7)
#define NS_PRO_BIN_IDXA               (13)
#define NS_PRO_BIN_IDXB               (31)
#define NS_PROABSENCE_BIN_IDXC        (2)
#define NS_PROABSENCE_SNR_H_LIMIT     (10)
#define NS_PROABSENCE_SNR_L_LIMIT     (0)
#define NS_PROABSENCE_ABSENCE_PRO_MAX (0.998)
#define NS_PSTSNRMAX                  (255)
#define NS_SMTHSTARTFRM               (14)
#define NS_SNRFACT_MAX                (15.9424)
#define NS_NOISESPECT_PROBTHD         (0.99945)
#define NS_NOISESPECT_NOISECOMPENS    (1.4685)
#define NS_PRESENCEPROB_TH1           (0.1)
#define NS_PRESENCEPROB_TH2           (0.3162)
#define NS_PRESENCEPROB_FRMTH         (120)
#define NS_PRESENCEPROB_POWTH         (2.5)
#define NS_PROB_MIN                   (0.005)
#define NS_ABSENCEPROB_SNRTH          (-10)
#define NS_ABSENCEPROB_SNRPENALTYA    (5)
#define NS_ABSENCEPROB_SNRPENALTYB    (10)
#define NS_ABSENCEPROB_DIVISOR        (5)
#define NS_GAIN_ABSENCEPROBTH         (0.9)
#define NS_GAIN_SMOOTHCOEF            (0.2)
#define NS_PRISNRMAXLIMIT             (255)
#define NS_MINPOWERTHD                (16384 * 4)

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
        float NoisePow[NS_MAXBINNUM];
        float NoisePowRud[NS_MAXBINNUM];
        float PowSpctSmth[NS_MAXBINNUM];
        float PowSpctCond[NS_MAXBINNUM];
        float PowSpctMin[NS_MAXBINNUM];
        float PowSpctMinC[NS_MAXBINNUM];
        float PowSpctMinCurt[NS_MAXBINNUM];
        float PowSpctMinCurtC[NS_MAXBINNUM];
        float SNRSmth[NS_MAXBINNUM];
        short PresenIndSmth[NS_MAXBINNUM];
        short shwSubBandNum;
        short shwOpertBandNum;
        short freqnum;
        short dt_flag;
    } NS_COMM_STRU;

    typedef struct
    {
        float PowSpctSmth[NS_MAXBINNUM];
        float PowSpct[NS_MAXBINNUM];
    } NS_TFSMOOTH_STRU;

    typedef struct
    {
        short SpechIdx[NS_MAXBINNUM];
    } NS_SPEECHDEC_STRU;

    typedef struct
    {
        float shwSNRMin;
        float PriSNR[NS_MAXBINNUM];
        float SpchProb[NS_MAXBINNUM];
    } NS_SPEECHPROB_STRU;

    typedef struct
    {
        float PowStck[NS_DATAWINLEN];
        float CondPowStck[NS_DATAWINLEN];
    } NS_MINTRACKUPDATE_STRU;

    typedef struct
    {
        float NoisePowLong[NS_MAXBINNUM];
    } NS_NOISESPECT_STRU;

    typedef struct
    {
        float SNRSmthGlb[NS_MAXBINNUM];
        float SNRSmthLcl[NS_MAXBINNUM];
    } NS_SNRSMOOTH_STRU;

    typedef struct
    {
        float ProbLcl[NS_MAXBINNUM];
        float ProbGlb[NS_MAXBINNUM];
    } NS_PRESENCEPROB_STRU;

    typedef struct
    {
        float shwMeanPriSNR;
        float shwPriSNR;
        float AbsenProb[NS_MAXBINNUM];
    } NS_ABSENCEPROB_STRU;

    typedef struct
    {
        float PriSNRMid[NS_MAXBINNUM];
        float shwSNRSqrtMin;
        float Gain[NS_MAXBINNUM];
    } NS_GAIN_STRU;

    typedef struct
    {
        NS_COMM_STRU           stCommon;
        NS_TFSMOOTH_STRU       stTFSmooth;
        NS_SPEECHDEC_STRU      stSpeechDec;
        NS_SPEECHPROB_STRU     stSpeechProb;
        NS_MINTRACKUPDATE_STRU stMinTrackUpdate;
        NS_NOISESPECT_STRU     stNoiseSpect;
        NS_SNRSMOOTH_STRU      stSnrSmooth;
        NS_PRESENCEPROB_STRU   stPresenceProb;
        NS_ABSENCEPROB_STRU    stAbsenceProb;
        NS_GAIN_STRU           stGain;
        short                  shwEnable;
        short                  shwReduceDB;
        short                  dt_flag;
    } NS_STRU;

    int NS_MIC_Main(void *pInstance, float *pshwFrmFreq, float *pshwFrmFreqRef);
    void NS_MIC_SetPara(void *pInstance, short enSampleRate, short ns_level);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif // _NS_CORE_H_
