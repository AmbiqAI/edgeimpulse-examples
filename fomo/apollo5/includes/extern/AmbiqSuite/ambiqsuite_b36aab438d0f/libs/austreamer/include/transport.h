/**
 ******************************************************************************
 *
 * @file transport.h
 *
 * @brief Audio transport layer APIs.
 *
 * Copyright (C) Ambiq Micro
 *
 *
 ******************************************************************************
 */

#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

#include <stdint.h>

//*****************************************************************************
// Configuration
//*****************************************************************************
#define CRC_SWITCH (CRC_OFF) // CRC_ON or CRC_OFF
#define CRC_LEN    (4)

#define LL_REQ_MTU (64)
#define LL_RSP_MTU (64)
#define LL_NTF_MTU (64)
#define LL_DLS_MTU (512)
#define LL_ULS_MTU (512)
//*****************************************************************************

#if defined __cplusplus
extern "C"
{
#endif

#define TRANSPORT_HEADER_LEN (6)

    typedef struct transport
    {
        uint16_t module_id    : 4; /**< Module ID */
        uint16_t msg_type     : 4; /**< Message Type */
        uint16_t msg_sub_type : 4; /**< Message Sub Type */
        uint16_t pb_flag      : 2; /**< Packet Boundary Flag */
        uint16_t flush_flag   : 1; /**< Link Layer Flush Flag */
        uint16_t crc_flag     : 1; /**< CRC Flag */
        uint16_t trid;             /**< Transaction ID */
        uint16_t data_len;         /**< Message Data Length */
        uint8_t  data[0];          /**< Message Specific Data */
    } __attribute__((packed)) transport_t;

    typedef enum MODULE_ID
    {
        MODULE_SCAN            = 0,    /**< Reserved for scan */
        MODULE_AUDIO_TOOL      = 1,    /**< Audio Tool */
        MODULE_AUDIO_BROADCAST = 0xFF, /**< Reserved for broadcast */
    } module_id_t;

    typedef enum MSG_TYPE
    {
        REQ = 0, /**< Request. Host to device */
        RSP = 1, /**< Response. Device to host */
        NTF = 2, /**< Notification. Device to host */
        DLS = 3, /**< Downlink Stream */
        ULS = 4  /**< Uplink Stream */
    } msg_type_t;

    typedef enum REQ_SUB_TYPE
    {
        REQ_GENERIC   = 0, /**< Generic REQ with Message-Specific-Data */
        REQ_ULS_START = 1, /**< REQ to start ULS */
        REQ_ULS_STOP  = 2, /**< REQ to stop ULS */
    } req_sub_type_t;

    typedef enum RSP_SUB_TYPE
    {
        RSP_SUCCESS   = 0, /**< Generic RSP Success */
        RSP_ULS_START = 1, /**< RSP for REQ_ULS_START. Start ULS */
        RSP_ULS_STOP  = 2, /**< RSP for REQ_ULS_STOP. Stop ULS */
        RSP_ERR_PB    = 3, /**< PB-Flag Incorrect */
        RSP_ERR_TRID  = 4, /**< Transaction-ID Incorrect */
        RSP_ERR_LEN   = 5, /**< Message-Data-Length Incorrect */
        RSP_ERR_CRC   = 6, /**< CRC Incorrect */
    } rsp_sub_type_t;

    typedef enum NTF_SUB_TYPE
    {
        NTF_GENERIC   = 0, /**< Generic NTF with Message-Specific-Data */
        NTF_BUF_TOTAL = 1, /**< NTF for DLS Total Buffer Size. DLS-Total-Buffer
                              use the Transaction-ID field */
        NTF_BUF_IDLE = 2, /**< NTF for DLS Idle Buffer Size. DLS-Idle-Buffer use
                             the Transaction-ID field */
        NTF_BUF_CLEAN = 3, /**< NTF for DLS Buffer Clean. DLS-Idle-Buffer use
                              the Transaction-ID field */
    } ntf_sub_type_t;

    typedef enum DLS_SUB_TYPE
    {
        DLS_RESERVED = 0, /**< Reserved */
        DLS_FIRST =
            1,          /**< First DLS without Message-Specific-Data.
                           Message-Data-Length should 0. Flush-Flag should be 1 */
        DLS_MIDDLE = 2, /**< Middle DLS with Message-Specific-Data. Flush-Flag
                           should be 0 */
        DLS_LAST = 3,   /**< Last DLS with Message-Specific-Data. Flush-Flag
                           should be 1 */
    } dls_sub_type_t;

    typedef enum ULS_SUB_TYPE
    {
        ULS_GENERIC = 0, /**< Generic ULS with Message-Specific-Data */
    } uls_sub_type_t;

    typedef enum PB_FLAG
    {
        PB_CPL         = 0, /**< Header and a complete packet */
        PB_FRAG_HEADER = 1, /**< Header and first fragment */
        PB_FRAG        = 2, /**< Fragment */
        PB_FRAG_LAST   = 3, /**< Last fragment */
    } pb_flag_t;

    typedef enum FLUSH_FLAG
    {
        FLUSH_AUTO = 0, /**< Flush depends on link layer */
        FLUSH_IMT  = 1, /**< Flush immediately */
    } flush_flag_t;

    typedef enum CRC_FLAG
    {
        CRC_OFF = 0, /**< Message has no CRC validation */
        CRC_ON  = 1, /**< Message has CRC32 validation */
    } crc_flag_t;

    typedef struct transport_endpoint
    {
        /**
         * @brief Request message callback, handle in_buf and feed back into
         * out_buf.
         *
         * @param in_buf Input buffer.
         * @param in_size Input buffer size.
         * @param out_buf Output buffer pointer, this buffer is allocated by
         * upper layer and will be freed by void (*free)(void *ptr) callback.
         * @param out_size Output buffer size pointer.
         * @param priv  Private pointer.
         */
        int (*req_cb)(const uint8_t *in_buf, uint32_t in_size,
                      uint8_t **out_buf, uint32_t *out_size, void *priv);

        /**
         * @brief Uplink Stream start message callback.
         *
         * @param in_buf Input buffer.
         * @param in_size Input buffer size.
         * @param priv  Private pointer.
         */
        int (*uls_start_cb)(const uint8_t *in_buf, uint32_t in_size,
                            void *priv);

        /**
         * @brief Uplink Stream stop message callback.
         *
         * @param in_buf Input buffer.
         * @param in_size Input buffer size.
         * @param priv  Private pointer.
         */
        int (*uls_stop_cb)(const uint8_t *in_buf, uint32_t in_size, void *priv);

        /**
         * @brief Downlink Stream start message callback.
         *
         * @param priv  Private pointer.
         */
        int (*dls_start_cb)(void *priv);

        /**
         * @brief Downlink Stream stop message callback.
         *
         * @param priv  Private pointer.
         */
        int (*dls_stop_cb)(void *priv);

        /**
         * @brief Memory free callback to free the memory allocated by upper
         * layer.
         *
         * @param ptr  Memory pointer.
         */
        void (*free)(void *ptr);

        /**
         * @brief Private pointer.
         */
        void *priv;
    } transport_ep_t;

    typedef struct link_layer_endpoint
    {
        /**
         * @brief Get link layer buffer total size.
         */
        uint32_t (*get_buffer_total_size)(void);

        /**
         * @brief Get link layer buffer idle size.
         */
        uint32_t (*get_buffer_idle_size)(void);

        /**
         * @brief Read data from link layer buffer to out buffer.
         *
         * @param data Data buffer to store the read data.
         * @param size Size of the data to be read.
         * @return Actual amount of read data size in bytes.
         */
        uint32_t (*read)(uint8_t *data, uint32_t size);

        /**
         * @brief Send data to link layer interrupt endpoint and flush
         * immediately.
         *
         * @param data Data buffer pointer.
         * @param size Size of the buffer.
         * @return Actual amount of sent data size in bytes.
         */
        uint32_t (*send_int_flush_imt)(const uint8_t *data, uint32_t size);

        /**
         * @brief Send data to link layer interrupt endpoint and flush auto
         * decided by link layer.
         *
         * @param data Data buffer pointer.
         * @param size Size of the buffer.
         * @return Actual amount of sent data size in bytes.
         */

        uint32_t (*send_int_flush_auto)(const uint8_t *data, uint32_t size);
        /**
         * @brief Send data to link layer bulk endpoint and flush immediately.
         *
         * @param data Data buffer pointer.
         * @param size Size of the buffer.
         * @return Actual amount of sent data size in bytes.
         */
        uint32_t (*send_bulk_flush_imt)(const uint8_t *data, uint32_t size);

        /**
         * @brief Send data to link layer bulk endpoint and flush auto decided
         * by link layer.
         *
         * @param data Data buffer pointer.
         * @param size Size of the buffer.
         * @return Actual amount of sent data size in bytes.
         */
        uint32_t (*send_bulk_flush_auto)(const uint8_t *data, uint32_t size);
    } link_layer_ep_t;

    /**
     * @brief Initialize transport.
     *
     * @param ep Transport endpoint reference.
     */
    void transport_init(transport_ep_t *ep);

    /**
     * @brief Initialize link layer.
     *
     * @param ep Link layer endpoint reference.
     */
    void transport_init_ll(link_layer_ep_t *ep);

    /**
     * @brief Transport message handler called by link layer.
     *
     * @param msg Link layer message in type of transport_t.
     * @param len Message length.
     * @return 0 success, otherwise error code.
     */
    int transport_ll_msg_handler(const uint8_t *msg, uint32_t len);

    /**
     * @brief Link layer tx complete callback.
     *
     * @param size The last completed tx size.
     * @param priv Private pointer.
     */
    void transport_ll_tx_complete_cb(uint32_t size, void *priv);

    /**
     * @brief Read downlink stream data from transport called by upper layer.
     *
     * @param data Data buffer to store the read data.
     * @param size Size of the data to be read.
     * @return Actual amount of read data size in bytes.
     */
    int transport_dls_read_data(uint8_t *data, uint32_t size);

    /**
     * @brief Send notification to transport called by upper layer.
     *
     * @param data Data buffer to be sent.
     * @param size Size of the data buffer.
     * @return 0 success, otherwise error code.
     */
    int transport_send_notification(const uint8_t *data, uint32_t size);

    /**
     * @brief Send uplink stream to transport called by upper layer.
     *
     * @param data Data buffer to be sent.
     * @param size Size of the data buffer.
     * @return 0 success, otherwise error code.
     */
    int transport_send_uplink_stream(const uint8_t *data, uint32_t size);

#if defined __cplusplus
}
#endif

#endif /* _TRANSPORT_H_ */
