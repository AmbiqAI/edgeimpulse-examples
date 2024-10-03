
#include "AM_BMI160.h"
#include "Common.h"
#include <stdbool.h>
#include "bmi160.h"

#define SPI_BUFFER_LEN        1024
#define SPI_READ                          0x80;
#define SPI_WRITE                         0x00
struct bmi160_t bmi160;
volatile bool g_bDataReady;

extern struct bmi160_accel_t accel_fifo[FIFO_FRAME_CNT];

uint8_t g_BMIDataReady;
/******************************************************************************/
/*                    FUNCTIONS                                               */
/******************************************************************************/


///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getDEVID_AD_BMI160()                                   //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t) BMI160's Analog Device device ID              //
//    DESCRIPTION:   This function returns the Analog Devices device ID.     //
//                   This value must be 0xAD.                                //
//    EXAMPLE:       uint8_t ID_AD_BMI160 = getDEVID_AD_BMI160();          //
///////////////////////////////////////////////////////////////////////////////
uint8_t getDEVID_AD_BMI160(void)
{
      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]         =     {COMMAND_READ_REG,REG_DEVID_AD} ;
      uint8_t        pRxData[3];
      uint8_t        value;

      /**
      * The first argument to the function BMI160_SPI_Receive is the address of
      * the data read command and the register address of the BMI device from
      * where the data is to be read.
      * The 2nd argument is the pointer to the buffer of received data.
      * The size of transmit and receive buffer should be equal to the sum of
      * data trasmitted and received.
      * The 3rd argument is the size of the trasmitted and received data in bytes
      * BMI160_SPI_Receive() should be implemented in such a way that it transmits
      * the pTxData buffer while saving the received data to pRxData buffer. Both
      * the tranmit and receive data are of size totalLength.
      * Since tranmitted data are two bytes, the recevied data will be available
      * from the third memory location.
      */
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[2];
      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getDEVID_MST_BMI160()                                  //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t) BMI160's MEMs device ID                       //
//    DESCRIPTION:   This function returns an Analog Devices MEMS device ID  //
//                   This value must be 0x1D.                                //
//    EXAMPLE:       uint8_t ID_MEM_BMI160 = getDEVID_MST_BMI160();        //
///////////////////////////////////////////////////////////////////////////////
uint8_t getDEVID_MST_BMI160(void)
{
      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[2]         =     {COMMAND_READ_REG,REG_DEVID_MST} ;
      uint8_t        pRxData[3];
      uint8_t        value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[2];
      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME:  getPARTID_BMI160()                                    //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t) BMI160's device ID                            //
//    DESCRIPTION:   This function returns the device ID                     //
//                   This value must be 0xF2.                                //
//    EXAMPLE:       uint8_t ID_BMI160 = getPARTID_BMI160();               //
///////////////////////////////////////////////////////////////////////////////
uint8_t getPARTID_BMI160(void)
{
      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]         =     {COMMAND_READ_REG,REG_PARTID} ;
      uint8_t        pRxData[3];
      uint8_t        value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[2];
      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getREVID_BMI160()                                      //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t) BMI160's product revision ID                  //
//    DESCRIPTION:   This function returns the product revision ID,          //
//                   beginning with 0x00 and is changed for each subsequent  //
//                   revision.                                               //
//    EXAMPLE:       uint8_t ID_REV_BMI160 = getREVID_BMI160();            //
///////////////////////////////////////////////////////////////////////////////
uint8_t getREVID_BMI160(void)
{
      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]         =     {COMMAND_READ_REG,REG_REVID} ;
      uint8_t        pRxData[3];
      uint8_t        value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[2];
      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getXdata8B_BMI160()                                    //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (int8_t) X axis value (only 8 most significant bits)     //
//    DESCRIPTION:   This function returns the 8 most-significant bits of    //
//                  the x-axis acceleration data                             //
//    EXAMPLE:      int8_t X = getXdata8B_BMI160();                         //
///////////////////////////////////////////////////////////////////////////////
int8_t getXdata8B_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]         =     {COMMAND_READ_REG,REG_XDATA} ;
      uint8_t        pRxData[3];
      uint8_t        value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[2];
      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getYdata8B_BMI160()                                    //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (int8_t) Y axis value (only 8 most significant bits)     //
//    DESCRIPTION:   This function returns the 8 most-significant bits of    //
//                  the y-axis acceleration data                             //
//    EXAMPLE:      int8_t Y = getYdata8B_BMI160();                         //
///////////////////////////////////////////////////////////////////////////////
int8_t getYdata8B_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]         =     {COMMAND_READ_REG,REG_YDATA} ;
      uint8_t        pRxData[3];
      uint8_t        value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[2];
      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getZdata8B_BMI160()                                    //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (int8_t) Z axis value (only 8 most significant bits)     //
//    DESCRIPTION:   This function returns the 8 most-significant bits of    //
//                  the z-axis acceleration data                             //
//    EXAMPLE:      int8_t Z = getZdata8B_BMI160();                         //
///////////////////////////////////////////////////////////////////////////////
int8_t getZdata8B_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]         =     {COMMAND_READ_REG,REG_ZDATA} ;
      uint8_t        pRxData[3];
      uint8_t        value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[2];
      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getXYZdata8B_BMI160(int8_t *X,                         //
//                                        int8_t *Y,                         //
//                                        int8_t *Z)                         //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (int8_t) pointer to X axis value. only the 8 MSB         //
//                  (int8_t) pointer to Y axis value. only the 8 MSB         //
//                  (int8_t) pointer to Z axis value. only the 8 MSB         //
//                  ( MSB = Most Significant bytes. )                        //
//    DESCRIPTION:   This function returns the 8 most-significant bits of    //
//                  the x-axis, y-axis and z-axis acceleration data.         //
//                  These 3 parameters must be passed by reference.          //
//                  This function obtains the same values than:              //
//                          getXdata8B_BMI160();                            //
//                          getYdata8B_BMI160();                            //
//                          getZdata8B_BMI160();                            //
//                  together, but this function is more efficient (less time)//
//    EXAMPLE:      int8_t X;                                                //
//                  int8_t Y;                                                //
//                  int8_t Z;                                                //
//                  getXYZdata8B_BMI160(&X,&Y,&Z);                          //
///////////////////////////////////////////////////////////////////////////////
void getXYZdata8B_BMI160(int8_t *_X, int8_t *_Y, int8_t *_Z) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     3;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[5]         =     {COMMAND_READ_REG,REG_XDATA} ;
      uint8_t        pRxData[5];
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      *_X = pRxData[2];
      *_Y = pRxData[3];
      *_Z = pRxData[4];
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getSTATUS_BMI160()                                     //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t) BMI160's status register                      //
//    DESCRIPTION:   This function returns the BMI160's status register.    //
//                  This register includes the following bits that descibe   //
//                  various conditions                                       //
//                                                                           //
//    EXAMPLE:      //User can use the structure bellow to do something in   //
//                  //each case                                              //
//    uint8_t status = getSTATUS_BMI160();                                  //
//    if((status&PRM_ERR_USER_REGS_MASK)==PRM_ERR_USER_REGS_MASK)            //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_AWAKE_MASK)==PRM_AWAKE_MASK)                            //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_INACT_MASK)==PRM_INACT_MASK)                            //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_ACT_MASK)==PRM_ACT_MASK)                                //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_FIFO_OVERRUN)==PRM_FIFO_OVERRUN)                        //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_FIFO_WATERMARK)==PRM_FIFO_WATERMARK)                    //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_FIFO_READY)==PRM_FIFO_READY)                            //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_DATA_READY)==PRM_DATA_READY)                            //
//    {                                                                      //
//    }                                                                      //
///////////////////////////////////////////////////////////////////////////////
uint8_t getSTATUS_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]         =     {COMMAND_READ_REG,REG_STATUS} ;
      uint8_t        pRxData[3];
      uint8_t        value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[2];
      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getFIFO_ENTRIES_BMI160()                               //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint16_t) number of valid samples present in the FIFO   //
//    DESCRIPTION:   This function returns the number of valid data samples  //
//                  present in the FIFO buffer.                              //
//                  This number ranges from 0 to 512                         //
//    EXAMPLE:      uint16_t NsamplesInFIFO = getFIFO_ENTRIES_BMI160();     //
///////////////////////////////////////////////////////////////////////////////
uint16_t getFIFO_ENTRIES_BMI160(void) {

      uint32_t ui32FIFOEntries;

      bmi160_fifo_length(&ui32FIFOEntries);

      return ui32FIFOEntries;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getXdata12B_BMI160()                                   //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (int16_t) X axis value                                   //
//    DESCRIPTION:   This function returns the 12b x-axis acceleration data  //
//    EXAMPLE:      int16_t X = getXdata12B_BMI160();                       //
///////////////////////////////////////////////////////////////////////////////
int16_t getXdata12B_BMI160(void){

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     2;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[4]            =     {COMMAND_READ_REG,REG_XDATA_L} ;
      uint8_t        pRxData[4];
      int16_t       value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[3] << 8;
      value += pRxData[2];
      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getYdata12B_BMI160()                                   //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (int16_t) Y axis value                                   //
//    DESCRIPTION:   This function returns the 12b y-axis acceleration data  //
//    EXAMPLE:      int16_t Y = getYdata12B_BMI160();                       //
///////////////////////////////////////////////////////////////////////////////
int16_t getYdata12B_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     2;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[4]            =     {COMMAND_READ_REG,REG_YDATA_L} ;
      uint8_t        pRxData[4];
      int16_t       value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[3] << 8;
      value += pRxData[2];
      return value;

}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getZdata12B_BMI160()                                   //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (int16_t) Z axis value                          //
//    DESCRIPTION:   This function returns the 12b z-axis acceleration data  //
//    EXAMPLE:      int16_t Z = getYdata12B_BMI160();              //
///////////////////////////////////////////////////////////////////////////////
int16_t getZdata12B_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     2;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[4]            =     {COMMAND_READ_REG,REG_ZDATA_L} ;
      uint8_t        pRxData[4];
      int16_t       value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[3] << 8;
      value += pRxData[2];
      return value;

}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getTdata12B_BMI160()                                   //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (int16_t) Temperature value                     //
//    DESCRIPTION:   This function returns the 12b temperature data          //
//    EXAMPLE:      int16_t T = getTdata12B_BMI160();              //
///////////////////////////////////////////////////////////////////////////////
int16_t getTdata12B_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     2;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[4]            =     {COMMAND_READ_REG,REG_TEMP_L} ;
      uint8_t        pRxData[4];
      int16_t       value;
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[3] << 8;
      value += pRxData[2];
      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getXYZdata12B_BMI160(int16_t *X,              //
//                                         int16_t *Y,              //
//                                         int16_t *Z)              //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (int16_t) pointer to X axis value. 12 bits.     //
//                  (int16_t) pointer to Y axis value. 12 bits.     //
//                  (int16_t) pointer to Z axis value. 12 bits.     //
//    DESCRIPTION:   This function returns the x-axis, y-axis and z-axis     //
//                  acceleration data.                                       //
//                  These 3 parameters must be passed by reference.          //
//                  This function obtains the same values than:              //
//                          getXdata12B_BMI160();                           //
//                          getYdata12B_BMI160();                           //
//                          getZdata12B_BMI160();                           //
//                  together, but this function is more efficient (less time)//
//    EXAMPLE:      int16_t X;                                      //
//                  int16_t Y;                                      //
//                  int16_t Z;                                      //
//                  getXYZdata12B_BMI160(&X,&Y,&Z);                         //
///////////////////////////////////////////////////////////////////////////////
void getXYZdata12B_BMI160(int16_t *_X, int16_t *_Y, int16_t *_Z) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     6;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[8]            =     {COMMAND_READ_REG,REG_XDATA_L} ;
      uint8_t        pRxData[8];
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

      *_X = pRxData[3] << 8;
      *_X += pRxData[2];
      *_Y = pRxData[5] << 8;
      *_Y += pRxData[4];
      *_Z = pRxData[7] << 8;
      *_Z += pRxData[6];

      lengthWrite = 1;


}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getXYZTdata12B_BMI160(int16_t *X,             //
//                                          int16_t *Y,             //
//                                          int16_t *Z,             //
//                                          int16_t *T)             //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (int16_t) pointer to X axis value. 12 bits.     //
//                  (int16_t) pointer to Y axis value. 12 bits.     //
//                  (int16_t) pointer to Z axis value. 12 bits.     //
//                  (int16_t) pointer to Temperature value. 12 bits.//
//    DESCRIPTION:   This function returns the x-axis, y-axis, z-axis        //
//                  acceleration data and the temperature.                   //
//                  These 4 parameters must be passed by reference.          //
//                  This function obtains the same values than:              //
//                          getXdata12B_BMI160();                           //
//                          getYdata12B_BMI160();                           //
//                          getZdata12B_BMI160();                           //
//                          getTdata12B_BMI160();                           //
//                  together, but this function is more efficient (less time)//
//    EXAMPLE:      int16_t X;                                      //
//                  int16_t Y;                                      //
//                  int16_t Z;                                      //
//                  int16_t T;                                      //
//                  getXYZTdata12B_BMI160(&X,&Y,&Z,&T);                     //
///////////////////////////////////////////////////////////////////////////////
void getXYZTdata12B_BMI160(int16_t *_X,
                            int16_t *_Y,
                            int16_t *_Z,
                            int16_t *_T) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     8;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[10]            =     {COMMAND_READ_REG,REG_XDATA_L} ;
      uint8_t        pRxData[10];
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

      *_X = pRxData[3] << 8;
      *_X += pRxData[2];
      *_Y = pRxData[5] << 8;
      *_Y += pRxData[4];
      *_Z = pRxData[7] << 8;
      *_Z += pRxData[6];
      *_Z = pRxData[9] << 8;
      *_Z += pRxData[8];
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: setSOFT_RESET_BMI160()                                 //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:       none                                                    //
//    DESCRIPTION:   This function will reset the sensor                     //
//    EXAMPLE:       setSOFT_RESET_BMI160();                                //
///////////////////////////////////////////////////////////////////////////////
void setSOFT_RESET_BMI160(void) {

      bmi160_set_command_register(0xB6);
			MCU_HAL_Delay(55);
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: setActivityAndInactivity_BMI160(uint8_t     control_byte, //
//                                                  uint16_t thresholdAct,  //
//                                                  uint8_t      timeAct,       //
//                                                  uint16_t thresholdInact,//
//                                                  uint16_t timeInact)     //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:       (uint8_t)control_byte:   This parameter is written in      //
//                   ACT_INACT_CTL register. User can use constants below to //
//                   enable or disable each ACT_INACT_CTL parameter:         //
//                        | bit7  | bit6  |                                  //
//                                X                                          //
//                        | bit5  |                                          //
//                        PRM_LOOP_ENABLE or  PRM_LOOP_DISABLE               //
//                        | bit4  |                                          //
//                        PRM_LINK_ENABLE or  PRM_LINK_DISABLE               //
//                        | bit3  |                                          //
//                        PRM_INACT_AC_DCB_ENABLE or PRM_INACT_AC_DCB_DISABLE//
//                        | bit2  |                                          //
//                        PRM_INACT_EN_ENABLE or PRM_INACT_EN_DISABLE        //
//                        | bit1  |                                          //
//                        PRM_ACT_AC_DCB_ENABLE or PRM_ACT_AC_DCB_DISABLE    //
//                        | bit0  |                                          //
//                        PRM_ACT_EN_ENABLE or PRM_ACT_EN_DISABLE            //
//                                                                           //
//                                                                           //
//                   (uint16_t)thresholdAct: This parameter is written in   //
//                   THRESH_ACT_H and THRESH_ACT_L registers. This 11bit     //
//                   unsigned value sets the threshold for activity          //
//                   detection. To detect activity, the absolute value of    //
//                   the 12 bit acceleration data is compared with the 11 bit//
//                   THRESH_ACT value.                                       //
//                                                                           //
//                   (uint8_t)timeAct: This parameter is witten in TIME_ACT     //
//                   register. The value in this register sets the number of //
//                   consecutive samples that must be greater the activity   //
//                   threshold for an Activity event to be detected.         //
//                                                                           //
//                   (uint16_t)thresholdInact: This parameter is written in //
//                   THRESH_INACT_H and THRESH_INACT_L registers. This 11bit //
//                   unsigned value sets the threshold for inactivity        //
//                   detection. To detect inactivity, the absolute value of  //
//                   the 12 bit acceleration data is compared with the 11 bit//
//                   THRESH_INACT value.                                     //
//                                                                           //
//                   (uint16_t)timeInact: This parameter is witten in       //
//                   TIME_INACT register. The value in this register sets the//
//                   number of consecutive samples that must be greater the  //
//                   inactivity threshold for an Inactivity event to be      //
//                   detected.                                               //
//                                                                           //
//    OUTPUTS:       none                                                    //
//    DESCRIPTION:   This function sets all parameters to configure the      //
//                   activity and inactivity functions                       //
//    EXAMPLE:       setActivityAndInactivity_BMI160(PRM_LOOP_DISABLE |     //
//                                                    PRM_LINK_DISABLE |     //
//                                             PRM_INACT_AC_DCB_ENABLE |     //
//                                             PRM_INACT_EN_ENABLE     |     //
//                                             PRM_ACT_AC_DCB_ENABLE   |     //
//                                             PRM_ACT_EN_ENABLE       ,     //
//                                                                   50,     //
//                                                                   10,     //
//                                                                   20,     //
//                                                                 1000);    //
///////////////////////////////////////////////////////////////////////////////
void setActivityAndInactivity_BMI160(uint8_t      control_byte,
                                      uint16_t thresholdAct,
                                      uint8_t      timeAct,
                                      uint16_t thresholdInact,
                                      uint16_t timeInact) {

      uint8_t        lenghtWrite            =     10;
      uint8_t        pTxData[10];
      uint16_t   _aux;

      pTxData[0]          =     COMMAND_WRITE;
      pTxData[1]          =     REG_THRESH_ACT_L;
      pTxData[2]          =     (thresholdAct & 0x00FF);
      _aux               =     ((thresholdAct & 0x700)>>8);
      pTxData[3]          =     _aux;
      pTxData[4]          =     timeAct;
      pTxData[5]          =     (thresholdInact & 0x00FF);
       _aux              =     ((thresholdAct & 0x700)>>8);
      pTxData[6]          =     _aux;
      pTxData[7]          =     (timeInact & 0x00FF);
      _aux               =     ((timeInact & 0xFF00)>>8);
      pTxData[8]          =     _aux;
      pTxData[9]          =     control_byte;

      BMI160_SPI_Transmit(pTxData,lenghtWrite);
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getActivityAndInactivity_BMI160(uint8_t    *_control_byte,//
//                                                  uint16_t *_thresholdAct,//
//                                                  uint8_t      *_timeAct,     //
//                                                uint16_t *_thresholdInact,//
//                                                  uint16_t *_timeInact)   //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t)*_control_byte:   This parameter is read from      //
//                   ACT_INACT_CTL register. User can use the masks below to //
//                   know the value of each parameter:                       //
//                        | bit7  | bit6  |                                  //
//                                X                                          //
//                        | bit5  |                                          //
//                        PRM_LOOP_MASK                                      //
//                        | bit4  |                                          //
//                        PRM_LINK_MASK                                      //
//                        | bit3  |                                          //
//                        PRM_INACT_AC_DCB_MASK                              //
//                        | bit2  |                                          //
//                        PRM_INACT_EN_MASK                                  //
//                        | bit1  |                                          //
//                        PRM_ACT_AC_DCB_MASK                                //
//                        | bit0  |                                          //
//                        PRM_ACT_EN_MASK                                    //
//                                                                           //
//                   (uint16_t)*_thresholdAct: This parameter is read from  //
//                   THRESH_ACT_H and THRESH_ACT_L registers.                //
//                                                                           //
//                   (uint8_t)*_timeAct: This parameter is read from TIME_ACT   //
//                   register.                                               //
//                                                                           //
//                   (uint16_t)*_thresholdInact: This parameter is read from//
//                   THRESH_INACT_H and THRESH_INACT_L registers.            //
//                                                                           //
//                   (uint16_t)*_timeInact: This parameter is read from     //
//                   TIME_INACT register.                                    //
//                                                                           //
//    DESCRIPTION:   This function reads all parameters which are related to //
//                   activity and inactivity functions                       //
//    EXAMPLE:       uint8_t actAndInactReg;                                    //
//                   uint16_t thresAct;                                     //
//                   uint8_t  timeAct;                                          //
//                   uint16_t thresInact;                                   //
//                   uint16_t timeInact;                                    //
//                   setActivityAndInactivity_BMI160(&actAndInactReg,       //
//                                                    &thresAct,             //
//                                                    &timeAct,              //
//                                                    &thresInact,           //
//                                                    &timeInact);           //
///////////////////////////////////////////////////////////////////////////////
void getActivityAndInactivity_BMI160(uint8_t      *_control_byte,
                                      uint16_t *_thresholdAct,
                                      uint8_t      *_timeAct,
                                      uint16_t *_thresholdInact,
                                      uint16_t *_timeInact) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     8;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[10]            =     {COMMAND_READ_REG,REG_THRESH_ACT_L} ;
      uint8_t        pRxData[10];

      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

      *_thresholdAct        = pTxData[1] << 8;
      *_thresholdAct        += pTxData[0];
      *_timeAct             = pTxData[2];
      *_thresholdInact      = pTxData[4] << 8;
      *_thresholdInact      += pTxData[3];
      *_timeInact           = pTxData[6] << 8;
      *_timeInact           += pTxData[5];
      *_control_byte        = pTxData[7];

}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: setFIFOConf_BMI160(uint8_t temperature,                   //
//                                       uint8_t FIFOmode,                      //
//                                       uint16_t FIFOsamples);             //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:       (uint8_t)temperature:   This parameter  is written in       //
//                   the bit called FIFO_TEMP inside FIFO_CONTROL            //
//                   register. User should use constants below to enable or  //
//                   disable the storing temperature data in the FIFO        //
//                   PRM_FIFO_TEMP_ENABLE => Enables storing temp data       //
//                   PRM_FIFO_TEMP_DISABLE=> Disables storing temp data      //
//                                                                           //
//                   uint8_t FIFOmode: This parameter is written in             //
//                   the bit called FIFO_MODE inside FIFO_CONTROL            //
//                   register. User should use constants below to enable the //
//                   FIFO and selects the mode.                              //
//                   PRM_FIFO_MODE_DISABLE      =>  FIFO is disabled         //
//                   PRM_FIFO_MODE_FIFO_MODE    =>  FIFO Mode is enabled     //
//                   PRM_FIFO_MODE_STREAM_MODE  =>  FIFO stream mode is enab.//
//                   PRM_FIFO_MODE_TRIG_MODE    =>  FIFO trig. mode is enab. //
//                                                                           //
//                   (uint16_t)FIFOsamples: This parameter is written in    //
//                   FIFO_SAMPLES and AH (parameter iniside FIFO_CONTROL reg)//
//                   The value in this register specifies number of samples  //
//                   to store in the FIFO. The full range of FIFO Samples is //
//                   0 to 511.                                               //
//                                                                           //
//    OUTPUTS:       none                                                    //
//    DESCRIPTION:   This function sets all parameters to configure the      //
//                   FIFO function                                           //
//    EXAMPLE:       setFIFOConf_BMI160(PRM_FIFO_TEMP_ENABLE    ,           //
//                                       PRM_FIFO_MODE_FIFO_MODE ,           //
//                                                          128);            //
///////////////////////////////////////////////////////////////////////////////
void setFIFOConf_BMI160(uint8_t temperature,
                         uint8_t FIFOmode,
                         uint16_t FIFOsamples) {

      uint8_t        lenghtWrite            =     4;
      uint8_t        pTxData[4];
      uint16_t   _aux               =     (FIFOsamples & 0x0100)>>5;

      pTxData[0]          =     COMMAND_WRITE;
      pTxData[1]          =     REG_FIFO_CONTROL;
      pTxData[2]          =     _aux | temperature | FIFOmode;
      pTxData[3]          =     (uint8_t)(FIFOsamples & 0xFF);

      /**
      * The first argument to the function BMI160_SPI_Transmit is the  the address
      * of data read command and the register address of the BMI device from where
      * the data is to be read. The following address contains the data value to be
      * written to the register. If more data is to be written to the consecutive
      * register address, it can be written to the following address of the transmit
      * buffer and the driver will write that data to the next register address.
      * The 2nd argument is the size of the trasmitted data in bytes.
      * BMI160_SPI_Transmit() should be implemented in such a way that it transmits
      * the data from pTxData buffer of size lenghtWrite.
      */
      BMI160_SPI_Transmit(pTxData,lenghtWrite);
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getFIFOConf_BMI160(uint8_t      *_temperature  ,          //
//                                       uint8_t      *_FIFOmode     ,          //
//                                       uint16_t *_FIFOsamples) ;          //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t)*_temperature:   This parameter  is read from      //
//                   the bit called FIFO_TEMP inside FIFO_CONTROL            //
//                   register. User should use constants below to know if    //
//                   the storing temperature data in the FIFO is enabled or  //
//                   disabled.                                               //
//                   PRM_FIFO_TEMP_MASK                                      //
//                   PRM_FIFO_TEMP_ENABLE => Enables storing temp data       //
//                   PRM_FIFO_TEMP_DISABLE=> Disables storing temp data      //
//                                                                           //
//                   uint8_t FIFOmode: This parameter is read from              //
//                   the bit called FIFO_MODE inside FIFO_CONTROL            //
//                   register. User should use constants below to know the   //
//                   FIFO mode selected.                                     //
//                   PRM_FIFO_MODE_MASK                                      //
//                   PRM_FIFO_MODE_DISABLE      =>  FIFO is disabled         //
//                   PRM_FIFO_MODE_FIFO_MODE    =>  FIFO Mode is enabled     //
//                   PRM_FIFO_MODE_STREAM_MODE  =>  FIFO stream mode is enab.//
//                   PRM_FIFO_MODE_TRIG_MODE    =>  FIFO trig. mode is enab. //
//                                                                           //
//                   (uint16_t)FIFOsamples: This parameter is read from     //
//                   FIFO_SAMPLES and AH (parameter iniside FIFO_CONTROL reg)//
//                   The value in this register specifies number of samples  //
//                   to store in the FIFO. The full range of FIFO Samples is //
//                   0 to 511.                                               //
//                                                                           //
//    DESCRIPTION:   This function reads all parameters which are related to //
//                   FIFO function.                                          //
//    EXAMPLE:       uint8_t temp;                                              //
//                   uint8_t FIFOmode;                                          //
//                   uint16_t FIFOsamp;                                     //
//                   setFIFOConf_BMI160(&temp    ,                          //
//                                       &FIFOmode,                          //
//                                       &FIFOsamp);                         //
///////////////////////////////////////////////////////////////////////////////
void getFIFOConf_BMI160(uint8_t *_temperature,
                         uint8_t *_FIFOmode,
                         uint16_t *_FIFOsamples) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     2;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[4]            =     {COMMAND_READ_REG,REG_FIFO_CONTROL} ;
      uint8_t        pRxData[4];

      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

      *_temperature         = pRxData[2] & PRM_FIFO_TEMP_MASK;
      *_FIFOmode            = pRxData[2] & PRM_FIFO_MODE_MASK;
      *_FIFOsamples         = ((((uint16_t)pRxData[2])&PRM_AH_MASK)<<5)+pRxData[3];

}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: setINTMAP1_BMI160(uint8_t _value);                        //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:       (uint8_t)_value: This parameter  is written in INTMAP1      //
//                  register. User can use constants below to enable or      //
//                  disable each INTMAP1 parameter:                          //
//                        | bit7  |                                          //
//                        PRM_INT_LOW_ACTIVE_HIGH or PRM_INT_LOW_ACTIVE_LOW  //
//                        | bit6  |                                          //
//                        PRM_AWAKE_ENABLE or PRM_AWAKE_DISABLE              //
//                        | bit5  |                                          //
//                        PRM_INACT_ENABLE or  PRM_INACT_DISABLE             //
//                        | bit4  |                                          //
//                        PRM_ACT_ENABLE or  PRM_ACT_DISABLE                 //
//                        | bit3  |                                          //
//                        PRM_FIFO_OVERRUN_ENABLE or PRM_FIFO_OVERRUN_DISABLE//
//                        | bit2  |                                          //
//                    PRM_FIFO_WATERMARK_ENABLE or PRM_FIFO_WATERMARK_DISABLE//
//                        | bit1  |                                          //
//                        PRM_FIFO_READY_ENABLE or PRM_FIFO_READY_DISABLE    //
//                        | bit0  |                                          //
//                        PRM_DATA_READY_ENABLE or PRM_DATA_READY_DISABLE    //
//                                                                           //
//    OUTPUTS:       none                                                    //
//    DESCRIPTION:   This function configures INT1 interrupt pin.            //
//    EXAMPLE:       setINTMAP1_BMI160(PRM_INT_LOW_ACTIVE_HIGH        |     //
//                                      PRM_AWAKE_DISABLE              |     //
//                                      PRM_INACT_ENABLE               |     //
//                                      PRM_ACT_ENABLE                 |     //
//                                      PRM_FIFO_OVERRUN_ENABLE        |     //
//                                      PRM_FIFO_WATERMARK_DISABLE     |     //
//                                      PRM_FIFO_READY_ENABLE          |     //
//                                      PRM_DATA_READY_ENABLE          );    //
///////////////////////////////////////////////////////////////////////////////
void setINTMAP1_BMI160(uint8_t _value) {

      uint8_t        lenghtWrite            =     3;
      uint8_t        pTxData[3]    =     {COMMAND_WRITE,REG_INTMAP1,_value} ;
      BMI160_SPI_Transmit(pTxData,lenghtWrite);
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getINTMAP1_BMI160();                                   //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t) BMI160's INTMAP1 register                        //
//    DESCRIPTION:   This function returns the BMI160's INTMAP1 register.   //
//                  This register includes the following bits that describe  //
//                  various conditions.                                      //
//                                                                           //
//    EXAMPLE:      //User can use the structure bellow to do something in   //
//                  //each case                                              //
//    uint8_t status = getINTMAP1_BMI160();                                    //
//    if((status&PRM_INT_LOW_MASK)==PRM_INT_LOW_ACTIVE_HIGH)                 //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_AWAKE_MASK)==PRM_AWAKE_ENABLE)                          //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_INACT_MASK)==PRM_INACT_ENABLE)                          //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_ACT_MASK)==PRM_ACT_ENABLE)                              //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_FIFO_OVERRUN_MASK)==PRM_FIFO_OVERRUN_ENABLE)            //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_FIFO_WATERMARK_MASK)==PRM_FIFO_WATERMARK_ENABLE)        //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_FIFO_READY_MASK)==PRM_FIFO_READY_ENABLE)                //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_DATA_READY_MASK)==PRM_DATA_READY_ENABLE)                //
//    {                                                                      //
//    }                                                                      //
///////////////////////////////////////////////////////////////////////////////
uint8_t getINTMAP1_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]            =     {COMMAND_READ_REG,REG_INTMAP1} ;
      uint8_t        pRxData[3];
      uint8_t        value;

      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

      value = pRxData[2];

      return value;

}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: setINTMAP2_BMI160(uint8_t _value);                        //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:       (uint8_t)_value: This parameter  is written in INTMAP2      //
//                  register. User can use constants below to enable or      //
//                  disable each INTMAP2 parameter:                          //
//                        | bit7  |                                          //
//                        PRM_INT_LOW_ACTIVE_HIGH or PRM_INT_LOW_ACTIVE_LOW  //
//                        | bit6  |                                          //
//                        PRM_AWAKE_ENABLE or PRM_AWAKE_DISABLE              //
//                        | bit5  |                                          //
//                        PRM_INACT_ENABLE or  PRM_INACT_DISABLE             //
//                        | bit4  |                                          //
//                        PRM_ACT_ENABLE or  PRM_ACT_DISABLE                 //
//                        | bit3  |                                          //
//                        PRM_FIFO_OVERRUN_ENABLE or PRM_FIFO_OVERRUN_DISABLE//
//                        | bit2  |                                          //
//                    PRM_FIFO_WATERMARK_ENABLE or PRM_FIFO_WATERMARK_DISABLE//
//                        | bit1  |                                          //
//                        PRM_FIFO_READY_ENABLE or PRM_FIFO_READY_DISABLE    //
//                        | bit0  |                                          //
//                        PRM_DATA_READY_ENABLE or PRM_DATA_READY_DISABLE    //
//                                                                           //
//    OUTPUTS:       none                                                    //
//    DESCRIPTION:   This function configures INT2 interrupt pin.            //
//    EXAMPLE:       setINTMAP2_BMI160(PRM_INT_LOW_ACTIVE_HIGH        |     //
//                                      PRM_AWAKE_DISABLE              |     //
//                                      PRM_INACT_ENABLE               |     //
//                                      PRM_ACT_ENABLE                 |     //
//                                      PRM_FIFO_OVERRUN_ENABLE        |     //
//                                      PRM_FIFO_WATERMARK_DISABLE     |     //
//                                      PRM_FIFO_READY_ENABLE          |     //
//                                      PRM_DATA_READY_ENABLE          );    //
///////////////////////////////////////////////////////////////////////////////
void setINTMAP2_BMI160(uint8_t _value) {

      uint8_t        lenghtWrite            =     3;
      uint8_t        pTxData[3]    =     {COMMAND_WRITE,REG_INTMAP2,_value} ;
      BMI160_SPI_Transmit(pTxData,lenghtWrite);
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getINTMAP2_BMI160();                                   //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t) BMI160's INTMAP2 register                        //
//    DESCRIPTION:   This function returns the BMI160's INTMAP2 register.   //
//                  This register includes the following bits that descibe   //
//                  various conditions.                                      //
//                                                                           //
//    EXAMPLE:      //User can use the structure bellow to do something in   //
//                  //each case                                              //
//    uint8_t status = getINTMAP2_BMI160();                                    //
//    if((status&PRM_INT_LOW_MASK)==PRM_INT_LOW_ACTIVE_HIGH)                 //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_AWAKE_MASK)==PRM_AWAKE_ENABLE)                          //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_INACT_MASK)==PRM_INACT_ENABLE)                          //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_ACT_MASK)==PRM_ACT_ENABLE)                              //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_FIFO_OVERRUN_MASK)==PRM_FIFO_OVERRUN_ENABLE)            //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_FIFO_WATERMARK_MASK)==PRM_FIFO_WATERMARK_ENABLE)        //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_FIFO_READY_MASK)==PRM_FIFO_READY_ENABLE)                //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_DATA_READY_MASK)==PRM_DATA_READY_ENABLE)                //
//    {                                                                      //
//    }                                                                      //
///////////////////////////////////////////////////////////////////////////////
uint8_t getINTMAP2_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]            =     {COMMAND_READ_REG,REG_INTMAP2} ;
      uint8_t        pRxData[3];
      uint8_t        value;

      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

      value = pRxData[2];

      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: setFilter_BMI160(uint8_t _value);                         //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:       (uint8_t)_value: This parameter  is written in FILTER_CTL   //
//                  register. User can use constants below to enable or      //
//                  disable each FILTER_CTL parameter:                       //
//                        | bit7  | bit6  |                                  //
//                        PRM_RANGE_2GEE                                     //
//                        PRM_RANGE_4GEE                                     //
//                        PRM_RANGE_8GEE                                     //
//                        | bit5  | bit4  |                                  //
//                            X                                              //
//                        | bit3  |                                          //
//                        PRM_EXT_SAMPLE_ENABLE or PRM_EXT_SAMPLE_DISABLE    //
//                        | bit2  | bit1  | bit0  |                          //
//                        PRM_ODR_12_5                                       //
//                        PRM_ODR_25                                         //
//                        PRM_ODR_50                                         //
//                        PRM_ODR_100                                        //
//                        PRM_ODR_200                                        //
//                        PRM_ODR_400                                        //
//                                                                           //
//    OUTPUTS:       none                                                    //
//    DESCRIPTION:   This function configures BMI160's filter.              //
//    EXAMPLE:       setFilter_BMI160(PRM_RANGE_8GEE                  |     //
//                                     PRM_EXT_SAMPLE_DISABLE          |     //
//                                     PRM_ODR_400                     );    //
///////////////////////////////////////////////////////////////////////////////
void setFilter_BMI160(uint8_t _value) {

      uint8_t        lenghtWrite            =     3;
      uint8_t        pTxData[3]    =     {COMMAND_WRITE,REG_FILTER_CTL,_value} ;
      BMI160_SPI_Transmit(pTxData,lenghtWrite);
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getFilter_BMI160();                                    //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t) BMI160's FILTER_CTL register. User should use the//
//                   below masks to know the value of each parameter         //
//                   PRM_RANGE_MASK                                          //
//                   PRM_EXT_SAMPLE_MASK                                     //
//                   PRM_ODR_MASK                                            //
//                                                                           //
//    DESCRIPTION:   This function reads all parameters which are related to //
//                   filter.                                                 //
//    EXAMPLE:      //User can use the structure bellow to do something in   //
//                  //each case                                              //
//    uint8_t status = getFilter_BMI160();                                     //
//    if((status&PRM_RANGE_MASK)==PRM_RANGE_2GEE)                            //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_EXT_SAMPLE_MASK)==PRM_EXT_SAMPLE_ENABLE)                //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_ODR_MASK)==PRM_ODR_50)                                  //
//    {                                                                      //
//    }                                                                      //
///////////////////////////////////////////////////////////////////////////////
uint8_t getFilter_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]            =     {COMMAND_READ_REG,REG_FILTER_CTL} ;
      uint8_t        pRxData[3];
      uint8_t        value;

      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

			//debug("%u %u %u", pRxData[0], pRxData[1], pRxData[2])

      value = pRxData[2];

      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: setPowerControl_BMI160(uint8_t _value);                   //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:       (uint8_t)_value: This parameter  is written in POWER_CTL    //
//                  register. User can use constants below to enable or      //
//                  disable each FILTER_CTL parameter:                       //
//                        | bit7  |                                          //
//                            X                                              //
//                        | bit6  |                                          //
//                        PRM_EXT_CLK_ENABLE                                 //
//                        PRM_EXT_CLK_DISABLE                                //
//                        | bit5  | bit4  |                                  //
//                        PRM_LOW_NOISE_ULTRA_LOW                            //
//                        PRM_LOW_NOISE_LOW_POWER                            //
//                        PRM_LOW_NOISE_LOW_NOISE                            //
//                        | bit3  |                                          //
//                        PRM_SLEEP_ENABLE or PRM_SLEEP_DISABLE              //
//                        | bit2  |                                          //
//                        PRM_AUTOSLEEP_ENABLE or PRM_AUTOSLEEP_DISABLE      //
//                        | bit1  | bit0  |                                  //
//                        PRM_MEASURE_STANDBY_MODE                           //
//                        PRM_MEASURE_MEASURE_MODE                           //
//                                                                           //
//    OUTPUTS:       none                                                    //
//    DESCRIPTION:   This function configures BMI160's power control.       //
//    EXAMPLE:       setPowerControl_BMI160(PRM_EXT_CLK_DISABLE        |    //
//                                     PRM_LOW_NOISE_ULTRA_LOW          |    //
//                                     PRM_SLEEP_DISABLE                |    //
//                                     PRM_AUTOSLEEP_DISABLE            |    //
//                                     PRM_MEASURE_MEASURE_MODE         );   //
///////////////////////////////////////////////////////////////////////////////
void setPowerControl_BMI160(uint8_t _value) {

      uint8_t        lenghtWrite            =     3;
      uint8_t        pTxData[3]    =     {COMMAND_WRITE,REG_POWER_CTL,_value} ;
      BMI160_SPI_Transmit(pTxData,lenghtWrite);
}

void setFIFOSamples_BMI160(uint8_t _value) {

    uint8_t        lenghtWrite            =     3;
    uint8_t        pTxData[3]    =     {COMMAND_WRITE,REG_FIFO_SAMPLES,_value} ;
    BMI160_SPI_Transmit(pTxData,lenghtWrite);
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getPowerControl_BMI160();                              //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t) BMI160's POWER_CTL register. User should use the //
//                   below masks to know the value of each parameter         //
//                   PRM_EXT_CLK_MASK                                        //
//                   PRM_LOW_NOISE_MASK                                      //
//                   PRM_SLEEP_MASK                                          //
//                   PRM_AUTOSLEEP_MASK                                      //
//                   PRM_MEASURE_MASK                                        //
//                                                                           //
//    DESCRIPTION:   This function reads all parameters which are related to //
//                   BMI160's power settings.                               //
//    EXAMPLE:      //User can use the structure bellow to do something in   //
//                  //each case                                              //
//    uint8_t status = getPowerControl_BMI160();                               //
//    if((status&PRM_EXT_CLK_MASK)==PRM_EXT_CLK_ENABLE)                      //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_LOW_NOISE_MASK)==PRM_LOW_NOISE_LOW_NOISE)               //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_SLEEP_MASK)==PRM_SLEEP_ENABLE)                          //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_AUTOSLEEP_MASK)==PRM_AUTOSLEEP_ENABLE)                  //
//    {                                                                      //
//    }                                                                      //
//    if((status&PRM_MEASURE_MASK)==PRM_MEASURE_MEASURE_MODE)                //
//    {                                                                      //
//    }                                                                      //
///////////////////////////////////////////////////////////////////////////////
uint8_t getPowerControl_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]            =     {COMMAND_READ_REG,REG_POWER_CTL} ;
      uint8_t        pRxData[3];
      uint8_t        value;

      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

      value = pRxData[2];

      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: setSelfTest_BMI160(uint8_t _value);                       //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:       (uint8_t)_value: This parameter  is written in SELF_TEST    //
//                  register. User can use constants below to enable or      //
//                  disable each SELF_TEST parameter:                        //
//                  | bit7  | bit6  | bit5  | bit4  | bit3  | bit2  | bit1  |//
//                            X                                              //
//                  | bit0  |                                                //
//                  PRM_ST_ENABLE or PRM_ST_DISABLE                          //
//                                                                           //
//    OUTPUTS:       none                                                    //
//    DESCRIPTION:   This function configures BMI160's self test.           //
//    EXAMPLE:       setSelfTest_BMI160(PRM_ST_ENABLE);                     //
///////////////////////////////////////////////////////////////////////////////
void setSelfTest_BMI160(uint8_t _value) {

      uint8_t        lenghtWrite            =     3;
      uint8_t        pTxData[3]    =     {COMMAND_WRITE,REG_SELF_TEST,_value} ;
      BMI160_SPI_Transmit(pTxData,lenghtWrite);
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: setSelfTest_BMI160();                                  //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        none                                                    //
//    OUTPUTS:      (uint8_t) BMI160's SELF_TEST register. User should use the //
//                   below masks to know the value of each parameter         //
//                   PRM_ST_MASK                                             //
//                                                                           //
//    DESCRIPTION:   This function reads SELF_TEST registar and we can know  //
//                   if BMI160's test is enabled.                           //
//    EXAMPLE:      //User can use the structure bellow to do something in   //
//                  //this case                                              //
//    uint8_t status = setSelfTest_BMI160();                                   //
//    if((status&PRM_ST_MASK)==PRM_ST_ENABLE)                                //
//    {                                                                      //
//    }                                                                      //
///////////////////////////////////////////////////////////////////////////////
uint8_t getSelfTest_BMI160(void) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]            =     {COMMAND_READ_REG,REG_SELF_TEST} ;
      uint8_t        pRxData[3];
      uint8_t        value;

      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

      value = pRxData[2];

      return value;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getFIFO(int _lengthR, uint8_t *_arrayR);                   //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        (int) _lengthR: number of bytes to read from FIFO.      //
//                   this value must be between 0 - 511.                     //
//    OUTPUTS:      (uint8_t)*fifoData: This parameter  stores bytes read from   //
//                  the BMI160's FIFO.                                      //
//                                                                           //
//    DESCRIPTION:   This function reads _lengthR bytes from BMI160's FIFO. //
//                   Those bytes are stored in *fifoData.                     //
//    EXAMPLE:       int length = 48;                                        //
//                   uint8_t fifoData[48];                                         //
//                   getFIFO(length, array);                                 //
///////////////////////////////////////////////////////////////////////////////
void getFIFO(uint32_t _lengthR, uint16_t * fifoData) {
    
	    bmi160_read_fifo_header_data(_lengthR);
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getFIFOXYZ(  int _lengthR        ,                      //
//                                int16_t *_X,                      //
//                                int16_t *_Y,                      //
//                                int16_t *_Z);                     //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        (int) fifo_bytes: number of bytes to read from FIFO.      //
//                   this value must be between 0 - 511. In this function,   //
//                   fifo_bytes should be multiple of 6.                       //
//    OUTPUTS:      (int16_t)*_X: This parameter  stores X-axis     //
//                  values which were stored in the FIFO between the position//
//                  0 and _lengthR of the BMI160's FIFO.                    //
//                                                                           //
//                  (int16_t)*_Y: This parameter  stores Y-axis     //
//                  values which were stored in the FIFO between the position//
//                  0 and _lengthR of the BMI160's FIFO.                    //
//                                                                           //
//                  (int16_t)*_Z: This parameter  stores Z-axis     //
//                  values which were stored in the FIFO between the position//
//                  0 and _lengthR of the BMI160's FIFO.                    //
//                                                                           //
//    DESCRIPTION:   This function reads fifo_bytes bytes from BMI160's FIFO. //
//                   Those bytes are arranged and loaded in its corresponding//
//                   array.                                                  //
//    EXAMPLE:       int length = 48;                                        //
//                   int16_t X[48];                                 //
//                   int16_t Y[48];                                 //
//                   int16_t Z[48];                                 //
//                   getFIFOXYZ(length, X,Y,Z);                              //
///////////////////////////////////////////////////////////////////////////////
uint8_t getFIFOXYZ(uint16_t  fifo_bytes,
                int16_t *_X      ,
                int16_t *_Y      ,
                int16_t *_Z      ) {

    uint16_t  _arrayR[150];
//    uint8_t  max_lenght = 150;
//    uint8_t  error = 0;
//    uint16_t  bytes_to_read = 0;
		int i;

		fifo_bytes = fifo_bytes;
									
	  getFIFO(fifo_bytes, _arrayR);
									
		for (i = 0 ; i < fifo_bytes; i++ ){
			// shift for 12 bit.
			*_X++ =  (accel_fifo[i].x >> 4);
			*_Y++ =  (accel_fifo[i].y >> 4);
			*_Z++ =  (accel_fifo[i].z >> 4);
		}
		 
//			while(fifo_bytes > 0){

//      if(fifo_bytes >= max_lenght){
//        bytes_to_read = 150;
//      }
//      else{
//        bytes_to_read = fifo_bytes;
//      }
//      //getFIFO(bytes_to_read, _arrayR);

//     

////      if(fifo_bytes > max_lenght){
////        fifo_bytes -= max_lenght;
////      }
////      else{
////        fifo_bytes = 0;
////      }

//    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//    FUNCTION NAME: getFIFOXYZT( int _lengthR        ,                      //
//                                int16_t *_X,                      //
//                                int16_t *_Y,                      //
//                                int16_t *_Z,                      //
//                                int16_t *_T);                     //
//    DATE:          11/8/2011                                               //
//    REVISION:      1                                                       //
//    AUTHOR:        Jose Carlos Conchell                                    //
//    INPUTS:        (int) _lengthR: number of bytes to read from FIFO.      //
//                   this value must be between 0 - 511. In this function,   //
//                   _lengthR should be multiple of 8.                       //
//    OUTPUTS:      (int16_t)*_X: This parameter  stores X-axis     //
//                  values which were stored in the FIFO between the position//
//                  0 and _lengthR of the BMI160's FIFO.                    //
//                                                                           //
//                  (int16_t)*_Y: This parameter  stores Y-axis     //
//                  values which were stored in the FIFO between the position//
//                  0 and _lengthR of the BMI160's FIFO.                    //
//                                                                           //
//                  (int16_t)*_Z: This parameter  stores Z-axis     //
//                  values which were stored in the FIFO between the position//
//                  0 and _lengthR of the BMI160's FIFO.                    //
//                                                                           //
//                  (int16_t)*_T: This parameter  stores temperature//
//                  values which were stored in the FIFO between the position//
//                  0 and _lengthR of the BMI160's FIFO.                    //
//                                                                           //
//    DESCRIPTION:   This function reads _lengthR bytes from BMI160's FIFO. //
//                   Those bytes are arranged and loaded in its corresponding//
//                   array.                                                  //
//    EXAMPLE:       int length = 48;                                        //
//                   int16_t X[48];                                 //
//                   int16_t Y[48];                                 //
//                   int16_t Z[48];                                 //
//                   int16_t T[48];                                 //
//                   getFIFOXYZT(length, X,Y,Z,T);                           //
///////////////////////////////////////////////////////////////////////////////
void getFIFOXYZT(uint16_t               _lengthR,
                 int16_t *_X      ,
                 int16_t *_Y      ,
                 int16_t *_Z      ,
                 int16_t *_T      ) {
    uint16_t  _arrayR[512];
    getFIFO(_lengthR, _arrayR);
    uint16_t   _aux  = 0;
    for(uint16_t i = 0; i<(_lengthR>>1); i++)
    {
        _aux      =     _arrayR[(i<<1)+1];
        _aux      =     (_aux<<8) + _arrayR[(i<<1)+0];

        if (  (_aux & PRM_DATA_TYPE_MASK) == PRM_DATA_TYPE_X_AXIS )
        {
            *_X = (_aux & PRM_DATA_VALUE_MASK)|((_aux&PRM_SIGN_EXTENSION_MASK)<<2);
             _X++;
        }
        else if (  (_aux & PRM_DATA_TYPE_MASK) == PRM_DATA_TYPE_Y_AXIS )
        {
            *_Y = (_aux & PRM_DATA_VALUE_MASK)|((_aux&PRM_SIGN_EXTENSION_MASK)<<2);
             _Y++;
        }
        else if (  (_aux & PRM_DATA_TYPE_MASK) == PRM_DATA_TYPE_Z_AXIS )
        {
            *_Z = (_aux & PRM_DATA_VALUE_MASK)|((_aux&PRM_SIGN_EXTENSION_MASK)<<2);
             _Z++;
        }
        else// if (  (_aux & PRM_DATA_TYPE_MASK) == PRM_DATA_TYPE_T_AXIS )
        {
            *_T = (_aux & PRM_DATA_VALUE_MASK)|((_aux&PRM_SIGN_EXTENSION_MASK)<<2);
             _T++;
        }
    }
}
/******************************************************************************/

/******************************************************************************/
/*                    INTERNAL FUNCTIONS                                      */
/******************************************************************************/
void setRegisterBMI160(uint8_t _reg, uint8_t _value) {

      uint8_t        lenghtWrite            =     3;
      uint8_t        pTxData[3]    =     {COMMAND_WRITE,_reg,_value} ;
      BMI160_SPI_Transmit(pTxData,lenghtWrite);
}


void getRegisterBMI160( uint8_t _reg, uint16_t _lengthR, uint8_t *value)
{

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]            =     {COMMAND_READ_REG,_reg} ;
      uint8_t        pRxData[3];

      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

      *value = pRxData[2];

}

//void getBufferBMI160(uint16_t _lengthR, uint8_t *_arrayR)
//{
//      uint8_t        _lengthW           =     1;
//      uint8_t        pTxData[1]         =     {COMMAND_READ_BUFFER} ;
//
//      readSPI(_lengthW,pTxData,_lengthR,_arrayR);
//
//      uint8_t        lengthWrite           =     1;
//      uint8_t        lengthRead            =     _lengthR;
//      uint8_t        totalLength           =     lengthWrite + lengthRead;
//      uint8_t        pTxData[512];
//      uint8_t        pRxData[512];
//
//      pTxData[0] = COMMAND_READ_BUFFER;
//
//      BMI160_SPI_Receive(pTxData,pRxData,totalLength);
//
//      for (uint16_t i = 0 ; i < lengthRead ; i++ ){
//        *fifoData++ =  pRxData[i+1];
//      }
//}
/******************************************************************************/

void getIDs_BMI160(int16_t *A, int16_t *B, int16_t *C) {

      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     3;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[8]            =     {COMMAND_READ_REG,REG_DEVID_AD} ;
      uint8_t        pRxData[8];
      BMI160_SPI_Receive(pTxData,pRxData,totalLength);

      *A = pRxData[2];
      *B = pRxData[3];
      *C = pRxData[4];


      lengthWrite = 1;


}
