ReadMe.txt
==========

ADXLDrv Version: 1.3.1

Release Notes
-------------
Version 0.9, Sept 24, 2014                                                  
  Change FIFO filled threshold                                              
Version 1.0, Oct 29, 2014                                                   
  Add timestamp output                                                      
Version 1.1, Jan 28, 2015                                                   
  Fixed timestamp related bug                                               
Version 1.2, Feb 27, 2015                                                   
  Bug fix in timestamp calculation for watermark more than one sample       
Version 1.3, Mar  9, 2015                                                   
  Added setting of ODR and watermark at init
Version 1.3.1, Apr 2, 2015                                                  
  Fixed bug with timestamp when the watermark is greater than 1               
  
HAL Functions
-------------  

-----> Typedefs used <-----
typedef enum
{
  ADI_HAL_OK       = 0x00,
  ADI_HAL_ERROR    = 0x01,
  ADI_HAL_BUSY     = 0x02,
  ADI_HAL_TIMEOUT  = 0x03
} ADI_HAL_STATUS_t;

-----> void GPIO_IRQ_ADXL362_Disable(void) <-----
* Disable the interrupt line supporting the ADXL362

-----> void GPIO_IRQ_ADXL362_Enable(void) <-----
* Enable the interrupt line supporting the ADXL362

-----> void MCU_HAL_Delay(uint32_t delay) <-----
* Implement a millisecond delay, specified by the value "delay".

-----> uint32_t MCU_HAL_GetTick() <-----
* returns the current clock tick in milliseconds.

-----> ADXL362_SPI_Transmit <-----
* Transmits the buffer pointed to by "pData", where the size is specified by "Size".

ADI_HAL_STATUS_t ADXL362_SPI_Transmit(uint8_t *pData, uint16_t Size) {
  return SPI2_Transmit(pData,Size,10000);
}

Example:
void setINTMAP1_ADXL362(uint8_t _value) {

      uint8_t        lenghtWrite            =     3;
      uint8_t        pTxData[3]    =     {COMMAND_WRITE,REG_INTMAP1,_value} ;
      ADXL362_SPI_Transmit(pTxData,lenghtWrite);
}

-----> ADXL362_SPI_Receive <-----
* Does a transmit and receive, transmitting "pTxData" of "Size", receiving data in pRxData.
* It is assumed that the receiver knows the expected receive data size based on the command sent to the device.

ADI_HAL_STATUS_t ADXL362_SPI_Receive(uint8_t *pTxData, uint8_t *pRxData, uint16_t Size) {
  return SPI2_TransmitReceive(pTxData,pRxData,Size,10000);
}

Example:
uint8_t getREVID_ADXL362(void)
{
      uint8_t        lengthWrite           =     2;
      uint8_t        lengthRead            =     1;
      uint8_t        totalLength           =     lengthWrite + lengthRead;
      uint8_t        pTxData[3]         =     {COMMAND_READ_REG,REG_REVID} ;
      uint8_t        pRxData[3];
      uint8_t        value;
      ADXL362_SPI_Receive(pTxData,pRxData,totalLength);
      value = pRxData[2];
      return value;
}

Usage
-----  

#include "ADXL362.h"

static void DriverBringUp();

void main(void) {

...
    /* Configures GPIO pin for interrupt */
    GPIO_IRQ_ADXL362_Config();
...
    ADXLDrv_OpenDriver(100, 4);
...
    while(1) {
       DriverBringUp();
    }
    ADXLDrv_CloseDriver();
}

/**
    * @brief    Read data from the ADXL362
    * @param    file: pointer to the source file name
    * @param    data: point to an array in which the data will be placed
    * @retval   error
    */
int32_t ReadADXL362Data(int16_t *data, uint32_t *time) {

    int16_t value[3] = {0};
    int32_t i = 0;

    if (data == 0 || time == 0)
        return -1;

    if (ADXLDrv_ReadData(0, value, time) == ADXLDrv_ERROR)
        return -2;

    for (i = 0; i < 3; i++)
        data[i] = (int16_t)value[i];

    return 0;
}

/**
* DriverBringUp(): Sets up the ADXL driver for reading data
*/

static void DriverBringUp() {

    uint32_t i;
    uint32_t tsADXL = 0;
    int16_t value[3] = {0};
    
    while (1) {
        if (ReadADXL362Data(value, &tsADXL) != ADXLDrv_ERROR) {

            for (i = 0; i < 3; i++) {

                debug("%04X ", value[i]);
            }
            debug("\r\n");
        }
        MCU_HAL_Delay(5); //Give the CPU a break
    }
}
