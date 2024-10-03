ReadMe.txt
==========

ADPDDrv Version: 1.4.1

                                                    ro code  ro data  rw data
ADPDDrv (Release Build)                             1,744    68       532

Release Notes
-------------
Version 0.9, Sept 24, 2014                                                  
  Change FIFO filled threshold                                              
Version 1.0, Oct 01, 2014                                                   
  Add efuse function                                                        
Version 1.1, Oct 29, 2014                                                   
  Add timestamp output                                                      
Version 1.2, Nov 21, 2014                                                   
  Created MCU_HAL_I2C_TxRx() and use it here                                
  Added set to Idle mode in Open/close driver                               
  Enable to read FIFO from ISR                                              
Version 1.3, Jan 28, 2015                                                   
  Fixed bug with time stamp and FIFO watermark                              
Version 1.4, Feb 28, 2015                                                   
  Added device reset and FIFO overflow indicator in debug mode              
Version 1.4.1, Apr 2, 2015                                                  
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

-----> void  GPIO_IRQ_ADPD_Disable(void) <-----
* Disable the interrupt line supporting the ADPD device

-----> void  GPIO_IRQ_ADPD_Enable(void) <-----
* Enable the interrupt line supporting the ADPD device

-----> void GPIO_DUT_POWER_Off(void) <-----
* Set GPIO line to do an ADPD device power off

-----> void  GPIO_DUT_POWER_On(void) <-----
* Set GPIO line to do an ADPD device power on

-----> void MCU_HAL_Delay(uint32_t delay) <-----
* Implement a millisecond delay, specified by the value "delay".

-----> uint32_t MCU_HAL_GetTick() <-----
* returns the current clock tick in milliseconds.

-----> MCU_HAL_I2C_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout) <-----
* Transmits the buffer pointed to by "pData", where the size is specified by "size". 
* Time out if the device don't respond within specified time "Timeout"

-----> MCU_HAL_I2C_TxRx(uint8_t *pTxData, uint8_t *pRxData, uint16_t RxSize, uint32_t Timeout) <-----
* Transmits the address pointed to by "pTxData" buffer. 
* Receives the data from ADPD in buffer pointed to by "pRxData", where the size is specified by "RxSize". 
* Time out if the device don't respond within specified time "Timeout"

Usage
-----

#include "ADPDDrv.h"

static void LoadDefaultConfig(uint32_t *cfg);
static void DriverBringUp(uint8_t slot);

static uint32_t dcfg[] = {
    0x004b2695,  // Data sampling clock 32KHz frequency adjust
    0x004d4272,  // Internal timing clock 32MHz frequency adjust
    0x00020005,
    0x00060000,
    0x0012000A,
    0x00140447, //0x00140449
    0x00150333,
    0x00181F00,
    0x00191F00,
    0x001a1F00,
    0x001b1F00,
    0x001e1F00,
    0x001f1F00,
    0x00201F00,
    0x00211F00,
    0x00223030,
    0x00233035, //-> 3x, adjusted slew rate //0x00233035 // * 0x00233035
    0x0024303C, //-> 3x, adjusted slew rate //0x0024303A // * 0x0024303A
    0x002502DF,
    0x00340300,
    0x00300330,
    0x00310113, // LED pulses
    0x00421C36, // Integrator register
    0x0043ADA5,
    0x003924D4,
    0x003b24D4,
    0x00350330,
    0x00360813, //0x00360813 // LED pulses
    0x00441C35, // 35 -> 100K (writst) //0x00441C36 -> 50K (finger), // Integrator register 0x1c37 -> 25K
    0x0045ADA5,
    0x004e0040,
    0xFFFFFFFF,
};

main() {

    ADPDDrv_SoftReset();
    ADPDDrv_OpenDriver();
    LoadDefaultConfig(dcfg);
    
    DriverBringUp(ADPDDrv_SLOT_AB);
    
    ADPDDrv_CloseDriver();
}

/**
  * Load ADPD default configuration
  */
static void LoadDefaultConfig(uint32_t *cfg) {
    uint8_t regAddr, i;
    uint16_t regData;

    if (cfg == 0)
        return;

    // clear FIFO
    ADPDDrv_RegWrite(0x10, 0);
    ADPDDrv_RegWrite(0x5F, 1);
    ADPDDrv_RegWrite(0x00, 0x80FF);
    ADPDDrv_RegWrite(0x5F, 0);

    i=0;
    while(1) {

        regAddr = (uint8_t) (cfg[i]>>16);
        regData = (uint16_t)(cfg[i]);
        i++;
        if (regAddr == 0xFF)
            break;

        if (ADPDDrv_RegWrite(regAddr, regData) != ADPDDrv_SUCCESS)
            break;
    }
}

/**
 * DriverBringUp(): Sets up the ADPD driver for reading data
 */
static void DriverBringUp(uint8_t slot) {

    uint32_t i;
    uint32_t tsADPD = 0;
    uint16_t value[8] = {0};
    uint32_t index = (slot == ADPDDrv_SLOT_B)?4:0; // For display only

    ADPDDrv_SelectSlot(slot);
    ADPDDrv_SetOperationMode(ADPDDrv_MODE_SAMPLE);
    while (1) {
        if (ADPDDrv_ReadData(0, &value[index], &tsADPD) != ADPDDrv_ERROR) {

            for (i = 0; i < 8; i++) {

                debug("%04X ", value[i]);
            }
            debug("\r\n");
        }
    }
}
