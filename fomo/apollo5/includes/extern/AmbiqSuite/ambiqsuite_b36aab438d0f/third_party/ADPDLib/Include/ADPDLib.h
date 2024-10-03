/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2015 Analog Devices Inc.  		                              *
* All rights reserved.                                                        *
*                                                                             *
* This source code is intended for the recipient only under the guidelines of *
* the non-disclosure agreement with Analog Devices Inc.                       *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
*                                                                             *
* This software is intended for use with the ADUX1020 and derivative parts    *
* only                                                                        *
*								              *
******************************************************************************/
#ifndef __HEARTRATE_H__
#define __HEARTRATE_H__

#include <stdint.h>

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

typedef enum {
    ADPDLIB_ERR_SUCCESS_WITH_RESULT        =  1,
    ADPDLIB_ERR_SUCCESS                    =  0,
    ADPDLIB_ERR_ON_SENSOR                  = -1,
    ADPDLIB_ERR_DEVICE_ON_MOTION           = -2,
    ADPDLIB_ERR_OFF_SENSOR                 = -3,
    ADPDLIB_ERR_DEVICE_ON_TABLE            = -5,
    ADPDLIB_ERR_TIMEOUT_WAITING_FOR_PERSON = -6,
    ADPDLIB_ERR_SYNC_ERR                   = -100,
    ADPDLIB_ERR_SYNC_BUFFERING             = -101,
    ADPDLIB_ERR_SYNC_OUTOFSYNC             = -102,
} ADPDLIB_ERROR_CODE_t;

typedef struct LibraryResultX {
    float HR;
    float SNR;
    float reserved1;
    float B2B;
    float reserved2;
    float reserved3;
    float reserved4;
} LibResultX_t;

typedef struct timestamp {
    uint32_t tsADPD;
    uint32_t tsADXL;
    uint32_t tsAlgorithmCall;
} TimeStamps_t;

typedef enum {
    ADPDLIB_IDLEMODE = 0x00,
    ADPDLIB_PAUSEMODE = 0x01,
    ADPDLIB_SLOTAMODE = 0x30,
    ADPDLIB_SLOTBMODE = 0x32,
    ADPDLIB_SLOTABMODE = 0x31,
} ADPDLIB_DEVICE_MODE_t;

typedef enum {
    ADPDLIB_STAGE_DETECT_PERSON = 2,
    ADPDLIB_STAGE_LEDCAL = 4,
    ADPDLIB_STAGE_DCLEVELECHECK = 6,
    ADPDLIB_STAGE_HEART_RATE = 7,
    ADPDLIB_STAGE_DARKOFFSET_CALIBRATION = 9
} ADPDLIB_STAGES_t;

extern uint32_t ADPDLib_GetVersion(void);
extern uint32_t ADPDLib_GetAlgorithmVersion(void);
extern ADPDLIB_ERROR_CODE_t ADPDLib_OpenHR(void);
extern ADPDLIB_ERROR_CODE_t ADPDLib_CloseHR(void);
extern ADPDLIB_ERROR_CODE_t ADPDLib_SetConfig(int32_t *lcfg, uint32_t *dcfg);
extern ADPDLIB_ERROR_CODE_t ADPDLib_GetHR(LibResultX_t *result,
                                  uint32_t *slotACh,
                                  uint32_t *slotBCh,
                                  int16_t *acceldata,
                                  TimeStamps_t timeStamp);
extern int32_t ADPDLib_GetMode(int32_t *mode);
extern void ADPDLib_SetFlag5Threshold(float);
extern void ADPDLib_SetInitialTimeout(uint32_t);
extern void ADPDLib_SetFinalTimeout(uint32_t);

extern void ADPDLib_SetOnSettleCnt(uint8_t);
extern void ADPDLib_SetOffSettleCnt(uint8_t);
extern void ADPDLib_SetTriggerOnLevel(uint32_t);
extern void ADPDLib_SetTriggerOnStablizeVR(uint32_t);
extern void ADPDLib_SetTriggerOnIdleVR(uint32_t);
extern void ADPDLib_SetOffTriggerPercent(uint32_t);
extern void ADPDLib_SetTriggerOffStablizeVR(uint32_t);
extern void ADPDLib_SetTriggerLevel(uint32_t);

extern void ADPDLib_SetTransitionFlag(uint8_t);
extern void ADPDLib_SetSNRDisplayHRThreshold(float);
extern void ADPDLib_SetFlagSNRSpotThreshold(float);
extern void ADPDLib_SetSNRB2BThreshold(float);
extern void ADPDLib_Delay(uint32_t delay);
extern unsigned long ADPDLib_GetTick(void);
extern int32_t ADPDLib_TestAlg(LibResultX_t *result,
                                uint32_t *slotA,
                                uint32_t *slotB,
                                int16_t *acceldata);
extern int32_t ADPDLib_GetState(void);
extern int32_t ADPDLib_GetDetectOnValues(uint32_t *val, uint32_t *var);
extern int32_t ADPDMwLib_SetMode(ADPDLIB_DEVICE_MODE_t mode);
extern int ADPDMwLib_DoClockCalibration(uint16_t reg12h, uint16_t reg15h);
extern void ADPDLib_SyncInit(void);
extern int16_t *ADPDLib_GetSyncAccel(void);
extern uint32_t ADPDLib_GetSyncAccelTs(void);
extern uint32_t *ADPDLib_GetSyncAdpd(void);
extern uint32_t ADPDLib_GetSyncAdpdTs(void);
extern void ADPDLib_SyncInit(void);
extern int32_t ADPDLib_DoSyncV1(uint32_t    *slotACh,
             uint32_t    *slotBCh,
             uint32_t     tsADPD,
             int16_t     *acceldata,
             uint32_t     tsAccel);

#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif /*__HEARTRATE_H__*/
