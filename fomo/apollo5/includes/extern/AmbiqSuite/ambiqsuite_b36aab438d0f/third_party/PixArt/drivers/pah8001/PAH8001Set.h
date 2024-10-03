
#ifndef PAH8001SET_H
#define PAH8001SET_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "stdint.h"
#include "stdbool.h"
#define I2C_ID_PAH8001	0x33 //7-bit I2C address

bool pah8001_init(void);
uint8_t pah8001_task(void);
uint8_t writeRegister(uint8_t addr, uint8_t data);
uint8_t readRegister(uint8_t addr);
bool Pixart_HRD(void);
static const uint16_t HR_Rpt = 1200; //Heart report rate

static float myHR=0;
static float grade=0;
static uint8_t touch=0;

typedef struct {
    uint8_t HRD_Data[13];
    float MEMS_Data[3];
}ppg_mems_data_t;

#define FIFO_SIZE 5
#define FIFO_SIZE_M1 (FIFO_SIZE-1)
static bool isFIFOEmpty(void);
static bool Push(ppg_mems_data_t *data);
static bool Pop(ppg_mems_data_t *data);
static volatile int _read_index = 0;
static volatile int _write_index = 0;

static ppg_mems_data_t _ppg_mems_data[FIFO_SIZE];
static uint8_t Frame_Count = 0;
static ppg_mems_data_t ppg_mems_data;
//unsigned char ready_flag = 0;
//unsigned char motion_flag = 0;
static float MEMS_Data[3] = {0};

//Accelerometer Sensor Functions
bool accelerometer_init(void);
void accelerometer_start(void);
void accelerometer_stop(void);
#define I2C_ID_ACCEL	0x69
#define ACCE_FIFO_DEPTH 32

static int16_t _g_sensor_data[ACCE_FIFO_DEPTH*3] ;
static uint8_t _bmi160_exist = 0 ;

/***********************LED Control Start***********************************/
#define LED_INC_DEC_STEP 2
#define LED_CTRL_EXPO_TIME_HI_BOUND 496
#define LED_CTRL_EXPO_TIME_LOW_BOUND 32
#define LED_CTRL_EXPO_TIME_HI 420
#define LED_CTRL_EXPO_TIME_LOW 64
#define LED_CURRENT_HI 31
#define LED_CURRENT_LOW 1
#define STATE_COUNT_TH 3
#define DEFAULT_LED_STEP 10
void led_ctrl(void);

static uint8_t _led_step = DEFAULT_LED_STEP;
static uint8_t _state = 0, _state_count = 0;
static uint8_t _led_current_change_flag = 0;
static uint8_t _sleepflag = 1 ;
/***********************LED Control End ***********************************/

static const uint8_t init_ppg_register_array[][2] = {  //based on ap note v0.93
{0x7F,0x00},//Switch bank
{0x09,0x5A},
{0x05,0x99},
{0x17,0xA2},//Make sure MSB =1, without changing other bits
{0x27,0xFF},
{0x28,0xFA},
{0x29,0x0A},
{0x2A,0xC8},
{0x2B,0xA0},
{0x2C,0x8C},
{0x2D,0x64},
{0x42,0x20},
{0x48,0x00},
{0x4D,0x18},//0x1A-->wrist application; 0x18--> Finger tip
{0x7A,0xB5},
{0x7F,0x01},//Switch bank
{0x07,0x48},
{0x23,0x3C},
{0x26,0x0F},
{0x2E,0x48},
{0x38,0xEA},
{0x42,0xA4},
{0x43,0x41},
{0x44,0x41},
{0x45,0x24},
{0x46,0xC0},
{0x52,0x32},
{0x53,0x28},
{0x56,0x60},
{0x57,0x28},
{0x6D,0x02},
{0x0F,0xC8},
{0x7F,0x00},//Switch bank
{0x5D,0x81},//Start ppg
};
#define INIT_PPG_REG_ARRAY_SIZE (sizeof(init_ppg_register_array)/sizeof(init_ppg_register_array[0]))


#define PXIALG_API

typedef enum {
    FLAG_DATA_READY = 0,
    FLAG_DATA_NOT_READY,
    FLAG_DATA_LOSS,
    FLAG_NO_TOUCH,
    FLAG_DATA_ERROR,
    FLAG_POOR_SIGNAL,
    FLAG_FIFO_ERROR,
} PXI_STATUS_FLAG;

/**
* @brief When HRD and MEMS data are ready, call this function to do the algorithm processing
* @param[in] HRD_Data Pointer to the buffer where HRD data (13 Bytes) is stored.
* @param[in] MEMS_Data Pointer to the buffer where MEMS data (3*sizeof(float) Bytes) is stored.
* @return one of the PXI_STATUS_FLAG types.
*/
PXIALG_API int32_t PxiAlg_Process(unsigned char *HRD_Data, float *MEMS_Data);

/**
* @brief Call this function to get Heart Rate
* @param[out] hr Pointer to a float variable where heart rate is stored
* @return None
*/
PXIALG_API void PxiAlg_HrGet(float *hr);

/**
* @brief Call this function to determine the version of the algorithm
* @return Version of the algorithm
*/
//Note: Recommend call PxiAlg_Version() function to get the version of the algorithm first. If return value is 1025, it means the version is 25th version. If return value is 0, it means unconnected the library.
PXIALG_API int32_t PxiAlg_Version(void) ;

/**
* @brief Call this function to get Ready_Flag
* @return Ready_Flag
*/
PXIALG_API unsigned char PxiAlg_GetReadyFlag(void);

/**
* @brief Call this function to get Motion_Flag
* @return Motion_Flag
*/
PXIALG_API unsigned char PxiAlg_GetMotionFlag(void);

/**
* @brief Call this function to notify algorithm the MEMS Scale of Motion Sensor
* @param[in] scale The MEMS Scale of Motion Sensor. Only 0(A+-2G), 1(A+-4G, A+-8G, A+-16G) are supported.
* @return 1 for success. 0 for failure.
*/
PXIALG_API uint8_t PxiAlg_SetMemsScale(int scale);

/**
* @brief Call this function to get PPG Signal Grade. High signal grade is meant high signal at steady state.
* @param[out] grade Pointer to a float variable where signal grade is stored.
* @return Return 1 when Signal Grade is ready. Otherwise, return 0.
*/
PXIALG_API uint8_t PxiAlg_GetSigGrade(float *grade);

/**
* @brief Call this function to set PPG Signal Grade Threshold. Default Signal Grade Threshold is 40.
* @param[in] threshold The PPG Signal Grade Threshold. Its value ranges from 0 to 100.
* @return 1 for success. 0 for failure.
*/
PXIALG_API uint8_t PxiAlg_SetSigGradeThrd(float thrd);

/**
* @brief Call this function to enable or disable fast output mode
* @param[in] enable The flag of fast output mode.
* @when enable fast output mode, 1st HR value is output about 6 sec.
* @For getting high accuracy, please don't move in the beginning.
*/
PXIALG_API void PxiAlg_EnableFastOutput(uint8_t en);

/**
* @brief Call this function to enable or disable motion mode. Default is disable motion mode.
* @param[in] enable The flag of motion mode.
* @enable motion mode for whole body movement: such as running
* @disable motion mode for parts body movement: such as car driving, weight lifting, slow walking, bicycling
*/
PXIALG_API void PxiAlg_EnableMotionMode(uint8_t en);

/**
* @brief Call this function to enable or disable auto mode. Default is enable auto mode.
* @param[in] enable The flag of auto mode.
*/
PXIALG_API void PxiAlg_EnableAutoMode(uint8_t en);

/**
* @brief Call this function to open algorithm
*/
PXIALG_API void PxiAlg_Open(void);

/**
* @brief Call this function to close/reset algorithm
*/
PXIALG_API void PxiAlg_Close(void);

/**
* @brief Call this function to get the alarm flag of fast output
* @param[out] get the alarm flag of fast output when signal is poor during 60 seconds in the beginning.
* @return Return 1 when the flag is set. Otherwise, return 0.
*/
PXIALG_API uint8_t PxiAlg_GetFastOutAlarmFlag(void);

/**
* @brief Call this function to enable or disable mems0 signal grade mode for factory testing
* @param[in] 1 : enable. 0 : disable.
*/
PXIALG_API void PxiAlg_EnableMEMS0SigGrade(uint8_t en);

/**
* @brief Call this function to get the alarm flag of signal bad
* @param[out] get the alarm flag of bad signal
* @return Return 1 when the flag is set. Otherwise, return 0.
* @when get this flag=1, host could reset algorithm and stop heart calculation.
*/
PXIALG_API uint8_t PxiAlg_GetSignalBadAlarm(void);

/**
* @brief Call this function to set PPG buffer pointer and size
* @param[in] ppg_buffer Pointer to a float buffer where the PPG data would be stored
* @param[in] size PPG buffer size
* @return None
*/
PXIALG_API void PxiAlg_SetPPGBuffer(float *ppg_buffer, int32_t size);

#ifdef __cplusplus
}
#endif

#endif // PAH8001SET_H
