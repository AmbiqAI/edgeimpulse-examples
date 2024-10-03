/**
 ****************************************************************************************
 *
 * @file aec_nlp.h
 *
 * @brief API of Non-Linear Processing module.
 * NLP is a frequency domain processing algorithm.
 *
 * Copyright (c) 2024, Ambiq Micro, Inc.
 * All rights reserved.
 *
 ****************************************************************************************
 */
#ifndef _NLP_H_
#define _NLP_H_
#include <math.h>
#include "arch.h"
#include "os_support.h"
#ifdef CMSIS_15
#include "arm_math.h"
#include "arm_const_structs.h"
#else
#include "kiss_fft.h"
#include "kiss_fftr.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define NLP_GAMMA_MAX     (1)
#define NLP_GAMMA_FIRST   (0.5)
#define NLP_GAMMA_MIN     (0.01)
#define NLP_GRAD_NUM      (20)
#define NLP_SUB_FRAME_NUM (2)

#define MAX_WIENER_DEC      (10)
#define MAX_WIENER_OFF      (19)
#define NLP_WIENER_LVL      (1)
#define NLP_WIENER_MIN_COEF (0.18)

#define NLP_SUPP_EN_THD (10000)

#define NLP_NL_COEFF_MIN_TRACK

#define DTD_FLAG_ST     (1) // echo
#define DTD_FLAG_DT     (2) // local + echo
#define DTD_FLAG_PASS   (0) // no echo
#define DTD_MUTE_HO_THD (30)

#define RES_MIN_LEAK  (0.005f)
#define POSTSER_THR   (100)
#define PRIORISER_THR (100)

#define XMIN(X, Y) ((X) < (Y) ? (X) : (Y))
#define XMAX(X, Y) ((X) > (Y) ? (X) : (Y))

    typedef struct
    {
        float nl_coeff_fb_local_min;
        float nl_coeff_new_min;
        int   nl_coeff_min_ctr;
        float nl_coeff_fb_min;
#ifdef NLP_NL_COEFF_MIN_TRACK
        int   min_hold_time;
        int   min_track_len;
        float min_coeff;
        float min_coeff_tmp;
#endif
    } LocalMinTrack_t;

    typedef struct
    {
        float GammaPre;
        float Dt2StGradsTab[20];
        short Dt2StGradsLen;
        float St2DtGradsTab[20];
        short St2DtGradsLen;
        short Dt2StCnt;
        short St2DtCnt;
    } NLP_SWITCH_STRU;

    typedef struct
    {
        short first_DtdFlag;
        short second_DtdFlag;
        short shwDtConLen;
        short shwDtHangLen;
        float shwFarPowerThd;
        short shwMuteHandover;
        float near_eng;
        float res_eng;
        float ref_eng;
        float ref_max_eng;
        float ref_min_eng;
        float ref_count;
        float ref_count_thrd1;
    } DdoubleTalkDetect_t;

    typedef struct
    {
        float *Xf_res_echo;
        float *Xf_echo;
        int    ccsize;
        int    nb_adapt;
        float *echoPsd;
        float *res1_old_ps;
        float *res2_old_ps;
        float *Eh;
        float *Yh;
        float *res1_echo_noise;
        float *res2_echo_noise;
        float *res1_zeta;
        float *res2_zeta;
        float  spec_average;
        float  beta0;
        float  beta_max;
        float  Pey_avg;
        float  Pyy_avg;
        float  res1_echo_noise_factor;
        float  res2_echo_noise_factor;
        float  res1_echo_suppress_default;
        float  res2_st_echo_suppress_default;
        float  res2_dt_echo_suppress_default;
        float  res1_echo_suppress_active_default;
        float  res2_st_echo_suppress_active_default;
        float  res2_dt_echo_suppress_active_default;
        float  res1_suppress_factor;
        float  res2_st_suppress_factor;
        float  res2_dt_suppress_factor;

        float *res_echo_noise;
        float *res_zeta;
        float  res_echo_noise_factor;
        float *res_echo_psd;
        float *res_old_ps;
    } objRES;

    typedef struct
    {
        float          *psd_mic_smooth;
        float          *psd_res_smooth;
        float          *psd_ref_smooth;
        float          *xpsd_mic_res_smooth;
        float          *xpsd_mic_ref_smooth;
        float          *outBuf;
        LocalMinTrack_t mintrack;
        float           nl_coeff_xd_avg_min;
        int             diverge_state;
        float           over_drive_smooth;
        float           over_drive;
        int             near_state;
        int             near_state_hold;
        float           nl_coeff_fb;
        float           alpha_nl_coeff;
        float           spk_peak_time;
        unsigned int    seed;
        float          *coh_near_res;
        float          *coh_near_far;
        float          *nl_coeff;
        float          *coeff_nl_pref;
        float          *window;
        float          *ref_freq;
        float          *mic_freq;
        float          *res_freq;
#ifdef CMSIS_15
        arm_rfft_fast_instance_f32 *varInstRfftF32;
        arm_cfft_instance_f32      *varInstCfftF32;
#else
    kiss_fftr_cfg kiss_fftr_state;
    kiss_fftr_cfg kiss_ifftr_state;
#endif
        int                 cng_enable;
        DdoubleTalkDetect_t dtd_status;
        NLP_SWITCH_STRU     nlp_switch;
        float               WienerCoef[64 + 1];
        float               psd_echo_smooth[64 + 1];
        float               Sme[64 + 1];
        short               DtCnt;
        short               WienerFulEn;
        short               WienerPrtEn;
        short               nlp_wiener_level;
        float               MIC_noi_est;
        objRES              res_t;
        float               echo_freq[128 + 2];
        short               dt_flag;
    } Nlp_t;

    /**
    ****************************************************************************************
    * @brief NLP initialization function.
    * @param srv NLP instance pointer.
    * @param nlp_level NLP Level.
    * @param band Frequency domain sub-band number.
    ****************************************************************************************
    */
    void aec_nlp_init(Nlp_t *srv, int band, short nlp_level);

    /**
    ****************************************************************************************
    * @brief Reset NLP instance. All the filter buffering data will be clean.
    * This API is called for tuning or debug
    * @param srv NLP instance pointer.
    * @return Success: return 0, Failure: return ERROR_AEC.
    ****************************************************************************************
    */
    int aec_nlp_reset(Nlp_t *srv);

    /**
    ****************************************************************************************
    * @brief Performs NLP processing.
    * @param srv NLP instance pointer.
    * @param noisePow Power of noise
    * @param ref Reference data PCM data from playback.
    * @param mic Input audio PCM data from microphone.
    * @param res_echo Linear processed audio PCM data.
    * @param output Output processed audio data PCM data.
    * @return Success: return 0, Failure : return ERROR_AEC
    ****************************************************************************************
    */
    int aec_nlp_process(Nlp_t *srv, float *noisePow, float *ref, float *mic,
                        float *res_echo, float *output);

    /**
    ****************************************************************************************
    * @brief NLP deinitialization function.
    * @param srv NLP instance pointer.
    * @return Success: return 0, Failure : return ERROR_AEC
    ****************************************************************************************
    */
    int aec_nlp_deinit(Nlp_t *srv);

#ifdef __cplusplus
}
#endif

#endif // _NLP_H_
