//*****************************************************************************
//
//! @file pixart_pah8001.c
//
//*****************************************************************************
#include <stdio.h>
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"

#include "pixart_pah8001.h"
#include "PAH8001Set.h"

// RTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#include "hrm_queue.h"
#include "hrm_task.h"

#include "mbapp_buffer_pool.h"

//*****************************************************************************
//
// Forward declaration for the burst read callback function.
//
//*****************************************************************************
extern const uint8_t init_ppg_register_array[][2];
volatile uint32_t g_ui32BurstReadComplete = false;
volatile uint16_t HR_Cnt = 0;
volatile uint16_t Poll_Cnt = 0;

const pixart_pah8001_t sPAH8001 =
{
    .bMode = PAH8001_MODE_I2C,
    .ui32IOMModule = 0,
    .ui32ChipSelect = 0,
    .ui32Address = I2C_ID_PAH8001
};


uint8_t
writeRegister(uint8_t addr, uint8_t data)
{
    am_hal_iom_buffer(4) sBuffer;
    sBuffer.bytes[0] = data;

    if (sPAH8001.bMode == PAH8001_MODE_I2C)
    {
        am_hal_iom_queue_i2c_write(sPAH8001.ui32IOMModule, sPAH8001.ui32Address,
                             sBuffer.words, 1, AM_HAL_IOM_OFFSET(addr), HRMCallback);

        //
        // Wait for the transaction to complete.
        //
        HRMWait4CommandComplete();
    }
    else
    {
        while(1);
    }
    return 0;
}

uint8_t
readRegisterA(uint8_t addr, uint8_t *data)
{
    am_hal_iom_buffer(4) sBuffer;

    if (sPAH8001.bMode == PAH8001_MODE_I2C)
    {
        am_hal_iom_queue_i2c_read(sPAH8001.ui32IOMModule, sPAH8001.ui32Address,
                             sBuffer.words, 1, AM_HAL_IOM_OFFSET(addr), HRMCallback);

        //
        // Wait for the transaction to complete.
        //
        HRMWait4CommandComplete();
    }
    else
    {
        while(1);
    }

    // save the read data.
    *data = sBuffer.bytes[0];
    return 0;
}

static uint8_t
burstreadRegister(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
    am_hal_iom_buffer(300) sBuffer;
    int i;

    if (sPAH8001.bMode == PAH8001_MODE_I2C)
    {
        while (rx_size)
        {
            if (rx_size > 250)
            {
                am_hal_iom_queue_i2c_read(sPAH8001.ui32IOMModule, sPAH8001.ui32Address,
                             sBuffer.words, 250, AM_HAL_IOM_OFFSET(addr), HRMCallback);

                //
                // Wait for the transaction to complete.
                //
                HRMWait4CommandComplete();

                // save the read data.
                for(i = 0; i < 250; i++)
                {
                    *data++ = sBuffer.bytes[i];
                }

                rx_size -= 250;
            }
            else if (rx_size < 60)
            {
                am_hal_iom_queue_i2c_read(sPAH8001.ui32IOMModule, sPAH8001.ui32Address,
                             sBuffer.words, rx_size, AM_HAL_IOM_OFFSET(addr), HRMCallback);

                //
                // Wait for the transaction to complete.
                //
                HRMWait4CommandComplete();

                // save the read data.
                for(i = 0; i < rx_size; i++)
                {
                    *data++ = sBuffer.bytes[i];
                }
                rx_size = 0;
            }
            else
            {
               am_hal_iom_queue_i2c_read(sPAH8001.ui32IOMModule, sPAH8001.ui32Address,
                             sBuffer.words, rx_size, AM_HAL_IOM_OFFSET(addr), HRMCallback);

                //
                // Wait for the transaction to complete.
                //
                HRMWait4CommandComplete();

                // save the read data.
                for(i = 0; i < rx_size; i++)
                {
                    *data++ = sBuffer.bytes[i];
                }
                rx_size = 0;
            }
        }
    }
    else
    {
        while(1);
    }

    return 0;
}

uint8_t
writeBMI160(uint8_t addr, uint8_t data)
{
    return 0;
}

uint8_t
readBMI160(uint8_t addr, uint8_t *data)
{
    return 0;
}

#if 0
static uint8_t
burstreadBMI160(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
    return 0;
}
#endif


bool
accelerometer_init(void)
{
    uint8_t who_am_i = 0;

    readBMI160(0x7f, &who_am_i);	//dummy single spi read, recommended on datasheet
	readBMI160(0x00, &who_am_i); am_util_debug_printf("Accel ID = %X\n", who_am_i);
    if(who_am_i != 0xD1)
    {
        //no g-sensor
        memset(_g_sensor_data, 0, sizeof(_g_sensor_data));
    }
    else
    {
        _bmi160_exist = 1 ;
        /*0x40, 0x86,   // set ODR to 25Hz with no averaging
        0x41, 0x05, // set FS range to +/- 4G
        0x7e, 0x12, // set the accelerometer to low power mode
        */
        writeBMI160(0x7e, 0xb6);    // softreset BMI160 back to suspend for both ACC and GYR
        am_util_delay_ms(50);                   // wait for BMI160 to finish softreset and enter suspend mode

        writeBMI160(0x7e, 0x11);    // set the accelerometer to normal mode
        am_util_delay_ms(5);                    // wait for the accelerometer to stabilize

        writeBMI160(0x40, 0x26);    // set ODR to 25Hz with averaging
		am_util_delay_ms(1); readBMI160(0x40, &who_am_i);
		if(who_am_i != 0x26)
		{
			am_util_debug_printf("Init BMI160 Reg0x40 fail!!");
			am_util_debug_printf("W 40 26, R 40 %x\n", who_am_i);
		}
        writeBMI160(0x41, 0x05);    // set FS range to +/- 4G
		am_util_delay_ms(1); readBMI160(0x41, &who_am_i);
		if(who_am_i != 0x05)
		{
			am_util_debug_printf("Init BMI160 Reg0x41 fail!!");
			am_util_debug_printf("W 41 05, R 41 %x\n", who_am_i);
		}
        writeBMI160(0x47, 0x40);    // set Accel FIFO enable, no header
		am_util_delay_ms(1); readBMI160(0x47, &who_am_i);
		if(who_am_i != 0x40)
		{
			am_util_debug_printf("Init BMI160 Reg0x47 fail!!");
			am_util_debug_printf("W 47 40, R 4e %x\n", who_am_i);
		}


		writeBMI160(0x7e, 0x11);    // set the accelerometer to normal mode
        am_util_delay_ms(5);
		am_util_delay_ms(1); readBMI160(0x03, &who_am_i);
		if(who_am_i != 0x10)
		{
			am_util_debug_printf("BMI160 Not in Normal mode!!");
			am_util_debug_printf("R 03 %x\n", who_am_i);
		}
        //writeBMI160(0x47, 0x50);  // set Accel FIFO enable, with header
    }
    return true;
}

void
accelerometer_start(void)
{
    accelerometer_init();
}
void
accelerometer_stop(void)
{
    /*uint8_t tmp;
	readBMI160(0x7f, &tmp);	//dummy single spi read, recommended on datasheet
	writeBMI160(0x7e, 0xb6);    // softreset BMI160 back to suspend for both ACC and GYR
    am_util_delay_ms(50);                   // wait for BMI160 to finish softreset and enter suspend mode
	*/
}

/* ---------------------------------------------------------------
 * Below for PAH8001 main code
*///--------------------------------------------------------------
bool
pah8001_init()
{   uint16_t q;
    uint8_t bank=0, temp;

	PxiAlg_SetMemsScale(1);
    PxiAlg_EnableFastOutput(false);
    PxiAlg_EnableAutoMode(true);
    PxiAlg_EnableMotionMode(false);

    uint8_t tmp;
	writeRegister(0x7f, 0x00);
	writeRegister(0x7f, 0x00);
	tmp = readRegister(0x00);
	if(tmp == 0x30)
    {
        am_util_debug_printf("PAH8001 I2C Link Successful!\n\r");
    }
    else{
        am_util_debug_printf("PAH8001 I2C Link Fail!  ID = %d\n\r", tmp);
		while(1);
		//return false;
	}

    //Initialization settings
    writeRegister(0x06, 0x82);  //Reset sensor
    am_util_delay_ms(10);    //make a delay

    for(q=0;q<INIT_PPG_REG_ARRAY_SIZE;q++){
        if(init_ppg_register_array[q][0] == 0x7F)
            bank = init_ppg_register_array[q][1];

        if((bank == 0) && (init_ppg_register_array[q][0] == 0x17) )
        {
            //read and write bit7=1
            temp = readRegister(0x17);
            temp |= 0x80 ;
            writeRegister(0x17, temp) ;
        }
        else
            writeRegister(init_ppg_register_array[q][0], init_ppg_register_array[q][1]);
    }

	return true;
}

uint8_t
readRegister(uint8_t addr)
{
    uint8_t data = 0 ;
    readRegisterA(addr, &data);

    return data;
}

void
pixart_accel_set(int16_t x, int16_t y, int16_t z)
{
    ppg_mems_data.MEMS_Data[0] = x;
    ppg_mems_data.MEMS_Data[1] = y;
    ppg_mems_data.MEMS_Data[2] = z;
}

bool
Pixart_HRD(void)
{
    ppg_mems_data_t ppg_mems_data;

    //Check Touch Status for power saving
	touch = readRegister(0x59)&0x80;
	if( touch == 0x80)
	{
		writeRegister(0x7F,0x01); //bank1
		ppg_mems_data.HRD_Data[0]=readRegister(0x68)&0x0f; //check status: 0 is not ready, 1 is ready, 2 is loss one data?

		if(ppg_mems_data.HRD_Data[0] ==0)
		{
			writeRegister(0x7F,0x00); //bank0
			led_ctrl();
			return false;
		}
		else
		{
			//Only support burst read (0x64~0x67), when using I2C interface
			burstreadRegister(0x64, &ppg_mems_data.HRD_Data[1], 4);
			writeRegister(0x7F,0x00); //bank0

			ppg_mems_data.HRD_Data[8]= Frame_Count++;
			ppg_mems_data.HRD_Data[9]= Poll_Cnt;
			ppg_mems_data.HRD_Data[10]=_led_current_change_flag;

			//Assign fixed number to PPG data
			ppg_mems_data.HRD_Data[5]= 0;
			ppg_mems_data.HRD_Data[6]= 128;
			ppg_mems_data.HRD_Data[7]= 128;
			ppg_mems_data.HRD_Data[11]= 128;
			ppg_mems_data.HRD_Data[12]= 128;
			Poll_Cnt = 0;

			//If no G sensor, please set G_Sensor_Data[3] = {0};
#if 0
            uint8_t dataL, dataH;
			readBMI160(0x12, &dataL);
            readBMI160(0x13, &dataH);
			ppg_mems_data.MEMS_Data[0] = (int16_t)((uint16_t)dataH<<8 | dataL);

            readBMI160(0x14, &dataL);
            readBMI160(0x15, &dataH);
			ppg_mems_data.MEMS_Data[1] = (int16_t)((uint16_t)dataH<<8 | dataL);

            readBMI160(0x16, &dataL);
            readBMI160(0x17, &dataH);
			ppg_mems_data.MEMS_Data[2] = (int16_t)((uint16_t)dataH<<8 | dataL);
#endif

			Push(&ppg_mems_data); //Save data into FIFO

			return true;
		}
	}

	else
	{
		if (_sleepflag != 1)
		{
			writeRegister(0x05,0xB8);
			writeRegister(0x7F,0x01);
			_led_step = DEFAULT_LED_STEP;
			writeRegister(0x38, 0xFF);
			writeRegister(0x7f,0x00);
			_sleepflag = 1;
			_led_current_change_flag = 0;
		}

		return false;
	}
}

bool
isFIFOEmpty(void)
{
    return (_write_index == _read_index);
}

bool
Push(ppg_mems_data_t *data)
{
    int tmp = _write_index;
    tmp++;
    if(tmp >= FIFO_SIZE)
        tmp = 0;
    if(tmp == _read_index)
        return false;
    _ppg_mems_data[tmp] = *data;
    _write_index = tmp;

    return true;
}

bool
Pop(ppg_mems_data_t *data)
{
    int tmp;
    if(isFIFOEmpty())
        return false;
    *data = _ppg_mems_data[_read_index];
    tmp = _read_index + 1;
    if(tmp >= FIFO_SIZE)
        tmp = 0;
    _read_index = tmp;

    return true;
}

void
pah8001_low_power_enable(void)
{
    //
    // Enter Power down mode.
    //
    writeRegister(0x06, 0xa);
}

void
pah8001_low_power_disable(void)
{
    //
    // Enter normal mode.
    //
    writeRegister(0x06, 0x2);
}

/***********************LED Control Start***********************************/
void
led_ctrl(void)    {
	uint8_t data;
	uint16_t EP_L, EP_H, Exposure_Line;
	writeRegister(0x05,0x98);

	data = readRegister(0x33);
	EP_H=data&0x03;
	data = readRegister(0x32);
	EP_L=data;
	Exposure_Line=(EP_H<<8)+EP_L;
	writeRegister(0x7f,0x01);
	if(_sleepflag==1)   {
		writeRegister(0x38, (0xE0|DEFAULT_LED_STEP));
		_sleepflag = 0 ;
	}

	if (_state_count <= STATE_COUNT_TH) {
		_state_count++;
		_led_current_change_flag = 0;
	}
	else {
		_state_count = 0;
		if(_state == 0) {
			if( (Exposure_Line>=LED_CTRL_EXPO_TIME_HI_BOUND) || (Exposure_Line<=LED_CTRL_EXPO_TIME_LOW_BOUND) ) {
				//writeRegister(0x7f,0x01);
				data = readRegister(0x38);
				_led_step=data&0x1f;
				if( (Exposure_Line>=LED_CTRL_EXPO_TIME_HI_BOUND) && (_led_step < LED_CURRENT_HI) )  {
					_state = 1 ;
					_led_step=_led_step+LED_INC_DEC_STEP;

					if(_led_step>LED_CURRENT_HI)
						_led_step=LED_CURRENT_HI;
					writeRegister(0x38, (_led_step|0xE0));
					_led_current_change_flag = 1;
				}
				else if((Exposure_Line<=LED_CTRL_EXPO_TIME_LOW_BOUND) && (_led_step > LED_CURRENT_LOW) )    {
					_state = 2 ;
					if(_led_step<=(LED_CURRENT_LOW+LED_INC_DEC_STEP))
						_led_step=LED_CURRENT_LOW;
					else
						_led_step=_led_step-LED_INC_DEC_STEP;
					writeRegister(0x38, (_led_step|0xE0));
					_led_current_change_flag = 1;
				}
				else    {
					_state = 0 ;
					_led_current_change_flag = 0;
				}
			}
			else {
				_led_current_change_flag = 0;
			}
		}
		else if(_state == 1)    {
			if(Exposure_Line > LED_CTRL_EXPO_TIME_HI)   {
				_state = 1 ;
				_led_step=_led_step+LED_INC_DEC_STEP;
				if(_led_step>=LED_CURRENT_HI)   {
					_state = 0 ;
					_led_step=LED_CURRENT_HI;
				}
				writeRegister(0x38, (_led_step|0xE0));
				_led_current_change_flag = 1;
			}
			else    {
				_state = 0 ;
				_led_current_change_flag = 0;
			}
		}
		else    {
			if(Exposure_Line < LED_CTRL_EXPO_TIME_LOW)  {
				_state = 2 ;
				if(_led_step<=(LED_CURRENT_LOW+LED_INC_DEC_STEP))   {
					_state = 0 ;
					_led_step=LED_CURRENT_LOW;
				}
				else
					_led_step=_led_step-LED_INC_DEC_STEP;
				writeRegister(0x38, (_led_step|0xE0));
				_led_current_change_flag = 1;
			}
			else    {
				_state = 0;
				_led_current_change_flag = 0;
			}
		}
	}
	writeRegister(0x7f,0x00);
}
/***********************LED Control End ***********************************/

uint8_t
pah8001_task(void)
{
	if(!isFIFOEmpty())
	{
		if(Pop(&ppg_mems_data)) //Get data from FIFO
		{
			MEMS_Data[0] = ppg_mems_data.MEMS_Data[0];
			MEMS_Data[1] = ppg_mems_data.MEMS_Data[1];
			MEMS_Data[2] = ppg_mems_data.MEMS_Data[2];

			uint8_t tmp = PxiAlg_Process(ppg_mems_data.HRD_Data, MEMS_Data);
			if( tmp != FLAG_DATA_READY)
			{
				am_util_debug_printf("AlgoProcssRtn: %d\n\r", tmp);
				am_util_debug_printf("PPG[8]: %d\n\r", ppg_mems_data.HRD_Data[8]);
			}
			else{
				PxiAlg_HrGet(&myHR);
				PxiAlg_GetSigGrade(&grade);
			}
		}
	}

	if( (HR_Cnt>HR_Rpt) && (touch==0x80) )
	{
		HR_Cnt = 0;
		am_util_debug_printf("HR: %3.2f, SG: %3.2f\r\n", myHR, grade);
	}

    return (uint8_t)myHR;
}
