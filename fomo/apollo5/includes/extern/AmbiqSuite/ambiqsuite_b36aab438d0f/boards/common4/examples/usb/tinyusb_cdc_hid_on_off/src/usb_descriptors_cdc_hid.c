//*****************************************************************************
//
//! @file usb_descriptors_cdc_hid.c
//!
//! @brief contains descriptor handling for this example. This consists of
//! three separate USB profiles. (so there are three separate USB descriptors)
//! The selected profile is loaded during USB enumeration.
//!
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

#include "usb_descriptors.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4020 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

//
//! collect global RAM used in this file here
//
typedef struct
{
    //
    //! the USB profile being used
    //
    usb_profile_e   eProfileNumber;
    //
    //! This is global because it has to exist long enough for a transfer to complete
    //
    uint16_t        desc_str[32];
}
hid_var_t ;

//
//! allocate global RAM used in this file
//
static hid_var_t g_usbdv ;


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+

//
//! composite HID + CDC
//
const tusb_desc_device_t  desc_device_wHID =
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
    .idProduct          = USB_PID + 2,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

//
//! Composite CDC only
//
const tusb_desc_device_t desc_device_cdc_composite =
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
    .idProduct          = USB_PID + 1,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

//
//! Non-Composite CDC only
//
const tusb_desc_device_t desc_device_cdc =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    // Use Interface Association Descriptor (IAD) for CDC
    // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
    .bDeviceClass       = TUSB_CLASS_CDC,
    .bDeviceSubClass    = 0,
    .bDeviceProtocol    = 0,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

//
//! used to collect the use profiles used in a table
//
typedef struct
{
    const tusb_desc_device_t *devDesciptor;
    char * descriptorName;

}
devDescTable_t ;

//
//! table mapping usb profiles and names to the profile enum
//
static const devDescTable_t devDescTable[e_PROFILE_MAX] =
{
    [e_PROFILE_CDC]               = { .devDesciptor = &desc_device_cdc, .descriptorName = "CDC non Composite" },
    [e_PROFILE_COMPOSITE_CDC]     = { .devDesciptor = &desc_device_cdc_composite, .descriptorName = "CDC Composite" },
    [e_PROFILE_COMPOSITE_CDC_HID] = { .devDesciptor = &desc_device_wHID, .descriptorName = "CDC HID Composite" },
};

//
//! @brief Invoked when received GET DEVICE DESCRIPTOR
//! @return Application return pointer to descriptor
//
const uint8_t *tud_descriptor_device_cb(void)
{
    if (g_usbdv.eProfileNumber >= e_PROFILE_MAX)
    {
        g_usbdv.eProfileNumber = e_PROFILE_CDC;
    }
    return (const uint8_t *) devDescTable[g_usbdv.eProfileNumber].devDesciptor;

}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
enum
{
    ITF_NUM_CDC = 0,
    ITF_NUM_CDC_DATA,
    ITF_NUM_HID,
    ITF_NUM_TOTAL   // needs subtract 1 when doHid is false
};

#define CONFIG_TOTAL_LENHID     (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_DESC_LEN)
#define CONFIG_TOTAL_LEN_NO_HID    (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN )

#define EPNUM_CDC_NOTIF   0x81
#define EPNUM_CDC_OUT     0x02
#define EPNUM_CDC_IN      0x82

#define EPNUM_HID         0x03

const uint8_t desc_hid_report[] =
{
    TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_EP_BUFSIZE)
};


//
//! config descriptor for composite CDC and HID full-speed
//
const uint8_t desc_fs_configurationHID[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LENHID, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),

    // Interface number, string index, protocol, report descriptor len, EP In & Out address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)
};

//
//! config descriptor for composite CDC full-speed
//
const uint8_t desc_fs_configuration[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL-1, 0, CONFIG_TOTAL_LEN_NO_HID, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
};

//
//! config descriptor for composite CDC-HID high-speed
//
uint8_t const desc_hs_configurationHID[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LENHID, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 512),

    // Interface number, string index, protocol, report descriptor len, EP In & Out address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_report), EPNUM_HID, CFG_TUD_HID_EP_BUFSIZE, 5)
};

//
//! config descriptor for composite CDC high-speed
//
uint8_t const desc_hs_configuration[] =
{
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL-1, 0, CONFIG_TOTAL_LEN_NO_HID, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, 4, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 512),
};


//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// array of pointers to string descriptors
const char *string_desc_arr [] =
{
    (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
    "TinyUSB",                     // 1: Manufacturer
    "TinyUSB Device",              // 2: Product
    "123458",                      // 3: Serials, should use chip ID
    "TinyUSB CDC",                 // 4: CDC Interface
    "TinyUSB HID",                 // 5: HID Interface
};

//*****************************************************************************
// print current USB profile name
//*****************************************************************************
usb_profile_e usb_desc_print_profile_name(void)
{
    usb_profile_e eProfile = g_usbdv.eProfileNumber;

    const char *cpProfileName = (eProfile >= e_PROFILE_MAX) ?
                                "Invalid index" :
                                devDescTable[eProfile].descriptorName;


    am_util_stdio_printf("current USB profile is %s\n", cpProfileName );

    return eProfile;
}

//*****************************************************************************
// set USB profile
//*****************************************************************************
void usb_desc_set_profile(usb_profile_e eProfile)
{
    if ( eProfile < e_PROFILE_MAX)
    {
        g_usbdv.eProfileNumber = eProfile;
    }
    usb_desc_print_profile_name() ;
}

//*****************************************************************************
// increment to next USB profile
//*****************************************************************************
usb_profile_e usb_desc_set_next_profile(void)
{
    if (++g_usbdv.eProfileNumber >= e_PROFILE_MAX)
    {
        g_usbdv.eProfileNumber = (usb_profile_e) 0;
    }

    return usb_desc_print_profile_name();
}


//*****************************************************************************
// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// contents must exist long enough for transfer to complete
//*****************************************************************************
const uint8_t *tud_hid_descriptor_report_cb(uint8_t itf)
{
    (void) itf;
    return desc_hid_report;
}

//
//! table maps FS config descriptors to profile enum
//
static const uint8_t *descCfgTableFS[] = {
    desc_fs_configuration,
    desc_fs_configuration,
    desc_fs_configurationHID,
};

//
//! table maps HS config descriptors to profile enum
//
static const uint8_t *descCfgTableHS[] = {
    desc_hs_configuration,
    desc_hs_configuration,
    desc_hs_configurationHID,
};


//*****************************************************************************
// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
//  contents must exist long enough for transfer to complete
//*****************************************************************************
const uint8_t *tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index; // for multiple configu

    if (g_usbdv.eProfileNumber >= e_PROFILE_MAX)
    {
        return NULL;
    }

    if (tud_speed_get() == TUSB_SPEED_HIGH)
    {
        return descCfgTableHS[g_usbdv.eProfileNumber];
    }
    return descCfgTableFS[g_usbdv.eProfileNumber];
} // end tud_descriptor_configuration_cb()

//*****************************************************************************
//
// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
//
//*****************************************************************************
const uint16_t *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void) langid;

    uint8_t chr_count;

    if ( index == 0)
    {
        memcpy(&g_usbdv.desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    }
    else
    {
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if ( index >= (sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) )
        {
            return NULL;
        }

        const char* str = string_desc_arr[index];

        // Cap at max char
        chr_count = strlen(str);
        if ( chr_count > 31 )
        {
            chr_count = 31;
        }

        // Convert ASCII string into UTF-16
        for ( uint8_t i = 0; i < chr_count; i++ )
        {
            g_usbdv.desc_str[1 + i] = (uint16_t) str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    g_usbdv.desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8 ) | (2 * chr_count + 2));

    return g_usbdv.desc_str;
}
