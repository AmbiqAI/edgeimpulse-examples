//*****************************************************************************
//
//! @file usb_descriptors.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "tusb.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]       MIDI | HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};


//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
static char const *string_desc_arr[] =
{
    (const char[]) {0x09, 0x04}, // 0: is supported language is English (0x0409)
    "TinyUSB",                     // 1: Manufacturer
    "TinyUSB Device",              // 2: Product
    "123456",                      // 3: Serials, should use chip ID
    "TinyUSB CDC",                 // 4: CDC Interface
};

static uint16_t _desc_str[32];

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
    ITF_NUM_CDC_0 = 0,
    ITF_NUM_CDC_0_DATA,
    ITF_NUM_CDC_1,
    ITF_NUM_CDC_1_DATA,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + CFG_TUD_CDC * TUD_CDC_DESC_LEN)

  #define EPNUM_CDC_0_NOTIF   0x81
  #define EPNUM_CDC_0_DATA    0x02

  #define EPNUM_CDC_1_NOTIF   0x83
  #define EPNUM_CDC_1_DATA    0x04

#define POWER 100
#define CDC_FS_EP_LEN 0x40
#define CDC_HS_EP_LEN  0x200

static uint8_t const desc_fs_configuration[] =
{
    //
    // Config number, interface count, string index, total length, attribute, power in mA
    //
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, POWER),

    //
    // 1st CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    //
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 4, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_DATA, 0x80 | EPNUM_CDC_0_DATA, CDC_FS_EP_LEN),

    //
    // 2nd CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    //
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1, 4, EPNUM_CDC_1_NOTIF, 8, EPNUM_CDC_1_DATA, 0x80 | EPNUM_CDC_1_DATA, CDC_FS_EP_LEN),
};

#if TUD_OPT_HIGH_SPEED
static uint8_t const desc_hs_configuration[] =
{
    //
    // Config number, interface count, string index, total length, attribute, power in mA
    //
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, POWER),

    //
    // 1st CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    //
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 4, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_DATA, 0x80 | EPNUM_CDC_0_DATA, CDC_HS_EP_LEN),

    //
    // 2nd CDC: Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    //
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1, 4, EPNUM_CDC_1_NOTIF, 8, EPNUM_CDC_1_DATA, 0x80 | EPNUM_CDC_1_DATA, CDC_HS_EP_LEN),
};

const uint8_t desc_hs_dev_qualifier_desc_hs[] =
{
    10, // len
    TUSB_DESC_DEVICE_QUALIFIER,  // dev qual desc type = 6
    U16_TO_U8S_LE(0x200),
    0, //class @todo
    0, //subclass @todo
    0, // protocol
    CDC_FS_EP_LEN, //
    1, // number of other speed configs
    0, // reserved, must be zero

};

static const uint8_t desc_hs_dev_qualifier_desc_fs[] =
{
    10, // len
    TUSB_DESC_DEVICE_QUALIFIER,  // dev qual desc type = 6
    U16_TO_U8S_LE(0x200), //
    0, //class @todo
    0, //subclass @todo
    0, // protocol @todo
    CDC_FS_EP_LEN, //
    1, // number of other speed configs
    0, // reserved, must be zero

};

static const uint8_t desc_hs_dev_other_speed_desc_hs[] =
{
    9, // len
    TUSB_DESC_OTHER_SPEED_CONFIG,  // dev qual desc type = 7
    U16_TO_U8S_LE(CONFIG_TOTAL_LEN),      // total length of this descitpr and everything else
    ITF_NUM_TOTAL, //num interfaces         2 interfaces here
    1, // config value
    0, // index of string descriptor
    TU_BIT(7) | TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, // bitmask
    POWER, // power ma

    //
    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    //
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 4, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_DATA, 0x80 | EPNUM_CDC_0_DATA, CDC_FS_EP_LEN),

    //
    // Interface number, string index, EP Out & EP In address, EP size
    //
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1, 4, EPNUM_CDC_1_NOTIF, 8, EPNUM_CDC_1_DATA, 0x80 | EPNUM_CDC_1_DATA, CDC_FS_EP_LEN),

};

static const uint8_t desc_hs_dev_other_speed_desc_fs[] =
{
    9, // len
    TUSB_DESC_OTHER_SPEED_CONFIG,  // dev qual desc type = 7
    U16_TO_U8S_LE(CONFIG_TOTAL_LEN),      // @todo not sure what this is
    ITF_NUM_TOTAL, //num interfaces
    1, // config value
    0, // index of string descriptor
    TU_BIT(7) | TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, // bitmask
    POWER, // power ma


    //
    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    //
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_0, 4, EPNUM_CDC_0_NOTIF, 8, EPNUM_CDC_0_DATA, 0x80 | EPNUM_CDC_0_DATA, CDC_HS_EP_LEN),

    //
    // Interface number, string index, EP Out & EP In address, EP size
    //
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC_1, 4, EPNUM_CDC_1_NOTIF, 8, EPNUM_CDC_1_DATA, 0x80 | EPNUM_CDC_1_DATA, CDC_HS_EP_LEN),


};
#endif // TUD_OPT_HIGH_SPEED


// ********************** executable functions *****************************

//*****************************************************************************
//! @brief Invoked when received GET DEVICE DESCRIPTOR
//! @return Application return pointer to descriptor
//*****************************************************************************
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *) &desc_device;
}
//*****************************************************************************
//! @brief implement tinyusb callback for device qualifier
//! @return
//*****************************************************************************
const uint8_t *tud_descriptor_device_qualifier_cb(void)
{
#if TUD_OPT_HIGH_SPEED
    //
    // return a pointer to a device qulifier descriptor
    // these can be different for FS and HS
    //
    return (tud_speed_get() == TUSB_SPEED_HIGH) ? desc_hs_dev_qualifier_desc_hs
                                                : desc_hs_dev_qualifier_desc_fs;
#else // TUD_OPT_HIGH_SPEED
    //
    // if not high speed capable, stall this request
    //
    return 0 ;
#endif // TUD_OPT_HIGH_SPEED

}

//*****************************************************************************
//! @brief implement tinyusb callback for other speed config descirptor
//! @param index
//! @return Application return pointer to descriptor.
//*****************************************************************************
const uint8_t *tud_descriptor_other_speed_configuration_cb(uint8_t index)
{
#if TUD_OPT_HIGH_SPEED
    //
    // return a pointer to a device qulifier descriptor
    // these can be different for FS and HS
    //
    return (tud_speed_get() == TUSB_SPEED_HIGH) ? desc_hs_dev_other_speed_desc_hs
                                                : desc_hs_dev_other_speed_desc_fs;
#else // TUD_OPT_HIGH_SPEED
    //
    // if not high speed capable, stall this request
    //
    return 0 ;
#endif // TUD_OPT_HIGH_SPEED

}
//*****************************************************************************
//! @brief Invoked when received GET CONFIGURATION DESCRIPTOR
//!
//! @note Descriptor contents must exist long enough for transfer to complete
//!
//! @param index
//! @return Application return pointer to descriptor.
//*****************************************************************************
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index; // for multiple configurations

#if TUD_OPT_HIGH_SPEED
    // Although we are highspeed, host may be fullspeed.
    return (tud_speed_get() == TUSB_SPEED_HIGH) ?  desc_hs_configuration : desc_fs_configuration;
#else
    return desc_fs_configuration;
#endif
}

//*****************************************************************************
//
//! @brief Invoked when received GET STRING DESCRIPTOR request
//!
//! @return Application return pointer to descriptor.
//
//*****************************************************************************
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void) langid;

    uint8_t chr_count;

    if ( index == 0)
    {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        //
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors
        //

        if ( !(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) )
        {
            return NULL;
        }

        const char* str = string_desc_arr[index];

        //
        // Cap at max char
        //
        chr_count = strlen(str);
        if ( chr_count >= sizeof(_desc_str) )
        {
            chr_count = sizeof(_desc_str)-1;
        }

        //
        // Convert ASCII string into UTF-16
        //
        for ( uint8_t i = 0; i < chr_count; i++ )
        {
            _desc_str[1 + i] = str[i];
        }
    }

    //
    // first byte is length (including header), second byte is string type
    //
    _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

    return _desc_str;
}
