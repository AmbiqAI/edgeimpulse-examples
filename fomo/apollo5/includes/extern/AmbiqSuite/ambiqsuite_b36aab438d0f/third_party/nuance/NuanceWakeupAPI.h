/***********************************************************************
 * Copyright 2013, Nuance Communications, Inc.
 *
 *  No part of this manual may be reproduced in any form or by any means,
 *  electronic or mechanical, including photocopying, recording, or by any
 *  information storage and retrieval systems, without the express written
 *  consent of Voice Signal Technologies, Inc.
 *
 *  The content of this manual is furnished for informational use only, is
 *  subject to change without notice, and should not be construed as a
 *  commitment Nuance Communications, Inc. Nuance Communications Inc. assumes
 *  no responsibility or liability for any errors or inaccuracies
 *  that may appear.
 *
 */
/*!
 *  \file NuanceWakeupAPI.h
 *  \brief Nuance Wakeup API (NWA) for low power embedded (DSP) word spotting.
 *   chris.leblanc@nuance.com
 */
#ifndef NWA_INCLUDED
#define NWA_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
#include <limits.h>

#define NWA_HEAP_REQUEST_SIZE (24 * 1024) /* 24 KB  */
#define NWA_FRAME_SAMPLES     (160) /* 10 ms interval */
#define NWA_NUM_FE_FRAMES     (2)
#define NWA_MIN_SPEECH_FRAMES      (60)
#define NWA_END_SILENCE_FRAMES     (25)
#define NWA_ISSPEECH          (1)
#define NWA_LOOKBACK_FRAMES   (50 * 2)
#define NWA_PHRASE_MAX_CHARS  (256)
#define NWA_SAMPLE_BITS       (16)
#define NWA_SAMPLE_RATE       (16000)

/*! \breif Create the Nuance Wakeup object given a language model. Resulting object in idle state.
 *  \param[in] pLanguageModel A pointer to the language data containing the wakeup phrase(s).
 *  \param[in] modelSize Actual size in bytes of the language model to load.
 *  \param[in] pHeapMem Pointer to block of memory size in bytes NWA_HEAP_REQUEST_SIZE.
 *  \return non-zero for error
 */
int nwa_createObject(void * pLanguageModel, size_t modelSize, void * pHeapMem);
    
/*! \breif Destroy the Nuance Wakeup object given a language model. Object must be in idle state.
 *  \return non-zero for error
 */
int nwa_destroyObject(void);
    
/*! \breif Transition to wakeup state.  
 *  \param[in] bUsesMAD specify if the phrase may be chopped from MAD
 *  \return non-zero for error
 */
int nwa_startWakeup(int bUsesMAD);

/*! \breif Transition to idle state.  
 *  \return non-zero for error
 */
int nwa_stopWakeup(void);

/*! \breif Scan the audio samples searching for the wakeup phrase.
 *  \param[in] pFrame Pointer to the current audio frame to scan.
 *  \param[in] nSamples Number of samples available.
 *  \param[out] pbWakeup Set when a wakeup phrase is recognized.
 *  \return non-zero for error
 */
int nwa_scanAudioFrame(short * pFrame, int nSamples, int * pbWakeup);

#define NWA_13MHZ 1
#define NWA_20MHZ 2
#define NWA_30MHZ 3
#define NWA_40MHZ 4
#define NWA_50MHZ 5
#define NWA_100MHZ INT_MAX
/*! \breif Scan the audio samples searching for the wakeup phrase.
 *  \param[in] pFrame Pointer to the current audio frame to scan.
 *  \param[in] nSamples Number of samples available.
 *  \param[in] uPSpeed Minimum uP speed.
 *  \param[out] isSpeech Set when Elvis believes the scanned frame is speech.
 *  \param[out] pbWakeup Set when a wakeup phrase is recognized.
 *  \return non-zero for error
 */
int nwa_scanAudioFrameEx(short * pFrame, int nSamples, int uPSpeed, int * isSpeech, int * pbWakeup);

/*! \breif Get info for last recognized wakeup utterance. Used to identify audio.
 *  \param[out] ppWakeupString Pointer to the wakeup string returned here.
 *  \param[out] pStartFramesBack Number of 10ms frames back where wakeup audio began.
 *  \param[out] pLengthFrames Length in 10ms frames of the recognized wakeup phrase.
 */
int nwa_getWakeupInfo(unsigned short ** ppWakeupString, int * pStartFramesBack, int * pLengthFrames);

/*! \breif Get wakeup score.
 *  \param[out] pScore the score of detected wakeup.
 */
int nwa_getWakeupScore(int *pScore);
    
#ifdef __cplusplus
}
#endif
#endif /* NWA_INCLUDED */
