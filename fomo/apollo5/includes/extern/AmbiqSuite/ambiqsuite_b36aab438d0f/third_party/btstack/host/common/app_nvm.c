//*****************************************************************************
//
//  app_nvm.c
//! @file
//!
//! @brief Application non volatile data management.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include "am_mcu_apollo.h"
#include "am_types.h"
#include "app_nvm.h"
#include "am_logger.h"
#include "rpc_common.h"
#include "am_alloc.h"
#include "am_error.h"
#include "am_assert.h"
#include "rpc_client_nv_info.h"

/**************************************************************************************************
  Data Types
**************************************************************************************************/

/**************************************************************************************************
  Local Variables
**************************************************************************************************/

/*! Database */
uint8_t *nvm_db_pointer             = (uint8_t *)NV_MEMORY_ADDESS_START;
uint8_t *nvm_db_bt_bond_key_pointer = (uint8_t *)NV_MEMORY_ADDRESS_BOND_KEY;

// 1byte num entry + bonded key record
uint16_t total_bond_key_len;
uint16_t nvm_pkt_offset;
uint16_t nvm_remain_len;

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/*
 ****************************************************************************************
 * @brief Function used to callback nvm module event to application
 *
 * @param[in] type       packet type defined in #eBTDMReq
 * @param[in] evt_pkt    received device configuration event packet in the format #btDmRPMsgType
 * @param[in] len        packet message length
 ****************************************************************************************
 */
static void app_nvm_event_handler(uint8_t type, uint8_t *evt_pkt, uint16_t len)
{
    uint8_t *p_evt_pkt    = NULL;
    uint16_t ack_status   = GAP_ERR_NO_ERROR;

    ASSERT_ERR(evt_pkt != NULL);

    p_evt_pkt = evt_pkt;

    am_debug_printf(true, "nvm evt handler, type: %d\r\n", type);

    switch (type)
    {
        case RPMSG_NVM_DB_LOAD:
        {
            BSTREAM_TO_UINT16(ack_status, p_evt_pkt);

            if (rpc_request_is_busy())
            {
                m_return_value = ack_status;
                rpc_update_rsp_wait_flag(false);
            }
        }
        break;

        case RPMSG_NVM_DB_UPDATE:
        {
            app_nvm_update_db(p_evt_pkt, len);
        }
        break;

        default:
        break;
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Initialize the non volatile device database.
 *
 *  \return None.
 */
/*************************************************************************************************/
void app_nvm_init(void)
{
    am_util_debug_printf("nvm init\r\n");

    nvm_pkt_offset = 0;
    total_bond_key_len = 0;

    rpc_client_nvm_cb_set(app_nvm_event_handler);

    // update HFP volume gain to default value if it's invalid value in mram
    if(app_nvm_get_hfp_vol() == 0xFF)
    {
        am_info_printf(true, "update HPF vol gain to default %d\r\n", NV_MEMORY_HFP_GAIN_DEFAULT);
        app_nvm_update_hfp_vol(NV_MEMORY_HFP_GAIN_DEFAULT);
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Update the received non volatile message to specified flash space
 *
 *  \param  msg      content to update to flash
 *  \param  msg_len  content length.
 *
 */
/*************************************************************************************************/
void app_nvm_update_db(uint8_t *msg, uint16_t msg_len)
{
    uint16_t write_len = NV_MEMORY_BT_BOND_KEY_SIZE;
    static uint8_t *buf_write = NULL;
    uint8_t *p_msg = msg;
    uint32_t *dst_addr = (uint32_t *)nvm_db_bt_bond_key_pointer;
    uint16_t rcvd_bond_key_len = 0; //received bonded key length
    static uint8_t *p_buf_write = NULL;

    if(nvm_pkt_offset == 0)
    {
        uint8_t num_entry = 0;
        uint16_t total_key_rec_len = 0;

        BSTREAM_TO_UINT8(num_entry, p_msg);
        BSTREAM_TO_UINT16(total_key_rec_len, p_msg);

        total_bond_key_len = total_key_rec_len+1;

        am_info_printf(true, "total bond key len:%d\r\n", total_bond_key_len);

        buf_write = (uint8_t *)am_allocate_memory(write_len);

        if(buf_write == NULL)
        {
            am_error_printf(true, "buffer allocate failed\r\n");
            return;
        }

        rcvd_bond_key_len = msg_len-2; //exclude the length field
        p_buf_write = buf_write;

        // initialize MRAM space
        memset(p_buf_write, 0xFF, write_len);

        uint32_t ui32Critical = am_hal_interrupt_master_disable();
        int rc = am_hal_mram_main_program(AM_HAL_MRAM_PROGRAM_KEY, (uint32_t *)&buf_write, dst_addr,
                              (write_len/4));
        am_info_printf(true, "reset mram program:%d\r\n", rc);

        am_hal_interrupt_master_set(ui32Critical);

        // save entry number and key data base length to mram
        UINT16_TO_BSTREAM(p_buf_write, total_bond_key_len);
        UINT8_TO_BSTREAM(p_buf_write, num_entry);
    }
    else
    {
        am_debug_printf(true, "msg len:%d, total_bond_key_len:%d\r\n", msg_len, total_bond_key_len);
        rcvd_bond_key_len = msg_len;
    }

    nvm_pkt_offset += rcvd_bond_key_len;

    memcpy(p_buf_write, p_msg, rcvd_bond_key_len);
    p_buf_write += rcvd_bond_key_len;

    am_debug_printf(true, "total_key_len:%d,offset:%d\r\n", total_bond_key_len, nvm_pkt_offset);

    if(total_bond_key_len == nvm_pkt_offset)
    {
        nvm_pkt_offset = 0;
        p_buf_write = NULL;

        uint32_t ui32Critical = am_hal_interrupt_master_disable();

        int rc = am_hal_mram_main_program(AM_HAL_MRAM_PROGRAM_KEY, (uint32_t *)buf_write, dst_addr,
                                  (write_len/4));
        am_info_printf(true, "mram program:%d, dst_addr:0x%x, write_len:%d, entry:%d\r\n", rc, dst_addr, write_len, *nvm_db_bt_bond_key_pointer);

        am_hal_interrupt_master_set(ui32Critical);

        if(buf_write != NULL)
        {
            am_free_memory(buf_write);
        }
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Load the corresponding item type data to Ambt53
 *
 *  \param  msg      content to update to flash
 *  \param  msg_len  content length.
 *
 *  \ret    0 for allocation failure, else the send RPMsg send status
 */
/*************************************************************************************************/
int app_nvm_load(uint8_t item_type)
{
    int      send_status    = 0;
    uint16_t packet_len     = 0;
    uint8_t  *p_bond_key    = NULL;
    uint8_t *p_buf          = NULL;
    uint8_t pkt_hdr_len     = 0;

    if(item_type == NVM_ITEM_DEV_BOND_INFO)
    {
        p_bond_key = nvm_db_bt_bond_key_pointer;
        pkt_hdr_len = 3;

        BSTREAM_TO_UINT16(packet_len, p_bond_key);

        am_info_printf(true, "entry num:%d, total key len:%d\r\n",*p_bond_key, packet_len);

        // make sure the bonded key parameters in MRAM are valid
        if((packet_len > 0)&& (packet_len!=0xFFFF))
        {
            p_buf = (uint8_t *)am_allocate_memory(packet_len+pkt_hdr_len);

            if(p_buf == NULL)
            {
                am_error_printf(true, "allocate failed\r\n");
                return 0;
            }

            uint8_t *p_buf_tmp = p_buf;

            UINT8_TO_BSTREAM(p_buf_tmp, item_type);
            UINT16_TO_BSTREAM(p_buf_tmp, packet_len);

            memcpy(p_buf_tmp, p_bond_key, packet_len);
        }
    }
    else if(item_type == NVM_ITEM_HFP_VOL)
    {
        pkt_hdr_len = 1;
        packet_len = 1;
        p_buf = (uint8_t *)am_allocate_memory(packet_len+pkt_hdr_len);

        if(p_buf == NULL)
        {
            am_error_printf(true, "allocate failed\r\n");
            return 0;
        }

        uint8_t *p_buf_tmp = p_buf;

        UINT8_TO_BSTREAM(p_buf_tmp, item_type);
        *p_buf_tmp = app_nvm_get_hfp_vol();
    }

    if(p_buf != NULL)
    {
        uint16_t remain_len = packet_len + pkt_hdr_len;
        uint16_t offset_len = 0;

        am_info_printf(true, "remain len:%d\r\n", remain_len);

        while (remain_len)
        {
            uint16_t pkt_send_len = (remain_len <= RPMSG_NVM_PAYLOAD_MAX)?remain_len:RPMSG_NVM_PAYLOAD_MAX;

            send_status = rpc_client_nvm_send_packet(RPMSG_NVM_DB_LOAD,
                                               &p_buf[offset_len],
                                               pkt_send_len);
            if (send_status < 0)
            {
                am_error_printf(true, "send nvm bond info failed, %d\r\n", send_status);
            }

            offset_len += pkt_send_len;
            remain_len -= pkt_send_len;
            am_debug_printf(true, "cont. remain_len:%d\r\n", remain_len);
        }

        am_free_memory(p_buf);
    }

    return send_status;
}

void app_nvm_update_hfp_vol(uint8_t gain)
{
    uint16_t write_len = NV_MEMORY_SYS_SETTING_SIZE;

    uint8_t *buf_write = (uint8_t *)am_allocate_memory(write_len);

    if(buf_write == NULL)
    {
        am_error_printf(true, "buffer allocate failed\r\n");
        return ;
    }

    uint32_t ui32Critical = am_hal_interrupt_master_disable();

    *buf_write = gain;
    int rc = am_hal_mram_main_program(AM_HAL_MRAM_PROGRAM_KEY, (uint32_t *)buf_write, (uint32_t *)nvm_db_pointer,
                              (write_len/4));

    am_info_printf(true, "mram program:%d, updated hfp vol:%d\r\n", rc, *nvm_db_pointer);

    am_hal_interrupt_master_set(ui32Critical);
}

uint8_t app_nvm_get_hfp_vol(void)
{
    return *nvm_db_pointer;
}
