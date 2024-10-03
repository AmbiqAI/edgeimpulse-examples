//*****************************************************************************
//
//! @file am_devices_mcp79410.c
//!
//! @brief This is a driver for the Micochip MCP79410 RTC.
//!
//! @addtogroup mcp79410 MCP79410 RTC Driver
//! @ingroup devices
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_bsp.h"
#include "am_util.h"
#include "am_devices_mcp79410.h"

//*****************************************************************************
//
//! Global variables.
//
//*****************************************************************************
am_hal_iom_i2c_device_t *g_psIOMSettings;

//*****************************************************************************
//
//  Initialize the MCP79410 structure.
//
//*****************************************************************************
void
am_devices_mcp79410_init(am_hal_iom_i2c_device_t *psIOMSettings)
{
    //
    // Initialize the IOM settings for the MCP79410.
    //
    g_psIOMSettings = psIOMSettings;
}

//*****************************************************************************
//
//  Start the oscillator
//
//*****************************************************************************
int
am_devices_mcp79410_start(void)
{
    uint32_t ui32Temp;
    uint8_t *pui8Temp;

    pui8Temp = (uint8_t *) (&ui32Temp);

    //
    // Read Second.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCSEC));

    //
    // Or in Seconds with Start Bit.
    //
    *pui8Temp = 0x80 | *pui8Temp;

    //
    // Start the RTC OSC.
    //
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCSEC));

    //
    // Read OSC. status.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCWKDAY));

    //
    // Is the oscillator started and running?
    //
    if (!(*pui8Temp & 0x20))
    {
        //
        // return error.
        //
        return -1;
    }

    return 0;
}

//*****************************************************************************
//
//  Stop the oscillator
//
//*****************************************************************************
int
am_devices_mcp79410_stop(void)
{
    uint32_t ui32Temp;
    uint8_t *pui8Temp;

    pui8Temp = (uint8_t *) (&ui32Temp);

    //
    // Read Second.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCSEC));

    //
    // Clear start bit.
    //
    *pui8Temp = ~0x80 & *pui8Temp;

    //
    // Stop the RTC OSC.
    //
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCSEC));

    //
    // Read OSC. status.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCWKDAY));

    //
    // Is the oscillator stopped?
    //
    if (*pui8Temp & 0x20)
    {
        //
        // return error.
        //
        return -1;
    }

    return 0;
}

//*****************************************************************************
//
//  Enable VBAT.
//
//*****************************************************************************
int
am_devices_mcp79410_vbat_enable(void)
{
    uint32_t ui32Temp;
    uint8_t *pui8Temp;

    pui8Temp = (uint8_t *) (&ui32Temp);

    //
    // Read Weekday register.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCWKDAY));

    //
    // Or in Weekday with VBAT enable.
    //
    *pui8Temp = 0x8 | *pui8Temp;

    //
    // Enable VBAT supply.
    //
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCWKDAY));

    //
    // Read OSC. status.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCWKDAY));

    //
    // Is vbat enabled?
    //
    if (!(*pui8Temp & 0x8))
    {
        //
        // return error.
        //
        return -1;
    }

    return 0;
}

//*****************************************************************************
//
//  Set the time of day in the MCP79410 RTC
//
//*****************************************************************************
int
am_devices_mcp79410_time_set(am_devices_mcp79410_time_t *pTime)
{
    uint32_t ui32Temp;
    uint8_t *pui8Temp;

    pui8Temp = (uint8_t *) (&ui32Temp);

    //
    // Write Second.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->Second);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCSEC));

    //
    // Write Minute.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->Minute);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCMIN));

    //
    // Write Hour.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->Hour);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCHOUR));

    //
    // Write Weekday.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->Weekday);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCWKDAY));

    //
    // Write Date.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->DayOfMonth);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCDATE));

    //
    // Write Year.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->Year);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCYEAR));

    return 0;
}

//*****************************************************************************
//
// Get the time of day from the MCP79410 RTC
//
//*****************************************************************************
int
am_devices_mcp79410_time_get(am_devices_mcp79410_time_t *pTime)
{
    uint32_t ui32Temp;
    uint8_t *pui8Temp;

    pui8Temp = (uint8_t *) (&ui32Temp);

    //
    // Read Second.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCSEC));

    pTime->Second = am_util_math_bcd_to_dec((*pui8Temp) & (~0x80));

    //
    // Read Minute.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCMIN));

    pTime->Minute = am_util_math_bcd_to_dec(*pui8Temp);

    //
    // Read Hour.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCHOUR));

    pTime->Hour = am_util_math_bcd_to_dec(*pui8Temp);

    //
    // Read Weekday.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCWKDAY));

    pTime->Weekday = am_util_math_bcd_to_dec((*pui8Temp) & 0x7);

    //
    // Read Date.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCDATE));

    pTime->DayOfMonth = am_util_math_bcd_to_dec(*pui8Temp);

    //
    // Read Year.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_RTCYEAR));

    pTime->Year = am_util_math_bcd_to_dec(*pui8Temp);

    //
    // return success.
    //
    return 0;
}

//*****************************************************************************
//
//  Set the alarm in the MCP79410 RTC
//
//*****************************************************************************
int
am_devices_mcp79410_alarm_set(am_devices_mcp79410_time_t *pTime)
{
    uint32_t ui32Temp;
    uint8_t *pui8Temp;

    pui8Temp = (uint8_t *) (&ui32Temp);

    //
    // Write Second.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->Second);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0SEC));

    //
    // Write Minute.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->Minute);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0MIN));

    //
    // Write Hour.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->Hour);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0HOUR));

    //
    // Write Weekday.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->Weekday);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0WKDAY));

    //
    // Write Date.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->DayOfMonth);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0DATE));

    //
    // Write Year.
    //
    *pui8Temp = am_util_math_dec_to_bcd(pTime->Month);
    am_hal_iom_i2c_write(g_psIOMSettings->ui32Module,
                         g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                         AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0MTH));

    //
    // return success.
    //
    return 0;
}

//*****************************************************************************
//
//  Get the alarm time from the MCP79410 RTC
//
//*****************************************************************************
int am_devices_mcp79410_alarm_get(am_devices_mcp79410_time_t *pTime)
{
    uint32_t ui32Temp;
    uint8_t *pui8Temp;

    pui8Temp = (uint8_t *) (&ui32Temp);

    //
    // Read Second.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0SEC));

    pTime->Second = am_util_math_bcd_to_dec(*pui8Temp);

    //
    // Read Minute.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0MIN));

    pTime->Minute = am_util_math_bcd_to_dec(*pui8Temp);

    //
    // Read Hour.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0HOUR));

    pTime->Hour = am_util_math_bcd_to_dec(*pui8Temp);

    //
    // Read Weekday.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0WKDAY));

    pTime->Weekday = am_util_math_bcd_to_dec(*pui8Temp);

    //
    // Read Date.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0DATE));

    pTime->DayOfMonth = am_util_math_bcd_to_dec(*pui8Temp);

    //
    // Read Month.
    //
    am_hal_iom_i2c_read(g_psIOMSettings->ui32Module,
                        g_psIOMSettings->ui32BusAddress, &ui32Temp, 1,
                        AM_HAL_IOM_OFFSET(AM_DEVICES_MCP79410_ALM0MTH));

    pTime->Month = am_util_math_bcd_to_dec(*pui8Temp);

    //
    // return success.
    //
    return 0;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

