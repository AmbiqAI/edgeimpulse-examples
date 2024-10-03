//*****************************************************************************
//
//! @file pixart_pah8005.c
//
//*****************************************************************************
#include <stdio.h>
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"

#include "pixart_pah8005.h"
#include "PAH8005Set.h"

//*****************************************************************************
//
// Forward declaration for the burst read callback function.
//
//*****************************************************************************
void burst_callback(void);

extern const uint8_t init_ppg_register_array[][2];
volatile uint32_t g_ui32BurstReadComplete = false;
volatile uint16_t HR_Cnt = 0;
volatile uint16_t Poll_Cnt = 0;

const bosch_bmi160_t sBMI160 =
{
    .bMode = BMI160_MODE_SPI,
    .ui32IOMModule = AM_BSP_BMI160_IOM,
    .ui32ChipSelect = AM_BSP_BMI160_CS,
    .ui32Address = I2C_ID_ACCEL
};

const pixart_pah8005_t sPAH8005 =
{
    .bMode = pah8005_MODE_I2C,
    .ui32IOMModule = 0,
    .ui32ChipSelect = 0,
    .ui32Address = I2C_ID_PAH8005
};

void
burst_callback(void)
{
    g_ui32BurstReadComplete = true;
}

void
am_iomaster0_isr(void)
{
    uint32_t ui32IntStatus;

    //
    // Read and clear the interrupt status.
    //
    ui32IntStatus = am_hal_iom_int_status_get(0, false);
    am_hal_iom_int_clear(0, ui32IntStatus);

    am_hal_iom_int_service(0, ui32IntStatus);
}

void
am_iomaster1_isr(void)
{
    uint32_t ui32IntStatus;

    //
    // Read and clear the interrupt status.
    //
    ui32IntStatus = am_hal_iom_int_status_get(1, false);
    am_hal_iom_int_clear(1, ui32IntStatus);

    am_hal_iom_int_service(1, ui32IntStatus);
}

uint8_t
writeRegister(uint8_t addr, uint8_t data)
{
    am_hal_iom_buffer(4) sBuffer;
    sBuffer.bytes[0] = data;

    am_hal_iom_enable(sPAH8005.ui32IOMModule);
    if (sPAH8005.bMode == pah8005_MODE_I2C)
    {
        am_hal_iom_i2c_write(sPAH8005.ui32IOMModule, sPAH8005.ui32Address,
                             sBuffer.words, 1, AM_HAL_IOM_OFFSET(addr));
    }
    else
    {
        while(1);
    }
    am_hal_iom_disable(sPAH8005.ui32IOMModule);
    return 0;
}

uint8_t
readRegisterA(uint8_t addr, uint8_t *data)
{
    am_hal_iom_buffer(4) sBuffer;

    am_hal_iom_enable(sPAH8005.ui32IOMModule);
    if (sPAH8005.bMode == pah8005_MODE_I2C)
    {
        am_hal_iom_i2c_read(sPAH8005.ui32IOMModule, sPAH8005.ui32Address,
                             sBuffer.words, 1, AM_HAL_IOM_OFFSET(addr));
    }
    else
    {
        while(1);
    }

    // save the read data.
    *data = sBuffer.bytes[0];

    am_hal_iom_disable(sPAH8005.ui32IOMModule);
    return 0;
}

static uint8_t
burstreadRegister(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
    am_hal_iom_buffer(300) sBuffer;
    int i;

    am_hal_iom_enable(sPAH8005.ui32IOMModule);
    if (sPAH8005.bMode == pah8005_MODE_I2C)
    {
        while (rx_size)
        {
            if (rx_size > 250)
            {
                am_hal_iom_i2c_read(sPAH8005.ui32IOMModule, sPAH8005.ui32Address,
                             sBuffer.words, 250, AM_HAL_IOM_OFFSET(addr));

                // save the read data.
                for(i = 0; i < 250; i++)
                {
                    *data++ = sBuffer.bytes[i];
                }

                rx_size -= 250;
            }
            else if (rx_size < 60)
            {
                am_hal_iom_i2c_read(sPAH8005.ui32IOMModule, sPAH8005.ui32Address,
                             sBuffer.words, rx_size, AM_HAL_IOM_OFFSET(addr));

                // save the read data.
                for(i = 0; i < rx_size; i++)
                {
                    *data++ = sBuffer.bytes[i];
                }

                rx_size = 0;
            }
            else
            {
               am_hal_iom_i2c_read_nb(sPAH8005.ui32IOMModule, sPAH8005.ui32Address,
                             sBuffer.words, rx_size, AM_HAL_IOM_OFFSET(addr), burst_callback);

                //
                // Sleep while the transfer is underway.
                //
                while(g_ui32BurstReadComplete == false);
                {
                    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);
                }

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

    am_hal_iom_disable(sPAH8005.ui32IOMModule);
    return 0;
}

uint8_t
writeBMI160(uint8_t addr, uint8_t data)
{
    am_hal_iom_buffer(4) sBuffer;
    sBuffer.bytes[0] = data;

    am_hal_iom_enable(sBMI160.ui32IOMModule);
    if (sBMI160.bMode == BMI160_MODE_I2C)
    {
        am_hal_iom_i2c_write(sBMI160.ui32IOMModule, sBMI160.ui32Address,
                             sBuffer.words, 1, AM_HAL_IOM_OFFSET(addr));
    }
    else
    {
        am_hal_iom_spi_write(sBMI160.ui32IOMModule, sBMI160.ui32ChipSelect,
                             sBuffer.words, 1, AM_HAL_IOM_OFFSET(addr));
    }
    am_hal_iom_disable(sBMI160.ui32IOMModule);
    return 0;
}

uint8_t
readBMI160(uint8_t addr, uint8_t *data)
{
    am_hal_iom_buffer(4) sBuffer;
	addr = 0x80 | addr;

    am_hal_iom_enable(sBMI160.ui32IOMModule);
    if (sBMI160.bMode == BMI160_MODE_I2C)
    {
        am_hal_iom_i2c_read(sBMI160.ui32IOMModule, sBMI160.ui32Address,
                             sBuffer.words, 1, AM_HAL_IOM_OFFSET(addr));
    }
    else
    {
        am_hal_iom_spi_read(sBMI160.ui32IOMModule, sBMI160.ui32ChipSelect,
                             sBuffer.words, 1, AM_HAL_IOM_OFFSET(addr));
    }

   // save the read data.
   *data = sBuffer.bytes[0];

   am_hal_iom_disable(sBMI160.ui32IOMModule);
    return 0;
}

#if 0
static uint8_t
burstreadBMI160(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
    am_hal_iom_buffer(1024) sBuffer;
    int i;
	addr = 0x80 | addr;

    am_hal_iom_enable(sBMI160.ui32IOMModule);
    if (sBMI160.bMode == BMI160_MODE_I2C)
    {
        am_hal_iom_i2c_read_nb(sBMI160.ui32IOMModule, sBMI160.ui32Address,
                             sBuffer.words, rx_size, AM_HAL_IOM_OFFSET(addr), burst_callback);
    }
    else
    {
        am_hal_iom_spi_read_nb(sBMI160.ui32IOMModule, sBMI160.ui32ChipSelect,
                             sBuffer.words, rx_size, AM_HAL_IOM_OFFSET(addr), burst_callback);
    }

    //
    // Sleep while the transfer is underway.
    //
    while(g_ui32BurstReadComplete == false);
    {
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);
    }

    // save the read data.
    for(i = 0; i < rx_size; i++)
    {
        *data++ = sBuffer.bytes[i];
    }

    am_hal_iom_disable(sBMI160.ui32IOMModule);
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
 * Below for pah8005 main code
*///--------------------------------------------------------------
bool
pah8005_init()
{
	PxiAlg_EnableAutoMode(1);
    PxiAlg_EnableMotionMode(0);
    PxiAlg_EnableFastOutput(0);
    PxiAlg_SetSigGradeThrd(40);
    PxiAlg_SetMemsScale(1);
	PxiAlg_EnableMEMS0SigGrade(0);
    //PxiAlg_Check_Not_Stable_First_HR(0);
    //PxiAlg_Set_Signal_Grade_Large_Energy_Num_Th_Lo(95);
	PxiAlg_Close();

    uint8_t tmp;
	writeRegister(0x7f, 0x00);
	writeRegister(0x7f, 0x00);
	tmp = readRegister(0x00);
	if(tmp == 0x30)
    {
        am_util_debug_printf("pah8005 I2C Link Successful!\n\r");
		if(readRegister(0x01) != 0xD5)
		{
			am_util_debug_printf("Not Support!!\n\r");
			while(1);
		}
    }
    else{
        am_util_debug_printf("pah8005 I2C Link Fail!  ID = %d\n\r", tmp);
		while(1);
		//return false;
	}

    //Initialization settings
    writeRegister(0x06, 0x82);  //Reset sensor
    am_util_delay_ms(10);    //make a delay

    InitialSettingLEDOn();

	return true;
}

uint8_t
readRegister(uint8_t addr)
{
    uint8_t data = 0 ;
    readRegisterA(addr, &data);

    return data;
}

bool
Pixart_HRD (void)
{
	if(_state == STATE_NO_TOUCH)
	{
  		writeRegister(0x7F,0x00); //bank0
		_touch_flag = readRegister(0x59) & 0x80;
		if(_touch_flag == 0x80)
		{
			_state = STATE_NORMAL;
		}
	}
	else
	{
		switch(_step)
		{
	    case 0:
			Scan_LEDOn(); //LED on
			_led_status = _led_on;
			_step++;
			break;
		case 1:
			GetRawData ();
			led_ctrl(_touch_flag);
			PowerDownSensor();
			_led_status = _led_on;
			if(_touch_flag != 0x80)
			{
				_state = STATE_NO_TOUCH;
			}
			_step ++ ;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
	      	_step++;
			break;
		case 9:
			_step = 0 ;
	  	default:
	    	break;
		}
	}

   return true;
}

void
GetRawData(void)
{
	ppg_mems_data_t ppg_mems_data;

	writeRegister(0x7F,0x01);  //bank1

	do
	{	//check status: 0 is not ready, 1 is ready, 2 is loss one data?
		ppg_mems_data.HRD_Data[0]=readRegister(0x68)&0x0f;
	}while(ppg_mems_data.HRD_Data[0] == 0);

	//Only support burst read (0x64~0x67), when using I2C interface
	burstreadRegister(0x64, &ppg_mems_data.HRD_Data[1], 4);

	writeRegister(0x7F,0x00); //bank0
	writeRegister(0x5D,0x01);
	//disable led
	writeRegister(0x7F,0x01); //bank1
	writeRegister(0x38,0xe0);
	writeRegister(0x7F,0x00); //bank0

	//Assign fixed number to PPG data
	ppg_mems_data.HRD_Data[8]= Frame_Count++;
	ppg_mems_data.HRD_Data[9]= Poll_Cnt;
	ppg_mems_data.HRD_Data[11]= readRegister(0x59) & 0x80; //Check Touch Flag

	//Assign fixed number to PPG data
	ppg_mems_data.HRD_Data[5]= 1;
	ppg_mems_data.HRD_Data[6]= 1;
	ppg_mems_data.HRD_Data[7]= 1;
	ppg_mems_data.HRD_Data[10]= 0;
	ppg_mems_data.HRD_Data[12]= 1;
	_touch_flag = ppg_mems_data.HRD_Data[11];
	Poll_Cnt = 0;

	//If no G sensor, please set G_Sensor_Data[3] = {0};
	uint8_t dataL, dataH;
	readBMI160(0x12, &dataL); readBMI160(0x13, &dataH);
	ppg_mems_data.MEMS_Data[0] = (int16_t)((uint16_t)dataH<<8 | dataL);
	readBMI160(0x14, &dataL); readBMI160(0x15, &dataH);
	ppg_mems_data.MEMS_Data[1] = (int16_t)((uint16_t)dataH<<8 | dataL);
	readBMI160(0x16, &dataL); readBMI160(0x17, &dataH);
	ppg_mems_data.MEMS_Data[2] = (int16_t)((uint16_t)dataH<<8 | dataL);
	/*
	readBMI160(0x12, &ppg_mems_data.HRD_Data[13]);//ReadGSensorXL;
	readBMI160(0x13, &ppg_mems_data.HRD_Data[14]);//ReadGSensorXH;
	readBMI160(0x14, &ppg_mems_data.HRD_Data[15]);//ReadGSensorYL;
	readBMI160(0x15, &ppg_mems_data.HRD_Data[16]);//ReadGSensorYH;
	readBMI160(0x16, &ppg_mems_data.HRD_Data[17]);//ReadGSensorZL;
	readBMI160(0x17, &ppg_mems_data.HRD_Data[18]);//ReadGSensorZH;*/

    if(Push(&ppg_mems_data)==false) //Save data into FIFO
    {
			am_util_debug_printf("FIFO overflow\n");
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

/***********************LED Control Start***********************************/
void
led_ctrl(uint8_t touch)
{
	if(touch == 0x80)
	{
		uint8_t data;
#if 1
		uint16_t                EP_L, EP_H, Exposure_Line;
#else
		uint16_t Frame_Average, EP_L, EP_H, Exposure_Line;
#endif

		writeRegister(0x7f,0x00);

		data = readRegister(0x33);
		EP_H=data&0x03;
		data = readRegister(0x32);
		EP_L=data;
		Exposure_Line=(EP_H<<8)+EP_L;

		writeRegister(0x7f,0x01);
		if(_sleepflag==1)
		{
			writeRegister(0x38, (0xE0|DEFAULT_LED_STEP));
			_sleepflag = 0 ;
		}

		if (_state_count <= STATE_COUNT_TH) {
			_state_count++;
			_led_current_change_flag = 0;
		}
		else {
			_state_count = 0;

			if(_state == 0)
			{
				if(	(Exposure_Line>=LED_CTRL_EXPO_TIME_HI_BOUND) ||
						(Exposure_Line<=LED_CTRL_EXPO_TIME_LOW_BOUND  )
					)
				{

					if( (Exposure_Line>=LED_CTRL_EXPO_TIME_HI_BOUND)
						&& (_led_step < LED_CURRENT_HI))
					{
						_state = 1 ;
						_led_step=_led_step+LED_INC_DEC_STEP;
						if(_led_step>LED_CURRENT_HI)
							_led_step=LED_CURRENT_HI;
						writeRegister(0x38, (_led_step|0xE0));
						_led_current_change_flag = 1;
					}
					else if((Exposure_Line<=LED_CTRL_EXPO_TIME_LOW_BOUND)
							&& (_led_step > LED_CURRENT_LOW))
					{
						_state = 2 ;
						if(_led_step<=(LED_CURRENT_LOW+LED_INC_DEC_STEP))
							_led_step=LED_CURRENT_LOW;
						else
							_led_step=_led_step-LED_INC_DEC_STEP;
						writeRegister(0x38, (_led_step|0xE0));
						_led_current_change_flag = 1;
					}else
					{
						_state = 0 ;
						_led_current_change_flag = 0;
					}
				}
				else {
					_led_current_change_flag = 0;
				}
			}
			else if(_state == 1)
			{
				if(Exposure_Line > LED_CTRL_EXPO_TIME_HI)
				{
					_state = 1 ;
					_led_step=_led_step+LED_INC_DEC_STEP;

					if(_led_step>=LED_CURRENT_HI)
					{
						_state = 0 ;
						_led_step=LED_CURRENT_HI;
					}
					writeRegister(0x38, (_led_step|0xE0));
					_led_current_change_flag = 1;
				}
				else
				{
					_state = 0 ;
					_led_current_change_flag = 0;
				}
			}
			else
			{
				if(Exposure_Line < LED_CTRL_EXPO_TIME_LOW)
				{
					_state = 2 ;
					if(_led_step<=(LED_CURRENT_LOW+LED_INC_DEC_STEP))
					{
						_state = 0 ;
						_led_step=LED_CURRENT_LOW;
					}
					else
						_led_step=_led_step-LED_INC_DEC_STEP;
					writeRegister(0x38, (_led_step|0xE0));
					_led_current_change_flag = 1;

				}
				else
				{
					_state = 0;
					_led_current_change_flag = 0;
				}
			}
		}
	}
	else
	{
		writeRegister(0x7f,0x00);
		writeRegister(0x05, 0xB8);
		writeRegister(0x7F, 0x01);
		_led_step = DEFAULT_LED_STEP;
		//writeRegister(0x38, (0xE0 | DEFAULT_LED_STEP));	//for Asian person only
		writeRegister(0x38, 0xFF);
		_sleepflag = 1;

		_led_current_change_flag = 0;
	}
}
/***********************LED Control End ***********************************/

void
InitialSettingLEDOn(void)
{
	uint8_t temp;
	writeRegister(0x7f,0x00);
	writeRegister(0x09,0x5A);
	writeRegister(0x05,0x98);//1000fps
	temp = readRegister(0x17);
	writeRegister(0x17,temp|0x80);

	writeRegister(0x20,0x0E); //AE off

	writeRegister(0x27,0xFF);
	writeRegister(0x28,0xFA);
	writeRegister(0x29,0x0A);
	writeRegister(0x2A,0xC8);
	writeRegister(0x2B,0xB4);
	writeRegister(0x2C,0x78);
	writeRegister(0x2D,0x64);
	writeRegister(0x42,0x20);
	//writeRegister(0x48,0x00);
	writeRegister(0x4D,0x1A);
	writeRegister(0x7A,0xB5); //1000fps
	writeRegister(0x7F,0x01);

	writeRegister(0x07,0x48);
	writeRegister(0x23,0x3c);
	writeRegister(0x26,0x0F);
	writeRegister(0x2E,0x48);
	writeRegister(0x36,_b1_0x36_on);
	writeRegister(0x37,_b1_0x37_on);
	writeRegister(0x38,0xEA);
	writeRegister(0x42,0xA4);
	writeRegister(0x43,0x41);
	writeRegister(0x44,0x41);
	writeRegister(0x45,0x26);
	writeRegister(0x46,0x00); //NO GAIN 512
	writeRegister(0x52,0x04); //16 frames
	writeRegister(0x53,0x28);
	writeRegister(0x56,0x60);
	writeRegister(0x57,0x28);
	writeRegister(0x6D,0x02); //1000fps
	writeRegister(0x0F,0xC8); //1000fps
}

void
Scan_LEDOn(void)
{
	writeRegister(0x7f,0x01); //for bank1
	writeRegister(0x38,( _led_step |0xE0));
	writeRegister(0x7F,0x00);
	writeRegister(0x20,0x0F); //AE on
	writeRegister(0x06,0x02); //leave power down
	writeRegister(0x05,0x99); //1000fps wake up sensor
	writeRegister(0x5D,0x81);
}

/*********************** PowerDownSensor start ***********************************/
void
PowerDownSensor(void)
{
	uint8_t b0_0x32_on;
	uint8_t b0_0x33_on;

	writeRegister(0x7F,0x00); //bank0

	b0_0x32_on = readRegister(0x32);
	b0_0x33_on = readRegister(0x33);

	_b1_0x37_on = b0_0x32_on ;
	_b1_0x36_on = 0x02 | (( b0_0x33_on & 0x3 ) << 5) ;

	if(_touch_flag == 0x80)	//if no touch, do not go to power down.
	{
		writeRegister(0x06,0x0A);
		writeRegister(0x7F,0x01);
		writeRegister(0x36,_b1_0x36_on);
		writeRegister(0x37,_b1_0x37_on);
	}
	else
	{
		writeRegister(0x7f,0x00);		//for bank0
		writeRegister(0x5, 0xB8);
		writeRegister(0x7F, 0x01);
		writeRegister(0x38, 0xFF);
		_b1_0x36_on = 0x02 ;
		_b1_0x37_on = 0xc8 ;
	}
}
/*********************** PowerDownSensor End ***********************************/

void
pah8005_task(void)
{
	if(!isFIFOEmpty())
	{
		if(Pop(&ppg_mems_data)) //Get data from FIFO
		{
			uint8_t tmp = PxiAlg_Process(ppg_mems_data.HRD_Data, ppg_mems_data.MEMS_Data);
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

	if( (HR_Cnt>HR_Rpt) && (_touch_flag==0x80) )
	{
		HR_Cnt = 0;
		am_util_debug_printf("HR: %d, SG: %3.2f\r\n", (uint8_t)myHR, grade);
	}
}
