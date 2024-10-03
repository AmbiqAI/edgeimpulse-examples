/* Includes ---------------------------------------------------------------- */
#include <stdint.h>
#include <assert.h>
#include "bmi160.h"
#include "AM_BMI160.h"
#include "Common.h"
#include "am_mcu_apollo.h"


/* Public variables -------------------------------------------------------- */

/* Private define ---------------------------------------------------------- */
#define RX_BUFFER_SIZE                  (3*40)
#define MAXDATASETSIZE                  (3)

extern struct bmi160_accel_t accel_fifo[FIFO_FRAME_CNT];
extern struct bmi160_t bmi160;



/** @struct Rx_Buffer_Def_Struct
 *  @brief Received data's Ring Buffer Structure
 *  @var Rx_Buffer_Def_Struct::DataValue
 *  Data contained in the current buffer
 *  @var Rx_Buffer_Def_Struct::Next
 *  Pointer to next buffer structure
 */
typedef struct Rx_Buffer_Def_Struct {
    int16_t DataValue[MAXDATASETSIZE];
    uint32_t TimeStamp;
    struct Rx_Buffer_Def_Struct* Next;
} RX_BUFFER_DEF;

static int16_t X_data_array[171], Y_data_array[171], Z_data_array[171];

/* Public function prototypes ---------------------------------------------- */
int16_t BMIDrv_OpenDriver(uint8_t inrate, uint8_t watermark);
int16_t BMIDrv_CloseDriver(void);
void BmiISR(uint16_t);
int16_t BMIDrv_RegRead(uint16_t addr, uint16_t *data);
int16_t BMIDrv_RegWrite(uint16_t addr, uint16_t data);
int16_t BMIDrv_ReadData(uint8_t, int16_t *data, uint32_t *time);
int16_t BMIDrv_SetOperationMode(uint8_t opmode);
int16_t BMIDrv_SelectSlot(uint8_t slot);
int16_t BMIDrv_EfuseRead(uint16_t *reg_values, uint8_t size);

/* Private variables ------------------------------------------------------- */
static uint8_t DataSetSize;
static RX_BUFFER_DEF RxBuff[RX_BUFFER_SIZE];
static RX_BUFFER_DEF *WtBufferPtr, *RdBufferPtr;
volatile static uint8_t IRQHappen, BMIDataReady;
volatile static uint32_t TimeCurVal, TimePreVal;
volatile static uint8_t TimeGap=10, TimeIrqSet, TimeIrqSet1;

/* Private function prototypes --------------------------------------------- */
static void RxBufferInit(void);
static uint16_t RxBufferRemoveData(int16_t *rdData, uint32_t *time);
static uint16_t RxBufferInsertData(uint32_t index, uint32_t tcv);
static void BmiDrv_Init(void);

/*****************************************************************************/
/*****************************************************************************/

static int16_t readBMI160FIFO(uint8_t fifo_bytes,
                        int16_t *X_data_array,
                        int16_t *Y_data_array,
                        int16_t *Z_data_array) {

    //uint8_t bytes_per_sample = 6;
    //uint16_t fifo_lenght_bytes = bytes_per_sample * fifo_samples;

    getFIFOXYZ(fifo_bytes,X_data_array,Y_data_array,Z_data_array);
    return 0;
}

// Function to check the samples in the FIFO
static uint16_t checkSamplesInFIFO() {

    return getFIFO_ENTRIES_BMI160();
}

static int32_t BMI160_SetON(){

  //setPowerControl_BMI160(powerCtlWord);

  return 0;
}

static int32_t BMI160_SetOFF(){

  //setPowerControl_BMI160(powerCtlWord&0xFC);

  return 0;
}

static int32_t BmiDeviceInit(uint8_t inrate, uint8_t watermark) {

    GPIO_IRQ_BMI160_Disable();
    BMIDataReady = 0;
	uint8_t array[2];

    //
    // Initialize structure.
    //
    bmi160.burst_read = BMI160_SPI_Receive_Burst_Driv;
    bmi160.bus_write = BMI160_SPI_Transmit_Driv;
    bmi160.bus_read = BMI160_SPI_Receive_Driv;
    bmi160.delay_msec = MCU_HAL_Delay;
    bmi160.dev_addr = 0xD1;
    bmi160_init(&bmi160);

    setSOFT_RESET_BMI160();							//BMI160 software reset.
    MCU_HAL_Delay(200);

    //
    // Power on the accel.
    //
    while(1)
    {
        bmi160_set_command_register(ACCEL_MODE_NORMAL);
        MCU_HAL_Delay(50);
        bmi160.bus_read(bmi160.dev_addr, 0x3, array, 1);
        if (array[0] != 0)
        {
            break;
        }
    }

    //
    // Set the sensitivity to 8G.
    //
    bmi160_set_accel_range(C_BMI160_EIGHT_U8X);

    //
    // Set the output data rate to 50 Hz.
    //
    bmi160_set_accel_output_data_rate(BMI160_ACCEL_OUTPUT_DATA_RATE_50HZ);

    //
    // FIFO watermark.
    //
    array[0] = (0x7 >> 2);
    bmi160.bus_write(bmi160.dev_addr, 0x46, array, 1);

    //
    // Enable interrupts on INT1.
    //
    array[0] = 0x40;
    bmi160.bus_write(bmi160.dev_addr, 0x51, array, 1);
    array[0] = 0x40;
    bmi160.bus_write(bmi160.dev_addr, 0x56, array, 1);
    array[0] = 0x0B;
    bmi160.bus_write(bmi160.dev_addr, 0x53, array, 1);

    //
    // Enable Accel.
    //
    array[0] = 0x52;
    bmi160.bus_write(bmi160.dev_addr, 0x47, array, 1);

    //
    // Time gap = 20 as ODR = 50.
    //
    TimeGap = 20;

    //    if (rate == PRM_ODR_12_5)
    //        TimeGap = 80;
    //    else if (rate == PRM_ODR_25)
    //        TimeGap = 40;
    //    else if (rate == PRM_ODR_50)
    //        TimeGap = 20;
    //    else //if (rate == PRM_ODR_100)
    //        TimeGap = 10;

    GPIO_IRQ_BMI160_Enable();
    return 0;
}

/**
    * @brief    Open Driver, setting up the interrupt and I2C lines
    * @param    none
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t BMIDrv_OpenDriver(uint8_t inrate, uint8_t watermark) {

    BmiDeviceInit(inrate, watermark);
    /*Initialise variables after setting up the interrupt pin to prevent
    changing the value of these variables due to false interrupt*/
    BmiDrv_Init();
    RxBufferInit();
    BMI160_SetON();

    return BMIDrv_SUCCESS;
}

/**
    * @brief    Close Driver. Clear up before existing
    * @param    none
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t BMIDrv_CloseDriver() {

    BMI160_SetOFF();
    return BMIDrv_SUCCESS;
}

/**
    * @brief BMI interrupt service routine
    * @param GPIO_Pin: Specifies the pins connected Interrupt line
    * @retval None
    */
void BmiISR(uint16_t GPIO_Pin) {

    TimeCurVal = MCU_HAL_GetTick();
    TimeIrqSet = 1;
    TimeIrqSet1 = 1;
    BMIDataReady = 1;
    IRQHappen = 1;
}

/**
    * @brief    Read data out from Bmi FIFO
    * @param    pointer to 16-bit data that is read from FIFO
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t BMIDrv_ReadData(uint8_t pollMode, int16_t *rxData, uint32_t *time) {

    uint16_t fifoLevel;
    uint32_t tcv;

    fifoLevel = checkSamplesInFIFO();
		//debug("FIFO Level: %u\r\n", fifoLevel);
    if (BMIDataReady == 0)
        return BMIDrv_ERROR;

    if (fifoLevel != 0)
    {
        tcv = MCU_HAL_GetTick();
	    if(tcv > TimeCurVal)
			{
	       TimeCurVal += (((tcv-TimeCurVal)/TimeGap)*TimeGap);
			}
			tcv = TimeCurVal -((fifoLevel*TimeGap)-TimeGap);
			readBMI160FIFO(fifoLevel, X_data_array, Y_data_array, Z_data_array);

			TimeIrqSet1 = 0;

			RxBufferInsertData(0, tcv);

    }

    int16_t ret = RxBufferRemoveData(rxData, time);

    return ret;
}

/**
    * @brief    Driver Initialization.
    * @param    none
    * @retval none
    */
static void BmiDrv_Init(void) {

    TimePreVal = 0;
    TimeCurVal = 0;
    BMIDataReady = 0;
    IRQHappen = 0;
    TimeIrqSet = 0;
    TimeIrqSet1 = 0;
    DataSetSize = MAXDATASETSIZE;
}


/**
    * @brief    Create a Ring Buffer to store received data.
    * @param    none
    * @retval none
    */
static void RxBufferInit(void) {

    uint8_t i;

    WtBufferPtr = &RxBuff[0];
    RdBufferPtr = &RxBuff[0];
    for (i=0; i<RX_BUFFER_SIZE; i++)
        RxBuff[i].Next = &RxBuff[i+1];
    RxBuff[RX_BUFFER_SIZE-1].Next = &RxBuff[0];
}

/**
    * @brief    Move data from i2c FIFO to this Ring Buffer. If WtPtr reach RdPtr,
    *                 host's reading is too slow. Toggle LED to indicate this event.
    * @param    none
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
static uint16_t RxBufferInsertData(uint32_t index, uint32_t tcv) {
    uint32_t time_s;

    if (WtBufferPtr->Next == RdBufferPtr) {
        return BMIDrv_ERROR;
    }

    if (TimeIrqSet == 1 && TimeIrqSet1 == 0) {
        time_s = tcv; //TimeCurVal;
        TimeIrqSet = 0;
    } else {
        time_s = TimePreVal + TimeGap;
        //time_s = TimeGap;
    }
    TimePreVal = time_s;

    WtBufferPtr->TimeStamp = time_s;
    WtBufferPtr->DataValue[0] = X_data_array[index];
    WtBufferPtr->DataValue[1] = Y_data_array[index];
    WtBufferPtr->DataValue[2] = Z_data_array[index];
    WtBufferPtr = WtBufferPtr->Next;

		//debug("%u %u %u\r\n", TimePreVal, TimeGap, tcv);

    return BMIDrv_SUCCESS;
}

/**
    * @brief    Read data out from Ring Buffer. If there's nothing, return ERROR
    * @param    pointer to 16-bit data that is read from Buffer
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
static uint16_t RxBufferRemoveData(int16_t *rxData, uint32_t *time) {
    uint8_t i;
    //uint8_t temp=0;

#if 0
    if (RdBufferPtr == WtBufferPtr) {
        BMIDataReady = 0;      // check again later to make sure it is true;
        temp = 1;
    }

    if (IRQHappen != 0) {
        BMIDataReady = 1;      // IRQ just happened right before BMIDataReady is set
        temp |= 2;
    }

    if (temp == 3) {
        temp = 0;
        return BMIDrv_ERROR;           // take data in when next added to buffer
    }

    if (BMIDataReady==0)
        return BMIDrv_ERROR;
#else
    if (RdBufferPtr == WtBufferPtr)
    {
        /* Set BMIDataReady to 1 if IRQ just happened else set it to 0 */
        BMIDataReady = IRQHappen;
        /* Return error as there is no data in soft FIFO */
        return BMIDrv_ERROR;
    }
#endif

    for (i=0; i<DataSetSize; i++)
    {
        *rxData = RdBufferPtr->DataValue[i];
        rxData++;
    }

    if (time != 0)
        *time = RdBufferPtr->TimeStamp;

    RdBufferPtr = RdBufferPtr->Next;
    IRQHappen = 0;

    return BMIDrv_SUCCESS;
}
