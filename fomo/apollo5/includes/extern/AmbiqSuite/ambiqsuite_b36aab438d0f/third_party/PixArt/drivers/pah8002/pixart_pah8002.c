//*****************************************************************************
//
//! @file pixart_pah8002.c
//
//*****************************************************************************
#include <stdio.h>
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"

#include "pixart_pah8002.h"
#include "PAH8002Set.h"

//*****************************************************************************
//
// Forward declaration for the burst read callback function.
//
//*****************************************************************************
am_hal_iom_callback_t pixart_pah8002_yield;
am_hal_iom_callback_t pixart_pah8002_callback;

extern const uint8_t init_touch_register_array[][2];
extern const uint8_t init_ppg_register_array[][2];
extern const uint8_t suspend_register_array[][2];
extern const uint8_t init_stress_register_array[][2];
volatile uint32_t tick_cnt;
volatile uint32_t g_ui32BurstReadComplete = false;

const bosch_bmi160_t sBMI160 =
{
    .bMode = BMI160_MODE_SPI,
    .ui32IOMModule = AM_BSP_BMI160_IOM,
    .ui32ChipSelect = AM_BSP_BMI160_CS,
    .ui32Address = I2C_ID_ACCEL
};

const pixart_pah8002_t sPAH8002 =
{
    .bMode = PAH8002_MODE_I2C,
    .ui32IOMModule = 0,
    .ui32ChipSelect = 0,
    .ui32Address = I2C_ID_PAH8002
};

uint8_t
writeRegister(uint8_t addr, uint8_t data)
{
    am_hal_iom_buffer(4) sBuffer;
    sBuffer.bytes[0] = data;

    if (sPAH8002.bMode == PAH8002_MODE_I2C)
    {
        am_hal_iom_queue_i2c_write(sPAH8002.ui32IOMModule, sPAH8002.ui32Address,
                             sBuffer.words, 1, AM_HAL_IOM_OFFSET(addr), pixart_pah8002_callback);

        //
        // Wait for the IOM to complete.
        //
        pixart_pah8002_yield();
    }
    else
    {
        while(1);
    }
    return 0;
}

uint8_t
readRegister(uint8_t addr, uint8_t *data)
{
    am_hal_iom_buffer(4) sBuffer;

    if (sPAH8002.bMode == PAH8002_MODE_I2C)
    {
        am_hal_iom_queue_i2c_read(sPAH8002.ui32IOMModule, sPAH8002.ui32Address,
                             sBuffer.words, 1, AM_HAL_IOM_OFFSET(addr), pixart_pah8002_callback);

        //
        // Wait for the IOM to complete.
        //
        pixart_pah8002_yield();
    }
    else
    {
        while(1);
    }

    // save the read data.
    *data = sBuffer.bytes[0];
    return 0;
}

uint8_t
burstreadRegister(uint8_t addr, uint8_t *data, uint32_t rx_size)
{
    am_hal_iom_buffer(300) sBuffer;
    int i;

    g_ui32BurstReadComplete = false;

    if (sPAH8002.bMode == PAH8002_MODE_I2C)
    {
        while (rx_size)
        {
            if (rx_size > 250)
            {
                am_hal_iom_queue_i2c_read(sPAH8002.ui32IOMModule, sPAH8002.ui32Address,
                             sBuffer.words, 250, AM_HAL_IOM_OFFSET(addr), pixart_pah8002_callback);

                //
                // Wait for the IOM to complete.
                //
                pixart_pah8002_yield();

                // save the read data.
                for(i = 0; i < 250; i++)
                {
                    *data++ = sBuffer.bytes[i];
                }

                rx_size -= 250;
            }
            else
            {
                am_hal_iom_queue_i2c_read(sPAH8002.ui32IOMModule, sPAH8002.ui32Address,
                             sBuffer.words, rx_size, AM_HAL_IOM_OFFSET(addr), pixart_pah8002_callback);

                //
                // Wait for the IOM to complete.
                //
                pixart_pah8002_yield();

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

    g_ui32BurstReadComplete = false;
    am_hal_iom_enable(sBMI160.ui32IOMModule);
    am_hal_iom_int_clear(sBMI160.ui32IOMModule, AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR);
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

void
pixart_callbacks_config(am_hal_iom_callback_t pfnYield,
                                am_hal_iom_callback_t pfnCallback)
{
    pixart_pah8002_yield = pfnYield;
    pixart_pah8002_callback = pfnCallback;
}

void
pah8002_accel_set(int16_t *sensor_data, uint32_t sensor_data_size)
{
    _pah8002_data.nf_mems = sensor_data_size;
    _pah8002_data.mems_data = sensor_data;
}

bool
accelerometer_init(void)
{
    //int i = 0 ;
    uint8_t who_am_i = 0;
    //uint8_t data;

    readBMI160(0x7f, &who_am_i);	//dummy single spi read, recommended on datasheet
	readBMI160(0x00, &who_am_i);
    am_util_debug_printf("Accel ID = %X\n", who_am_i);
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

#if 0
void
accelerometer_get_fifo(int16_t **fifo, uint32_t *fifo_size)
{
    if(_bmi160_exist)
    {
        uint8_t tmp_L, tmp_H;
		readBMI160(0x7f, &tmp_L);	//dummy single spi read, recommended on datasheet
		readBMI160(0x00, &tmp_L);
		while(tmp_L != 0xD1)
		{
			am_util_debug_printf("BMI160 Not in Normal mode!!");
			am_util_debug_printf("R 03 %x\n", tmp_L);
			am_util_delay_ms(1); readBMI160(0x00, &tmp_L);
		}
		readBMI160(0x03, &tmp_L);
		while(tmp_L != 0x10)
		{
			am_util_debug_printf("BMI160 Not in Normal mode!!");
			am_util_debug_printf("R 03 %x\n", tmp_L);
			am_util_delay_ms(1); readBMI160(0x03, &tmp_L);
		}
        readBMI160(0x22, &tmp_L);
        readBMI160(0x23, &tmp_H);
        *fifo_size = (tmp_L + (tmp_H&0x07)*256) / 6;
        am_util_debug_printf("mems fifo size %d\n", *fifo_size);
        if(*fifo_size == 0){}
        else
            burstreadBMI160(0x24, (void *)(_g_sensor_data), (*fifo_size)*6);
    }
    else
    {
        *fifo_size = ACCE_FIFO_DEPTH ;
    }
    *fifo = _g_sensor_data;
}
#endif

void
accelerometer_start(void)
{
    //accelerometer_init();
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
 * Below for PAH8002 main code
*///--------------------------------------------------------------
uint32_t
get_sys_tick()
{
    return tick_cnt;
}
//++++++++++++++++++++++PAH8002 Functions++++++++++++++++++++++++++++++++++++++++++++++++++++
static bool
pah8002_sw_reset(void)
{
        uint8_t data ;
        am_util_debug_printf(">>> pah8002_sw_reset \r\n");
        pah8002_wakeup();

        if(0 != writeRegister(0x7f, 0x00))
        {
            goto RTN;
        }
        if(0 != readRegister(0, &data))
        {
            goto RTN;
        }
        am_util_debug_printf("ID = %d\r\n", data);
        if(data != 0x02)
        {
            goto RTN;
        }
        if(0 != writeRegister(0xe1, 0)) //write 0 to trigger Software Reset
        {
            goto RTN;
        }

        //delay 5ms
        am_util_delay_ms(5);
        am_util_debug_printf("<<< pah8002_sw_reset \r\n");
        return true;
RTN:
        return false;
}

static bool
pah8002_start(void)
{
        uint8_t data = 0;
        int samples_per_read =  HEART_RATE_MODE_SAMPLES_PER_READ ;
        am_util_debug_printf(">>> pah8002_start \r\n");

        pah8002_wakeup();

        if(0 != writeRegister(0x7f, 0x01))
        {
            goto RTN;
        }
        else if(0 != writeRegister(0xea, (samples_per_read+1)))
        {
            goto RTN;
        }
        else if(0 != writeRegister(0xd5, 1))    //TG enable. REQTIMER_ENABLE
        {
            goto RTN;
        }
        else if(0 != readRegister(0xd5, &data)) //TG enable. REQTIMER_ENABLE
        {
            goto RTN;
        }
        pah8002_check();
        am_util_debug_printf("<<< pah8002_start %d\r\n",data);

        return true;

RTN:
        return false;
}

static bool
pah8002_touch_mode_init(void)
{
    int i =0 ;

    am_util_debug_printf(">>> pah8002_touch_mode_init \r\n");
    pah8002_wakeup();
    for(i = 0; i < INIT_TOUCH_REG_ARRAY_SIZE;i++)
    {
        if ( writeRegister(  init_touch_register_array[i][0], init_touch_register_array[i][1]) != 0 )
        {
           goto RTN;
        }
    }
    am_util_debug_printf("<<< pah8002_touch_mode_init \r\n");
    return true;
RTN:
    return false;
}

static bool pah8002_normal_mode_init(void)
{
    int i =0 ;
    am_util_debug_printf(">>> pah8002_normal_mode_init \r\n");
    pah8002_wakeup();
    for(i = 0; i < INIT_PPG_REG_ARRAY_SIZE;i++)
    {
        if ( writeRegister( init_ppg_register_array[i][0], init_ppg_register_array[i][1]) != 0 )
        {
           goto RTN;
        }
    }

    am_util_debug_printf("<<< pah8002_normal_mode_init \r\n");

    return true;
RTN:
    return false;
}

//static bool
//pah8002_stress_mode_init(void)
//{
//    int i =0 ;

//    am_util_debug_printf(">>> pah8002_stress_mode_init \r\n");
//    pah8002_wakeup();
//    for(i = 0; i < INIT_STRESS_REG_ARRAY_SIZE;i++)
//    {
//        if ( writeRegister(  init_stress_register_array[i][0], init_stress_register_array[i][1]) != 0 )
//        {
//           goto RTN;
//        }
//     }

//    am_util_debug_printf("<<< pah8002_stress_mode_init \r\n");

//    return true;
//RTN:
//    return false;
//}

static bool
pah8002_enter_touch_mode(void)
{
    am_util_debug_printf(">>> pah8002_enter_touch_mode\r\n");
	if(_mode == TOUCH_MODE) return true;

    //1. software reset
    if( !pah8002_sw_reset() )
        goto RTN;

    //2. load registers for touch mode
    if( !pah8002_touch_mode_init())
        goto RTN;

    //3. enable sensor
    if( !pah8002_start())
        goto RTN;

    _mode = TOUCH_MODE;
    am_util_debug_printf("<<< pah8002_enter_touch_mode\r\n");
    return true;

RTN:
    return false ;
}

//static bool
//pah8002_get_touch_flag( uint8_t *touch_flag)
//{
//    am_util_debug_printf(">>> pah8002_touch_status \r\n");
//    pah8002_wakeup();
//    if(0 != writeRegister(0x7f, 0x02))
//    {
//        goto RTN;
//    }
//    else if(0 != readRegister(0x45, touch_flag))    //
//    {
//        goto RTN;
//    }

//    am_util_debug_printf("<<< pah8002_touch_status %d\r\n", *touch_flag);

//    return true;

//RTN:
//    return false;
//}

static bool
pah8002_enter_normal_mode(void)
{
    am_util_debug_printf(">>> pah8002_enter_normal_mode\r\n");
	if(_mode == NORMAL_MODE) return true;

    //1. software reset
    if( !pah8002_sw_reset())
        goto RTN;

    //2. load registers for normal mode
    if( !pah8002_normal_mode_init())
        goto RTN;

    writeRegister(0x7f, 0x00);  //Bank0
    readRegister(0x0D, &_ir_expo);  // IR Exposure Time
    writeRegister(0x7f, 0x01);  //Bank1
    readRegister(0xBA, &_ir_dac);   //IR Led DAC

    //3. enable sensor
    if( !pah8002_start())
        goto RTN;
    _mode = NORMAL_MODE;
    am_util_debug_printf("<<< pah8002_enter_normal_mode ir_dac %x, ir_expo %x\r\n", _ir_dac, _ir_expo);
    return true;
RTN:
    return false ;
}

static uint8_t
pah8002_get_touch_flag_ppg_mode(void)
{
    static uint8_t touch_sts_output = 1 ;
    int32_t *s = (int32_t *)pah8002_ppg_data ;
    int32_t ch0 ;
    int32_t ch1 ;
    int64_t ir_rawdata;
    int i;
    static int touch_cnt = 0, no_touch_cnt = 0 ;

    #define TouchDetection_Upper_TH (600)
    #define TouchDetection_Lower_TH (512)

    #define TouchDetection_Count_TH (3)             //(3+1)*50ms = 200ms
    #define NoTouchDetection_Count_TH (3)           //(3+1)*50ms = 200ms


    for(i=0; i<HEART_RATE_MODE_SAMPLES_PER_READ; i+=TOTAL_CHANNELS)
    {
        ch0 = *s;
        ch1 = *(s+1);
        ir_rawdata = ch0 - ch1 ;
        ir_rawdata = (ir_rawdata * _ir_dac * _ir_expo)>>20 ;

        if( ir_rawdata > TouchDetection_Upper_TH)
        {
            touch_cnt++;
            no_touch_cnt = 0;
        }
        else if( ir_rawdata < TouchDetection_Lower_TH)
        {
            no_touch_cnt++;
            touch_cnt = 0 ;
        }
        else
        {
            touch_cnt = 0 ;
            no_touch_cnt = 0;
        }

        s+=TOTAL_CHANNELS;
    }

    if(touch_cnt > TouchDetection_Count_TH)
    {
        touch_sts_output = 1;
    }
    else if( no_touch_cnt > NoTouchDetection_Count_TH)
    {
        touch_sts_output = 0;
    }

    am_util_debug_printf("<<< pah8002_get_touch_flag_ppg_mode %d, %d\n",touch_cnt, no_touch_cnt);
    am_util_debug_printf("<<< pah8002_get_touch_flag_ppg_mode %d\n", touch_sts_output);

    return touch_sts_output;
}

//static bool
//pah8002_enter_stress_mode(void)
//{
//    am_util_debug_printf(">>> pah8002_enter_stress_mode\r\n");
//	if(_mode == STRESS_MODE) return true;

//    //1. software reset
//    if( !pah8002_sw_reset())
//        goto RTN;

//    //2. load registers for normal mode
//    if( !pah8002_stress_mode_init())
//        goto RTN;

//    writeRegister(0x7f, 0x00);  //Bank0
//    readRegister(0x0D, &_ir_expo);  // IR Exposure Time
//    writeRegister(0x7f, 0x01);  //Bank1
//    readRegister(0xBA, &_ir_dac);   //IR Led DAC

//    //3. enable sensor
//    if( !pah8002_start())
//        goto RTN;

//    _mode = STRESS_MODE;
//    am_util_debug_printf("<<< pah8002_enter_stress_mode \r\n");
//    return true;
//RTN:
//    return false ;
//}

static int
pah8002_wakeup(void)
{
    int retry = 0 ;
    int success = 0 ;
    uint8_t data = 0 ;
    readRegister(0, &data);
    readRegister(0, &data);

    do
    {
        writeRegister(0x7f, 0x00);
        readRegister(0, &data);
        if(data == 0x02) success++;
        else success = 0 ;

        if(success >=2) break;
        retry ++;

    }while(retry < 20);

    if(_chip_id == 0)
    {
        readRegister(0x02, &data);

        _chip_id = data & 0xF0 ;
        if(_chip_id != 0xD0)
        {
            //am_util_debug_printf("Not support anymore\r\n");
            while(1);
            /*am_util_debug_printf("============================================================================================\r\n");
            am_util_debug_printf("============================================================================================\r\n");
            am_util_debug_printf("==== This is old revision of PAH8002, please contact PixArt to have most updated chip!! ====\r\n");
            am_util_debug_printf("============================================================================================\r\n");
            am_util_debug_printf("============================================================================================\r\n");*/

        }
    }

    writeRegister(0x7f, 0x02);
    writeRegister(0x70, 0x00);

    am_util_debug_printf("pah8002_wakeup retry %d \r\n", retry);

    return retry;
}

static int
pah8002_check(void)
{
    int retry = 0 ;
    int success = 0 ;
    uint8_t data = 0 ;
    uint8_t b1_0xd5 = 0 ;
    uint8_t b1_0xe6 = 0 ;
    readRegister(0, &data);
    readRegister(0, &data);

    do
    {
        writeRegister(0x7f, 0x00);
        readRegister(0, &data);
        if(data == 0x02) success++;
        else success = 0 ;

        if(success >=2) break;
        retry ++;
    }while(retry < 20);

    writeRegister(0x7f, 0x01);
    readRegister(0xd5, &b1_0xd5);
    readRegister(0xe6, &b1_0xe6);
    am_util_debug_printf("pah8002_check retry %d \r\n", retry);
    if(b1_0xd5 != 1)
        am_util_debug_printf("pah8002_check error Bank1 0xD5 0x%x \r\n", b1_0xd5);
    if(b1_0xe6 != 0xC8)
        am_util_debug_printf("pah8002_check error Bank1 0xE6 0x%x \r\n", b1_0xe6);
    return retry;
}

static bool
pah8002_enter_suspend_mode(void)
{
    int i = 0 ;

    am_util_debug_printf("pah8002_enter_suspend_mode");
    pah8002_sw_reset();

    for(i = 0; i < SUSPEND_REG_ARRAY_SIZE;i++)
    {
        if ( writeRegister(suspend_register_array[i][0],
                                suspend_register_array[i][1]) != 0 )
        {
           return false;
        }
    }
    _mode = SUSPEND_MODE;
    pah8002_check();
    return true;
}

static bool
_pah8002_task(void)
{
    uint8_t cks[4] ;
    uint8_t int_req = 0;

    am_util_debug_printf(">>> pah8002_task\n");
    pah8002_wakeup();
    if(0 != writeRegister(0x7f, 0x02))
    {}
    else if(0 != readRegister(0x73, &int_req))
    {}
    else
    {
        if( (int_req & 0x04) != 0)
        {
            //overflow
            while(1);
        }

        if( (int_req & 0x02) != 0)
        {
            //touch
            am_util_debug_printf("touch interrupt\n");
        }

        if( (int_req & 0x08) != 0)
        {
            //overflow
            while(1);
        }


        if( (int_req & 0x01) != 0)
        {
            int samples_per_read = HEART_RATE_MODE_SAMPLES_PER_READ ;
            am_util_debug_printf("FIFO interrupt\n");
            //pah8002_get_touch_flag(&state->pah8002_touch_flag);
            if(0 != writeRegister(0x7f, 0x03))
            {}
            else if(0 != burstreadRegister(0, pah8002_ppg_data, samples_per_read*4))
            {}
            else if(0 != writeRegister(0x7f, 0x02))
            {}
            else if(0 != burstreadRegister(0x80, cks, 4))
            {}
            else if(0 != writeRegister(0x75, 0x01)) //read fifo first, then clear SRAM FIFO interrupt
            {}
            else if(0 != writeRegister(0x75, 0x00))
            {}
            else
            {
                uint32_t *s = (uint32_t *)pah8002_ppg_data ;
                uint32_t cks_cal = *s;
                uint32_t cks_rx = *((uint32_t *)cks) ;
                uint32_t i ;

                //checksum compare
                for(i=1; i<samples_per_read; i++)
                {
                    cks_cal = cks_cal ^ (*(s+i)) ;
                }

                if(cks_cal != cks_rx)
                {
                    am_util_debug_printf("checksum error\r\n");
                }
                else
                {
                    am_util_debug_printf("checksum OK %d\r\n", cks_cal);
                }
                _touch_flag = pah8002_get_touch_flag_ppg_mode();
            }
        }
        else
        {
            am_util_debug_printf("not fifo interrupt%d\r\n", int_req);
        }
    }

    am_util_debug_printf("<<< pah8002_task\n");
    return true;
}

static bool
pah8002_normal_long_et_mode_init()
{
  int i =0 ;
  am_util_debug_printf(">>> pah8002_normal_long_et_mode_init \r\n");
  pah8002_wakeup();
  for(i = 0; i < INIT_PPG_LONG_REG_ARRAY_SIZE;i++)
  {
      if ( writeRegister( init_ppg_long_register_array[i][0],
                                init_ppg_long_register_array[i][1]) != 0 )
      {
         goto RTN;
      }
    }

  am_util_debug_printf("<<< pah8002_normal_long_et_mode_init \r\n");

  return true;
RTN:
  return false;
}
static bool
pah8002_enter_normal_long_et_mode()
{
  am_util_debug_printf(">>> pah8002_enter_normal_long_et_mode\r\n");
  if(_mode == NORMAL_LONG_ET_MODE) return true;

  //1. software reset
  if( !pah8002_sw_reset())
    goto RTN;

//2. load registers for normal mode
  if( !pah8002_normal_long_et_mode_init())
    goto RTN;

  writeRegister(0x7f, 0x00);  //Bank0
  readRegister(0x0D, &_ir_expo);  // IR Exposure Time
  writeRegister(0x7f, 0x01);  //Bank1
  readRegister(0xBA, &_ir_dac);  //IR Led DAC

  //3. enable sensor
  if( !pah8002_start())
    goto RTN;
  _mode = NORMAL_LONG_ET_MODE;
  am_util_debug_printf("<<< pah8002_enter_normal_long_et_mode ir_dac %x, ir_expo %x\r\n", _ir_dac, _ir_expo);
  return true;
RTN:
  return false ;
}


static void
pah8002_dyn_switch_ppg_mode()
{
  uint8_t b2a4, b2a5 ;
  uint16_t value ;
  pah8002_wakeup();
  writeRegister(0x7F, 0x02);
  readRegister(0xa4, &b2a4);
  readRegister(0xa5, &b2a5);
  value = b2a5 ;
  value <<= 8 ;
  value += b2a4 ;
  if (value >  4639)
  {
    pah8002_enter_normal_long_et_mode();
  }
}

//---------------------------------------PAH8002 functions-----------------------------------------------
bool
pah8002_init(void)
{
    //Algorithm initialization
    _pah8002_data.frame_count = 0 ;
    _pah8002_data.nf_ppg_channel = TOTAL_CHANNELS_FOR_ALG;
    _pah8002_data.nf_ppg_per_channel = HEART_RATE_MODE_SAMPLES_PER_CH_READ;
    _pah8002_data.ppg_data = (int32_t *)pah8002_ppg_data;
#ifdef MEMS_ZERO
    memset(_mems_data, 0, sizeof(_mems_data));
    _pah8002_data.nf_mems = HEART_RATE_MODE_SAMPLES_PER_CH_READ;
    _pah8002_data.mems_data = _mems_data;
#endif
    pah8002_open();
    #ifdef LOG_8002
    am_util_debug_printf("PPG CH#, %d\n", TOTAL_CHANNELS_FOR_ALG);
    #endif
    am_util_delay_ms(300);
#ifdef PPG_MODE_ONLY
    return pah8002_enter_normal_mode();
#else
    return pah8002_enter_touch_mode();
#endif
}

void
pah8002_log(void)
{
    int i = 0 ;
    uint32_t *ppg_data = (uint32_t *)_pah8002_data.ppg_data ;
    int16_t *mems_data = _pah8002_data.mems_data ;
    #ifdef LOG_8002
    am_util_debug_printf("Frame Count, %d \n", _pah8002_data.frame_count);
    am_util_debug_printf("Time, %d \n", _pah8002_data.time);
    am_util_debug_printf("PPG, %d, %d, ", _pah8002_data.touch_flag, _pah8002_data.nf_ppg_per_channel);
    #endif
    for(i=0; i<_pah8002_data.nf_ppg_channel * _pah8002_data.nf_ppg_per_channel; i++)
    {
        #ifdef LOG_8002
        am_util_debug_printf("%d, ", *ppg_data);
        #endif
        ppg_data ++;
    }
    #ifdef LOG_8002
    am_util_debug_printf("\n");
    am_util_debug_printf("MEMS, %d, ", _pah8002_data.nf_mems);
    #endif
    for(i=0; i<_pah8002_data.nf_mems*3; i++)
    {
        #ifdef LOG_8002
        am_util_debug_printf("%d, ", *mems_data);
        #endif
        mems_data ++;
    }
    #ifdef LOG_8002
    am_util_debug_printf("\n");
    #endif
}

static void
data_convert_4ch_to_3ch(uint32_t *pdata, uint32_t len)
{
    uint32_t i = 0, j = 0;
    for(i=0, j=2; j<len; i+=3, j+=4)
    {
        *(pdata+i+1) = *(pdata+j);
        *(pdata+i+2) = *(pdata+j+1);
    }
}

uint8_t
pah8002_task(void)
{
    uint8_t ret;
    float hr = 0 ;
    uint32_t sys_tick;
    if(_pah8002_interrupt == 1)
    {	//uint32_t sssys_tick0 = get_sys_tick();
        if(_mode == TOUCH_MODE)
        {
            pah8002_enter_normal_mode();
            _timestamp = get_sys_tick();
            accelerometer_start();
        }
        else if(_mode == NORMAL_MODE)
        {
            _pah8002_task();
			pah8002_dyn_switch_ppg_mode();

#ifdef PPG_MODE_ONLY
#else
            if(_touch_flag == 0)
            {
                pah8002_enter_touch_mode();
                accelerometer_stop();
            }
#endif

            //process algorithm
#ifdef MEMS_ZERO
#else
            //accelerometer_get_fifo(&_pah8002_data.mems_data, &_pah8002_data.nf_mems);
#endif
            sys_tick = get_sys_tick();
            _pah8002_data.time = sys_tick - _timestamp;
            _timestamp = sys_tick;
            _pah8002_data.touch_flag = _touch_flag;
            data_convert_4ch_to_3ch((uint32_t *)pah8002_ppg_data, HEART_RATE_MODE_SAMPLES_PER_READ);
			//uint32_t sssys_tick1 = get_sys_tick();
            ret = pah8002_entrance(&_pah8002_data);
			//uint32_t sssys_tick2 = get_sys_tick();
            if((ret & 0x0f) != 0)
            {
                switch(ret) //check error status
                {
                    case MSG_ALG_NOT_OPEN:
                        am_util_debug_printf("Algorithm is not initialized.\r\n");
                        break;
                    case MSG_MEMS_LEN_TOO_SHORT:
                        am_util_debug_printf("MEMS data length is shorter than PPG data length.\r\n");
                        break;
                    case MSG_NO_TOUCH:
                        am_util_debug_printf("PPG is no touch.\r\n");
                        hr = 0xFF;
                        break;
                    case MSG_PPG_LEN_TOO_SHORT:
                        am_util_debug_printf("PPG data length is too short.\r\n");
                        break;
                    case MSG_FRAME_LOSS:
                        am_util_debug_printf("Frame count is not continuous.\r\n");
                        break;
                }
            }
            pah8002_log();
            if((ret & 0xf0) == MSG_HR_READY)
            {
                pah8002_get_hr(&hr);
				am_util_debug_printf("HR = %d\r\n", (int)(hr));
            }
            _pah8002_data.frame_count++;
        }
        _pah8002_interrupt = 0;        
		//uint32_t sssys_tick3 = get_sys_tick(); am_util_debug_printf("8002 Task Time = %d\r\n", sssys_tick3-sssys_tick0);
    }
    return (uint8_t) hr;
}

void
pah8002_intr_isr(void)
{
    _pah8002_interrupt = 1;
}

void
pah8002_low_power_enable(void)
{
    //
    // Enter Power down mode.
    //
    pah8002_enter_suspend_mode();
}

void
pah8002_low_power_disable(void)
{
    //
    // Enter normal mode.
    //
    pah8002_enter_touch_mode();
}
