/**
    ******************************************************************************
    * @file     ADXLDrv.c
    * @author   ADI
    * @version  V1.3.1
    * @date     2-Apr-2015
    * @brief    Reference design device driver to access ADI ADXL chip.
    *                    LED3 is ON with any I2C error.
    *                    LED4 is toggling when Host is too slow to pickup FIFO data.
    ******************************************************************************
    * @attention
    ******************************************************************************
    */
/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2015 Analog Devices Inc.                                      *
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
* This software is intended for use with the ADXL and derivative parts     *
* only                                                                        *
*                                                                             *
******************************************************************************/

/******************************************************************************
* Version 0.9, Sept 24, 2014                                                  *
*   Change FIFO filled threshold                                              *
* Version 1.0, Oct 01, 2014                                                   *
*   Add efuse function                                                        *
* Version 1.1, Oct 29, 2014                                                   *
*   Add timestamp output                                                      *
* Version 1.2, Jan 28, 2014                                                   *
*   Fixed timestamp related bug                                               *
* Version 1.3, Mar  9, 2014                                                   *
*   Added setting of ODR and watermark at init				                  *
* Version 1.3.1, Apr 2, 2015                                                  *
*   Fixed bug with timestamp when the watermark is greater than 1             *
******************************************************************************/

/* Includes ---------------------------------------------------------------- */
#include <stdint.h>
#include <assert.h>
#include "ADXL362.h"
#include "Common.h"

/* Public variables -------------------------------------------------------- */

/* Private define ---------------------------------------------------------- */
#define RX_BUFFER_SIZE                  (3*40)
#define MAXDATASETSIZE                  (3)

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

static uint8_t powerCtlWord  = PRM_EXT_CLK_DISABLE      |			//External clock must be disabled
                               PRM_LOW_NOISE_NORMAL     |			//Normal operation (reset default). Current consumption is 1.8 uA (if ODR = 100 Hz & Vdd = 2v) in this mode.
                               PRM_WAKEUP_DISABLE       |			//Not necessary in this application.
                               PRM_AUTOSLEEP_ENABLE     |			//Acceleromter will go to sleep mode if inactivity is detected.
                               PRM_MEASURE_MEASURE_MODE ;			//Measurement mode must be enabled.


/* Public function prototypes ---------------------------------------------- */
int16_t ADXLDrv_OpenDriver(uint8_t inrate, uint8_t watermark);
int16_t ADXLDrv_CloseDriver(void);
void AdxlISR(uint16_t);
int16_t ADXLDrv_RegRead(uint16_t addr, uint16_t *data);
int16_t ADXLDrv_RegWrite(uint16_t addr, uint16_t data);
int16_t ADXLDrv_ReadData(uint8_t, int16_t *data, uint32_t *time);
int16_t ADXLDrv_SetOperationMode(uint8_t opmode);
int16_t ADXLDrv_SelectSlot(uint8_t slot);
int16_t ADXLDrv_EfuseRead(uint16_t *reg_values, uint8_t size);

/* Private variables ------------------------------------------------------- */
static uint8_t DataSetSize;
static RX_BUFFER_DEF RxBuff[RX_BUFFER_SIZE];
static RX_BUFFER_DEF *WtBufferPtr, *RdBufferPtr;
volatile static uint8_t IRQHappen, AdxlDataReady;
volatile static uint32_t TimeCurVal, TimePreVal;
volatile static uint8_t TimeGap=10, TimeIrqSet, TimeIrqSet1;

/* Private function prototypes --------------------------------------------- */
static void RxBufferInit(void);
static uint16_t RxBufferRemoveData(uint16_t *rdData, uint32_t *time);
static uint16_t RxBufferInsertData(uint32_t index, uint32_t tcv);
static void AdxlDrv_Init(void);

/*****************************************************************************/
/*****************************************************************************/

static int16_t readADXL362FIFO(uint8_t fifo_samples,
                        int16_t *X_data_array,
                        int16_t *Y_data_array,
                        int16_t *Z_data_array) {

    uint8_t bytes_per_sample = 6;
    uint16_t fifo_lenght_bytes = bytes_per_sample * fifo_samples;

    getFIFOXYZ(fifo_lenght_bytes,X_data_array,Y_data_array,Z_data_array);
    return 0;
}

// Function to check the samples in the FIFO
static uint16_t checkSamplesInFIFO() {

    uint8_t fifo_samples = getFIFO_ENTRIES_ADXL362()/3;

    return fifo_samples;
}

static int32_t ADXL362_SetON(){

  setPowerControl_ADXL362(powerCtlWord);

  return 0;
}

static int32_t ADXL362_SetOFF(){

  setPowerControl_ADXL362(powerCtlWord&0xFC);

  return 0;
}

static int32_t AdxlDeviceInit(uint8_t inrate, uint8_t watermark) {

    uint8_t rate;

    GPIO_IRQ_ADXL362_Disable();
    g_ADXLDataReady = 0;

    //FILTER CONTROL REGISTER (Address: 0x2C)
    uint8_t filterCtlWord = PRM_RANGE_8GEE           |			//Measurement Range Selection = +-8 g
                            PRM_EXT_SAMPLE_DISABLE;			//We are not going to use an external clock signal to control ADXL362 sampling.


    if (inrate <= 12) {

        filterCtlWord |= PRM_ODR_12_5;
    } else if (inrate <= 25) {

        filterCtlWord |= PRM_ODR_25;
    } else if (inrate <= 50) {

        filterCtlWord |= PRM_ODR_50;
    } else { //if (inrate <= 100)

        filterCtlWord |= PRM_ODR_100;
    }

    //ACTIVITY/INACTIVITY CONTROL REGISTER (Address: 0x27)     // Not used
    uint8_t AAICtlWord = PRM_LOOPLINK_LINKED_MODE         |     //This mode means the uC must read the status register to clean the interrrupts.
                         PRM_INACT_REF_REF_MODE   |			    //Inactivity detection function operates in referenced mode.
                         PRM_INACT_EN_DISABLE      |			//This parameter enables the inactivity (underthreshold) functionality.
                         PRM_ACT_REF_REF_MODE     |			    //Activity detection function operates in referenced mode.
                         PRM_ACT_EN_DISABLE        ;			//This parameter enables the activity (overthreshold) functionality.


    setSOFT_RESET_ADXL362();							//ADXL362 software reset.

    MCU_HAL_Delay(1000);

    uint8_t ADXL362Ad = getDEVID_AD_ADXL362();			//This function obtain the AD value. if this value is not 0xAD,  we have not read correctly the ADXL362, that means we should check the SPI communication, code, supply, connections...
    uint8_t ADXL362Mst = getDEVID_MST_ADXL362();		//This function obtain the MST value. if this value is not 0x1D,  we have not read correctly the ADXL362, that means we should check the SPI communication, code, supply, connections...
    uint8_t ADXL362Partid = getPARTID_ADXL362();		//This function obtain the ID value. if this value is not 0xF2,  we have not read correctly the ADXL362, that means we should check the SPI communication, code, supply, connections...

    //debug("AD, MST, PartID = %02X %02X %02X\r\n", ADXL362Ad, ADXL362Mst, ADXL362Partid);

    //This function sets the FIFO's parameters.
    setFIFOConf_ADXL362(PRM_FIFO_TEMP_DISABLE, // Disable the temperature measurement.
                        PRM_FIFO_MODE_STREAM, // Enable the FIFO in Stream mode.
                        3*watermark);       // The number of samples to store and FIFO watermark. 3 (axis X, axis Y and axis Z) * 4 (FiFOsamples) = 12.

    //This function configures INT1 interrupt pin.
    setINTMAP1_ADXL362( 	PRM_INT_LOW_ACTIVE_LOW     |			//This means if INT1 = 1 ==> no event, if INT1 = 0 ==> new event.
                                PRM_AWAKE_DISABLE           | 		//Awave interrupt must be disabled.
                                PRM_INACT_DISABLE 	    |			//Inactivity interrupt must be disabled.
                                PRM_ACT_DISABLE 	    |			//Activity interrupt must be disabled.
                                PRM_FIFO_OVERRUN_DISABLE    |			//FIFO overrrun interrupt must be disabled.
                                PRM_FIFO_WATERMARK_ENABLE   |			//FIFO watermark is enabled.
                                PRM_FIFO_READY_DISABLE      |			//FIFO ready interrupt must be disabled.
                                PRM_DATA_READY_DISABLE      );			//Data ready interrupt must be disabled.

    setFilter_ADXL362(filterCtlWord);						//This function configures the Filter Control Register

    setPowerControl_ADXL362(powerCtlWord&0xFC);				//This function configures the Power Control Register but the accelerometer must remain in standby.

    uint32_t fifoLevel = checkSamplesInFIFO();

    if (fifoLevel > 0)
        readADXL362FIFO(fifoLevel, X_data_array, Y_data_array, Z_data_array);

    rate = (getFilter_ADXL362()) & PRM_ODR_MASK;

    if (rate == PRM_ODR_12_5)
        TimeGap = 80;
    else if (rate == PRM_ODR_25)
        TimeGap = 40;
    else if (rate == PRM_ODR_50)
        TimeGap = 20;
    else //if (rate == PRM_ODR_100)
        TimeGap = 10;

    GPIO_IRQ_ADXL362_Enable();
    return 0;
}

/**
    * @brief    Open Driver, setting up the interrupt and I2C lines
    * @param    none
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADXLDrv_OpenDriver(uint8_t inrate, uint8_t watermark) {

    AdxlDeviceInit(inrate, watermark);
    /*Initialise variables after setting up the interrupt pin to prevent
    changing the value of these variables due to false interrupt*/
    AdxlDrv_Init();
    RxBufferInit();
    ADXL362_SetON();

    return ADXLDrv_SUCCESS;
}

/**
    * @brief    Close Driver. Clear up before existing
    * @param    none
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADXLDrv_CloseDriver() {

    ADXL362_SetOFF();
    return ADXLDrv_SUCCESS;
}

/**
    * @brief ADXL interrupt service routine
    * @param GPIO_Pin: Specifies the pins connected Interrupt line
    * @retval None
    */
void AdxlISR(uint16_t GPIO_Pin) {

    TimeCurVal = MCU_HAL_GetTick();
    TimeIrqSet = 1;
    TimeIrqSet1 = 1;
    AdxlDataReady = 1;
    IRQHappen = 1;
}

/**
    * @brief    Read data out from Adxl FIFO
    * @param    pointer to 16-bit data that is read from FIFO
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADXLDrv_ReadData(uint8_t pollMode, int16_t *rxData, uint32_t *time) {

    uint16_t fifoLevel;
    uint32_t tcv;
    uint32_t i;

    fifoLevel = checkSamplesInFIFO();
		//debug("FIFO Level: %u\r\n", fifoLevel);
    if (AdxlDataReady == 0)
        return ADXLDrv_ERROR;

    if (fifoLevel != 0)
    {
        tcv = MCU_HAL_GetTick();
	    if(tcv > TimeCurVal)
	       TimeCurVal += (((tcv-TimeCurVal)/TimeGap)*TimeGap);
	    tcv = TimeCurVal -((fifoLevel*TimeGap)-TimeGap);
        readADXL362FIFO(fifoLevel, X_data_array, Y_data_array, Z_data_array);

	TimeIrqSet1 = 0;
        for (i = 0; i < fifoLevel; i++)
            RxBufferInsertData(i, tcv);
    }

    int16_t ret = RxBufferRemoveData(rxData, time);

    return ret;
}

/**
    * @brief    Driver Initialization.
    * @param    none
    * @retval none
    */
static void AdxlDrv_Init(void) {

    TimePreVal = 0;
    TimeCurVal = 0;
    AdxlDataReady = 0;
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
        return ADXLDrv_ERROR;
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

    return ADXLDrv_SUCCESS;
}

/**
    * @brief    Read data out from Ring Buffer. If there's nothing, return ERROR
    * @param    pointer to 16-bit data that is read from Buffer
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
static uint16_t RxBufferRemoveData(uint16_t *rxData, uint32_t *time) {
    uint8_t i;
    //uint8_t temp=0;

#if 0
    if (RdBufferPtr == WtBufferPtr) {
        AdxlDataReady = 0;      // check again later to make sure it is true;
        temp = 1;
    }

    if (IRQHappen != 0) {
        AdxlDataReady = 1;      // IRQ just happened right before AdxlDataReady is set
        temp |= 2;
    }

    if (temp == 3) {
        temp = 0;
        return ADXLDrv_ERROR;           // take data in when next added to buffer
    }

    if (AdxlDataReady==0)
        return ADXLDrv_ERROR;
#else
    if (RdBufferPtr == WtBufferPtr)
    {
        /* Set AdxlDataReady to 1 if IRQ just happened else set it to 0 */
        AdxlDataReady = IRQHappen;
        /* Return error as there is no data in soft FIFO */
        return ADXLDrv_ERROR;
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

    return ADXLDrv_SUCCESS;
}

uint16_t ADXLDrv_SelfTest(){

  uint8_t i;
  uint32_t ltck;
  int16_t xclData[3];
  int16_t xclData_ST[3];
  int16_t xclScaledDiff[3];

  uint8_t filterCtlWord = PRM_RANGE_8GEE           |			//Measurement Range Selection = +-8 g
                          PRM_EXT_SAMPLE_DISABLE   |			//We are not going to use an external clock signal to control ADXL362 sampling.
                          PRM_ODR_100              ;			//ODR = 100 Hz

  setSOFT_RESET_ADXL362();							//ADXL362 software reset.
  MCU_HAL_Delay(10);

  uint8_t ADXL362Ad = getDEVID_AD_ADXL362();			//This function obtain the AD value. if this value is not 0xAD,  we have not read correctly the ADXL362, that means we should check the SPI communication, code, supply, connections...
  uint8_t ADXL362Mst = getDEVID_MST_ADXL362();		//This function obtain the MST value. if this value is not 0x1D,  we have not read correctly the ADXL362, that means we should check the SPI communication, code, supply, connections...
  uint8_t ADXL362Partid = getPARTID_ADXL362();		//This function obtain the ID value. if this value is not 0xF2,  we have not read correctly the ADXL362, that means we should check the SPI communication, code, supply, connections...

  debug("\r\nAD: Expected value = 0xAD, Read value = 0x%02X\r\n", ADXL362Ad);
  debug("MST: Expected value = 0x1D, Read value = 0x%02X\r\n", ADXL362Mst);
  debug("PartID: Expected value = 0xF2, Read value = 0x%02X\r\n", ADXL362Partid);

  if((ADXL362Ad != 0xAD) || (ADXL362Mst != 0x1D) || (ADXL362Partid != 0xF2)){
    debug("\r\n:FAIL. Expected and read values do not match\r\n");
    return 1;
  }
  else{
    debug("\r\n:PASS. Expected and read values match\r\n");
  }

  debug("\r\nPerforming ADXL self test routine...\r\nPlease do not move the device.\r\n");
  MCU_HAL_Delay(2000);

  setFilter_ADXL362(filterCtlWord);						//This function configures the Filter Control Register

  setPowerControl_ADXL362(PRM_EXT_CLK_DISABLE      |			//External clock must be disabled
                          PRM_LOW_NOISE_NORMAL     |			//Normal operation (reset default). Current consumption is 1.8 uA (if ODR = 100 Hz & Vdd = 2v) in this mode.
                          PRM_WAKEUP_DISABLE       |			//Not necessary in this application.
                          PRM_MEASURE_MEASURE_MODE );				//Run mode.

   for(i=0;i<8;i++){                                                    //Wait for output to settle
     //ltck = MCU_HAL_GetTick();
     while((getSTATUS_ADXL362() & PRM_DATA_READY) != PRM_DATA_READY);
     //debug("%d\r\n", MCU_HAL_GetTick() - ltck);
     getXYZdata12B_ADXL362(&xclData[0],&xclData[1],&xclData[2]);
   }

  setSelfTest_ADXL362(PRM_ST_ENABLE);
  for(i=0;i<8;i++){                                                     //Wait for output to settle
    //ltck = MCU_HAL_GetTick();
    while((getSTATUS_ADXL362() & PRM_DATA_READY) != PRM_DATA_READY);
    //debug("%d\r\n", MCU_HAL_GetTick() - ltck);
    getXYZdata12B_ADXL362(&xclData_ST[0],&xclData_ST[1],&xclData_ST[2]);
  }

  setSelfTest_ADXL362(PRM_ST_DISABLE);
  setPowerControl_ADXL362(0);				//Standby mode

  debug("%-20s%15s%15s%15s","\r\nAxis","X","Y","Z\r\n");
  debug("%-20s","\r\nInitial values");

  for (i=0; i<3; i+=1){
    debug("%15d", xclData[i]);
  }

  debug("%-20s","\r\nTest values");

  for (i=0; i<3; i+=1){
    debug("%15d", xclData_ST[i]);
  }

  debug("%-20s","\r\nScaled difference");

  for (i=0; i<3; i+=1){
    xclScaledDiff[i] = (int32_t) ((xclData_ST[i] - xclData[i]) * 4 );    //Multiply by 4 for sensitivity
    debug("%15d", xclScaledDiff[i]);
  }

  debug("%-20s","\r\nExpected range");
  debug("%15s%15s%15s\r\n","200:2800","-2800:+200","200:2800");

  if((xclScaledDiff[0] > 2800) || (xclScaledDiff[0] < 200) ||
     (xclScaledDiff[1] > 200) || (xclScaledDiff[1] < -2800) ||
       (xclScaledDiff[2] > 2800)  || (xclScaledDiff[2] < 200)){
         debug("\r\nFAIL: ADXL self test failed\r\n");
         return 1;
       }
      else{
        debug("\r\nPASS. ADXL self test pass\r\n");
        return 0;
      }
}
