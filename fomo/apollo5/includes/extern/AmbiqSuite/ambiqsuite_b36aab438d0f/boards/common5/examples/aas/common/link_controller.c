//*****************************************************************************
//!
//! @file link_controller.c
//!
//! @brief Link layer controller.
//!
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include <stdint.h>
#include <string.h>

#include "vendor_device.h"
#include "hid_device.h"
#include "webusb_controller.h"
#include "hid_controller.h"

#include "am_mcu_apollo.h"
#include "am_util_stdio.h"
#include "am_util_debug.h"

#include "transport.h"

//*****************************************************************************
//
// LOCAL FUNCTION DECLARATIONS
//
//*****************************************************************************
static uint32_t get_buffer_total_size(void);
static uint32_t get_buffer_idle_size(void);
static uint32_t read(uint8_t *data, uint32_t size);
static uint32_t send_int_flush_imt(const uint8_t *data, uint32_t size);
static uint32_t send_int_flush_auto(const uint8_t *data, uint32_t size);
static uint32_t send_bulk_flush_imt(const uint8_t *data, uint32_t size);
static uint32_t send_bulk_flush_auto(const uint8_t *data, uint32_t size);


// MCU link layer endpoint.
static link_layer_ep_t ll_ep =
{
    .get_buffer_total_size = get_buffer_total_size,
    .get_buffer_idle_size  = get_buffer_idle_size,
    .read                  = read,
    .send_int_flush_imt    = send_int_flush_imt,
    .send_int_flush_auto   = send_int_flush_auto,
    .send_bulk_flush_imt   = send_bulk_flush_imt,
    .send_bulk_flush_auto  = send_bulk_flush_auto,
};

static uint32_t get_buffer_total_size(void)
{
    uint32_t size;

    size = tud_vendor_write_available() + tud_vendor_available();

    am_util_debug_printf("link_layer: buffer total %d\n", size);

    return size;
}

static uint32_t get_buffer_idle_size(void)
{
    uint32_t size;

    size = tud_vendor_write_available();

    am_util_debug_printf("link_layer: buffer idle %d\n", size);

    return size;
}

static uint32_t read(uint8_t *data, uint32_t size)
{
    uint32_t read = 0;

    (void)read;
    am_util_debug_printf("link_layer: read data, need %d read %d\n", read, size);

    return size;
}

static uint32_t send_int_flush_imt(const uint8_t *data, uint32_t size)
{
    if (!is_hid_ready())
    {
        return 0;
    }

    am_util_debug_printf("link_layer: %s size %d\n", __func__, size);

#ifdef TRANSPORT_RAW_DEBUG
    for (int i = 0; i < size; i++)
    {
        if ((i & 0xf) == 0)
        {
            am_util_debug_printf("\n");
        }
        am_util_debug_printf("%02x ", data[i]);
    }
    am_util_debug_printf("\n");
#endif // TRANSPORT_RAW_DEBUG

    size = hid_send_data(data, size);

    return size;
}

static uint32_t send_int_flush_auto(const uint8_t *data, uint32_t size)
{
    am_util_debug_printf("link_layer: %s size %d\n", __func__, size);

    size = hid_send_data(data, size);

    return size;
}

static uint32_t send_bulk_flush_imt(const uint8_t *data, uint32_t size)
{
    am_util_debug_printf("link_layer: %s size %d\n", __func__, size);

    size = webusb_send_data_flush((uint8_t *)data, size);

    return size;
}

static uint32_t send_bulk_flush_auto(const uint8_t *data, uint32_t size)
{
    // am_util_debug_printf("link_layer: %s size %d\n", __func__, size);

    size = webusb_send_data_flush((uint8_t *)data, size);

    return size;
}

static void webusb_msg_cb(const uint8_t *buf, uint32_t buf_len, void *param)
{
    transport_ll_msg_handler(buf, buf_len);
}

static void ll_int_msg_handler(const uint8_t *buf, uint32_t len, void *priv)
{
    // am_util_debug_printf("link_layer: rcv %d\n", len);

#ifdef TRANSPORT_RAW_DEBUG
    for (int i = 0; i < len; i++)
    {
        if ((i & 0xf) == 0)
        {
            am_util_debug_printf("\n");
        }
        am_util_debug_printf("%02x ", buf[i]);
    }
    am_util_debug_printf("\n");
#endif // TRANSPORT_RAW_DEBUG

    if (len)
    {
        int ret = transport_ll_msg_handler(buf, len);
        if (ret)
        {
            am_util_debug_printf("link_layer: ret %d\n", ret);
        }
    }
}

void link_controller_init(void)
{
    transport_init_ll(&ll_ep);

    webusb_register_msg_cb(webusb_msg_cb, NULL);

    hid_register_req_cb(ll_int_msg_handler, NULL);

    hid_register_tx_cmpl_cb(transport_ll_tx_complete_cb, NULL);
}



