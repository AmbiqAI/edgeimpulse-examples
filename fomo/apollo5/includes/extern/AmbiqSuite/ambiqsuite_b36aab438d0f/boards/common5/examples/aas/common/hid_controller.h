//*****************************************************************************
//
//! @file hid_controller.h
//!
//! @brief Connect MCU and Audio Tool via HID.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef HID_CONTROLLER_H
#define HID_CONTROLLER_H

#if defined __cplusplus
extern "C"
{
#endif

    typedef void (*hid_rx_cb)(const uint8_t *buf, uint32_t buf_len,
                                 void *param);

    typedef void (*hid_tx_cmpl_cb)(uint32_t buf_len, void *param);

    bool is_hid_ready(void);

    /**
     * @brief Blocking HID Send Data
     *
     * @param buf Pointer to buffer with data to be sent
     * @param bufsize Requested number of bytes
     * @return bool True is successful, false is failure.
     */
    bool hid_send_data(const uint8_t *buf, uint32_t bufsize);


        /**
     * @brief Register request callback function and pass to callback pointer
     *
     * @param cd Callback function
     * @param param Pointer to user parameter which will be passed to callback
     *
     */
    void hid_register_req_cb(hid_rx_cb cb, void *param);

        /**
     * @brief Register tx complete callback function and pass to callback pointer
     *
     * @param cd Callback function
     * @param param Pointer to user parameter which will be passed to callback
     *
     */
    void hid_register_tx_cmpl_cb(hid_tx_cmpl_cb cb, void *param);

#if defined __cplusplus
}
#endif

#endif // HID_CONTROLLER_H
