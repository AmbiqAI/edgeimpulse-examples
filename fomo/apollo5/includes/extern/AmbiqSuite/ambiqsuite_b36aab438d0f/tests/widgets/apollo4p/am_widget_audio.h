//*****************************************************************************
//
//! @file am_widget_audio.h
//!
//! @brief Functions for using the IOM/IOS/UART interface for audio application.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_WIDGET_AUDIO_H
#define AM_WIDGET_AUDIO_H

#ifdef __cplusplus
extern "C"
{
#endif

#define UART_BUFFER_SIZE 2048
#define GPIO_COM_UART_TX  30
#define GPIO_COM_UART_RX  32
#define GPIO_COM_UART_RTS 76
#define GPIO_COM_UART_CTS 77

#ifdef CODEC_MODE
//
// Address of control words in CODEC DSP MEM for sync between Apollo4 and CODEC
//
#define CONTROL_WORD_ADDR 0x000A7FFC // last 2 words in DSP1 XMEM in CODEC
//
// bit 31: 0 - write, 1 - read
//
#define RW_BIT_MASK 0x80000000
//
// SPI master reads 2 words (8 bytes) from codec, only 6 bytes are useful
// because bit width of DSP MEM is 24-bit, the 8 MSBs in each word are all 0's.
//
#define CONTROL_WORDS_LENGTH 2
//
// A simple communication protocal between codec and apollo4
//
// control word 0 bit 0.
// Set by slave - request master to read and provide master the buffer address to read and the packet length.
// Clear by master - accept read request.
//
#define SLAVE_REQUEST_READ_MSK    0x00000001
//
// control word 0 bit 1.
// Set by master - notify slave that read complete.
// Clear by slave - acknowledgement.
//
#define MASTER_READ_COMPLETE_MSK  0x00000002
//
// control word 0 bit 2.
// Set by master - request to write to slave and provide slave the packet length.
// Clear by slave - accept write request and provide master the buffer address to write.
//
#define MASTER_REQUEST_WRITE_MSK  0x00000004
//
// control word 0 bit 3.
// Set by master - notify slave that write complete.
// Clear by slave - acknowledgement.
//
#define MASTER_WRITE_COMPLETE_MSK 0x00000008
//
// transfer length
//
#define TRANSFER_LENGTH_MSK       0x00FFFF00 // control word 0 bit 8~23
//
// SPI buffer address in codec
//
#define SLAVE_BUFFER_ADDR_MSK     0x00FFFFFF // control word 1 bit 0~23
#endif

// IOINT bits for handshake
#define HANDSHAKE_IOM_TO_IOS     0x1
#define HANDSHAKE_IOS_TO_IOM     0x2
#define HANDSHAKE_IOM_PIN        8 // GPIO used to connect the IOSINT to host side
#define HANDSHAKE_IOS_PIN        4

#define MST_RECV_INT_FROM_SLV_PIN    HANDSHAKE_IOM_PIN // For handshake with codec
#define MST_SEND_INT_TO_SLV_PIN      5 // For handshake with codec

extern AM_SHARED_RW uint8_t g_pUartRecvPacket[];
extern AM_SHARED_RW uint8_t g_pIomRecvPacket[];
extern AM_SHARED_RW uint8_t g_pIosRecvPacket[];
typedef struct
{
    uint32_t            ui32PacketSize;
    uint32_t            ui32PacketAddress;
    uint32_t            ui32LramSize;
    uint8_t             ui8LramAddress;

} am_widget_audio_spi_test_t;

typedef struct
{
    uint8_t  uart;
    uint32_t length;

    bool txblocking;
    bool rxblocking;

    uint32_t txbufferSize;
    uint32_t rxbufferSize;

    uint32_t txBytesTransferred;
    uint32_t rxBytesTransferred;

    bool txQueueEnable;
    bool rxQueueEnable;

    uint8_t pui8QueueTx[UART_BUFFER_SIZE];
    uint8_t pui8QueueRx[UART_BUFFER_SIZE];

    uint8_t * pui8SpaceTx;
    uint8_t * pui8SpaceRx;

    uint32_t result;
}
am_widget_uart_loopback_config_t;

typedef struct
{
    uint32_t length;

    bool txblocking;

    uint32_t txbufferSize;

    uint32_t txBytesTransferred;

    bool txQueueEnable;

    uint8_t *pui8QueueTx;

    uint8_t *pui8SpaceTx;
}
am_widget_uart_send_config_t;

typedef struct
{
    uint32_t length;

    bool rxblocking;

    uint32_t rxbufferSize;

    uint32_t rxBytesTransferred;

    bool rxQueueEnable;

    uint8_t *pui8QueueRx;

    uint8_t *pui8SpaceRx;

}
am_widget_uart_recv_config_t;

typedef struct
{
    uint32_t            iomModule;
    am_hal_iom_config_t iomHalCfg;
} am_widget_audio_stimulus_iom_t;

typedef struct
{
    uint32_t            iosModule;
    am_hal_ios_config_t iosHalCfg;
} am_widget_audio_stimulus_ios_t;

typedef struct
{
    uint32_t             uartModule;
    am_hal_uart_config_t uartHalCfg;
} am_widget_audio_stimulus_uart_t;

typedef struct
{
    am_widget_audio_stimulus_ios_t stimulusCfgIos;
    am_widget_audio_stimulus_iom_t stimulusCfgIom;
    am_widget_audio_stimulus_uart_t stimulusCfgUart;
} am_widget_audio_config_t;

uint32_t
am_widget_audio_setup(am_widget_audio_config_t *pAudioCfg, void **ppWidget);

uint32_t
clear_buffers(void);

#ifdef CODEC_MODE
uint32_t
am_widget_audio_receive_packet_from_codec(uint8_t * pui8PacketAddr, uint32_t * pui32BuffSize);

uint32_t
am_widget_audio_send_packet_to_codec(uint8_t * pui8PacketAddr, uint32_t ui32BuffSize);
#endif

uint32_t
am_widget_audio_ios2iom_xfer(void *pWidget, void *pTestCfg);

uint32_t
am_widget_audio_iom2ios_xfer(void *pWidget, void *pTestCfg);

uint32_t
am_widget_audio_uart_loopback(void *pWidget, am_widget_uart_loopback_config_t *psConfig);

void
am_widget_audio_cleanup(void);

uint32_t
verify_result(uint32_t ui32Length, uint8_t * pui8Input, uint8_t * pui8Output);

#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_AUDIO_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************