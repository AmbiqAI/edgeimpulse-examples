/**
    ******************************************************************************
    * @file     ADPDDrv.c
    * @author   ADI
    * @version  V1.4.1
    * @date     2-Apr-2015
    * @brief    Reference design device driver to access ADI ADPD chip.
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
* This software is intended for use with the ADPD and derivative parts        *
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
* Version 1.2, Nov 21, 2014                                                   *
*   Created MCU_HAL_I2C_TxRx() and use it here                                *
*   Added set to Idle mode in Open/close driver                               *
*   Enable to read FIFO from ISR                                              *
* Version 1.3, Jan 28, 2015                                                   *
*   Fixed bug with time stamp and FIFO watermark                              *
* Version 1.4, Feb 28, 2015                                                   *
*   Added device reset and FIFO overflow indicator in debug mode              *
* Version 1.4.1, Apr 2, 2015                                                  *
*   Fixed bug with timestamp when the watermark is greater than 1             *
******************************************************************************/

/* Includes ---------------------------------------------------------------- */
#include "stdint.h"
#include "ADPDDrv.h"
#include "common.h"

/* Public variables -------------------------------------------------------- */

/* Private define ---------------------------------------------------------- */
//#define AdpdISR(s)                    HAL_GPIO_EXTI_Callback(s)
#define RX_BUFFER_SIZE                  20            // must > 8
#define MAXDATASETSIZE                  SLOT_AB_DATA_SIZE // maximum dataset size

#define FIFO_WATERMARK                  (4)

//#define POWER_SAVING                  // Disable FIFO clock when no access.
//#define USE_SAMPLE_IRQ                // use Sample IRQ instead of FIFO IRQ
//#define DEBUG_MODE_LED                // use LED for debugging

#ifndef DEBUG_MODE_LED
#define BSP_LED_On(s)
#define BSP_LED_Off(s)
#define BSP_LED_Toggle(s)
#endif

/** @struct Rx_Buffer_Def_Struct
 *  @brief Received data's Ring Buffer Structure
 *  @var Rx_Buffer_Def_Struct::DataValue
 *  Data contained in the current buffer
 *  @var Rx_Buffer_Def_Struct::Next
 *  Pointer to next buffer structure
 */
typedef struct Rx_Buffer_Def_Struct {
    uint8_t DataValue[MAXDATASETSIZE];
    uint32_t TimeStamp;
    struct Rx_Buffer_Def_Struct* Next;
} RX_BUFFER_DEF;

/* Public function prototypes ---------------------------------------------- */
uint32_t* ADPDDrv_GetDebugInfo(void);
void ADPDDrv_SetIsrReadDataFromFIFO(uint8_t enable_set);

/* Private variables ------------------------------------------------------- */
static uint8_t DataSetSize;
static RX_BUFFER_DEF RxBuff[RX_BUFFER_SIZE];
static RX_BUFFER_DEF *WtBufferPtr, *RdBufferPtr;
static uint8_t IRQHappen, AdpdDataReady, SlotMode;
static uint32_t TimeInitVal, TimeCurVal, TimePreVal;
static uint8_t TimeGap=10, TimeIrqSet, TimeIrqSet1, IsrReadFIFO;
static uint32_t AccessCnt[5];
#ifndef NDEBUG
static uint32_t overflow_cnt = 0;
#endif //NDEBUG
/* Private function prototypes --------------------------------------------- */
static void RxBufferInit(void);
static uint16_t RxBufferRemoveData(uint16_t *rdData, uint32_t *time);
static uint16_t RxBufferInsertData(uint32_t tcv);
static void AdpdDrv_Init(void);
static int16_t SetAdpdIdleMode(void);
static int16_t ADPDDrv_ReadFifoData(uint8_t pollMode);

/*****************************************************************************/
/*****************************************************************************/
/**
    * @brief    Open Driver, setting up the interrupt and I2C lines
    * @param    none
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADPDDrv_OpenDriver() {

#ifndef NDEBUG
    overflow_cnt = 0;
#endif //NDEBUG
    SetAdpdIdleMode();
    AdpdDrv_Init();
    RxBufferInit();

    return ADPDDrv_SUCCESS;
}

/**
    * @brief    Close Driver. Clear up before existing
    * @param    none
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADPDDrv_CloseDriver(void) {

#ifndef NDEBUG
    debug("FIFO Full = %i\r\n", overflow_cnt);
#endif //NDEBUG
    return SetAdpdIdleMode();
}

/**
    * @brief    Synchronous register write to the ADPD
    * @param    16-bit register address
    * @param    16-bit register data value
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADPDDrv_RegWrite(uint16_t addr, uint16_t regValue) {
    uint8_t i2cData[3];
    i2cData[0] = (uint8_t)addr;
    i2cData[1] = (uint8_t)(regValue>>8);
    i2cData[2] = (uint8_t)(regValue);
    /**
    * The first argument to the function MCU_HAL_I2C_Transmit is the register 
    * address of the ADPD device and the 16 bits data value to be written to the 
    * device register. 
    * The 1st argument to the function MCU_HAL_I2C_Transmit is the pointer to the 
    * buffer of the size of three bytes in which first byte is the register 
    * address of the ADPD device.
    * The second and the third bytes are the 16 bits data value to be written 
    * to the device register 
    * The 2nd argument is the size of the buffer in bytes (3 bytes).
    * The 3rd argument is the timeout duration which is typically 100ms.
    * MCU_HAL_I2C_Transmit() should be implemented in such a way that it transmits
    * the data from i2cData buffer of size specified in the second argument. 
    */
    if (MCU_HAL_I2C_Transmit((uint8_t*)i2cData, 3, 200)!= ADI_HAL_OK) {
        return ADPDDrv_ERROR;
    }

    return ADPDDrv_SUCCESS;
}

/**
    * @brief    Synchronous register read from the ADPD
    * @param    16-bit register address
    * @param    pointer to a 16-bit register data value
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADPDDrv_RegRead(uint16_t addr, uint16_t *data) {
    uint8_t regAddr, rxData[2];
    regAddr = (uint8_t)addr;
    /**
    * The first argument to the function is the the register address of the 
    * ADPD device from where the data is to be read. 
    * The 2nd argument is the pointer to the buffer of received data. 
    * The size of this buffer should be equal to the number of data requested.
    * The 3rd argument is the size of requested data in bytes.
    * The 4th argument is the timeout duration which is typically 100ms.
    * MCU_HAL_I2C_TxRx() should be implemented in such a way that it transmits
    * the register address from the first argument and receives the data 
    * specified by the address in the second argument. The received data will
    * be of size specified by 3rd argument.  
    */
    if (MCU_HAL_I2C_TxRx(&regAddr, (uint8_t*)rxData, 2, 100)!= ADI_HAL_OK) {
        return ADPDDrv_ERROR;
    }

    *data = (rxData[0]<<8) + rxData[1];
    return ADPDDrv_SUCCESS;
}

/**
    * @brief    Set Adpd operating mode, clear FIFO if needed
    * @param    8-bit operating mode
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADPDDrv_SetOperationMode(uint8_t opmode) {
    int16_t retCode = ADPDDrv_SUCCESS;
    uint16_t regValue1, regValue2;

    retCode = ADPDDrv_RegWrite(REG_FIFO_CLK, FIFO_CLK_EN);      // set clock ON
    if (opmode == ADPDDrv_MODE_IDLE) {
        retCode = SetAdpdIdleMode();
        AdpdDrv_Init();
    } else if (opmode == ADPDDrv_MODE_PAUSE) {
        retCode = ADPDDrv_RegWrite(REG_OP_MODE, OP_PAUSE_MODE);    // set Pause
    } else if (opmode == ADPDDrv_MODE_SAMPLE) {
#if USE_SAMPLE_IRQ
        if (SlotMode == ADPDDrv_SLOT_A)
            retCode = ADPDDrv_RegWrite(REG_INT_MASK, IRQ_RAW_A_EN);
        else if (SlotMode == ADPDDrv_SLOT_B)
            retCode = ADPDDrv_RegWrite(REG_INT_MASK, IRQ_RAW_B_EN);
        else
            retCode = ADPDDrv_RegWrite(REG_INT_MASK, IRQ_RAW_AB_EN);
#else
        retCode = ADPDDrv_RegWrite(REG_I2CS_CTL_MATCH, (((FIFO_WATERMARK * DataSetSize)-1)<<7)|1);
#endif
        retCode |= ADPDDrv_RegRead(REG_SAMPLING_FREQ, &regValue1);
        retCode |= ADPDDrv_RegRead(REG_DEC_MODE, &regValue2);
        regValue2 = (regValue2&0xF0)>>4;
        regValue2 = 1<<regValue2;
        TimeGap = (uint8_t)(regValue1*regValue2/8);
        retCode |= ADPDDrv_RegWrite(REG_OP_MODE, OP_PAUSE_MODE);    // set Pause
        retCode |= ADPDDrv_RegWrite(REG_FIFO_CLK, FIFO_CLK_EN);     // enable FIFO clock
        TimeInitVal = 0;
        retCode |= ADPDDrv_RegWrite(REG_OP_MODE, OP_RUN_MODE);      // set GO
    } else {
        retCode = ADPDDrv_ERROR;
    }

#ifdef POWER_SAVING
    retCode |= ADPDDrv_RegWrite(REG_FIFO_CLK, FIFO_CLK_DIS);        // set clock OFF
#endif

    for (uint8_t i=0; i<5; i++)
        AccessCnt[i] = 0;
    return retCode;
}

/**
    * @brief    Select operation time slot
    * @param    8-bit time slot
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADPDDrv_SelectSlot(uint8_t slot) {
    uint16_t regValue;

    if (ADPDDrv_RegRead(REG_OP_MODE_CFG, &regValue) != ADPDDrv_SUCCESS)
        return ADPDDrv_ERROR;
    SlotMode = slot;
    regValue &= SLOT_MASK;
    if (slot == ADPDDrv_SLOT_A) {
        regValue |= SLOT_A_MODE;
        ADPDDrv_RegWrite(REG_OP_MODE_CFG, regValue);
        DataSetSize = SLOT_A_DATA_SIZE;
    } else if (slot == ADPDDrv_SLOT_B) {
        regValue |= SLOT_B_MODE;
        ADPDDrv_RegWrite(REG_OP_MODE_CFG, regValue);
        DataSetSize = SLOT_B_DATA_SIZE;
    } else {
        regValue |= SLOT_AB_MODE;
        ADPDDrv_RegWrite(REG_OP_MODE_CFG, regValue);
        DataSetSize = SLOT_AB_DATA_SIZE;
    }
    return ADPDDrv_SUCCESS;
}

uint32_t ADPDDrv_GetRefTime() {

    return TimeInitVal;
}

/**
    * @brief ADPD interrupt service routine
    * @param GPIO_Pin: Specifies the pins connected Interrupt line
    * @retval None
    */
void AdpdISR(uint16_t GPIO_Pin) {

    TimeCurVal = MCU_HAL_GetTick();
    TimeIrqSet = 1;
    TimeIrqSet1 = 1;
    AdpdDataReady = 1;
    IRQHappen = 1;

    BSP_LED_Toggle(LED6);
    if (IsrReadFIFO == 1) {
        if (ADPDDrv_ReadFifoData(1) == ADPDDrv_ERROR)
            AccessCnt[4]++;
    }
    AccessCnt[0]++;
}

/**
    * @brief    Read data out from Adpd FIFO
    * @param    pollMode=1=continue polling.
    * @param    rxData = pointer to 16-bit data that is read from FIFO
    * @param    time = time stamp
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADPDDrv_ReadData(uint8_t pollMode, uint16_t *rxData, uint32_t *time) {
    if (ADPDDrv_ReadFifoData(pollMode) != ADPDDrv_SUCCESS)
        return ADPDDrv_ERROR;

    return RxBufferRemoveData(rxData, time);
}

/**
    * @brief    Debug function. Read out debug info
    * @param    none
    * @retval   debug info pointer
    */
uint32_t* ADPDDrv_GetDebugInfo() {
    return AccessCnt;
}

/**
    * @brief    Enable/Disable reading data out from Adpd FIFO to soft buffer
    *           Donot set this switch unless you clear on what you are doing
    * @param    1=enable, 0=disable
    * @retval   none
    */
void ADPDDrv_SetIsrReadDataFromFIFO(uint8_t enable_set) {
    if (enable_set == 0)
        IsrReadFIFO = 0;
    else
        IsrReadFIFO = 1;
}

/**
    * @brief    Driver Initialization.
    * @param    none
    * @retval none
    */
static void AdpdDrv_Init(void) {

    AdpdDataReady = 0;
    IRQHappen = 0;
    TimeIrqSet = 0;
    TimeIrqSet1 = 0;
    TimeGap = 10;
    SlotMode = ADPDDrv_SLOT_A;
    DataSetSize = SLOT_A_DATA_SIZE;
    IsrReadFIFO = 0;

    WtBufferPtr = RdBufferPtr;
    BSP_LED_Off(LED3);
    BSP_LED_Off(LED4);
    BSP_LED_Off(LED5);
    BSP_LED_Off(LED6);
}


/**
    * @brief    Read data out from Adpd FIFO
    * @param    pollMode 1=continue polling.
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
static int16_t ADPDDrv_ReadFifoData(uint8_t pollMode) {
    uint16_t statData;
    uint8_t fifoLevel, tempFifoLevel, getFifoData;
    uint32_t tcv;

    
		if (pollMode == 0) {
        if (AdpdDataReady != 1)
            return ADPDDrv_ERROR;
    } else {
        AdpdDataReady = 1;
    }

    ADPDDrv_RegRead(REG_INT_STATUS, &statData);
    fifoLevel = statData>>8;
   		
		tempFifoLevel = fifoLevel;
    AccessCnt[1]++;

#ifndef NDEBUG
    if (tempFifoLevel >= 128) {

        overflow_cnt++;
				debug ("Overflow %d\r\n", overflow_cnt);
    }
#endif //NDEBUG

    if(tempFifoLevel) {
       tcv = MCU_HAL_GetTick();
	   if(tcv > TimeCurVal)
	      TimeCurVal += (((tcv-TimeCurVal)/TimeGap)*TimeGap);
       tcv = TimeCurVal - (((tempFifoLevel/DataSetSize)*TimeGap)-TimeGap);
    }

#ifdef POWER_SAVING
    if (tempFifoLevel>=DataSetSize) {
        getFifoData = 1;
        if (ADPDDrv_RegWrite(REG_FIFO_CLK, FIFO_CLK_EN) == ADPDDrv_ERROR)      // enable FIFO clock
            return ADPDDrv_ERROR;
        if (ADPDDrv_RegWrite(REG_FIFO_CLK, FIFO_CLK_EN) == ADPDDrv_ERROR)      // enable FIFO clock twice
            return ADPDDrv_ERROR;
    } else {
        getFifoData = 0;
    }
#endif
    TimeIrqSet1 = 0;
    while (tempFifoLevel>=DataSetSize) {
        AccessCnt[2]++;
        if (RxBufferInsertData(tcv) != ADPDDrv_SUCCESS) {
            break;
        }
        tempFifoLevel -= DataSetSize;
    }

#if USE_SAMPLE_IRQ
    if ((statData&IRQ_MASK_RAW) != 0) {
        ADPDDrv_RegWrite(REG_INT_STATUS, statData&IRQ_MASK_RAW);
    }
#endif


#ifdef POWER_SAVING
    if (getFifoData == 1) {
        ADPDDrv_RegWrite(REG_FIFO_CLK, FIFO_CLK_DIS);                   // disable FIFO clock
    }
#endif

    return ADPDDrv_SUCCESS;
}

/**
    * @brief    Read all Efuse Register
    * @param    pointer to a 16-bit register data value
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
int16_t ADPDDrv_EfuseRead(uint16_t *reg_values, uint8_t size) {
    uint8_t i;
    uint16_t reg_value, clk32K, clkfifo;

    ADPDDrv_RegRead(REG_OSC32K, &clk32K);
    ADPDDrv_RegWrite(REG_OSC32K, clk32K | 0x80);

    MCU_HAL_Delay(10);				//Need to check whether this delay is sufficient

    ADPDDrv_RegRead(REG_FIFO_CLK, &clkfifo);
    ADPDDrv_RegWrite(REG_FIFO_CLK, clkfifo|1);

    ADPDDrv_RegWrite(REG_EFUSE_CTRL, 0x0003);

    ADPDDrv_RegRead(REG_EFUSE_STATUS0, &reg_value);
    ADPDDrv_RegRead(REG_EFUSE_STATUS0, &reg_value);

    i = 0;
    if ((reg_value&0x7) == 0x4) {
        for (; i<size; i++)
            ADPDDrv_RegRead(0x70+i, &reg_values[i]);
    }

    ADPDDrv_RegWrite(REG_EFUSE_CTRL, 0x0);
    ADPDDrv_RegWrite(REG_OSC32K, clk32K);
    ADPDDrv_RegWrite(REG_FIFO_CLK, clkfifo);

    if (i==0)
        return ADPDDrv_ERROR;

    return ADPDDrv_SUCCESS;
}

int16_t ADPDDrv_EfuseModuleTypeRead(uint16_t *moduletype){
    return ADPDDrv_EfuseRead(moduletype, 1);
}

uint8_t* ADPDDrv_EfuseModuleNameRead(){

    uint16_t moduletype;

    ADPDDrv_EfuseModuleTypeRead(&moduletype);
    switch (moduletype) {
    case 1: return ("ADPD153GGRI");
    case 2: return ("ADPD163URI");
    case 3: return ("ADPD163URI_Rev2");
    case 4: return ("ADPD163URI_Rev3");
    case 5: return ("ADPD173GGI");
    default: return ("ADPD153GGRI");      //unknown device is detected/efuse not programmed
    }
}

int16_t ADPDDrv_SoftReset(){

    GPIO_IRQ_ADPD_Disable();

    GPIO_DUT_POWER_Off();
    MCU_HAL_Delay(500);
    GPIO_DUT_POWER_On();
    MCU_HAL_Delay(10);

    GPIO_IRQ_ADPD_Enable();

    return ADPDDrv_SUCCESS;
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
        for (i=0; i<RX_BUFFER_SIZE; i++) {
            RxBuff[i].Next = &RxBuff[i+1];
        }
        RxBuff[RX_BUFFER_SIZE-1].Next = &RxBuff[0];
}

/**
    * @brief    Move data from i2c FIFO to this Ring Buffer. If WtPtr reach RdPtr,
    *                 host's reading is too slow. Toggle LED to indicate this event.
    * @param    none
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
static uint16_t RxBufferInsertData(uint32_t tcv) {
    uint8_t addr;
    uint32_t time_s;

    if (WtBufferPtr->Next == RdBufferPtr) {
        BSP_LED_Toggle(LED4);               // Host is too slow, do nothing
        AccessCnt[3]++;
        return ADPDDrv_ERROR;
    }

    if (TimeIrqSet == 1 && TimeIrqSet1 == 0) {
        TimeIrqSet = 0;
        if (TimeCurVal > TimeInitVal)
            time_s = tcv - TimeInitVal;
        else
            time_s = TimeInitVal - tcv;
    } else {
        time_s = TimePreVal + TimeGap;
        //time_s = TimeGap;
    }
    TimePreVal = time_s;

    WtBufferPtr->TimeStamp = time_s;

    addr = REG_DATA_BUFFER;
    /**
    * The first argument to the function is the the register (FIFO) address 
    * of the ADPD device from where the data is to be read. 
    * The 2nd argument is the pointer to the buffer of received data. 
    * The size of this buffer should be equal to the number of data requested.
    * The 3rd argument is the size of requested data in bytes.
    * The 4th argument is the timeout duration which is typically 100ms.
    * MCU_HAL_I2C_TxRx() should be implemented in such a way that it transmits
    * the register address from the first argument and receives the data 
    * specified by the address in the second argument. The received data will
    * be of size DataSetSize.  
    */
    if (MCU_HAL_I2C_TxRx((uint8_t*)&addr, (uint8_t*)&(WtBufferPtr->DataValue), DataSetSize, 100)!= ADI_HAL_OK) {
        return ADPDDrv_ERROR;
    }

    WtBufferPtr = WtBufferPtr->Next;

    return ADPDDrv_SUCCESS;
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
        AdpdDataReady = 0;      // check again later to make sure it is true;
        temp = 1;
    }

    if (IRQHappen != 0) {
        AdpdDataReady = 1;      // IRQ just happened right before AdpdDataReady is set
        temp |= 2;
    }

    if (temp == 3) {
        BSP_LED_Toggle(LED5);
        temp = 0;
        return ADPDDrv_ERROR;           // take data in when next added to buffer
    }

    if (AdpdDataReady==0)
        return ADPDDrv_ERROR;
#else
    if (RdBufferPtr == WtBufferPtr)
    {
        /* Set AdpdDataReady to 1 if IRQ just happened else set it to 0 */
        AdpdDataReady = IRQHappen;
        /* Return error as there is no data in soft FIFO */
        return ADPDDrv_ERROR;
    }
#endif

    for (i=0; i<DataSetSize; i+=2)
    {
        *rxData = (RdBufferPtr->DataValue[i]<<8)+RdBufferPtr->DataValue[i+1];
        rxData++;
    }
    if (time != 0)
        *time = RdBufferPtr->TimeStamp;

    RdBufferPtr = RdBufferPtr->Next;
    IRQHappen = 0;

    return ADPDDrv_SUCCESS;
}

/**
    * @brief    Set device to Idle mode
    * @param    none
    * @retval a 16-bit integer:    0 - success; < 0 - failure
    */
static int16_t SetAdpdIdleMode() {
    int16_t retCode;
    retCode = ADPDDrv_RegWrite(REG_OP_MODE, OP_PAUSE_MODE); // set to Pause Mode
    retCode |= ADPDDrv_RegWrite(REG_FIFO_CLK, FIFO_CLK_EN); // set clock ON
    retCode |= ADPDDrv_RegWrite(REG_INT_STATUS, FIFO_CLR|IRQ_CLR_ALL);
    retCode |= ADPDDrv_RegWrite(REG_OP_MODE, OP_IDLE_MODE); // set to Idle Mode
    retCode |= ADPDDrv_RegWrite(REG_FIFO_CLK, FIFO_CLK_DIS);// disable FIFO clock
    return retCode;
}
